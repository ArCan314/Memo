#pragma once

namespace MemoServer
{
template <typename QueueType>
class AbstractComponent
{
public:
	virtual QueueType *GetHandle() = 0;
};
} // namesapce MemoServer