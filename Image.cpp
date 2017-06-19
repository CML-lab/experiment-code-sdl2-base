#include <cmath>
#include "SDL_image.h"
#include "Image.h"
#include <iostream>


Image::Image(SDL_Surface* surface, float ratio)
{

	/*This function converts an SDL surface directly to an openGL texture, rather than using the SDL_Renderer.
	 * 
	 * "SDL2's accelerated rendering is nothing more than a software wrapper around OpenGL or Direct3D.  
	 *  By using OpenGL you bypass the wrapper and get to use the underlying API directly, which can 
	 *  give you more control but at the expense of greater complexity."
	 * 
	 */

	//set up some preliminary texture information
	GLenum texture_format;
	GLint channels;

	channels = surface->format->BytesPerPixel;
	if (channels == 4) // contains an alpha channel
	{
		if (surface->format->Rmask == 0x000000ff)
		{
			texture_format = GL_RGBA;
		}
		else
		{
			texture_format = GL_BGRA;
		}
	}
	else // no alpha channel
	{
		if (surface->format->Rmask == 0x000000ff)
		{
			texture_format = GL_RGB;
		}
		else
		{
			texture_format = GL_BGR;
		}
	}


	//create a texture for each specified window/context
	for (int a = 0; a < NSCREEN; a++)
	{

		SDL_GL_MakeCurrent(screens[a].window, screens[a].glcontext);

		// generate the OpenGL texture and store the width and height
		glGenTextures(1, &texture[a]);
		glBindTexture(GL_TEXTURE_2D, texture[a]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexImage2D(GL_TEXTURE_2D, 0, channels, surface->w, surface->h, 0, texture_format, GL_UNSIGNED_BYTE, surface->pixels);//
		width = (GLfloat)surface->w * ratio;  //this won't change since it is always the same texture
		height = (GLfloat)surface->h * ratio;  //this won't change since it is always the same texture

		drawOn = 1;

	}

}

Image* Image::LoadFromFile(char* filePath)
{
	SDL_Surface* surface = IMG_Load(filePath);
	if (surface == NULL) // failed to load file
	{
		return NULL;
	}
	else
	{
		Image* image = new Image(surface);
		SDL_FreeSurface(surface);
		return image;
	}

	//text = false;
}


Image* Image::ImageText(Image* txt, const char* txtstr, const std::string& fonttype, int fontsize, SDL_Color fontcolor)
{
	/* To create text, call a render function from SDL_ttf and use it to create
	 * an Image object. See http://www.libsdl.org/projects/SDL_ttf/docs/SDL_ttf.html#SEC42
	 * for a list of render functions.
	 *
	 * Note, this method has not been updated for new SDL2 functionality, but it still works.
	 */


	delete txt;

	TTF_Font *font;

	const std::string strdir = "Resources/";
	const std::string fontstr = strdir + fonttype;
	//std::cerr << fontstr << std::endl;

	font = TTF_OpenFont(fontstr.c_str(), fontsize);
	//std::cerr << font << std::endl;

	txt = new Image(TTF_RenderText_Blended(font, txtstr, fontcolor));
	//txt = new Image(TTF_RenderText_Shaded(font, txtstr, fontcolor, bgcolor));
	//txt = new Image(TTF_RenderText_Solid(font, txtstr, fontcolor));
	
	//text = true;

	//std::cerr << fontstr << std::endl;

	TTF_CloseFont(font);

	return(txt);

}


GLuint Image::GetTexture(int win) const
{
	return texture[win];
}

GLfloat Image::GetWidth() const
{
	return width;
}

GLfloat Image::GetHeight() const
{
	return height;
}

void Image::Draw(GLfloat xPos, GLfloat yPos, int win, GLfloat theta)
{
	Draw(xPos, yPos, width, height, win, theta);
}

void Image::Draw(GLfloat xPos, GLfloat yPos, GLfloat w, GLfloat h, int win, GLfloat theta)
{	

	if (drawOn)
	{

		//get the current active window and context, so we can restore it after the draw
		SDL_Window* curwin = SDL_GL_GetCurrentWindow();
		SDL_GLContext curcntxt = SDL_GL_GetCurrentContext();

		//switch to the requested window/context for the draw
		SDL_GL_MakeCurrent(screens[win].window, screens[win].glcontext);

		glColor3f(1.0f, 1.0f, 1.0f);

		glBindTexture(GL_TEXTURE_2D, texture[win]);
		glEnable(GL_TEXTURE_2D);

		// Draw a quad with the texture on it
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(xPos - (w / 2) * cos(theta) - (h / 2) * sin(theta),
			yPos - (w / 2) * sin(theta) + (h / 2) * cos(theta), 0.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(xPos + (w / 2) * cos(theta) - (h / 2) * sin(theta),
			yPos + (w / 2) * sin(theta) + (h / 2) * cos(theta), 0.0f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(xPos + (w / 2) * cos(theta) + (h / 2) * sin(theta),
			yPos + (w / 2) * sin(theta) - (h / 2) * cos(theta), 0.0f);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(xPos - (w / 2) * cos(theta) + (h / 2) * sin(theta),
			yPos - (w / 2) * sin(theta) - (h / 2) * cos(theta), 0.0f);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);

		//switch back to the last active context/window after the draw
		SDL_GL_MakeCurrent(curwin, curcntxt);

	}
}


void Image::DrawAlign(GLfloat xPos, GLfloat yPos, GLfloat w, GLfloat h, GLint cflag, int win)
{
	if (drawOn)
	{

		//get the current active window and context, so we can restore it after the draw
		SDL_Window* curwin = SDL_GL_GetCurrentWindow();
		SDL_GLContext curcntxt = SDL_GL_GetCurrentContext();

		//switch to the requested window/context for the draw
		SDL_GL_MakeCurrent(screens[win].window, screens[win].glcontext);


		glBindTexture(GL_TEXTURE_2D, texture[win]);
		glEnable(GL_TEXTURE_2D);

		if (cflag == 1)  //right-align
		{
			// Draw a quad with the texture on it
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(xPos - w, yPos + (h / 2), 0.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(xPos,     yPos + (h / 2), 0.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(xPos,     yPos - (h / 2), 0.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(xPos - w, yPos - (h / 2), 0.0f);
			glEnd();
		}
		else if (cflag == 2) //top-align
		{
			// Draw a quad with the texture on it
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(xPos - (w / 2), yPos, 0.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(xPos + (w / 2), yPos, 0.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(xPos + (w / 2), yPos - h,     0.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(xPos - (w / 2), yPos - h,     0.0f);
			glEnd();

		}
		else if (cflag == 3) //left-align 
		{

			// Draw a quad with the texture on it
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(xPos,	 yPos + (h / 2), 0.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(xPos + w, yPos + (h / 2), 0.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(xPos + w, yPos - (h / 2), 0.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(xPos,     yPos - (h / 2), 0.0f);
			glEnd();

		}
		else if (cflag == 4)  //bottom-align
		{
			// Draw a quad with the texture on it
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(xPos - (w / 2), yPos + h, 0.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(xPos + (w / 2), yPos + h, 0.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(xPos + (w / 2), yPos,     0.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(xPos - (w / 2), yPos,     0.0f);
			glEnd();
		}

		else
		{
			// Draw a quad with the texture on it
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(xPos - (w / 2), yPos + (h / 2), 0.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(xPos + (w / 2), yPos + (h / 2), 0.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(xPos + (w / 2), yPos - (h / 2), 0.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(xPos - (w / 2), yPos - (h / 2), 0.0f);
			glEnd();
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);

		//switch back to the last active context/window after the draw
		SDL_GL_MakeCurrent(curwin, curcntxt);

	}

}

void Image::On()
{
	drawOn = 1;
}

void Image::Off()
{
	drawOn = 0;
}

int Image::DrawState()
{
	return(drawOn);
}
