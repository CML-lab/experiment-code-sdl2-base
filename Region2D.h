#ifndef REGION2D_H
#define REGION2D_H
#pragma once

#include "SDL.h"
#include "SDL_opengl.h"
#include "Object2D.h"
#include "HandCursor.h"

// Stores an Region as a series of vertices, and enables rendering capabilties
class Region2D
{
private:
	GLint nSides;
	GLfloat Vertices[10][2];  //vertices in meters from the origin (bottom left corner of the screen)
	GLfloat color[3];  //rgb array, with range 0-1
	GLfloat borderColor[3];  //rgb array, with range 0-1
	GLint drawOn;
	GLint borderOn;
	GLfloat xcenter;
	GLfloat ycenter;

public:

	~Region2D() { }
	
	GLfloat GetRegionVert(GLint i, GLint j);	// Gets a requsted polygon vertex
	GLint GetRegionSides();					// Gets number of sides of the polygon
	GLfloat GetRectWidth();					// Gets width, for rectangular polygon
	GLfloat GetRectHeight();				// Gets height, for rectangular polygon
	
	void SetNSides(GLint sides);			//set the number of sides in the polygon
	void SetOneVert(GLint i, GLint j, GLfloat vert);	// Sets one polygon vertex of the object (in meters)
	void SetRegionCenter(GLfloat centerx, GLfloat centery);		//set the center position of the region
	void SetRegionVerts(GLfloat Verts[][2]);	// Sets the polygon vertices of the object (in meters)
	int SetRectDims(GLfloat width, GLfloat height);	// Sets rectangular polygon vertices of the object (in meters) from specified width and heigh
	void SetRegionColor(GLfloat clr[]);		// Sets the polygon color
	void SetBorderColor(GLfloat clr[]);		// Sets the polygon color

	static Region2D LoadRegionFromFile(char* filePath);	//load polygon from file

	void Draw();		// Draw the object
	
	void On();		//set draw flag on
	void Off();		//set draw flag off
	void BorderOn();		//set draw flag on
	void BorderOff();		//set draw flag off
	int DrawState();	//returns the draw flag

	// Determines if the cursor is located within the region
	bool InRegion(Object2D* cursor);
	bool InRegion(HandCursor* cursor);
	bool InRegion(GLfloat xcurs, GLfloat ycurs);
};

#endif
