#include "rulematch.h"
#include <algorithm>
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>

using std::invalid_argument;
using std::string;
using std::stringstream;

Rule::Rule(bool isAllow, const std::string &item) : isAllow(isAllow)
{
    size_t pos;
    struct sockaddr_in sa;
    if ((pos = item.find('/')) != string::npos) {
        string s = item.substr(pos + 1);

        // turn mask string into int mask (4 bytes)
        // the mask may like 255.255.255.255 or 32
        if (s.find('.') != string::npos) {
            if (inet_pton(AF_INET, s.c_str(), &(sa.sin_addr)) == 1)
                this->mask = sa.sin_addr.s_addr;
            else
                throw invalid_argument("invalid rule: invalid mask: " + s);
        } else {
            try {
                int m = stoi(item.substr(pos + 1));
                this->mask = 0xffffffff >> (32 - m);
            } catch (invalid_argument &e) {
                throw invalid_argument("invalid rule: invalid mask: " + s);
            }
        }
    }

    // turn ip string into int address (4 bytes)
    if (inet_pton(AF_INET, item.substr(0, pos).c_str(), &(sa.sin_addr)) == 1)
        this->address = sa.sin_addr.s_addr;
    else
        this->host = item;
}

bool Rule::pass(const u_int32_t &addr) const
{
    if (host != "")
        return !isAllow;
    if ((address & mask) == (addr & mask))
        return isAllow;
    return !isAllow;
}

bool Rule::allowRule() const
{
    return isAllow;
}

bool Rule::denyRule() const
{
    return !allowRule();
}

bool Rule::isHostRule() const
{
    return this->host != "";
}

bool Rule::isIpRule() const
{
    return this->host == "";
}

bool Rule::pass(const std::string &host) const
{
    if (host == "")
        return !isAllow;
    if (this->host == host)
        return isAllow;
    return !isAllow;
}

bool RuleList::addRule(const std::string &rule)
{
    bool isAllow;
    stringstream ss(rule);
    // this format of rule is like "[allow/deny] from [ip/host] [ip/host] [ip/host]", we need to parse it.
    string part1, part2;
    ss >> part1 >> part2;
    if (part1 == "allow")
        isAllow = true;
    else if (part1 == "deny")
        isAllow = false;
    else {
        std::cerr << ("invalid rule: no 'allow' or 'deny'");
        return false;
    }

    if (part2 != "from") {
        std::cerr << ("invalid rule: no 'from'");
        return false;
    }

    while (ss >> part2)
        addRule(Rule(isAllow, part2));
    return true;
}

bool RuleList::addRule(const Rule &rule)
{
    if (rule.isHostRule()) {
        if (rule.allowRule())
            this->hostAllowRules.push_back(rule);
        else
            this->hostDenyRules.push_back(rule);
    } else {
        if (rule.allowRule())
            this->ipAllowRules.push_back(rule);
        else
            this->ipDenyRules.push_back(rule);
    }
    return true;
}

bool RuleList::pass(const std::string &host) const
{
    std::cout << "host vaild: " << host << std::endl;
    const std::vector<Rule> &first = this->allowFirst ? this->hostAllowRules : this->hostDenyRules;
    const std::vector<Rule> &last = this->allowFirst ? this->hostDenyRules : this->hostAllowRules;

    // allow优先时，默认全部deny。deny优先，默认全部allow
    bool flag = !this->allowFirst;

    for (auto &r : first) {
        // 分两种情况，allow优先则先执行allow，deny优先则先执行deny。
        // 当先执行allow时，一旦有一个规则将其allow，就可以返回allow优先的结果了（通过）。
        // 当先执行deny时，一旦有一个规则将其deny，就可以返回deny优先的结果了（拒绝）。
        if (r.pass(host) != flag) {
            flag = !flag;
            break;
        }
    }

    // allow优先时，默认全部deny。上述已经过allow规则检测，若没有allow host，则一定deny，因此可以直接返回结果，反之亦然
    if (flag == !this->allowFirst)
        return flag;

    for (auto &r : last) {
        if (r.pass(host) != flag) {
            flag = !flag;
            break;
        }
    }
    return flag;
}

bool RuleList::pass(const struct sockaddr_in &address, string &falseRes) const
{
    // 关于Order顺序可见 https://httpd.apache.org/docs/2.2/mod/mod_authz_host.html#order
    // 通过反DNS获取address的域名，然后再进行host规则匹配

    // Perform reverse DNS lookup
    falseRes = "(No forbidden, if you see this, something is wrong)";
    uint32_t addrInt = address.sin_addr.s_addr;
    char host[NI_MAXHOST];
    int res = getnameinfo((struct sockaddr *)&address, sizeof(address), host, NI_MAXHOST, NULL, 0, 0);
    if (res == 0) {
        // 事实上还需要进行一次dns获取ip，验证两个ip是否相同，但是这里不做了
        struct sockaddr_in tmp;
        int res = inet_aton(host, &tmp.sin_addr);
        // 是ip则返回非0
        if (res == 0 && !this->pass(host)) {
            falseRes = host;
            return false;
        }
    } else if (res != EAI_NONAME)
        std::cerr << "getnameinfo error: " << gai_strerror(res) << std::endl;
    else
        std::cout << "No domain: " << std::endl;

    const std::vector<Rule> &first = this->allowFirst ? this->ipAllowRules : this->ipDenyRules;
    const std::vector<Rule> &last = this->allowFirst ? this->ipDenyRules : this->ipAllowRules;

    // allow优先时，默认全部deny。deny优先，默认全部allow
    bool flag = !this->allowFirst;

    for (auto &r : first) {
        // 分两种情况，allow优先则先执行allow，deny优先则先执行deny。
        // 当先执行allow时，一旦有一个规则将其allow，就可以返回allow优先的结果了（通过）。
        // 当先执行deny时，一旦有一个规则将其deny，就可以返回deny优先的结果了（拒绝）。
        if (r.pass(addrInt) != flag) {
            flag = !flag;
            break;
        }
    }

    // allow优先时，默认全部deny。上述已经过allow规则检测，若没有allow host，则一定deny，因此可以直接返回结果，反之亦然
    if (flag == this->allowFirst) {
        for (auto &r : last) {
            if (r.pass(addrInt) != flag) {
                flag = !flag;
                break;
            }
        }
    }

    if (flag == false) {
        falseRes = inet_ntoa(address.sin_addr);
    }
    return flag;
}

RuleList *RuleList::getFromFile(const std::string &path)
{
    std::ifstream file(path, std::ios::in);
    if (!file.is_open()) {
        std::cerr << "open file '" << path << "' failed" << std::endl;
        return nullptr;
    }
    string l;
    getline(file, l);
    stringstream ss(l);
    // this format of first line is like "Order Allow,Deny", we need to parse it.
    string part1, part2;
    ss >> part1 >> part2;

    RuleList *ruleList;

    if (part1 == "allow" || part1 == "deny") {
        // 默认allow优先
        ruleList = new RuleList(true);
        ruleList->addRule(l);
    } else {
        if (part1 != "Order" || (part2 != "Allow,Deny" && part2 != "Deny,Allow")) {
            std::cerr << "invalid rule file: first line is not 'Order Allow,Deny' or 'Order Deny,Allow'" << std::endl;
            return nullptr;
        }
        ruleList = new RuleList((part2 == "Allow,Deny"));
    }
    while (getline(file, l)) {
        ruleList->addRule(l);
    }
    return ruleList;
}
