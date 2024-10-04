// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include <iostream>
#include <clarisma/http/Httpclient.h>

using namespace clarisma;

int main(int argc, char* argv[])
{
	HttpClient client("httpbin.org");
	HttpResponse res = client.get("/get");
	for(;;)
	{
		char buf[1024];
		size_t read = res.read(buf, sizeof(buf));
		if(!read) break;
		std::cout << std::string_view(buf, read);
	}
	std::cout << std::endl;
}
