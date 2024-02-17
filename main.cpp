#include <iostream>
#include <chrono>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

const int WIDTH = 1080, HEIGHT = 720;
const int BALL_WIDTH = 45, BALL_HEIGHT = 45;
const int PADDLE_WIDTH = 10, PADDLE_HEIGHT = 50;
const float PADDLE_SPEED = 1.0f;
const float BALL_SPEED = 1.0f;

enum Buttons
{
	PaddleOneUp = 0,
	PaddleOneDown,
	PaddleTwoUp,
	PaddleTwoDown,
};

enum class CollisionType
{
	None,
	Top,
	Middle,
	Bottom,
	Left,
	Right
};

struct Contact
{
	CollisionType type;
	float penetration;
};

class Vec2
{
public:
    float x, y;
	Vec2() : x(0.0f), y(0.0f){}

	Vec2(float x, float y) : x(x), y(y){}

	Vec2 operator+(Vec2 const& rhs)
	{
		return Vec2(x + rhs.x, y + rhs.y);
	}

	Vec2& operator+=(Vec2 const& rhs)
	{
		x += rhs.x;
		y += rhs.y;

		return *this;
	}

	Vec2 operator*(float rhs)
	{
		return Vec2(x * rhs, y * rhs);
	}

};


class Ball
{
public:

    Vec2 position;
	Vec2 velocity;
	SDL_Rect rect{};
    SDL_Texture *texture;

    Ball(Vec2 position, Vec2 velocity, SDL_Renderer* renderer)
    : position(position), velocity(velocity)
    {
    rect.x = static_cast<int>(position.x);
    rect.y = static_cast<int>(position.y);
    rect.w = BALL_WIDTH;
    rect.h = BALL_HEIGHT;

    SDL_Surface* imageSurface = IMG_Load("./assets/ball.png"); // Replace "image.png" with the path to your PNG image
    if (imageSurface == nullptr) {
        // Handle error loading image
    }

    texture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    SDL_FreeSurface(imageSurface);
    }

	void Update(float dt)
	{
		position += velocity * dt;
	}

	void Draw(SDL_Renderer* renderer)
	{
		rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);

		SDL_RenderCopy(renderer, texture, nullptr, &rect);
	}

	void CollideWithPaddle(Contact const& contact)
	{
		position.x += contact.penetration;
		velocity.x = -velocity.x;

		if (contact.type == CollisionType::Top)
		{
			velocity.y = -.75f * BALL_SPEED;
		}
		else if (contact.type == CollisionType::Bottom)
		{
			velocity.y = 0.75f * BALL_SPEED;
		}
	}

	void CollideWithWall(Contact const& contact)
	{
		if ((contact.type == CollisionType::Top)
		    || (contact.type == CollisionType::Bottom))
		{
			position.y += contact.penetration;
			velocity.y = -velocity.y;
		}
		else if (contact.type == CollisionType::Left)
		{
			position.x = WIDTH / 2.0f;
			position.y = HEIGHT / 2.0f;
			velocity.x = BALL_SPEED;
			velocity.y = 0.75f * BALL_SPEED;
		}
		else if (contact.type == CollisionType::Right)
		{
			position.x = WIDTH / 2.0f;
			position.y = HEIGHT / 2.0f;
			velocity.x = -BALL_SPEED;
			velocity.y = 0.75f * BALL_SPEED;
		}
	}

	
};


class Paddle
{
public:
	Paddle(Vec2 position, Vec2 v)
		: position(position), velocity(v)
	{
		rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);
		rect.w = PADDLE_WIDTH;
		rect.h = PADDLE_HEIGHT;
	}

	void Draw(SDL_Renderer* renderer)
	{
		rect.y = static_cast<int>(position.y);
		SDL_RenderFillRect(renderer, &rect);
	}

    void Update(float dt)
	{
		position += velocity * dt;

		if (position.y < 0)
		{
			// Restrict to top of the screen
			position.y = 0;
		}
		else if (position.y > (HEIGHT - PADDLE_HEIGHT))
		{
			// Restrict to bottom of the screen
			position.y = HEIGHT - PADDLE_HEIGHT;
		}
	}

	Vec2 position;
    Vec2 velocity;
	SDL_Rect rect{};
};

class PlayerScore
{
public:
	PlayerScore(Vec2 position, SDL_Renderer* renderer, TTF_Font* font)
		: renderer(renderer), font(font)
	{
		surface = TTF_RenderText_Solid(font, "0", {0xFF, 0xFF, 0xFF, 0xFF});
		texture = SDL_CreateTextureFromSurface(renderer, surface);

		int width, height;
		SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

		rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);
		rect.w = width;
		rect.h = height;
	}

	~PlayerScore()
	{
		SDL_FreeSurface(surface);
		SDL_DestroyTexture(texture);
	}

	void Draw()
	{
		SDL_RenderCopy(renderer, texture, nullptr, &rect);
	}

	void SetScore(int score)
	{
		SDL_FreeSurface(surface);
		SDL_DestroyTexture(texture);

		surface = TTF_RenderText_Solid(font, std::to_string(score).c_str(), {0xFF, 0xFF, 0xFF, 0xFF});
		texture = SDL_CreateTextureFromSurface(renderer, surface);

		int width, height;
		SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
		rect.w = width;
		rect.h = height;
	}

	SDL_Renderer* renderer;
	TTF_Font* font;
	SDL_Surface* surface{};
	SDL_Texture* texture{};
	SDL_Rect rect{};
};


