#ifndef SKELETONDESIGN_H
#define SKELETONDESIGN_H
#include<iostream>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<vector>
#include<stdlib.h>
#include "tinyxml2.h"

#define MAX_SKIN_LABEL_SIZE 50
using namespace tinyxml2;


//global variable declaration

//Geometry Information
struct VertexInfo
{
	int stride;
	int size;
	std::vector<std::vector<float>> vec_vertex;
	std::vector<float> vec_vertex_all;
};

struct NormalInformation
{
	int stride;
	int size;
	std::vector<std::vector<float>> vec_normal;
};

struct TexcoordInformation
{
	int stride;
	int size;
	std::vector<std::vector<float>> vec_texcoord;
};

struct ColorInformation
{
	int stride;
	int size;
	std::vector<std::vector<float>> vec_color;
};

struct MeshInformation
{
	struct VertexInfo vertexInfo;
	struct NormalInformation normalInfo;
	struct TexcoordInformation texcoordInfo;
	struct ColorInformation colorInfo;
};

//Joint Information - Skeleton Info
struct Joint
{
	std::vector<Joint> vec_children;
	char *id;
	std::vector<int> vec_transform_matrix; //4*4 matrix
};

//Animation Data
//source - Armature_Torso_pose_matrix-input
struct TorsoPoseMatrixInput
{
	int stride;
	std::vector<float> vec_post_matrix_input_data;
};

//source - Armature_Torso_pose_matrix-output
struct TorsoPoseMatrixOutput
{
	int stride;
	std::vector<float> vec_post_matrix_output_data;
};

struct Animation
{
	char *id;
	struct TorsoPoseMatrixInput matrix_input;
	struct TorsoPoseMatrixOutput matrix_output;
};

//skin data
struct SkinJoint
{
	int index;
	char name[MAX_SKIN_LABEL_SIZE];
};

struct VertexSkinData
{
	int vertexIndex;
	std::vector<int> jointIds;
	std::vector<float> weights;
};

struct SkinningInformation
{
	std::vector<struct VertexSkinData> vec_vertex_skin_data;
};
struct SkinInformation
{
std::vector<struct SkinJoint> vec_skin_joints; //joint vector
std::vector<float> vec_skin_bind_poses; //bind poses
std::vector<float> vec_skin_weights; //weights
std::vector<int> vec_vertex_weight_vcount; //vcount
std::vector<int> vec_vertex_weight_v; //vertex
};

void getGeometryData(XMLNode *);
void getSkinData(XMLElement *); //raw data
void getVertexSkinningData(void);
struct Joint getJointData(XMLNode *);
void getAnimationData(XMLNode *);

struct ColladaData
{
	struct MeshInformation meshData;
	struct Joint skeletonData;
	struct SkinningInformation skinningInfo;
	struct SkinInformation skinData; //raw data
	std::vector<struct Animation> vec_animation; //to store all animation data
};
struct ColladaData loadDaeData(char *);
#endif