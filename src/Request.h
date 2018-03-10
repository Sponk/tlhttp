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

#ifndef _REQUEST_H
#define _REQUEST_H

#include <unordered_map>
#include <string>
#include <sstream>

#include <iostream>
#include <algorithm>

namespace tlhttp
{

class Request
{
	std::unordered_map<std::string, std::string> m_headers;
	std::stringstream m_body;
	std::string m_url;
	std::string m_host;
	bool m_isPostRequest;
	
	uint16_t m_response;
public:

	Request() {}
	Request(const std::string& host, const std::string& url, bool isPost);
	
	/**
	 * @brief Retrieves a field from the header.
	 * @param key The field name to fetch.
	 * @return The field.
	 */
	std::string& operator[](const std::string& key) { return m_headers[key]; }

	/**
	 * @brief Builds a string from the headers.
	 * @note Does not include the body.
	 * @return The string representation of the HTTP header.
	 */
	std::string toString() const;
	
	/**
	 * @brief Returns the stream of the request body.
	 * @return The read-only request body stream
	 */
	const std::stringstream& getBody() const { return m_body; }
	
	/**
	 * @brief Returns the stream of the request body.
	 * @return The write enabled request body stream
	 */
	std::stringstream& getBody() { return m_body; }

	/**
	 * @brief Returns the request URL without the host.
	 * @return The URL.
	 */
	std::string getUrl() { return m_url; }

	void setResponse(uint16_t v) { m_response = v; }
	
	/**
	 * @brief Parses an HTTP request and builds an object out of it.
	 * @throws std::runtime_error when the header can not be parsed.
	 * @return The constructed Request.
	 */
	static Request parse(const std::string& str);
};

/**
 * @brief Writes the body of a request.
 * @param rq The request to write into.
 * @param str The string to append to the body.
 * @return The request reference for chaining.
 */
inline Request& operator<<(Request& rq, const std::string& str)
{
	rq.getBody() << str;
	return rq;
}
}
#endif
