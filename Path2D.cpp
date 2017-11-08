#include <cmath>
#include <math.h>
#include <limits>
#include <sstream>
#include <iostream>
#include <fstream>
#include <istream>

#include "config.h"
#include "Path2D.h"


void Path2D::SetNVerts(GLint sides)
{
	nVerts = sides;
	//path must have at least 1 segment!
	if (nVerts < 1)
		nVerts = 1;
	if (nVerts > 8)
		nVerts = 8;
}

void Path2D::SetPathVerts(GLfloat Verts[][7])
{
	for (int i = 0; i < nVerts; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			Vertices[i][j] = Verts[i][j];
		}
	}

}

void Path2D::SetOneVert(GLint i, GLint j, GLfloat vert)
{
	Vertices[i][j] = vert;
}

void Path2D::SetPathColor(GLfloat clr[])
{
	color[0] = clr[0];
	color[1] = clr[1];
	color[2] = clr[2];
}

void Path2D::SetPathWidth(GLfloat width)
{

	PathWidth = width;  //path width, in meters

}

void Path2D::SetPathCenter(GLfloat centerx, GLfloat centery)
{
	xcenter = centerx;
	ycenter = centery;
}

GLfloat Path2D::GetPathVert(GLint i, GLint j)
{
	return(Vertices[i][j]);
}

GLint Path2D::GetPathNVerts()
{
	return(nVerts);
}

GLfloat Path2D::GetPathCenterX()
{
	return(xcenter);
}

GLfloat Path2D::GetPathCenterY()
{
	return(ycenter);
}

GLfloat Path2D::GetPathCenteredVert(GLint i, GLint j)
{
	GLfloat tmpvertices[7];
	tmpvertices[0] = Vertices[i][0]+xcenter;
	tmpvertices[1] = Vertices[i][1]+ycenter;
	tmpvertices[2] = Vertices[i][2]+xcenter;
	tmpvertices[3] = Vertices[i][3]+ycenter;
	tmpvertices[4] = Vertices[i][4];
	tmpvertices[5] = Vertices[i][5];
	tmpvertices[6] = Vertices[i][6];

	return(tmpvertices[j]);
}


Path2D Path2D::LoadPathFromFile(char* filePath)
{
	Path2D path;
	
	int nVerts = 0;
	GLfloat tmpclr[3];
	GLfloat tmpverts[8][7];
	GLfloat tmpwidth;
	char tmpline[80] = ""; 

	std::ifstream pathfile(filePath);

	if (!pathfile)
	{
		std::cerr << "Cannot open input file." << std::endl;
		path.nVerts = -1;
		return(path);
	}
	else
		std::cerr << "Opened Path File " << filePath << std::endl;


	pathfile.getline(tmpline, sizeof(tmpline), '\n');
	if (!pathfile.eof())
	{
		sscanf(tmpline,"%f %f %f", &tmpclr[0], &tmpclr[1], &tmpclr[2]);
		path.SetPathColor(tmpclr);
		std::cerr << "    Color: " << tmpclr[0] << " " << tmpclr[1] << " " << tmpclr[2] << std::endl;
	}
	else
		return(path);

	pathfile.getline(tmpline,sizeof(tmpline),'\n');
	if (!pathfile.eof())
	{
		sscanf(tmpline,"%f",&tmpwidth);
		float lineWidth[2];
		glGetFloatv(GL_LINE_WIDTH_RANGE, lineWidth);
		if (tmpwidth/PHYSICAL_RATIO > lineWidth[1])
			tmpwidth = lineWidth[1]*PHYSICAL_RATIO;
		path.SetPathWidth(tmpwidth);
		std::cerr << "    PathWidth: " << tmpwidth << std::endl;
	}
	else
		path.SetPathWidth(1.0f);


	pathfile.getline(tmpline, sizeof(tmpline),'\n');
	while(!pathfile.eof() && nVerts<10)
	{
			sscanf(tmpline, "%f %f %f %f %f %f %f", 
				&tmpverts[nVerts][0], &tmpverts[nVerts][1], &tmpverts[nVerts][2], 
				&tmpverts[nVerts][3], &tmpverts[nVerts][4], &tmpverts[nVerts][5], &tmpverts[nVerts][6]);
			nVerts++;
			pathfile.getline(tmpline, sizeof(tmpline), '\n');
	}
	path.SetNVerts(nVerts);
	path.SetPathVerts(tmpverts);

	path.Off();
	path.SetPathCenter(0.0f,0.0f);

	std::cerr << "    N Verts: " << path.GetPathNVerts() << std::endl;

	return(path);

}

