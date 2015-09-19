#version 430 core

layout (local_size_x = 1, local_size_y = 1) in;

void RayVsTriangle(vec3 ray_origin, vec3 ray_dir,
 vec3 tri_x, vec3 tri_y,  vec3 tri_z)
{
	
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

void main(void) 
{
		
}
