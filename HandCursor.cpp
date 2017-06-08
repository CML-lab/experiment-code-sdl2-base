#include <cmath>
#include "HandCursor.h"
#include "Circle.h"
#include "config.h"

HandCursor::HandCursor(GLfloat x, GLfloat y, GLfloat diam, GLfloat clr[]) : Circle(x, y, diam, clr)
{
	x0 = PHYSICAL_WIDTH/2;  //initialize to the center of the screen
	y0 = PHYSICAL_HEIGHT/2; //initialize to the center of the screen
	xgain = 1;
	ygain = 1;

	hitMargin = 1;

	rotMat[0] = 1;
	rotMat[1] = 0;
	rotMat[2] = 0;
	rotMat[3] = 1;
	
	clamp = 0;

	for (int i = 0; i < 5; i++)
	{
		xhist[i] = 0;
		yhist[i] = 0;
		xhistScr[i] = 0;
		yhistScr[i] = 0;
	}
}

void HandCursor::UpdatePos(GLfloat x, GLfloat y)
{

	GLfloat xTrue, yTrue;

	//store the true hand position (which is the input to this function)
	xTrue = x;
	yTrue = y;

	for (int i = (NCHIST - 1); i > 0; i--)	//update the true hand position history
	{
		xhist[i] = xhist[i - 1];
		yhist[i] = yhist[i - 1];
	}
	xhist[0] = x;
	yhist[0] = y;

	//---- Estimate true hand velocity (non-distorted) as slope of last 5 observations ----
	// get mean
	GLfloat xmean = 0;
	GLfloat ymean = 0;
	for (int i = 0; i < NCHIST; i++)
	{
		xmean += xhist[i];
		ymean += yhist[i];
	}
	xmean = xmean/NCHIST;
	ymean = ymean/NCHIST;
	
	// compute xvel as slope of data over last few 5 samples
	xvel = (-2*xhist[4] -1*xhist[3] + 1*xhist[1] + 2*xhist[0])*SAMPRATE/10;
	yvel = (-2*yhist[4] -1*yhist[3] + 1*yhist[1] + 2*yhist[0])*SAMPRATE/10;

	//set the circle xpos and ypos. this is initially set as the true value, then modified as necessary
	xpos = xTrue;
	ypos = yTrue;


	if(clamp)
	{
		GLfloat r = sqrtf((xpos - x0)*(xpos - x0) + (ypos - y0)*(ypos - y0));
		if(1)
		{
			xpos = r*sin(thetaClamp) + x0;
			ypos = r*cos(thetaClamp) + y0;
		}
		else
		{
			xpos = -r*sin(thetaClamp) + x0;
			ypos = -r*cos(thetaClamp) + y0;
		}
	}
	else
	{
		// apply rotation
		xpos = rotMat[0]*(xpos - x0) + rotMat[1]*(ypos - y0) + x0;
		ypos = rotMat[2]*(xpos - x0) + rotMat[3]*(ypos - y0) + y0;

		// apply gain
		xpos = xgain*(xpos - x0) + x0;
		ypos = ygain*(ypos - y0) + y0;
	}

	for (int i = (NCHIST - 1); i > 0; i--)
	{
		xhistScr[i] = xhistScr[i - 1];
		yhistScr[i] = yhistScr[i - 1];
	}
	xhistScr[0] = xpos;
	yhistScr[0] = ypos;

	MakeVerts();
}

GLfloat HandCursor::GetTrueX()
{
	return xTrue;
}

GLfloat HandCursor::GetTrueY()
{
	return yTrue;
}

GLfloat HandCursor::GetLastX()
{
	return xhist[1];  //xhist[0] is the current sample
}

GLfloat HandCursor::GetLastY()
{
	return yhist[1];  //yhist[0] is the current sample
}

void HandCursor::SetRotation(GLfloat theta)
{
	rotMat[0] = cos(theta*PI/180);
	rotMat[1] = sin(theta*PI/180);
	rotMat[2] = -sin(theta*PI/180);
	rotMat[3] = cos(theta*PI/180);
}

void HandCursor::SetOrigin(GLfloat x, GLfloat y)
{
	x0 = x;
	y0 = y;
}

GLfloat HandCursor::GetXVel()
{
	return xvel;  //return the true hand velocity (non-distorted)
}

GLfloat HandCursor::GetYVel()
{
	return yvel;  //return the true hand velocity (non-distorted)
}

GLfloat HandCursor::GetVel()
{
	return sqrtf(xvel*xvel + yvel*yvel);   //return the true hand velocity (non-distorted)
}

GLfloat HandCursor::Distance(GLfloat x, GLfloat y)
{
	return sqrtf(powf(xpos - x, 2.0f) + powf(ypos - y, 2.0f));
}

GLfloat HandCursor::Distance(Circle* c)
{
	return sqrtf(powf(xpos - c->GetX(), 2.0f) + powf(ypos - c->GetY(),2.0f));
}

GLfloat HandCursor::Distance(Object2D* obj)
{
	return sqrtf(powf(xpos - obj->GetX(), 2.0f) + powf(ypos - obj->GetY(),2.0f));
}


void HandCursor::Null()
{
	rotMat[0] = 1;
	rotMat[1] = 0;
	rotMat[2] = 0;
	rotMat[3] = 1;
}

void HandCursor::SetClampC(Circle* c1, Circle* c2)
{
	GLfloat th = atan2(c2->GetX()-c1->GetX(), c2->GetY() - c1->GetY());
	SetClamp(th*PI/180);
}

void HandCursor::SetClamp(GLfloat th)
{
	clamp = 1;
	thetaClamp = th*180/PI; // clamp angle
}

void HandCursor::ClampOff()
{
	clamp = 0;
}

GLfloat HandCursor::GetHitMargin()
{
	return hitMargin;
}

void HandCursor::SetHitMargin(GLfloat m)
{
	hitMargin = m;
}


bool HandCursor::HitTarget(Circle* targ)
{
	return HitTarget(targ->GetX(), targ->GetY(), hitMargin*targ->GetRadius());
}

// check if the cursor path ever crossed through the target
bool HandCursor::HitTarget(GLfloat x, GLfloat y, GLfloat rad)
{
	if(powf(xhistScr[0]-x,2.0f)+powf(yhistScr[0]-y,2.0f) < rad*rad || powf(xhistScr[1]-x,2.0f) + powf(yhistScr[1]-y,2.0f) < rad*rad) 
		return 1; // if current or previous sample is in circle, then return HIT
	else
	{
		// otherwise, need to check if cursor path crossed the circle (in case samples skipped over target)

		// solve simultaneous equations of circle and line. Check if either solution is between consecutive samples
		GLfloat A = powf(xhistScr[0] - xhistScr[1],2.0f) + powf(yhistScr[0] - yhistScr[1],2.0f);
		GLfloat B = 2*((xhistScr[0] - xhistScr[1])*(xhistScr[1] - x) + (yhistScr[0] - yhistScr[1])*(yhistScr[1] - y));
		GLfloat C = powf(xhistScr[1] - x,2.0f) + powf(yhistScr[1] - y,2.0f) - rad*rad;

		if(B*B - 4*A*C > 0) // if real solution exists, infinite line intersects target
		{
			GLfloat t1 = (-B + sqrtf(B*B - 4*A*C))/(2*A);
			if(t1 < 1 && t1 > 0) // line intersects target in between the two points
			{
				return 1;
			}
			else
				return 0;
		}
		else
			return 0;
	}
}

void HandCursor::SetGain(GLfloat xg, GLfloat yg)
{
	xgain = xg;
	ygain = yg;
}

