#ifndef RULE_MATCH_H
#define RULE_MATCH_H

#include <string>
#include <vector>

class Rule
{
public:
    Rule(bool isAllow, const std::string &item);
    bool pass(const std::string &host) const;
    bool pass(const u_int32_t &addr) const;
    bool isHostRule() const;
    bool isIpRule() const;
    bool allowRule() const;
    bool denyRule() const;
private:
    const bool isAllow;
    std::string host;
    u_int32_t address;
    u_int32_t mask;
};

class RuleList
{
public:
    RuleList(bool allowFirst): allowFirst(allowFirst) {}
    bool addRule(const std::string &rule);
    bool addRule(const Rule &rule);

    bool pass(const struct sockaddr_in &address, std::string &falseRes) const;

    static RuleList *getFromFile(const std::string &path);

private:
    const bool allowFirst;
    std::vector<Rule> hostAllowRules, ipAllowRules;
    std::vector<Rule> hostDenyRules, ipDenyRules;
    bool pass(const std::string &host) const;
};

#endif // RULE_MATCH_H