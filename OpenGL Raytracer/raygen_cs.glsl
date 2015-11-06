#version 430 core

struct ray {	
	vec3 origin;
	vec3 dir;
	vec3 color;
	float t;
	int primitiveID;	
	// the surface normal from the last primitive this ray 
	// intersected with
	vec3 n;	
};

struct sphere {
	vec3 pos;
	float r;
};

struct triangle {
	vec3 x, y, z;
};

struct light {
	vec3 pos;
	vec3 color;
};

layout (local_size_x = 32, local_size_y = 1) in;
layout (rgba8, binding = 0) uniform image2D outTexture;
layout (binding = 1) buffer ray_buffer { ray rays[]; };

uniform vec3 camera_pos;
uniform vec3 camera_dir;
uniform vec3 camera_up;
uniform vec3 camera_right;

uniform int num_lights;
layout (binding = 0) uniform LightsBuffer {
 light lights[10];
};

uniform vec3 light_position;
uniform vec3 light_color;

#define NEAR_ZERO 1e-20 // 1 * 10^-20
#define NEAR_PLANE_DIST 1.0f

void main(void) 
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	float dx = (storePos.x - 400) / 400.0f; 
	float dy = (storePos.y - 400) / 400.0f;
	
	// A point in the near plane
	vec3 s = camera_pos + camera_dir * NEAR_PLANE_DIST + dx * camera_right + dy * camera_up;	
	
	vec3 ray_dir = normalize(s - camera_pos);		
	rays[storePos.x + storePos.y * 800] = ray(camera_pos, ray_dir, vec3(1.0, 0.0, 0.0), -1, -1, vec3(0.0));		
}