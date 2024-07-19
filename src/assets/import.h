#pragma once
#include <typeindex>
#include <stb_image.h>
#include "../rendering/renderer.h"
#include "../venum.h"


class ImportError {
	std::string error;
};

class BaseFileImport {
public:
	virtual type_index file_type() = 0;
	virtual Result<void*, ImportError> raw_load_file(const char* path) = 0;
};

template <typename T>
class FileImport : public BaseFileImport {
public:
	type_index file_type() override { return typeid(T); }
	virtual Result<void*, ImportError> raw_load_file(const char* path) { return load_file(path); }
	virtual Result<T*, ImportError> load_file(const char* path) = 0;
};

class GPUShaderImport : public FileImport<GPUShader> {
public:
	Result<GPUShader*, ImportError> load_file(const char* path) override;
};

class GPUModelImport : public FileImport<GPUModel> {
public:
	Result<GPUModel*, ImportError> load_file(const char* path) override;
	void process_ai_node(GPUModel* model, aiNode* node, const aiScene* scene);
	GPUMesh* process_ai_mesh(aiMesh* mesh, const aiScene* scene);
};

class GPUTexture2DImport : public FileImport<GPUTexture2D> {
public:
	Result<GPUTexture2D*, ImportError> load_file(const char* path) override;
};

class GPUCubemapTextureImport : public FileImport<GPUCubemapTexture> {
public:
	Result<GPUCubemapTexture*, ImportError> load_file(const char* path) override;
};
