#include "Thread.h"
#include "ThreadPool.h"

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <exception>
#include <iostream>

// �߳����ݽṹ��
struct ThreadStructure
{
	std::thread thread;					// �߳�
	ThreadPool *threadPool;				// �̳߳�ָ��
	std::mutex threadMutex;				// �̻߳���Ԫ
	std::condition_variable signal;		// ��������
	TaskPair task;						// ��������
	std::atomic_bool bClosed;			// �ر�״̬��־
	std::atomic_bool bRunning;			// ����״̬��־
	std::atomic_bool bIntervening;		// ��������־
	//void *vpParameters;
	// �̹߳��̲�����������������ָ�룬������̳�����࣬ͨ��ǿ������ת�����ڹ��̺����з���
};

// �̹߳��캯��
Thread::Thread(ThreadPool *threadPool)
{
	/* shared_ptr��Ҫά�����ü����������ù��캯������ͨ��new���ʽ��������֮�󴫵ݸ�shared_ptr����
	һ�������ڴ����룬����������ڴ棬��������ƿ��ڴ棬�����ڴ�Ϳ��ƿ��ڴ治������
	��ʹ��make_shared����ֻ����һ���ڴ棬�����ڴ�Ϳ��ƿ��ڴ���һ�� */
	threadData = std::make_shared<ThreadStructure>();
	threadData->threadPool = threadPool;	// ָ���̳߳أ������Զ����û�ȡ�̳߳���������е�����
	setCloseStatus(false);	// �����߳�δ�ر�
	setRunStatus(false);	// �����߳�δ����
	setInterveneStatus(true);	// ��������̷߳�������
	threadData->thread = std::thread(&Thread::execute, this);	// ����thread�������е���this�����execute����
}

// �߳���������
Thread::~Thread()
{
	destroy();
}

// �����̹߳ر�״̬
inline void Thread::setCloseStatus(bool bClosed)
{
	threadData->bClosed = bClosed;
}

// ��ȡ�̹߳ر�״̬
inline bool Thread::getCloseStatus() const
{
	return threadData->bClosed;
}

// �����߳�����״̬
inline void Thread::setRunStatus(bool bRunning)
{
	threadData->bRunning = bRunning;
}

// ��ȡ�߳�����״̬
inline bool Thread::getRunStatus() const
{
	return threadData->bRunning;
}

// �����߳̽���״̬
inline void Thread::setInterveneStatus(bool bIntervening)
{
	threadData->bIntervening = bIntervening;
}

// ��ȡ�߳̽���״̬
inline bool Thread::getInterveneStatus() const
{
	return threadData->bIntervening;
}

// Ϊ�����߳�����������
bool Thread::configure(TaskPair &&task)
{
	// ���̴߳�������״̬��˵������ִ����������������ʧ��
	if (getRunStatus())
		return false;
	threadData->task = std::move(task);	// ���蹤���߳���������
	//threadData->vpParameters = vpParameters;
	return true;
}

// ���ѹ����߳�
bool Thread::start()
{
	if (getRunStatus())	// �������̴߳�������״̬����־���Ѿ���������
		return false;
	threadData->signal.notify_one();	// ͨ���������������źţ����ѹ����߳�
	return true;
}

// ���ٹ����߳�
void Thread::destroy()
{
	// �������߳��Ѿ����٣��������²���
	if (getCloseStatus())
		return;
	setCloseStatus(true);	// ���ù����߳�Ϊ�ر�״̬��������״̬
	// �����߳̿��ܴ�������״̬��ͨ���������������źŻ��ѹ����߳�
	threadData->signal.notify_one();
	// �ȴ�δִ����ɵĹ����߳̽���
	if (threadData->thread.joinable())
		threadData->thread.join();
	// Ԥ���ȴ�ʱ��
	//std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

// ��ȡ�����߳�id
std::thread::id Thread::getThreadId() const
{
	return threadData->thread.get_id();
}

//const void *Thread::getParameters()
//{
//	return threadData->vpParameters;
//}

// �����߳��Ƿ��ڿ���״̬
bool Thread::isFree() const
{
	return getInterveneStatus();
}

// �����߳���
void Thread::execute()
{
	// ����mem_fun��ȡ��������������mem_fun1_t���͵Ķ���getTask���˶����б���ָ��ThreadPool::getTask��ָ�룬�������������operator()
	auto &&getTask = std::mem_fun(&ThreadPool::getTask);
	std::unique_lock<std::mutex> threadLocker(threadData->threadMutex);	// ����unique_lock�����������ڹ��캯���л�ȡ������
	threadData->signal.wait(threadLocker);	// ��ȡ����������֮ǰ�Ѿ���ȡ���������������̣߳��ȴ����������Ļ����ź�
	while (!getCloseStatus())	// �����߳����˳�ͨ��
	{
		setRunStatus(true);	// �����߳�Ϊ����״̬
		try
		{
			if (threadData->task.first)	// ���������Ӳ�Ϊ��
				threadData->task.first();	// ִ����������
			else
				run();	// ִ��Ĭ��������
			if (threadData->task.second)	// ���ص������Ӳ�Ϊ��
				threadData->task.second();	// ִ�лص�������
			else
				callback();	// ִ��Ĭ�ϻص�����
		}
		catch (std::exception exception)	// ����ִ����������ʱ���ֵ��쳣����ֹ�߳�й©
		{
			std::cerr << exception.what() << std::endl;
		}
		setRunStatus(false);
		/* ��threadData->threadPool->getTask(this->shared_from_this())����ʽ����ThreadPool::getTask������ȡ�̳߳���������е�����
		��δ�ɹ���ȡ���������̣߳��ȴ����������Ļ����ź� */
		if (!(threadData->threadPool
			&& getTask(threadData->threadPool, this->shared_from_this())))
		{
			if (getCloseStatus())	// �����߳��˳�ͨ��
				break;
			setInterveneStatus(true);	// ������������̷߳�������
			threadData->signal.wait(threadLocker);
			setInterveneStatus(false);
		}
	}
}

// �߳�Ĭ�������������ڼ̳���չ�̣߳��γɹ���
void Thread::run()
{
}

// �߳�Ĭ�ϻص����������ڼ̳���չ�̣߳��γɹ���
void Thread::callback()
{
}
