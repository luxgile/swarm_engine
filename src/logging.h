#pragma once
#include <string>
#include <format>
#include <print>
#include <iostream>
#include <sstream>
#include <streambuf>

enum LogType {
	LogVerbose,
	LogInfo,
	LogWarning,
	LogError,
	LogCritical,
};

class ConsoleLog {
	LogType type;
	std::string log;

	const char* type_to_string() const;

public:
	ConsoleLog() = delete;
	ConsoleLog(LogType type, std::string log) : type(type), log(log) {}

	operator std::string() const {
		return to_str();
	}

	const std::string to_str() const {
		return type_to_string() + log;
	}

	friend std::ostream& operator <<(std::ostream& out, const ConsoleLog& obj) {
		return out << obj.to_str();
	}
};

class Console {
private:
	std::ostringstream buffer;

	Console() {}
	static Console& get_instance() {
		static Console console;
		return console;
	}
public:
	Console(const Console&) = delete;
	Console& operator=(const Console&) = delete;

	static std::ostringstream* get_ouput_stream() { return &get_instance().buffer; }

	static void log_verbose(const char* log) { Console::log(ConsoleLog(LogType::LogVerbose, log)); }
	template <class... _Types>
	static void log_verbose(const std::format_string<_Types...> fmt, _Types&&... args) { Console::log(ConsoleLog(LogType::LogVerbose, std::format(fmt, std::forward<_Types>(args)...))); }
	static void log_info(const char* log) { Console::log(ConsoleLog(LogType::LogInfo, log)); }
	template <class... _Types>
	static void log_info(const std::format_string<_Types...> fmt, _Types&&... args) { Console::log(ConsoleLog(LogType::LogInfo, std::format(fmt, std::forward<_Types>(args)...))); }
	static void log_warning(const char* log) { Console::log(ConsoleLog(LogType::LogWarning, log)); }
	template <class... _Types>
	static void log_warning(const std::format_string<_Types...> fmt, _Types&&... args) { Console::log(ConsoleLog(LogType::LogWarning, std::format(fmt, std::forward<_Types>(args)...))); }
	static void log_error(const char* log) { Console::log(ConsoleLog(LogType::LogError, log)); }
	template <class... _Types>
	static void log_error(const std::format_string<_Types...> fmt, _Types&&... args) { Console::log(ConsoleLog(LogType::LogError, std::format(fmt, std::forward<_Types>(args)...))); }
	static void log_critical(const char* log) { Console::log(ConsoleLog(LogType::LogCritical, log)); }
	template <class... _Types>
	static void log_critical(const std::format_string<_Types...> fmt, _Types&&... args) { Console::log(ConsoleLog(LogType::LogCritical, std::format(fmt, std::forward<_Types>(args)...))); }
	static void log(ConsoleLog log) { Console::log("{}", log.to_str()); }

	template <class... _Types>
	static void log(const std::format_string<_Types...> fmt, _Types&&... args) {
		std::println(get_instance().buffer, fmt, std::forward<_Types>(args)...);
		std::println(std::cout, fmt, std::forward<_Types>(args)...);
	}
};
