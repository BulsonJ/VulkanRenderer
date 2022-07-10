#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;
layout (location = 3) in vec2 vTexCoord;

layout (location = 0) out vec3 outColor;

layout( push_constant ) uniform constants
{
	int transformIndex;
} pushConstans;

layout(std140,set = 0, binding = 0) uniform  TransformBuffer{
	mat4 modelMatrix;
} transformData;

void main(void)		{
	gl_Position = vec4(vPosition, 1.0f);
	outColor = vColor;
}
