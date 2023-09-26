# CSE124-project1 By zsl

## Completed

1. 基础的http1.1协议

    - 接收请求中的initial line，header，和body，其中换行符为'\r\n'，但根据协议要求，也可正确接收'\n'换行
    - 正确检测请求header，根据协议要求，拒绝不含host的请求
    - 对URL进行正确解码，具体为将%开头的字符转义为utf-8
    - URL中的参数正确接收，即?后面的key和value

2. 拓展一：Http长连接，复用TCP
    
    - 同一个网页所有请求，使用同一个tcp，顺序接收
    - 正确切割每个报文，先接收header，获取body的长度，按长度接收body，再接收下一个报文
    - 正确设置和识别header中的Connection、keep-alive、close标头
    - **Tips:** http1.1中，同个tcp连接中多个报文是顺序接收的，如果一个请求未完成了，则后续请求会被阻塞（但不同tcp连接是非阻塞的）。
    - **Tips 2:** tcp多路复用，是http2的内容，其为每个frame增加了一个Stream ID，识别不同顺序的报文。如果不进行特殊处理，试图在同个tcp内多线程进行报文接收，会导致请求乱序，客户端接收到的respond也是乱序的。见https://cloud.tencent.com/developer/article/1573513

3. 拓展二：简易防火墙“.htaccess"规则表

    - 根据Apache的规则，正确识别首行Order，未设置则默认"Allow,Deny"
    - 根据Apache的规则，正确判断是否允许通过
    - 正确识别规则，样式为 allow from [ip/mask] [ip/mask] [ip/mask] [host]，注意同一行内可能有多条[ip/mask]或[host]
    - 每条ip通过反DNS获取host，加入判断（注意，根据Apache的规则，需要双向检验，即再进行一次正向DNS，判断两个ip是否相等，若不等，则拒绝访问，此处并没有实现双向检验）
    - Apache的规则见https://httpd.apache.org/docs/2.2/mod/mod_authz_host.html

4. 拓展三：线程池管理不同tcp连接

    - 实现一个线程池管理线程，参考来源：https://github.com/lzpong/

## Incomplete
1. 拓展四：事件驱动
