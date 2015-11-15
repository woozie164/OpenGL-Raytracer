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

struct vertex {
	vec3 pos;
	vec2 texCoord;
};

layout (local_size_x = 32, local_size_y = 1) in;
layout (rgba8, binding = 0) uniform image2D outTexture;
layout (rgba8, binding = 1) uniform image2D meshTexture;

layout (binding = 1) buffer ray_buffer { ray rays[]; };
layout (binding = 2) buffer sword_buffer { vertex vertices[]; };
uniform int num_vertices;

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