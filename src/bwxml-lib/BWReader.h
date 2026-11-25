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

#pragma once

#include <string>
#include <vector>

#include "DataStream.h"
#include "BW.hpp"
#include <pugixml.hpp>

namespace BWPack
{
	class BWXMLReader
	{
	public:
		BWXMLReader(const std::string& fname);
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
