

/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include <iostream>

#include "texture.h"


Texture2D::Texture2D()
	: Width(0), Height(0), Internal_Format(GL_RGB), Image_Format(GL_RGB), Wrap_S(GL_REPEAT), Wrap_T(GL_REPEAT), Filter_Min(GL_LINEAR), Filter_Max(GL_LINEAR)
{
	/*glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);*/
	glGenTextures(1, &this->ID);
}

void Texture2D::Generate(GLuint width, GLuint height, unsigned char* data)
{
	this->Width = width;
	this->Height = height;
	// Create Texture

	glBindTexture(GL_TEXTURE_2D, this->ID);
	glTexImage2D(GL_TEXTURE_2D, 0, this->Internal_Format, width, height, 0, this->Image_Format, GL_UNSIGNED_BYTE, data);
	// Set Texture wrap and filter modes
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->Wrap_S);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->Wrap_T);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->Filter_Max);
	// Unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::GenerateFromMat(cv::Mat &frame)
{

	// Create Texture
	//cv::cvtColor(frame, frame, CV_BGR2RGBA);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, 0.0f);

	cv::cvtColor(frame, frame, CV_BGR2RGBA);
	glEnable(GL_TEXTURE_RECTANGLE_ARB; //GL_TEXTURE_RECTANGLE_ARB
	//// Typical texture generation using data from the bitmap
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, this->ID);//GL_TEXTURE_RECTANGLE_ARB
	//// Transfer image data to the GPU
	glTexImage2D(GL_TEXTURE_2D, 0,
		GL_RGBA, frame.cols, frame.rows, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, frame.data);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);                         glVertex2f(1.0, 1.0);
	glTexCoord2f(frame.cols, 0);             glVertex2f(-1.0, 1.0);
	glTexCoord2f(frame.cols, frame.rows); glVertex2f(-1.0, -1.0);
	glTexCoord2f(0, frame.rows);             glVertex2f(1.0, -1.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	//glBindTexture(GL_TEXTURE_2D, this->ID);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.cols, frame.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, frame.data);
	//// Set Texture wrap and filter modes
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->Wrap_S);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->Wrap_T);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->Filter_Max);
	//// Unbind texture
	//glBindTexture(GL_TEXTURE_2D, 0);
}



void Texture2D::Bind() const
{
	glBindTexture(GL_TEXTURE_2D, this->ID);
}

