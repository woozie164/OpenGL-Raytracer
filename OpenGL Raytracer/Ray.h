#ifdef _WIN32
#define vec2 glm::vec2
#define vec3 glm::vec3
#define vec4 glm::vec4
#endif // _WIN32

#include "glm\glm.hpp"

struct ray {
	vec3 origin;
	float padding0;	
	vec3 dir;
	float padding1;
	vec3 color;
	float t;
	int primitiveID;
	float padding2;
	float padding3;
	float padding4;
	// the surface normal from the last primitive this ray 
	// intersected with
	vec3 n;
	float padding5;	
};

#ifdef _WIN32
#undef vec2 glm::vec2
#undef vec3 glm::vec3
#undef vec4 glm::vec4
#endif // _WIN32
