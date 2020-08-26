#include <fstream>

static std::vector<char> readBinaryFile(const std::string& filename)
{
	// start at end, read as binary
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		std::string errorMessage {"Failed to open file: " + filename};
		throw std::runtime_error(errorMessage);
	}

	size_t fileSize			{static_cast<size_t>(file.tellg())};
	std::vector<char> buffer(fileSize);

	// go to beginning of file
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

