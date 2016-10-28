#include <cmath>
#include "HandCursor.h"
#include "Circle.h"
#include "config.h"

HandCursor::HandCursor(Circle* c)
{
	circ = c;
	circ->On();

	yvel = 0;
	xvel = 0;
	xpos = 0;
	ypos = 0;
	x0 = .5;
	y0 = .5;
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

void HandCursor::Draw()
{
	circ->Draw();
}

GLint HandCursor::drawState()
{
	return circ->drawState();
}

void HandCursor::UpdatePos(GLfloat x, GLfloat y)
{

	GLfloat xScr, yScr;

	xpos = x;
	ypos = y;

	for (int i = (NCHIST - 1); i > 0; i--)
	{
		xhist[i] = xhist[i - 1];
		yhist[i] = yhist[i - 1];
	}
	xhist[0] = x;
	yhist[0] = y;

	//---- Estimate velocity as slope of last 5 observations ----
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


	if(clamp)
	{
		GLfloat r = sqrtf((x - x0)*(x - x0) + (y - y0)*(y - y0));
		if(1)
		{
			xScr = r*sin(thetaClamp) + x0;
			yScr = r*cos(thetaClamp) + y0;
		}
		else
		{
			xScr = -r*sin(thetaClamp) + x0;
			yScr = -r*cos(thetaClamp) + y0;
		}
	}
	else
	{
		// apply rotation
		xScr = rotMat[0]*(x - x0) + rotMat[1]*(y - y0) + x0;
		yScr = rotMat[2]*(x - x0) + rotMat[3]*(y - y0) + y0;

		// apply gain
		xScr = xgain*(xScr - x0) + x0;
		yScr = ygain*(yScr - y0) + y0;
	}

	for (int i = (NCHIST - 1); i > 0; i--)
	{
		xhistScr[i] = xhistScr[i - 1];
		yhistScr[i] = yhistScr[i - 1];
	}
	xhistScr[0] = xScr;
	yhistScr[0] = yScr;

	circ->SetPos(xScr, yScr);
}

GLfloat HandCursor::GetTrueX()
{
	return xpos;
}

GLfloat HandCursor::GetTrueY()
{
	return ypos;
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
	return xvel;
}

GLfloat HandCursor::GetYVel()
{
	return yvel;
}

GLfloat HandCursor::Distance(GLfloat x, GLfloat y)
{
	return sqrtf(powf(circ->GetX() - x, 2.0f) + powf(circ->GetY() - y, 2.0f));
}

GLfloat HandCursor::Distance(Circle* c)
{
	return sqrtf(powf(circ->GetX() - c->GetX(), 2.0f) + powf(circ->GetY() - c->GetY(),2.0f));
}

GLfloat HandCursor::Distance(Object2D* obj)
{
	return sqrtf(powf(circ->GetX() - obj->GetX(), 2.0f) + powf(circ->GetY() - obj->GetY(),2.0f));
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

void HandCursor::On()
{
	circ->On();
}

void HandCursor::Off()
{
	circ->Off();
}

GLfloat HandCursor::GetX()
{
	return circ->GetX();
}

GLfloat HandCursor::GetY()
{
	return circ->GetY();
}

GLfloat HandCursor::GetVel()
{
	return sqrtf(xvel*xvel + yvel*yvel);
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


void HandCursor::SetColor(GLfloat clr[])
{
	circ->SetColor(clr);
}