#include "assets.h"

#include "../core.h"

AssetId::AssetId(string id) {
	this->id = id;
	auto hasher = hash<string>();
	hashed_id = hasher(id);
}

void AssetBackend::set_asset_folder(string path) {
	asset_path = path;
}

