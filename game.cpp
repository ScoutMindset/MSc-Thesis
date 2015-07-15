/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include <vector>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "ball_object.h"
#include "crosshair_object.h"

// Game-related State data
SpriteRenderer  *Renderer;
BallObject		*Ball;
std::vector<BallObject> Balls;
CrosshairObject *Crosshair;


// Initial velocity of the ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
//Radius of the ball object
const GLfloat BALL_RADIUS = 22.5f;
const glm::vec2 CROSSHAIR_SIZE(50.0f, 50.0f);
const int noBalls = 5;

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
	double xPos, yPos;
	// Initialize random number generator
	srand(time(NULL));	
	// Load shaders
	ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.frag", nullptr, "sprite");
	// Configure shaders
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
	ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
	// Load textures
	ResourceManager::LoadFileTexture("textures/awesomeface.png", GL_TRUE, "face");
	ResourceManager::LoadFileTexture("textures/crosshairDisactive.png", GL_TRUE, "crosshair");
	// Set render-specific controls
	Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
	// Configure game objects
	glm::vec2 ballPos = glm::vec2(this->Width / 2, this->Height/2);	
	glm::vec2 crosshairPos = glm::vec2(this->Width/4, this->Height/4);
	//Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
	for (int i = 0; i < noBalls; i++)
	{
		xPos = rand() % this->Width;
		yPos = rand() % this->Height / 2;
		Ball = new BallObject(glm::vec2(xPos, yPos), BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
		Balls.push_back(*Ball);// BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face")); //Ball;// (glm::vec2(xPos, yPos), BALL_RADIUS, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
	}
	Crosshair = new CrosshairObject(crosshairPos, CROSSHAIR_SIZE, ResourceManager::GetTexture("crosshair"));
}

void Game::Update(GLfloat dt)
{
	Crosshair->Position = this->CursorPosition;
	Crosshair->PositionPrev = this->CursorPositionPrev;
	//Ball->Move(dt, this->Width, this->Height);
	for (std::vector<BallObject>::iterator i = Balls.begin(); i != Balls.end(); i++)
	{
		i->Move(dt, this->Width, this->Height);
	}
	this->DoCollisions();
}

void Game::CursorUpdate()
{
	Crosshair->Position = this->CursorPosition;
	Crosshair->PositionPrev = this->CursorPositionPrev;
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
					Ball->Position.x -= 5;
			}
		}
		if (this->Keys[GLFW_KEY_D])
		{
			if (Ball->Position.x <= this->Width - Ball->Size.x)
			{
				if (Ball->Stuck == true)
					Ball->Position.x += 5;
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
		Crosshair->Draw(*Renderer);

		//if (!Ball->Destroyed)
		//	Ball->Draw(*Renderer);
		for (std::vector<BallObject>::iterator i = Balls.begin(); i != Balls.end(); i++)
		{
			if (!(i->Destroyed))
				i->Draw(*Renderer);
		}
	}
}

