#include "Mesh.h"

#include <iostream>
#include <array>

VertexInputDescription Vertex::getVertexDescription()
{
	VertexInputDescription description;

	//we will have just 1 vertex buffer binding, with a per-vertex rate
	VkVertexInputBindingDescription mainBinding = {};
	mainBinding.binding = 0;
	mainBinding.stride = sizeof(Vertex);
	mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	description.bindings.push_back(mainBinding);

	//Position will be stored at Location 0
	VkVertexInputAttributeDescription positionAttribute = {};
	positionAttribute.binding = 0;
	positionAttribute.location = 0;
	positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	positionAttribute.offset = offsetof(Vertex, position);

	//Normal will be stored at Location 1
	VkVertexInputAttributeDescription normalAttribute = {};
	normalAttribute.binding = 0;
	normalAttribute.location = 1;
	normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	normalAttribute.offset = offsetof(Vertex, normal);

	//Color will be stored at Location 2
	VkVertexInputAttributeDescription colorAttribute = {};
	colorAttribute.binding = 0;
	colorAttribute.location = 2;
	colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	colorAttribute.offset = offsetof(Vertex, color);

	//UV will be stored at Location 3
	VkVertexInputAttributeDescription uvAttribute = {};
	uvAttribute.binding = 0;
	uvAttribute.location = 3;
	uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;
	uvAttribute.offset = offsetof(Vertex, uv);

	description.attributes.push_back(positionAttribute);
	description.attributes.push_back(normalAttribute);
	description.attributes.push_back(colorAttribute);
	description.attributes.push_back(uvAttribute);
	return description;
}

Mesh Mesh::GenerateTriangle() {
	Mesh _triangleMesh;

	_triangleMesh.vertices.resize(3);

	_triangleMesh.vertices[0].position = {	 0.5f, 0.5f, 0.0f };
	_triangleMesh.vertices[1].position = { -0.5f, 0.5f, 0.0f };
	_triangleMesh.vertices[2].position = { -0.5f,-0.5f, 0.0f };

	_triangleMesh.vertices[0].normal = { 0.f, 1.f, 0.0f };
	_triangleMesh.vertices[1].normal = { 0.f, 1.f, 0.0f };
	_triangleMesh.vertices[2].normal = { 0.f, 1.f, 0.0f };

	_triangleMesh.vertices[0].color = { 0.f, 1.f, 0.0f };
	_triangleMesh.vertices[1].color = { 0.f, 1.f, 0.0f };
	_triangleMesh.vertices[2].color = { 0.f, 1.f, 0.0f };

	_triangleMesh.vertices[0].uv = { 0.f, 1.f};
	_triangleMesh.vertices[1].uv = { 1.f, 0.f};
	_triangleMesh.vertices[2].uv = { 0.f, 0.f};

	return _triangleMesh;
}

Mesh Mesh::GenerateQuad() {
	Mesh mesh;

	mesh.vertices.resize(4);

	mesh.vertices[0].position = { -1.0f, 1.0f, 0.0f };
	mesh.vertices[1].position = { 1.0f, 1.0f, 0.0f };
	mesh.vertices[2].position = { -1.0f,-1.0f, 0.0f };
	mesh.vertices[3].position = { 1.0f,-1.0f, 0.0f };

	mesh.indices.resize(6);
	mesh.indices[0] = 0;
	mesh.indices[1] = 1;
	mesh.indices[2] = 2;
	mesh.indices[3] = 1;
	mesh.indices[4] = 3;
	mesh.indices[5] = 2;

	mesh.vertices[0].normal = { 0.f, 1.f, 0.0f };
	mesh.vertices[1].normal = { 0.f, 1.f, 0.0f };
	mesh.vertices[2].normal = { 0.f, 1.f, 0.0f };
	mesh.vertices[3].normal = { 0.f, 1.f, 0.0f };

	mesh.vertices[0].color = { 0.f, 1.f, 0.0f };
	mesh.vertices[1].color = { 0.f, 1.f, 0.0f };
	mesh.vertices[2].color = { 0.f, 1.f, 0.0f };
	mesh.vertices[3].color = { 0.f, 1.f, 0.0f };

	mesh.vertices[0].uv = { 0.f, 1.f };
	mesh.vertices[1].uv = { 1.f, 1.f };
	mesh.vertices[2].uv = { 0.f, 0.f };
	mesh.vertices[3].uv = { 1.f, 0.f };

	return mesh;
}

