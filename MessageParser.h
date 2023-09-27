#ifndef MESSAGEPARSER_HPP
#define MESSAGEPARSER_HPP

#include "HttpBuilder.h"
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <queue>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class MessageParser
{
public:
    MessageParser() {}
    void pushMsg(const std::string &msg);
    HttpBuilder popHttp();
    bool empty() const;

private:
    std::string receivedMessage = "";
    std::queue<HttpBuilder> httpQueue;

    // 0表示文件头接收已完成，大于0表示需要接收body的大小
    size_t needSize = 0;

    // 判断是否已经接收完毕
    // bool ready = true;
};

#endif // MESSAGEPARSER_HPP