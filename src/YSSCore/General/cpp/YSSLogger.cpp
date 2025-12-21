#include "General/YSSLogger.h"

namespace YSSCore::General {
	YSSLogger* YSSLogger::Instance = nullptr;
	YSSLogger::YSSLogger() : Visindigo::General::Logger("YSSLogger") {

	}
	YSSLogger* YSSLogger::getInstance() {
		if (Instance == nullptr) {
			Instance = new YSSLogger();
		}
		return Instance;
	}
}