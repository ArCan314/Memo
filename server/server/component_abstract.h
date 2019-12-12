#pragma once

#include "job_queue.h"

namespace MemoServer
{
template <typename T>
class ComponentAbstract
{
public:
	ComponentAbstract();
	virtual void Start() = 0;
	
	JobQueue<T> *GetHandle()
	{
		return &_jobs;
	}

protected:
	JobQueue<T> _jobs;
};

};