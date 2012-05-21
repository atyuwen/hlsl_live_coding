#ifndef _HR_TIMER_HPP_INCLUDED_
#define _HR_TIMER_HPP_INCLUDED_

#include <boost/function.hpp>
#include <list>

typedef boost::function<void(int, const tstring&)> TimerEventCallBack;

class HRTimer
{
	struct TimerEvent 
	{
		tstring tag;
		int cur_count;
		int tot_count;
		float cur_time;
		float tot_time;
        TimerEventCallBack callback;
	};

public:
	HRTimer();

public:
	void Start();
	void Tick();
	bool SyncTick(float sync_period);

	float GetTime();
	float GetDeltaTime();

	template<typename Func>
	void AddEvent(float interval, Func callback, const tstring& tag = TEXT(""), int tot_cnt = 0)
	{
		TimerEvent evt = {
			tag,
			0,
			tot_cnt,
			0,
			interval,
			TimerEventCallBack(callback)
		};
		m_timer_events.push_back(evt);
	}
	
	void RemoveEvent(const tstring& tag);

private:
	void TickTimerEvents(float delta_time);

private:
	long long m_count_start;
	long long m_count_tick;
	long long m_count_delta;
	long long m_count_per_seccond;
	bool m_first_tick;
	std::list<TimerEvent> m_timer_events;
};

#endif  // _HR_TIMER_HPP_INCLUDED_