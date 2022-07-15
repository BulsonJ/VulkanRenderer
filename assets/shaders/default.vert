#version 460

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;
layout (location = 3) in vec2 vTexCoord;

layout (location = 0) out vec3 outColor;

layout( push_constant ) uniform constants
{
	int transformIndex;
} pushConstants;

struct ObjectData{
	mat4 modelMatrix;
};

layout(std140,set = 0, binding = 0) readonly buffer TransformBuffer{
	ObjectData objects[];
} transformData;

layout(std140,set = 1, binding = 0) uniform  CameraBuffer{
	mat4 viewMatrix;
	mat4 projMatrix;
} cameraData;

void main(void)		{
	mat4 proj = cameraData.projMatrix;
	mat4 view = cameraData.viewMatrix;
	mat4 model = transformData.objects[pushConstants.transformIndex].modelMatrix;
	mat4 transformMatrix = (proj * view * model);	
	gl_Position = transformMatrix * vec4(vPosition, 1.0f);
	outColor = vColor;
}
