// TinyLittleHTTP
// Copyright (c) 2017 Yannick Pflanzer, All rights reserved.
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3.0 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library.

#ifndef TLHTTP_SERVER_H
#define TLHTTP_SERVER_H

#include <memory>
#include <functional>
#include "Connection.h"

namespace tlhttp
{
class Server
{
	bool m_running;

	uint16_t m_port;
	std::string m_address;
	int m_socket, m_socketFd;

public:
	Server(const std::string& address, uint16_t port)
		: m_port(port),
		  m_address(address),
		  m_running(false),
		  m_socket(0), m_socketFd(0) {}

	~Server()
	{
		if(m_socketFd)
			close(m_socketFd);
	}

	void start(const std::function<bool(const std::shared_ptr<Connection>&)>& requestHandler);
	void stop();
};
}
#endif