//Helper Function
Contact  CheckPaddleCollision(Ball const& ball, Paddle const& paddle)
{
	float ballLeft = ball.position.x;
	float ballRight = ball.position.x + BALL_WIDTH;
	float ballTop = ball.position.y;
	float ballBottom = ball.position.y + BALL_HEIGHT;

	float paddleLeft = paddle.position.x;
	float paddleRight = paddle.position.x + PADDLE_WIDTH;
	float paddleTop = paddle.position.y;
	float paddleBottom = paddle.position.y + PADDLE_HEIGHT;

	Contact contact{};

	if (ballLeft >= paddleRight)
	{
		return contact;
	}

	if (ballRight <= paddleLeft)
	{
		return contact;
	}

	if (ballTop >= paddleBottom)
	{
		return contact;
	}

	if (ballBottom <= paddleTop)
	{
		return contact;
	}

	float paddleRangeUpper = paddleBottom - (2.0f * PADDLE_HEIGHT / 3.0f);
	float paddleRangeMiddle = paddleBottom - (PADDLE_HEIGHT / 3.0f);

	if (ball.velocity.x < 0)
	{
		// Left paddle
		contact.penetration = paddleRight - ballLeft;
	}
	else if (ball.velocity.x > 0)
	{
		// Right paddle
		contact.penetration = paddleLeft - ballRight;
	}

	if ((ballBottom > paddleTop)
	    && (ballBottom < paddleRangeUpper))
	{
		contact.type = CollisionType::Top;
	}
	else if ((ballBottom > paddleRangeUpper)
	     && (ballBottom < paddleRangeMiddle))
	{
		contact.type = CollisionType::Middle;
	}
	else
	{
		contact.type = CollisionType::Bottom;
	}

	return contact;
}


Contact CheckWallCollision(Ball const& ball)
{
	float ballLeft = ball.position.x;
	float ballRight = ball.position.x + BALL_WIDTH;
	float ballTop = ball.position.y;
	float ballBottom = ball.position.y + BALL_HEIGHT;

	Contact contact{};

	if (ballLeft < 0.0f)
	{
		contact.type = CollisionType::Left;
	}
	else if (ballRight > WIDTH)
	{
		contact.type = CollisionType::Right;
	}
	else if (ballTop < 0.0f)
	{
		contact.type = CollisionType::Top;
		contact.penetration = -ballTop;
	}
	else if (ballBottom > HEIGHT)
	{
		contact.type = CollisionType::Bottom;
		contact.penetration = HEIGHT - ballBottom;
	}

	return contact;
}



