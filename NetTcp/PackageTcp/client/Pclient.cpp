#include "TcpClient.hpp"
#include <thread>


//线程函数 发送命令给服务器
void cmdThread(EasyTcpClient  *client)
{
	while(true)
	{
		printf("请输入命令:\n");
		char cmdBuf[1024] = {};
		scanf("%s",cmdBuf);
		if(0 == strcmp(cmdBuf,"exit"))
		{
			client->Close();
			printf("退出cmdThread线程\n");
			break;
		}
		else if(0==strcmp(cmdBuf,"login"))
		{
			Login login;
			strcpy(login.username,"xiaoxiao");
			strcpy(login.pwd,"123456");
			client->sendData(&login);
		}
		else if(0==strcmp(cmdBuf,"logout"))
		{
			Logout logout;
			strcpy(logout.username,"xiaoxiao");
			client->sendData(&logout);
		}
		else
		{
			printf("不支持的命令，请重新输入\n");
		}
	}
}
int main(int argc, char* argv[])
{
	EasyTcpClient client; //创建客户端
	client.initSocket(); //初始化
	if(argc==3)
	{
		client.Connect(argv[1],atoi(argv[2])); //连接
	}
	else
	{
		client.Connect("127.0.0.1",4567); //连接
	}


	std::thread t1(cmdThread,&client); //创建线程
	t1.detach();

	while(client.isRun())  //
	{
		client.onRun();
	}
	client.Close();  //退出

	printf("已退出客户端\n");
	getchar();
	return 0;
}
