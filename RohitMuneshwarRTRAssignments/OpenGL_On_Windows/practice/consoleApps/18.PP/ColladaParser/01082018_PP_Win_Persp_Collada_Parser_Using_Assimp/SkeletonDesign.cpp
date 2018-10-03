#include "SkeletonDesign.h"

//global variables
struct ColladaData collada;
char *data; //for data
char *attrib_token; //for tokenization

const char *space_seperator = " ";
char *first_token = NULL;
/*int main(void) {
	//function prototypes declaration

	char *fileName = "colladaFiles/Ball.dae";
	struct ColladaData colladaFileData = loadDaeData(fileName);
	//printf("main>> %d %f\n",collada.meshData.vertexInfo.vec_vertex.size(),
		//collada.meshData.vertexInfo.vec_vertex[0]);
	//for (int i = 0; i < collada.meshData.vertexInfo.vec_vertex.size(); i++)
	//{
		//printf("v data > %f\n", collada.meshData.vertexInfo.vec_vertex[i]);
	//}
	printf("done");
	return EXIT_SUCCESS;
}*/

struct ColladaData loadDaeData(char *fileName)
{
	//local variables
	XMLDocument doc;

	//code
	//load collada file
	doc.LoadFile(fileName);

	//get mesh data - library_geometries
	XMLElement *meshData = doc.FirstChildElement("COLLADA")->FirstChildElement("library_geometries")->FirstChildElement("geometry")->FirstChildElement("mesh");
	XMLNode *geometryData = doc.FirstChildElement("COLLADA")->FirstChildElement("library_geometries")->FirstChildElement("geometry")->FirstChild();
	getGeometryData(geometryData);
	
	//printf("entry>> %d %d\n", collada.meshData.vertexInfo.stride, collada.meshData.vertexInfo.vec_vertex.size());
	//get skin data - library_controllers
	//XMLElement *skinData = doc.FirstChildElement("COLLADA")->FirstChildElement("library_controllers")->FirstChildElement("controller")->FirstChildElement("skin");
	//getSkinData(skinData); //this is raw data
	//printf("Skin info>>%s\n",collada.skinData.vec_skin_joints[0].name);
	//getVertexSkinningData();

	//get joint data - library_visual_scenes
	//XMLElement *armature = doc.FirstChildElement("COLLADA")->FirstChildElement("library_visual_scenes")->FirstChildElement("visual_scene")->FirstChildElement("node")->NextSiblingElement("node");

	//node - Armature - Torso node
	//XMLElement *jointData = armature->FirstChildElement("translate")->NextSiblingElement("rotate")->NextSiblingElement("rotate")->NextSiblingElement("rotate")
		//->NextSiblingElement("scale");
	//struct Joint jointD = getJointData(jointData->NextSibling());
	//collada.skeletonData = jointD;
	//printf("Joint id-%s joint child size:%d %d\n", collada.skeletonData.vec_children[1].id, collada.skeletonData.vec_children[1].vec_children.size(), collada.skeletonData.vec_children[1].vec_transform_matrix.size());

	//Animation data - library_animations
	//XMLNode *animationData = doc.FirstChildElement("COLLADA")->FirstChildElement("library_animations")->FirstChildElement("animation");
	//getAnimationData(animationData);

	return collada;
}

void getVertexSkinningData()
{
	printf("vsd - %d\n",collada.skinData.vec_vertex_weight_vcount.size());

	int pointer = 0; //for indexing joint and weight data in v
	//iterate effective joints of vertex data
	for (int i = 0; i < collada.skinData.vec_vertex_weight_vcount.size(); i++)
	{
		struct VertexSkinData skinData;
		skinData.vertexIndex = i;

		for (int j = 0; j < collada.skinData.vec_vertex_weight_vcount[i]; j++)
		{
			int vertIndex = collada.skinData.vec_vertex_weight_v[pointer]; //joint
			skinData.jointIds.push_back(vertIndex);
			pointer++; //moves pointer to weight associated with that joint

			int weightIndex = collada.skinData.vec_vertex_weight_v[pointer]; //weight index
			float weight = collada.skinData.vec_skin_weights[weightIndex]; //weight
			skinData.weights.push_back(weight);
			pointer++; //moves pointer to next joint
		}
		collada.skinningInfo.vec_vertex_skin_data.push_back(skinData);
	}

}

