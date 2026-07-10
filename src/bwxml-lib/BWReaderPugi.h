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

#include <string>
#include <vector>

#include <pugixml.hpp>

#include "BW.hpp"
#include "DataStream.h"

namespace BWPack
{
	class BWXMLReaderPugi
	{
	public:
		BWXMLReaderPugi(const std::string& fname);
		void saveTo(const std::string& destname) const;

	protected:
		IO::StreamReader mStream;
		std::vector<std::string> mStrings;
		pugi::xml_node mTree;
		pugi::xml_document doc;

		void ReadStringTable();
		void readData(BigWorld::DataDescriptor descr, pugi::xml_node& current_node, uint32_t prev_offset);
		void ReadSection(pugi::xml_node& parent_node);
	};
}