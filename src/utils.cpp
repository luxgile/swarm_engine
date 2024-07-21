#include "utils.h"
#include "logging.h"

const std::string utils::load_text(const char* path) {
	std::string text;
	std::ifstream file;

	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		file.open(path);
		std::stringstream stream;
		stream << file.rdbuf();
		file.close();

		text = stream.str();
	}
	catch (std::ifstream::failure e) {
		Console::log_error("File at {} not succesfully read.", path);
	}

	return text;
}
