#ifndef MESSAGEPARSER_HPP
#define MESSAGEPARSER_HPP

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sstream>
#include <queue>
#include "HttpBuilder.h"
#include <iostream>
using namespace std;

class MessageParser
{
public:
    MessageParser(){}
    void pushMsg(const char* msg);
    HttpBuilder popHttp();
    bool empty() const;
private:
    string receivedMessage = "";
    queue<HttpBuilder> httpQueue;

    // 0表示文件头接收已完成，大于0表示需要接收body的大小
    size_t needSize = 0;

    // 判断是否已经接收完毕
    // bool ready = true;
};

#endif // MESSAGEPARSER_HPP