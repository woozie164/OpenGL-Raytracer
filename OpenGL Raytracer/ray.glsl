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