void Path2D::Draw()
{
	
	int nsegments = 100;
	int i, j;
	float x,y;


	if(drawOn)
	{

		// Draw the path
		glColor3f(color[0],color[1],color[2]);
		glLineWidth(PathWidth/PHYSICAL_RATIO);
		glBegin(GL_LINE_STRIP);

		for (i = 0; i < nVerts; i++)
		{
			if(Vertices[i][6] <= 0.1)  //flag = 0
			{
				//straight line segment
				if (i == 0)
				{
					//first line segment, straight line
					glVertex3f(Vertices[i][0]+xcenter, Vertices[i][1]+ycenter, 0.0f);
					glVertex3f(Vertices[i][2]+xcenter, Vertices[i][3]+ycenter, 0.0f);
				}
				else
				{
					//connecting line segment, straight line (assume there is a repeated vertex!)
					glVertex3f(Vertices[i][2] + xcenter, Vertices[i][3] + ycenter, 0.0f);				
				}
			} //end if(flag = 0), i.e., draw a straight line
			else if(Vertices[i][6] <= 1.1)   //flag = 1
			{
				//semicircular arc curved line segment.  assume the user provided the proper parameters in the input file
				//algorithm taken from http://www.allegro.cc/forums/thread/594175/715617#target

				float theta = Vertices[i][4]/float(nsegments);  //this is a small number so we shouldn't run into undefined/infinity problems with tan
				float tangential_factor = tanf(theta);
				float radial_factor = 1 - cosf(theta);

				x = Vertices[i][0] + xcenter + Vertices[i][2]*cosf(Vertices[i][3]);
				y = Vertices[i][1] + ycenter + Vertices[i][2]*sinf(Vertices[i][3]);

				for (j = 0; j < nsegments + 1; j++)
				{
					glVertex3f(x, y, 0.0f);

					//tx and ty are tangent vectors, perpendicular to the radius (so swap x and -y)
					float tx = -(y - (Vertices[i][1] + ycenter));
					float ty = x - (Vertices[i][0] + xcenter);
					x += tx*tangential_factor;
					y += ty*tangential_factor;

					float rx = (Vertices[i][0] + xcenter) - x;
					float ry = (Vertices[i][1] + ycenter) - y;

					x += rx*radial_factor;
					y += ry*radial_factor;
				}

			}
			else if(Vertices[i][6] <= 2.1)   //flag = 2
			{

				float t;

				//quadratic bezier curve
				for (j = 0; j < 100; j++)
				{
					t = float(j)/100.0f;

					x = (1.0f-t)*( (1.0f-t)*Vertices[i][0] + t*Vertices[i][2]) + (t)*( (1.0f-t)*Vertices[i][2] + t*Vertices[i][4]);
					y = (1.0f-t)*( (1.0f-t)*Vertices[i][1] + t*Vertices[i][3]) + (t)*( (1.0f-t)*Vertices[i][3] + t*Vertices[i][5]);

					glVertex3f(x, y, 0.0f);

				}


			}//end if-else Vertices[i][6] flag

		}//end for(nVerts)
		glEnd();

		//reset defaults after the draw
		glColor3f(1.0f, 1.0f, 1.0f);
		glLineWidth(1.0f);

	}
	
}

bool Path2D::OnPath(Object2D* cursor)
{
	return OnPath(cursor->GetX(), cursor->GetY());
}

bool Path2D::OnPath(HandCursor* cursor)
{
	return OnPath(cursor->GetX(), cursor->GetY());
}

