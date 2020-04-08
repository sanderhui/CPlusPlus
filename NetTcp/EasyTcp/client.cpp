/*
简易Tcp客户端 不支持Linux,只支持windows
*/
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>  //避免重定义 1.WinSocket放在windows之前 2. 加入宏定义#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#pragma comment(lib,"ws2_32.lib")

//利用结构体传输数据
enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
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
};
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
		cmd = CMD_LOGIN_RESULT;
		result=1;
	}
	int result;

};

int main(int argc, char* argv[])
{
	WORD ver = MAKEWORD(2,2);  //创建版本号
	WSADATA dat;
	WSAStartup(ver,&dat);  //WinSock启动函数

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
	if(argc==3)
	{
		_sin.sin_port= htons((unsigned short)argv[2]);//host to net 大小端  端口
		_sin.sin_addr.S_un.S_addr = inet_addr(argv[1]); //或者inet_addr("127.0.0.1");//ip地址
		int ret =  connect(_sock,(sockaddr *)&_sin,sizeof(sockaddr_in));
		if(SOCKET_ERROR == ret)
		{
			printf("连接服务器ip<%s:%d>失败\n",argv[1],argv[2]);
		}
		else
		{
			printf("连接到服务器ip<%s:%d>\n",argv[1],argv[2]);
		}
	}
	else
	{
		_sin.sin_port = htons(4567);
		_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

		int ret =  connect(_sock,(sockaddr *)&_sin,sizeof(sockaddr_in));
		if(SOCKET_ERROR == ret)
		{
			printf("连接服务器ip<127.0.0.1:4567>失败\n");
		}
		else
		{
			printf("连接到服务器ip<127.0.0.1:4567>\n");
		}
	}


	//3.接收服务器信息recv

	while(true)
	{
		printf("请输入命令:\n");
		char cmdBuf[1024] = {};
		scanf("%s",cmdBuf);
		if(0 == strcmp(cmdBuf,"exit"))
		{
			break;
		}
		else if(0==strcmp(cmdBuf,"login"))
		{
			Login login;
			strcpy(login.username,"xiaoxiao");
			strcpy(login.pwd,"123456");
			send(_sock,(const char *)&login,sizeof(login),0);
			//接收服务器返回数据

			LoginResult ret;
			recv(_sock,(char *)&ret,sizeof(ret),0);
			printf("收到服务器数据：%d\n",ret.result);

		}
		else if(0==strcmp(cmdBuf,"logout"))
		{
			Logout logout;
			strcpy(logout.username,"xiaoxiao");
			send(_sock,(const char *)&logout,sizeof(logout),0);
			//接收服务器返回数据
			LoginResult ret;
			recv(_sock,(char *)&ret,sizeof(ret),0);
			printf("收到服务器数据：%d\n",ret.result);
		}
		else
		{
			printf("不支持的命令，请重新输入\n");
		}
	}

	//4.关闭socket closesocket
	closesocket(_sock);
	WSACleanup();  //WinSock清除函数
	printf("已退出\n");
	getchar();
	return 0;
}
