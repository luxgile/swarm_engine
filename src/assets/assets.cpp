#include "assets.h"

#include "../core.h"

AssetId::AssetId(std::string id) {
	this->id = id;
	auto hasher = std::hash<std::string>();
	hashed_id = hasher(id);
}

void AssetBackend::set_asset_folder(std::string path) {
	asset_path = path;
}