Mesh Mesh::GenerateCube() {
	Mesh mesh;

	mesh.vertices.resize(4*6);

	// Z- face
	mesh.vertices[0].position  = { -0.5f, 0.5f, -0.5f };
	mesh.vertices[1].position  = {  0.5f, 0.5f, -0.5f };
	mesh.vertices[2].position  = { -0.5f,-0.5f, -0.5f };
	mesh.vertices[3].position  = {  0.5f,-0.5f, -0.5f };

	mesh.vertices[0].normal = { 0.f, 0.0f, -1.0f };
	mesh.vertices[1].normal = { 0.f, 0.0f, -1.0f };
	mesh.vertices[2].normal = { 0.f, 0.0f, -1.0f };
	mesh.vertices[3].normal = { 0.f, 0.0f, -1.0f };

	// Z+ face
	mesh.vertices[4].position = { -0.5f, 0.5f, 0.5f };
	mesh.vertices[5].position = { 0.5f, 0.5f, 0.5f };
	mesh.vertices[6].position = { -0.5f,-0.5f, 0.5f };
	mesh.vertices[7].position = { 0.5f,-0.5f, 0.5f };

	mesh.vertices[4].normal = { 0.f, 0.0f, 1.0f};
	mesh.vertices[5].normal = { 0.f, 0.0f, 1.0f };
	mesh.vertices[6].normal = { 0.f, 0.0f, 1.0f };
	mesh.vertices[7].normal = { 0.f, 0.0f, 1.0f  };

	// Bottom Face
	mesh.vertices[8].position = { -0.5f, -0.5f, 0.5f };
	mesh.vertices[9].position = { 0.5f, -0.5f, 0.5f };
	mesh.vertices[10].position = { -0.5f, -0.5f,-0.5f };
	mesh.vertices[11].position = { 0.5f, -0.5f,-0.5f };

	mesh.vertices[8].normal = { 0.f, -1.f, 0.0f };
	mesh.vertices[9].normal = { 0.f, -1.f, 0.0f };
	mesh.vertices[10].normal = { 0.f, -1.f, 0.0f };
	mesh.vertices[11].normal = { 0.f, -1.f, 0.0f };

	// Top Face
	mesh.vertices[12].position = { -0.5f, 0.5f, 0.5f };
	mesh.vertices[13].position = {  0.5f, 0.5f, 0.5f };
	mesh.vertices[14].position = { -0.5f, 0.5f,-0.5f };
	mesh.vertices[15].position = {  0.5f, 0.5f,-0.5f };

	mesh.vertices[12].normal = { 0.f, 1.f, 0.0f };
	mesh.vertices[13].normal = { 0.f, 1.f, 0.0f };
	mesh.vertices[14].normal = { 0.f, 1.f, 0.0f };
	mesh.vertices[15].normal = { 0.f, 1.f, 0.0f };

	// X- face
	mesh.vertices[16].position = { -0.5f,  -0.5f, 0.5f };
	mesh.vertices[17].position = { -0.5f,   0.5f, 0.5f };
	mesh.vertices[18].position = { -0.5f,  -0.5f,-0.5f };
	mesh.vertices[19].position = { -0.5f,   0.5f,-0.5f };

	mesh.vertices[16].normal = { -1.f, 0.0f, 0.0f };
	mesh.vertices[17].normal = { -1.f, 0.0f, 0.0f };
	mesh.vertices[18].normal = { -1.f, 0.0f, 0.0f };
	mesh.vertices[19].normal = { -1.f, 0.0f, 0.0f };
	// X+ face
	mesh.vertices[20].position = { 0.5f,  -0.5f, 0.5f };
	mesh.vertices[21].position = { 0.5f,   0.5f, 0.5f };
	mesh.vertices[22].position = { 0.5f,  -0.5f,-0.5f };
	mesh.vertices[23].position = { 0.5f,   0.5f,-0.5f };

	mesh.vertices[20].normal = { 1.f, 0.0f, 0.0f };
	mesh.vertices[21].normal = { 1.f, 0.0f, 0.0f };
	mesh.vertices[22].normal = { 1.f, 0.0f, 0.0f };
	mesh.vertices[23].normal = { 1.f, 0.0f, 0.0f };

	mesh.indices.resize(6*6);
	for (int i = 0; i < mesh.indices.size()/6; ++i) {
		int triIndex = i * 6;
		int vertIndex = i * 4;
		mesh.indices[triIndex + 0] = vertIndex + 0;
		mesh.indices[triIndex + 1] = vertIndex + 1;
		mesh.indices[triIndex + 2] = vertIndex + 2;
		mesh.indices[triIndex + 3] = vertIndex + 1;
		mesh.indices[triIndex + 4] = vertIndex + 3;
		mesh.indices[triIndex + 5] = vertIndex + 2;
	}

	for (int i = 0; i < mesh.vertices.size() / 4; ++i) {
		int vertIndex = i * 4;

		mesh.vertices[vertIndex + 0].color = { 1.f, 1.f, 1.f };
		mesh.vertices[vertIndex + 1].color = { 1.f, 1.f, 1.f };
		mesh.vertices[vertIndex + 2].color = { 1.f, 1.f, 1.f };
		mesh.vertices[vertIndex + 3].color = { 1.f, 1.f, 1.f };
					   
		mesh.vertices[vertIndex + 0].uv = { 0.f, 1.f };
		mesh.vertices[vertIndex + 1].uv = { 1.f, 1.f };
		mesh.vertices[vertIndex + 2].uv = { 0.f, 0.f };
		mesh.vertices[vertIndex + 3].uv = { 1.f, 0.f };
	}

	return mesh;
}