bool Path2D::OnPath(float xcurs, float ycurs)
{
	//determine if the cursor is on or off the path.

	//two cases: if on a straight line, or on the semicircular-arc curve.
	bool onpath = false;
	float x1, x2, x3, x4, x5, x6;
	float a,b,c;
	float epsilon = PathWidth/2;


	//iterate through each segment of the path
	for (int i = 0; i < nVerts; i++)
	{
		if(Vertices[i][6] <= 0.1)
		{
			//straight line segment
			if (fabsf(Vertices[i][2] - Vertices[i][0]) < epsilon)
			{
				//check for line verticality; if so, the check is easy: is the x coordinate the same and does the y coordinate fall between the two vertices
				onpath = onpath || 
				( (fabsf((Vertices[i][2] + xcenter) - xcurs) < epsilon) &&
				(((Vertices[i][3] + ycenter) + epsilon/2 > ycurs && (Vertices[i][1] + ycenter) - epsilon/2 < ycurs) ||
				((Vertices[i][1] + ycenter) + epsilon/2 > ycurs && (Vertices[i][3] + ycenter) - epsilon/2 < ycurs)) );
			}
			else
			{
				//not a vertical line, so see if the cursor is on the line: point falls on the line's equation, and between the two x values of the vertices
				x1 = ((Vertices[i][3] + ycenter) - (Vertices[i][1] + ycenter))/
				     ((Vertices[i][2] + xcenter) - (Vertices[i][0] + xcenter));
				x2 = (Vertices[i][1] + ycenter) - x1 * (Vertices[i][0] + xcenter);
				onpath = onpath || 
					( (fabsf(ycurs - (x1*xcurs + x2)) < epsilon) && 
					(((Vertices[i][2] + xcenter) + epsilon/2 > xcurs && (Vertices[i][0] + xcenter) - epsilon/2 < xcurs) ||
					((Vertices[i][0] + xcenter) + epsilon/2 > xcurs && (Vertices[i][2] + xcenter) - epsilon/2 < xcurs)) );
			}

		}
		else if(Vertices[i][6] <= 1.1)
		{
			//semicircular arc - curved line segment: point is (radius) away from the arc center, and at an angle between start_angle and (start_angle+arc_length)
			x1 = sqrtf( powf(xcurs - Vertices[i][0] + xcenter, 2.0f) + powf(ycurs - Vertices[i][1] + ycenter, 2.0f) );  //distance from the cursor to center of the arc
			x2 = atan2f(ycurs - Vertices[i][1] + ycenter,xcurs - Vertices[i][0] + xcenter);	//angle of the cursor from the center of the arc
			x2 = (x2 <= 0 ? x2 + 2*PI : x2 );  //set the angle to be between 0 and 2*pi
			x3 = (Vertices[i][3] <= 0 ? Vertices[i][3] + 2*PI : Vertices[i][3]);  //set the start angle between 0 and 2*pi
			x4 = (Vertices[i][3] + Vertices[i][4] <= 0 ? Vertices[i][3] + Vertices[i][4] + 2*PI : Vertices[i][3] + Vertices[i][4]);  //set the end angle between 0 and 2*pi if negative
			x4 = (x4 >= 2*PI ? x4 - 2*PI : x4);  //set the end angle between 0 and 2*pi if greater than 2*pi
			x5 = fabs(atanf(epsilon/Vertices[i][2]));  //angle "slop", or angular equivalent of a segment of length equal to the path width at the circle radius

			onpath = onpath || ( (fabsf(x1 - Vertices[i][2]) < epsilon) && 
				( (x2 > x3-x5 && x2 < x4+x5) || (x2 < x3+x5 && x2 > x4-x5) ) );
		}
		else if(Vertices[i][6] <= 2.1)
		{
			//bezier curve
			//is the point on the bezier curve?

			//solve for t for the x coordinate
			a = (Vertices[i][0] + xcenter) - 2.0f*(Vertices[i][2] + xcenter) + (Vertices[i][4] + xcenter);
			b = -2.0f*(Vertices[i][0] + xcenter) + 2.0f*(Vertices[i][2] + xcenter);
			c = (Vertices[i][0] + xcenter);

			if (a != 0)
			{
				//solution exists
				x1 = (-b + sqrtf( b*b - 4.0f*a*c)) / (2.0f*a);
				x1 = (x1 >= 0.0f ? x1 : -1.0f); //if the solution is not in the domain [0,1] it's not a valid solution
				x1 = (x1 <= 1.0f ? x1 : -1.0f);

				x2 = (-b - sqrtf( b*b - 4.0f*a*c)) / (2.0f*a);
				x2 = (x2 >= 0.0f ? x2 : -1.0f); //if the solution is not in the domain [0,1] it's not a valid solution
				x2 = (x2 <= 1.0f ? x2 : -1.0f);
			}
			else
			{
				//no solution
				x1 = -1.0f;
				x2 = -1.0f;
			}

			//solve for t for the y coordinate
			a = (Vertices[i][1] + ycenter) - 2.0f*(Vertices[i][3] + ycenter) + (Vertices[i][5] + ycenter);
			b = -2.0f*(Vertices[i][1] + ycenter) + 2*(Vertices[i][3] + ycenter);
			c = (Vertices[i][1] + ycenter);

			if (a != 0)
			{
				//solution exists
				x3 = (-b + sqrtf( b*b - 4.0f*a*c)) / (2.0f*a);
				x3 = (x3 >= 0.0f ? x3 : -1.0f); //if the solution is not in the domain [0,1] it's not a valid solution
				x3 = (x3 <= 1.0f ? x3 : -1.0f);

				x4 = (-b - sqrtf( b*b - 4.0f*a*c)) / (2.0f*a);
				x4 = (x4 >= 0.0f ? x4 : -1.0f); //if the solution is not in the domain [0,1] it's not a valid solution
				x4 = (x4 <= 1.0f ? x4 : -1.0f);
			}
			else
			{
				//no solution
				x3 = -1.0f;
				x4 = -1.0f;
			}

			//check if the solution for t exists and is common for both x and y coordinates
			onpath = onpath || 
				( x1 > 0 && ( (fabs(x1-x3) < epsilon) || (fabs(x1-x4) < epsilon) )) || //t(x1 value) is in the domain and matches one of the solutions for t(y value)
				( x2 > 0 && ( (fabs(x2-x3) < epsilon) || (fabs(x2-x4) < epsilon) ));   //t(x2 value) is in the domain and matches one of the solutions for t(y value)

		}
	}
	return(onpath);
}

