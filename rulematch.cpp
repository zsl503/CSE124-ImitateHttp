#include "rulematch.h"
#include <sstream>
#include <stdexcept>
#include <algotirhm>
#include <sys/socket.h>

using namespace std;

Rule::Rule(const std::string &rule)
{
    this->rule = rule;
    stringstream ss(rule);
    // this format of rule is like "[allow/deny] from [ip/host]", we need to parse it.
    string part1, part2, part3;
    ss >> part1 >> part2 >> part3;
    if (part1 == "allow")
        this->isDeny = false;
    else if (part1 == "deny")
        this->isDeny = true;
    else
        throw invalid_argument("invalid rule: no 'allow' or 'deny'");

    if (part2 != "from")
        throw invalid_argument("invalid rule: no 'from'");
    
    int num = count(part3.begin(),part3.end(),'.'), pos;
    if (num == 3 && (pos = part3.find('/')) != string::npos){
        // turn ip into ip and mask
        int m = stoi(part3.substr(pos+1));
        this->mask = 0xffffffff << (32-m);

        // turn ip string into int address (4 bytes)
        struct sockaddr_in sa;
        inet_pton(AF_INET, part3.substr(0,pos), &(sa.sin_addr));
        this->address = sa.sin_addr.s_addr;
    }
    else
        this->host = part3;
}