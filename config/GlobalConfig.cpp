#include "GlobalConfig.hpp"

GlobalConfig::GlobalConfig() {}

GlobalConfig::GlobalConfig(const GlobalConfig& other) { *this = other; }

GlobalConfig& GlobalConfig::operator=(const GlobalConfig& other) {
	if (this != &other) {
		// ...
	}
	return *this;
}

GlobalConfig::~GlobalConfig() {}