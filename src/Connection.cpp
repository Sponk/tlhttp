// TinyLittleHTTP
// Copyright (c) Yannick Pflanzer 2017, All rights reserved.
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

#include <openssl/x509.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include <sys/types.h>

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <sstream>

#include <iostream>
#include <thread>

#include "Connection.h"

using namespace tlhttp;

namespace
{
class InitSSL
{
public:
	InitSSL()
	{
		SSL_load_error_strings();
		SSL_library_init();
	}
};

static InitSSL initSSL;
}

Connection::~Connection()
{
	if (m_socketFd > 0)
	{
		shutdown(m_socketFd, SHUT_WR);
		close(m_socketFd);
	}
}

void Connection::connect(const std::string& address, uint16_t port)
{
	m_address = address;
	m_port = port;

	if(m_socketFd)
	{
		close(m_socketFd);
	}

	struct addrinfo hints, *sockaddr;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	m_socket = getaddrinfo(address.c_str(), std::to_string(port).c_str(), &hints, &sockaddr);

	int err;
	m_socketFd = socket(sockaddr->ai_family, sockaddr->ai_socktype, sockaddr->ai_protocol);
	if(m_socketFd < 0)
		throw std::runtime_error("Could not create socket to " + address + ": " + gai_strerror(m_socketFd));

	if((err = ::connect(m_socketFd, sockaddr->ai_addr, sockaddr->ai_addrlen)) < 0)
		throw std::runtime_error("Could not connect to " + address + ": " + gai_strerror(err));

	freeaddrinfo(sockaddr);
}

void Connection::send(const std::string& message)
{
	if(!m_socketFd)
		throw std::runtime_error("Not connected!");

	::send(m_socketFd, message.c_str(), message.size(), 0);
}

std::string Connection::receive()
{
	if(!m_socketFd)
		throw std::runtime_error("Not connected!");

	char buffer[512];
	buffer[511] = 0;

	int err = 0;
	std::stringstream ss;
	while((err = ::recv(m_socketFd, buffer, sizeof(buffer) - 1, 0)) > 0)
	{
		buffer[err] = 0;
		ss << buffer;
	}

	if(err != 0)
		throw std::runtime_error("Error while receiving data!");

	return ss.str();
}

Request Connection::get()
{
	if(!m_socketFd)
		throw std::runtime_error("Not connected!");

	char buffer[512];
	buffer[511] = 0;

	int err = 0;
	Request req;

	// Get header
	if((err = ::recv(m_socketFd, buffer, sizeof(buffer) - 1, 0)) <= 0)
		throw std::runtime_error("Could not fetch HTTP header!");

	req = Request::parse(std::string(buffer, err));

	// Calculate remaining size
	size_t bytecount = std::stoll(req["Content-Length"]) - req.getBody().str().size();
	while((err = ::recv(m_socketFd, buffer, sizeof(buffer) - 1, 0)) > 0 && bytecount > 0)
	{
		bytecount -= err;
		buffer[err] = 0;
		req << buffer;
	}

	if(err != 0)
		throw std::runtime_error("Error while receiving data!");

	return req;
}

Request Connection::get(const std::string& url, const std::string& str)
{
	Request req(m_address, url, false);
	req << str;
	send(req.toString());
	return Request::parse(receive());
}

SSLConnection::~SSLConnection()
{
	if(m_sslHandle != nullptr)
	{
		SSL_shutdown(m_sslHandle);
		SSL_free(m_sslHandle);
	}

	if(m_sslContext != nullptr)
	{
		SSL_CTX_free(m_sslContext);
	}
}

void SSLConnection::connect(const std::string& address, uint16_t port)
{
	Connection::connect(address, port);
	m_sslContext = SSL_CTX_new(SSLv23_method());

	if(!m_sslContext) throw std::runtime_error(std::string("Could not create SSL context: ") + ERR_error_string(0, nullptr));
	//SSL_CTX_set_options(m_sslContext, SSL_OP_NO_COMPRESSION );

	m_sslHandle = SSL_new(m_sslContext);
	if(!m_sslContext)
	{
		SSL_CTX_free(m_sslContext);
		m_sslContext = nullptr;

		throw std::runtime_error(std::string("Could not create SSL handle: ") + ERR_error_string(0, nullptr));
	}

	if(!SSL_set_fd(m_sslHandle, getSocket()))
	{
		SSL_free(m_sslHandle);
		m_sslHandle = nullptr;

		SSL_CTX_free(m_sslContext);
		m_sslContext = nullptr;

		throw std::runtime_error(std::string("Could not create SSL socket: ") + ERR_error_string(0, nullptr));
	}

	for(int error = SSL_connect(m_sslHandle); error != 1; error = SSL_connect(m_sslHandle))
	{
		switch(SSL_get_error(m_sslHandle, error))
		{
			case SSL_ERROR_WANT_READ:
			case SSL_ERROR_WANT_WRITE:
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				break;
			default:
				SSL_free(m_sslHandle);
				SSL_CTX_free(m_sslContext);

				m_sslContext = nullptr;
				m_sslHandle = nullptr;

				throw std::runtime_error(std::string("SSL handshake error: ") + ERR_error_string(error, nullptr));
				break;
		}
	}
}

void SSLConnection::send(const std::string& message)
{
	if(!m_sslHandle)
		throw std::runtime_error("Not connected!");

	SSL_write(m_sslHandle, message.c_str(), message.size());
}

std::string SSLConnection::receive()
{	
	if(!m_sslHandle)
		throw std::runtime_error("Not connected!");

	char buffer[512];
	buffer[511] = 0;

	int err = 0;
	std::stringstream ss;
	
	
	while((err = SSL_read(m_sslHandle, buffer, sizeof(buffer) - 1)) > 0)
	{
		switch(SSL_get_error(m_sslHandle, err))
		{
			case SSL_ERROR_NONE:
				buffer[err] = 0;
				ss << buffer;
				break;

			case SSL_ERROR_WANT_READ:
				continue;

			case SSL_ERROR_SYSCALL:
			case SSL_ERROR_ZERO_RETURN:
				return ss.str();

			default: throw std::runtime_error(std::string("Error while receiving data: ") + ERR_error_string(err, nullptr));
		}
	}

	if(err != 0)
		throw std::runtime_error(std::string("Error while receiving data: ") + ERR_error_string(err, nullptr));

	return ss.str();
}
