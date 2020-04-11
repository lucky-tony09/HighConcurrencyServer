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
			CELLLog::Info("�˳�cmdThread�߳�\n");
			break;
		}
		else {
			CELLLog::Info("��֧�ֵ����\n");
		}
	}
}

class MyServer : public EasyTcpServer
{
public:

	//ֻ�ᱻһ���̴߳��� ��ȫ
	virtual void OnNetJoin(ClientSocket* pClient)
	{
		_clientCount++;
		//CELLLog::Info("client<%d> join\n", pClient->sockfd());
	}
	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetLeave(ClientSocket* pClient)
	{
		_clientCount--;
		//CELLLog::Info("client<%d> leave\n", pClient->sockfd());
	}
	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetMsg(ClientSocket* pClient, netmsg_DataHeader* header)
	{
		_msgCount++;
		//Ϊ�˼��ģ�⽫��½��Ϣ�Ϳ���������Ϣ
		pClient->resetDTHeart();
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			netmsg_Login* login = (netmsg_Login*)header;
			pClient->resetDTHeart();
			//CELLLog::Info("�յ��ͻ���<Socket=%d>����CMD_LOGIN,���ݳ��ȣ�%d,userName=%s PassWord=%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			//LoginResult ret;
			//pClient->SendData(&ret);
		}
		break;
		case CMD_LOGOUT:
		{
			netmsg_Logout* logout = (netmsg_Logout*)header;
			//CELLLog::Info("�յ��ͻ���<Socket=%d>����CMD_LOGOUT,���ݳ��ȣ�%d,userName=%s \n", cSock, logout->dataLength, logout->userName);
			//�����ж��û������Ƿ���ȷ�Ĺ���
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
			CELLLog::Info("<socket=%d>�յ�δ������Ϣ,���ݳ��ȣ�%d\n", pClient->sockfd(), header->dataLength);
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

	//����UI�߳�
	std::thread t1(cmdThread);
	t1.detach();

	while (g_bRun)
	{
		server.OnRun();
		//CELLLog::Info("����ʱ�䴦������ҵ��..\n");
	}
	server.Close();
	CELLLog::Info("���˳���\n");
	getchar();
	return 0;
}