//Main
int main(int argc, char* argv[]) {
    //Init
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init(); // Score
    SDL_Window* window = SDL_CreateWindow("Tiny Ball", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) {
        std::cout << "Error" << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,0);
    TTF_Font* scoreFont = TTF_OpenFont("./assets/DejaVuSansMono.ttf", 40);

    //Init
    
    Ball ball(
	Vec2((WIDTH / 2.0f) - (BALL_WIDTH / 2.0f),
	(HEIGHT / 2.0f) - (BALL_WIDTH / 2.0f)),
	Vec2(BALL_SPEED, 0.0f),renderer);

    PlayerScore playerOneScoreText(Vec2(WIDTH / 4, 50), renderer, scoreFont);
    PlayerScore playerTwoScoreText(Vec2(3 * WIDTH / 4, 50), renderer, scoreFont);

	// Create the paddles
    Paddle paddleOneA(
            Vec2(80.0f, (HEIGHT / 2.0f) - (PADDLE_HEIGHT / 2.0f)),
            Vec2(0.0f, 0.0f));
	Paddle paddleOneB(
            Vec2(160.0f, (HEIGHT / 2.0f) - (PADDLE_HEIGHT / 2.0f)),
            Vec2(0.0f, 0.0f));

    Paddle paddleTwoA(
            Vec2(WIDTH - 80.0f, (HEIGHT / 2.0f) - (PADDLE_HEIGHT / 2.0f)),
            Vec2(0.0f, 0.0f));
	Paddle paddleTwoB(
            Vec2(WIDTH - 160.0f, (HEIGHT / 2.0f) - (PADDLE_HEIGHT / 2.0f)),
            Vec2(0.0f, 0.0f));
			
	Paddle* currentOne = &paddleOneA; 
	Paddle* currentTwo = &paddleTwoA;
    SDL_Surface * image = IMG_Load("./assets/football-pitch.png");
    SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, image); 

		int playerOneScore = 0;
		int playerTwoScore = 0;
		bool running = true;
		bool buttons[4] = {};

		float dt = 0.0f;

		while (running)
		{
			auto startTime = std::chrono::high_resolution_clock::now();

			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT)
				{
					running = false;
				}
				else if (event.type == SDL_KEYDOWN)
				{
					if (event.key.keysym.sym == SDLK_ESCAPE)
					{
						running = false;
					}
					else if (event.key.keysym.sym == SDLK_w)
					{
						buttons[Buttons::PaddleOneUp] = true;
					}
					else if (event.key.keysym.sym == SDLK_s)
					{
						buttons[Buttons::PaddleOneDown] = true;
					}
					else if (event.key.keysym.sym == SDLK_UP)
					{
						buttons[Buttons::PaddleTwoUp] = true;
					}
					else if (event.key.keysym.sym == SDLK_DOWN)
					{
						buttons[Buttons::PaddleTwoDown] = true;
					}
					else if(event.key.keysym.sym == SDLK_LSHIFT) {
						std::cout<<event.key.keysym.sym;
						if (currentOne == &paddleOneA) {
							currentOne = &paddleOneB;
						} else {
							currentOne = &paddleOneA;
						}
					}
					else if(event.key.keysym.sym == SDLK_RSHIFT) {
						std::cout<<event.key.keysym.sym;
						if (currentTwo == &paddleTwoA) {
							currentTwo = &paddleTwoB;
						} else {
							currentTwo = &paddleTwoA;
						}
					}
				}
				else if (event.type == SDL_KEYUP)
				{
					if (event.key.keysym.sym == SDLK_w)
					{
						buttons[Buttons::PaddleOneUp] = false;
					}
					else if (event.key.keysym.sym == SDLK_s)
					{
						buttons[Buttons::PaddleOneDown] = false;
					}
					else if (event.key.keysym.sym == SDLK_UP)
					{
						buttons[Buttons::PaddleTwoUp] = false;
					}
					else if (event.key.keysym.sym == SDLK_DOWN)
					{
						buttons[Buttons::PaddleTwoDown] = false;
					}
					
				}
				
			}


			if (buttons[Buttons::PaddleOneUp])
			{
				currentOne->velocity.y = -PADDLE_SPEED;
			}
			else if (buttons[Buttons::PaddleOneDown])
			{
				currentOne->velocity.y = PADDLE_SPEED;
			}
			else
			{
				currentOne->velocity.y = 0.0f;
			}

			if (buttons[Buttons::PaddleTwoUp])
			{
				currentTwo->velocity.y = -PADDLE_SPEED;
			}
			else if (buttons[Buttons::PaddleTwoDown])
			{
				currentTwo->velocity.y = PADDLE_SPEED;
			}
			else
			{
				currentTwo->velocity.y = 0.0f;
			}

		// Update the paddle positions
		paddleOneA.Update(dt);
		paddleOneB.Update(dt);
		paddleTwoA.Update(dt);
		paddleTwoB.Update(dt);

		// Update the ball position
		ball.Update(dt);

		// Check collisions
		if (Contact contact = CheckPaddleCollision(ball, paddleOneA);
			contact.type != CollisionType::None)
		{
			ball.CollideWithPaddle(contact);
		}
		else if (Contact contact = CheckPaddleCollision(ball, paddleOneB);
			contact.type != CollisionType::None)
		{
			ball.CollideWithPaddle(contact);
		}
		else if (contact = CheckPaddleCollision(ball, paddleTwoA);
			contact.type != CollisionType::None)
		{
			ball.CollideWithPaddle(contact);
		}
		else if (contact = CheckPaddleCollision(ball, paddleTwoB);
			contact.type != CollisionType::None)
		{
			ball.CollideWithPaddle(contact);
		}
		else if (contact = CheckWallCollision(ball);
			contact.type != CollisionType::None)
		{
			ball.CollideWithWall(contact);
			if (contact.type == CollisionType::Left)
			{
				++playerTwoScore;
				playerTwoScoreText.SetScore(playerTwoScore);
			}
			else if (contact.type == CollisionType::Right)
			{
				++playerOneScore;
				playerOneScoreText.SetScore(playerOneScore);
			}
		}

        // Clear the window to black
		SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF);
		SDL_RenderClear(renderer);
		//
		// Rendering will happen here
		//
        SDL_RenderCopy(renderer,texture,NULL,NULL);
		// SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

        
        // Draw the ball
        ball.Draw(renderer);

        // Draw the paddles
        paddleOneA.Draw(renderer);
		paddleOneB.Draw(renderer);
        paddleTwoA.Draw(renderer);
        paddleTwoB.Draw(renderer);

        // Display the scores
        playerOneScoreText.Draw();
        playerTwoScoreText.Draw();

		// Present the backbuffer
		SDL_RenderPresent(renderer);
        
        // Calculate frame time
        auto stopTime = std::chrono::high_resolution_clock::now();
	    dt = std::chrono::duration<float, std::chrono::milliseconds::period>(stopTime - startTime).count();
    }

    //Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_FreeSurface(image);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(scoreFont);
	TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}