bool Path2D::PathCollision(Object2D* cursor)
{
	return (PathCollision(cursor->GetX(), cursor->GetY(), cursor->GetLastX(), cursor->GetLastY() ));
}

bool Path2D::PathCollision(HandCursor* cursor)
{
	return (PathCollision(cursor->GetX(), cursor->GetY(), cursor->GetLastX(), cursor->GetLastY() ));
}

bool Path2D::PathCollision(float xcurs, float ycurs, float xcurslast, float ycurslast)
{
	// calculate the intersection of 2 lines (the line connecting cursor and LastCursorPos, and any segment of the path)
	bool onpath = false;
	float p[2], q[2], r[2], s[2], e[2];
	float t, u, v;
	bool tflag, vflag;
	float epsilon = PathWidth/100;
	float x3, x4;
	float a,b,c;

	//define seg1 as p to p+r
	p[0] = xcurslast;
	p[1] = ycurslast;
	r[0] = xcurs - p[0];
	r[1] = ycurs - p[1];


	//iterate through each segment of the path
	for (int i = 0; i < nVerts; i++)
	{
		if(Vertices[i][6] <= 0.1)
		{
			//straight line segment
			
			//method taken from http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
			// this method uses vector cross products to determine if the two segments intersect. it should be robust to detect
			// both intersections and also points where the cursor is on the path.
			
			//define seg2 as q to q+s.
			q[0] = (Vertices[i][0] + xcenter);
			q[1] = (Vertices[i][1] + ycenter);
			s[0] = (Vertices[i][2] + xcenter) - q[0];
			s[1] = (Vertices[i][3] + ycenter) - q[1];

			if ( fabsf(r[0]*s[1] - r[1]*s[0]) < epsilon )
			{
				//the lines are parallel
				//onpath = false;  //we don't need to set this again!
			}
			else
			{
				//the lines are not parallel. test if they intersect.

				t = ( (q[0]-p[0])*s[1] - (q[1]-p[1])*s[0] ) / (r[0]*s[1] - r[1]*s[0]);
				u = ( (q[0]-p[0])*r[1] - (q[1]-p[1])*r[0] ) / (r[0]*s[1] - r[1]*s[0]);

				if ( (t >= -epsilon && t <= 1+epsilon) && (u >= -epsilon && u <= 1+epsilon) )
					onpath = true;
				else
				{
					//onpath = false; //we don't need to set this again!
				}
			}

		}  //end if straight line segment
		else if(Vertices[i][6] <= 1.1) //else, semicircular arc path segment
		{
			//calculate if the line will intersect the circle at all
			q[0] = p[0] - (Vertices[i][0] + xcenter);
			q[1] = p[1] - (Vertices[i][1] + ycenter);

			u = pow(r[0]*q[0] + r[1]*q[1], 2.0f) - pow(r[0]*r[0] + r[1]*r[1], 2.0f) *
				( pow(q[0]*q[0] + q[1]*q[1], 2.0f) - Vertices[i][2]*Vertices[i][2] );

			//calculate the endpoints of the arc
			x3 = Vertices[i][3];  //the start angle
			x4 = Vertices[i][3] + Vertices[i][4];  //the end angle
			//if the arc length is negative, reverse these
			if (Vertices[i][4] <= 0)
			{
				x3 = x4;
				x4 = Vertices[i][3];
			}
			//calculate the start and end points of the arc, assuming it runs ccw from S to E
			s[0] = (Vertices[i][0] + xcenter) + Vertices[i][2]*cos(x3);
			s[1] = (Vertices[i][1] + ycenter) + Vertices[i][2]*sin(x3);
			e[0] = (Vertices[i][0] + xcenter) + Vertices[i][2]*cos(x4);
			e[1] = (Vertices[i][1] + ycenter) + Vertices[i][2]*sin(x4);

			if (u < 0)
			{
				//onpath = false;  //no intersection of line and circle  //we don't need to set this again!
			}
			else if (fabs(u) < epsilon)
			{
				//one intersection point; must test where the tangent point lies
				t = (-pow(r[0]*q[0] + r[1]*q[1], 2.0f) + sqrt(fabs(u)) ) / pow(r[0]*r[0] + r[1]*r[1], 2.0f);
				
				//test if the intersection point is off the line segment
				if (t < 0 || t > 1)
				{
					//onpath = false;  //we don't need to set this again!
				}

				//test if the intersection point is on the arc
				if ( (p[0]+t*r[0] - s[0])*(-(e[1]-s[1])) + (p[1]+t*r[1] - s[1])*(e[0]-s[0]) >= -epsilon)
					onpath = true;
				else
				{
					//onpath = false; //we don't need to set this again!
				}


			} //end one intersection point exists
			else
			{
				//two intersection points; must test where these points lie
				//one intersection point; must test where the tangent point lies
				t = (-pow(r[0]*q[0] + r[1]*q[1], 2.0f) + sqrt(fabs(u)) ) / pow(r[0]*r[0] + r[1]*r[1], 2.0f);
				v = (-pow(r[0]*q[0] + r[1]*q[1], 2.0f) - sqrt(fabs(u)) ) / pow(r[0]*r[0] + r[1]*r[1], 2.0f);

				//test if any intersection point is on the line segment; if not, then return false.
				if (t < 0 || t > 1)
					tflag = false;
				else
					tflag = true;

				if (v < 0 || v > 1)
					vflag = false;
				else
					vflag = true;

				if (!tflag && !vflag)
				{
					//onpath = false;  //we don't need to set this again!
				}
				
				//test if the intersection points lie on the same side as the arc
				if (tflag)
				{
					//if t is on the line segment, see if it is on the arc; if so, set the value to be true.
					if ( (p[0] + t*r[0] - s[0])*(-(e[1] - s[1])) + (p[1] + t*r[1] - s[1])*(e[0] - s[0]) >= -epsilon)
						onpath = true;
				}

				if (vflag)
				{
					//if v is on the line segment, see if it is on the arc; if so, set the value to be true.
					if ( (p[0] + v*r[0] - s[0])*(-(e[1] - s[1])) + (p[1] + v*r[1] - s[1])*(e[0] - s[0]) >= -epsilon)
						onpath = true;
				}

			} //end else 2 intersection points exist

		} //end if intersection with semicircular arc segment
		else if(Vertices[i][6] <= 2.1) //else, bezier curve
		{
			//calculate if the line segment and the bezier curve segment intersect
			u = (ycurslast - xcurs) / (xcurslast - ycurs);  //slope
			a = (Vertices[i][1] + ycenter) + 2.0f*(Vertices[i][3] + ycenter) + (Vertices[i][5] + ycenter) - u*(Vertices[i][0] + xcenter) -2.0f*u*(Vertices[i][2] + xcenter) - u*(Vertices[i][4] + xcenter);
			b = -2.0f*(Vertices[i][1] + ycenter) -2.0f*(Vertices[i][3] + ycenter) + 2.0f*u*(Vertices[i][0] + xcenter) + 2.0f*u*(Vertices[i][2] + xcenter);
			c = (Vertices[i][1] + ycenter) - ycurs - u*(Vertices[i][0] + xcenter) + u*xcurs;

			//calculate values of t for which the line intersects the bezier curve
			x3 = (-b + sqrtf( b*b - 4.0f*a*c))/ (2.0f*a);
			x3 = (x3 >= 0.0f ? x3 : -1.0f); //if the solution is not in the domain [0,1] it's not a valid solution
			x3 = (x3 <= 1.0f ? x3 : -1.0f);

			x4 = (-b - sqrtf( b*b - 4.0f*a*c))/ (2.0f*a);
			x4 = (x4 >= 0.0f ? x4 : -1.0f); //if the solution is not in the domain [0,1] it's not a valid solution
			x4 = (x4 <= 1.0f ? x4 : -1.0f);


			//check that the solution yields a point on the line segment
			t = -1.0f;
			u = -1.0f;
			if (x3 > 0)
			{
				q[0] = powf(1.0f-x3,2.0f)*(Vertices[i][0] + xcenter) + 2.0f*(x3-1.0f)*x3*(Vertices[i][2] + xcenter) + x3*x3*(Vertices[i][4] + xcenter);
				q[1] = powf(1.0f-x3,2.0f)*(Vertices[i][1] + ycenter) + 2.0f*(x3-1.0f)*x3*(Vertices[i][3] + ycenter) + x3*x3*(Vertices[i][5] + ycenter);

				if (fabs(xcurslast - xcurs) > epsilon) //make sure we don't run into an INF problem
					t = (q[0]-xcurs)/(xcurslast-xcurs);
				else
					t = (q[1]-ycurs)/(ycurslast-ycurs);
				//u = (q[1]-ycurs)/(ycurslast-ycurs);
				t = (t >= 0.0f ? t : -1.0f); //if the solution is not in the domain [0,1] it's not a valid solution
				t = (t <= 1.0f ? t : -1.0f);
			}
			if (x4 > 0)
			{
				q[0] = powf(1.0f-x4,2.0f)*(Vertices[i][0] + xcenter) + 2.0f*(x4-1.0f)*x4*(Vertices[i][2] + xcenter) + x4*x4*(Vertices[i][4] + xcenter);
				q[1] = powf(1.0f-x4,2.0f)*(Vertices[i][1] + ycenter) + 2.0f*(x4-1.0f)*x4*(Vertices[i][3] + ycenter) + x4*x4*(Vertices[i][5] + ycenter);

				if (fabs(xcurslast - xcurs) > epsilon)
					t = (q[0]-xcurs)/(xcurslast-xcurs);
				else
					t = (q[1]-ycurs)/(ycurslast-ycurs);
				//u = (q[1]-ycurs)/(ycurslast-ycurs);
				t = (t >= 0.0f ? t : -1.0f); //if the solution is not in the domain [0,1] it's not a valid solution
				t = (t <= 1.0f ? t : -1.0f);
			}

			onpath = (t >= 0.0f ? true : onpath);  //if we have found a valid intersection point, set onpath to true

		}

	} // end for loop

	return(onpath);
}

