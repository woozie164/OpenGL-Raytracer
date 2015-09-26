#version 430 core

layout (local_size_x = 1, local_size_y = 1) in;
layout (rgba8, binding = 0) uniform image2D outTexture;

uniform vec3 camera_pos;
uniform vec3 camera_dir;
uniform vec3 camera_up;
uniform vec3 camera_right;

bool RayVsTriangle(vec3 ray_origin, vec3 ray_dir,
 vec3 tri_x, vec3 tri_y,  vec3 tri_z,
 out float t)
{
	vec3 e1 = tri_y - tri_x;
	vec3 e2 = tri_z - tri_x;
	vec3 q = cross(ray_dir,  e2);
	float a = dot(e1, q);
	float NEAR_ZERO = 1e-20; // 1 * 10^-20
	if(a > - NEAR_ZERO && a < NEAR_ZERO) return false;
	
	float f = 1 / a;
	vec3 s = ray_origin - tri_x;
	float u = f * dot(s, q);
	if(u < 0.0f) return false;
	
	vec3 r = cross(s, e1);
	float v = f * dot(ray_dir, r);
	if(v < 0.0f || u + v > 1.0f) return false;
	
	t = f * dot(e2, r);
	return true;
}

/*
void RayVsTriangle(Ray & ray, Triangle & tri, HitData & hitData)
{
	Vec e1 = tri.p[1] - tri.p[0];
	Vec e2 = tri.p[2] - tri.p[0];
	Vec q = ray.d ^ e2;
	float a = e1 * q;
	if(a > - NEAR_ZERO && a < NEAR_ZERO) return;
	float f = 1 / a;
	Vec s = ray.o - tri.p[0];
	float u = f * (s * q);
	if(u < 0.0f) return;
	Vec r = s ^ e1;
	float v = f * (ray.d * r);
	if(v < 0.0f || u + v > 1.0f) return;
	float t = f * (e2 * r);
	hitData.t = t;
	hitData.color = tri.c;
}
*/
#define NEAR_PLANE_DIST 4.0f
void main(void) 
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	float dx = (storePos.x - 400) / 400.0f; 
	float dy = (storePos.y - 400) / 400.0f;
	
	// A point in the near plane
	vec3 s = camera_pos + camera_dir * NEAR_PLANE_DIST + dx * camera_right + dy * camera_up;	
	
	// Hardcoded geometry data (1 triangle)
	vec3 x = vec3(-0.5f, -0.5f, 0.5f);
	vec3 y = vec3(0.5f, -0.5f, 0.5f);
	vec3 z = vec3(0.5f, 0.5f, 0.5f);
	
	float t;
	vec4 color;
	vec3 ray_dir = camera_pos + (s - camera_pos) * NEAR_PLANE_DIST;
	//vec3 intersectionPoint = camera_pos = camera_dir * t;
	if(RayVsTriangle(camera_pos, ray_dir, x, y, z, t)) {
	//if(RayVsTriangle(vec3(0.25, -0.25, 0), vec3(0.0, 0.0, 1.0), x, y, z, t)) {
		color = vec4(0.0, 1.0, 1.0, 1.0);			
	} else {
		color = vec4(0.0, 1.0, 0.0, 1.0);
	}
	imageStore(outTexture, storePos, color);
	
	/*
	vec4 color = vec4(0.0, 0.0, 1.0, 1.0);
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	imageStore(outTexture, storePos, color);
	*/
}
