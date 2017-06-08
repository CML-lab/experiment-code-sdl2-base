#include <cmath>
#include <limits>
#include <sstream>
#include <iostream>
#include <fstream>
#include <istream>

#include "Region2D.h"
#include "Object2D.h"
#include "HandCursor.h"

void Region2D::SetNSides(GLint sides)
{
	nSides = sides;
	//2D region must have at least 3 sides!
	if (nSides < 3)
		nSides = 3;
	if (nSides > 10)
		nSides = 10;
}

void Region2D::SetRegionVerts(GLfloat Verts[][2])
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

void Region2D::SetRegionColor(GLfloat clr[])
{
	color[0] = clr[0];
	color[1] = clr[1];
	color[2] = clr[2];
}

void Region2D::SetBorderColor(GLfloat clr[])
{
	borderColor[0] = clr[0];
	borderColor[1] = clr[1];
	borderColor[2] = clr[2];
}

void Region2D::SetBorderWidth(GLfloat width)
{
	borderWidth = width;
}

void Region2D::SetRegionCenter(GLfloat centerx, GLfloat centery)
{
	xcenter = centerx;
	ycenter = centery;
}

GLfloat Region2D::GetRegionCenterX()
{
	return(xcenter);
}

GLfloat Region2D::GetRegionCenterY()
{
	return(ycenter);
}

GLfloat Region2D::GetRegionVert(GLint i, GLint j)
{
	return(Vertices[i][j]);
}

GLint Region2D::GetRegionSides()
{
	return(nSides);
}

GLfloat Region2D::GetRectWidth()
{
	if (nSides == 4)
		return(Vertices[1][0]-Vertices[0][0]);
	else
		return(-1);
}

GLfloat Region2D::GetRectHeight()
{
	if (nSides == 4)
		return(Vertices[2][1]-Vertices[0][1]);
	else
		return(-1);
}

int Region2D::SetRectDims(GLfloat width, GLfloat height)
{
	if (nSides == 4)
	{
		//create a rectangle of width and heigh, with the bottom left corner at (0,0)
		Vertices[0][0] = 0;
		Vertices[0][1] = 0;
		Vertices[1][0] = width;
		Vertices[1][1] = 0;
		Vertices[2][0] = width;
		Vertices[2][1] = height;
		Vertices[3][0] = 0;
		Vertices[3][1] = height;

		return(0);
	}
	else
		return(-1);  //failed to set vertices because dimension of object is not a rectangle


}

int Region2D::SetCenteredRectDims(GLfloat width, GLfloat height)
{
	if (nSides == 4)
	{
		//create a rectangle of width and heigh, centered at (0,0)
		Vertices[0][0] = -width/2;
		Vertices[0][1] = -height/2;
		Vertices[1][0] = width/2;
		Vertices[1][1] = -height/2;
		Vertices[2][0] = width/2;
		Vertices[2][1] = height/2;
		Vertices[3][0] = -width/2;
		Vertices[3][1] = height/2;

		return(0);
	}
	else
		return(-1);  //failed to set vertices because dimension of object is not a rectangle


}


Region2D Region2D::LoadRegionFromFile(char* filePath)
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

	regionfile.getline(tmpline,sizeof(tmpline),'\n');
	if (!regionfile.eof())
	{
		sscanf(tmpline,"%f %f %f", &tmpclr[0], &tmpclr[1], &tmpclr[2]);
		region.SetRegionColor(tmpclr);
	}
	else
		return(region);

	regionfile.getline(tmpline, sizeof(tmpline), '\n');
	while(!regionfile.eof() && nSides < 10)
	{
			sscanf(tmpline, "%f %f", &tmpverts[nSides][0], &tmpverts[nSides][1]);
			nSides++;
			regionfile.getline(tmpline, sizeof(tmpline), '\n');
	}
	region.SetNSides(nSides);
	region.SetRegionVerts(tmpverts);

	region.SetRegionCenter(0.0f, 0.0f);

	float blkColor[3] = {0.0f, 0.0f, 0.0f};
	region.SetBorderColor(blkColor);
	region.SetBorderWidth(3.0f);
	region.BorderOff();
	
	region.Off();

	return(region);

}

