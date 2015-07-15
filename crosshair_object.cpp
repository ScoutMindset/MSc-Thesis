/******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/

#include "crosshair_object.h"
#include <GLFW/glfw3.h>

CrosshairObject::CrosshairObject() : GameObject(glm::vec2(5,5), glm::vec2(12.5, 12.5)),PositionPrev(0,0) {}

CrosshairObject::CrosshairObject(glm::vec2 pos, glm::vec2 size, Texture2D texture) : 
	GameObject(pos, size, texture), PositionPrev(0, 0) {}

//glm::vec2 CrosshairObject::Move(GLfloat dt, GLuint window_width, GLuint window_height)
//{
//	// If not stuck to player board
//	// Move the ball
//	if (glm::length(this->Position - this->PositionPrev));
//		this->Position =  
//	this->Position += this->Velocity * dt;
//	// Then check if outside window bounds and if so, reverse velocity and restore at correct position
//	if (this->Position.x <= 0.0f)
//	{
//		this->Velocity.x = -this->Velocity.x;
//		this->Position.x = 0.0f;
//	}
//	else if (this->Position.x + this->Size.x >= window_width)
//	{
//		this->Velocity.x = -this->Velocity.x;
//		this->Position.x = window_width - this->Size.x;
//	}
//	if (this->Position.y <= 0.0f)
//	{
//		this->Velocity.y = -this->Velocity.y;
//		this->Position.y = 0.0f;
//	}
//	else if (this->Position.y + this->Size.y >= window_height)
//	{
//		this->Velocity.y = -this->Velocity.y;
//		this->Position.y = window_height - this->Size.x;
//	}
//
//	return this->Position;
//}

void CrosshairObject::Reset(glm::vec2 position, glm::vec2 velocity)
{
	this->Position = position;
	this->Velocity = velocity;
}

void CrosshairObject::ChangeTexture(Texture2D texture)
{

}