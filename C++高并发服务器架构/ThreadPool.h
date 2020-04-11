/*
Free Classes Library For Microsoft Visual C++

Copyright @2017 CDU Innovation Studio
All rights reserved.

�ļ����ƣ�ThreadPool.h
ժҪ��
1.��ͷ�ļ������̳߳����������ʵ����ThreadPool.cpp�С�
2.���������Ϊ��ʱ�������߳̽�������״̬������pushTask���������������������ʱ�����������Ĺ����̣߳�Ϊ�����̷߳�������
3.�������̱߳�Ϊ��ʱ�������߳̽�������״̬��������̱߳�����ӹ����߳�ʱ�����������Ĺ����̣߳�Ϊ�¼���Ĺ����̷߳�������
4.�̳߳�������а����������Ӻͻص������ӣ�ͬʱ���ݸ������̣߳������߳�ִ����������֮����ûص������ӡ�

��ǰ�汾��V1.3
���ߣ����
Mailbox: 2592419242@qq.com
�������ڣ�2017��09��22��
�������ڣ�2019��03��08��

������־��
V1.1
1.�߳����ÿ����̱߳��������߳���������getTask��ȡ����ʱ�����������Ϊ�գ���Ѷ�Ӧ�Ĺ����̼߳�������̱߳��ɹ����̷߳�������ʱ�����������̱߳����ѿ����߳�
2.�������������̷߳������̳߳ع����̸߳�������������������������ı䵱ǰ�����߳�����
	���������������ʵ����ӻ��߼��ٹ����̣߳����ӹ����߳�������ʵ�ָ߲������ͷſ��й����̣߳������Դ������
V1.2
1.���ӿ��й����̼߳�����ɾ�������̱߳�
2.�޸������̳߳�ʱ��������
V1.3
1.���ƶ������Ż���ȡ���񣬼��ٲ���Ҫ�ĸ��Ʋ���
2.ȥ����������Ķ����жϲ���
3.ʹ��˫����������У����ٶ�д����֮���Ӱ�죬��߷�������ͻ�ȡ�����Ч��
*/

#pragma once

#include <list>
#include <memory>
#include <functional>

//#define DEFAULT_TIME_SLICE 2

class Thread;
struct ThreadPoolStructure;
using TaskPair = std::pair<std::function<void()>, std::function<void()>>;

class ThreadPool
{
	friend class Thread;
	std::unique_ptr<ThreadPoolStructure> threadPoolData;
	inline void setCloseStatus(bool bClosed);
	inline bool getCloseStatus() const;
public:
	ThreadPool(unsigned nThread = 0, unsigned maxThreads = getMaxConcurrency() * 100);
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool operator= (const ThreadPool&) = delete;
	~ThreadPool();
	static unsigned getMaxConcurrency();
	bool setCurrentThreads(unsigned nThread);
	unsigned getCurrentThreads() const;
	unsigned getTasks() const;
	void setMaxThreads(unsigned maxThreads);
	unsigned getMaxThreads() const;
	//bool setTimeSlice(unsigned timeSlice);
	//unsigned getTimeSlice() const;
	void pushTask(std::function<void()> run, std::function<void()> callback);
	void pushTask(TaskPair &&task);
	void pushTask(std::list<TaskPair> &tasks);
	void destroy();
protected:
	bool getTask(std::shared_ptr<Thread> thread);
	void execute();
};
