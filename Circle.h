#ifndef CIRCLE_H
#define CIRCLE_H
#pragma once

#include "SDL.h"
#include "SDL_opengl.h"
#include "Object2D.h"
#include "Timer.h"

// creates a circle object to render as the target
class Circle
{
protected:
	GLint nVerts;
	GLfloat vertices[2][100]; // vertices in meters from center
	GLfloat verticesInner[2][100];
	GLfloat color[3];
	GLfloat diameter;
	GLfloat startdiameter;  //temporary value to keep track of the original size of the circle
	GLfloat startxpos;
	GLfloat startypos;
	GLfloat xpos;
	GLfloat ypos;
	Timer CircTimer;  //timer to keep track of elapsed time from target onset
	int timerdelay; //hold time before the timer takes effect, in msec
	GLfloat xvel;  //velocity in m/s
	GLfloat yvel;
	GLint border;
	GLint drawOn; // controls whether the object will be drawn or not
	GLfloat borderWidth;
	GLfloat borderColor[3];
	GLfloat hitMargin;

public:
	// constructor
	Circle(GLfloat x, GLfloat y, GLfloat diam, GLfloat clr[]);
	
	// destructor
	~Circle(){}

	// sets the diameter
	void SetDiameter(GLfloat diam);
	void SetRadius(GLfloat rad);
	void SetPos(GLfloat x, GLfloat y);
	void SetHitMargin(GLfloat m = 1.0f);  //set the multiplication factor for the target radius used to determine a Hit (default = 1.0f)
	void SetVel(GLfloat vx, GLfloat vy);	//set a fixed x,y velocity of the circle (for pursuit targets)


	// draw the circle
	void Draw();
	void UpdatePos();  //update circle position according to velocity and timer info

	void MakeVerts();	//regenerate vertices of circle given current position and diameter

	void SetColor(GLfloat clr[]);
	void SetBorderColor(GLfloat clr[]);

	void ResetTimer(int timerdelay = 0);	//reset the circle internal timer
	Uint32 TgtTime();

	GLfloat GetX();
	GLfloat GetY();
	GLfloat GetDiam();
	GLfloat GetRadius();
	GLfloat GetHitMargin();  //returns the multiplication factor for the target radius used to determine a Hit

	void BorderOn();
	void BorderOff();
	void On();
	void Off();

	void SetBorderWidth(GLfloat w);
	
	GLfloat Distance(Circle* c);
	GLfloat Distance(Object2D* c);

	GLint drawState();   //return the current draw state (On or Off)

	bool Explode(GLfloat ntimes, GLfloat vel, Uint32 refTime);  //increase the size of the target until specified (i.e., "explode" animation);
};

#endif
	
