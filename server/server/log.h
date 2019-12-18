#pragma once

#include <map>
#include <string>

namespace MemoServer
{
enum class LogLevel
{
	ERROR = 0,
	WARN,
	DEBUG,
	INFO
};

enum class LogOption
{
	OutputToStdout
};

template <typename EnumType>
inline int EnumToInt(EnumType enum_val)
{
	static_assert(std::is_enum<EnumType>::value, "EnumToInt requires valid EnumType.");
	return static_cast<int>(enum_val);
}

template <typename NumType>
inline std::string NumStr(const NumType num)
{
	static_assert(std::is_arithmetic<NumType>::value, "NumType must be arithmetic.");
	return std::to_string(num); // error handling
}

inline std::string __Str(const char *cstr)
{
	return std::string(cstr);
}

class Log
{
public:
	static void InitLog(const LogLevel max_log_level);
	static void SetOption(const LogOption option, const int value);
	static bool WriteLog(const LogLevel level, const std::string &log);
	static void CloseLog();
};
};