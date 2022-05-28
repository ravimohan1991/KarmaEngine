// Shader needs be conforming to Mesh vertexdata. So we need an algorithm 
// to gauge that.
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUVs;// location 1 is reserved for color?


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
	fragUVs = inUVs;
}
/*
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inUVs;
layout(location = 3) in ivec4 inBoneIds;
layout(location = 4) in vec4 inWeights;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragUVs;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

layout(std140, binding = 0) uniform MVPUniformBufferObject
{
	mat4 u_Projection;
	mat4 u_View; // from world coordiantes to view coordinates
	mat4 u_World;// Converts from model coordinates to world coordinates (?)
	mat4 u_FinalBonesMatrices[MAX_BONES]; // Bone transformations which shall influence all the vertices in a Model.
};
	
void main()
{
	vec4 processedPosition = vec4(0.0f);
	for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
	{
		if(inBoneIds[i] == -1)
		{
			continue;
		}

		if(inBoneIds[i] >= MAX_BONES)
		{
			processedPosition = vec4(inPosition, 1.0f);// We don't want the influence of this bone!
			break;
		}

		// The local position of the vertices, guided by the (dynamical) bone arrangement, in Model space 
		// Model Space coordinates = dynamics capturing coordinates + the static coordinates of original model
		// No scope of confusion by "+" sign because we are talking about abstractions
		vec4 localPosition = u_FinalBonesMatrices[inBoneIds[i]] * vec4(inPosition, 1.0f);
		
		// Weighting each of the local coordinates of vertices as per bones' regulation specified 
		processedPosition += localPosition * inWeights[i];
	}

	// POV of camera
	mat4 viewWorldTransform = u_View * u_World;
	
	// Projection onto the screen of the camera, coordinates
	gl_Position = u_Projection * viewWorldTransform * processedPosition;
	fragColor = inColor;
	fragUVs = inUVs;
}
*/