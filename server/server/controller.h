#pragma once

#include <deque>
#include <utility>
#include <mutex>
#include <memory>

#include "semaphore.h"
#include "../include/cpp-base64/base64.h"

#define RAPIDJSON_HAS_STDSTRING 1
#include "../include/rapidjson/document.h"

namespace MemoServer
{
using RegistType = std::pair<Semaphore, std::string>;
using RegistPointer = std::shared_ptr<RegistType>;

class Controller
{
public:
	Controller() = default;

	RegistPointer Register(const std::string &recv_str, std::size_t &reg_pos)
	{
		std::lock_guard<std::mutex> lock(_queue_mtx);
		std::string json_str(base64_decode(recv_str)); // TODO : check if the string has invalid base64 characters

		RegistPointer res(new RegistType(1, json_str));
		// auto res = std::make_shared<RegistType>(Semaphore(1), json_str);
		_reg_queue.push_back({ res, true });
		return res;
	}

	bool Dispatch(RegistPointer &regist_ptr)
	{
		rapidjson::Document dom;
		dom.Parse(regist_ptr->second);
		if (dom.HasParseError() || !dom.IsObject() || !dom.HasMember("EventType"))
		{
			return false;
		}
		else
		{
			// type = parse
			// switch(type) -> call processors
		}
		
	}

	void Unregister(const std::size_t reg_pos)
	{
		std::lock_guard<std::mutex> lock(_queue_mtx);
		_reg_queue[reg_pos].second = false;
		
		_clean_cnt++;
		if (_clean_cnt >= _clean_cnt_max)
		{
			_clean_cnt = 0;
			while (!_reg_queue.front().second)
			{
				_reg_queue.pop_front();
			}
		}
	}

private:
	std::deque<std::pair<RegistPointer, /*valid bit*/bool>> _reg_queue;
	std::mutex _queue_mtx;
	std::mutex _clean_mtx;
	int _clean_cnt = 0;
	static constexpr int _clean_cnt_max = 128;
};

}; // MemoServer