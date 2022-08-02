#version 460

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;
layout (location = 3) in vec2 vTexCoord;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outTexCoords;
layout (location = 2) out flat int outDrawDataIndex;

layout( push_constant ) uniform constants
{
	int drawDataIndex;
} pushConstants;

struct DrawData{
	int transformIndex;
	int materialIndex;
};

struct ObjectData{
	mat4 modelMatrix;
};

struct MaterialData{
	ivec4 diffuseIndex;
};

layout(std140,set = 0, binding = 0) readonly buffer DrawDataBuffer{
	DrawData objects[];
} drawDataArray;

layout(std140,set = 0, binding = 1) readonly buffer TransformBuffer{
	ObjectData objects[];
} transformData;

layout(std140,set = 0, binding = 2) readonly buffer MaterialDataBuffer{
	MaterialData objects[];
} materialDataArray;

layout(std140,set = 1, binding = 0) uniform  CameraBuffer{
	mat4 viewMatrix;
	mat4 projMatrix;
} cameraData;

void main(void)		{
	DrawData draw = drawDataArray.objects[pushConstants.drawDataIndex];
	mat4 proj = cameraData.projMatrix;
	mat4 view = cameraData.viewMatrix;
	mat4 model = transformData.objects[draw.transformIndex].modelMatrix;
	mat4 transformMatrix = (proj * view * model);	
	gl_Position = transformMatrix * vec4(vPosition, 1.0f);
	outColor = vColor;
	outTexCoords = vTexCoord;
	outDrawDataIndex = pushConstants.drawDataIndex;
}