void getAnimationData(XMLNode * animationData)
{
	//local variables declaration


	//loop through animation data
	while(animationData != nullptr)
	{
		XMLNode *child_source = animationData->FirstChild();

		XMLElement *child_source_element = child_source->ToElement();
		struct Animation animation;

		//set id of animation
		animation.id=const_cast<char *> (child_source_element->FirstAttribute()->Value());

		//set id of input
		child_source_element->FirstChildElement("float_array")->NextSiblingElement("technique_common")->FirstChildElement("accessor")->QueryIntAttribute("stride",&animation.matrix_input.stride);
		//set Armature_Torso_pose_matrix-input
		data = const_cast<char *> (child_source_element->FirstChildElement("float_array")->GetText());

		attrib_token = strtok(data, space_seperator);
		while(attrib_token != nullptr)
		{
			animation.matrix_input.vec_post_matrix_input_data.push_back(atoi(attrib_token));
			attrib_token = strtok(NULL, space_seperator);
		}


		//set id of output
		child_source_element->NextSiblingElement("source")->FirstChildElement("float_array")->NextSiblingElement("technique_common")->FirstChildElement("accessor")->QueryIntAttribute("stride",&animation.matrix_output.stride);

		//set Armature_Torso_pose_matrix-output
		data = const_cast<char *> (child_source_element->NextSiblingElement("source")->FirstChildElement("float_array")->GetText());

		attrib_token = strtok(data, space_seperator);
		while(attrib_token != nullptr)
		{
			animation.matrix_output.vec_post_matrix_output_data.push_back(atoi(attrib_token));
			attrib_token = strtok(NULL, space_seperator);
		}

		collada.vec_animation.push_back(animation);

		//point to next animation data
		animationData= animationData->NextSibling();
	}
	printf("animation data size - %d\n", collada.vec_animation.size());
}

struct Joint getJointData(XMLNode *jointData)
{
	//local variables
	struct Joint joint;

	if (jointData != nullptr)
	{

		//code

		printf(">Processing %s|\n", jointData->ToElement()->FirstAttribute()->Value());
		//get id
		char *id = const_cast<char *> (jointData->ToElement()->FirstAttribute()->Value());
		joint.id = id;

		//get matrix data
		data = const_cast<char *> (jointData->FirstChildElement("matrix")->GetText());
		attrib_token = strtok(data, space_seperator);
		while (attrib_token != nullptr)
		{
			joint.vec_transform_matrix.push_back(atoi(attrib_token));
			attrib_token = strtok(NULL, space_seperator);
		}

		//set childre nodes
		XMLNode *node = jointData->FirstChild();
		while (node != nullptr)
		{
			if (strcmp("node", node->Value()) == 0)
			{
				joint.vec_children.push_back(getJointData(node));
			}

			node = node->NextSibling();
		}
	}
	else
	{
		printf("test passed\n");
	}
	return joint;
}

