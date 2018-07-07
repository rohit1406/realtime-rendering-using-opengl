#ifndef HITABLEH
#define HITABLEH

#include "ray.h"
#include "material.h"
class material;

struct hit_record
{
	float t;
	vec3 p;
	vec3 normal;
	material* mat_prt;	
};



#endif // !HITABLEH