void Game::DoCollisions()//cv::Mat &frame)
{
	glm::vec2 bBall_center, cBall_center;// = Ball->Position + Ball->Radius;
	GLint bBall_radius, cBall_radius;// = Ball->Radius;
	GLfloat current_velocity_length;// = glm::length(Ball->Velocity);

	glm::vec2 velocity = glm::vec2(0,0);
	GLuint pixelValue;
	
	GLint center_distanceX, center_distanceY, ball_distanceX, ball_distanceY;
	
	int i, j;

	for (std::vector<BallObject>::iterator b = Balls.begin(); b != Balls.end(); b++)
	{
		if (!b->Destroyed)
		{
			bBall_center = b->Position + b->Radius;
			bBall_radius = b->Radius;
			velocity = glm::vec2(0, 0);
			current_velocity_length = glm::length(b->Velocity);
			for (std::vector<BallObject>::iterator c = Balls.begin(); c != Balls.end(); c++)
			{
				if (!c->Destroyed)
				{
					cBall_center = c->Position + c->Radius;
					cBall_radius = c->Radius;
					if (b != c)
					{
						for (i = b->Position.x; i <= b->Position.x + 2 * bBall_radius; i++)
						{
							for (j = b->Position.y; j <= b->Position.y + 2 * bBall_radius; j++)
							{
								//if ((i == Ball->Position.x) && (j == Ball->Position.y))
								//{
								//	currentFrame.at<cv::Vec3b>(j, i)[0] = 255;
								//	currentFrame.at<cv::Vec3b>(j, i)[1] = 255;
								//	currentFrame.at<cv::Vec3b>(j, i)[2] = 0;
								//}
								center_distanceX = (bBall_center.x - i);
								center_distanceY = (bBall_center.y - j);

								if (sqrt(center_distanceX*center_distanceX + center_distanceY*center_distanceY) <= bBall_radius)
								{
									//currentFrame.at<cv::Vec3b>(j, i)[0] = 255;
									//currentFrame.at<cv::Vec3b>(j, i)[1] = 0;
									//currentFrame.at<cv::Vec3b>(j, i)[2] = 0;
									ball_distanceX = (cBall_center.x - i);
									ball_distanceY = (cBall_center.y - j);

									if ((playerFrame.at<uchar>(j, i) == 255) ||
										(sqrt(ball_distanceX*ball_distanceX + ball_distanceY*ball_distanceY) <= cBall_radius))
									{

										velocity += glm::vec2(center_distanceX, center_distanceY);
										b->Stuck = GL_FALSE;
									}
								}
							}
						}
						if (glm::length(velocity) > 0)
						{
							b->Velocity = glm::vec2(glm::length(INITIAL_BALL_VELOCITY)*velocity.x / (abs(velocity.x) + abs(velocity.y)),
								glm::length(INITIAL_BALL_VELOCITY)*velocity.y / (abs(velocity.x) + abs(velocity.y)));
						}
					}
				}
			}
		}
	}

}

void Game::CheckCrosshair()
{
	for (std::vector<BallObject>::iterator i = Balls.begin(); i != Balls.end(); i++)
	{
		if ( ((i->Position.x > Crosshair->Position.x) && (i->Position.x<Crosshair->Position.x + Crosshair->Size.x)
			&& (i->Position.y > Crosshair->Position.y) && (i->Position.y < Crosshair->Position.y + Crosshair->Size.y))||

			((i->Position.x + i->Radius * 2> Crosshair->Position.x) && (i->Position.x + i->Radius * 2 <Crosshair->Position.x + Crosshair->Size.x)
			&& (i->Position.y > Crosshair->Position.y) && (i->Position.y < Crosshair->Position.y + Crosshair->Size.y))||

			((i->Position.x > Crosshair->Position.x) && (i->Position.x<Crosshair->Position.x + Crosshair->Size.x)
			&& (i->Position.y + i->Radius * 2 > Crosshair->Position.y) && (i->Position.y + i->Radius * 2 < Crosshair->Position.y + Crosshair->Size.y)) ||

			((i->Position.x + i->Radius * 2> Crosshair->Position.x) && (i->Position.x + i->Radius * 2 <Crosshair->Position.x + Crosshair->Size.x)
			&& (i->Position.y + i->Radius * 2> Crosshair->Position.y) && (i->Position.y + i->Radius * 2< Crosshair->Position.y + Crosshair->Size.y)))
		{
			i->Destroyed = GL_TRUE;
		}
	}
}

void Game::ActivateCrosshair()
{
	ResourceManager::LoadFileTexture("textures/crosshairActive.png", GL_TRUE, "crosshair");
}

void Game::DisactivateCrosshair()
{
	ResourceManager::LoadFileTexture("textures/crosshairDisactive.png", GL_TRUE, "crosshair");
}
