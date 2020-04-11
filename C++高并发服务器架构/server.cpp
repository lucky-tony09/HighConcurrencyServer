#include "EasyTcpServer.hpp"
#include<thread>

bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			CELLLog::Info("退出cmdThread线程\n");
			break;
		}
		else {
			CELLLog::Info("不支持的命令。\n");
		}
	}
}

class MyServer : public EasyTcpServer
{
public:

	//只会被一个线程触发 安全
	virtual void OnNetJoin(ClientSocket* pClient)
	{
		_clientCount++;
		//CELLLog::Info("client<%d> join\n", pClient->sockfd());
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetLeave(ClientSocket* pClient)
	{
		_clientCount--;
		//CELLLog::Info("client<%d> leave\n", pClient->sockfd());
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetMsg(ClientSocket* pClient, netmsg_DataHeader* header)
	{
		_msgCount++;
		//为了简介模拟将登陆消息就看做心跳信息
		pClient->resetDTHeart();
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			netmsg_Login* login = (netmsg_Login*)header;
			pClient->resetDTHeart();
			//CELLLog::Info("收到客户端<Socket=%d>请求：CMD_LOGIN,数据长度：%d,userName=%s PassWord=%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//忽略判断用户密码是否正确的过程
			//LoginResult ret;
			//pClient->SendData(&ret);
		}
		break;
		case CMD_LOGOUT:
		{
			netmsg_Logout* logout = (netmsg_Logout*)header;
			//CELLLog::Info("收到客户端<Socket=%d>请求：CMD_LOGOUT,数据长度：%d,userName=%s \n", cSock, logout->dataLength, logout->userName);
			//忽略判断用户密码是否正确的过程
			//LogoutResult ret;
			//SendData(cSock, &ret);
		}
		break;
		case CMD_C2S_HEART:
		{
			pClient->resetDTHeart();
			netmsg_s2c_Heart ret;
			pClient->SendData(&ret);
		}
		break;
		default:
		{
			CELLLog::Info("<socket=%d>收到未定义消息,数据长度：%d\n", pClient->sockfd(), header->dataLength);
			//netmsg_DataHeader ret;
			//SendData(cSock, &ret);
		}
		break;
		}
	}

	virtual void OnNetRecv(ClientSocket* pClient)
	{
		_recvCount++;
		//CELLLog::Info("client<%d> leave\n", pClient->sockfd());
	}
private:

};

int main()
{
	CELLLog::Instance().setLogPath("serverLog.txt", "w");
	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 122);
	server.Listen(5);
	server.Start(4);

	//启动UI线程
	std::thread t1(cmdThread);
	t1.detach();

	while (g_bRun)
	{
		server.OnRun();
		//CELLLog::Info("空闲时间处理其它业务..\n");
	}
	server.Close();
	CELLLog::Info("已退出。\n");
	getchar();
	return 0;
}