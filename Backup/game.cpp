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
GameObject      *Player;
BallObject		*Ball;

// Initial velocity of the ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
//Radius of the ball object
const GLfloat BALL_RADIUS = 12.5f;

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
	delete Player;
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
	glm::vec2 playerPos = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
	glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2);
	Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
	Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
}

void Game::Update(GLfloat dt)
{
	Ball->Move(dt, this->Width, this->Height);
	this->DoCollisions();
	//if (Ball->Position.y >= this->Height)
	//{
	//	//this->ResetLevel();
	//	this->ResetPlayer();
	//}
}


void Game::ProcessInput(GLfloat dt)
{
	if (this->State == GAME_ACTIVE)
	{
		GLfloat velocity = PLAYER_VELOCITY * dt;
		// Move playerboard
		if (this->Keys[GLFW_KEY_A])
		{
			if (Player->Position.x >= 0)
			{
				Player->Position.x -= velocity;
				if (Ball->Stuck == true)
					Ball->Position.x -= velocity;
			}
		}
		if (this->Keys[GLFW_KEY_D])
		{
			if (Player->Position.x <= this->Width - Player->Size.x)
			{
				Player->Position.x += velocity;
				if (Ball->Stuck == true)
					Ball->Position.x += velocity;
			}
		}
		if (this->Keys[GLFW_KEY_SPACE])
			Ball->Stuck = false;
		if (this->Keys[GLFW_KEY_ENTER])
			Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2), INITIAL_BALL_VELOCITY);
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
		// Draw level
		//this->Levels[this->Level].Draw(*Renderer);
		// Draw player
		Player->Draw(*Renderer);
		Ball->Draw(*Renderer);
	}
}

void Game::DoCollisions()
{
	//for (GameObject &box : this->Levels[this->Level].Bricks)
	//{
	//	if (!box.Destroyed)
	//	{
	//		Collision collision = CheckCollision(*Ball, box);
	//		if (std::get<0>(collision))
	//		{
	//			if (!box.IsSolid)
	//				box.Destroyed = GL_TRUE;
	//			Direction dir = std::get<1>(collision);
	//			glm::vec2 collision_vector = std::get<2>(collision);
	//			if ((dir == LEFT) || (dir == RIGHT))
	//			{
	//				Ball->Velocity.x = -Ball->Velocity.x;
	//				GLfloat penetration = Ball->Radius - std::abs(collision_vector.x);
	//				if (dir == LEFT)
	//					Ball->Position.x -= penetration;
	//				else
	//					Ball->Position.x += penetration;
	//			}
	//			else
	//			{
	//				Ball->Velocity.y = -Ball->Velocity.y;
	//				GLfloat penetration = Ball->Radius - std::abs(collision_vector.y);
	//				if (dir == UP)
	//					Ball->Position.y -= penetration;
	//				else
	//					Ball->Position.y += penetration;
	//			}
	//		}
	//	}
	//}
	Collision collision = CheckCollision(*Ball, *Player);
	if (!Ball->Stuck && std::get<0>(collision))
	{
		GLfloat centerBoard = Player->Position.x + Player->Size.x / 2;
		GLfloat distance = (Ball->Position.x + Ball->Radius) - centerBoard;
		GLfloat percentage = distance / (Player->Size.x / 2);

		GLfloat strength = 2.0f;

		glm::vec2 oldVelocity = Ball->Velocity;
		Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
		Ball->Velocity.y = -1 * abs(Ball->Velocity.y);
		Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity); // We normalize the velocity to keep it constant!
	}

}

//void Game::ResetLevel()
//{
//	if (this->Level == 0)this->Levels[0].Load("levels/one.lvl", this->Width, this->Height * 0.5f);
//	else if (this->Level == 1)
//		this->Levels[1].Load("levels/two.lvl", this->Width, this->Height * 0.5f);
//	else if (this->Level == 2)
//		this->Levels[2].Load("levels/three.lvl", this->Width, this->Height * 0.5f);
//	else if (this->Level == 3)
//		this->Levels[3].Load("levels/four.lvl", this->Width, this->Height * 0.5f);
//}

void Game::ResetPlayer()
{
	Player->Size = PLAYER_SIZE;
	Player->Position = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
	Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -(BALL_RADIUS * 2)), INITIAL_BALL_VELOCITY);
}