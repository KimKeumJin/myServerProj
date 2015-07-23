#pragma once

#include "boost\timer\timer.hpp"
using namespace boost;
using namespace boost::timer;

#define NANOSECONDS_TO_MILISECONDS(n) (n / 1000000)

class CElapsedProfile 
{
private:
	cpu_timer	m_timer;

	// check
	cpu_times	m_check_prev;
	cpu_times	m_check_current;
	bool		m_isFirst;

	struct stTimeElapsed
	{
		long long total;
		long long user;
		long long kernel;
	};

	std::list<stTimeElapsed> m_elapsedList;


	// check loop
	enum LOOP_CHECK
	{
		TOTAL,
		USER,
		KERNEL,
		END
	};

	cpu_times	m_checkloop_prev;
	long long	m_sum[LOOP_CHECK::END];
	long long	m_min[LOOP_CHECK::END];
	long long	m_max[LOOP_CHECK::END];
	
	DWORD		m_checkloop_count;


public:
	void Start();
	void Stop();
	void Check();
	void Result();

	void LoopReset();
	void LoopBegin();
	void LoopEnd();
	void LoopResult();


public:
	CElapsedProfile(bool isStop = false);
	~CElapsedProfile();
};

