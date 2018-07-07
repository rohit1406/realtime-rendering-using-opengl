#include<iostream>
#include "sphere.h"
#include "hitable_list.h"
#include "camera.h"
#include "float.h"


vec3 random_in_unit_sphere()
{
	vec3 p;
	do
	{
		double d = rand() / (RAND_MAX + 1.0);
		p = 2.0 * vec3(d,d,d) - vec3(1,1,1);
	} while (p.squared_length() >= 1.0);
	return p;
}

//this function neatly blends white and blue depending on
//the up/downness of the y coordinate
vec3 color(const ray &r, hitable *world)
{
	//function prototypes
	float hit_sphere(const vec3&, float, const ray&);
	vec3 random_in_unit_sphere();

	//local variable declaration
	hit_record rec;

	//code
	if (world->hit(r, 0.0, FLT_MAX, rec))
	{
		vec3 target = rec.p + rec.normal + random_in_unit_sphere();
		return 0.5 * color(ray(rec.p, target - rec.p), world);
	}
	else
	{
		//make a unit vector so value is between -1<y<1
		vec3 unit_direction = unit_vector(r.direction());

		//scale the values of above unit vector to 0<t<1
		float t = 0.5 * (unit_direction.y() + 1.0);

		//when t=1, I want blue and when t=0, I want white and inbetween I want to blend
		//this forms a linear blend or linear interpolation or lerp in short between two things
		//A lerp is always of the form:
		//blended_value = (1-t)*start_value + t*end_value, with t going from 0 to 1
		return ((1.0 - t) * vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0));
	}
	
}

int main()
{
	
	//local variable declaration
	int nx = 400; //columns
	int ny = 200; //rows
	int ns = 100;

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

	hitable *list[2];
	list[0] = new sphere(vec3(0,0,-1), 0.5);
	list[1] = new sphere(vec3(0, -100.5, -1), 100);

	hitable *world = new hitable_list(list, 2);

	camera cam;
	//RGB triplets
	for (int j=ny-1; j >= 0; j--) //width
	{
		for (int i = 0; i < nx; i++) //height
		{
			vec3 col(0,0,0);
			for (int s = 0; s < ns; s++) 
			{
				double d= rand() / (RAND_MAX + 1.0);
				float u = float(i + d) / float(nx);
				float v = float(j + d) / float(ny);
				ray r = cam.get_ray(u, v);
				vec3 p = r.point_at_parameter(2.0);
				col += color(r, world);
			}
			col /= float(ns);
			col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
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