#include "httpd.h"
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

const string CLIENT_ERROR_STR = "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\"><html><head><title>400 Client Error</title></head><body><h1>Client Error</h1><p>Your client has issued a malformed ot illegal request.</p></body></html>";
const string NOT_ALLOWED_STR = "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\"><html><head><title>405 Method Not Allowed</title></head><body><h1>Method Not Allowed</h1><p>The requested method %s is not allowed for the URL %s.</p></body></html>";
const string NOT_FOUND_STR = "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\"><html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL %s was not found on this server.</p></body></html>";

std::string urlDecode(std::string &SRC)
{
    std::string ret;
    char ch;
    unsigned int ii;
    for (size_t i = 0; i < SRC.length(); i++)
    {
        if (int(SRC[i]) == 37)
        {
            sscanf(SRC.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            ret += ch;
            i = i + 2;
        }
        else
        {
            ret += SRC[i];
        }
    }
    return (ret);
}

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

Http *handle_file(const string filename)
{
    std::ifstream in((DOC_ROOT + filename), std::ios::in);
    if (!in.is_open())
        return new Http(Http::getNotFound(filename));
    else
    {
        std::istreambuf_iterator<char> beg(in), end;
        string str(beg, end);
        in.close();
        Http *h = new Http("OK", "200", "HTTP/1.1", std::map<string, string>(), str, false);
        h->setContentLength(str.size());
        h->setServer("CentOS");

        const size_t pos = filename.rfind(".");
        const string back = filename.substr(pos + 1);
        if (CONTENT_TYPE_MAP.find(back) != CONTENT_TYPE_MAP.end())
            h->setContentType(CONTENT_TYPE_MAP[back]);
        else
            h->setContentType("application/octet-stream");

        struct stat result;
        if (stat((DOC_ROOT + filename).c_str(), &result) == 0)
        {
            time_t mod_time = result.st_mtime;
            time(&mod_time); /*获取time_t类型的当前时间*/
            char *t = asctime(gmtime(&mod_time));
            t[strlen(t) - 1] = '\0';
            h->setLastModified(t);
        }
        else
            return new Http(Http::getNotFound(filename));
        return h;
    }
}

void handle_request(){
    
}

void handle_client(int client_socket)
{
    char buffer[BUFFER_SIZE + 1];
    ssize_t request_size;
    // 读取客户端发送的HTTP请求
    while (true)
    {
        request_size = recv(client_socket, buffer, BUFFER_SIZE, 0);

        if (request_size == -1)
        {
            perror("Receive failed");
            close(client_socket);
            return;
        }

        buffer[request_size] = '\0';

        // 处理HTTP请求

        Http h = Http::str2req(buffer);
        Http *resp;
        // 处理GET请求
        if (h.meth == "")
            resp = new Http(Http::getClientError());
        else if (h.meth == "GET")
        {

            if (!vaildAddr(h.getUrl().addr))
                resp = new Http(Http::getNotFound(h.getUrlStr()));
            else if (h.header.find("Host") == h.header.end())
            {
                resp = new Http(Http::getClientError());
            }
            else
                try
                {
                    resp = handle_file(h.getUrl().addr);
                }
                catch (const std::exception &e)
                {
                    cerr << "catch:" << e.what();
                    resp = new Http(Http::getNotFound(h.getUrlStr()));
                }
        }
        else
        {
            resp = new Http(Http::getNotAllowed(h.meth, h.getUrlStr()));
        }
        sendAll(client_socket, resp->toString());
        delete resp;
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

Http::Http(const string meth_stat, const string url_code, const string ver, map<string, string> header, const string body, const bool isRequest) : meth(isRequest ? meth_stat : ""), ver(ver), header(header), body(body), code(isRequest ? "" : url_code), status(isRequest ? "" : meth_stat), isRequest(isRequest)
{
    if (isRequest)
    {
        if (url_code == "/")
        {
            this->url.addr = "/index.html";
            this->urlstr = "/index.html";
        }
        else
        {
            this->urlstr = url_code;
            const size_t pos = url_code.find('?');
            if (pos == string::npos)
                this->url.addr = url_code;
            else
            {
                this->url.addr = url_code.substr(0, pos);
                stringstream ss(url_code.substr(pos + 1));
                string line;
                while (getline(ss, line, '&'))
                {
                    const size_t p = line.find('=');
                    if (p == string::npos)
                        continue;
                    else
                        this->url.param[line.substr(0, p)] = line.substr(p + 1);
                }
            }
        }
    }
}

const string Http::toString(const Http &h)
{
    return h.toString();
}

const string Http::toString() const
{
    stringstream ss;
    if (this->isRequest)
        ss << this->meth << " " << this->urlstr << " " << this->ver << "\r\n";
    else
        ss << this->ver << " " << this->code << " " << this->status << "\r\n";

    for (map<string, string>::const_iterator i = this->header.begin(); i != this->header.end(); i++)
    {
        ss << i->first << ":" << i->second << "\r\n";
    }
    ss << "\r\n"
       << this->body;
    return ss.str();
}

const URL &Http::getUrl() const
{
    return this->url;
}

const string &Http::getUrlStr() const
{
    return this->urlstr;
}

const Http Http::str2req(const string &req)
{
    try
    {
        string method, url, ver;
        map<string, string> header;
        string body;

        stringstream ss(req);
        ss >> method >> url >> ver;
        url = urlDecode(url);
        if (ver.find("HTTP/") == string::npos)
            return Http("", "", "", {}, "", true);
        string line;
        getline(ss, line);

        if (!(line.size() == 0 || line[0] == '\r'))
            return Http("", "", "", {}, "", true);

        while (getline(ss, line))
        {
            /* code */
            if (line.at(line.size() - 1) == '\r')
                line = line.substr(0, line.size() - 1);
            if (0 == line.length())
                break;
            size_t pos = line.find(':');

            if (pos == string::npos)
                return Http("", "", "", {}, "", true);

            header[line.substr(0, pos)] = line.substr(pos + 1);
        }
        ss >> body;
        return Http(method, url, ver, header, body, true);
    }
    catch (const std::exception &e)
    {
        cerr << e.what() << endl;
        return Http("", "", "", {}, "", true);
    }
}

const Http Http::str2resp(const string &resp)
{
    string code, status, ver;
    map<string, string> header;
    string body;

    stringstream ss(resp);
    ss >> ver >> code >> status;
    string line;
    getline(ss, line);
    while (getline(ss, line))
    {
        /* code */
        if (line.at(line.size() - 1) == '\r')
            line = line.substr(0, line.size() - 1);
        if (0 == line.length())
            break;
        size_t pos = line.find(':');
        header[line.substr(0, pos)] = line.substr(pos + 1);
    }
    ss >> body;
    return Http(status, code, ver, header, body, false);
}

const Http Http::getNotFound(const string &url)
{
    const string &str = NOT_FOUND_STR;
    char buffer[BUFFER_SIZE];
    snprintf(buffer, str.size() + url.size(), str.c_str(), url.c_str());

    Http h = Http("Not Found", "404", "HTTP/1.1", {}, buffer, false);
    h.setContentLength(strlen(buffer));
    h.setContentType("text/html;charset=utf-8");
    h.setServer("CentOS");
    return h;
}

const Http Http::getNotAllowed(const string &meth, const string &url)
{
    const string &str = NOT_ALLOWED_STR;
    char buffer[BUFFER_SIZE];
    snprintf(buffer, str.size() + url.size() + meth.size(), str.c_str(), meth.c_str(), url.c_str());

    Http h = Http("Method Not Allowed", "405", "HTTP/1.1", {}, buffer, false);
    h.setContentLength(strlen(buffer));
    h.setContentType("text/html;charset=utf-8");
    h.setServer("CentOS");
    return h;
}

const Http Http::getClientError()
{
    Http h = Http("Client Error", "400", "HTTP/1.1", {}, CLIENT_ERROR_STR, false);
    h.setContentLength(CLIENT_ERROR_STR.size());
    h.setContentType("text/html;charset=utf-8");
    h.setServer("CentOS");
    return h;
}

void Http::setContentType(const string &type)
{
    this->header["Content-Type"] = type;
}

void Http::setContentLength(const unsigned long &len)
{
    stringstream s;
    s << len;
    this->header["Content-Length"] = s.str();
}

void Http::setServer(const string &server)
{
    this->header["Server"] = server;
}

void Http::setLastModified(const string &time)
{
    this->header["Last-Modified"] = time;
}