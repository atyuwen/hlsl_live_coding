#include "common.hpp"
#include "shader_header.hpp"

#include <fstream>
#include <vector>
#include <algorithm>

namespace ShaderHeader
{
	static const tstring headers[] =
	{
		TEXT("snoise.hlsl"),
	};

	static tstring g_shader_header;
	static size_t g_num_lines;

	void InitShaderHeader()
	{
		for (int i = 0;  i != ARRAYSIZE(headers); ++i)
		{
			tstring file_path = TEXT("fx/") + headers[i];
			std::ifstream ifs(file_path.c_str());
			if (!ifs.is_open()) return;

			ifs.seekg(0, std::ios::end);
			int length = static_cast<int>(ifs.tellg());

			std::vector<tchar> buffer(length + 1);
			ifs.seekg(0, std::ios::beg);
			ifs.read(&buffer[0], length);
			ifs.close();

			g_shader_header.append(&buffer[0]);
		}

		g_shader_header.append(1, '\n');
		g_num_lines = std::count_if(g_shader_header.begin(), g_shader_header.end(),
			[](tchar c){return c == '\n';});
	}

	const tstring& GetHeaderText()
	{
		return g_shader_header;
	}

	size_t GetHeaderLines()
	{
		return g_num_lines;
	}
}
