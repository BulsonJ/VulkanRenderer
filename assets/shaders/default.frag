#version 460

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inTexCoords;

layout (location = 0) out vec4 outFragColor;

layout (set = 0, binding = 3) uniform sampler samp;
layout (set = 0, binding = 4) uniform texture2D bindlessTextures[];


void main(void)	{
	vec4 diffuse = texture(sampler2D(bindlessTextures[0], samp), inTexCoords);
	outFragColor = vec4(inColor,1.0);
}