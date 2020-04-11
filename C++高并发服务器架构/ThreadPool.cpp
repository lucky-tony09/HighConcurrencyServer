#include "Thread.h"
#include "ThreadPool.h"
#include "Queue.h"

#include <vector>
#include <thread>
#include <atomic>
#include <condition_variable>

// �̳߳����ݽṹ��
struct ThreadPoolStructure
{
	std::vector<std::shared_ptr<Thread>> vThreads;		// �����̱߳�
	Queue<TaskPair> tasks;				// �������
	std::thread thread;					// �߳�
	std::mutex threadMutex;				// �̻߳���Ԫ
	std::condition_variable signal;		// ��������
	std::atomic_bool bClosed;			// �رձ��
	std::atomic_uint maxThreads;		// ����߳���
	std::atomic_uint freeThreads;		// �����߳���
	//std::atomic_int timeSlice;
};

// �̳߳ع��캯��
ThreadPool::ThreadPool(unsigned nThread, unsigned maxThreads)
{
	threadPoolData = std::make_unique<ThreadPoolStructure>();
	setCloseStatus(false);	// �����߳�δ�ر�
	//setTimeSlice(timeSlice);
	setMaxThreads(maxThreads);	// ��������߳�����
	if (nThread > getMaxThreads())	// ��֤�����̲߳���������߳���
		nThread = getMaxThreads();
	threadPoolData->vThreads.reserve(nThread);	// Ԥ�����ڴ�ռ䣬���ǲ���ʼ���ڴ棬��δ���ù��캯��
	/* shared_ptr��Ҫά�����ü����������ù��캯������ͨ��new���ʽ��������֮�󴫵ݸ�shared_ptr����
	һ�������ڴ����룬����������ڴ棬��������ƿ��ڴ棬�����ڴ�Ϳ��ƿ��ڴ治������
	��ʹ��make_shared����ֻ����һ���ڴ棬�����ڴ�Ϳ��ƿ��ڴ���һ�� */
	for (unsigned i = 0; i < nThread; ++i)
		threadPoolData->vThreads.push_back(std::make_shared<Thread>(this));
	threadPoolData->freeThreads = threadPoolData->vThreads.size();	// ���ÿ����߳�����
	threadPoolData->thread = std::thread(&ThreadPool::execute, this);	// ����thread�������е���this�����execute����
}

// �̳߳���������
ThreadPool::~ThreadPool()
{
	destroy();
}

// �����̳߳عر�״̬�����ڸտ�ʼ�����̳߳�״̬�͹ر��̳߳�
inline void ThreadPool::setCloseStatus(bool bClosed)
{
	threadPoolData->bClosed = bClosed;
}

// ��ȡ�̳߳صĹر�״̬
inline bool ThreadPool::getCloseStatus() const
{
	return threadPoolData->bClosed;
}

// ��ȡӲ���豸�������е�����߳�����
unsigned ThreadPool::getMaxConcurrency()
{
	return std::thread::hardware_concurrency();
}

// �����̳߳ع����߳�����
bool ThreadPool::setCurrentThreads(unsigned nThread)
{
	if (nThread > getMaxThreads())
		return false;
	auto result = nThread - threadPoolData->vThreads.size();
	if (result > 0)	// ���ӹ����߳�
	{
		std::lock_guard<std::mutex> threadLocker(threadPoolData->threadMutex);	// ����
		threadPoolData->vThreads.reserve(nThread);	// �������̱߳�����
													// �����̱߳�����ӹ����߳�
		for (unsigned i = 0; i < result; ++i)
			threadPoolData->vThreads.push_back(std::make_shared<Thread>(this));
		threadPoolData->freeThreads += result;
		// ���δ��ӹ����߳�ʱ�������̱߳�Ϊ�գ����������Ĺ����߳�
		if (threadPoolData->freeThreads == result)
			threadPoolData->signal.notify_one();
		return true;
	}
	else if (result < 0)	// ���ٹ����̣߳�δ�ƶ����ԣ�
	{
		return false;
	}
	return false;
}

// ��ȡ�̳߳��е�ǰ�����߳�����
unsigned ThreadPool::getCurrentThreads() const
{
	//std::lock_guard<std::mutex> threadLocker(threadPoolData->threadMutex);	// ����
	return threadPoolData->vThreads.size();
}

// ��ȡ�������������е�������
unsigned ThreadPool::getTasks() const
{
	return threadPoolData->tasks.size();
}

// �����̳߳ع����̵߳��������
void ThreadPool::setMaxThreads(unsigned maxThreads)
{
	threadPoolData->maxThreads = maxThreads ? maxThreads : 1;
}

// ��ȡ�̳߳�������߳�����
unsigned ThreadPool::getMaxThreads() const
{
	return threadPoolData->maxThreads;
}

//// �����̳߳ع�������ѯʱ��Ƭ
//bool ThreadPool::setTimeSlice(unsigned timeSlice)
//{
//	if (timeSlice < 0)
//		return false;
//	threadPoolData->timeSlice = timeSlice;
//	return true;
//}
//
//// ��ȡ�̳߳ع�������ѯʱ��Ƭ
//unsigned ThreadPool::getTimeSlice() const
//{
//	return threadPoolData->timeSlice;
//}

