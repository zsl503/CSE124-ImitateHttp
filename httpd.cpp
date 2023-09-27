#include "httpd.h"
#include "HttpBuilder.h"
#include "MessageParser.h"
#include "rulematch.h"
#include "threadpool.h"
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <set>
#include <sstream>
#include <stack>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <bitset>

using std::cout;
using std::endl;
using std::string;
using std::stringstream;

RuleList *RULE_LIST;

std::map<std::string, std::string> CONTENT_TYPE_MAP = {
    {"html", "text/html;charset=utf-8;"},
    {"htm", "text/html;charset=utf-8;"},
    {"jpg", "image/jpeg;"},
    {"jpeg", "image/jpeg;"},
    {"png", "image/png;"},
    {"gif", "image/gif;"},
    {"ico", "image/x-icon;"},
    {"svg", "image/svg+xml;"},
    {"js", "application/javascript;"},
    {"xml", "application/xml;"},
    {"css", "text/css;"},
    {"txt", "text/plain;"},
};

string DOC_ROOT = "";

void sendAll(int sockfd, const string &msg)
{
    const char *s = msg.c_str();
    size_t pos = 0, errcnt = 0;
    int tmp;
    while (pos != msg.size())
    {
        tmp = send(sockfd, s + pos, msg.size() - pos, MSG_NOSIGNAL);
        if (tmp != -1)
        {
            pos += tmp;
            errcnt = 0;
        }
        else if (tmp == -1 || ++errcnt >= 3)
            return;
    }
}

bool vaildUrl(const string &url)
{
    stringstream ss(url);
    string line;
    std::stack<string> s;

    getline(ss, line, '/');
    while (getline(ss, line, '/'))
    {
        if (line == "..")
        {
            if (s.empty())
                return false;
            else
                s.pop();
        }
        s.push(line);
    }
    return true;
}

HttpBuilder handle_file(const string filename)
{
    cout << "Request file:" << DOC_ROOT + filename << endl;
    std::ifstream in((DOC_ROOT + filename), std::ios::in);
    if (!in.is_open())
        return HttpBuilder::getNotFound(filename);
    else
    {
        std::istreambuf_iterator<char> beg(in), end;
        string str(beg, end);
        in.close();
        HttpBuilder h = HttpBuilder("OK", "200", "HTTP/1.1", false)
                            .setHeader("Content-Length", str.size())
                            .setHeader("Server", "CentOS")
                            .setBody(str);

        const size_t pos = filename.rfind(".");
        const string back = filename.substr(pos + 1);
        if (CONTENT_TYPE_MAP.find(back) != CONTENT_TYPE_MAP.end())
            h.setHeader("Content-Type", CONTENT_TYPE_MAP[back]);
        else
            h.setHeader("Content-Type", "application/octet-stream");

        struct stat result;
        if (stat((DOC_ROOT + filename).c_str(), &result) == 0)
        {
            time_t mod_time = result.st_mtime;
            time(&mod_time); /*获取time_t类型的当前时间*/
            char *t = asctime(gmtime(&mod_time));
            t[strlen(t) - 1] = '\0';
            h.setHeader("Last-Modified", t);
        }
        else
            return HttpBuilder::getNotFound(filename);
        return h;
    }
}

void handle_request(int client_socket, const HttpBuilder h)
{
    // 处理HTTP请求
    HttpBuilder resp;
    // 处理GET请求
    if (h.getHeader("Host") == "")
        resp = HttpBuilder::getClientError();
    else if (h.meth == "")
        resp = HttpBuilder::getClientError();
    else if (h.meth == "GET")
    {
        if (!vaildUrl(h.getUrl().addr))
            resp = HttpBuilder::getNotFound(h.getUrlStr());
        else if (h.getHeader("Host") == "")
            resp = HttpBuilder::getClientError();
        else
            try
            {
                resp = handle_file(h.getUrl().addr);
            }
            catch (const std::exception &e)
            {
                std::cerr << "catch:" << e.what();
                resp = HttpBuilder::getNotFound(h.getUrlStr());
            }
    }
    else
        resp = HttpBuilder::getNotAllowed(h.meth, h.getUrlStr());

    if (resp.getHeader("Connection") == "")
    {
        if (h.getHeader("Connection") == "")
            resp.setHeader("Connection", "keep-alive");
        else
            resp.setHeader("Connection", h.getHeader("Connection"));
    }

    sendAll(client_socket, resp.toString());
}

