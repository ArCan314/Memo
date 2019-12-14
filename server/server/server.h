#pragma once

#define BOOST_ASIO_NO_DEPRECATED
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <boost/asio.hpp>
#include <iostream>

#include "controller.h"


namespace MemoServer
{
using namespace boost::asio::ip;

class ServerSession : public std::enable_shared_from_this<ServerSession>
{
public:
	ServerSession(tcp::socket socket, boost::asio::io_context &io_context, Controller *ctrler) 
		: _strand(io_context),
		  _socket(std::move(socket)),
		  _ctrler(ctrler),
		  _buf_vec(10240)
	{
	}

	void Start()
	{
		ReadClientData();
	}

private:

	void ReadClientData()
	{
		_buf.clear();
		auto self(shared_from_this());
		_socket.async_receive(boost::asio::buffer(_buf_vec),
							  boost::asio::bind_executor(_strand, [this, self](const boost::system::error_code ec, std::size_t byte_transfered)
							  {
								  if (!ec)
								  {
									  for (int i = 0; i < byte_transfered; i++)
										  _buf.push_back(_buf_vec[i]);
									  Proceed(byte_transfered);
								  }
							  }));
	}

	void Proceed(const std::size_t len)
	{
		std::size_t reg_pos;
		std::cerr << "Receive: " << _buf << std::endl;
		RegPointer reg_ptr = _ctrler->Register(_buf, reg_pos);
		bool is_recv_str_valid = _ctrler->Dispatch(reg_ptr);
		if (is_recv_str_valid)
		{
			reg_ptr->first.Wait();
			_respond = reg_ptr->second;
			
		}
		else
		{
			static const char *invalid_json_query = "SU5WQUxJRCBKU09OIFFVRVJZLg==";
			reg_ptr->second = invalid_json_query;
			// log
			// _respond = error json
		}
		_ctrler->Unregister(reg_pos);

		_respond.swap(reg_ptr->second);
		std::cerr << "Result: " << _respond << std::endl;
		Respond();
	}

	void Respond()
	{
		auto self(shared_from_this());
		boost::asio::async_write(_socket, boost::asio::buffer(_respond, _respond.length()),
								 boost::asio::bind_executor(_strand, [this, self](const boost::system::error_code ec, std::size_t write_len)
								 {
									 if (!ec)
									 {
										 ReadClientData();
									 }
								 }));

	}

	tcp::socket _socket;
	std::vector<unsigned char> _buf_vec;
	std::string _buf;
	std::string _respond;
	boost::asio::io_context::strand _strand;
	Controller *_ctrler;
};

class Server
{
public:
	Server(boost::asio::io_context &io_context, short port, Controller *ctrler) 
		: _io_context(io_context),
		  _acceptor(io_context, tcp::endpoint(tcp::v4(), port))
	{
		Accept(ctrler);
	}

private:

	void Accept(Controller * ctrler)
	{
		_acceptor.async_accept(
			[this, ctrler](const boost::system::error_code ec, tcp::socket socket)
			{
				if (!ec)
				{
					std::make_shared<ServerSession>(std::move(socket), _io_context, ctrler)->Start();
				}

				Accept(ctrler);
			});
	}

	boost::asio::io_context &_io_context;
	tcp::acceptor _acceptor;
};

};// MemoServer End