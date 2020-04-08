/*
简易Tcp服务端 不支持Linux,只支持windows
*/

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>  //避免重定义 1.WinSocket放在windows之前 2. 加入宏定义#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#pragma comment(lib,"ws2_32.lib")

enum CMD  //消息枚举类型
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_ERROR
};
//利用结构体传输数据
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
		result = 1;
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
		printf("错误，创建socket失败\n");
	}
	else
	{
		printf("创建socket<%d>成功\n",_sock);
	}
	//2.绑定接受客户端连接的端口bind
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET; //协议
	if(argc==3)
	{
		_sin.sin_port= htons((unsigned short)argv[2]);//host to net 大小端  端口
		_sin.sin_addr.S_un.S_addr = inet_addr(argv[1]); //或者inet_addr("127.0.0.1");//ip地址
		if(SOCKET_ERROR == bind(_sock,(sockaddr *)&_sin,sizeof(_sin)))
		{
			printf("ip<%s>,端口=<%d>,绑定端口失败...\n",argv[1],argv[2]);
		}
		else
		{
			printf("ip<%s>,端口=<%d>绑定端口成功...\n",argv[1],argv[2]);
		}
	}
	else
	{
		_sin.sin_port= htons(4567);//host to net 大小端  端口
		_sin.sin_addr.S_un.S_addr = INADDR_ANY; //或者inet_addr("127.0.0.1");//ip地址
		if(SOCKET_ERROR == bind(_sock,(sockaddr *)&_sin,sizeof(_sin)))
		{
			printf("端口=<%d>,绑定端口失败...\n",4567);
		}
		else
		{
			printf("端口=<%d>绑定端口成功...\n",4567);
		}
	}


	//3.监听网络端口listen
	if(SOCKET_ERROR ==listen(_sock,10))
	{
		printf("socket<%d>监听失败...\n",_sock);
	}
	else
	{
		printf("socket<%d>监听成功...\n",_sock);
	}


	//4.等待接受客户端连接accept
	sockaddr_in clientAddr={}; //客户端地址
	int nAddrLen = sizeof(clientAddr);
	SOCKET _csock = INVALID_SOCKET;

	_csock = accept(_sock,(sockaddr *)&clientAddr,&nAddrLen);
	if (INVALID_SOCKET==_csock)
	{
		printf("错误，无效的客户端<%d>...\n",_csock);
	}
	printf("新客户端加入：sock =%d,ip=%s\n",_csock ,inet_ntoa(clientAddr.sin_addr));

	while(true)
	{

		//5.向客户端发送数据send
		//缓冲区
		char recvBuf[1024] = {};
		int nlen = recv(_csock,recvBuf,sizeof(DataHeader),0); //接收消息头
		DataHeader *header = (DataHeader *)recvBuf;
		if (nlen<=0)
		{
			printf("socket<%d>客户端已退出\n",_csock);
			break;
		}

		//发送数据  根据头部消息类型返回消息
		switch (header->cmd)
		{
		case CMD_LOGIN:
			{
				int retflag = recv(_csock,recvBuf+sizeof(DataHeader),header->dataLength-sizeof(DataHeader),0);
				Login *login = (Login *)recvBuf;
				if (retflag>0)
				{
					printf("收到客户端命令：CMD_LOGIN,socket<%d>,数据长度：%d,收到用户名和密码：%s：%s \n",_csock,login->dataLength,login->username,login->pwd);
				}
				//忽略判断正确
				LoginResult ret;
				send(_csock,(const char *)&ret,sizeof(LoginResult),0);
			}
			break;
		case CMD_LOGOUT:
			{

				int retflag = recv(_csock,recvBuf+sizeof(DataHeader),header->dataLength-sizeof(DataHeader),0);
				Logout * logout=(Logout *)recvBuf;
				if (retflag>0)
				{
					printf("收到客户端命令：CMD_LOGOUT,socket<%d>,数据长度：%d,收到用户名：%s\n",_csock,logout->dataLength,logout->username);
				}
				//忽略判断正确
				LogoutResult ret;
				send(_csock,(const char *)&ret,sizeof(LogoutResult),0);
			}
			break;
		default:
			{
				DataHeader header = {0,CMD_ERROR};
				send(_csock,(const char *)&header,sizeof(header),0);
			}
			break;
		}

	}
	//6.关闭socket closesocket
	closesocket(_csock);
	closesocket(_sock);
	printf("已退出服务器\n");
	getchar();
	WSACleanup();
	return 0;
}
