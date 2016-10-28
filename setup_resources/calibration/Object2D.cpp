#include <cmath>
#include <limits>
#include "Object2D.h"

Object2D::Object2D(Image* i)
{
	image = i;
	angle = 0.0f;
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
	return xPos;
}

GLfloat Object2D::GetY() const
{
	return yPos;
}

void Object2D::SetPos(GLfloat x, GLfloat y)
{
	xPos = x;
	yPos = y;
}

void Object2D::SetAngle(GLfloat theta)
{
	angle = theta;
}

void Object2D::Draw()
{
	image->Draw(xPos, yPos, angle);
}

void Object2D::Draw(GLfloat width, GLfloat height)
{
	image->Draw(xPos, yPos, width, height, angle);
}

float Object2D::Distance(Object2D* ob1, Object2D* ob2)
{
	return sqrtf(powf(ob2->xPos - ob1->xPos, 2.0f) + powf(ob2->yPos - ob1->yPos, 2.0f));
}
