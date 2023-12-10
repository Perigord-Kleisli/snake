#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <stdlib.h>
#include <wchar.h>
#include <math.h>
#include <time.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define GAME_WIDTH 32
#define GAME_HEIGHT 18
#define CELL_SIZE 60
#define SCREEN_WIDTH (GAME_WIDTH * CELL_SIZE)
#define SCREEN_HEIGHT (GAME_HEIGHT * CELL_SIZE)

typedef enum { UP, DOWN, LEFT, RIGHT } Direction;

typedef struct {
  float x;
  float y;
} Coordinate;

typedef struct {
  bool keep_window_open;
  SDL_Window *window;
  SDL_Renderer *renderer;
  float last_frame_time;
  float speed; // Cells per second
  Coordinate food;
  Coordinate *snake;
  int snake_length;
  Direction direction;
} GameState;

GameState make_initial_gamestate(SDL_Window *window, SDL_Renderer *renderer) {
  GameState state;
  state.keep_window_open = true;
  state.window = window;
  state.renderer = renderer;
  state.last_frame_time = 0;
  state.speed = 1;
  srand(time(NULL));
  Coordinate food_coord = {rand() % GAME_WIDTH, rand() % GAME_HEIGHT};
  state.food = food_coord;
  state.snake_length = 1;
  state.snake = (Coordinate*)malloc(sizeof(Coordinate));
  Coordinate initial_snake_coord = { GAME_WIDTH/2, GAME_HEIGHT/2 };
  state.snake[0] = initial_snake_coord;
  state.direction = DOWN;
  return state;
}

void process_input(GameState *gamestate) {
  SDL_Event event;
  SDL_PollEvent(&event);
  switch (event.type) {
  case SDL_QUIT:
    gamestate->keep_window_open = false;
    break;
  case SDL_KEYDOWN:
    switch (event.key.keysym.sym) {
    case SDLK_q:
      gamestate->keep_window_open = false;
      break;
    case SDLK_w:
      gamestate->direction = UP;
      break;
    case SDLK_a:
      gamestate->direction = LEFT;
      break;
    case SDLK_d:
      gamestate->direction = RIGHT;
      break;
    case SDLK_s:
      gamestate->direction = DOWN;
      break;
    }
  }
}

float fmod_neg(float a, float n) {          
  return a - n * floor(a / n);
}

bool in_same_cell(Coordinate a, Coordinate b) {
  return (int)a.x == (int)b.x && (int)a.y == (int)b.y;
}

bool intersects(Coordinate a, Coordinate b) {
  return b.x <= a.x && a.x <= (b.x + 1) && b.y <= a.y && a.y <= (b.y + 1)
    ||   b.x <= (a.x + 1) && (a.x + 1) <= (b.x + 1) && b.y <= a.y && a.y <= (b.y + 1)
    ||   b.x <= (a.x + 1) && (a.x + 1) <= (b.x + 1) && b.y <= (a.y + 1) && (a.y + 1) <= (b.y + 1)
    ||   b.x <= a.x && a.x <= (b.x + 1) && b.y <= (a.y + 1) && (a.y + 1) <= (b.y + 1);
}

bool any_intersects(Coordinate* as, int a_length, Coordinate b) {
  for (int i = 0; i < a_length; i++) {
    if (intersects(as[i],b)) return true;
  }
  return false;
}

bool intersects_with_treshold(Coordinate a, Coordinate b, float treshold) {
  return (b.x + treshold) <= a.x && a.x <= (b.x + 1 - treshold) 
	  && (b.y + treshold) <= a.y && a.y <= (b.y + 1 - treshold)
    ||   (b.x + treshold) <= (a.x + 1) && (a.x + 1) <= (b.x + 1 - treshold) 
          && (b.y + treshold) <= a.y && a.y <= (b.y + 1 - treshold)
    ||   (b.x + treshold) <= (a.x + 1) && (a.x + 1) <= (b.x + 1 - treshold) 
          && (b.y + treshold) <= (a.y + 1) && (a.y + 1) <= (b.y + 1 - treshold)
    ||   (b.x + treshold) <= a.x && a.x <= (b.x + 1 - treshold) 
          && (b.y + treshold) <= (a.y + 1) && (a.y + 1) <= (b.y + 1 - treshold);
}