// �������������ӵ�����
void ThreadPool::pushTask(std::function<void()> run, std::function<void()> callback)
{
	threadPoolData->tasks.push(TaskPair(std::move(run), std::move(callback)));
	// δ�������֮ǰ���������Ϊ��ʱ�����������Ĺ����߳�
	if (threadPoolData->tasks.size() == 1)
		threadPoolData->signal.notify_one();
}

// �������������ӵ�����
void ThreadPool::pushTask(TaskPair &&task)
{
	threadPoolData->tasks.push(std::move(task));
	// δ�������֮ǰ���������Ϊ��ʱ�����������Ĺ����߳�
	if (threadPoolData->tasks.size() == 1)
		threadPoolData->signal.notify_one();
}

// ����������������������
void ThreadPool::pushTask(std::list<TaskPair> &tasks)
{
	auto size = tasks.size();
	threadPoolData->tasks.push(tasks);
	if (threadPoolData->tasks.size() == size)
		threadPoolData->signal.notify_one();
}

// �����̳߳�
void ThreadPool::destroy()
{
	if (getCloseStatus())	// ���Ѿ������̳߳أ��������²���
		return;
	threadPoolData->thread.detach();	// �����̳߳ع����߳�
	setCloseStatus(true);	// �����̳߳�Ϊ�ر�״̬��������״̬
	threadPoolData->signal.notify_one();	// ���������Ĺ����߳�
	std::unique_lock<std::mutex> threadLocker(threadPoolData->threadMutex);
	//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	// ���������̱߳��������й����߳�
	/*for each (auto &thread in threadPoolData->vThreads)
	thread->destroy();*/
	/*for (auto it = threadPoolData->vThreads.cbegin(); it != threadPoolData->vThreads.cend(); ++it)
	it->get()->destroy();*/
	for (auto &thread : threadPoolData->vThreads)
		thread->destroy();
	threadLocker.unlock();
}

bool ThreadPool::getTask(std::shared_ptr<Thread> thread)
{
	std::unique_lock<std::mutex> taskLocker(threadPoolData->tasks.mutex());
	if (!threadPoolData->tasks.empty())	// ������зǿ�
	{
		thread->configure(std::move(threadPoolData->tasks.front()));	// Ϊ�����߳�����������
		threadPoolData->tasks.pop();	// �����Ѿ����ù�������
		return true;
	}
	taskLocker.unlock();
	// �������Ϊ�գ������߳�������һ����δ���ӿ����߳�֮ǰ�������߳�����Ϊ��ʱ�����������Ĺ����߳�
	if (++threadPoolData->freeThreads == 1)
		threadPoolData->signal.notify_one();
	return false;
}

// �̳߳ع����߳���
void ThreadPool::execute()
{
	std::unique_lock<std::mutex> threadLocker(threadPoolData->threadMutex, std::defer_lock);
	std::unique_lock<std::mutex> taskLocker(threadPoolData->tasks.mutex(), std::defer_lock);
	while (!getCloseStatus())	// �̳߳ع����߳����˳�ͨ��
	{
		threadLocker.lock();
		if (!threadPoolData->freeThreads)	// ���޿����߳�
		{
			// �������̣߳��ٴλ�ȡ�߳������ȴ����������Ļ����źţ�ֱ�������߳�������Ϊ����߹رչ����̣߳��ͷ�һ���߳���
			threadPoolData->signal.wait(threadLocker,
				[this] {return threadPoolData->freeThreads || getCloseStatus(); });
			if (getCloseStatus())	// �������̱߳�����Ϊ�رգ��˳�ѭ�������������߳�
				break;	// ����unique_lock����ʱ�Զ��ͷ��������������ֶ��ͷ�
		}
		// ���������̱߳�Ϊ���й����̷߳�������
		for (auto it = threadPoolData->vThreads.begin();
			it != threadPoolData->vThreads.end() && threadPoolData->freeThreads && !getCloseStatus(); ++it)
		{
			auto &thread = *it;
			if (thread->isFree())	// �������̴߳��ڿ���״̬
			{
				taskLocker.lock();
				// ���������Ϊ�գ������̣߳��ȴ����������Ļ����źţ����һ��Ѻ�������л���������δ�����������߳�ȡ�ߣ������ٴ������߳�
				while (threadPoolData->tasks.empty())
				{
					// �������̣߳��ٴλ�ȡ���������ȴ����������Ļ����źţ�ֱ�������̱߳�Ϊ�ջ��߹رչ����̣߳��ͷ�һ��������
					threadPoolData->signal.wait(taskLocker,
						[this] {return !threadPoolData->tasks.empty() || getCloseStatus(); });
					if (getCloseStatus())
						return;
				}
				thread->configure(std::move(threadPoolData->tasks.front()));	// Ϊ�����̷߳���������
				threadPoolData->tasks.pop();
				taskLocker.unlock();
				thread->start();	// ���������еĹ����߳�
				--threadPoolData->freeThreads;	// ���й����߳�������һ
			}
		}
		threadLocker.unlock();
	}
}
