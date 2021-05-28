#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform MVPUniformBufferObject
{
	mat4 u_ViewProjection;
	mat4 u_Transform;
};


void main()
{
	gl_Position = u_ViewProjection *  u_Transform * vec4(inPosition, 1.0);
	fragColor = inColor;
}