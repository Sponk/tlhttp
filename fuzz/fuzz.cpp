#include <cstdint>
#include <string>
#include <Request.h>

using namespace tlhttp;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, std::size_t size)
{
	std::string str((const char*) data, size);

	try
	{
		Request r = Request::parse(str);
	}catch(...) {}

	return 0;
}
