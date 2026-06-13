#include "LocationConfig.hpp"

LocationConfig::LocationConfig() {}

LocationConfig::LocationConfig(const LocationConfig& other) { *this = other; }

LocationConfig& LocationConfig::operator=(const LocationConfig& other) {
	if (this != &other) {
		// ...
	}
	return *this;
}

LocationConfig::~LocationConfig() {}