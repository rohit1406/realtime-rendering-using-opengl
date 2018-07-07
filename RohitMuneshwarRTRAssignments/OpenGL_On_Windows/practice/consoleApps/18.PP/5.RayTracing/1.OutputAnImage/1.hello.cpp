#include<iostream>
#include "ray.h"

//this function neatly blends white and blue depending on
//the up/downness of the y coordinate
vec3 color(const ray &r)
{
	//function prototypes
	bool hit_sphere(const vec3&, float, const ray&);

	//code
	if (hit_sphere(vec3(0,0,-1), 0.5, r))
	{
		return vec3(1, 0, 0);
	}

	//make a unit vector so value is between -1<y<1
	vec3 unit_direction = unit_vector(r.direction());

	//scale the values of above unit vector to 0<t<1
	float t = 0.5 * (unit_direction.y() + 1.0);

	//when t=1, I want blue and when t=0, I want white and inbetween I want to blend
	//this forms a linear blend or linear interpolation or lerp in short between two things
	//A lerp is always of the form:
	//blended_value = (1-t)*start_value + t*end_value, with t going from 0 to 1
	return ((1.0-t) * vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0));
}

/*
	Add Sphere into ray tracers
	Because calculating whether a ray hits a sphere is pretty straightforward.

	Equation for a sphere centered at the origin of Radius R is
	x*x + y*y + z*z = R*R
	We can read it as, "For any (x,y,z), if x*x + y*y + z*z = R*R then (x,y,z) is on the sphere otherwise it is not".

	if center is at (cx, cy, cz): (x-cx)*(x-cx) + (y-cy)*(y-cy) + (z-cz)*(z-cz) = R*R.

	here all the xyz stuffs are in a vector vec

	vector from center C(cx,cy,cz) to point P(px,py,pz) is (P-C)
	And dot((P-C),(P-C)) =  (x-cx)*(x-cx) + (y-cy)*(y-cy) + (z-cz)*(z-cz);

	So, dot((p-c),(p-c)) = R*R
	So, we can say that, any point p that satisfies this equation is on the sphere.

	we want to know if our ray p(t)=A+t*B ever hits the sphere anywhere. If it does hit, there is some t
	for which p(t) satisfies the sphere equation.

	So we are looking for any t where this is true:
	dot((p(t)-c),(p(t)-c)) = R*R

	by expanding the full form of the ray p(t)
	dot((A+t*B - c), (A+t*B - c)) = R*R

	by expanding the above equation using algebra, we get:
	t*t*dot(B,B) + 2*t*dot(B,A-C) _ dot(A-C, A-C) - R*R = 0;

	in above equation, vectors and R are constant and known, only t is varying.

	The equation is a quadratic, like you probably saw in your high school math class. You can solve for t
	and there is a square root part that is either positive (meaning two real solutions), negative
	(meaning no real solutions), or zero (meaning one real solution).
	In graphics, the algrebra almost always relates very directly to the geometry.
	What we have is:
	0 roots - not touching sphere
	1 root - touching sphere like a tangent
	2 roots - passing though the sphere
	
	*/
bool hit_sphere(const vec3& center, float radius, const ray& r)
{
	//code
	vec3 oc = r.origin() - center;

	float a = dot(r.direction(), r.direction());

	float b = 2.0 * dot(oc, r.direction());

	float c = dot(oc, oc) - radius*radius;

	float discriminant = b*b - 4 * a*c;

	return (discriminant > 0);
}
int main()
{
	
	//local variable declaration
	int nx = 200; //columns
	int ny = 100; //rows

	//output file
	FILE *lPpmFile = NULL;
	
	lPpmFile = fopen("ppmFile.ppm","w");

	//check if file is created
	if (!lPpmFile)
	{
		printf("File is not created...\n");
	}

	//P3 is a magic number of ppm file
	//nx is number of columns and ny is number of rows i.e. width and height of the image in the pixels
	//255 for max color
	//std::cout << "P3\n" << nx << " " << ny << "\n255\n";
	fprintf(lPpmFile, "P3\n%d %d\n255\n", nx, ny);

	vec3 lower_left_corner(-2.0, -1.0, -1.0);
	vec3 horizontal(4.0, 0.0, 0.0);
	vec3 vertical(0.0, 2.0, 0.0);
	vec3 origin(0.0, 0.0, 0.0);

	//RGB triplets
	for (int j=ny-1; j >= 0; j--) //width
	{
		for (int i = 0; i < nx; i++) //height
		{
			
			float u = float(i) / float(nx);
			float v = float(j) / float(ny);
			ray r(origin, lower_left_corner + u*horizontal + v*vertical);
			vec3 col = color(r);
			int ir = int(255.99 * col[0]);
			int ig = int(255.99 * col[1]);
			int ib = int(255.99 * col[2]);

			//std::cout << ir << " " << ig << " " << ib << "\n";
			fprintf(lPpmFile, "%d %d %d\n",ir, ig, ib);
		}
	}

	//close the file
	if (lPpmFile)
	{
		fclose(lPpmFile);
		lPpmFile = NULL;
	}
}