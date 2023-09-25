#include "HttpBuilder.h"
#include <iostream>
#include <sstream>
#include <string.h>

using namespace std;

const string CLIENT_ERROR_STR = "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\"><html><head><title>400 Client Error</title></head><body><h1>Client Error</h1><p>Your client has issued a malformed ot illegal request.</p></body></html>";
const string NOT_ALLOWED_STR = "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\"><html><head><title>405 Method Not Allowed</title></head><body><h1>Method Not Allowed</h1><p>The requested method %s is not allowed for the URL %s.</p></body></html>";
const string NOT_FOUND_STR = "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\"><html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL %s was not found on this server.</p></body></html>";
const string FORBIDDEN = "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\"><html><head><title>403 Forbidden</title></head><body><h1>Forbidden</h1><p>The requested from %s was Forbidden on this server.</p></body></html>";

std::string urlDecode(std::string &SRC)
{
    std::string ret;
    char ch;
    unsigned int ii;
    for (size_t i = 0; i < SRC.length(); i++) {
        if (int(SRC[i]) == 37) {
            sscanf(SRC.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            ret += ch;
            i = i + 2;
        } else {
            ret += SRC[i];
        }
    }
    return (ret);
}

HttpBuilder::HttpBuilder(const string meth_stat, const string url_code, const string ver, const bool isRequest) : meth(isRequest ? meth_stat : ""), ver(ver), code(isRequest ? "" : url_code), status(isRequest ? "" : meth_stat), isRequest(isRequest)
{
    if (isRequest) {
        if (url_code == "/") {
            this->url.addr = "/index.html";
            this->urlstr = "/index.html";
        } else {
            this->urlstr = url_code;
            const size_t pos = url_code.find('?');
            if (pos == string::npos)
                this->url.addr = url_code;
            else {
                this->url.addr = url_code.substr(0, pos);
                stringstream ss(url_code.substr(pos + 1));
                string line;
                while (getline(ss, line, '&')) {
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

const string HttpBuilder::toString(const HttpBuilder &h)
{
    return h.toString();
}

const string HttpBuilder::toString() const
{
    stringstream ss;
    if (this->isRequest)
        ss << this->meth << " " << this->urlstr << " " << this->ver << "\r\n";
    else
        ss << this->ver << " " << this->code << " " << this->status << "\r\n";

    for (map<string, string>::const_iterator i = this->header.begin(); i != this->header.end(); i++) {
        ss << i->first << ":" << i->second << "\r\n";
    }
    ss << "\r\n"
       << this->body;
    return ss.str();
}

const URL &HttpBuilder::getUrl() const
{
    return this->url;
}

const string &HttpBuilder::getUrlStr() const
{
    return this->urlstr;
}

const HttpBuilder HttpBuilder::str2req(const string &req)
{
    try {
        string method, url, ver;
        map<string, string> header;
        string body;

        stringstream ss(req);
        ss >> method >> url >> ver;
        url = urlDecode(url);
        if (ver.find("HTTP/") == string::npos)
            return HttpBuilder("", "", "", true);
        string line;
        getline(ss, line);

        if (!(line.size() == 0 || line[0] == '\r'))
            return HttpBuilder("", "", "", true);

        while (getline(ss, line)) {
            /* code */
            if (line.at(line.size() - 1) == '\r')
                line = line.substr(0, line.size() - 1);
            if (0 == line.length())
                break;
            size_t pos = line.find(':');

            if (pos == string::npos)
                return HttpBuilder("", "", "", true);

            header[line.substr(0, pos)] = line.substr(pos + 1);
        }
        ss >> body;
        return HttpBuilder(method, url, ver, true).setBody(body).setHeader(header);
    } catch (const std::exception &e) {
        cerr << e.what() << endl;
        return HttpBuilder("", "", "", true);
    }
}

const HttpBuilder HttpBuilder::str2resp(const string &resp)
{
    string code, status, ver;
    map<string, string> header;
    string body;

    stringstream ss(resp);
    ss >> ver >> code >> status;
    string line;
    getline(ss, line);
    while (getline(ss, line)) {
        /* code */
        if (line.at(line.size() - 1) == '\r')
            line = line.substr(0, line.size() - 1);
        if (0 == line.length())
            break;
        size_t pos = line.find(':');
        header[line.substr(0, pos)] = line.substr(pos + 1);
    }
    ss >> body;
    return HttpBuilder(status, code, ver, false).setBody(body).setHeader(header);
}

HttpBuilder HttpBuilder::getNotFound(const string &url)
{
    const string &str = NOT_FOUND_STR;
    char buffer[BUFFER_SIZE];
    snprintf(buffer, str.size() + url.size(), str.c_str(), url.c_str());

    return HttpBuilder("Not Found", "404", "HTTP/1.1", false)
        .setBody(buffer)
        .setHeader("Content-Type", "text/html;charset=utf-8")
        .setHeader("Server", "CentOS")
        .setHeader("Connection", "close");
}

HttpBuilder HttpBuilder::getNotAllowed(const string &meth, const string &url)
{
    const string &str = NOT_ALLOWED_STR;
    char buffer[BUFFER_SIZE];
    snprintf(buffer, str.size() + url.size() + meth.size(), str.c_str(), meth.c_str(), url.c_str());

    return HttpBuilder("Method Not Allowed", "405", "HTTP/1.1", false)
        .setBody(buffer)
        .setHeader("Content-Type", "text/html;charset=utf-8")
        .setHeader("Server", "CentOS")
        .setHeader("Connection", "close");
}

HttpBuilder HttpBuilder::getClientError()
{
    return HttpBuilder("Client Error", "400", "HTTP/1.1", false)
        .setBody(CLIENT_ERROR_STR)
        .setHeader("Content-Type", "text/html;charset=utf-8")
        .setHeader("Server", "CentOS")
        .setHeader("Connection", "close");
}

HttpBuilder HttpBuilder::getForbidden(const string &src)
{
    const string &str = FORBIDDEN;
    char buffer[BUFFER_SIZE];
    snprintf(buffer, str.size() + src.size(), str.c_str(), src.c_str());

    return HttpBuilder("Forbidden", "403", "HTTP/1.1", false)
        .setBody(buffer)
        .setHeader("Content-Type", "text/html;charset=utf-8")
        .setHeader("Server", "CentOS")
        .setHeader("Connection", "close");
}

HttpBuilder &HttpBuilder::setBody(const std::string &body)
{
    this->body = body;
    this->setHeader("Content-Length", body.size());
    return *this;
}

HttpBuilder &HttpBuilder::setHeader(const std::map<std::string, std::string> &header)
{
    this->header = header;
    return *this;
}

HttpBuilder &HttpBuilder::setHeader(const std::string key, const size_t &value)
{
    stringstream s;
    s << value;
    this->header[key] = s.str();
    return *this;
}

HttpBuilder &HttpBuilder::setHeader(const std::string key, const int &value)
{
    stringstream s;
    s << value;
    this->header[key] = s.str();
    return *this;
}

HttpBuilder &HttpBuilder::setHeader(const std::string key, const double &value)
{
    stringstream s;
    s << value;
    this->header[key] = s.str();
    return *this;
}

HttpBuilder &HttpBuilder::setHeader(std::string key, std::string value)
{
    this->header[key] = value;
    return *this;
}

const string HttpBuilder::getBody() const
{
    return this->body;
}

const string HttpBuilder::getHeader(const std::string key) const
{
    if (this->header.find(key) != this->header.end())
        return this->header.at(key);
    else
        return "";
}
