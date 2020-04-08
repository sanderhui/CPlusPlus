#ifndef _TcpServer_hpp_
#define _TcpServer_hpp_

#ifdef _WIN32    //判断是哪个系统
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>  //避免重定义 1.WinSocket放在windows之前 2. 加入宏定义#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <unistd.h> //uni std unix 标准头文件
#include <arpa/inet.h>  //linxu 网络编程库
#include <string.h>  //字符串操作
#define SOCKET int  //linxu 是int类型，windows也是类似只不过换成了SOCKET宏，这里换回来
#define INVALID_SOCKET  (SOCKET)(~0)  //将宏换回来
#define SOCKET_ERROR            (-1)
#endif
#include <stdio.h>
#include<stdlib.h>
#include <vector>
#include "MessageHeader.hpp"


class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<SOCKET> g_client_fds; //存储已连接的套接字
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}
	//初始化
	SOCKET initSocket()
	{
#ifdef _WIN32
		WORD ver = MAKEWORD(2,2);  //创建版本号
		WSADATA dat;
		WSAStartup(ver,&dat);  //WinSock启动函数
#endif

		if(INVALID_SOCKET!=_sock)
		{
			printf("<sock=%d>关闭旧连接\n",_sock);
			Close();
		}
		_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

		if(INVALID_SOCKET==_sock)
		{
			printf("错误，创建socket失败\n");
		}
		else
		{
			printf("创建socket<%d>成功\n",_sock);
		}
		return _sock;
	}
	//绑定
	int Bind(const char *ip, unsigned short port)
	{
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET; //协议
		_sin.sin_port= htons(port);//host to net 大小端  端口

#ifdef _WIN32
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}

#else
		if (ip)
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.s_addr = INADDR_ANY;
		}