Mesh Mesh::GenerateSkyboxCube() {
	Mesh mesh;

	mesh.vertices.resize(4 * 6);

	// Z- face
	mesh.vertices[0].position = { -1.0f, 1.0f, -1.0f };
	mesh.vertices[1].position = { 1.0f, 1.0f, -1.0f };
	mesh.vertices[2].position = { -1.0f,-1.0f, -1.0f };
	mesh.vertices[3].position = { 1.0f,-1.0f, -1.0f };

	mesh.vertices[0].normal = { 0.f, 0.0f, -1.0f };
	mesh.vertices[1].normal = { 0.f, 0.0f, -1.0f };
	mesh.vertices[2].normal = { 0.f, 0.0f, -1.0f };
	mesh.vertices[3].normal = { 0.f, 0.0f, -1.0f };

	// Z+ face
	mesh.vertices[4].position = { -1.0f, 1.0f, 1.0f };
	mesh.vertices[5].position = { 1.0f, 1.0f, 1.0f };
	mesh.vertices[6].position = { -1.0f,-1.0f, 1.0f };
	mesh.vertices[7].position = { 1.0f,-1.0f, 1.0f };

	mesh.vertices[4].normal = { 0.f, 0.0f, 1.0f };
	mesh.vertices[5].normal = { 0.f, 0.0f, 1.0f };
	mesh.vertices[6].normal = { 0.f, 0.0f, 1.0f };
	mesh.vertices[7].normal = { 0.f, 0.0f, 1.0f };

	// Bottom Face
	mesh.vertices[8].position = { -1.0f, -1.0f, 1.0f };
	mesh.vertices[9].position = { 1.0f, -1.0f, 1.0f };
	mesh.vertices[10].position = { -1.0f, -1.0f,-1.0f };
	mesh.vertices[11].position = { 1.0f, -1.0f,-1.0f };

	mesh.vertices[8].normal = { 0.f, -1.f, 0.0f };
	mesh.vertices[9].normal = { 0.f, -1.f, 0.0f };
	mesh.vertices[10].normal = { 0.f, -1.f, 0.0f };
	mesh.vertices[11].normal = { 0.f, -1.f, 0.0f };

	// Top Face
	mesh.vertices[12].position = { -1.0f, 1.0f, 1.0f };
	mesh.vertices[13].position = { 1.0f, 1.0f, 1.0f };
	mesh.vertices[14].position = { -1.0f, 1.0f,-1.0f };
	mesh.vertices[15].position = { 1.0f, 1.0f,-1.0f };

	mesh.vertices[12].normal = { 0.f, 1.f, 0.0f };
	mesh.vertices[13].normal = { 0.f, 1.f, 0.0f };
	mesh.vertices[14].normal = { 0.f, 1.f, 0.0f };
	mesh.vertices[15].normal = { 0.f, 1.f, 0.0f };

	// X- face
	mesh.vertices[16].position = { -1.0f,  -1.0f, 1.0f };
	mesh.vertices[17].position = { -1.0f,   1.0f, 1.0f };
	mesh.vertices[18].position = { -1.0f,  -1.0f,-1.0f };
	mesh.vertices[19].position = { -1.0f,   1.0f,-1.0f };

	mesh.vertices[16].normal = { -1.f, 0.0f, 0.0f };
	mesh.vertices[17].normal = { -1.f, 0.0f, 0.0f };
	mesh.vertices[18].normal = { -1.f, 0.0f, 0.0f };
	mesh.vertices[19].normal = { -1.f, 0.0f, 0.0f };
	// X+ face
	mesh.vertices[20].position = { 1.0f,  -1.0f, 1.0f };
	mesh.vertices[21].position = { 1.0f,   1.0f, 1.0f };
	mesh.vertices[22].position = { 1.0f,  -1.0f,-1.0f };
	mesh.vertices[23].position = { 1.0f,   1.0f,-1.0f };

	mesh.vertices[20].normal = { 1.f, 0.0f, 0.0f };
	mesh.vertices[21].normal = { 1.f, 0.0f, 0.0f };
	mesh.vertices[22].normal = { 1.f, 0.0f, 0.0f };
	mesh.vertices[23].normal = { 1.f, 0.0f, 0.0f };

	mesh.indices.resize(6 * 6);
	for (int i = 0; i < mesh.indices.size() / 6; ++i) {
		int triIndex = i * 6;
		int vertIndex = i * 4;
		mesh.indices[triIndex + 0] = vertIndex + 0;
		mesh.indices[triIndex + 1] = vertIndex + 1;
		mesh.indices[triIndex + 2] = vertIndex + 2;
		mesh.indices[triIndex + 3] = vertIndex + 1;
		mesh.indices[triIndex + 4] = vertIndex + 3;
		mesh.indices[triIndex + 5] = vertIndex + 2;
	}

	for (int i = 0; i < mesh.vertices.size() / 4; ++i) {
		int vertIndex = i * 4;

		mesh.vertices[vertIndex + 0].color = { 1.f, 1.f, 1.f };
		mesh.vertices[vertIndex + 1].color = { 1.f, 1.f, 1.f };
		mesh.vertices[vertIndex + 2].color = { 1.f, 1.f, 1.f };
		mesh.vertices[vertIndex + 3].color = { 1.f, 1.f, 1.f };

		mesh.vertices[vertIndex + 0].uv = { 0.f, 1.f };
		mesh.vertices[vertIndex + 1].uv = { 1.f, 1.f };
		mesh.vertices[vertIndex + 2].uv = { 0.f, 0.f };
		mesh.vertices[vertIndex + 3].uv = { 1.f, 0.f };
	}

	return mesh;
}

