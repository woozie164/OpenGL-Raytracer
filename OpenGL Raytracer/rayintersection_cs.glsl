#version 430 core

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

// Do intersection tests with all the geometry in the scene
void trace(vec3 ray_origin, vec3 ray_dir, out float t, in out int primitiveID) {	
	// Set initial value to infinity
	float t_min = 1.0 / 0.0;	
	
	// -1 indicates that this ray haven't intersected anything
	//primitiveID = -1;
	
	if(RayVsTriangle(ray_origin, ray_dir, x, y, z, t)) {
		if(t > 0 && t < t_min && primitiveID != 0) {
			t_min = t;
			primitiveID = 0;
		}
	}
	
	my_sphere = sphere(vec3(-5.0f, -5.0f, -5.0f), 1.0f);
	ray my_ray = ray(ray_origin, ray_dir);
	/*
	RaySphereIntersect(my_ray, my_sphere, t);
	if(t < t_min) {
		t_min = t;
		primitiveID = 1;
	}
	*/
	
	float t0, t1;
	if(intersectSphere(ray_origin, ray_dir, my_sphere, t0, t1)){
		t = min(t0, t1);
		if(t > 0 && t < t_min && primitiveID != 1) {
			t_min = t;
			primitiveID = 1;
		}
	}
	/*
	RayVsSphere(ray_origin, ray_dir, vec3(-5.0f, -5.0f, -5.0f), 1.0f, t);
	if(t < t_min) {
		t_min = t;
		primitiveID = 1;
	}
	*/
	t = t_min;
}

void main(void) 
{
	rays[gl_LocalInvocationID.x]
}