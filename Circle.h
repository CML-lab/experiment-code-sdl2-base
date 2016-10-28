#ifndef CIRCLE_H
#define CIRCLE_H
#pragma once

#include "SDL.h"
#include "SDL_opengl.h"
#include "Object2D.h"

// creates a circle object to render as the target
class Circle
{
private:
	GLint nVerts;
	GLfloat vertices[2][100]; // vertices in meters from center
	GLfloat verticesInner[2][100];
	GLfloat color[3]; //
	GLfloat diameter;
	GLfloat xpos;
	GLfloat ypos;
	GLint border;
	GLint drawOn; // controls whether the object will be drawn or not
	GLfloat borderWidth;
	GLfloat borderColor[3];

public:
	// constructor
	Circle(GLfloat x, GLfloat y, GLfloat diam, GLfloat clr[]);
	
	// destructor
	~Circle(){}

	// sets the diameter
	void SetDiameter(GLfloat diam);
	void SetRadius(GLfloat rad);

	// draw the circle
	void Draw();

	void MakeVerts();

	void SetColor(GLfloat clr[]);
	void SetBorderColor(GLfloat clr[]);

	void SetPos(GLfloat x, GLfloat y);

	GLfloat GetX();
	GLfloat GetY();
	GLfloat GetDiam();
	GLfloat GetRadius();

	void BorderOn();
	void BorderOff();
	void On();
	void Off();

	void SetBorderWidth(GLfloat w);
	
	GLfloat Distance(Circle* c);
	GLfloat Distance(Object2D* c);

	GLint drawState();
};

#endif
	
