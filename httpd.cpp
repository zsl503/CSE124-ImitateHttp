#include "httpd.h"
#include "HttpBuilder.h"
#include "MessageParser.h"
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <stack>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <thread>
#include <unistd.h>

using namespace std;

#define THREAD_NUM 200

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

bool vaildAddr(const string &addr)
{
    stringstream ss(addr);
    string line;
    stack<string> s;

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
    if (h.meth == "")
        resp = HttpBuilder::getClientError();

    else if (h.meth == "GET")
    {
        if (!vaildAddr(h.getUrl().addr))
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
                cerr << "catch:" << e.what();
                resp = HttpBuilder::getNotFound(h.getUrlStr());
            }
    }
    else
        resp = HttpBuilder::getNotAllowed(h.meth, h.getUrlStr());

    if (h.getHeader("Connection") == "")
        resp.setHeader("Connection", "keep-alive");
    else
        resp.setHeader("Connection", h.getHeader("Connection"));

    sendAll(client_socket, resp.toString());
}

void handle_client(int client_socket)
{
    char buffer[BUFFER_SIZE + 1];
    ssize_t request_size;
    // 读取客户端发送的HTTP请求
    MessageParser mp;
    struct timeval timeout = {3, 0};
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
    bool flag = true;
    while (flag)
    {
        request_size = recv(client_socket, buffer, BUFFER_SIZE, 0);

        if (request_size == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            else
            {
                perror("Receive failed");
                flag = false;
            }
        }
        else if (request_size < 0)
        {
            sprintf(buffer, "Unknow socket error. Error code is %d.", errno);
            perror(buffer);
            flag = false;
        }
        else if(request_size > 0)
        {
            buffer[request_size] = '\0';

            mp.pushMsg(buffer);
            if (mp.empty())
                continue;

            HttpBuilder h = mp.popHttp();

            if (h.getHeader("Connection") == "close")
                flag = false;

            // http1.1 中是半双工的，同个tcp连接不同报文不能并发，因此此处没有thread，http2进行tcp多路复用，才会用到thread
            // new thread(handle_request, client_socket, h);

            handle_request(client_socket, h);
        }
    }

    // 关闭客户端套接字
    close(client_socket);
}

void start_httpd(unsigned short port, string doc_root)
{
    typedef struct sockaddr *SP;

    cerr << "Starting server (port: " << port << ", doc_root: " << doc_root << ")" << endl;
    DOC_ROOT = doc_root;
    if (DOC_ROOT[DOC_ROOT.size() - 1] == '/')
        DOC_ROOT = DOC_ROOT.substr(0, DOC_ROOT.size() - 1);
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

    struct sockaddr_in client_address;

    int cnt = 0;
    thread th[THREAD_NUM];
    while (true)
    {
        // 等待连接
        int clifd;
        if (0 <= (clifd = accept(sockfd, (SP)&client_address, &addrlen)))
        {
            th[cnt++] = thread(handle_client, clifd);
            // handle_client(clifd);
            // pthread_t client_thread;
            // pthread_create(&client_thread, NULL, (void *(*)(void *))handle_client, (void *)clifd);
        }
        else
            perror("accept error");

        if (cnt == THREAD_NUM)
        {
            for (int i = 0; i < THREAD_NUM; i++)
                th[i].join();
            cnt = 0;
        }
    }
}
