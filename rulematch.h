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
    /**
     * @brief 构造函数
     * @param allowFirst 是否allow优先，关联于Order Deny,Allow和Order Allow,Deny，
     * 具体规则见 https://httpd.apache.org/docs/2.2/mod/mod_authz_host.html#order
     */
    RuleList(bool allowFirst): allowFirst(allowFirst) {}

    /**
     * @brief 添加一条规则
     * @param rule 
     * @return 是否成功添加
     */
    bool addRule(const std::string &rule);

    /**
     * @brief 添加一条规则
     * @param rule 
     * @return 是否成功添加
     */
    bool addRule(const Rule &rule);

    /**
     * @brief 判断一个ip地址是否通过了规则
     * @param address ip地址
     * @param falseRes 如果没有通过规则，返回的错误信息
     * @return 如果通过了规则，返回true，否则返回false
     */
    bool pass(const struct sockaddr_in &address, std::string &falseRes) const;

    /**
     * @brief 通过文件获取一个RuleList对象指针，如果该文件中不含Order Deny,Allow或Order Allow,Deny，则默认为Order Allow,Deny
     * @param path 规则文件路径，通常为 .htaccess
     * @return 如果成功，返回RuleList对象指针，否则返回nullptr
     */
    static RuleList *getFromFile(const std::string &path);

private:
    const bool allowFirst;
    std::vector<Rule> hostAllowRules, ipAllowRules;
    std::vector<Rule> hostDenyRules, ipDenyRules;
    bool pass(const std::string &host) const;
};

#endif // RULE_MATCH_H