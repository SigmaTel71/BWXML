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

#include <cstdlib>
#include <filesystem>
#include <iomanip>

#include "BWReaderPugi.h"
#include "Base64.h"
#include "BWCommon.h"

namespace BWPack
{
	using namespace BigWorld;

	BWXMLReaderPugi::BWXMLReaderPugi(const std::string& fname): mStream(fname)
	{
		uint32_t magic = mStream.get<uint32_t>();
		if (magic != PACKED_SECTION_MAGIC)
			throw std::runtime_error("Wrong header magic");

		uint8_t version = mStream.get<uint8_t>();
		if (version != 0)
			throw std::runtime_error("Unsupported file version");
		
		ReadStringTable();

		pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
		decl.append_attribute("version") = "1.0";
		decl.append_attribute("encoding") = "utf-8";

		mTree = doc.append_child("root");
		ReadSection(mTree);
	}
	
	void BWXMLReaderPugi::saveTo(const std::string& destname) const
	{
		doc.save_file(destname.c_str(), "\t", pugi::format_indent | pugi::format_save_file_text);
	}
	
	void BWXMLReaderPugi::ReadStringTable()
	{
		for (std::string tmp = mStream.getNullTerminatedString(); !tmp.empty(); tmp = mStream.getNullTerminatedString())
			mStrings.push_back(tmp);
	}
	
	void BWXMLReaderPugi::readData(BigWorld::DataDescriptor descr, pugi::xml_node& current_node, uint32_t prev_offset)
	{
		current_node.remove_children();
		uint32_t startPos = prev_offset, endPos = descr.offset();
		uint32_t var_size = endPos - startPos;
		assert(var_size >= 0);

		std::stringstream contentBuffer;
		switch (descr.typeId())
		{
		case BW_Section:
			ReadSection(current_node); //yay recursion!
			break;

		case BW_String:
			contentBuffer << mStream.getString(var_size);
			if (var_size > 0)
				current_node.text().set(contentBuffer.str());
			if (PackBuffer(contentBuffer.str()).type != BW_String)
				current_node.append_child(pugi::node_comment).set_value("BW_String");
			
			break;

		case BW_Int:
			switch (var_size)
			{
			case 8:
				current_node.text().set(mStream.get<int64_t>());
				break;
			case 4:
				current_node.text().set(mStream.get<int32_t>());
				break;
			case 2:
				current_node.text().set(mStream.get<int16_t>());
				break;
			case 1:
				current_node.text().set(mStream.get<int8_t>());
				break;
			case 0:
				current_node.text().set(0);
				break;
			default:
				throw std::runtime_error("Unsupported int size!");
			}
			break;

		case BW_Float:
			assert(var_size % sizeof(float) == 0);
			contentBuffer << std::fixed << std::setfill('\t');
			if (var_size / sizeof(float) == BW_MATRIX_SIZE) // we've got a matrix!
			{
				for (size_t i = 0; i < BW_MATRIX_NROWS; ++i)
				{
					for (size_t j = 0; j < BW_MATRIX_NCOLS; ++j)
					{
						if (!contentBuffer.str().empty())
							contentBuffer << " ";
						contentBuffer << mStream.get<float>();
					}
					current_node.append_child("row" + std::to_string(i)).set_value(contentBuffer.str());
					contentBuffer.str(""); // clearing our buffer
				}
				break;
			}
			// not a matrix, building a plain string
			for (size_t i = 0; i < (var_size / sizeof(float)); ++i)
			{
				if (!contentBuffer.str().empty())
					contentBuffer << " ";
				contentBuffer << mStream.get<float>();
			}
			current_node.text().set(contentBuffer.str());
			break;

		case BW_Bool:
			current_node.text().set(var_size != 0 ? true : false);
			mStream.getString(var_size);
			break;

		case BW_Blob:
			current_node.text().set(B64::Encode(mStream.getString(var_size)));
			break;

		case BW_Enc_blob:
			mStream.getString(var_size); // TBD?
			current_node.text().set("TYPE_ENCRYPTED_BLOB is (yet) unsupported!");
			std::cerr << "unsupported section TYPE_ENCRYPTED_BLOB!" << std::endl;
			break;

		default:
			throw std::runtime_error("Unsupported section type!");
		}
	}
	
	void BWXMLReaderPugi::ReadSection(pugi::xml_node& parent_node)
	{
		int nChildren = mStream.get<uint16_t>();
		DataDescriptor ownData = mStream.get<DataDescriptor>();

		std::vector<DataNode> children;
		children.reserve(nChildren);

		for (int i = 0; i < nChildren; ++i)
			children.push_back(mStream.get<DataNode>());

		readData(ownData, parent_node, 0);

		uint32_t prev_offset = ownData.offset();

		for (auto& child: children)
		{
			assert(child.nameIdx < mStrings.size());
			readData(child.data, parent_node.append_child(mStrings[child.nameIdx]), prev_offset);
			prev_offset = child.data.offset();
		}
	}
}