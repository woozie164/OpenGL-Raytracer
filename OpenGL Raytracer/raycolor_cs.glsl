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
layout (binding = 1) uniform sampler2D meshSampler;
layout (binding = 1) buffer ray_buffer { ray rays[]; };

uniform vec3 camera_pos;
uniform vec3 camera_dir;
uniform vec3 camera_up;
uniform vec3 camera_right;

uniform int num_lights;

// This uniform buffer should probably be shared, since i want to share it 
// betweeen several shader programs. But I think it's shared by default.
layout (binding = 0) uniform LightsBuffer {
 light lights[10];
};

uniform vec3 light_position;
uniform vec3 light_color;

#define NEAR_ZERO 1e-20 // 1 * 10^-20
#define NEAR_PLANE_DIST 1.0f

// Compute barycentric coordinates (u, v, w) for
// point p with respect to triangle (a, b, c)
void CartesianToBarycentricCoord(vec3 a, vec3 b, vec3 c,
 vec3 p, out float u, out float v, out float w)
{
	vec3 v0 = b - a, v1 = c - a, v2 = p - a;
	float d00 = dot(v0, v0);
	float d01 = dot(v0, v1);
	float d11 = dot(v1, v1);
	float d20 = dot(v2, v0);
	float d21 = dot(v2, v1);
	float denom = d00 * d11 - d01 * d01;
	v = (d11 * d20 - d01 * d21) / denom;
	w = (d00 * d21 - d01 * d20) / denom;
	u = 1.0f - v - w;
}

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
	float squaredRadius = s.r * s.r;
    //If the distance is less than the squared radius of the sphere...
    if(squaredDist <= squaredRadius)
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
    float c = dot(origin - s.pos, origin - s.pos) - squaredRadius; // c = (o-C)^2-R^2

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
	
	// Hardcoded geometry data (1 triangle)
	vec3 x = vec3(-0.5f, -0.5f, 0.5f);
	vec3 y = vec3(0.5f, -0.5f, 0.5f);
	vec3 z = vec3(0.5f, 0.5f, 0.5f);
	
	// And 1 sphere
	sphere my_sphere = sphere(vec3(-5.0f, -5.0f, -5.0f), 1.0f);	
	
	if(RayVsTriangle(r.origin, r.dir, x, y, z, t)) {
		if(t > 0 && t < t_min /*&& r.primitiveID != 0*/) {
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
		/*
		if(t < 0) {
			t = max(t0, t1);
		}*/
		if(t > 0 && t < t_min /*&& r.primitiveID != 1*/) {
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
		// Select the smallest positive t-value.
		t = min(t0, t1);
		/*		
		if(t < 0) {
			t = max(t0, t1);
		}*/
		if(t > 0 && t < t_min /*&& r.primitiveID != 3*/) {
			t_min = t;
			r.primitiveID = 3;
			r.color = vec3(0.0, 0.3, 0.3);
			
			// First calculate the intersection point of the ray and the sphere
			vec3 p = r.origin + r.dir * t_min;
			
			// Then calculate the surface normal of the sphere
			n = normalize(p - my_sphere2.pos);							
		}
	}	
	
	// This overwrites the old t-value. Should I check the value before writing?
	// Probably shouldn't, since each trace will set the inital t to infinity 
	// before finding the closest t-value
	r.t = t_min;
	
	// Update the ray position to the closest intersection point with the geometry
	r.origin = r.origin + r.dir * t_min;
	
	// Update ray direction
	r.dir = r.dir - 2 * dot(r.dir, n) * n;
	
	r.n = n;
}

void main()
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	int i = storePos.x + storePos.y * 800;
	// Load the current pixel color, because it might contain intermediate
	// color calculations
	vec3 finalColor = vec3(imageLoad(outTexture, storePos));
	ray lightRay;		
	bool shadowed = true;
	vec3 light = vec3(0.0);
	
	if(rays[i].primitiveID != -1) {			
		for(int a = 0; a < num_lights; a++) 
		{			
			// No need to calcalulte the intersection point, because
			// this is already done in the intersection stage
			//vec3 intersectionPoint = rays[i].origin + rays[i].dir * rays[i].t;
			int lightPrimitiveID = rays[i].primitiveID;
			
			// if collision with triangle
			// figure out where in the texture to retrieve pixel data
			//if(lightRay.primitiveID == 0) {
			if(lightPrimitiveID == 0) {
				// Some hardcoded uv-coordinates
				vec2 uv_x = vec2(0.0);
				vec2 uv_y = vec2(0.0, 1.0);
				vec2 uv_z = vec2(1.0, 1.0);	
				vec3 p = rays[i].origin;
				// Hardcoded geometry data (1 triangle)
				vec3 x = vec3(-0.5f, -0.5f, 0.5f);
				vec3 y = vec3(0.5f, -0.5f, 0.5f);
				vec3 z = vec3(0.5f, 0.5f, 0.5f);
				float u, v, w;
				CartesianToBarycentricCoord(x, y, z, p, u, v, w);
				vec2 uv = vec2(u * uv_x + v * uv_y + w * uv_z);
				
				// Renders a green triangle. Was expecting to see half of img_test.png
				lightRay.color = vec3(texture(meshSampler, vec2(0.0)));				
				
				// Gives the texture as a "reflection" inside the triangle
				//lightRay.color = vec3(imageLoad(meshTexture, storePos));				
				
				// Gives a triangle with 3 different colors. Looks right IMO.
				//lightRay.color = vec3(u, v, w);				
				
				// Should give a triangle that's red, green and black in
				// each corner, but it gives a completely black triangle.
				//lightRay.color = vec3(uv, 0);				
			}				
			/*	
			vec3 lightDir = normalize(lights[a].pos - rays[i].origin);			
			
			lightRay = ray(rays[i].origin + lightDir * 0.001, lightDir, rays[i].color, -1, lightPrimitiveID, vec3(0.0));
			
			trace(lightRay);			
			
			// The trace function doesn't count the intersections between the ray and the
			// primitive it was created from, so if this is true
			// it means that the light ray didn't intersect with some other primitive 
			//if(lightPrimitiveID == lightRay.primitiveID) {		

			// if t is set to infinity, there's were no collision between
			// the lightRay and the geometry
			if(lightRay.t == 1.0 / 0.0) {				
				shadowed = false;
				// Makes the dark side of the sphere the ambient color
				//float diffuse = max(0.0, dot(rays[i].n, lightDir));
									
				// Makes the dark side of the sphere completely dark
				float diffuse = dot(rays[i].n, lightDir);
				
				float k;					
				//if(diffuse > 0) {
					// view vector, i.e. the unit vector from the surface point to the eye position
					vec3 v = normalize(camera_pos - rays[i].origin);			
					
					// The incident vector
					vec3 I = lightDir * -1;		
					
					// reflection vector
					vec3 r = normalize(reflect(I, rays[i].n));
					
					//k = max(dot(v, reflect(I, rays[i].dir)), 0);
					k = pow(max(dot(v, r), 0), 30);
					//k = max(dot(v, rays[i].dir), 0);
				/*	
				} else {
					k = 0;
				}*/	
				/*
				// Light attenuation
				float d = length(lights[a].pos - rays[i].origin);
				light += (lights[a].color * diffuse + lights[a].color * k) / d;
				
				//lightRay.color = rays[i].color + vec3(1.0, 0.0, 0.0) * k;			
				//lightRay.color = rays[i].color + lights[a].color * diffuse;
			} else {
				//shadowed = true;
				// Shadow color
				// Shadows on the backside of geometry doesn't work because 
				// I don't count self-intersections
				//finalColor = vec3(0.2);	
				//light -= vec3(0.1);				
			}	*/				
		}
		finalColor = lightRay.color + light;
		
		// This makes things look a lot buggier for some reason.
		// Should give the same result as the other one.
		//finalColor += lightRay.color + light;
	} else {
		// Background color
		finalColor = vec3(0.0, 0.3, 0.0);
	}
	
	if(shadowed) {
		//finalColor = vec3(0.1);
	}
	//finalColor = vec3(imageLoad(meshTexture, storePos));				
	//imageStore(outTexture, storePos, vec4(lights[0].color, 1.0));	
	imageStore(outTexture, storePos, vec4(finalColor, 1.0));
}