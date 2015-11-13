#pragma once

#include "glm\common.hpp"
#include <string>
#include <vector>

struct VertexData
{
	glm::vec3 point;
	glm::vec2 texCoord;
	glm::vec3 normal;

	VertexData()
	{

	}

	VertexData(glm::vec3 point, glm::vec2 texCoord, glm::vec3 normal)
	{
		this->point = point;
		this->texCoord = texCoord;
		this->normal = normal;
	}
};

struct MeshMaterialData
{
	std::string Name;
	unsigned int Illum;
	float	Kd[3];
	float	Ka[3];
	float	Tf[3];
	std::string	map_Kd;
	std::string bump;
	std::string disp;
	std::string occlusion;
	float	Ni;

	MeshMaterialData()
	{
		Name = "none";
		map_Kd = "none";
		bump = "none";
		disp = "none";
		occlusion = "none";
	}
};

struct VertexGroup
{
	std::string name;
	std::vector<VertexData> vertices;
	MeshMaterialData materialData;
};

class Mesh
{
	/*
	GLuint bufferHandle[1];
	GLuint VAOHandle[1];
	bool buffersInitialized = false;
	*/
	std::vector<VertexGroup> groups;

	void InitBuffers();

public:
	//~Mesh();

	void Draw();

	void LoadMaterialData(std::string filepath, std::vector<MeshMaterialData> & data);
	void LoadFromObjFile(std::string dir, std::string filename);

	MeshMaterialData GetGroupMaterialData(int groupIndex) { return groups[groupIndex].materialData; };
	int GetVertexCount(int groupIndex) { return groups[groupIndex].vertices.size(); };
	int GetVertexDataSize(int groupIndex) { return groups[groupIndex].vertices.size() * sizeof(VertexData); };
	const VertexData * GetVertexData(int groupIndex) { return groups[groupIndex].vertices.data(); };
};