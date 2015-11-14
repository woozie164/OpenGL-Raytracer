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
void trace(in out ray r) {	
	// Set initial value to infinity
	float t_min = 1.0 / 0.0;	
	float t;
	vec3 n;
	
	for(int i = 0; i < num_vertices / 3; i += 3) {
		vertex x = vertices[i];
		vertex y = vertices[i+1];
		vertex z = vertices[i+2];
		if(RayVsTriangle(r.origin, r.dir, x.pos, y.pos, z.pos, t)) {
			if(t > 0 && t < t_min) {
				t_min = t;
				r.primitiveID = 0;
				r.color = vec3(0.0);
				
				// Calculate the surface normal of the triangle			
				vec3 u = x.pos - z.pos;
				vec3 v = y.pos - z.pos;
				n = normalize(cross(u, v));
			}
		}
	}
	// Hardcoded geometry data (1 triangle)
	
	vec3 x = vec3(-0.5f, -0.5f, 0.5f);
	vec3 y = vec3(0.5f, -0.5f, 0.5f);
	vec3 z = vec3(0.5f, 0.5f, 0.5f);
	
	// And 1 sphere
	sphere my_sphere = sphere(vec3(-5.0f, -5.0f, -5.0f), 1.0f);	
	
	if(RayVsTriangle(r.origin, r.dir, x, y, z, t)) {
		if(t > 0 && t < t_min && r.primitiveID != 0) {
			t_min = t;
			r.primitiveID = 0;
			r.color = vec3(0.0, 1.0, 0.0);
			
			// Calculate the surface normal of the triangle			
			vec3 u = x - z;
			vec3 v = y - z;
			n = normalize(cross(u, v));
		}
	}	
	
	
	
	float t0, t1;
	if(intersectSphere(r.origin, r.dir, my_sphere, t0, t1)){
		t = min(t0, t1);
		if(t > 0 && t < t_min && r.primitiveID != 1) {
			t_min = t;
			r.primitiveID = 1;
			r.color = vec3(0.0, 0.0, 1.0);
			
			// First calculate the intersection point of the ray and the sphere
			vec3 p = r.origin + r.dir * t_min;
			
			// Then calculate the surface normal of the sphere
			n = normalize(p - my_sphere.pos);							
		}
	}

	sphere my_sphere2 = sphere(vec3(-3.0f, -3.0f, -3.0f), 0.5f);	
	
	if(intersectSphere(r.origin, r.dir, my_sphere2, t0, t1)){
		t = min(t0, t1);
		if(t > 0 && t < t_min && r.primitiveID != 3) {
			t_min = t;
			r.primitiveID = 3;
			r.color = vec3(0.0, 0.3, 0.3);
			
			// First calculate the intersection point of the ray and the sphere
			vec3 p = r.origin + r.dir * t_min;
			
			// Then calculate the surface normal of the sphere
			n = normalize(p - my_sphere2.pos);							
		}
	}	
	
	r.t = t_min;
	
	// Update the ray position to the closest intersection point with the geometry
	r.origin = r.origin + r.dir * t_min;
	
	// Update ray direction
	r.dir = r.dir - 2 * dot(r.dir, n) * n;
	
	r.n = n;
}

void main(void) 
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	ray my_ray = rays[storePos.x + storePos.y * 800];	
	trace(my_ray);
	rays[storePos.x + storePos.y * 800] = my_ray;
}