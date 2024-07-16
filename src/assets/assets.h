#pragma once

#include <string>

using namespace std;

class Asset {
	string path;

	void serialize();
	void deserialize();
};

// Add importers here.

class AssetBackend {
	// Map of assets with IDs	
};
