#include "stdafx.h"
#include "ElapsedProfile.h"


CElapsedProfile::CElapsedProfile(bool isStop)
	: m_isFirst(true),
	m_checkloop_count(0)
{
	if (isStop)
		m_timer.stop();

	// default setting
	for (int i = 0; i < LOOP_CHECK::END; ++i)
	{
		m_sum[i] = 0;
		m_max[i] = 0;
		m_min[i] = LLONG_MAX;
	}
}

CElapsedProfile::~CElapsedProfile()
{
}
void CElapsedProfile::Start()
{
	m_timer.start();
}

void CElapsedProfile::Stop()
{
	m_timer.stop();
}

void CElapsedProfile::Check()
{
	cpu_times checktime = m_timer.elapsed();

	if (m_isFirst)
	{
		m_isFirst = false;

		m_check_prev = checktime;
		m_check_current = checktime;
		
	}
	else
	{
		m_check_prev = m_check_current;
		m_check_current = checktime;

		stTimeElapsed elapsed;
		elapsed.total = m_check_current.wall - m_check_prev.wall;
		elapsed.user = m_check_current.user - m_check_prev.user;
		elapsed.kernel = m_check_current.system - m_check_prev.system;

		m_elapsedList.push_back(elapsed);
	}
}

void CElapsedProfile::Result()
{
	m_isFirst = true;

	int index = 0;
	std::list<stTimeElapsed>::iterator iter;
	for (iter = m_elapsedList.begin(); iter != m_elapsedList.end(); ++iter)
	{
		stTimeElapsed& elapsed = *iter;

		printf("#%d - elapsed:%lld ms (user:%lld ms, kernel:%lld ms)\n", index++,
			NANOSECONDS_TO_MILISECONDS(elapsed.total),
			NANOSECONDS_TO_MILISECONDS(elapsed.user),
			NANOSECONDS_TO_MILISECONDS(elapsed.kernel));
	}
}

void CElapsedProfile::LoopReset()
{
	// default setting
	for (int i = 0; i < LOOP_CHECK::END; ++i)
	{
		m_sum[i] = 0;
		m_max[i] = 0;
		m_min[i] = LLONG_MAX;
	}

	m_checkloop_count = 0;
}

void CElapsedProfile::LoopBegin()
{
	m_checkloop_prev = m_timer.elapsed();

}

void CElapsedProfile::LoopEnd()
{
	cpu_times checktime = m_timer.elapsed();

	cpu_times elapsed;
	elapsed.wall = checktime.wall - m_checkloop_prev.wall;
	elapsed.user = checktime.user - m_checkloop_prev.user;
	elapsed.system = checktime.system - m_checkloop_prev.system;


	m_sum[LOOP_CHECK::TOTAL] += elapsed.wall;
	m_sum[LOOP_CHECK::USER] += elapsed.user;
	m_sum[LOOP_CHECK::KERNEL] += elapsed.system;

	if (m_min[LOOP_CHECK::USER] > elapsed.user)
		m_min[LOOP_CHECK::USER] = elapsed.user;

	if (m_min[LOOP_CHECK::KERNEL] > elapsed.system)
		m_min[LOOP_CHECK::KERNEL] = elapsed.system;


	if (m_max[LOOP_CHECK::USER] < elapsed.user)
		m_max[LOOP_CHECK::USER] = elapsed.user;

	if (m_max[LOOP_CHECK::KERNEL] < elapsed.system)
		m_max[LOOP_CHECK::KERNEL] = elapsed.system;

	++m_checkloop_count;
}

void CElapsedProfile::LoopResult()
{
	if (m_checkloop_count < 1)
		return;

	printf("##### loop result  count(%ld) #####\n", m_checkloop_count);
	printf("total - %lld ms\n", NANOSECONDS_TO_MILISECONDS(m_sum[LOOP_CHECK::TOTAL]));

	printf("user - min:%lld ms, max:%lld ms, average:%.5f ms\n",
		NANOSECONDS_TO_MILISECONDS(m_min[LOOP_CHECK::USER]),
		NANOSECONDS_TO_MILISECONDS(m_max[LOOP_CHECK::USER]),
		NANOSECONDS_TO_MILISECONDS((m_sum[LOOP_CHECK::USER] / (double)m_checkloop_count)));

	printf("kernel - min:%lld ms, max:%lld ms, average:%.5f ms\n",
		NANOSECONDS_TO_MILISECONDS(m_min[LOOP_CHECK::KERNEL]),
		NANOSECONDS_TO_MILISECONDS(m_max[LOOP_CHECK::KERNEL]),
		NANOSECONDS_TO_MILISECONDS((m_sum[LOOP_CHECK::KERNEL] / (double)m_checkloop_count)));

	LoopReset();
}