#endif
		int ret = bind(_sock,(sockaddr *)&_sin,sizeof(_sin));
		if(SOCKET_ERROR == ret )
		{
			printf("绑定ip<%s>端口<%d>失败...\n",ip,port);
		}
		else
		{
			printf("绑定ip<%s>端口<%d>成功...\n",ip,port);
		}
		return ret;
	}
	//监听
	int Listen(int n)
	{
		//3.监听网络端口listen
		int ret = listen(_sock,n);
		if(SOCKET_ERROR ==ret)
		{
			printf("socket<%d>监听失败...\n",_sock);
		}
		else
		{
			printf("socket<%d>监听成功...\n",_sock);
		}
		return ret;
	}
	//接收客户端连接
	SOCKET Accept()
	{
		sockaddr_in clientAddr={}; //客户端地址
#ifdef _WIN32
		int nAddrLen = sizeof(clientAddr);
#else
		unsigned int nAddrLen = sizeof(clientAddr);
#endif

		SOCKET _csock = INVALID_SOCKET;
		_csock = accept(_sock,(sockaddr *)&clientAddr,&nAddrLen);
		if (INVALID_SOCKET==_csock)
		{
			printf("错误，无效的客户端socket<%d>...\n",_csock);
		}
		else
		{
			NewUserJoin newuser;
			newuser.sock = _csock;
			sendDataAll(&newuser);
			g_client_fds.push_back(_csock);
			printf("新客户端加入：sock =%d,ip=%s\n",(int)_csock ,inet_ntoa(clientAddr.sin_addr));
		}
		return _csock;
	}

	//判断是否运行
	bool isRun()
	{
		return INVALID_SOCKET !=_sock;
	}
	//接收数据
	bool onRun()
	{
		if(isRun())
		{
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExp;
			FD_ZERO(&fdRead); //清空
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);
			FD_SET(_sock,&fdRead); //把描述符放入集合
			FD_SET(_sock,&fdWrite);
			FD_SET(_sock,&fdExp);
			//将已连接的描述符放入监听集合
			SOCKET maxSock = _sock;
			for(int n =(int)g_client_fds.size()-1;n>=0;n--)
			{
				FD_SET(g_client_fds[n],&fdRead);
				if(maxSock<g_client_fds[n])
				{
					maxSock = g_client_fds[n];
				}
			}
			timeval t = {0,0};
			int ret = select(_sock+1,&fdRead,&fdWrite,&fdExp,&t); //这个是阻塞的
			if (ret<0)
			{
				printf("select任务结束\n");
				Close();
				return false;
			}
			//让连接的客户端的描述符加入g_client_fds数列
			if(FD_ISSET(_sock,&fdRead))  //有没有可读的描述符
			{
				FD_CLR(_sock,&fdRead); //用于在文件描述符集合中删除一个文件描述符
				Accept();

			}
			//对连接的客户端进行操作
			for(int n =(int)g_client_fds.size()-1;n>=0;n--)  //为了linux修改
			{
				if(FD_ISSET(g_client_fds[n],&fdRead))
					if (-1==RecvData(g_client_fds[n]))
					{
						auto iter = g_client_fds.begin()+n;
						//客户端退出，在存储描述符列表找到并且移除
						if(iter!=g_client_fds.end())
						{
							g_client_fds.erase(iter);
						}
					}
			}
			return true;
		}
		return false;
	}

	//响应数据
	int RecvData(SOCKET _csock)
	{
		char recvBuf[1024] = {};
		int nlen = recv(_csock,recvBuf,sizeof(DataHeader),0);
		DataHeader *header = (DataHeader *)recvBuf;
		if (nlen<=0)
		{
			printf("客户端<socket=%d>已退出\n",_csock);
			return -1;
		}

		recv(_csock,recvBuf+sizeof(DataHeader),header->dataLength-sizeof(DataHeader),0);
		onNetMsg(_csock,header);
	}
	//响应消息
	virtual void onNetMsg(SOCKET _csock,DataHeader *header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
			{

				Login *login = (Login *)header;
				printf("socket<%d>收到客户端命令CMD_LOGIN,数据长度：%d,收到用户名和密码：%s：%s \n",_csock,login->dataLength,login->username,login->pwd);
				//忽略判断正确
				LoginResult ret;
				send(_csock,(const char *)&ret,sizeof(LoginResult),0);
			}
			break;
		case CMD_LOGOUT:
			{
				Logout * logout=(Logout *)header;
				printf("socket<%d>收到客户端命令CMD_LOGOUT,数据长度：%d,收到用户名：%s\n",_csock,logout->dataLength,logout->username);
				//忽略判断正确
				LogoutResult ret;
				send(_csock,(const char *)&ret,sizeof(LogoutResult),0);
			}
			break;
		default:
			{
				DataHeader header = {0,CMD_ERROR};
				send(_csock,(const char *)&header,sizeof(DataHeader),0);
			}
			break;
		}
	}
	//发送指定数据
	int sendData(SOCKET _csock,DataHeader *header)
	{
		if(isRun() && header)
		{
			return send(_csock,(const char *)header,header->dataLength,0);
		}
		return SOCKET_ERROR;

	}
	//群发数据
	void sendDataAll(DataHeader *header)
	{
		if(isRun())
		{
			for(int n =(int)g_client_fds.size()-1;n>=0;n--)  //向已连接的客户端发送数据
			{
				sendData(g_client_fds[n],header);
			}
		}

	};
	//关闭
	void Close()
	{
		if(_sock !=INVALID_SOCKET)
		{
#ifdef _WIN32
			for(int n =g_client_fds.size()-1;n>=0;n--)
			{
				closesocket(g_client_fds[n]);//将所有socket关闭
			}
			closesocket(_sock);
			WSACleanup();
#else
			for(int n =g_client_fds.size()-1;n>=0;n--)
			{
				close(g_client_fds[n]);//将所有socket关闭
			}
			close(_sock);
#endif
		}
	}
};
#endif
