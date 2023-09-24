#ifndef HTTPD_H
#define HTTPD_H

#include <map>

#define BUFFER_SIZE 1024*100

typedef struct URL{
    std::string addr;
    std::map<std::string, std::string> param;
}URL;

class Http
{
    public:
    const std::string meth;
    const std::string ver;
    std::map<std::string,std::string> header;
    const std::string body;
    const std::string code;
    const std::string status;
    const bool isRequest;

    Http(const std::string meth_stat, const std::string url_code, const std::string ver, std::map<std::string,std::string> header,const std::string body, const bool isRequest);
    // Http(std::string req);
    const URL &getUrl() const;
    const std::string &getUrlStr() const;
    void setContentType(const std::string&);
    void setContentLength(const unsigned long &len);
    void setServer(const std::string&);
    void setLastModified(const std::string&);
    
    const std::string toString() const;
    static const std::string toString(const Http&);

    static const Http str2resp(const std::string& resp);
    static const Http str2req(const std::string&);

    static const Http getNotFound(const std::string &url);
    static const Http getNotAllowed(const std::string &meth, const std::string &url);
    static const Http getClientError();

    protected:
    URL url;
    std::string urlstr;
};

/**
 * @brief 验证http报文中url请求资源的合法性，主要为是否利用".."请求到上级目录
 * @param addr url中的资源定位符部分，不含?后面内容
 * @return 验证成功与否
 */
bool vaildAddr(const std::string &addr);

/**
 * @brief 处理请求，接收客户端的http报文，解析并回复
 * @param req 请求的串
 * @param client_socket socket文件描述符 
 */
void start_httpd(unsigned short port, std::string doc_root);

/**
 * @brief 处理客户端socket连接请求，接收并回复来自客户端的http报文
 * @param client_socket socket文件描述符
 */
void handle_client(int client_socket);

/**
 * @brief 请求文件，并返回文件报文
 * @param filename 文件名，为url中的资源定位符部分，不含?后面内容
 * @return 请求的文件报文
 */
Http *handle_file(const std::string filename);

/**
 * @brief 发送全部http报文。由于send可能不能一次性发送全部信息，通过循环保证发送完整
 * @param sockfd socket文件描述符
 * @param msg 全部http报文 
 */
void sendAll(int sockfd, const std::string& msg);

/**
 * @brief 解析URL
 * @param SRC 含%的原始串
 * @return 解码后的串
 */
std::string urlDecode(std::string &SRC);

#endif // HTTPD_H