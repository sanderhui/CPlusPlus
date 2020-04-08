#ifndef _TcpClient_hpp_
#define _TcpClient_hpp_

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
#include "MessageHeader.hpp"



// 封装client类
class EasyTcpClient
{
private:
	SOCKET _sock;
public:
	EasyTcpClient()
	{
		_sock=INVALID_SOCKET;
	}
	//需将析构函数该成虚析构，养成习惯
	virtual ~EasyTcpClient()
	{
		Close();
	}
	//初始化socket
	void initSocket() //创建套接字
	{
		//启动winsocket2.x环境
#ifdef _WIN32
		WORD ver = MAKEWORD(2,2);  //创建版本号
		WSADATA dat;
		WSAStartup(ver,&dat);  //WinSock启动函数
#endif
		if(INVALID_SOCKET!=_sock)
		{
			printf("socket<%d>关闭旧连接\n",_sock);
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
	}
	//连接服务器
	int Connect(const char *ip,unsigned short port)
	{

		if(INVALID_SOCKET==_sock)
		{
			initSocket();
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		int ret =  connect(_sock,(sockaddr *)&_sin,sizeof(sockaddr_in));
		if(SOCKET_ERROR == ret)
		{
			printf("socket<%d><%s,%d>连接服务器失败\n",_sock,ip,port);
		}
		else
		{
			printf("socket<%d>连接到<%s,%d>服务器\n",_sock,ip,port);
		}
		return ret;
	}

	//判断是否运行
	bool isRun()
	{
		return INVALID_SOCKET !=_sock;
	}

	bool onRun()
	{
		if (isRun())
		{
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock,&fdReads);
			timeval t = {0,0};
			int ret = select(_sock+1,&fdReads,0,0,&t);
			if(ret <0)
			{
				printf("<socket=%d>select 任务结束1\n",_sock);
				Close();
				return false;
			}
			if(FD_ISSET(_sock,&fdReads))
			{
				FD_CLR(_sock,&fdReads);
				if(-1==RecvData(_sock))
				{
					printf("<socket=%d>select 任务结束2\n",_sock);
					Close();
					return false;
				}
			}
		}

		return true;
	}


	//接收数据 处理包问题，粘包，拆包
	int RecvData(SOCKET _sock)
	{
		char recvBuf[1024] = {};
		int nlen = recv(_sock,recvBuf,sizeof(DataHeader),0);

		if (nlen<=0)
		{
			printf("与服务端断开连接<socket=%d>\n",_sock);
			return -1;
		}
		DataHeader *header = (DataHeader *)recvBuf;
		recv(_sock,recvBuf+sizeof(DataHeader),header->dataLength-sizeof(DataHeader),0);
		onNetMsg(header);
		return 0;

	}
	//处理消息 根据传入参数不同构造不同消息
	virtual void onNetMsg(DataHeader *header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
			{
				LoginResult *login = (LoginResult *)header;
				printf("<socket=%d>收到服务器数据：CMD_LOGIN_RESULT  %d,数据长度：%d,结果：%d \n",_sock,header->cmd,login->dataLength,login->result);
			}
			break;
		case CMD_LOGOUT_RESULT:
			{
				LogoutResult * logout=(LogoutResult *)header;
				printf("<socket=%d>收到客户端命令：CMD_LOGOUT_RESULT ,数据长度：%d,结果：%d\n",_sock,header->dataLength,logout->result);

			}
			break;
		case CMD_NEW_USER_JOIN:
			{
				NewUserJoin * userjoin=(NewUserJoin *)header;
				printf("<socket=%d>收到客户端命令：CMD_NEW_USER_JOIN ,数据长度：%d,结果：%d\n",_sock,header->dataLength,userjoin->sock);
			}
			break;
		}
	}
	//发送数据 多态
	int sendData(DataHeader *header)
	{
		if(isRun() && header)
		{
			return send(_sock,(const char *)header,header->dataLength,0);
		}
		return SOCKET_ERROR;

	}
	//关闭服务器
	void Close()
	{
		if(_sock !=INVALID_SOCKET)
		{
			//关闭winsock2.x环境
#ifdef _WIN32
			closesocket(_sock);
			WSACleanup();  //WinSock清除函数
#else
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}
	}


};






#endif
