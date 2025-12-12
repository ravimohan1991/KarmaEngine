#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUVs;

layout(location = 0) out vec4 outColor;

// set 1: texture
layout(set = 1, binding = 0) uniform sampler2D texSampler;

void main()
{
	outColor = vec4(vec3(fragColor.x, fragColor.y, fragColor.z) * texture(texSampler, fragUVs).rgb, 1.0);
}
