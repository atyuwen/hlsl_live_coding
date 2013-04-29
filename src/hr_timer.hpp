#ifndef _HR_TIMER_HPP_INCLUDED_
#define _HR_TIMER_HPP_INCLUDED_

#include <boost/function.hpp>
#include <list>

typedef boost::function<void(int, const tstring&)> TimerEventCallBack;

static const int gpu_query_buffer_size = 4;

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
	~HRTimer();

public:
	void Start();
	void Tick();
	void SyncTick(float sync_period);

	float GetTime() const;
	float GetDeltaTime() const;
	float GetCPUDeltaTime() const;

	void BeginGPUTimming();
	void EndGPUTimming();
	float GetGPUDeltaTime() const;

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
	long long m_count_delta_busy;
	long long m_count_per_second;

	long long m_count_delta_gpu;
	long long m_count_per_second_gpu;

	ID3D11Query *m_query_timestamp_start[gpu_query_buffer_size];
	ID3D11Query *m_query_timestamp_end[gpu_query_buffer_size];
	ID3D11Query *m_query_timestamp_disjoint[gpu_query_buffer_size];
	int m_query_idx;
	int m_query_number;
	bool m_query_skipped;

	bool m_first_tick;
	std::list<TimerEvent> m_timer_events;
};

#endif  // _HR_TIMER_HPP_INCLUDED_