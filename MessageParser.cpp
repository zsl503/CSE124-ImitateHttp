#include "MessageParser.h"
#include <iostream>
using std::string;

void MessageParser::pushMsg(const string& msg)
{
    if (msg.size() == 0)
        return;

    receivedMessage.append(msg);

    if (this->needSize == 0) {
        size_t pos;
        if ((pos = receivedMessage.find("\r\n\r\n")) != string::npos) {
            httpQueue.push(HttpBuilder::str2req(receivedMessage.substr(0, pos)));
            // move pos to the end of the request header
            receivedMessage = receivedMessage.substr(pos + 4);

            string need = httpQueue.back().getHeader("Content-Length");
            if (need == "")
                this->needSize = 0;
            else
                this->needSize = atoi(need.c_str());
        } else if ((pos = receivedMessage.find("\n\n")) != string::npos) {
            httpQueue.push(HttpBuilder::str2req(receivedMessage.substr(0, pos)));
            receivedMessage = receivedMessage.substr(pos + 2);

            string need = httpQueue.back().getHeader("Content-Length");
            if (need == "")
                this->needSize = 0;
            else
                this->needSize = atoi(need.c_str());
        }
    }

    if (this->needSize > 0 && receivedMessage.size() >= this->needSize) {
        string r = receivedMessage.substr(0, this->needSize);
        httpQueue.back().setBody(r);
        receivedMessage = receivedMessage.substr(this->needSize);
        this->needSize = 0;
    }
}

HttpBuilder MessageParser::popHttp()
{
    // TODO: insert return statement here
    if (empty()) {
        std::cerr << "queue is empty" << std::endl;
        throw std::exception();
    } else {
        HttpBuilder tmp = httpQueue.front();
        httpQueue.pop();
        return tmp;
    }
}

bool MessageParser::empty() const
{
    return httpQueue.empty() || (httpQueue.size() == 1 && needSize > 0);
}