#pragma once

#include <deque>
#include <utility>
#include <mutex>
#include <map>
#include <memory>

#include "global.h"
#include "semaphore.h"
#include "user_management.h"
#include "../include/cpp-base64/base64.h"

#define RAPIDJSON_HAS_STDSTRING 1
#include "../include/rapidjson/document.h"

namespace MemoServer
{
class Controller
{
public:
	Controller() = default;

	RegPointer Register(const std::string &recv_str, std::size_t &reg_pos)
	{
		std::lock_guard<std::mutex> lock(_queue_mtx);
		std::string json_str(base64_decode(recv_str)); // TODO : check if the string has invalid base64 characters

		RegPointer res(new RegType(0, json_str));
		// auto res = std::make_shared<RegistType>(Semaphore(0), json_str);
		_reg_queue.push_back({ res, true });
		reg_pos = _reg_queue.size() - 1;
		return res;
	}

	bool RegHandle(const ComponentType comp_type, JobQueue<RegPointer> *handle)
	{
		if (!_handle_map.count(comp_type))
		{
			// log
			_handle_map[comp_type] = handle;
			return true;
		}
		else
		{
			// log
			return false;
		}
	}

	bool Dispatch(RegPointer &regist_ptr)
	{
		bool res = true;
		rapidjson::Document dom;
		dom.Parse(regist_ptr->second.c_str());
		if (dom.HasParseError() || !dom.IsObject() || !dom.HasMember(kEventGroupStr))
		{
			// log
			res = false;
		}
		else
		{
			std::string group_str(dom[kEventGroupStr].GetString());
			if (kEventGroupToCompType.count(group_str))
			{
				ComponentType type = kEventGroupToCompType.at(group_str);
				_handle_map.at(type)->Push(regist_ptr);
			}
			else
			{
				// log
				res = false;
			}
		}
		return res;
	}

	void Unregister(const std::size_t reg_pos)
	{
		std::lock_guard<std::mutex> lock(_queue_mtx);
		_reg_queue[reg_pos].second = false;
		
		_clean_cnt++;
		if (_clean_cnt >= _clean_cnt_max)
		{
			_clean_cnt = 0;
			while (_reg_queue.size() && !_reg_queue.front().second)
			{
				_reg_queue.pop_front();
			}
		}
	}

private:
	std::deque<std::pair<RegPointer, /*valid bit*/bool>> _reg_queue;
	std::map<ComponentType, JobQueue<RegPointer> *> _handle_map;
	std::mutex _queue_mtx;
	std::mutex _clean_mtx;
	int _clean_cnt = 0;
	static constexpr int _clean_cnt_max = 128;
};

}; // MemoServer