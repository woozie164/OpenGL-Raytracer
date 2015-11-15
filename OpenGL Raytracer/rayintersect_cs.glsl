void main(void) 
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	ray my_ray = rays[storePos.x + storePos.y * 800];	
	trace(my_ray);
	rays[storePos.x + storePos.y * 800] = my_ray;
}