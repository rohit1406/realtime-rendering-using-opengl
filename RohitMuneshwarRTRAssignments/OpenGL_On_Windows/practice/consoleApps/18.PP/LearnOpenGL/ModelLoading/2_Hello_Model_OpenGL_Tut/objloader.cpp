#include<windows.h>
#include<stdio.h> //for file IO
#include<stdlib.h>
#include "vmath.h"
#include<vector>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

using namespace vmath;
//load the obj file "path", write the data in out_vertices, out_uvs, out_normals adn return
//false if something went wrong
bool loadOBJ(const char* path, //path of the obj file
	std::vector <vec3> &out_vertices, //to store vertices data
	std::vector <vec2> &out_uvs, //to store texture data
	std::vector <vec3> &out_normals //to store normals data
)
{
	//soem temporary variables in which we will store the contents of the obj file
	std::vector<vec3> temp_vertices;
	std::vector<vec2> temp_uvs;
	std::vector<vec3> temp_normals;
	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;

	//open the file first
	FILE* file = fopen(path, //path of the file
		"r" //open the file in read-only mode
	);

	//check if file is opened or not
	if (file == NULL)
	{
		printf("Unable to open the file");
		return false;
	}

	//read the file until the end
	while (1)
	{
		char lineHeader[128]; //we assumed here that the first word of a line won't be longer than 128

							  //read the first word of the line
		int res = fscanf(file, //which file to scan
			"%s",
			lineHeader //read data in lineHeader
		);

		//check if we have reached the end of the file
		if (res == EOF)
		{
			break;
		}

		//deal with the vertices list
		if (strcmp(lineHeader, "v") == 0)
		{
			vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex[0], &vertex[1], &vertex[2]);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) //if vt then
		{
			vec2 uv;
			fscanf(file, "%f %f\n", &uv[0], &uv[1]);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) //if vn then
		{
			vec3 normal;
			fscanf(file, "%f %f %f\n", &normal[0], &normal[1], &normal[2]);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) //if f then
		{
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[2], normalIndex[3];

			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
				&vertexIndex[0], &uvIndex[0], &normalIndex[0],
				&vertexIndex[1], &uvIndex[1], &normalIndex[1],
				&vertexIndex[2], &uvIndex[2], &normalIndex[2]);

			if (matches != 9)
			{
				printf("File can't be read by our simple parser: Try exporting with other options\n");
				return 0;
			}

			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}

	}

	//for each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++)
	{
		unsigned int vertexIndex = vertexIndices[i];
		vec3 vertex = temp_vertices[vertexIndex - 1]; //indexing starts at 0 and obj indexing starts at 1
		out_vertices.push_back(vertex);
	}

	//for each tex coords of each triangle
	for (unsigned int i = 0; i < uvIndices.size(); i++)
	{
		unsigned int uvIndex = uvIndices[i];
		vec2 uv = temp_uvs[uvIndex - 1];
		out_uvs.push_back(uv);
	}

	//for each normal of each triangle
	for (unsigned int i = 0; i < normalIndices.size(); i++)
	{
		unsigned int normalIndex = normalIndices[i];
		vec3 normal = temp_normals[normalIndex - 1]; //indexing starts at 0 and obj indexing starts at 1
		out_normals.push_back(normal);
	}
	return true;
}