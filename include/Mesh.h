#pragma once

#include <glm.hpp>

#include <vector>
#include <optional>

#include "ResourceManager.h"

struct VertexInputDescription {

	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;

	VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct Vertex {

	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 uv;

	static VertexInputDescription getVertexDescription();
};

struct Mesh {
    std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	BufferView vertexBuffer;
	BufferView indexBuffer;

	bool hasIndices() const;

	static glm::vec3 CalculateSurfaceNormal(glm::vec3 pointA, glm::vec3 pointB, glm::vec3 pointC) {
		const glm::vec3 sideAB = pointB - pointA;
		const glm::vec3 sideAC = pointC - pointA;

		return glm::cross(sideAB, sideAC);
	}

	static Mesh GenerateTriangle();
	static Mesh GenerateQuad();
	static Mesh GenerateCube();
	static Mesh GenerateSkyboxCube();
	static Mesh GeneratePlane(int size);
};