void getSkinData(XMLElement *skinData)
{
	//local variables
	

	//code
	//Armature_Cube_skin_joints
	XMLElement *Armature_Cube_skin_joints = skinData->FirstChildElement("source");

	//get skin joint array
	const char *attributeType = Armature_Cube_skin_joints->FirstAttribute()->Value();
	printf(">>attributeType - %s\n",attributeType);

	const char *type = Armature_Cube_skin_joints->FirstChildElement("Name_array")->FirstAttribute()->Value();
	printf(">>type - %s\n",type);

	//data
	data = const_cast<char *> (Armature_Cube_skin_joints->FirstChildElement("Name_array")->GetText());
	attrib_token = strtok(data, space_seperator);
	int i = 0;
	while(attrib_token != nullptr)
	{
		struct SkinJoint sj;
		char label[MAX_SKIN_LABEL_SIZE];
		strcpy(sj.name, attrib_token);
		sj.index = i;
		collada.skinData.vec_skin_joints.push_back(sj);
		i++;
		attrib_token = strtok(NULL, space_seperator);
	}
	printf(">>joint size - %d\n", collada.skinData.vec_skin_joints.size());


	//Armature_Cube-skin-bind_poses - 4*4 matrices for each joint
	XMLElement *Armature_Cube_skin_bind_poses = skinData->FirstChildElement("source")->NextSiblingElement("source");
	//data
	data = const_cast<char *>(Armature_Cube_skin_bind_poses->FirstChildElement("float_array")->GetText());
	attrib_token = strtok(data, space_seperator);
	while(attrib_token != nullptr)
	{
		collada.skinData.vec_skin_bind_poses.push_back(atof(attrib_token));
		attrib_token = strtok(NULL, space_seperator);
	}
	printf(">>bind poses size - %d\n", collada.skinData.vec_skin_bind_poses.size());


	//Armature_Cube-skin-weights
	XMLElement *Armature_Cube_skin_weights = skinData->FirstChildElement("source")->NextSiblingElement("source")->NextSiblingElement("source");
	//data
	data = const_cast<char *>(Armature_Cube_skin_weights->FirstChildElement("float_array")->GetText());
	attrib_token = strtok(data, space_seperator);
	while(attrib_token != nullptr)
	{
		collada.skinData.vec_skin_weights.push_back(atof(attrib_token));
		attrib_token = strtok(NULL, space_seperator);
	}
	printf(">>weights size - %d\n", collada.skinData.vec_skin_weights.size());

	//vertex_weights - vcount
	XMLElement *vertex_weights = skinData->FirstChildElement("source")->NextSiblingElement("source")->NextSiblingElement("source")->NextSiblingElement("vertex_weights");
	//data
	data = const_cast<char *>(vertex_weights->FirstChildElement("input")->NextSiblingElement("input")->NextSiblingElement("vcount")->GetText());
	attrib_token = strtok(data, space_seperator);
	while(attrib_token != nullptr)
	{
		collada.skinData.vec_vertex_weight_vcount.push_back(atof(attrib_token));
		attrib_token = strtok(NULL, space_seperator);
	}
	printf(">>vec_vertex_weight_vcount size - %d\n", collada.skinData.vec_vertex_weight_vcount.size());

	//vertex_weights - v
	//data
	data = const_cast<char *>(vertex_weights->FirstChildElement("input")->NextSiblingElement("input")->NextSiblingElement("vcount")->NextSiblingElement("v")->GetText());
	attrib_token = strtok(data, space_seperator);
	while(attrib_token != nullptr)
	{
		collada.skinData.vec_vertex_weight_v.push_back(atof(attrib_token));
		attrib_token = strtok(NULL, space_seperator);
	}
	printf(">>vec_vertex_weight_v size - %d\n", collada.skinData.vec_vertex_weight_v.size());
}

