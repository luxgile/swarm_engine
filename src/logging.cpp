#include "logging.h"

const char* ConsoleLog::type_to_string() const {
	switch (type) {
	case LogVerbose:
		return "[VERBOSE] ";
	case LogInfo:
		return "[INFO] ";
	case LogWarning:
		return "[WARNING] ";
	case LogError:
		return "[ERROR] ";
	case LogCritical:
		return "[CRITICAL] ";
	}
}
