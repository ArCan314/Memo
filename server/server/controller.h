#pragma once

#include <deque>
#include <utility>
#include <mutex>
#include <map>
#include <memory>
#include <string>

#include "global.h"
#include "semaphore.h"
#include "user_management.h"
#include "log.h"
#include "../include/cpp-base64/base64.h"

#define RAPIDJSON_HAS_STDSTRING 1
#include "../include/rapidjson/document.h"
#include "../include/rapidjson/error/error.h"
#include "../include/rapidjson/error/en.h"

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

		_reg_queue[_queue_now] = res;
		_queue_now++;
		if (_queue_now >= kQueueLen)
			_queue_now = 0;

		return res;
	}

	void RegHandle(const ComponentType comp_type, JobQueue<RegPointer> *handle)
	{
		WRITE_LOG(LogLevel::DEBUG,
					  __Str("Registing handle of a ")
					  .append(kCompTypeToStr.at(comp_type))
					  .append(" at address ")
					  .append(NumStr(reinterpret_cast<int64_t>(handle)))
					  .append("."));
		_handle_map[comp_type] = handle;
	}

	bool Dispatch(RegPointer &regist_ptr)
	{
		bool res = true;
		rapidjson::Document dom;
		dom.Parse(regist_ptr->second.c_str());
		// rapidjson::ParseErrorCode ec = dom.Parse(regist_ptr->second.c_str(), rapidjson::kParseValidateEncodingFlag).GetParseError();
		if (dom.HasParseError() || !dom.IsObject() || !dom.HasMember(kEventGroupStr))
		{
			WRITE_LOG(LogLevel::WARN,
						  __Str("Cannot parse string ")
						  .append(regist_ptr->second)
						  .append(", parse error: ")
						  .append(rapidjson::GetParseError_En(dom.GetParseError())));

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
				WRITE_LOG(LogLevel::WARN,
							  __Str("Invalid EventGroup: ")
							  .append(group_str));

				res = false;
			}
		}
		return res;
	}

	//void Unregister(const std::size_t reg_pos)
	//{
	//	std::lock_guard<std::mutex> lock(_queue_mtx);
	//	_reg_queue[reg_pos].second = false;
	//	
	//	_clean_cnt++;
	//	if (_clean_cnt >= _clean_cnt_max)
	//	{
	//		int has_cleaned = 0;
	//		_clean_cnt = 0;
	//		while (_reg_queue.size() && !_reg_queue.front().second)
	//		{
	//			has_cleaned++;
	//			_reg_queue.pop_front();
	//		}

	//		WRITE_LOG(LogLevel::DEBUG,
	//					  __Str("Release ")
	//					  .append(NumStr(has_cleaned))
	//					  .append(" instances in reg_queue."));
	//	}
	//}

private:
	static constexpr int kQueueLen = 64;
	RegPointer _reg_queue[kQueueLen];
	int _queue_now = 0;
	std::mutex _queue_mtx;

	std::map<ComponentType, JobQueue<RegPointer> *> _handle_map;
	
	// std::mutex _clean_mtx;
	// static constexpr int _clean_cnt_max = 102400;
};

}; // MemoServer