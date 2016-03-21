//#include definitions.glsl
//#include trace.glsl

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

void main()
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	if(storePos.x > windowWidth) return;
	int i = storePos.x + storePos.y * windowHeight;
	// Load the current pixel color, because it might contain intermediate
	// color calculations
	vec3 finalColor = vec3(imageLoad(outTexture, storePos));
	//vec3 finalColor = vec3(0.0, 0.0,  0.0);
	ray lightRay;		
	bool shadowed = true;
	vec3 light = vec3(0.0);
	vec3 texColor = vec3(0.0);
		
	if(!isinf(rays[i].t)) {
		int lightPrimitiveID = rays[i].primitiveID;				
		vertex a = vertices[lightPrimitiveID * 3];
		vertex b = vertices[lightPrimitiveID * 3 + 1];
		vertex c = vertices[lightPrimitiveID * 3 + 2];
	
		float u, v, w;
		CartesianToBarycentricCoord(a.pos, b.pos, c.pos, rays[i].origin, u, v, w);
		vec2 uv = vec2(u * a.texCoord + v * b.texCoord + w * c.texCoord);
					
		ivec2 meshTexSize = imageSize(meshTexture);			
		ivec2 pixelCoord = ivec2(uv * meshTexSize);			
		texColor = vec3(imageLoad(meshTexture, pixelCoord));
		
		// Should be able to use a sampler2D, but I always get a black triangle
		//lightRay.color = vec3(texture(meshSampler, uv));				
		
		// Gives the texture as a "reflection" inside the triangle
		//lightRay.color = vec3(imageLoad(meshTexture, storePos));				
		
		// Gives a triangle with 3 different colors. Looks right IMO.
		//lightRay.color = vec3(u, v, w);					
		
		for(int a = 0; a < num_lights; a++) 
		{			
			// No need to calculate the intersection point, because
			// this is already done in the intersection stage
			//vec3 intersectionPoint = rays[i].origin + rays[i].dir * rays[i].t;
			
			// A vector from the intersection point pointing towards the light
			vec3 lightDir = normalize(lights[a].pos - rays[i].origin);			
			
			lightRay = ray(rays[i].origin + lightDir * 0.001, lightDir, rays[i].color, -1, lightPrimitiveID, vec3(0.0));
			
			trace(lightRay, true);			
			
			// The trace function doesn't count the intersections between the ray and the
			// primitive it was created from, so if this is true
			// it means that the light ray didn't intersect with some other primitive 
			//if(lightPrimitiveID == lightRay.primitiveID) {		
			
			// if t is set to infinity, there's were no collision between
			// the lightRay and the geometry
			if(lightRay.t == 1.0 / 0.0) {	
				shadowed = false;				
				float diffuse = max(dot(rays[i].n, lightDir), 0);				
				
				// view vector, i.e. the unit vector from the surface point to the eye position
				vec3 v = normalize(camera_pos - rays[i].origin);			
				
				// The incident vector
				vec3 I = lightDir * -1;		
				
				// reflection vector
				float k = 0;
				//if(dot(rays[i].n, lightDir) > 0)  // makes the highlights on the walls dissapear
				{
				vec3 r = normalize(reflect(I, rays[i].n));
				
				k = pow(max(dot(v, r), 0), 30);
				}
				// Distance to the light source
				float d = length(lights[a].pos - rays[i].origin);
				
				// Linear Light attenuation
				float attentuation = 1.0 / (1.0 * d);
				attentuation = 1;
				//float attentuation = 1.0 / (1.0 + 0.0 * d + 0.9 * d * d);				
				//float attentuation = 1.0;
				
				//light += (lights[a].color * diffuse + lights[a].color * k) / d;
				light += ((diffuse + k) * lights[a].color); // test with different diffuse and specular constants?
				//light += ((diffuse + k);
				//light += lights[a].color;
			}		
		}
		// No ambient color
		//finalColor += light;
		if(shadowed == false) {
			finalColor += texColor + light;			
		}
	} else {
		// Background color
		finalColor = vec3(1.0, 0.0, 0.0);
	}

	
	imageStore(outTexture, storePos, vec4(finalColor, 1.0));
}