void Region2D::Draw()
{
			
	// Draw the polygon
	if(drawOn)
	{

		glColor3f(color[0],color[1],color[2]);
		glBegin(GL_TRIANGLE_FAN);
	
		for (int i = 0; i<nSides; i++)
			glVertex3f(Vertices[i][0] + xcenter, Vertices[i][1] + ycenter, 0.0f);

		glEnd();
		glColor3f(1.0f, 1.0f, 1.0f);


		if (borderOn)
		{
			glColor3f(borderColor[0],borderColor[1],borderColor[2]);
			glLineWidth(borderWidth);
			glBegin(GL_LINE_STRIP);

			for (int i = 0; i < nSides; i++)
			{
				//straight line segment
				glVertex3f(Vertices[i][0]+xcenter, Vertices[i][1]+ycenter, 0.0f);
			}
			glVertex3f(Vertices[0][0]+xcenter, Vertices[0][1]+ycenter, 0.0f);
			glEnd();

			//reset defaults after the draw
			glColor3f(1.0f, 1.0f, 1.0f);
			glLineWidth(1.0f);
		}




		
	}
}

bool Region2D::InRegion(Object2D* cursor)
{
	return InRegion(cursor->GetX(), cursor->GetY());
}

bool Region2D::InRegion(HandCursor* cursor)
{
	return InRegion(cursor->GetX(), cursor->GetY());
}

bool Region2D::InRegion(GLfloat xcurs, GLfloat ycurs)
{
	//ray-casting technique
	//adapted from: http://sidvind.com/wiki/Point-in-polygon:_Jordan_Curve_Theorem
	int crossings = 0;
	float x1, x2;
	float y1, y2;
	double k;
	bool inregion = false;


	//iterate through each size of the polygon
	for (int i = 0; i < nSides; i++)
	{
		//make sure it doesn't matter if the line goes from left to right or right to left
		if( (Vertices[i][0] + xcenter) < (Vertices[(i + 1)%nSides][0]+xcenter) )
		{
			x1 = (Vertices[i][0] + xcenter);
			x2 = (Vertices[(i+1)%nSides][0] + xcenter);
		}
		else
		{
			x1 = (Vertices[(i + 1)%nSides][0] + xcenter);
			x2 = (Vertices[i][0] + xcenter);
		}

		//make sure it doesn't matter if the line goes from up to down or down to up
		if( (Vertices[i][1] + ycenter) < (Vertices[(i + 1)%nSides][1] + ycenter) )
		{
			y1 = (Vertices[i][1] + ycenter);
			y2 = (Vertices[(i + 1)%nSides][1] + ycenter);
		}
		else
		{
			y1 = (Vertices[(i + 1)%nSides][1] + ycenter);
			y2 = (Vertices[i][1] + ycenter);
		}
		//check to see if the ray can cross the line
		if (xcurs > x1 && xcurs <= x2 && (ycurs < y1 || ycurs <= y2))
		{
			static const float eps = 0.000001;

			//equation of the line
			float dx = Vertices[(i+1)%nSides][0] - Vertices[i][0];
			float dy = Vertices[(i+1)%nSides][1] - Vertices[i][1];

			if (fabs(dx) < eps)
				k = std::numeric_limits<int>::infinity();
			else
				k = dy/dx;

			float m = (Vertices[i][1] + ycenter) - k*(Vertices[i][0] + xcenter);

			//find if the ray crosses the line
			float y2 = k*xcurs + m;
			if (ycurs <= y2)
				crossings++;
		}

	}
	
	if (crossings%2 > 0.1)
		inregion = true;
	
	return(inregion);

}

void Region2D::On()
{
	drawOn = 1;
}

void Region2D::Off()
{
	drawOn = 0;
}

int Region2D::DrawState()
{
	return(drawOn);
}

void Region2D::BorderOn()
{
	borderOn = 1;

}

void Region2D::BorderOff()
{
	borderOn = 0;
}


GLfloat Region2D::Distance(Circle* c)
{
	return sqrtf(powf(c->GetX() - xcenter, 2.0f) + powf(c->GetY() - ycenter, 2.0f));
}

GLfloat Region2D::Distance(Object2D* c)
{
	return sqrtf(powf(c->GetX() - xcenter, 2.0f) + powf(c->GetY() - ycenter, 2.0f));
}