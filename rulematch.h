#ifndef RULE_LIST_H
#define RULE_LIST_H

#include <string>
#include <vector>

class Rule
{
public:
    Rule(const std::string &rule);
    bool pass(const std::string &url);

private:
    std::string rule;
    std::string host;
    u_int32_t address;
    u_int32_t mask;
    bool isDeny;
};

class RuleList
{
public:
    RuleList(const std::string &list);
    bool addRule(const std::string &rule);
    bool addRule(const Rule &rule);

    bool pass(const std::string &host);

    bool pass(const u_int32_t &address);

    static RuleList *getFromFile(const std::string &path);

private:
    bool allowFirst;
    std::vector<Rule> hostRules, ipRules;
};

#endif // RULE_LIST_H