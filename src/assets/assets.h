#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include "../rendering/renderer.h"
#include <typeindex>

using namespace std;

class AssetId {
	string id;
	size_t hashed_id;
public:
	AssetId(string id);
	const string get_id() { return id; }
	const size_t get_hash() { return hashed_id; }
};

class Asset {
	AssetId id;
	string path;

public:
	void serialize();
	void deserialize();

	AssetId get_id() { return id; }
};

class BaseFileImport {
public:
	virtual type_index file_type() = 0;
	virtual void* raw_load_file(const char* path) = 0;
};
template <typename T>
class FileImport : public BaseFileImport {
public:
	type_index file_type() override { return typeid(T); }
	virtual void* raw_load_file(const char* path) { return load_file(path); }
	virtual T* load_file(const char* path) = 0;
};
class ShaderImport : public FileImport<GPUShader> {
public:
	GPUShader* load_file(const char* path) override;
};
class ModelImport : public FileImport<GPUModel> {
public:
	GPUModel* load_file(const char* path) override;
	void process_ai_node(GPUModel* model, aiNode* node, const aiScene* scene);
	GPUMesh* process_ai_mesh(aiMesh* mesh, const aiScene* scene);
};
class Texture2DImport : public FileImport<GPUTexture2D> {
public:
	GPUTexture2D* load_file(const char* path) override;
};
class CubemapTextureImport : public FileImport<GPUCubemapTexture> {
public:
	GPUCubemapTexture* load_file(const char* path) override;
};

class AssetBackend {
	string asset_path;
	map<type_index, BaseFileImport*> importers;
	map<size_t, Asset> assets;

public:
	void set_asset_folder(string path);

	template<typename T>
	void register_importer();

	template<typename T>
	T* load_file(const char* path);
	template<typename T>
	T* load_file(string path) { return load_file<T>(path.c_str()); }

};

template<typename T>
inline void AssetBackend::register_importer() {
	static_assert(is_base_of<BaseFileImport, T>(), "T must be a FileImporter");
	T* importer = new T();
	importers[importer->file_type()] = static_cast<BaseFileImport*>(importer);
}

template<typename T>
inline T* AssetBackend::load_file(const char* path) {
	auto it = importers.find(typeid(T));
	if (it == importers.end()) {
		fprintf(stderr, "ERROR: No importer registered for file: %s\n", path);
		return nullptr;
	}
	auto importer = importers[typeid(T)];
	string abs_path = asset_path + path;
	return static_cast<T*>(importer->raw_load_file(abs_path.c_str()));
}