void update(GameState *state) {
  float delta_time = ((SDL_GetTicks() - state->last_frame_time) / 1000.0f) * state->speed;

  Coordinate previous_cell = state->snake[0];
  for (int i = 0; i < state->snake_length; i++) {
    Coordinate temp = state->snake[i];
    state->snake[i] = previous_cell;
    previous_cell = temp;
  }

  if (state->snake_length >= 80) {
    bool all_same = true;
    for (int i = 1; i < state->snake_length; i++) {
      if (state->snake[0].x != state->snake[i].x || state->snake[0].y != state->snake[i].y) {
	all_same = false;
        break;
      }
    }

    if (all_same) state->keep_window_open = false;
    for (int i = 80; i < state->snake_length; i++) {
      if (intersects_with_treshold(state->snake[0], state->snake[i], 0.2)) {
        state->speed = 0;
      }
    } 
  }

  switch (state->direction) {
    case UP: 
      state->snake[0].y =
      	fmod_neg(state->snake[0].y - delta_time * 8, GAME_HEIGHT);
      break;
    case DOWN: 
      state->snake[0].y =
      	fmod(state->snake[0].y + delta_time * 8, GAME_HEIGHT);
      break;
    case LEFT: 
      state->snake[0].x =
        fmod_neg(state->snake[0].x - delta_time * 8, GAME_WIDTH);
      break;
    case RIGHT: 
      state->snake[0].x =
        fmod(state->snake[0].x + delta_time * 8, GAME_WIDTH);
      break;
  }

  state->food.x = fmod_neg(state->food.x + ((float)(rand() % 10) - 4.5) / 500, GAME_WIDTH);
  state->food.y = fmod_neg(state->food.y + ((float)(rand() % 10) - 4.5) / 500, GAME_HEIGHT);

  
  if (intersects(state->snake[0], state->food)) {

    while (any_intersects(state->snake, state->snake_length, state->food)) {
      state->food.x = rand() % GAME_WIDTH;
      state->food.y = rand() % GAME_HEIGHT;
    }
    
    state->speed += 0.02;

    state->snake = realloc(state->snake, sizeof(Coordinate) * (state->snake_length + 20));
    for (int i = 0; i < 20; i++) {
      state->snake[state->snake_length+i] = state->snake[state->snake_length-1];
    }
    state->snake_length += 20;

  }

  state->last_frame_time = SDL_GetTicks();
}

void render(GameState *state) {
  SDL_SetRenderDrawColor(state->renderer, 0x00, 0x00, 0x00, 0xFF);
  SDL_RenderClear(state->renderer);

  
  SDL_SetRenderDrawColor(state->renderer, 0x33, 0x33, 0x33, 0x88);
  for (int i = 0; i < GAME_HEIGHT; i++) {
    for (int j = 0; j < GAME_WIDTH; j++) {
      SDL_Rect bg_grid_cell = {j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE};
      SDL_RenderDrawRect(state->renderer, &bg_grid_cell);
    }
  }
  if (state->speed == 0) {
    SDL_SetRenderDrawColor(state->renderer, 0xFF, 0x00, 0x00, 0xFF);
    for (int i = 0; i < state->snake_length; i++) {
          SDL_Rect snakeCell = {
          	(state->snake[i].x) * CELL_SIZE,
          	(state->snake[i].y) * CELL_SIZE,
                 	CELL_SIZE,
                 	CELL_SIZE};
          SDL_RenderDrawRect(state->renderer, &snakeCell);
    }
  } else {
    SDL_SetRenderDrawColor(state->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    for (int i = 0; i < state->snake_length; i++) {
          SDL_Rect snakeCell = {
          	(state->snake[i].x) * CELL_SIZE,
          	(state->snake[i].y) * CELL_SIZE,
                 	CELL_SIZE,
                 	CELL_SIZE};
          SDL_RenderFillRect(state->renderer, &snakeCell);
    }
  }

 SDL_Rect foodCell = {
	state->food.x * CELL_SIZE + 4,
        state->food.y * CELL_SIZE + 4,
	CELL_SIZE - 8,
	CELL_SIZE - 8};
  SDL_RenderFillRect(state->renderer, &foodCell);
  SDL_RenderPresent(state->renderer);
}

GameState main_state;

void mainLoop() {
  if (!main_state.keep_window_open) {
#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop(); /* this should "kill" the app. */
#endif
  }
  process_input(&main_state);
  update(&main_state);
  render(&main_state);
}

int main() {
  SDL_Window *window;
  SDL_Renderer *renderer;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s",
                 SDL_GetError());
    return EXIT_FAILURE;
  }

  if (SDL_CreateWindowAndRenderer(1920, 1080, SDL_WINDOW_RESIZABLE, &window,
                                  &renderer)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Couldn't create window and renderer: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_SetWindowTitle(window, "Pong");

  main_state = make_initial_gamestate(window, renderer);
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(mainLoop, 0, 1);
#else
  while (main_state.keep_window_open) {
    mainLoop();
  }
#endif

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
