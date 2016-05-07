void main(void) 
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	if(storePos.x > windowWidth) return;
	int i = storePos.x + storePos.y * windowHeight;
	ray my_ray = rays[i];
		
	// Move the ray's origin point slighly forward so that 
	// it avoids interesections with the primitive that spawned it.
	my_ray.origin = my_ray.origin + my_ray.dir * 0.001;
		
	trace(my_ray);
	
	float d = distance(my_ray.origin, camera_pos);
	//d = (d - NEAR_PLANE_DIST) / FAR_PLANE_DIST; // Scaling seems to be handled automatically if the texture is set as an DEPTHCOMPONENT_24
	imageStore(depthTexture, storePos, vec4(d)); // writes a vector of 32 bit floats to a 16 bit float pixel ... converts correctly?
	
	rays[i] = my_ray;
}