/*
Free Classes Library For Microsoft Visual C++

Copyright @2017 CDU Innovation Studio
All rights reserved.

�ļ����ƣ�Thread.h
ժҪ��
1.��ͷ�ļ����й����߳����������ʵ����Thread.cpp�С�
2.Thread���ṩ�߳����÷������������߳̿��Ա��ظ�ʹ�á�
3.�߳���ִ�е����񣬼ȿ������̴߳���ʱָ����Ҳ�����ڴ����߳�֮�󣬵���configure����Ϊ�̷߳�������
4.�����̺߳�������������״̬������start�����������̡߳�
5.�̵߳��õĺ�������configure��������������������Ϊ�գ����ÿպ�����run��run�������麯�����ɱ���д�����粻Ϊ�գ����ô���ĺ����ӡ�
6.�߳���ִ����������֮����ûص������ӣ������ڵ��ú����ӵĹ��̲����쳣����֤�߳��������У��������߳�й©��

��ǰ�汾��V1.2
���ߣ����
Mailbox: 2592419242@qq.com
�������ڣ�2017��09��22��
�������ڣ�2019��03��08��

������־��
V1.1
1.�����߳�ִ�е�ǰ����֮���Զ���ȡ�̳߳��������֮�е���������ȡ����ʧ�ܣ���������״̬���ȴ������̷߳������񣬷���ִ�л�ȡ��������
	�����̳߳ر��������̱߳�Ϊ�����̷߳�������ʱ������ʱ�����ģ���߹����̵߳�Ч��
V1.2
1.�򻯹��캯����ȡ������ӿڣ��Ӷ���������
2.���ƶ������Ż��������񣬼��ٲ���Ҫ�ĸ��Ʋ���
*/

#pragma once

#include <thread>
#include <memory>
#include <functional>

class ThreadPool;
struct ThreadStructure;
using TaskPair = std::pair<std::function<void()>, std::function<void()>>;

/* �̳�enable_shared_from_thisģ���࣬��Thread��shared_ptr�йܣ�
��Thread���а�ָ������������ָ��this��Ϊ����������������ʱ��
��Ҫ����ָ�������shared_ptr������shared_from_this������ȡָ�������shared_ptr��
������ֱ�Ӵ���ԭʼָ��this�������ܱ�֤shared_ptr�����壬Ҳ��ᵼ���ѱ��ͷŵĴ���
Ҳ�������ٴ�����һshared_ptr�������йܵĶ��shared_ptr�Ŀ��ƿ鲻ͬ������ͬһ�����ͷŶ�Ρ� */
class Thread :public std::enable_shared_from_this<Thread>
{
	std::shared_ptr<ThreadStructure> threadData;
	inline void setCloseStatus(bool bClosed);
	inline bool getCloseStatus() const;
	inline void setRunStatus(bool bRunning);
	inline bool getRunStatus() const;
	inline void setInterveneStatus(bool bIntervening);
	inline bool getInterveneStatus() const;
public:
	Thread(ThreadPool *threadPool);
	Thread(const Thread&) = delete;
	Thread operator= (const Thread&) = delete;
	~Thread();
	bool configure(TaskPair &&task);
	bool start();
	void destroy();
	std::thread::id getThreadId() const;
	//const void *getParameters();
	bool isFree() const;
protected:
	void execute();
	virtual void run();
	virtual void callback();
};
