/*
Copyright 2026 Vitaly Orekhov

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <base64.hpp>

namespace B64
{
	std::string Encode(std::string& src)
	{
		return base64::to_base64(src);
	}

	std::string Decode(std::string& src)
	{
		return base64::from_base64(src);
	}

	bool Is(std::string& src)
	{
		try
		{
			std::string tmp = Decode(src);
			bool ret = !Encode(tmp).compare(src);
			return (ret);
		}
		catch (...)
		{
			return false;
		}
	}
}