void handle_client(int client_socket, MessageParser mp, string buffer)
{
    string falseStr;
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    socklen_t len = sizeof(struct sockaddr);

    getpeername(client_socket, (struct sockaddr *)&sin, &len);
    bool isAccess = RULE_LIST ? RULE_LIST->pass(sin, falseStr) : true;

    if (!isAccess)
    {
        sendAll(client_socket, HttpBuilder::getForbidden(falseStr).toString());
        shutdown(client_socket, SHUT_RDWR);
        close(client_socket);
        return;
    }
    mp.pushMsg(buffer);

    if (mp.empty())
        return;

    HttpBuilder h = mp.popHttp();

    if (h.getHeader("Connection") == "close")
    {
        shutdown(client_socket, SHUT_RDWR);
        close(client_socket);
        return;
    }
    // http1.1 中是半双工的，同个tcp连接不同报文不能并发，因此此处没有thread，http2进行tcp多路复用，才会用到thread
    // new thread(handle_request, client_socket, h);
    handle_request(client_socket, h);
}

void start_httpd(unsigned short port, string doc_root, int thread_num)
{
    typedef struct sockaddr *SP;

    if (doc_root[doc_root.size() - 1] == '/')
        doc_root = doc_root.substr(0, doc_root.size() - 1);
    DOC_ROOT = doc_root;

    std::cerr << "Starting server (port: " << port << ", doc_root: " << doc_root << ")" << endl;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (0 > sockfd)
    {
        perror("socket");
        return;
    }

    // 准备地址
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    socklen_t addrlen = sizeof(addr);

    // 绑定
    if (bind(sockfd, (SP)&addr, addrlen))
    {
        perror("bind");
        return;
    }

    // 监听
    if (listen(sockfd, SOMAXCONN))
    {
        perror("listen");
        return;
    }

    threadpool::threadpool *tp = nullptr;
    struct sockaddr_in client_address;
    if (thread_num > 0)
    {
        tp = new threadpool::threadpool(thread_num);
    }

    RULE_LIST = RuleList::getFromFile(doc_root + "/.htaccess");
    if (RULE_LIST == nullptr)
        std::cerr << "Can't open or read the .htaccess file" << endl;

    struct timeval timeout = {5, 0};

    int rc;
    fd_set fds_r;
    FD_ZERO(&fds_r);
    std::map<int, MessageParser> waitfds;
    cout << "Got socketID:" << sockfd << endl;
    int maxfd = sockfd;
    while (true)
    {
        // 等待连接
        FD_SET(sockfd, &fds_r);
        for (auto &i : waitfds)
        {
            FD_SET(i.first, &fds_r);
            maxfd = i.first > maxfd ? i.first:maxfd;
        }

        rc = select(maxfd + 1, &fds_r, NULL, NULL, NULL);
        if (rc == 0)
        {
            std::cerr << "Select return 0 without timeout. It shouldn't appear. Error code." << endl;
            continue;
        }
        else if (rc < 0)
        {
            perror("erro:" );
            std::cerr << "Runtime error." << rc << endl;
            continue;
        }
        else if (FD_ISSET(sockfd, &fds_r))
        {
            rc--;
            int clifd = accept(sockfd, (SP)&client_address, &addrlen);
            if (0 <= clifd)
            {
                FD_SET(clifd, &fds_r);
                waitfds[clifd] = MessageParser();
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_address.sin_addr, ip, INET_ADDRSTRLEN);
                cout << "Connection from: " << ip << ":" << ntohs(client_address.sin_port) << ". Allocate client ID: " << clifd << endl;
            }
            else
                std::cerr << ("Accept error") << endl;
        }

        for (auto iter = waitfds.begin(); rc > 0 && iter != waitfds.end();)
        {
            if (FD_ISSET(iter->first, &fds_r))
            {
                rc--;
                char buffer[BUFFER_SIZE + 1];
                setsockopt(iter->first, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
                int res = recv(iter->first, buffer, BUFFER_SIZE, 0);
                if (res == 0){
                    cout << "Client " << iter->first << " has closed." << endl;                
                    FD_CLR(iter->first, &fds_r);
                    waitfds.erase(iter++);
                }
                else if (res < 0){    
                    perror("errno");
                    shutdown(iter->first, SHUT_RDWR);
                    FD_CLR(iter->first, &fds_r);
                    close(iter->first);
                    waitfds.erase(iter++);
                }
                else
                {
                    buffer[res] = '\0';
                    if (tp != nullptr)
                        tp->commit(handle_client, iter->first, iter->second, buffer);
                    else
                        new std::thread(handle_client, iter->first, iter->second, string(buffer));
                    iter++;
                }
            }
            else
                iter++;
        }
        if (rc != 0)
        {
            cout << "-----info-----" << endl;
            std::cerr << "Still " << rc << " waitlist. It shouldn't appear. Error in code." << endl;
            cout << std::bitset<30>(fds_r.fds_bits[0]) << endl;
            // cout all key in map
            for (auto iter = waitfds.begin(); iter != waitfds.end(); iter++)
            {
                cout << iter->first << " ";
            }
            cout << endl;
            cout << "------end-----" << endl;
        }
    }
    delete RULE_LIST;
    delete tp;
    return;
}