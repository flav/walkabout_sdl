#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Note: this must be after SDL includes
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define TILE_COLUMNS 27
#define TILE_SIZE 16

#define TILE_SIZE_MULTIPLIER 4

#define PLAYER_SIZE TILE_SIZE * TILE_SIZE_MULTIPLIER
#define PLAYER_MOVEMENT 10

#define WORLD_WIDTH_TILES 80
#define WORLD_HEIGHT_TILES 80
#define WORLD_WIDTH WORLD_WIDTH_TILES * 16
#define WORLD_HEIGHT WORLD_HEIGHT_TILES * 16
#define WORLD_TILE_SIZE WORLD_WIDTH_TILES * WORLD_HEIGHT_TILES

enum {DIR_NONE, DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_WEST};

typedef struct {
	int x, y;
	int direction;
	int animation_frame;
} player_t;

typedef struct {
	int tiles[WORLD_TILE_SIZE];
} world_layer;

typedef struct {
	int done;
	player_t player;
	SDL_Texture *game_tiles;
	SDL_Renderer *renderer;
	world_layer map;
	world_layer overlay;
	SDL_Rect camera;
	int debug;
} game_t;


int
calculate_player_tile(const game_t * game) {
	// player tile
	int col = floor(game->player.x / (TILE_SIZE * TILE_SIZE_MULTIPLIER));
	int row = floor(game->player.y / (TILE_SIZE * TILE_SIZE_MULTIPLIER));
	int player_tile_location = row * TILE_COLUMNS + col;

	if (game->debug) {
		printf(
			"Position: %d %d tile: %d\n",
			game->player.x,
			game->player.y,
			player_tile_location
		);
	}
	return player_tile_location;
}

int
overlay_collision(const game_t * game) {
	return game->overlay.tiles[calculate_player_tile(game)];
}

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
						case SDLK_p:
							game->debug = !game->debug;
							break;
					}
				}
				break;

			case SDL_MOUSEMOTION:
				// printf("mouse %i %i\n", e.motion.x, e.motion.y);
				break;

			default: {}
		}
	}

	int is_moving = 0;
	int last_position = 0;
	const Uint8 *state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_L] || state[SDL_SCANCODE_D]) {
		is_moving = 1;
		game->player.direction = DIR_EAST;

		last_position = game->player.x;
		game->player.x += PLAYER_MOVEMENT;
		if (overlay_collision(game)) {
			game->player.x = last_position;
		}
		if (game->player.x > WORLD_WIDTH - PLAYER_SIZE) {
			game->player.x = WORLD_WIDTH - PLAYER_SIZE;
		}
	}
	if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_H] || state[SDL_SCANCODE_A]) {
		is_moving = 1;
		game->player.direction = DIR_WEST;

		last_position = game->player.x;
		game->player.x -= PLAYER_MOVEMENT;
		if (overlay_collision(game)) {
			game->player.x = last_position;
		}
		if (game->player.x < 0) {
			game->player.x = 0;
		}
	}
	if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_K] || state[SDL_SCANCODE_W]) {
		is_moving = 1;
		game->player.direction = DIR_NORTH;

		last_position = game->player.y;
		game->player.y -= PLAYER_MOVEMENT;
		if (overlay_collision(game)) {
			game->player.y = last_position;
		}
		if (game->player.y < 0) {
			game->player.y = 0;
		}
	}
	if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_J] || state[SDL_SCANCODE_S]) {
		is_moving = 1;
		game->player.direction = DIR_SOUTH;

		last_position = game->player.y;
		game->player.y += PLAYER_MOVEMENT;
		if (overlay_collision(game)) {
			game->player.y = last_position;
		}
		if (game->player.y > WORLD_HEIGHT - PLAYER_SIZE) {
			game->player.y = WORLD_HEIGHT - PLAYER_SIZE;
		}
	}

	if (is_moving) {
		game->player.animation_frame = ++game->player.animation_frame % 3;
	} else {
		game->player.animation_frame = 0;
		game->player.direction = DIR_NONE;
	}

	game->camera.x = game->player.x - SCREEN_WIDTH / 2;
	game->camera.y = game->player.y - SCREEN_HEIGHT / 2;

	if (game->camera.x < 0) {
		game->camera.x = 0;
	}
	if (game->camera.x > WORLD_WIDTH - SCREEN_WIDTH) {
		game->camera.x = WORLD_WIDTH - SCREEN_WIDTH;
	}
	if (game->camera.y < 0) {
		game->camera.y = 0;
	}
	if (game->camera.y > WORLD_HEIGHT - SCREEN_HEIGHT) {
		game->camera.y = WORLD_HEIGHT - SCREEN_HEIGHT;
	}

	return 0;
}

