#ifndef TGTFRAME_H
#define TGTFRAME_H
#pragma once


// Data type used by InputDevice to return input data
struct TargetFrame
{
	GLfloat startx;
	GLfloat starty;

	GLfloat tgtx;
	GLfloat tgty;

	//GLfloat rWin;  //radius of circular window around target
	GLint trace;

	GLchar flag;
};

#endif
