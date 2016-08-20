void main(void) 
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	if(storePos.x >= windowWidth) return;
	int i = storePos.x + storePos.y * windowHeight;
	ray r = rays[i];
		
	if(r.primitiveID == -2) return;
		
	// Move the ray's origin point slighly forward so that 
	// it avoids interesections with the primitive that spawned it.
	r.origin = r.origin + r.dir * 0.001;
		
	trace(r);
				
	r.origin = r.origin - r.dir * 0.001;				
	if(isinf(r.t)) 
	{
		r.primitiveID = -2;
	}
	
	// Update the ray position to the closest intersection point with the geometry
	r.origin = r.origin + r.dir * r.t;
	
	// Update ray direction
	//r.dir = 2 * dot(r.dir, r.n) * r.n - r.dir; // Alternative version, changes the dir of the resulting vector.
	r.dir = r.dir - 2 * dot(r.dir, r.n) * r.n;	
				
	float d = distance(r.origin, camera_pos);
	//d = (d - NEAR_PLANE_DIST) / FAR_PLANE_DIST; // Scaling seems to be handled automatically if the texture is set as an DEPTHCOMPONENT_24
	imageStore(depthTexture, storePos, vec4(d)); // writes a vector of 32 bit floats to a 16 bit float pixel ... converts correctly?
	
	rays[i] = r;
}