#ifndef _MessageHeader_hpp_
#define _MessageHeader_hpp_
//消息结构体
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


#endif
