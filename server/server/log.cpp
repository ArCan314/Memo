#include "log.h"

#include <thread>
#include <mutex>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <string>

namespace MemoServer
{

static const std::string log_dir = "./";
static const std::string log_ext = ".log.txt";
static std::string log_path;

static std::mutex mtx; // 避免流失去同步
static std::ofstream log_ofs;

static int dbg_level = 0;
static int log_id = 0;

static bool is_inited = false;
static bool is_output_to_stdout = false;

static const std::map<LogLevel, std::string> kLogLevelToStr =
{
	{LogLevel::ERROR, "[ERROR]"},
	{LogLevel::WARN, "[WARN]"},
	{LogLevel::INFO, "[INFO]"},
	{LogLevel::DEBUG, "[DEBUG]"}
};


void Log::InitLog(const LogLevel max_log_level)
{
	std::lock_guard<std::mutex> lock(mtx);
	if (is_inited)
	{
		std::stringstream ss;
		std::time_t current_time = std::time(nullptr);
		std::tm time_pack;
		localtime_s(&time_pack, &current_time);

		ss << std::put_time(&time_pack, "%d-%H%M%S");
		std::string log_name = ss.str();

		log_path = log_dir + log_name + log_ext;

		log_ofs.open(log_path);
		if (log_ofs)
		{
			dbg_level = EnumToInt(max_log_level);
			is_inited = true;
		}
	}
}

void Log::SetOption(const LogOption option, const int value)
{
	std::lock_guard<std::mutex> lock(mtx);
	switch (option)
	{
	case LogOption::OutputToStdout:
		is_output_to_stdout = true;
		break;
	default:
		break;
	}
}

bool Log::WriteLog(const LogLevel level, const std::string &log)
{
	std::lock_guard<std::mutex> lock(mtx);
	if (EnumToInt(level) > dbg_level)
		return false;

	if (!is_inited)
	{
		InitLog(LogLevel::DEBUG);
	}

	static std::stringstream ss;
	std::string timestamp, current_thread_id, log_id_str;
	std::time_t current_time = std::time(nullptr);
	std::tm time_pack;
	localtime_s(&time_pack, &current_time);

	log_id_str = std::to_string(log_id++) + "\t";

	ss << std::put_time(&time_pack, "%H:%M:%S") << " ";
	timestamp = ss.str();
	ss.clear();
	ss.str(std::string());

	ss << "tid: " << std::this_thread::get_id() << "\t";
	current_thread_id = ss.str();
	ss.clear();
	ss.str(std::string());

	const std::string &dbg_level_str = kLogLevelToStr.at(level);

	if (is_output_to_stdout)
		std::cout << log_id_str << timestamp << current_thread_id << dbg_level_str << ": " << log << std::endl;
	log_ofs << log_id_str << timestamp << current_thread_id << dbg_level_str << ": " << log << std::endl;

	return true;
}

void Log::CloseLog()
{
	std::lock_guard<std::mutex> lock(mtx);
	if (log_ofs)
		log_ofs.close();

	if (is_inited)
		is_inited = false;
}

};
