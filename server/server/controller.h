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
		std::string json_str(base64_decode(recv_str)); // TODO : check if the string has invalid base64 characters

		RegPointer res(new RegType(0, json_str));
		bool is_fin = false;

		std::lock_guard<std::mutex> lock(_queue_mtx);

		while (!is_fin)
		{
			if (!_reg_queue[_queue_now].second)
			{
				_reg_queue[_queue_now] = { res, true };
				reg_pos = _queue_now;

				is_fin = true;
			}
			_queue_now++;
			if (_queue_now >= kQueueLen)
				_queue_now = 0;
		}

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

	void Unregister(std::size_t reg_pos)
	{
		_reg_queue[reg_pos].second = false;
	}


private:
	static constexpr int kQueueLen = 1024;
	std::pair<RegPointer, bool> _reg_queue[kQueueLen]; // true means the resource is being used 
	int _queue_now = 0;
	std::mutex _queue_mtx;

	std::map<ComponentType, JobQueue<RegPointer> *> _handle_map;
};

}; // MemoServer