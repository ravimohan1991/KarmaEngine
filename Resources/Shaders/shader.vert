// Shader needs be conforming to Mesh vertexdata. So we need an algorithm
// to gauge that.
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;


layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragUVs;

layout(std140, binding = 0) uniform MVPUniformBufferObject
{
	mat4 u_Projection;
	mat4 u_View;
};


void main()
{
	gl_Position = u_Projection * u_View * vec4(inPosition, 1.0);
	fragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	fragUVs = inUV;
}
