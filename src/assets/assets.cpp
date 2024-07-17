#include "assets.h"
#include <stb_image.h>
#include "../core.h"

AssetId::AssetId(string id) {
	this->id = id;
	auto hasher = hash<string>();
	hashed_id = hasher(id);
}

GPUShader* ShaderImport::load_file(const char* path) {
	fprintf(stderr, "INFO: loading shader at path: %s\n", path);
	auto vert = utils::load_text(string(path).append(".vert").c_str());
	auto frag = utils::load_text(string(path).append(".frag").c_str());

	auto render_bd = App::get_render_backend();
	GPUShader* shader = render_bd->shaders.create();
	shader->compile_shader(vert.c_str(), frag.c_str());
	return shader;
}

GPUModel* ModelImport::load_file(const char* path) {
	fprintf(stderr, "INFO: loading model at path: %s\n", path);
	Assimp::Importer importer;
	auto scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		fprintf(stderr, "ERROR: loading model failed: %s\n", importer.GetErrorString());
		return nullptr;
	}

	GPUModel* model = App::get_render_backend()->models.create();
	process_ai_node(model, scene->mRootNode, scene);
	return model;
}

void ModelImport::process_ai_node(GPUModel* model, aiNode* node, const aiScene* scene) {
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		model->meshes.push_back(process_ai_mesh(mesh, scene));
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		process_ai_node(model, node->mChildren[i], scene);
	}
}

struct Vertex;
GPUMesh* ModelImport::process_ai_mesh(aiMesh* mesh, const aiScene* scene) {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	//vector<Texture> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;

		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;

		if (mesh->mTangents) {
			vertex.tangent.x = mesh->mTangents[i].x;
			vertex.tangent.y = mesh->mTangents[i].y;
			vertex.tangent.z = mesh->mTangents[i].z;
		}

		if (mesh->mTextureCoords[0]) {
			vertex.coords.x = mesh->mTextureCoords[0][i].x;
			vertex.coords.y = mesh->mTextureCoords[0][i].y;
		}
		else {
			vertex.coords = vec2(0.0f, 0.0f);
		}

		vertices.push_back(vertex);
	}
	// process indices
	for (size_t i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// process material
	if (mesh->mMaterialIndex >= 0) {
		//aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		//vector<Texture> diffuseMaps = loadMaterialTextures(material,
		//	aiTextureType_DIFFUSE, "texture_diffuse");
		//textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		//vector<Texture> specularMaps = loadMaterialTextures(material,
		//	aiTextureType_SPECULAR, "texture_specular");
		//textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	GPUMesh* gpu_mesh = App::get_render_backend()->meshes.create();
	gpu_mesh->set_vertices(vertices);
	gpu_mesh->set_triangles(indices);
	return gpu_mesh;
}

GPUTexture2D* Texture2DImport::load_file(const char* path) {
	fprintf(stderr, "INFO: Loading texture at: %s\n", path);
	int width, heigth, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path, &width, &heigth, &nrChannels, 0);
	if (!data) {
		fprintf(stderr, "ERROR: Failed to load texture at: %s\n %s\n", path, stbi_failure_reason());
		return nullptr;
	}

	GPUTexture2D* texture = App::get_render_backend()->textures.create();
	texture->set_as_rgb8(width, heigth, data);
	stbi_image_free(data);
	return texture;
}

GPUCubemapTexture* CubemapTextureImport::load_file(const char* path) {
	fprintf(stderr, "INFO: Loading cubemap at: %s\n", path);
	int width, heigth, nrChannels;
	vector<unsigned char*> cubemap_data;
	for (size_t i = 0; i < 6; i++) {
		auto face_path = string(path);
		std::replace(face_path.begin(), face_path.end(), '#', std::to_string(i).c_str()[0]);
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(face_path.c_str(), &width, &heigth, &nrChannels, 0);
		if (!data) {
			fprintf(stderr, "ERROR: Failed to load texture at: %s\n %s\n", face_path.c_str(), stbi_failure_reason());
			return nullptr;
		}
		fprintf(stderr, "\tLoading cubemap face at : %s\n", face_path.c_str());
		cubemap_data.push_back(data);
	}

	auto cubemap = App::get_render_backend()->cubemaps.create();
	cubemap->set_as_rgb8(width, heigth, cubemap_data);

	for (auto d : cubemap_data) {
		stbi_image_free(d);
	}

	return cubemap;
}


void AssetBackend::set_asset_folder(string path) {
	asset_path = path;
}

