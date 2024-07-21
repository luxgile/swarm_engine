#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include "../rendering/renderer.h"
#include <typeindex>
#include "../core.h"
#include "import.h"

class App;

class AssetId {
	std::string id;
	size_t hashed_id;
public:
	AssetId(std::string id);
	const std::string get_id() { return id; }
	const size_t get_hash() { return hashed_id; }

	bool operator ==(AssetId* id) const { return id->hashed_id == hashed_id; }
};

class Asset {
	AssetId id;
	std::string path;

public:
	void serialize();
	void deserialize();

	AssetId get_id() { return id; }
};

enum RefType {
	Id,
	Path,
};
template<typename T>
class AssetRef {
	AssetId id;
	std::string path;
	RefType type;
public:
	AssetRef(RefType type, std::string asset);
	const T* get() const {
		switch (type) {
		case Id:
			return nullptr;
		case Path:
			return App::get_asset_backend()->load_file(path);
		}
	}

	T* get_mut() const {
		switch (type) {
		case Id:
			return nullptr;
		case Path:
			return App::get_asset_backend()->load_file(path);
		}
	}
};

class AssetBackend {
	std::string asset_path;
	std::map<std::type_index, BaseFileImport*> importers;
	std::map<size_t, Asset> assets;

public:
	void set_asset_folder(std::string path);

	template<typename T>
	void register_importer();

	template<typename T>
	Result<T*, ImportError> load_file(const char* path);
	template<typename T>
	Result<T*, ImportError> load_file(std::string path) { return load_file<T>(path.c_str()); }
};

template<typename T>
inline void AssetBackend::register_importer() {
	static_assert(std::is_base_of<BaseFileImport, T>(), "T must be a FileImporter");
	T* importer = new T();
	importers[importer->file_type()] = static_cast<BaseFileImport*>(importer);
}

template<typename T>
inline Result<T*, ImportError> AssetBackend::load_file(const char* path) {
	auto it = importers.find(typeid(T));
	if (it == importers.end()) {
		fprintf(stderr, "ERROR: No importer registered for file: %s\n", path);
		return nullptr;
	}
	auto importer = importers[typeid(T)];
	std::string abs_path = asset_path + path;
	auto file = importer->raw_load_file(abs_path.c_str());
	if (!file) {
		return Error(file.error());
	}

	return static_cast<T*>(file.value());
}

template<typename T>
inline AssetRef<T>::AssetRef(RefType type, std::string asset) {
	this->type = type;
	switch (type) {
	case Id:
		id = AssetId(asset);
		fprintf(stderr, "ERROR: Asset references using ID are not supported yet.");
		break;
	case Path:
		path = asset;
		break;
	}
}

