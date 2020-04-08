/*
select 服务端 可适用linux
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
#include <vector>
#include<stdlib.h>


//消息结构体 添加群发消息类型 CMD_NEW_USER_JOIN
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
		cmd = CMD_LOGOUT_RESULT;//以前版本这里写错了 提醒一下
		result = 1;
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

int processor(SOCKET _csock)
{
	char recvBuf[1024] = {};
	int nlen = recv(_csock,recvBuf,sizeof(DataHeader),0);
	DataHeader *header = (DataHeader *)recvBuf;
	if (nlen<=0)
	{
		printf("客户端<socket=%d>已退出\n",_csock);
		return -1;
	}

	//发送数据
	switch (header->cmd)
	{
	case CMD_LOGIN:
		{
			int retflag = recv(_csock,recvBuf+sizeof(DataHeader),header->dataLength-sizeof(DataHeader),0);
			Login *login = (Login *)recvBuf;
			if (retflag>0)
			{
				printf("socket<%d>收到客户端命令CMD_LOGIN,数据长度：%d,收到用户名和密码：%s：%s \n",_csock,login->dataLength,login->username,login->pwd);
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
				printf("socket<%d>收到客户端命令CMD_LOGOUT,数据长度：%d,收到用户名：%s\n",_csock,logout->dataLength,logout->username);
			}
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

std::vector<SOCKET> g_client_fds; //存储已连接的套接字

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
		_sin.sin_port = htons(atoi(argv[2]));
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(argv[1]);
#else
		_sin.sin_addr.s_addr = inet_addr(argv[1]);
#endif
		if(SOCKET_ERROR == bind(_sock,(sockaddr *)&_sin,sizeof(_sin)))
		{
			printf("ip<%s>,端口=<%s>,绑定端口失败...\n",argv[1],argv[2]);
		}
		else
		{
			printf("ip<%s>,端口=<%s>绑定端口成功...\n",argv[1],argv[2]);
		}
	}
	else
	{
		_sin.sin_port= htons(4567);//host to net 大小端  端口
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = INADDR_ANY;
#else
		_sin.sin_addr.s_addr = INADDR_ANY;
#endif
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
	while(true)
	{
		//5.向客户端发送数据send

		//伯克利 socket  1.windows没意义，nfds 是整数，指fd_set集合所有描述符的范围，不是数量，即最大值+1
		//2.readfds 集合 3.writefds 集合，4.exceptfds 异常集合，5.在指定时间内返回，0是阻塞
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
		SOCKET maxSock = _sock; //为了作select参数
		for(int n =(int)g_client_fds.size()-1;n>=0;n--) //为了适应linux 代码改了
		{
			FD_SET(g_client_fds[n],&fdRead);
			if(maxSock<g_client_fds[n])
			{
				maxSock = g_client_fds[n];
			}
		}
		timeval t = {0,0};
		int ret = select(maxSock + 1,&fdRead,&fdWrite,&fdExp,&t); //这个是阻塞的
		if (ret<0)
		{
			printf("select任务结束\n");
			break;
		}
		//让连接的客户端的描述符加入g_client_fds数列
		if(FD_ISSET(_sock,&fdRead))  //有没有可读的描述符
		{
			FD_CLR(_sock,&fdRead); //用于在文件描述符集合中删除一个文件描述符
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
				printf("错误，无效的客户端<%d>...\n",_csock);
			}
			else
			{
				// 群发新客户端加入
				for(int n =(int)g_client_fds.size()-1;n>=0;n--)  //向已连接的客户端发送数据
				{
					NewUserJoin newuser;
					newuser.sock = _csock;
					send(g_client_fds[n],(const char *)&newuser,sizeof(NewUserJoin),0);
				}
				g_client_fds.push_back(_csock);
				printf("新客户端加入：sock =%d,ip=%s\n",(int)_csock ,inet_ntoa(clientAddr.sin_addr));
			}
		}
		//对连接的客户端进行操作
		for(int n =(int)g_client_fds.size()-1;n>=0;n--)
		{
			if(FD_ISSET(g_client_fds[n],&fdRead))
				if (-1==processor(g_client_fds[n]))
				{
					auto iter = g_client_fds.begin() + n;
					//客户端退出，在存储描述符列表找到并且移除
					if(iter!=g_client_fds.end())
					{
						g_client_fds.erase(iter);
					}
				}
		}
		//printf("空闲处理其他任务\n");

	}
	//6.关闭socket closesocket
#ifdef _WIN32
	for(int n =g_client_fds.size()-1;n>=0;n--)
	{
		closesocket(g_client_fds[n]);//将所有socket关闭
	}
	closesocket(_sock);
	WSACleanup();  //WinSock清除函数
#else
	for(int n =g_client_fds.size()-1;n>=0;n--)
	{
		close(g_client_fds[n]);//将所有socket关闭
	}
	close(_sock);
#endif



	printf("已退出服务器\n");
	getchar();
	return 0;
}
