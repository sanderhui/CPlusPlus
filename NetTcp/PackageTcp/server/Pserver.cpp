#include "TcpServer.hpp"

int main(int argc, char* argv[])
{
	EasyTcpServer server;
	server.initSocket();
	server.Bind(nullptr,4567);
	server.Listen(5);

	while(server.isRun())
	{
		server.onRun();
	}
	server.Close();
	printf("已退出服务器\n");
	getchar();
	return 0;
}
