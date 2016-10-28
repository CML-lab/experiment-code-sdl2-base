#ifndef OBJECT2D_H
#define OBJECT2D_H
#pragma once

#include "SDL.h"
#include "SDL_opengl.h"
#include "Image.h"


// Stores an Image along with position and rotation information
class Object2D
{
private:
	Image* image;
	GLfloat xPos;
	GLfloat yPos;
	GLfloat angle;
public:
	// i is the image to be displayed
	Object2D(Image* i);
	~Object2D() { }
	// Gets the default width (meters) of the image
	GLfloat GetWidth() const;
	// Gets the default height (meters) of the image
	GLfloat GetHeight() const;
	// Gets the X position (meters) of the object
	GLfloat GetX() const;
	// Gets the Y position (meters) of the object
	GLfloat GetY() const;
	// Gets the counterclockwise rotation (radians) of the object
	GLfloat GetAngle() const;
	// Sets the position (meters) of the object
	void SetPos(GLfloat x, GLfloat y);
	// Sets the counterclockwise rotation (radians) of the object
	void SetAngle(GLfloat theta);
	// Draw the object
	void Draw();
	// Draw the object
	void Draw(GLfloat width, GLfloat height);
	// Gets the distance between two objects (meters)
	static float Distance(Object2D* ob1, Object2D* ob2);
};

#endif
