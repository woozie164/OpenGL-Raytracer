#version 430 core

layout (local_size_x = 1, local_size_y = 1) in;
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
} my_sphere;

// Hardcoded geometry data (1 triangle)
vec3 x = vec3(-0.5f, -0.5f, 0.5f);
vec3 y = vec3(0.5f, -0.5f, 0.5f);
vec3 z = vec3(0.5f, 0.5f, 0.5f);

bool RayVsTriangle(vec3 ray_origin, vec3 ray_dir,
 vec3 tri_x, vec3 tri_y,  vec3 tri_z,
 out float t)
{
	vec3 e1 = tri_y - tri_x;
	vec3 e2 = tri_z - tri_x;
	vec3 q = cross(ray_dir,  e2);
	float a = dot(e1, q);
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

bool RayVsSphere(vec3 ray_origin, vec3 ray_dir,
	vec3 sphere_center, float radius, 
	out float t) {
	vec3 l = ray_origin - sphere_center;
	float b = dot(ray_dir, l); // 2* dot(ray_dir, l);
	float c = dot(l, l) - radius * radius;
	float a = dot(camera_dir, camera_dir);
	float x = b * b - c;
	// float x = (b*b)-(4.0 * a * c);
	//bool hit = x >= 0.0f ? true : false;
	//return hit;

	if(x < 0) {
		return false;
	} else {
	//	float t1 = (-b - sqrt(x)) / (2.0f * a);
		//float t2 = (-b + sqrt(x)) / (2.0f * a);
		float t1 = (-b - sqrt(x));
		float t2 = (-b + sqrt(x));
		
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

bool RaySphereIntersect(ray r, sphere s, out float t) {
	vec3 l = s.pos - r.origin;
	//vec3 l = r.origin - s.pos;
	float a = dot(l, r.dir);
	float l_squared = dot(l, l);
	float r_squared = s.r * s.r;
	if(a < 0 && l_squared > r_squared) return false;
	
	float m_squared = l_squared - a * a;
	if(m_squared > r_squared) return false;
	
	float q = sqrt(r_squared - m_squared);
	if(l_squared > r_squared) {
		t = a - q;
	} else {
		t = a + q;
	}
	return true;
}

bool intersectSphere(vec3 origin, vec3 dir, const sphere s, out float t0, out float t1) {
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
    //float t0, t1; // Replaced with out paramaters

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

// Do intersection tests with all the geometry in the scene
void trace(vec3 ray_origin, vec3 ray_dir, out float t, out int primitiveID) {	
	// Set initial value to infinity
	// Is this positive or negative infinity??
	float t_min = 1.0 / 0.0;
	
	//float t_min = 99999999999999999.0f;
	
	// -1 indicates that this ray haven't intersected anything
	primitiveID = -1;
	
	if(RayVsTriangle(ray_origin, ray_dir, x, y, z, t)) {
		if(t > 0 && t < t_min) {
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
		if(t > 0 && t < t_min) {
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

#define NEAR_PLANE_DIST 1.0f
void main(void) 
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	float dx = (storePos.x - 400) / 400.0f; 
	float dy = (storePos.y - 400) / 400.0f;
	
	// A point in the near plane
	vec3 s = camera_pos + camera_dir * NEAR_PLANE_DIST + dx * camera_right + dy * camera_up;	
	
	vec3 ray_dir = normalize(s - camera_pos);		

	float t;
	int primitiveID;
	vec4 color = vec4(0.0);
	
	trace(camera_pos, ray_dir, t, primitiveID);
	if(primitiveID != -1) {
		if(primitiveID == 0) {
			color = vec4(0.0, 0.0, 1.0, 1.0);
		}
		if(primitiveID == 1) {
			color = vec4(1.0, 0.0, 0.0, 1.0);
		}
		
		vec3 intersectionPoint = camera_pos + ray_dir * t;
		vec3 lightRayDir = normalize(light_position - intersectionPoint);
		int lightPrimitiveID;
		trace(intersectionPoint, lightRayDir, t, lightPrimitiveID);
		if(lightPrimitiveID != -1 && lightPrimitiveID != primitiveID) {
			color = vec4(light_color, 1.0f);			
		} else {
			color = vec4(0.3, 0.3, 0.3, 1.0);	
		}
		
	} else {
		// Background color
		color = vec4(0.0, 1.0, 0.0, 1.0);
	}
	
	
	/*
	if(RayVsTriangle(camera_pos, ray_dir, x, y, z, t)) {
	//if(RayVsTriangle(vec3(0.25, -0.25, 0), vec3(0.0, 0.0, 1.0), x, y, z, t)) {
		color = vec4(0.0, 1.0, 1.0, 1.0);			
		
	}	
	*/
	//RayVsTriangle(camera_pos, ray_dir, x, y, z, t);
		/*
	my_sphere = sphere(vec3(-5.0f, -5.0f, -5.0f), 1.0f);
	int primitiveId = -1;	
	if(RayVsSphere(camera_pos, ray_dir, vec3(-5.0f, -5.0f, -5.0f), 1.0f, t)) {
		primitiveId = 0;
	//if(intersectSphere(camera_pos, ray_dir, my_sphere, t0, t1)) {
		
		//**Light pass** 		
		// Send a ray from each intersection point towards each light source
		// if there's nothing in the way between the light source and the intersection point -> color this pixel with the light source color
		// else color this pixel black (because there's an object in the way of the light)
		
		//t = min(t, min(t0, t1));
		//t = min(t0, t1);
		vec3 intersectionPoint = camera_pos + ray_dir * t;
		vec3 lightRayDir = normalize(light_position - intersectionPoint);
		float t0, t1, t3;
		
		if(!RayVsTriangle(intersectionPoint, lightRayDir, x, y, z, t3)/* && !intersectSphere(intersectionPoint, lightRayDir, my_sphere, t0, t1)) {			
			color = vec4(light_color, 1.0f);			
		} else {		
			color = vec4(0.0, 0.0, 0.0, 1.0);
		}					
	} else {
		color = vec4(0.0, 1.0, 0.0, 1.0);
	}
	*/
	/*
	float min_t = min(t, min(t0, t1));
	if(min_t == t) {
		color = vec4(0.0, 1.0, 1.0, 1.0);	
	} else if(min_t == t0){
		color = vec4(light_color, 1.0f);
	} else if(min_t == t1){
		color = vec4(1.0, 1.0, 1.0, 1.0);
	} else {
		color = vec4(0.0, 1.0, 0.0, 1.0);
	}
	*/
	imageStore(outTexture, storePos, color);
}
