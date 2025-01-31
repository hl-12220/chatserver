/*
muduo网络库给用户提供了两个主要类
TcpServer：服务器程序
TcpClient: 客户端程序
epoll+线程池
将网络I/O和业务区分开
用户连接和断开 用户可读写时间
*/
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include<string>
#include <functional>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;
/*基于muduo网络库开发服务器程序
1.组合TcpServer对象
2.创建EventLoop事件循环对象的指针
3.明确TcpServer构造函数需要什么参数，输出ChatServer构造函数
4.在当前服务器类的构造函数当中，注册处理连接的回调函数和处理读写事件的回调函数
5.设置合适的服务端线程数量，muduo库自己会分配
*/
class ChatServer
{
public:
    ChatServer(EventLoop *loop,               // 事件循环
               const InetAddress &listenAddr, // IP + Port
               const string &nameArg)         // 服务器名字
        : _server(loop, listenAddr, nameArg), _loop(loop)
    {
        // 给服务器注册用户连接的创建和断开回调
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
        // 给服务器注册用户读写事件回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));
        // 设置服务器端的线程数量: 1个I/O 3个Worke
        _server.setThreadNum(4);
    }
    // 开启时间循环
    void start()
    {
        _server.start();
    }

private:
    // 专门处理用户的连接创建和断开 epoll listenfd accept
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state:online " << endl;
        }
        else
        {
            cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state:offline " << endl;
            conn->shutdown();
            //_loop->quit();
        }
    }
    // 专门处理用户的读写事件
    void onMessage(const TcpConnectionPtr &conn, // 连接
                   Buffer *buffer,                  // 缓冲区
                   Timestamp time)               // 接收数据的时间信息
    {
        string buf = buffer->retrieveAllAsString();
        cout << " recv data: " << buf << " time " << time.toString() << endl;
        conn->send(buf);
    }
    TcpServer _server;
    EventLoop *_loop;
};
int main() {
    EventLoop loop;
    InetAddress addr("192.168.235.128", 6000);
    ChatServer server(&loop, addr, "ChatServer");
    server.start(); //listenfd epoll_ctl=>epoll
    loop.loop();  //epoll_wait 以阻塞方式等待新用户连接，已连接用户的读写事件等
    return 0;
}