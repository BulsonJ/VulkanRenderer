#version 450

layout (location = 0) in vec3 vPosition;

layout (location = 0) out vec3 outPosition;


layout(std140,set = 0, binding = 0) uniform  CameraBuffer{
	mat4 view;
} cameraData;

void main(void)		{
	gl_Position	= vec4(1.0);
}
