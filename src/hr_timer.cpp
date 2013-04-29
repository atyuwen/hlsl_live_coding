#include "common.hpp"
#include "hr_timer.hpp"
#include "d3d_app.hpp"
#include <algorithm>

//////////////////////////////////////////////////////////////////////////
// constructor / destructor
//////////////////////////////////////////////////////////////////////////
HRTimer::HRTimer()
	: m_count_start(0)
	, m_count_tick(0)
	, m_count_delta(0)
	, m_count_delta_busy(0)
	, m_count_per_second(1)
	, m_count_delta_gpu(0)
	, m_count_per_second_gpu(1)
	, m_first_tick(true)
	, m_query_idx(0)
	, m_query_number(0)
	, m_query_skipped(false)
{
	ZeroMemory(m_query_timestamp_start, sizeof(m_query_timestamp_start));
	ZeroMemory(m_query_timestamp_end, sizeof(m_query_timestamp_end));
	ZeroMemory(m_query_timestamp_disjoint, sizeof(m_query_timestamp_disjoint));
}

HRTimer::~HRTimer()
{
	timeEndPeriod(1);

	for (int i = 0; i != gpu_query_buffer_size; ++i)
	{
		SAFE_RELEASE(m_query_timestamp_start[i]);
		SAFE_RELEASE(m_query_timestamp_end[i]);
		SAFE_RELEASE(m_query_timestamp_disjoint[i]);
	}
}

//////////////////////////////////////////////////////////////////////////
// public interfaces
//////////////////////////////////////////////////////////////////////////
void HRTimer::Start()
{
	timeBeginPeriod(1);

	LARGE_INTEGER frequency_count;
	QueryPerformanceFrequency(&frequency_count);
	m_count_per_second = frequency_count.QuadPart;
	QueryPerformanceCounter(&frequency_count);
	m_count_start = frequency_count.QuadPart;

	CD3D11_QUERY_DESC query_timestamp_desc(D3D11_QUERY_TIMESTAMP);
	CD3D11_QUERY_DESC query_timestamp_disjoint_desc(D3D11_QUERY_TIMESTAMP_DISJOINT);

	for (int i = 0; i != gpu_query_buffer_size; ++i)
	{
		D3DApp::GetD3D11Device()->CreateQuery(&query_timestamp_desc, &m_query_timestamp_start[i]);
		D3DApp::GetD3D11Device()->CreateQuery(&query_timestamp_desc, &m_query_timestamp_end[i]);
		D3DApp::GetD3D11Device()->CreateQuery(&query_timestamp_disjoint_desc, &m_query_timestamp_disjoint[i]);
	}
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

void HRTimer::SyncTick(float sync_period)
{
	if (m_first_tick)
	{
		m_count_delta_busy = 0;
		return Tick();
	}

	LARGE_INTEGER frequency_count;
	QueryPerformanceCounter(&frequency_count);
	m_count_delta_busy = frequency_count.QuadPart - m_count_tick;
	float delta_time = static_cast<float>(m_count_delta_busy) / m_count_per_second;
	if (delta_time < sync_period)
	{
		int waiting_ms = static_cast<int>((sync_period - delta_time) * 1000);
		if (waiting_ms > 1) Sleep(waiting_ms - 1);
		do
		{
			Sleep(0);
			QueryPerformanceCounter(&frequency_count);
			long long count_delta = frequency_count.QuadPart - m_count_tick;
			delta_time = static_cast<float>(count_delta) / m_count_per_second;
		} while (delta_time < sync_period);
	}

	Tick();
}

float HRTimer::GetTime() const
{
	LARGE_INTEGER frequency_count;
	QueryPerformanceCounter(&frequency_count);
	return static_cast<float>(frequency_count.QuadPart - m_count_start) / m_count_per_second;
}

float HRTimer::GetDeltaTime() const
{
	return static_cast<float>(m_count_delta) / m_count_per_second;
}

float HRTimer::GetCPUDeltaTime() const
{
	return static_cast<float>(m_count_delta_busy) / m_count_per_second;
}

float HRTimer::GetGPUDeltaTime() const
{
	return static_cast<float>(m_count_delta_gpu) / m_count_per_second_gpu;
}

void HRTimer::BeginGPUTimming()
{
	if (m_query_number == gpu_query_buffer_size)
	{
		m_query_skipped = true;
		return;
	}

	m_query_skipped = false;
	D3DApp::GetD3D11DeviceContext()->Begin(m_query_timestamp_disjoint[m_query_idx]);
	D3DApp::GetD3D11DeviceContext()->End(m_query_timestamp_start[m_query_idx]);
}

void HRTimer::EndGPUTimming()
{
	if (!m_query_skipped)
	{
		D3DApp::GetD3D11DeviceContext()->End(m_query_timestamp_end[m_query_idx]);
		D3DApp::GetD3D11DeviceContext()->End(m_query_timestamp_disjoint[m_query_idx]);
		m_query_idx = (m_query_idx + 1) % gpu_query_buffer_size;
		m_query_number += 1;
	}

	int feeded = 0;
	for (; feeded != m_query_number; ++feeded)
	{
		int read_idx = (m_query_idx - m_query_number + feeded + gpu_query_buffer_size) % gpu_query_buffer_size;

		UINT64 timestamp_start;
		UINT64 timestamp_end;
		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT timestamp_disjoint;

		bool query_complete = true;
		query_complete &= D3DApp::GetD3D11DeviceContext()->GetData(
			m_query_timestamp_start[read_idx], &timestamp_start, sizeof(timestamp_start), 0) == S_OK;
		query_complete &= D3DApp::GetD3D11DeviceContext()->GetData(
			m_query_timestamp_end[read_idx], &timestamp_end, sizeof(timestamp_end), 0) == S_OK;
		query_complete &= D3DApp::GetD3D11DeviceContext()->GetData(
			m_query_timestamp_disjoint[read_idx], &timestamp_disjoint, sizeof(timestamp_disjoint), 0) == S_OK;

		if (!query_complete) break;

		if (!timestamp_disjoint.Disjoint)
		{
			m_count_per_second_gpu = timestamp_disjoint.Frequency;
			m_count_delta_gpu = timestamp_end - timestamp_start;
		}
	}
	m_query_number -= feeded;
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
		if (it->cur_time > it->tot_time)
		{
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
