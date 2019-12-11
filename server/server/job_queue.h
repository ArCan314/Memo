#pragma once

#include <deque>
#include <mutex>

#include "semaphore.h"


namespace MemoServer
{
template <typename T>
class JobQueue
{
public:
	JobQueue() : _jobs(), _semaphore() {}
	void Push(const T &job)
	{
		std::lock_guard<>(_mtx);
		_jobs.push_back(job);
		_semaphore.Signal();
	}

	T Pop()
	{
		_semaphore.Wait();
		std::lock_guard<>(_mtx);
		T res = _jobs.front();
		_jobs.pop_front();
		return res;
	}

	JobQueue *GetReg() noexcept
	{
		return this;
	}

private:
	std::deque<T> _jobs;
	Semaphore _semaphore;
	std::mutex _mtx;
}; // JobQueue
}; // MemoServer