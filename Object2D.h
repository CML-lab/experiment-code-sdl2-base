#ifndef OBJECT2D_H
#define OBJECT2D_H
#pragma once

#include "SDL.h"
#include "SDL_opengl.h"
#include "Image.h"
#define NO2DHIST 5

// Stores an Image along with position and rotation information
class Object2D
{
private:
	Image* image;
	GLfloat xpos;
	GLfloat ypos;
	GLfloat xhist[NO2DHIST];
	GLfloat yhist[NO2DHIST];
	GLfloat xvel;
	GLfloat yvel;
	GLfloat angle;
	GLfloat sizex;
	GLfloat sizey;
	//GLint drawOn;
public:
	// i is the image to be displayed
	Object2D(Image* i);
	~Object2D() { }
	GLfloat GetWidth() const; 	// Gets the default width (meters) of the image
	GLfloat GetHeight() const; 	// Gets the default height (meters) of the image
	GLfloat GetX() const;		// Gets the X position (meters) of the object
	GLfloat GetY() const;		// Gets the Y position (meters) of the object
	GLfloat GetLastX();			//returns the last x position
	GLfloat GetLastY();			//returns the last y position
	GLfloat GetXVel();			//returns the actual x hand velocity, non-distorted
	GLfloat GetYVel();			//returns the actual y hand velocity, non-distorted
	GLfloat GetVel();			//returns the actual 2D hand velocity, non-distorted
	GLfloat GetAngle() const;	// Gets the counterclockwise rotation (radians) of the object

	void SetPos(GLfloat x, GLfloat y);	// Sets the position (meters) of the object
	void SetAngle(GLfloat theta);		// Sets the counterclockwise rotation (radians) of the object
	void Draw(int win = 0);						// Draw the object
	void Draw(GLfloat width, GLfloat height, int win = 0);	// Draw the object
	float Distance(Object2D* ob1, Object2D* ob2);	// Gets the distance between two objects (meters)
	float Distance(Object2D* ob1, GLfloat x, GLfloat y);	// Gets the distance between an object and a position (meters)

	void On();		//set the draw flag on
	void Off();		//set the draw flag off
	int DrawState();	//return the draw flag
};

#endif
