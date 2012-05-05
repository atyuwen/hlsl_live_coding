#include "common.hpp"
#include "hr_timer.hpp"
#include <algorithm>

//////////////////////////////////////////////////////////////////////////
// constructor / destructor
//////////////////////////////////////////////////////////////////////////
HRTimer::HRTimer()
	: m_count_start(0)
	, m_count_tick(0)
	, m_count_delta(0)
	, m_count_per_seccond(1)
	, m_first_tick(true)
{
	Start();
}

//////////////////////////////////////////////////////////////////////////
// public interfaces
//////////////////////////////////////////////////////////////////////////
void HRTimer::Start()
{
	LARGE_INTEGER frequency_count;
	QueryPerformanceFrequency(&frequency_count);
	m_count_per_seccond = frequency_count.QuadPart;
	QueryPerformanceCounter(&frequency_count);
	m_count_start = frequency_count.QuadPart;
}

void HRTimer::Tick()
{
	LARGE_INTEGER frequency_count;
	QueryPerformanceCounter(&frequency_count);
	m_count_delta = frequency_count.QuadPart - m_count_tick;
	m_count_tick = frequency_count.QuadPart;
	if (m_first_tick)
	{
		m_count_delta = 0;
		m_first_tick = false;
	}
	else
	{
		m_count_delta = std::max<long long>(0, m_count_delta);
	}

	TickTimerEvents(GetDeltaTime());
}

float HRTimer::GetTime()
{
	LARGE_INTEGER frequency_count;
	QueryPerformanceCounter(&frequency_count);
	return static_cast<float>(frequency_count.QuadPart - m_count_start) / m_count_per_seccond;
}

float HRTimer::GetDeltaTime()
{
	return static_cast<float>(m_count_delta) / m_count_per_seccond;
}

void HRTimer::RemoveEvent(const tstring& tag)
{
	for (auto it = m_timer_events.begin(); it != m_timer_events.end();)
	{
		if (it->tag == tag) it = m_timer_events.erase(it);
		else ++it;
	}
}

void HRTimer::TickTimerEvents(float delta_time)
{
	for (auto it = m_timer_events.begin(); it != m_timer_events.end();)
	{
		it->cur_time += delta_time;
		if (it->cur_time > it->tot_time) {
			// issue callback
			(it->callback)(it->cur_count, it->tag);
			it->cur_time = 0;
			if (++(it->cur_count) == it->tot_count)
			{
				it = m_timer_events.erase(it);
				continue;
			}
		}
		++it;
	}
}
