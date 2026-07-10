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
#include "BWCommon.h"
#include "DataStream.h"

namespace BWPack
{
	class BWXMLWriterPugi
	{
	public:
		BWXMLWriterPugi(const std::string& fname);
		void saveTo(const std::string& destname);

	protected:
		std::vector<std::string> mStrings;
		pugi::xml_document mTree;
		typedef std::vector<BWPack::dataBlock> dataArray;

		void collectStrings();
		uint16_t resolveString(const std::string& str) const;
		void treeWalker(const pugi::xml_node& node);

		BigWorld::DataDescriptor BuildDescriptor(BWPack::rawDataBlock block, uint32_t prevOffset) const;
		BWPack::rawDataBlock serializeNode(const pugi::xml_node& nodeVal, bool simple) const;
		std::string serializeSection(const pugi::xml_node& section) const;
	};
}