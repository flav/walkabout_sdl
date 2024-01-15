#include <stdlib.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Note: this must be after SDL includes
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define PLAYER_SIZE 20

typedef struct {
	int x, y;
} player_t;

typedef struct {
	int done;
	player_t player;
	SDL_Texture *player_image;
	SDL_Renderer *renderer;
} game_t;

int
process_events(game_t *game) {
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
			case SDL_QUIT:
				return 1;
				break;

			case SDL_KEYDOWN:
				{
					switch(e.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							return 1;
							break;
						case SDLK_q:
							return 1;
							break;
					}
				}
				break;

			default: {}
		}
	}

	const Uint8 *state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_L]) {
		game->player.x += 20;
		if (game->player.x > SCREEN_WIDTH - PLAYER_SIZE) {
			game->player.x = SCREEN_WIDTH - PLAYER_SIZE;
		}
	}
	if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_H]) {
		game->player.x -= PLAYER_SIZE;
		if (game->player.x < 0) {
			game->player.x = 0;
		}
	}
	if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_K]) {
		game->player.y -= PLAYER_SIZE;
		if (game->player.y < 0) {
			game->player.y = 0;
		}
	}
	if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_J]) {
		game->player.y += PLAYER_SIZE;
		if (game->player.y > SCREEN_HEIGHT - PLAYER_SIZE) {
			game->player.y = SCREEN_HEIGHT - PLAYER_SIZE;
		}
	}

	return 0;
}

void
render_game(SDL_Renderer *renderer, const game_t *game) {



	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // blue(00,00,ff)
	SDL_RenderClear(renderer);

	// SDL_RenderCopy(renderer, game->player_image, NULL, NULL);

	SDL_Rect dest;
	SDL_QueryTexture(game->player_image, NULL, NULL, &dest.w, &dest.h);
	dest.x = game->player.x;
	dest.y = game->player.y;
	dest.w = dest.w * 4;
	dest.h = dest.h * 4;

	SDL_RenderCopy(renderer, game->player_image, NULL, &dest);


	// SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // white(ff,ff,ff)
	// SDL_Rect rect = {game->player.x, game->player.y, PLAYER_SIZE, PLAYER_SIZE };
	// SDL_RenderFillRect(renderer, &rect);
}

// main_loop(game_t *game) {
void
main_loop(void *arg) {
	game_t *game = arg;

	game->done = process_events(game);

	SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);

	SDL_Delay(60);

	render_game(game->renderer, game);

	SDL_RenderPresent(game->renderer);
}

int main(int argc, char *argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "Could not initialize sdl2: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_Window *window = SDL_CreateWindow("Hello Wolrd",
										  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
										  SCREEN_WIDTH, SCREEN_HEIGHT,
										  SDL_WINDOW_SHOWN);

	if (window == NULL) {
		fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	game_t game = {
		.done = 0,
		.player = {
			55,
			55
		}
	};


	// SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
	game.renderer = SDL_CreateRenderer(window, -1,
												SDL_RENDERER_ACCELERATED |
												SDL_RENDERER_PRESENTVSYNC);
	if (game.renderer == NULL) {
		SDL_DestroyWindow(window);
		fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_Surface *image = IMG_Load("resources/tile_0485.png");
	if (!image) {
		printf("IMG_Load: %s\n", IMG_GetError());
		return 0;
	}

	// SDL_Texture* tex = SDL_CreateTextureFromSurface(game.renderer, image);
	game.player_image = SDL_CreateTextureFromSurface(game.renderer, image);
	SDL_FreeSurface(image);
	if (!game.player_image) {
		fprintf(stderr, "SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	printf("which path taken?\n");
	#ifdef __EMSCRIPTEN__
		printf("PATH OF emscripten\n");
		emscripten_set_main_loop_arg(main_loop, &game, -1, 1);
	#else
		printf("PATH OF gameloop\n");
		while (!game.done) {
			main_loop(&game);
		}
	#endif

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(game.renderer);
	SDL_Quit();

	return EXIT_SUCCESS;
}
