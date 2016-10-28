#include <cmath>
#include <limits>
#include <sstream>
#include <iostream>
#include <fstream>
#include <istream>

#include "Region2D.h"



void Region2D::SetNSides(GLint sides)
{
	nSides = sides;
	//2D region must have at least 3 sides!
	if (nSides < 3)
		nSides = 3;
}
void Region2D::SetPolyVerts(GLfloat Verts[][2])
{
	for (int i = 0; i<nSides; i++)
	{
		Vertices[i][0] = Verts[i][0];
		Vertices[i][1] = Verts[i][1];
	}

}

void Region2D::SetOneVert(GLint i, GLint j, GLfloat vert)
{
	Vertices[i][j] = vert;
}

void Region2D::SetPolyColor(GLfloat clr[])
{
	color[0] = clr[0];
	color[1] = clr[1];
	color[2] = clr[2];
}

GLfloat Region2D::GetPolyVert(GLint i, GLint j)
{
	return Vertices[i][j];
}

GLint Region2D::GetPolySides()
{
	return nSides;
}

Region2D Region2D::LoadPolyFromFile(char* filePath)
{
	Region2D region;
	
	int nSides = 0;
	GLfloat tmpclr[3];
	GLfloat tmpverts[10][2];
	char tmpline[50] = ""; 

	std::ifstream regionfile(filePath);

	if (!regionfile)
	{
		std::cerr << "Cannot open input file." << std::endl;
		return(region);
	}
	else
		std::cerr << "Opened Region File " << filePath << std::endl;

	//regionfile.getline(tmpline,sizeof(tmpline),'\n');
	//if (!regionfile.eof())
	//	sscanf(tmpline,'%d', &nSides);

	regionfile.getline(tmpline,sizeof(tmpline),'\n');
	if (!regionfile.eof())
	{
		sscanf(tmpline,"%f %f %f",&tmpclr[0],&tmpclr[1],&tmpclr[2]);
		region.SetPolyColor(tmpclr);
	}
	else
		return(region);

	regionfile.getline(tmpline,sizeof(tmpline),'\n');
	while(!regionfile.eof() && nSides<10)
	{
			sscanf(tmpline, "%f %f",&tmpverts[nSides][0],&tmpverts[nSides][1]);
			nSides++;
			regionfile.getline(tmpline,sizeof(tmpline),'\n');
	}
	region.SetNSides(nSides);
	region.SetPolyVerts(tmpverts);

	return(region);

}


void Region2D::Draw()
{
			
	// Draw the polygon
	glColor3f(color[0],color[1],color[2]);
	
	glBegin(GL_TRIANGLE_FAN);

	for (int i = 0; i<nSides; i++)
	{
		glVertex3f(Vertices[i][0],Vertices[i][1], 0.0f);
	}
	glEnd();
	glColor3f(1.0f,1.0f,1.0f);
	//glClearColor(255, 255, 255, 0);
	//glEnable (GL_BLEND);
	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	
}


int Region2D::InRegion(Object2D* cursor)
{
	//ray-casting technique
	int crossings = 0;
	float x1,x2;
	double k;


	//iterate through each size of the polygon
	for (int i = 0; i < nSides; i++)
	{
		//make sure it doesn't matter if the line goes from left to right or right to left
		if( Vertices[i][0] < Vertices[(i+1)%nSides][0] )
		{
			x1 = Vertices[i][0];
			x2 = Vertices[(i+1)%nSides][0];
		}
		else
		{
			x1 = Vertices[(i+1)%nSides][0];
			x2 = Vertices[i][0];
		}

		//check to see if the ray can cross the line
		if (cursor->GetX() > x1 && cursor->GetX() <= x2 && (cursor->GetY() < Vertices[i][1] || cursor->GetY() <= Vertices[(i+1)%8][1]))
		{
			static const float eps = 0.000001;

			//equation of the line
			float dx = Vertices[(i+1)%nSides][0] - Vertices[i][0];
			float dy = Vertices[(i+1)%nSides][1] - Vertices[i][1];

			if (fabs(dx) < eps)
				k = std::numeric_limits<int>::infinity();
			else
				k = dy/dx;

			float m = Vertices[i][1] - k*Vertices[i][0];

			//find if the ray crosses the line
			float y2 = k*cursor->GetX() + m;
			if (cursor->GetY() <= y2)
				crossings++;
		}

	}
	
	return(crossings%2);

	/*
	//simple code; works for some simple (convex) polygons.

	int stat;
	float temppos;

	stat = 0;

	//assume the vertices are defined from the bottom left corner and proceed ccw
	//compute the relative x or y position of the line using point-slope form
	temppos = vertices[0][0] + (vertices[3][0]-vertices[0][0])/(vertices[3][1]-vertices[0][1])*(cursor->GetY()-vertices[0][1]);
	if(cursor->GetX() > temppos)
		stat |= WIND_LEFT;

	temppos = vertices[1][0] + (vertices[2][0]-vertices[1][0])/(vertices[2][1]-vertices[1][1])*(cursor->GetY()-vertices[1][1]);
	if(cursor->GetX() < temppos)
		stat |= WIND_RIGHT;

	temppos = vertices[0][1] + (vertices[1][1]-vertices[0][1])/(vertices[1][0]-vertices[0][0])*(cursor->GetX()-vertices[0][0]);
	if(cursor->GetY() > temppos)
		stat |= WIND_BELOW;

	temppos = vertices[3][1] + (vertices[2][1]-vertices[3][1])/(vertices[2][0]-vertices[3][0])*(cursor->GetX()-vertices[3][0]);
	if(cursor->GetY() < temppos)
		stat |= WIND_ABOVE;
	
	if (stat == 15)
		return(1);
	else
		return(0);
	*/

}