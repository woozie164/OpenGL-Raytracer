#version 430 core

layout (local_size_x = 32, local_size_y = 1) in;
layout (rgba8, binding = 0) uniform image2D outTexture;

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

void main(void) 
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	float dx = (storePos.x - 400) / 400.0f; 
	float dy = (storePos.y - 400) / 400.0f;
	
	// A point in the near plane
	vec3 s = camera_pos + camera_dir * NEAR_PLANE_DIST + dx * camera_right + dy * camera_up;	
	
	vec3 ray_dir = normalize(s - camera_pos);		
	rays[storePos.x] = ray(camera_pos, ray_dir);	
}