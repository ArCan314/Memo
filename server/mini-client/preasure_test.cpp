#include <boost/asio.hpp>
#include <boost/locale.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <locale>
#include <codecvt>
#include <cstring>
#include <cstdlib>
#include <random>
#include <chrono>
#include <thread>

#include "../include/cpp-base64/base64.h"
using namespace boost::asio;

static const char *kHost = "localhost";
static const char *kPort = "12345";

std::string random_strs[102400];

std::string GenRandomStr()
{
	constexpr char dict[] = "1234567890-=qwertyuiop[]asdfghjkl;'zxcvbnm,./\\`~!@#$%^&*()_+QWERTYUIOP{}ASDFGHJKL:|ZXCVBNM<>?";
	std::default_random_engine dre(std::clock() +
								   std::hash<std::thread::id>()(std::this_thread::get_id()));
	std::uniform_int_distribution<int> uid(1, 512);
	std::uniform_int_distribution<int> dict_sel(0, sizeof(dict) - 2);

	int len = uid(dre);
	std::string res(len, '\0');
	for (int i = 0; i < len; i++)
	{
		res[i] = dict[dict_sel(dre)];
	}

	return res;
}

void InitRandomVec()
{
	for (int i = 0; i < 10240; i++)
	{
		random_strs[i] = GenRandomStr();
	}
}

std::string GenRandom()
{
	//std::default_random_engine dre(std::clock() +
	//							   std::hash<std::thread::id>()(std::this_thread::get_id()));
	//std::uniform_int_distribution<int> uid(0, 10240 - 1);
	//return random_strs[uid(dre)];
	return GenRandomStr();
}

std::string GenLogIn()
{
	constexpr char pre[] = "{\"EventGroup\":\"Account\",\"Event\":\"CreateAccount\",\"ID\":\"";
	constexpr char mid[] = "\",\"Pswd\":\"";
	constexpr char tail[] = "\"}";

	std::string id = GenRandom();
	std::string pswd = GenRandom();

	return std::string(pre) + id + mid + pswd + tail;
}

std::string GenCreate()
{
	constexpr char pre[] = "{\"EventGroup\":\"Account\",\"Event\":\"LogIn\",\"ID\":\"";
	constexpr char mid[] = "\",\"Pswd\":\"";
	constexpr char tail[] = "\"}";

	std::string id = GenRandom();
	std::string pswd = GenRandom();

	return std::string(pre) + id + mid + pswd + tail;
}

std::string GenClient()
{
	constexpr char pre[] = "{\"EventGroup\":\"Data\",\"Event\":\"SyncFromClient\",\"ID\":\"";
	constexpr char tail[] = "\"}";

	std::string id = GenRandom();

	return std::string(pre) + id + tail;
}

std::string GenServer()
{
	constexpr char pre[] = "{\"EventGroup\":\"Data\",\"Event\":\"SyncFromServer\",\"ID\":\"";
	constexpr char tail[] = "\"}";

	std::string id = GenRandom();
	return std::string(pre) + id + tail;
}

std::string GenStr()
{
	enum {LOGIN, CREATE, SYNC_CLIENT, SYNC_SERVER, RANDOM};
	static constexpr int max_option_no = RANDOM;
	std::default_random_engine dre(std::clock() +
								   std::hash<std::thread::id>()(std::this_thread::get_id()));
	std::uniform_int_distribution<int> uid(0, max_option_no);

	std::string res;
	switch (uid(dre))
	{
	case LOGIN:
		res = GenLogIn();
		break;
	case CREATE:
		res = GenCreate();
		break;
	case SYNC_CLIENT:
		res = GenClient();
		break;
	case SYNC_SERVER:
		res = GenServer();
		break;
	case RANDOM:
		res = GenRandom();
		break;
	default:
		assert(1 == 0);
	}

	return res;
}

constexpr int max_request = 100000;

void RunRandomSender()
{
	try
	{
		io_context ioc;
		ip::tcp::resolver resolver(ioc);

		auto ep = resolver.resolve(ip::tcp::v4(), kHost, kPort);
		

		ip::tcp::socket socket(ioc);
		socket.open(ip::tcp::v4());
		socket.set_option(ip::tcp::socket::reuse_address(true));
		

		connect(socket, ep);
		
		for (int req_num = 0; req_num < max_request; req_num++)
		{
			try
			{		
				std::string request = GenStr();

					// std::cout << "raw: " << request << std::endl;
					// request = ToUTF8(request);
					// std::cout << "UTF8" << request << std::endl;
				request = base64_encode(reinterpret_cast<const unsigned char *>(request.c_str()), request.size());
				// std::cout << "b64: " << request << std::endl;
				write(socket, buffer(request, request.size()));

				std::vector<unsigned char> buf(10240);
				std::string reply;
				auto len = socket.read_some(buffer(buf));
				for (int i = 0; i < len; i++)
					reply.push_back(buf[i]);

				// std::cout << "b64_reply: " << reply << std::endl;
				//std::cout << "reply: " << base64_decode(reply) << std::endl;
			}
			catch (const std::exception & e)
			{
				std::cerr << e.what() << std::endl;
			}
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
}

class Timer
{
public:
	using RetType = unsigned long long;
	Timer() = default;
	void start() noexcept
	{
		start_ = std::chrono::steady_clock::now().time_since_epoch();
	}

	void end() noexcept
	{
		end_ = std::chrono::steady_clock::now().time_since_epoch();
	}

	RetType count() const noexcept
	{
		return (end_ - start_).count();
	}

private:
	std::chrono::nanoseconds start_;
	std::chrono::nanoseconds end_;
};

int main()
{
	InitRandomVec();
	std::vector<std::thread> tvec;
	constexpr int max_tvec_size = 12;
	Timer tm;
	tm.start();
	for (int i = 0; i < max_tvec_size; i++)
	{
		tvec.emplace_back(RunRandomSender);
	}

	for (int i = 0; i < tvec.size(); i++)
	{
		tvec[i].join();
	}
	tm.end();

	std::cout << "Send " << (max_tvec_size * max_request) << " requests in " <<
		tm.count() / 1000000.0 << " ms." << std::endl;
	return 0;
}