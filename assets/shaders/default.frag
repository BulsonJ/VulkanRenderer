#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inTexCoords;
layout (location = 2) in flat int inDrawDataIndex;

layout (location = 0) out vec4 outFragColor;

struct DrawData{
	int transformIndex;
	int materialIndex;
};

struct MaterialData{
	ivec4 diffuseIndex;
};

layout(std140,set = 0, binding = 0) readonly buffer DrawDataBuffer{
	DrawData objects[];
} drawDataArray;

layout(std140,set = 0, binding = 2) readonly buffer MaterialDataBuffer{
	MaterialData objects[];
} materialDataArray;

layout (set = 0, binding = 3) uniform sampler samp;
layout (set = 0, binding = 4) uniform texture2D bindlessTextures[];

void main(void)	{
	DrawData draw = drawDataArray.objects[inDrawDataIndex];
	MaterialData matData = materialDataArray.objects[draw.materialIndex];
	int diffuseIndex = matData.diffuseIndex.x;

	vec4 diffuse = vec4(inColor,1.0);
	if (diffuseIndex >= 0){
		diffuse = texture(sampler2D(bindlessTextures[(nonuniformEXT(diffuseIndex))], samp), inTexCoords);
	}
	outFragColor = diffuse;
}