int Path2D::HitViaPts(Object2D* cursor, GLfloat dist)
{
	return HitViaPts(cursor->GetX(), cursor->GetY(), dist);
}

int Path2D::HitViaPts(HandCursor* cursor, GLfloat dist)
{
	return HitViaPts(cursor->GetX(), cursor->GetY(), dist);
}

int Path2D::HitViaPts(float xcurs, float ycurs, GLfloat dist)
{
	//check if the cursor is in the vicinity of the vertices; if so, return the vertex number
	int vert = -10;
	int ppdist = 0;
	int mindist = 10000;   //minimum distance; initialize to a large number 
	float x, y;

	//we will do this the "easy" way, by calculating the distance to each vertex and keeping track of the minimum
	for (int i = 0; i < nVerts; i++)
	{
		if(Vertices[i][6] <= 0.1)
		{
			//straight line
			if (i == 0)
			{
				//check also the starting vertex
				ppdist = sqrtf(powf(Vertices[i][0] + xcenter - xcurs, 2.0f) + powf(Vertices[i][1] + ycenter - ycurs, 2.0f));
				if (ppdist < mindist)
				{
					vert = -1;
					mindist = ppdist;
				}
			}
			ppdist = sqrtf(powf(Vertices[i][2] + xcenter - xcurs, 2.0f) + powf(Vertices[i][3] + ycenter - ycurs, 2.0f));
			if (ppdist < mindist)
			{
				vert = i;
				mindist = ppdist;
			}
		}
		else if(Vertices[i][6] <= 1.1)
		{
			//arc; we have to calculate the two vertices.
			if (i == 0)
			{
				//check also the starting vertex
				x = Vertices[i][0] + xcenter + Vertices[i][2]*cosf(Vertices[i][3]);  //center + radius*cos(start_angle)
				y = Vertices[i][1] + ycenter + Vertices[i][2]*sinf(Vertices[i][3]);	 //center + radius*sin(start_angle)
				ppdist = sqrtf(powf(x - xcurs, 2.0f) + powf(y - ycurs, 2.0f));
				if (ppdist < mindist)
				{
					vert = -1;
					mindist = ppdist;
				}
			}
			x = Vertices[i][0] + xcenter + Vertices[i][2]*cosf(Vertices[i][3]);	 //center + radius*cos(end_angle)
			y = Vertices[i][1] + ycenter + Vertices[i][2]*sinf(Vertices[i][3]);	 //center + radius*sin(end_angle)
			ppdist = sqrtf(powf(Vertices[i][2] + xcenter - xcurs, 2.0f) + powf(Vertices[i][3] + ycenter - ycurs, 2.0f));
			if (ppdist < mindist)
			{
				vert = i;
				mindist = ppdist;
			}

		}
		else if(Vertices[i][6] <= 2.1)
		{
			//bezier curve; we have to calculate the two vertices.
			if (i == 0)
			{
				//check also the starting vertex
				x = Vertices[i][0] + xcenter + Vertices[i][4]*cosf(Vertices[i][5]);  //center + radius*cos(start_angle)
				y = Vertices[i][1] + ycenter + Vertices[i][4]*sinf(Vertices[i][5]);	 //center + radius*sin(start_angle)
				ppdist = sqrtf(powf(x - xcurs, 2.0f) + powf(y - ycurs, 2.0f));
				if (ppdist < mindist)
				{
					vert = -1;
					mindist = ppdist;
				}
			}
			x = Vertices[i][0] + xcenter + Vertices[i][4]*cosf(Vertices[i][5]);	 //center + radius*cos(end_angle)
			y = Vertices[i][1] + ycenter + Vertices[i][4]*sinf(Vertices[i][5]);	 //center + radius*sin(end_angle)
			ppdist = sqrtf(powf(Vertices[i][4] + xcenter - xcurs, 2.0f) + powf(Vertices[i][5] + ycenter - ycurs, 2.0f));
			if (ppdist < mindist)
			{
				vert = i;
				mindist = ppdist;
			}
		}
	} //end for

	//now that we have the minimum distance and vertex number, determine if it is close enough to be "on" the vertex
	if (mindist <= dist)
		return(vert + 1);
	else
		return(-99);
}

void Path2D::On()
{
	drawOn = 1;
}

void Path2D::Off()
{
	drawOn = 0;
}

int Path2D::DrawState()
{
	return(drawOn);
}
