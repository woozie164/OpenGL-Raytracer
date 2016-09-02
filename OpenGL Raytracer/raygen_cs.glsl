void main(void) 
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	if(storePos.x >= windowWidth) return;	
	
/*	
	vec2 tempStorePos = vec2(storePos);
	tempStorePos.x /= windowWidth;
	tempStorePos.y /= windowHeight;
	imageStore(outTexture, storePos, vec4(tempStorePos, 1.0, 0.0));	
*/
	float halfWindowWidth = windowWidth / 2.0f;
	float halfWindowHeight = windowHeight / 2.0f;
	float dx = (storePos.x - halfWindowWidth) / halfWindowWidth; 
	float dy = (storePos.y - halfWindowHeight) / halfWindowHeight;
	
	// A point in the near plane
	vec3 s = camera_pos + camera_dir * NEAR_PLANE_DIST + dx * camera_right + dy * camera_up;	
	
	vec3 ray_dir = normalize(s - camera_pos);		
	rays[storePos.x + storePos.y * windowHeight] = ray(camera_pos, ray_dir, vec3(1.0, 0.0, 0.0), -1, -1, vec3(0.0));		
}