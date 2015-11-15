void main(void) 
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	ray my_ray = rays[storePos.x + storePos.y * 800];
	
	// Move the ray's origin point slighly forward so that 
	// it avoids interesections with the primitive that spawned it.
	my_ray.origin = my_ray.origin + my_ray.dir * 0.001;
		
	trace(my_ray);
	rays[storePos.x + storePos.y * 800] = my_ray;
}