Mesh Mesh::GeneratePlane(int size) {
	Mesh mesh;

	const int meshSize = size + 1;
	mesh.vertices.resize(meshSize * meshSize);

	const float halfPos = size / 2.f;

	int verts = 0;
	for (int z = 0; z < meshSize; z++) {
		for (int x = 0; x < meshSize; x++) {
			mesh.vertices[verts].position = { static_cast<float>(x) - halfPos, 0.f, z - halfPos };

			mesh.vertices[verts].normal = { 0.f, 1.f, 0.0f };

			mesh.vertices[verts].color = { 1.f, 1.f, 1.0f };

			mesh.vertices[verts].uv = { static_cast<float>(x) / size , static_cast<float>(z) / size };
			verts++;
		}
	}

	mesh.indices.resize(size * size * 6);
	int lastTri = 0;
	int vertex = 0;
	for (int z = 0; z < size; ++z) {
		for (int x = 0; x < size; ++x) {
			mesh.indices[lastTri + 0] = vertex + 0;
			mesh.indices[lastTri + 1] = vertex + meshSize;
			mesh.indices[lastTri + 2] = vertex + meshSize + 1;
			mesh.indices[lastTri + 3] = vertex + 0;
			mesh.indices[lastTri + 4] = vertex + meshSize + 1;
			mesh.indices[lastTri + 5] = vertex + 1;
			lastTri += 6;
			vertex++;
		}
		vertex++;
	}

	return mesh;
}

bool Mesh::hasIndices() const {
	if (this->indices.size() > 0) {
		return true;
	}
	return false;
}