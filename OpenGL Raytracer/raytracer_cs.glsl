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
bool RayVsSphere(vec3 ray_origin, vec3 ray_dir,
	vec3 sphere_center, float radius, 
	out float t) {
	vec3 l = ray_origin - sphere_center;
	float b = 2* dot(ray_dir, l);
	float c = dot(l, l) - radius * radius;
	float a = dot(camera_dir, camera_dir);
	//float x = b * b - c;
	 float x = (b*b)-(4.0 * a * c);
	//bool hit = x >= 0.0f ? true : false;
	//return hit;

	if(x < 0) {
		return false;
	} else {
		float t1 = (-b - sqrt(x)) / (2.0f * a);
		float t2 = (-b + sqrt(x)) / (2.0f * a);
		
		if(t1 > 0 || t2 > 0) 
		{
			if(t1 < t2 && t1 > 0)
				t = t1;
			else
				t = t2;
			
			return true;
		}
		return false;	
	}	
}
*/
struct sphere {
	vec3 pos;
	float r;
} my_sphere;

bool intersectSphere(vec3 origin, vec3 dir, const sphere s) {
	//Squared distance between ray origin and sphere center
    float squaredDist = dot(origin - s.pos, origin - s.pos);

    //If the distance is less than the squared radius of the sphere...
    if(squaredDist <= s.r)
    {
        //Point is in sphere, consider as no intersection existing        
        //return vec2(MAX_SCENE_BOUNDS, MAX_SCENE_BOUNDS);
		return false;
    }

    //Will hold solution to quadratic equation
    float t0, t1;

    //Calculating the coefficients of the quadratic equation
    float a = dot(dir,dir); // a = d*d
    float b = 2.0 * dot(dir, origin - s.pos); // b = 2d(o-C)
    float c = dot(origin - s.pos, origin - s.pos) - (s.r*s.r); // c = (o-C)^2-R^2

    //Calculate discriminant
    float disc = (b*b)-(4.0*a*c);

    if(disc < 0) //If discriminant is negative no intersection happens
    {
        //std::cout << "No intersection with sphere..." << std::endl;
        //return vec2(MAX_SCENE_BOUNDS, MAX_SCENE_BOUNDS);
		return false;
    }
    else //If discriminant is positive one or two intersections (two solutions) exists
    {
        float sqrt_disc = sqrt(disc);
        t0 = (-b - sqrt_disc) / (2.0f * a);
        t1 = (-b + sqrt_disc) / (2.0f * a);
		//return vec2(min(t0, t1), max(t0, t1));
		return true;
    }	
}

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
	/*
	if(RayVsTriangle(camera_pos, ray_dir, x, y, z, t)) {
	//if(RayVsTriangle(vec3(0.25, -0.25, 0), vec3(0.0, 0.0, 1.0), x, y, z, t)) {
		color = vec4(0.0, 1.0, 1.0, 1.0);			
		
	}
	*/
	my_sphere = sphere(vec3(-5.0f, -5.0f, -5.0f), 1.0f);
	//if(RayVsSphere(camera_pos, ray_dir, vec3(-5.0f, -5.0f, -5.0f), 1.0f, t)) {
	if(intersectSphere(camera_pos, ray_dir, my_sphere)) {
		color = vec4(1.0, 0.0, 0.0, 1.0f);
	} else {
		color = vec4(0.0, 1.0, 0.0, 1.0);
	}
	imageStore(outTexture, storePos, color);
}
