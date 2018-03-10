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

#include "Request.h"

using namespace tlhttp;

Request::Request(const std::string& host, const std::string& url, bool isPost)
		: m_url(url), m_host(host), m_isPostRequest(isPost)
{
	m_headers["Host"] = host;
	m_headers["User-Agent"] = "TinyLittleHTTP";
	m_headers["Accept"] = "text/html";
	m_headers["Connection"] = "close";
}

std::string Request::toString() const
{
	std::stringstream ss;
	
	if(m_response == 0)
		ss << (m_isPostRequest ? "POST " : "GET ") << m_url << " HTTP/1.1" << "\r\n";
	else
		ss << "HTTP/1.1 " << m_response << "OK\r\n";
	
	for (auto k : m_headers)
	{
		ss << k.first << ": " << k.second << "\r\n";
	}

	if(m_isPostRequest)
	{
		if (!m_body.str().empty())
		{
			ss << "Content-Length: " << m_body.str().size() << "\r\n";
			ss << "\r\n";
			ss << m_body.str();
		}
		else
			ss << "\r\n";
	}

	ss << "\r\n";
	return ss.str();
}

Request Request::parse(const std::string& str)
{
	Request ret;

	size_t headerEnd = str.find("\r\n\r\n");
	if(headerEnd == std::string::npos)
		throw std::runtime_error("Given string has no valid HTTP header!");

	ret.getBody() << str.substr(headerEnd + 4);

	std::string header = str;
	header.erase(headerEnd);
	header.erase(std::remove(header.begin(), header.end(), '\r'), header.end());

	std::stringstream ss(header);
	std::string key, value, version;

	std::getline(ss, version, '\n');
	
	{
		size_t urlstart = version.find(" ");
		size_t urlend = version.find(" ", urlstart + 1);
		if(urlstart == std::string::npos || urlend == std::string::npos)
			throw std::runtime_error("Invalid HTTP header: URL is invalid!");
		
		++urlstart;
		ret.m_url = version.substr(urlstart, urlend - urlstart);
	}
	while(ss)
	{
		std::getline(ss, key, ':');
		std::getline(ss, value, '\n');

		if(value[0] == ' ')
			value = value.substr(1);

		ret.m_headers[key] = value;
	}

	return ret;
}
