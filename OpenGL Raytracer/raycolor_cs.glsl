#version 430 core

struct ray {
	vec3 origin;
	vec3 dir;
	vec3 color;
	float t;
	int primitiveID;
};

struct sphere {
	vec3 pos;
	float r;
};

struct triangle {
	vec3 x, y, z;
};

layout (local_size_x = 32, local_size_y = 1) in;
layout (rgba8, binding = 0) uniform image2D outTexture;
layout (binding = 1) buffer ray_buffer { ray rays[]; };

uniform vec3 camera_pos;
uniform vec3 camera_dir;
uniform vec3 camera_up;
uniform vec3 camera_right;

uniform vec3 light_position;
uniform vec3 light_color;

#define NEAR_ZERO 1e-20 // 1 * 10^-20
#define NEAR_PLANE_DIST 1.0f

void main()
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	//color[gl_LocalInvocationID.x] = vec4(0.0, 1.0, 0.0, 1.0);
	imageStore(outTexture, storePos, vec4(rays[storePos.x + storePos.y * 800].color, 1.0));	
	//imageStore(outTexture, storePos, vec4(0.0, 1.0, 0.0, 1.0));	
}