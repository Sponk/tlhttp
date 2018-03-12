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

#include "Server.h"
#include <memory>
#include <cstring>

using namespace tlhttp;

void Server::start(const std::function<bool(const std::shared_ptr<Connection>&)>& requestHandler)
{
	if(m_running)
		throw std::runtime_error("Server is already running on one thread!");

	m_running = true;

	struct addrinfo hints, *sockaddr;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	m_socket = getaddrinfo(m_address.c_str(), std::to_string(m_port).c_str(), &hints, &sockaddr);

	m_socketFd = socket(sockaddr->ai_family, sockaddr->ai_socktype, sockaddr->ai_protocol);
	if(m_socketFd < 0)
		throw std::runtime_error("Could not create socket to " + m_address + ": " + gai_strerror(m_socketFd));

	bind(m_socketFd, sockaddr->ai_addr, sockaddr->ai_addrlen);
	listen(m_socketFd, -1);

	struct sockaddr* clientAddr = new struct sockaddr;
	while(m_running)
	{
		int fd = ::accept(m_socketFd, clientAddr, &hints.ai_addrlen);
		if(fd == -1)
			throw std::runtime_error(std::string("Could not create socket to client: ") + gai_strerror(fd));

		auto conn = std::shared_ptr<Connection>(new Connection(fd));
		if(!requestHandler(conn))
		{
			throw std::runtime_error("Request handler failed!");
		}
	}

	delete clientAddr;
	freeaddrinfo(sockaddr);
}

void Server::stop()
{
	m_running = false;

	if(m_socketFd)
	{
		close(m_socketFd);
	}
}

