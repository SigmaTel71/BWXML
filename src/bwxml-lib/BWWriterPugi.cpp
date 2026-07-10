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

#include <algorithm>
#include <array>
#include <optional>
#include "BWWriterPugi.h"

namespace BWPack
{
	using namespace BigWorld;

	BWXMLWriterPugi::BWXMLWriterPugi(const std::string& fname)
	{
		try
		{
			mTree.load_file(fname.c_str(), pugi::parse_default | pugi::parse_trim_pcdata | pugi::parse_comments);
		}
		catch (...)
		{
			throw std::runtime_error("XML parsing error");
		}

		if (std::distance(mTree.begin(), mTree.end()) != 1)
			throw std::runtime_error("XML file must contain only 1 root level node");
	}
	
	void BWXMLWriterPugi::saveTo(const std::string& destname)
	{
		collectStrings();

		std::stringstream outbuf;
		IO::StreamBufWriter outstream(outbuf.rdbuf());
		outstream.put(BigWorld::PACKED_SECTION_MAGIC);
		outstream.put<uint8_t>(0);
		for (const auto& nodeName: mStrings)
			outstream.putString(nodeName);
		outstream.put<uint8_t>(0);

		for (const auto& section : mTree)
			outstream.putString(serializeSection(section), false);

		std::ofstream mFile;
		mFile.open(destname, std::ios::binary);
		if (!mFile.is_open())
			throw std::runtime_error("Can't open the file");
		mFile << outbuf.rdbuf();
		mFile.close();
	}

	void BWXMLWriterPugi::collectStrings()
	{
		mStrings.clear();
		treeWalker(mTree.first_child());
		std::sort(mStrings.begin(), mStrings.end());
		mStrings.erase(std::unique(mStrings.begin(), mStrings.end()), mStrings.end());

		static std::array<std::string, 4> namesToClear = { "row0", "row1", "row2", "row3" };

		for (const auto& name : namesToClear)
		{
			auto idx = std::find(mStrings.begin(), mStrings.end(), name);

			if (idx != mStrings.end())
				mStrings.erase(idx);
		}
	}
	
	uint16_t BWXMLWriterPugi::resolveString(const std::string& str) const
	{
		auto pos = std::find(mStrings.begin(), mStrings.end(), str);
		if (pos == mStrings.end())
			throw std::runtime_error("String key not found!");
		
		size_t idx = std::distance(mStrings.begin(), pos);
		if (idx > std::numeric_limits<uint16_t>::max())
			throw std::runtime_error("String table overflow!");

		return static_cast<uint16_t>(idx);
	}
	
	void BWXMLWriterPugi::treeWalker(const pugi::xml_node& node)
	{
		if (node.type() == pugi::node_element && node.parent() != node.root())
			mStrings.push_back(node.name());

		for (auto& subNode: node)
		{
			if (subNode.type() == pugi::node_element)
				mStrings.push_back(subNode.name());

			for (const auto& child: subNode.children())
				treeWalker(child);
		}	
	}
	
	BigWorld::DataDescriptor BWXMLWriterPugi::BuildDescriptor(BWPack::rawDataBlock block, uint32_t prevOffset) const
	{
		if (block.data.length() > std::numeric_limits<uint32_t>::max())
			throw std::runtime_error("Data block is too large");

		return DataDescriptor(block.type, prevOffset + static_cast<uint32_t>(block.data.length()));
	}
	
	BWPack::rawDataBlock BWXMLWriterPugi::serializeNode(const pugi::xml_node& nodeVal, bool simple) const
	{
		size_t nodeValSize = std::distance(nodeVal.children().begin(), nodeVal.children().end());
		if (nodeValSize == BW_MATRIX_NROWS) // maybe that's a matrix?..
		{
			std::vector<pugi::xml_node> rows;
			for (int i = 0; i < BW_MATRIX_NROWS; ++i)
			{
				auto row = nodeVal.child("row" + std::to_string(i));
				if (!row) // bad luck.
					break;
				rows.emplace_back(row);
			}
			
			if (rows.size() == BW_MATRIX_NROWS) // we've found all 4 required rows
			{
				std::stringstream buffer;
				for (const auto row: rows)
				{
					rawDataBlock block = PackBuffer(row.child_value());
					assert(block.type == BW_Float);	// better safe than sorry
					buffer << block.data;
				}

				return rawDataBlock(BW_Float, buffer.str());
			}
		}

		bool forceStringSerialization = nodeVal.find_child([](pugi::xml_node node) {
			return node.type() == pugi::node_comment && std::string_view(node.value()) == "BW_String";
		});

		auto serializeString = [&](const pugi::xml_node& node)
		{
			if (forceStringSerialization)
				return rawDataBlock(BW_String, nodeVal.child_value());

			return PackBuffer(nodeVal.child_value());
		};

		if (!simple && nodeValSize && !forceStringSerialization) // has sub-nodes
		{
			for (const auto& child : nodeVal)
			{
				if (child.type() == pugi::node_element)
					return rawDataBlock(BW_Section, serializeSection(nodeVal));
				else if (child.type() == pugi::node_pcdata)
					return serializeString(nodeVal);
			}
		}

		return serializeString(nodeVal);
	}
	
	std::string BWXMLWriterPugi::serializeSection(const pugi::xml_node& section) const
	{
		std::stringstream _ret;
		IO::StreamBufWriter ret(_ret.rdbuf());

		rawDataBlock ownData = serializeNode(section, true); // getting own plain content
		dataArray childData;
		for (const auto& child: section)
		{
			std::string resolvableString;

			switch (child.type())
			{
			case pugi::node_comment: // skipping comments
				continue;
			case pugi::node_element:
				resolvableString = child.name();
				childData.push_back(dataBlock(resolveString(resolvableString), serializeNode(child, false)));
				break;
			default:
				__debugbreak();
			}
		}

		if (childData.size() > std::numeric_limits<uint16_t>::max())
			throw std::runtime_error("Too many children nodes!");

		DataDescriptor ownDescriptor = BuildDescriptor(ownData, 0);
		ret.put<uint16_t>(static_cast<uint16_t>(childData.size()));
		ret.put<DataDescriptor>(ownDescriptor);

		uint32_t currentOffset = ownDescriptor.offset();
		for (auto it = childData.begin(); it != childData.end(); ++it)
		{
			DataNode bwNode;
			bwNode.nameIdx = it->stringId;
			bwNode.data = BuildDescriptor(it->data, currentOffset);
			ret.put<DataNode>(bwNode);
			currentOffset = bwNode.data.offset();
		}

		ret.putString(ownData.data, false);
		for (auto it = childData.begin(); it != childData.end(); ++it)
		{
			ret.putString(it->data.data, false);
		}

		return _ret.str();
	}
}