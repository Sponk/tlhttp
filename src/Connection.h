// TinyLittleHTTP
// Copyright (c) 2017-2018 Yannick Pflanzer, All rights reserved.
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

#ifndef TLHTTP_CONNECTION_H
#define TLHTTP_CONNECTION_H

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <openssl/ssl.h>

#include "Request.h"

namespace tlhttp
{

/**
 * @brief Implements a TCP socket.
 * 
 * This class allows to send and receive bytes as well as HTTP
 * requests.
 */
class Connection
{
	uint16_t m_port;
	std::string m_address;
	int m_socket, m_socketFd;
	
public:
	Connection() : m_port(0), m_socket(0), m_socketFd(0) {}
	Connection(int fd) : m_socketFd(fd) {}

	~Connection();

	/**
	 * @brief Connects to a remote TCP server.
	 * @param address The DNS name or IP of the server.
	 * @param port The port to connect to.
	 * @throws std::runtime_error on failure.
	 */
	virtual void connect(const std::string& address, uint16_t port = 80);
	
	/**
	 * @brief Sends a string over the connection.
	 * @param message The message to send.
	 * @throws std::runtime_error on failure.
	 */
	virtual void send(const std::string& message);
	
	/**
	 * @brief Receives all bytes until the connection is closed.
	 * @return The string that was received.
	 * @note Only supports text based communication.
	 * @throws std::runtime_error on failure.
	 */
	virtual std::string receive();

	virtual Request get();

	/**
	 * @brief Sends a POST/GET request over the connection.
	 * @param url The parameter.
	 * @param str The body.
	 * @return The parsed response.
	 * @throws std::runtime_error on failure.
	 */
	Request get(const std::string& url, const std::string& str);

	uint16_t getPort() const
	{
		return m_port;
	}

	const std::string& getAddress() const
	{
		return m_address;
	}

	int getSocket() const
	{
		return m_socketFd;
	}
};

/**
 * @brief Implements an SSL secured TCP socket.
 * 
 * This class allows to do HTTPS requests.
 */
class SSLConnection : public Connection
{
	SSL* m_sslHandle;
	SSL_CTX* m_sslContext;

public:
	SSLConnection()
		: m_sslHandle(nullptr), m_sslContext(nullptr)
	{}

	~SSLConnection();
	void connect(const std::string& address, uint16_t port = 443) override;
	void send(const std::string& message) override;
	std::string receive() override;
};

}

#endif //TLHTTP_CONNECTION_H
