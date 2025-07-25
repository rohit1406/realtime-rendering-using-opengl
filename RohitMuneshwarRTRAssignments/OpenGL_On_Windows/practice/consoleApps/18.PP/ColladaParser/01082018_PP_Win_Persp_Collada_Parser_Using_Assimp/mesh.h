#pragma once

#include <iostream>

#include "Importer.hpp"

#include <vector>
#include <string>

using namespace std;

class Mesh
{
public:
	Mesh();

	~Mesh();

	bool LoadMesh(const std::string& Filename);

	void Render();

private:
	bool InitFromScene(const aiScene* pScene, const std::string& Filename);
	void InitMesh(unsigned int Index, const aiMesh* paiMesh);
	bool InitMaterials(const aiScene* pScene, const std::string& Filename);
	void Clear();

#define INVALID_MATERIAL 0xFFFFFFFF

	struct MeshEntry {
		MeshEntry();

		~MeshEntry();

		bool Init(const std::vector& Vertices,
			const std::vector& Indices);

		GLuint VB;
		GLuint IB;
		unsigned int NumIndices;
		unsigned int MaterialIndex;
	};

	std::vector m_Entries;
	std::vector m_Textures;
};