void getGeometryData(XMLNode *geometryData)
{
	//local variables
	const char *polyListStr = "polylist";
	struct CubeMeshPositions
	{
		char *id;
		int stride;
		std::vector<float> vec_cute_mesh_positions;
	};

	struct CubeMeshNormals
	{
		char *id;
		int stride;
		std::vector<float> vec_cute_mesh_normals;
	};

	struct CubeMeshTexcoords
	{
		char *id;
		int stride;
		std::vector<float> vec_cute_mesh_texcoords;
	};

	struct CubeMeshColors
	{
		char *id;
		int stride;
		std::vector<float> vec_cute_mesh_colors;
	};

	struct GeometryData
	{
		char *id;
		struct CubeMeshPositions cubeMeshPositions;
		struct CubeMeshNormals cubeMeshNormals;
		struct CubeMeshTexcoords cubeMeshTexcoords;
		struct CubeMeshColors cubeMeshColors;
	};

	struct MeshInformation meshInfo;

	//get positions, normals, textures and colors data
	struct GeometryData geometryMeshData;

	//to get data of geometry types and to find source
	struct InputTagData
	{
		const char *offset;
		const char *semantic;
		const char *source;
	};

	struct GeometryTypeData
	{
		struct InputTagData inputTags[4];
		char *vcount;
		char *p;
	};
	int vertexAttribInputCount = 0;//keep track of input type
	struct GeometryTypeData geometryTypeData;
	XMLNode *temp_child_source = geometryData->FirstChild();
	while (temp_child_source != nullptr)
	{
		XMLElement *temp_child_source_element = temp_child_source->ToElement();

		const char *lAttributeType = temp_child_source_element->Name();
		

		if (strcmp("triangles", lAttributeType) == 0 || strcmp("polylist", lAttributeType) == 0)
		{
			//check input tags starts
			
			XMLElement *ele = temp_child_source_element->FirstChildElement("input");
			while (ele != nullptr)
			{
				struct InputTagData inputTagData;
				ele->QueryStringAttribute("semantic", &inputTagData.semantic);
				ele->QueryStringAttribute("source", &inputTagData.source);
				ele->QueryStringAttribute("offset", &inputTagData.offset);
				//ele->Attribute("semantic", inputTagData.semantic);
				printf("vertexAttribInputCount >>>>>>>>%s\n", inputTagData.source);
				geometryTypeData.inputTags[vertexAttribInputCount] = inputTagData;
				vertexAttribInputCount++;
				ele = ele->NextSiblingElement("input");
			}
			printf("vertexAttribInputCount >>>>>>>>>>>>%d|\n", vertexAttribInputCount);
			break;
			//check input tags ends
		}
		temp_child_source = temp_child_source->NextSibling();
	}
	//fetching of data organization done


	XMLNode *child_source = geometryData->FirstChild();
	int sourceCounter = 0;
	while(child_source != nullptr)
	{
		XMLElement *child_source_element = child_source->ToElement();

		//type of data
		const char *attributeType = child_source_element->Name();
		printf("GeometryData: Node name - %s\n",child_source_element->Name());

		const char *attributeId = child_source_element->FirstAttribute()->Value();
		printf("GeometryData: attribute id - %s\n",attributeId);

		/*int re =strcmp(attributeType, "source");
		if(re==0)
		{
			printf("Yes %d %d |%s|\n",re,strlen(attributeType),attributeType);
		}else
		{
			printf("Not same %d %d |%s|\n",re,strlen(attributeType),attributeType);
		}*/
		if(strcmp(attributeType, "source")==0)
		{
			sourceCounter = sourceCounter + 1;
			//vertex
			printf(">>>>WhileIterating - inputcount - %d %s\n", vertexAttribInputCount, attributeId);
			printf(">>>>%s %s\n", geometryTypeData.inputTags[0].source, attributeId);
			
			switch (sourceCounter)
			{
			case 4:
				if (strstr(geometryTypeData.inputTags[3].source, attributeId) != NULL)
				{
					//set id
					//geometryMeshData.cubeMeshColors->id = const_cast<char *> (child_source_element->FirstAttribute()->Value());

					//set stride
					child_source_element->FirstChildElement("float_array")->NextSiblingElement("technique_common")->FirstChildElement("accessor")->QueryIntAttribute("stride", &geometryMeshData.cubeMeshColors.stride);
					//set data
					data = const_cast<char *> (child_source_element->FirstChildElement("float_array")->GetText());

					attrib_token = strtok(data, space_seperator);
					while (attrib_token != nullptr)
					{
						geometryMeshData.cubeMeshColors.vec_cute_mesh_colors.push_back(atof(attrib_token));
						attrib_token = strtok(NULL, space_seperator);
					}
				}
				break;
			case 3:
				if (strstr(geometryTypeData.inputTags[2].source, attributeId) != NULL)
				{
					//set id
					//geometryMeshData.cubeMeshTexcoords->id = const_cast<char *> (child_source_element->FirstAttribute()->Value());

					//set stride
					child_source_element->FirstChildElement("float_array")->NextSiblingElement("technique_common")->FirstChildElement("accessor")->QueryIntAttribute("stride", &geometryMeshData.cubeMeshTexcoords.stride);
					//set data
					data = const_cast<char *> (child_source_element->FirstChildElement("float_array")->GetText());

					attrib_token = strtok(data, space_seperator);
					while (attrib_token != nullptr)
					{
						geometryMeshData.cubeMeshTexcoords.vec_cute_mesh_texcoords.push_back(atof(attrib_token));
						attrib_token = strtok(NULL, space_seperator);
					}
				}
				break;
			case 2:
				if (strstr(geometryTypeData.inputTags[1].source, attributeId) != NULL)
				{
					//set id
					//geometryMeshData.cubeMeshNormals->id = const_cast<char *> (child_source_element->FirstAttribute()->Value());

					//set stride
					child_source_element->FirstChildElement("float_array")->NextSiblingElement("technique_common")->FirstChildElement("accessor")->QueryIntAttribute("stride", &geometryMeshData.cubeMeshNormals.stride);
					//set data
					data = const_cast<char *> (child_source_element->FirstChildElement("float_array")->GetText());

					attrib_token = strtok(data, space_seperator);
					while (attrib_token != nullptr)
					{
						geometryMeshData.cubeMeshNormals.vec_cute_mesh_normals.push_back(atof(attrib_token));
						attrib_token = strtok(NULL, space_seperator);
					}
				}
				break;
			case 1:
				//set id
				//geometryMeshData.cubeMeshPositions->id = const_cast<char *> (attributeId);
				//set stride
				//printf(">>|%s|\n", geometryData->FirstChild()->ToElement()->FirstAttribute()->Value());
				geometryData->FirstChild()->ToElement()->FirstChildElement("float_array")->NextSiblingElement("technique_common")->FirstChildElement("accessor")->QueryIntAttribute("stride", &geometryMeshData.cubeMeshPositions.stride);
				//set data
				data = const_cast<char *> (geometryData->FirstChild()->ToElement()->FirstChildElement("float_array")->GetText());
				attrib_token = strtok(data, space_seperator);
				while (attrib_token != nullptr)
				{
					geometryMeshData.cubeMeshPositions.vec_cute_mesh_positions.push_back(atof(attrib_token));
					attrib_token = strtok(NULL, space_seperator);
				}

				printf(">>|%d|\n", geometryMeshData.cubeMeshPositions.vec_cute_mesh_positions.size());
				break;
			default:
				printf("GeometryData: count is not matching for input count %d.\n", vertexAttribInputCount);
			}

		}
		else if (strcmp("triangles", attributeType) == 0 || strcmp("polylist", attributeType) == 0)
		{
			//local variables
			//char *polylist_vcount = child_source_element->FirstChildElement("input")->NextSiblingElement("vcount")->GetText();
			//char *polylist_p =  const_cast<char *> (child_source_element->FirstChildElement("input")->NextSiblingElement("p")->GetText());
			std::vector<int> polylist_faces_f;

			//print data sizes of all geometry data
			printf(">>>GeometryData: vec_positions size- %d \n", geometryMeshData.cubeMeshPositions.vec_cute_mesh_positions.size());
			printf(">>>GeometryData: vec_coloe size- %d \n", geometryMeshData.cubeMeshColors.vec_cute_mesh_colors.size());
			printf(">>>GeometryData: vec_normal size- %d \n", geometryMeshData.cubeMeshNormals.vec_cute_mesh_normals.size());
			printf(">>>GeometryData: vec_texcoord size- %d \n", geometryMeshData.cubeMeshTexcoords.vec_cute_mesh_texcoords.size());
			//check input tags starts
			/*int inputCount = 0;
			XMLElement *ele = child_source_element->FirstChildElement("input");
			while (ele != nullptr)
			{
				inputCount++;
				ele = ele->NextSiblingElement("input");
			}*/
			//check input tags ends

			//arrange all the positions in a vector
			std::vector<std::vector<float>> vec_positions;
			
			for (int i = 0; i < geometryMeshData.cubeMeshPositions.vec_cute_mesh_positions.size(); i=i+ geometryMeshData.cubeMeshPositions.stride)
			{
				std::vector<float> position;
				position.push_back(geometryMeshData.cubeMeshPositions.vec_cute_mesh_positions[i]);
				position.push_back(geometryMeshData.cubeMeshPositions.vec_cute_mesh_positions[i+1]);
				position.push_back(geometryMeshData.cubeMeshPositions.vec_cute_mesh_positions[i+2]);
				vec_positions.push_back(position);
			}
			printf(">>>GeometryData: vec_positions size- %d %d\n", geometryMeshData.cubeMeshPositions.vec_cute_mesh_positions.size(), vec_positions.size());
			//end arrangement

			//arrange all the normals in a vector
			std::vector<std::vector<float>> vec_normal;

			for (int i = 0; i < geometryMeshData.cubeMeshNormals.vec_cute_mesh_normals.size(); i = i + geometryMeshData.cubeMeshNormals.stride)
			{
				std::vector<float> normal;
				normal.push_back(geometryMeshData.cubeMeshNormals.vec_cute_mesh_normals[i]);
				normal.push_back(geometryMeshData.cubeMeshNormals.vec_cute_mesh_normals[i + 1]);
				normal.push_back(geometryMeshData.cubeMeshNormals.vec_cute_mesh_normals[i + 2]);
				vec_normal.push_back(normal);
			}
			printf(">>>GeometryData: vec_normal size- %d %d\n", geometryMeshData.cubeMeshNormals.vec_cute_mesh_normals.size(), vec_normal.size());
			//end arrangement

			//arrange all the texture in a vector
			std::vector<std::vector<float>> vec_texture;

			for (int i = 0; i < geometryMeshData.cubeMeshTexcoords.vec_cute_mesh_texcoords.size(); i = i + geometryMeshData.cubeMeshTexcoords.stride)
			{
				std::vector<float> texture;
				texture.push_back(geometryMeshData.cubeMeshTexcoords.vec_cute_mesh_texcoords[i]);
				texture.push_back(geometryMeshData.cubeMeshTexcoords.vec_cute_mesh_texcoords[i + 1]);
				vec_texture.push_back(texture);
			}
			printf(">>>GeometryData: vec_texture size- %d %d\n", geometryMeshData.cubeMeshTexcoords.vec_cute_mesh_texcoords.size(), vec_texture.size());
			//end arrangement

			//arrange all the colors in a vector
			std::vector<std::vector<float>> vec_color;

			for (int i = 0; i < geometryMeshData.cubeMeshColors.vec_cute_mesh_colors.size(); i = i + geometryMeshData.cubeMeshColors.stride)
			{
				std::vector<float> color;
				color.push_back(geometryMeshData.cubeMeshColors.vec_cute_mesh_colors[i]);
				color.push_back(geometryMeshData.cubeMeshColors.vec_cute_mesh_colors[i + 1]);
				color.push_back(geometryMeshData.cubeMeshColors.vec_cute_mesh_colors[i + 2]);
				vec_color.push_back(color);
			}
			printf(">>>GeometryData: vec_color size- %d %d\n", geometryMeshData.cubeMeshColors.vec_cute_mesh_colors.size(), vec_color.size());
			//end arrangement
			data = const_cast<char *> (child_source_element->FirstChildElement("input")->NextSiblingElement("p")->GetText());
			attrib_token = strtok(data, space_seperator);
			while(attrib_token != nullptr)
			{
				polylist_faces_f.push_back(atoi(attrib_token));
				attrib_token = strtok(NULL, space_seperator);
			}

			printf("GeometryData: size of faces - %d\n",polylist_faces_f.size());
			printf("GeometryData: test val - %d and input count - %d\n", geometryMeshData.cubeMeshPositions.vec_cute_mesh_positions.size(),vertexAttribInputCount);
			for(int i=0;i<polylist_faces_f.size();i=i+ vertexAttribInputCount)
			{
				switch (vertexAttribInputCount)
				{
				case 4:
					meshInfo.colorInfo.vec_color.push_back(vec_color[polylist_faces_f[i+3]]);
				case 3:
					meshInfo.texcoordInfo.vec_texcoord.push_back(vec_texture[polylist_faces_f[i+2]]);
				case 2:
					//printf("GeometryData: normals - %d %d %d\n",i, polylist_faces_f[i], polylist_faces_f[i+1]);
					meshInfo.normalInfo.vec_normal.push_back(vec_normal[polylist_faces_f[i+1]]);
				case 1:
					meshInfo.vertexInfo.vec_vertex.push_back(vec_positions[polylist_faces_f[i]]);
					break;
				default:
					printf("GeometryData: count is not matching for input count %d.\n", vertexAttribInputCount);
				}
			}
			printf("GeometryData: test val2 - %d first vertex val - %10.15f\n", meshInfo.vertexInfo.vec_vertex.size(), meshInfo.vertexInfo.vec_vertex[0][0]);

			//set stride
			meshInfo.vertexInfo.stride = geometryMeshData.cubeMeshPositions.stride;
			meshInfo.vertexInfo.size = polylist_faces_f.size();
			meshInfo.normalInfo.stride = geometryMeshData.cubeMeshNormals.stride;
			meshInfo.normalInfo.size = polylist_faces_f.size();
			meshInfo.texcoordInfo.stride = geometryMeshData.cubeMeshTexcoords.stride;
			meshInfo.normalInfo.size = polylist_faces_f.size();
			meshInfo.colorInfo.stride = geometryMeshData.cubeMeshColors.stride;
			meshInfo.normalInfo.size = polylist_faces_f.size();
			int size = meshInfo.vertexInfo.vec_vertex.size() + meshInfo.vertexInfo.vec_vertex.size() + meshInfo.vertexInfo.vec_vertex.size() + meshInfo.vertexInfo.vec_vertex.size();
			printf("GeometryData: calculated size - %d\n",size);
			printf("GeometryData: stride - %d\t%d\t%d\t%d\n",meshInfo.vertexInfo.vec_vertex.size(),meshInfo.normalInfo.vec_normal.size(),meshInfo.texcoordInfo.vec_texcoord.size(),meshInfo.colorInfo.vec_color.size());
		}else
		{
			printf("GeometryData: No match found for geometry data\n");
		}
	//point to next animation data
	child_source= child_source->NextSibling();
	}

	//convert vector<vector> to vector
	for (int i = 0; i < meshInfo.vertexInfo.vec_vertex.size(); i++)
	{
		std::vector<float> vec_pos = meshInfo.vertexInfo.vec_vertex[i];
		meshInfo.vertexInfo.vec_vertex_all.push_back(vec_pos[0]);
		meshInfo.vertexInfo.vec_vertex_all.push_back(vec_pos[1]);
		meshInfo.vertexInfo.vec_vertex_all.push_back(vec_pos[2]);
		//lPosition[3 * i + 0] = vec_pos[0];
		//lPosition[3 * i + 1] = vec_pos[1];
		//lPosition[3 * i + 2] = vec_pos[2];
	}
	for (int i = 0; i < meshInfo.normalInfo.vec_normal.size(); i++)
	{
		std::vector<float> vec_nor = meshInfo.normalInfo.vec_normal[i];
		meshInfo.normalInfo.vec_normal_all.push_back(vec_nor[0]);
		meshInfo.normalInfo.vec_normal_all.push_back(vec_nor[1]);
		meshInfo.normalInfo.vec_normal_all.push_back(vec_nor[2]);
		//lPosition[3 * i + 0] = vec_pos[0];
		//lPosition[3 * i + 1] = vec_pos[1];
		//lPosition[3 * i + 2] = vec_pos[2];
	}
	for (int i = 0; i < meshInfo.texcoordInfo.vec_texcoord.size(); i++)
	{
		std::vector<float> vec_texcord = meshInfo.texcoordInfo.vec_texcoord[i];
		meshInfo.texcoordInfo.vec_texcoord_all.push_back(vec_texcord[0]);
		meshInfo.texcoordInfo.vec_texcoord_all.push_back(vec_texcord[1]);
		meshInfo.texcoordInfo.vec_texcoord_all.push_back(vec_texcord[2]);
		//lPosition[3 * i + 0] = vec_pos[0];
		//lPosition[3 * i + 1] = vec_pos[1];
		//lPosition[3 * i + 2] = vec_pos[2];
	}
	for (int i = 0; i < meshInfo.colorInfo.vec_color.size(); i++)
	{
		std::vector<float> vec_col = meshInfo.colorInfo.vec_color[i];
		meshInfo.colorInfo.vec_color_all.push_back(vec_col[0]);
		meshInfo.colorInfo.vec_color_all.push_back(vec_col[1]);
		meshInfo.colorInfo.vec_color_all.push_back(vec_col[2]);
		//lPosition[3 * i + 0] = vec_pos[0];
		//lPosition[3 * i + 1] = vec_pos[1];
		//lPosition[3 * i + 2] = vec_pos[2];
	}
	printf("GeoData: positions sizes >> %d %d\n", meshInfo.vertexInfo.vec_vertex_all.size(),
		meshInfo.vertexInfo.vec_vertex.size());
	printf("GeoData: normal sizes >> %d %d\n", meshInfo.normalInfo.vec_normal_all.size(),
		meshInfo.normalInfo.vec_normal.size());
	printf("GeoData: texture sizes >> %d %d\n", meshInfo.texcoordInfo.vec_texcoord_all.size(),
		meshInfo.texcoordInfo.vec_texcoord.size());
	printf("GeoData: color sizes >> %d %d\n", meshInfo.colorInfo.vec_color_all.size(),
		meshInfo.colorInfo.vec_color.size());
	collada.meshData = meshInfo;
}
