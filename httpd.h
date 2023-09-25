#ifndef HTTPD_H
#define HTTPD_H

#include "HttpBuilder.h"
/**
 * @brief 验证http报文中url请求资源的合法性，主要为是否利用".."请求到上级目录
 * @param addr url中的资源定位符部分，不含?后面内容
 * @return 验证成功与否
 */
bool vaildUrl(const std::string &url);

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
void handle_client(int client_socket, struct sockaddr_in client_address);

/**
 * @brief 请求文件，并返回文件报文
 * @param filename 文件名，为url中的资源定位符部分，不含?后面内容
 * @return 请求的文件报文
 */
HttpBuilder handle_file(const std::string filename);

/**
 * @brief 发送全部http报文。由于send可能不能一次性发送全部信息，通过循环保证发送完整
 * @param sockfd socket文件描述符
 * @param msg 全部http报文 
 */
void sendAll(int sockfd, const std::string& msg);


#endif // HTTPD_H