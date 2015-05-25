/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "ball_object.h"


// Game-related State data
SpriteRenderer  *Renderer;
BallObject		*Ball;

// Initial velocity of the ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
//Radius of the ball object
const GLfloat BALL_RADIUS = 22.5f;

enum Direction {
	UP,
	DOWN,
	LEFT,
	RIGHT
};

typedef std::tuple<GLboolean, Direction, glm::vec2> Collision;

Direction VectorDirection(glm::vec2 &target)
{
	glm::vec2 compass[] = {
		glm::vec2(0.0f, 1.0f),
		glm::vec2(0.0f, -1.0f),
		glm::vec2(-1.0f, 0.0f),
		glm::vec2(1.0f, 0.0f)
	};

	GLfloat current_result, max = 0.0f;
	GLint best_match;
	for (int i = 0; i < 4; i++)
	{
		current_result = glm::dot(target, compass[i]);
		if (current_result >= max)
		{
			best_match = i;
			max = current_result;
		}
	}
	return (Direction)best_match;
}

Collision CheckCollision(BallObject &one, GameObject &two)
{
	// Rectangular collisions
	/*bool collisionX = (one.Position.x + one.Size.x >= two.Position.x) && (two.Position.x + two.Size.x >= one.Position.x);
	bool collisionY = (one.Position.y + one.Size.y >= two.Position.y) && (two.Position.y + two.Size.y >= one.Position.y);
	return (collisionX && collisionY);*/

	// Circular collisions
	glm::vec2 ball_center(one.Position + one.Radius);
	glm::vec2 half_extents(two.Size.x / 2, two.Size.y / 2);
	glm::vec2 rectangle_center(two.Position.x + half_extents.x, two.Position.y + half_extents.y);

	glm::vec2 difference = ball_center - rectangle_center;
	glm::vec2 clamped = glm::clamp(difference, -half_extents, half_extents);
	glm::vec2 closest = rectangle_center + clamped;
	difference = closest - ball_center;
	if (glm::length(difference) <= one.Radius)
		return std::make_tuple(GL_TRUE, VectorDirection(difference), difference);
	else
		return std::make_tuple(GL_FALSE, UP, glm::vec2(0.0f, 0.0f));
}

Game::Game(GLuint width, GLuint height)
	: State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{

}

Game::~Game()
{
	delete Renderer;
}

void Game::Init()
{
	// Load shaders
	ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.frag", nullptr, "sprite");
	// Configure shaders
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
	ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
	// Load textures
	ResourceManager::LoadFileTexture("textures/awesomeface.png", GL_TRUE, "face");
	ResourceManager::LoadFileTexture("textures/paddle.png", true, "paddle");
	// Set render-specific controls
	Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
	// Configure game objects
	glm::vec2 ballPos = glm::vec2(this->Width / 2, this->Height/2);	
	Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
}

void Game::Update(GLfloat dt)
{
	Ball->Move(dt, this->Width, this->Height);
	this->DoCollisions();
}


void Game::ProcessInput(GLfloat dt)
{
	if (this->State == GAME_ACTIVE)
	{
		GLfloat velocity = PLAYER_VELOCITY * dt;
		// Move playerboard
		if (this->Keys[GLFW_KEY_A])
		{
			if (Ball->Position.x >= 0)
			{
				if (Ball->Stuck == true)
					Ball->Position.x -= velocity;
			}
		}
		if (this->Keys[GLFW_KEY_D])
		{
			if (Ball->Position.x <= this->Width - Ball->Size.x)
			{
				if (Ball->Stuck == true)
					Ball->Position.x += velocity;
			}
		}
		if (this->Keys[GLFW_KEY_SPACE])
			Ball->Stuck = false;
		if (this->Keys[GLFW_KEY_ENTER])
			Ball->Reset(glm::vec2(this->Width / 2, this->Height), INITIAL_BALL_VELOCITY);
	}
	else
	{
		if (this->Keys[GLFW_KEY_ENTER])
		{
			this->State = GAME_ACTIVE;
			
		}
	}
}

void Game::Render(cv::Mat &frame)
{
	if (this->State == GAME_ACTIVE)
	{
		// Draw background
		ResourceManager::LoadMatTexture(frame, GL_FALSE, "background");
		Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f);		
		Ball->Draw(*Renderer);
	}
}

void Game::DoCollisions()//cv::Mat &frame)
{
	glm::vec2 ball_center = Ball->Position + Ball->Radius;
	glm::vec2 velocity = glm::vec2(0,0);
	GLuint pixelValue;
	GLint ball_radius = Ball->Radius;
	GLint center_distanceX, center_distanceY;
	GLfloat current_velocity_length = glm::length(Ball->Velocity);
	int i, j;
	cv::namedWindow("Collision");
	for (i = Ball->Position.x; i <= Ball->Position.x + 2 * ball_radius; i++)
	{
		for (j = Ball->Position.y; j <= Ball->Position.y + 2 * ball_radius; j++)
		{
			if ((i == Ball->Position.x) && (j == Ball->Position.y))
			{
				currentFrame.at<cv::Vec3b>(j, i)[0] = 255;
				currentFrame.at<cv::Vec3b>(j, i)[1] = 255;
				currentFrame.at<cv::Vec3b>(j, i)[2] = 0;
			}
			center_distanceX = (ball_center.x - i);
			center_distanceY = (ball_center.y - j);
			pixelValue = playerFrame.at<uchar>(j,i);
			
			/*if (pixelValue == 255)
				std::cout << "whatever";*/
			if (sqrt(center_distanceX*center_distanceX + center_distanceY*center_distanceY) <= ball_radius)
			{
				currentFrame.at<cv::Vec3b>(j, i)[0] = 255;
				currentFrame.at<cv::Vec3b>(j, i)[1] = 0;
				currentFrame.at<cv::Vec3b>(j, i)[2] = 0;
				if (playerFrame.at<uchar>(j, i) == 255)
				{

					velocity += glm::vec2(center_distanceX, center_distanceY);
					Ball->Stuck = GL_FALSE;
				}
			}
		}
	}
	if (glm::length(velocity) > 0)
	{
		Ball->Velocity = glm::vec2(glm::length(INITIAL_BALL_VELOCITY)*velocity.x / (abs(velocity.x) + abs(velocity.y)),
			glm::length(INITIAL_BALL_VELOCITY)*velocity.y / (abs(velocity.x) + abs(velocity.y)));
		cv::imshow("Collision", this->playerFrame);
	}

}