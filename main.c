#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "./constants.h"

int game_is_running = FALSE;
int move_ball = FALSE;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int last_frame_time = 0;
int last_paddle_hit = 0;
int ball_speed;

// Initialize the game objects for this Pong game
struct game_object {
	float x;
	float y;
	float width;
	float height;
	float vel_x;
	float vel_y;
} ball, paddle1, paddle2;

int initialize_window(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Error initializing SDL.\n");
		return FALSE;
	}

	window = SDL_CreateWindow(
		NULL,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_BORDERLESS
	);

	if (!window) {
		fprintf(stderr, "Error creating SDL Window.\n");
		return FALSE;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);

	if (!renderer) {
		fprintf(stderr, "Error creating SDL Renderer.\n");
		return FALSE;
	}

	return TRUE;
}

void process_input() {
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type) {
	case SDL_QUIT:
		game_is_running = FALSE;
		break;
	case SDL_KEYDOWN:
		if (event.key.keysym.sym == SDLK_ESCAPE)
			game_is_running = FALSE;
		else if (event.key.keysym.sym == SDLK_DOWN)
			// Move paddle 1 down
			paddle1.y += paddle1.vel_y;
		else if (event.key.keysym.sym == SDLK_UP)
			// Move paddle 1 up
			paddle1.y -= paddle1.vel_y;
		else if (event.key.keysym.sym == SDLK_s)
			// Move paddle 2 down
			paddle2.y += paddle2.vel_y;
		else if (event.key.keysym.sym == SDLK_w)
			// Move paddle 2 up
			paddle2.y -= paddle2.vel_y;
		break;
	}
}

void render() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	// Draw the ball
	SDL_Rect ball_rect = { (int)ball.x, (int)ball.y, (int)ball.width, (int)ball.height };

	// Draw the paddles
	SDL_Rect paddle1_rect = { (int)paddle1.x, (int)paddle1.y, (int)paddle1.width, (int)paddle1.height };
	SDL_Rect paddle2_rect = { (int)paddle2.x, (int)paddle2.y, (int)paddle2.width, (int)paddle2.height };

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &ball_rect);
	SDL_RenderFillRect(renderer, &paddle1_rect);
	SDL_RenderFillRect(renderer, &paddle2_rect);

	SDL_RenderPresent(renderer);
}

void start_ball(void) {
	ball.x = (int)WINDOW_WIDTH / 2;
	ball.y = (int)WINDOW_HEIGHT / 2;
	ball.width = 15;
	ball.height = 15;
	ball.vel_x = -ball_speed;
	ball.vel_y = ball_speed;
}

void start_paddles(void) {
	paddle1.x = PADDLE1_X;
	paddle1.width = PADDLE_WIDTH;
	paddle1.height = PADDLE_HEIGHT;
	paddle1.y = PADDLE_Y;
	paddle1.vel_y = PADDLE_VELOCITY;

	paddle2.x = PADDLE2_X;
	paddle2.width = PADDLE_WIDTH;
	paddle2.height = PADDLE_HEIGHT;
	paddle2.y = PADDLE_Y;
	paddle2.vel_y = PADDLE_VELOCITY;
}

void setup() {
	ball_speed = INITIAL_BALL_SPEED;
	ball.vel_x = ball_speed;
	ball.vel_y = ball_speed;
	move_ball = FALSE;

	start_ball();
	start_paddles();
	render();

	// Wait a bit of time before making the ball move
	SDL_Delay(1000);

	move_ball = TRUE;
}

void reverse_ball_direction(void) {
	last_paddle_hit = SDL_GetTicks();
	
	ball_speed += BALL_SPEED_INCREMENT;

	if (ball.vel_x > 0) {
		ball.vel_x = -ball_speed;
	}
	else {
		ball.vel_x = ball_speed;
	}
}

void check_ball_bounds(void) {
	if (ball.x < 0) {
		setup();
	}
	if (ball.y < 0) {
		ball.y = 0;
		ball.vel_y = -ball.vel_y;
	}
	if (ball.x + ball.width > WINDOW_WIDTH) {
		setup();
	}
	if (ball.y + ball.height > WINDOW_HEIGHT) {
		ball.y = WINDOW_HEIGHT - ball.height;
		ball.vel_y = -ball.vel_y;
	}

	if (SDL_GetTicks() - last_paddle_hit >= 100) {
		int absolute_paddle1_difference_x = abs((int)(ball.x + ball.width) - (int)(paddle1.x + paddle1.width));
		int paddle1_boundingbox_y = (int)paddle1.y + paddle1.height;

		int absolute_paddle2_difference_x = abs((int)(ball.x + ball.width) - (int)(paddle2.x + paddle1.width));
		int paddle2_boundingbox_y = (int)paddle2.y + paddle2.height;

		if (absolute_paddle1_difference_x <= 15) {
			if ((ball.y + ball.height) >= paddle1.y && (ball.y + ball.height) < paddle1_boundingbox_y) {
				reverse_ball_direction();
			}
		}

		if (absolute_paddle2_difference_x <= 15) {
			if ((ball.y + ball.height) >= paddle2.y && (ball.y + ball.height) < paddle2_boundingbox_y) {
				reverse_ball_direction();
			}
		}
	}
}

void check_paddles_bounds() {
	if (paddle1.y + paddle1.height > WINDOW_HEIGHT) {
		paddle1.y = WINDOW_HEIGHT - paddle1.height;
	} else if (paddle1.y < 0) {
		paddle1.y = 0;
	}

	if (paddle2.y + paddle2.height > WINDOW_HEIGHT) {
		paddle2.y = WINDOW_HEIGHT - paddle2.height;
	}
	else if (paddle2.y < 0) {
		paddle2.y = 0;
	}
}

void update(void) {
	// Get a delta time factor converted to seconds to be used to update my objects
	float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;

	last_frame_time = SDL_GetTicks();

	ball.x += ball.vel_x * delta_time;
	ball.y += ball.vel_y * delta_time;

	check_ball_bounds();
	check_paddles_bounds();
}

void destroy_window() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main(int argc, char* args[]) {
	game_is_running = initialize_window();
		
	setup();

	while (game_is_running) {
		process_input();
		if (move_ball == TRUE) {
			update();
		}
		render();
	}

	destroy_window();

	return 0;
}