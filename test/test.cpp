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

#include <gtest/gtest.h>
#include "../src/Connection.h"
#include "../src/Server.h"
#include "../src/Request.h"

/*
TEST(test, test)
{
	tlhttp::Connection connection;
	connection.connect("127.0.0.1", 8000);

	const tlhttp::Request result = connection.get("/index.html", "");
	EXPECT_FALSE(result.getBody().str().empty());
}

TEST(test, sslTest)
{
	tlhttp::SSLConnection connection;
	connection.connect("example.com", 443);

	const tlhttp::Request result = connection.get("/index.html", "");
	std::cout << result.toString() << std::endl;
	EXPECT_FALSE(result.getBody().str().empty());
}*/

/*TEST(test, connect)
{
	tlhttp::Server server("0.0.0.0", 8080);
	server.start([](tlhttp::Connection& connection) {

		connection.send("Hello World!\n");
		return true;
	});
}*/

const char* testHeader = 	"GET  HTTP/1.1\r\n"
				"Content-Length: 1270\r\n"
				"Vary: Accept-Encoding\r\n"
				"Content-Type: text/html\r\n"
				"Connection: close\r\n"
				"Accept-Ranges: bytes\r\n\r\nBODY";

// Corrupt header but right header end
const char* testCorrupt0 = "alskdjflkasjdalösdjfqpowetoipudfsglökjasdflkjqweropiudasfglkjasdf,nlköjasdfpoiuj;;::asDF:asdf:Asdf:ASfd:ASfd:Asdf:asdf:\n\nasdf\r\r\r\n\r\n";

// Corrupt header
const char* testCorrupt1 = "alskdjflkasjdalösdjfqpowetoipudfsglökjasdflkjq";

// Corrupt header
const char* testCorrupt2 = "\r\n\r\n";


TEST(Header, SimpleParse)
{
	auto req = tlhttp::Request::parse(testHeader);
	EXPECT_EQ("1270", req["Content-Length"]);
	EXPECT_EQ("Accept-Encoding", req["Vary"]);
	EXPECT_EQ("text/html", req["Content-Type"]);
	EXPECT_EQ("close", req["Connection"]);
	EXPECT_EQ("bytes", req["Accept-Ranges"]);
	EXPECT_EQ("BODY", req.getBody().str());
}

TEST(Header, SimpleSerialize)
{
	auto req = tlhttp::Request::parse(testHeader);
	req["Content-Length"] = "1270";
	req["Vary"] = "Accept-Encoding";
	req["Content-Type"] = "text/html";
	req["Connection"] = "close";
	req["Accept-Ranges"] = "bytes";
	
	auto ref = tlhttp::Request::parse(testHeader);
	EXPECT_EQ(ref["Content-Length"], req["Content-Length"]);
	EXPECT_EQ(ref["Vary"], req["Vary"]);
	EXPECT_EQ(ref["Content-Type"], req["Content-Type"]);
	EXPECT_EQ(ref["Connection"], req["Connection"]);
	EXPECT_EQ(ref["Accept-Ranges"], req["Accept-Ranges"]);
}

TEST(Header, Corrupt0)
{
	auto req = tlhttp::Request::parse(testCorrupt0);
}

TEST(Header, Corrupt1)
{
	EXPECT_ANY_THROW(tlhttp::Request::parse(testCorrupt1));
}

TEST(Header, Corrupt2)
{
	auto req = tlhttp::Request::parse(testCorrupt2);
}