void
render_player(const game_t *game) {
	int col = 23;
	int row = 0 + game->player.animation_frame;
	// W S N E
	switch (game->player.direction) {
		case DIR_WEST:
			col += 0;
			break;
		case DIR_EAST:
			col += 3;
			break;
		case DIR_NORTH:
			col += 2;
			break;
		case DIR_SOUTH:
			// fall through to default
		default:
			col += 1;
			break;
	}

	SDL_Rect source = {
		.x = col * 16,
		.y = row * 16,
		.h = 16,
		.w = 16
	};
	SDL_Rect dest = {
		.x = game->player.x - game->camera.x - PLAYER_SIZE / 2,
		.y = game->player.y - game->camera.y - PLAYER_SIZE / 2,
		.h = source.h * TILE_SIZE_MULTIPLIER,
		.w = source.w * TILE_SIZE_MULTIPLIER
	};

	SDL_RenderCopy(game->renderer, game->game_tiles, &source, &dest);
}

void
render_tile(const game_t *game, int tile, int x, int y) {
	int col = tile % TILE_COLUMNS;
	double row = floor(tile / TILE_COLUMNS);

	SDL_Rect source = {
		.x = col * 16,
		.y = row * 16,
		.h = 16,
		.w = 16
	};
	SDL_Rect dest = {
		.x = x - game->camera.x,
		.y = y - game->camera.y,
		.h = source.h * TILE_SIZE_MULTIPLIER,
		.w = source.w * TILE_SIZE_MULTIPLIER
	};

	SDL_RenderCopy(game->renderer, game->game_tiles, &source, &dest);

	if (game->debug) {
		SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
		SDL_RenderDrawRect(game->renderer, &dest);
	}
}

void
render_map(const game_t *game) {
	for(int i = 0; i < WORLD_TILE_SIZE; ++i) {
		int col = i % TILE_COLUMNS;
		double row = floor(i / TILE_COLUMNS);

		render_tile(
			game,
			game->map.tiles[i],
			col * TILE_SIZE * TILE_SIZE_MULTIPLIER,
			row * TILE_SIZE * TILE_SIZE_MULTIPLIER
		);

		if (game->overlay.tiles[i]) {
			render_tile(
				game,
				game->overlay.tiles[i],
				col * TILE_SIZE * TILE_SIZE_MULTIPLIER,
				row * TILE_SIZE * TILE_SIZE_MULTIPLIER
			);
		}
	}
}

void
render_game(const game_t *game) {
	SDL_Renderer *renderer = game->renderer;

	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // blue(00,00,ff)
	SDL_RenderClear(renderer);

	render_map(game);
	render_player(game);

	if (game->debug) {
		int padding = 10;
		SDL_Rect rect = {
			.x = padding - game->camera.x,
			.y = padding - game->camera.y,
			.w = WORLD_WIDTH - (padding * 2),
			.h = WORLD_HEIGHT - (padding * 2)
		};

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderDrawRect(renderer, &rect);

		SDL_RenderDrawLine(
			renderer,
			SCREEN_WIDTH / 2,
			0,
			SCREEN_WIDTH / 2,
			SCREEN_HEIGHT
		);
		SDL_RenderDrawLine(
			renderer,
			0,
			SCREEN_HEIGHT / 2,
			SCREEN_WIDTH,
			SCREEN_HEIGHT / 2
		);
	}

	SDL_RenderPresent(game->renderer);
}

// main_loop(game_t *game) {
void
main_loop(void *arg) {
	game_t *game = arg;

	// Do event loop
	game->done = process_events(game);

	// Do physics loop

	// Do rendering loop
	render_game(game);

	SDL_Delay(80);
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
		.debug = 0,
		.player = {
			.x = 400,
			.y = 400,
			.direction = 0,
			.animation_frame = 0
		}
	};

	srand(141);
	for(int i = 0; i < WORLD_TILE_SIZE; ++i) {
		game.map.tiles[i] = 28;

		game.overlay.tiles[i] = 0;
		if (rand() % 10 == 0) {
			game.overlay.tiles[i] = 373;
		} else if (rand() % 10 == 0) {
			game.overlay.tiles[i] = 372;
		}
	}

	// SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
	game.renderer = SDL_CreateRenderer(window, -1,
												SDL_RENDERER_ACCELERATED |
												SDL_RENDERER_PRESENTVSYNC);
	if (game.renderer == NULL) {
		SDL_DestroyWindow(window);
		fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_Surface *image = IMG_Load("resources/tilemap_packed.png");
	if (!image) {
		printf("IMG_Load: %s\n", IMG_GetError());
		return 0;
	}

	// SDL_Texture* tex = SDL_CreateTextureFromSurface(game.renderer, image);
	game.game_tiles = SDL_CreateTextureFromSurface(game.renderer, image);
	SDL_FreeSurface(image);
	if (!game.game_tiles) {
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
