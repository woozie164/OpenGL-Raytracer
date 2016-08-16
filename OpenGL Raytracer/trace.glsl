//#include ray.glsl
//#include sphere.glsl
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

bool RaySphereIntersect(vec3 o, vec3 d, vec3 c, float r, out float t)
{
	vec3 l = c - o;
	float s = dot(l, d);
	
	// The length of l squared
	float l_sqr = dot(l, l);
	float r_sqr = r * r;
	bool rayOutsideSphere = l_sqr > r_sqr;
	if(s < 0 && rayOutsideSphere) return false;
	
	float m_sqr = l_sqr - s * s;
	if(m_sqr > r_sqr) return false;
	
	float q = sqrt(r_sqr - m_sqr);	
	if(rayOutsideSphere) {
		t = s - q;
	} else {
		t = s + q;
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
void trace(in out ray r, bool earlyExit = false) {	
	// Set initial value to infinity
	r.t = 1.0 / 0.0;	
	float t;
	
	// Nvidia insight gave a warning that lightRay.color might sometimes
	// be unitialized. Not sure how this affects the rendered image,
	// or what to do about it.
	// 
	// Ray color is only unitialized when the ray doesn't hit any geometry. In that case, the color value 
	// doesn't matter, because the pixel is set to some arbitrary background color.
	//r.color = vec3(1.0, 1.0, 1.0);
	
	int primitiveID = -1;
	
	for(int i = 0; i < num_vertices; i += 3) 
	{
		vertex x = vertices[i];
		vertex y = vertices[i+1];
		vertex z = vertices[i+2];
		if(RayVsTriangle(r.origin, r.dir, x.pos, y.pos, z.pos, t)) 
		{
			if(t > 0 && t < r.t) 
			{
				r.t = t;
				r.primitiveID = primitiveID;
				r.color = vec3(0.0);
				
				// Calculate the surface normal of the triangle				
				vec3 u = x.pos - z.pos;
				vec3 v = y.pos - z.pos;
				r.n = normalize(cross(u, v));
				
				if(earlyExit) return;
			}
		}
		primitiveID++;
	}
	
	const int num_spheres = 2;
	sphere spheres[num_spheres];
	spheres[0] = sphere(vec3(0.0f, -0.7f, 2.0f), 0.5f);	
	spheres[1] = sphere(vec3(-2.0f, -00f, 0.0f), 0.2f);	

	for(int i = 0; i < num_spheres; i++)
	{	
		if(RaySphereIntersect(r.origin, r.dir, spheres[i].pos, spheres[i].r, t)) 
		{
			//t = min(t0, t1);
			if(t > 0 && t < r.t) {
				r.t = t;	
				r.color = vec3(0.0, 0.0, 1.0);
				
				// First calculate the intersection point of the ray and the sphere
				vec3 p = r.origin + r.dir * r.t;
				
				// Then calculate the surface normal of the sphere
				r.n = normalize(p - spheres[i].pos);							
				
				if(earlyExit) return;
			}
		}	
	}
}