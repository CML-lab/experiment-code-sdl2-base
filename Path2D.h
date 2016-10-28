#ifndef PATH2D_H
#define PATH2D_H
#pragma once

#include "SDL.h"
#include "SDL_opengl.h"
#include "Object2D.h"
#include "HandCursor.h"

// Stores a path as a series of vertices, and enables rendering capabilties
class Path2D
{
private:
	GLint nVerts;
	GLfloat PathWidth;		 //path width, in meters
	GLfloat Vertices[8][6];  //either 2 pairs of points and a 0 flag (straight line) OR arc center (0-1), radius (2), start angle (3), and arc length (4) with a 1 flag (circular arc - 5).  angles in radians
	GLfloat color[3];
	GLint drawOn;
	GLfloat xcenter;
	GLfloat ycenter;
public:
	~Path2D() { }
	
	GLfloat GetPathVert(GLint i, GLint j);		// Gets a requsted polygon vertex
	GLint GetPathNVerts();						// Gets number of vertices in the path
		
	void SetNVerts(GLint sides);				//set the number of sides in the polygon
	void SetPathWidth(GLfloat width);			// Set path line width
	void SetOneVert(GLint i, GLint j, GLfloat vert);	// Sets one polygon vertex of the object (in meters)	
	void SetPathVerts(GLfloat Verts[][6]);		// Sets the polygon vertices of the object (in meters)
	void SetPathColor(GLfloat clr[]);			// Sets the polygon color
	void SetPathCenter(GLfloat centerx, GLfloat centery);	// Sets the center of the path
	
	static Path2D LoadPathFromFile(char* filePath);	//load polygon from file

	void Draw();	// Draw the object.  pathx and pathy are the (center) offset of the path

	
	void On();		//set draw flag on
	void Off();		//set draw flag off
	int DrawState(); //returns the draw flag

	// Determines if the cursor is located on the path
	bool OnPath(Object2D* cursor);
	bool OnPath(HandCursor* cursor);
	bool OnPath(float xcurs, float ycurs);

	// Determines if the cursor trajectory has ever intersected the path
	bool PathCollision(Object2D* cursor);
	bool PathCollision(HandCursor* cursor);
	bool PathCollision(float xcurs, float ycurs, float xcurslast, float ycurslast);

	// Determines if the cursor is located on a path vertex  (may not be so useful?)
	int HitViaPts(Object2D* cursor, GLfloat dist);
	int HitViaPts(HandCursor* cursor, GLfloat dist);
	int HitViaPts(float xcurs, float ycurs, GLfloat dist);
};

#endif
