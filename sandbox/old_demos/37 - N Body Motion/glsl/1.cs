#version 430 core
layout(local_size_x = 256) in;

layout(location = 0) uniform float dt;
layout(std430, binding = 0) buffer pblock 
{
	vec4 positions[]; 
};

layout(std430, binding = 1) buffer vblock
{
	vec4 velocities[]; 
};

void main() 
{

	int N = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);

	int index = int(gl_GlobalInvocationID);

	vec3 position = positions[index].xyz;
	vec3 velocity = velocities[index].xyz;
	vec3 acceleration = vec3(0.0f, 0.0f, 0.0f);

	for(int i = 0; i < N; ++i)
	{
		vec3 other = positions[i].xyz;
		vec3 diff = position - other;
		float invdist = 1.0f / (length(diff) + 0.001f);
		acceleration -= diff * 0.1 * invdist * invdist;
	};

	velocities[index] = vec4(velocity + dt * acceleration, 0.0f);
};
