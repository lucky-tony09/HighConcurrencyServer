#ifndef _CELLTimestamp_hpp_
#define _CELLTimestamp_hpp_
//�ͻ����������������ʱʱ��
#define CLIENT_HREAT_DEAD_TIME 60000
//#include <windows.h>
#include<chrono>
using namespace std::chrono;

class CELLTime
{
public:
	//��ȡ��ǰʱ��� (����)
	static time_t getNowInMilliSec()
	{
		return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	}
};
class CELLTimestamp
{
public:
	CELLTimestamp()
	{
		update();
	}
	~CELLTimestamp()
	{}
	//��������
	void update()
	{
		_begin = high_resolution_clock::now();
	}
	/**
	*   ��ȡ��ǰ��
	*/
	double getElapsedSecond()
	{
		return  getElapsedTimeInMicroSec() * 0.000001;
	}
	/**
	*   ��ȡ����
	*/
	double getElapsedTimeInMilliSec()
	{
		return this->getElapsedTimeInMicroSec() * 0.001;
	}
	/**
	*   ��ȡ΢��
	*/
	long long getElapsedTimeInMicroSec()
	{
		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
	}
protected:
	time_point<high_resolution_clock> _begin;
};

#endif // !_CELLTimestamp_hpp_