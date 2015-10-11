#version 430 core

layout (local_size_x = 32, local_size_y = 1) in;
layout (rgba8, binding = 0) uniform image2D outTexture;
layout (binding = 1) buffer color_test { vec4 col[]; };

uniform vec3 camera_pos;
uniform vec3 camera_dir;
uniform vec3 camera_up;
uniform vec3 camera_right;

uniform vec3 light_position;
uniform vec3 light_color;

#define NEAR_ZERO 1e-20 // 1 * 10^-20

struct ray {
	vec3 origin;
	vec3 dir;
};

struct sphere {
	vec3 pos;
	float r;
};

struct triangle {
	vec3 x, y, z;
};

#define NEAR_PLANE_DIST 1.0f

// 1 ray for each pixel, 800x800 pixels total
// but an 800*800 array is too big for shared memory
// Not sure how to solve that. Probably need to be careful in 
// which order i run the shader kernels
shared ray rays[32];
shared vec4 color[32];

void main()
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	//color[gl_LocalInvocationID.x] = vec4(0.0, 1.0, 0.0, 1.0);
	imageStore(outTexture, storePos, col[gl_LocalInvocationID.x]);	
}