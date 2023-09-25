#ifndef HTTP_BUILDER_H
#define HTTP_BUILDER_H

#include <map>
#include <string>
// #include "HttpBuilder.cpp"

#define BUFFER_SIZE 65536

typedef struct URL {
    std::string addr;
    std::map<std::string, std::string> param;
} URL;

class HttpBuilder
{
public:
    std::string meth;
    std::string ver;
    std::string code;
    std::string status;
    bool isRequest;

    HttpBuilder() : meth(""), ver(""), code(""), status(""), isRequest(true) {}
    HttpBuilder(const std::string meth_stat, const std::string url_code, const std::string ver, const bool isRequest);
    const URL &getUrl() const;
    const std::string &getUrlStr() const;

    HttpBuilder &setBody(const std::string &);
    HttpBuilder &setHeader(const std::map<std::string, std::string> &header);
    HttpBuilder &setHeader(const std::string key, const std::string value);
    HttpBuilder &setHeader(const std::string key, const size_t &value);
    HttpBuilder &setHeader(const std::string key, const int &value);
    HttpBuilder &setHeader(const std::string key, const double &value);

    const std::string getBody() const;
    const std::string getHeader(const std::string key) const;
    const std::string toString() const;
    static const std::string toString(const HttpBuilder &);

    static const HttpBuilder str2resp(const std::string &resp);
    static const HttpBuilder str2req(const std::string &);

    static HttpBuilder getNotFound(const std::string &url);
    static HttpBuilder getNotAllowed(const std::string &meth, const std::string &url);
    static HttpBuilder getClientError();
    static HttpBuilder getForbidden(const std::string &src);

protected:
    URL url;
    std::string body;
    std::string urlstr;
    std::map<std::string, std::string> header;
};

/**
 * @brief 解析URL
 * @param SRC 含%的原始串
 * @return 解码后的串
 */
std::string urlDecode(std::string &SRC);

#endif // HTTP_BUILDER_H
