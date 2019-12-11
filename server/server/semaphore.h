#pragma once

#include <mutex>
#include <condition_variable>

namespace MemoServer
{
class Semaphore
{
public:
	Semaphore(unsigned long count = 0) : _count(count)
	{
	}

	Semaphore(const Semaphore &) = delete;
	Semaphore &operator=(const Semaphore &) = delete;

	void Signal()
	{
		std::unique_lock<std::mutex> lock(_mtx);
		_count++;
		_cv.notify_one();
	}

	void Wait()
	{
		std::unique_lock<std::mutex> lock(_mtx);
		while (_count == 0)
		{
			_cv.wait(lock);
		}
		_count--;
	}

private:
	unsigned long _count;
	std::mutex _mtx;
	std::condition_variable _cv;
};
};