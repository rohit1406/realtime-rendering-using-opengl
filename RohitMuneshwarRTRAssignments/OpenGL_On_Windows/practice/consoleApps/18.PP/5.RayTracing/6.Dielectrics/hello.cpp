#include<iostream>
#include "sphere.h"
#include "hitable_list.h"
#include "camera.h"
#include "float.h"
#include "material.h"

//this function neatly blends white and blue depending on
//the up/downness of the y coordinate
vec3 color(const ray &r, hitable *world, int depth)
{
	//function prototypes
	

	//local variable declaration
	hit_record rec;

	//code
	if (world->hit(r, 0.0, FLT_MAX, rec))
	{
		ray scattered;
		vec3 attenuation;

		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered))
		{
			return attenuation * color(scattered, world, depth+1);
		}
		else
		{
			return vec3(0, 0, 0);
		}
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
	int nx = 200; //columns
	int ny = 100; //rows
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

	hitable *list[4];
	list[0] = new sphere(vec3(0,0,-1), 0.5 , new lambertian(vec3(0.1, 0.2, 0.5)));
	list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
	list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 1.0));
	list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(0.8));
	hitable *world = new hitable_list(list, 4);

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
				col += color(r, world,0);
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