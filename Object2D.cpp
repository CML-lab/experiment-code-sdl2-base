#include <cmath>
#include "Object2D.h"

Object2D::Object2D(Image* i,CODASYSCONFIG *CodaSysConfig)
{
	image = i;
	angle = 0.0f;
	samprate = CodaSysConfig->MonitorRate;
}

GLfloat Object2D::GetWidth() const
{
	return image->GetWidth();
}

GLfloat Object2D::GetHeight() const
{
	return image->GetHeight();
}

GLfloat Object2D::GetX() const
{
	return xpos;
}

GLfloat Object2D::GetY() const
{
	return ypos;
}

GLfloat Object2D::GetLastX()
{
	return xhist[1];
}

GLfloat Object2D::GetLastY()
{
	return yhist[1];
}


void Object2D::SetPos(GLfloat x, GLfloat y)
{
	xpos = x;
	ypos = y;

	for (int i = (NO2DHIST - 1); i > 0; i--)
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
	for (int i = 0; i < NO2DHIST; i++)
	{
		xmean += xhist[i];
		ymean += yhist[i];
	}
	xmean = xmean/NO2DHIST;
	ymean = ymean/NO2DHIST;
	
	// compute xvel as slope of data over last few 5 samples
	xvel = (-2*xhist[4] -1*xhist[3] + 1*xhist[1] + 2*xhist[0])*samprate/10;
	yvel = (-2*yhist[4] -1*yhist[3] + 1*yhist[1] + 2*yhist[0])*samprate/10;

}

GLfloat Object2D::GetXVel()
{
	return xvel;
}

GLfloat Object2D::GetYVel()
{
	return yvel;
}

GLfloat Object2D::GetVel()
{
	return sqrtf(xvel*xvel + yvel*yvel);
}

void Object2D::SetAngle(GLfloat theta)
{
	angle = theta;
}

void Object2D::Draw()
{
	image->Draw(xpos, ypos, angle);  //draw is subject to the image draw flag
}

void Object2D::Draw(GLfloat width, GLfloat height)
{
	image->Draw(xpos, ypos, width, height, angle);  //draw is subject to the image draw flag
}

float Object2D::Distance(Object2D* ob1, Object2D* ob2)
{
	return Distance(ob1, ob2->GetX(), ob2->GetY());
}

float Object2D::Distance(Object2D* ob1, GLfloat x, GLfloat y)
{
	return sqrtf(powf(x - ob1->xpos, 2.0f) + powf(y - ob1->ypos, 2.0f));
}

void Object2D::On()
{
	image->On();
}

void Object2D::Off()
{
	image->Off();
}

int Object2D::DrawState()
{
	return(image->DrawState());
}

