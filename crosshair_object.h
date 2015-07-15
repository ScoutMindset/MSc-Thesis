/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/

#ifndef CROSSHAIROBJECT_H
#define CROSSHAIROBJECT_H

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.h"
#include "sprite_renderer.h"
#include "resource_manager.h"
#include "game_object.h"

class CrosshairObject : public GameObject
{
public:
	//CrosshairState
	CrosshairObject();
	CrosshairObject(glm::vec2 pos, glm::vec2 size);
	CrosshairObject(glm::vec2 pos, glm::vec2 size, Texture2D texture);
	glm::vec2   PositionPrev;

	//glm::vec2 Move(GLfloat dt, GLuint window_width, GLuint window_height);
	void Reset(glm::vec2 position, glm::vec2 velocity);
	void ChangeTexture(Texture2D texture);
};

#endif