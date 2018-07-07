#include<iostream>

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

	//RGB triplets
	for (int j=ny-1; j >= 0; j--) //width
	{
		for (int i = 0; i < nx; i++) //height
		{
			
			float u = float(i) / float(nx);
			float v = float(j) / float(ny);
			float w = 0.2;
			
			int ir = int(255.99 * u);
			int ig = int(255.99 * v);
			int ib = int(255.99 * w);

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