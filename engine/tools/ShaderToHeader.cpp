/*
 * ShaderToHeader.cpp
 *
 *  Created on: Jul 14, 2024
 *      Author: irrl
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <algorithm>

// This is an utility app for converting shader files to header files as an array.
// Similar to xxd, but it's compiled alongside the engine so it work on all platforms.
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " <input shader file> <output header file>" << std::endl;
        return 1;
    }

    std::ifstream shader_file(argv[1], std::ios::binary | std::ios::ate);
    if (!shader_file.is_open()) {
        std::cerr << "error: could not open shader file " << argv[1] << std::endl;
        return 2;
    }

    std::ofstream header_file(argv[2]);
    if (!header_file.is_open()) {
        std::cerr << "error: could not open header file " << argv[2] << std::endl;
        return 3;
    }

    // Get the filename without extension for the variable name
    std::string file_name = argv[1];
    size_t last_slash = file_name.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        file_name = file_name.substr(last_slash + 1);
    }
    size_t last_dot = file_name.find_last_of(".");
    if (last_dot != std::string::npos) {
        file_name = file_name.substr(0, last_dot);
    }

	size_t size = shader_file.tellg();
	shader_file.clear(); // Clear EOF flag to read file again
	shader_file.seekg(0, std::ios::beg);  // Rewind to the beginning of the file

    // Write the header file

	std::string include_guard = "GLIST_GENERATED_" + file_name;
	std::transform(include_guard.begin(), include_guard.end(), include_guard.begin(),
			  ::toupper);
	header_file << "#ifndef " << include_guard << "\n";
	header_file << "#define " << include_guard << "\n\n";
    header_file << "#include <array>\n\n";
    header_file << "constexpr std::array<const char, " << size + 1 << "> shader_" << file_name << " = { ";

    std::string line;
    while (std::getline(shader_file, line)) {
        for (char c : line) {
            header_file << "0x" << std::hex << std::uppercase << static_cast<uint32_t>(c) << ", ";
        }
        header_file << "0x" << std::hex << std::uppercase << static_cast<uint32_t>('\n') << ", ";
    }

    header_file << " };\n\n";
	header_file << "#endif // " << include_guard;

	std::cout << "generated header file " << argv[2] << std::endl;

    return 0;
}
