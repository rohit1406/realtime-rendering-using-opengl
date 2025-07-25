#ifndef MATERIALH
#define MATERIALH
#include "ray.h"
#include "hitable.h"

vec3 random_in_unit_sphere()
{
	vec3 p;
	do
	{
		double d = rand() / (RAND_MAX + 1.0);
		p = 2.0 * vec3(d, d, d) - vec3(1, 1, 1);
	} while (p.squared_length() >= 1.0);
	return p;
}

class material
{
public:
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;
};

class lambertian : public material
{
public:
	lambertian(const vec3& a) : albedo(a)
	{

	}

	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
	{
		vec3 random_in_unit_sphere();

		vec3 target = rec.p + rec.normal + random_in_unit_sphere();
		scattered = ray(rec.p, target-rec.p);
		attenuation = albedo;
		return true;
	}

	vec3 albedo;
};

vec3 reflect(const vec3& v, const vec3& n)
{
	return v - 2 * dot(v, n) * n;
}

class metal : public material
{
public:
	metal(const vec3& a) : albedo(a)
	{

	}

	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
	{
		vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
		scattered = ray(rec.p, reflected);
		attenuation = albedo;
		return (dot(scattered.direction(), rec.normal) > 0);
	}

	vec3 albedo;
};
#endif // !MATERIALH
