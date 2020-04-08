/*
select 客户端 可适用linux
*/
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
#include <thread>
#include<stdlib.h>

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
struct  DataHeader
{
	short dataLength;//数据 长度
	short cmd;
};
struct Login:public DataHeader
{
	Login()
	{
		dataLength=sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char username[32];
	char  pwd[32];
};  //利用结构体传输数据
struct LoginResult:public DataHeader
{
	LoginResult()
	{
		dataLength=sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;

};
struct Logout:public DataHeader
{
	Logout()
	{
		dataLength=sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char username[32];

};
struct LogoutResult:public DataHeader
{
	LogoutResult()
	{
		dataLength=sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result=1;
	}
	int result;

};
struct NewUserJoin:public DataHeader
{
	NewUserJoin()
	{
		dataLength=sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;

};

bool g_bRun = true;
//处理数据函数
int processor(SOCKET _sock)
{
	char recvBuf[1024] = {};
	int nlen = recv(_sock,recvBuf,sizeof(DataHeader),0);
	DataHeader *header = (DataHeader *)recvBuf;
	if (nlen<=0)
	{
		printf("与服务端断开连接<socket=%d>\n",_sock);
		return -1;
	}

	//接收数据
	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
		{
			recv(_sock,recvBuf+sizeof(DataHeader),header->dataLength-sizeof(DataHeader),0);
			LoginResult *login = (LoginResult *)recvBuf;
			printf("收到服务器数据：CMD_LOGIN_RESULT  %d,数据长度：%d,结果：%d \n",header->cmd,login->dataLength,login->result);
		}
		break;
	case CMD_LOGOUT_RESULT:
		{
			recv(_sock,recvBuf+sizeof(DataHeader),header->dataLength-sizeof(DataHeader),0);
			LogoutResult * logout=(LogoutResult *)recvBuf;
			printf("收到服务器数据：CMD_LOGOUT_RESULT ,数据长度：%d,结果：%d\n",header->dataLength,logout->result);

		}
		break;
	case CMD_NEW_USER_JOIN:
		{
			recv(_sock,recvBuf+sizeof(DataHeader),header->dataLength-sizeof(DataHeader),0);
			NewUserJoin * userjoin=(NewUserJoin *)recvBuf;
			printf("收到服务器数据：CMD_NEW_USER_JOIN ,数据长度：%d,结果：%d\n",header->dataLength,userjoin->sock);
		}
		break;
	}
}

void cmdThread(SOCKET _sock)
{
	while(true)
	{
		printf("请输入命令:\n");
		char cmdBuf[1024] = {};
		scanf("%s",cmdBuf);
		if(0 == strcmp(cmdBuf,"exit"))
		{
			g_bRun = false;
			printf("退出cmdThread线程\n");
			break;
		}
		else if(0==strcmp(cmdBuf,"login"))
		{
			Login login;
			strcpy(login.username,"xiaoxiao");
			strcpy(login.pwd,"123456");
			send(_sock,(const char *)&login,sizeof(login),0);
		}
		else if(0==strcmp(cmdBuf,"logout"))
		{
			Logout logout;
			strcpy(logout.username,"xiaoxiao");
			send(_sock,(const char *)&logout,sizeof(logout),0);
		}
		else
		{
			printf("不支持的命令，请重新输入\n");
		}
	}
}


int main(int argc, char* argv[])
{
#ifdef _WIN32
	WORD ver = MAKEWORD(2,2);  //创建版本号
	WSADATA dat;
	WSAStartup(ver,&dat);  //WinSock启动函数
#endif
	//1.建立socket
	SOCKET _sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(INVALID_SOCKET==_sock)
	{
		printf("错误，创建套接字失败\n");
	}
	else
	{
		printf("创建socket<%d>成功\n",_sock);
	}
	//2.连接服务器connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	if(argc ==3)
	{
		_sin.sin_port = htons(atoi(argv[2]));
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(argv[1]);
#else
		_sin.sin_addr.s_addr = inet_addr(argv[1]);
#endif
		int ret =  connect(_sock,(sockaddr *)&_sin,sizeof(sockaddr_in));
		if(SOCKET_ERROR == ret)
		{
			printf("连接服务器ip<%s:%s>失败\n",argv[1],argv[2]);
		}
		else
		{
			printf("连接到服务器ip<%s:%s>\n",argv[1],argv[2]);
		}
	}
	else
	{
		_sin.sin_port = htons(4567);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
#else
		_sin.sin_addr.s_addr = inet_addr("192.168.139.1");
#endif
		int ret =  connect(_sock,(sockaddr *)&_sin,sizeof(sockaddr_in));
		if(SOCKET_ERROR == ret)
		{

			printf("连接服务器失败\n");
		}
		else
		{
			printf("连接到服务器\n");
		}
	}



	//3.接收服务器信息recv
	//启动线程
	std::thread t1(cmdThread,_sock);
	t1.detach();
	while(g_bRun)
	{
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock,&fdReads);
		timeval t = {0,0};
		int ret = select(_sock+1,&fdReads,0,0,&t);  //之前这里漏写加一，windows没影响，现在修改
		if(ret <0)
		{
			printf("select 任务结束1\n");
			break;
		}
		if(FD_ISSET(_sock,&fdReads))
		{
			FD_CLR(_sock,&fdReads);

			if(-1==processor(_sock))
			{
				printf("select任务结束2\n");
				break;
			}
		}
		//线程
		//Sleep(1000);
	}

	//4.关闭socket closesocket

#ifdef _WIN32
	closesocket(_sock);
	WSACleanup();  //WinSock清除函数
#else
	close(_sock);
#endif
	printf("已退出\n");
	getchar();
	return 0;
}
