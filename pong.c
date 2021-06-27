#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define WHITE 0xFFFFFFFF
#define BLACK 0x00000000

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define CLAMP(x, lo, hi) ((lo) > (x) ? (lo) : ((hi) < (x) ? (hi) : (x)))

/************************************
 * Typedefs
*************************************/

typedef unsigned char u8;
typedef unsigned int  u32;

typedef struct Window Window;
typedef struct Renderer Renderer;

typedef struct Ball Ball;
typedef struct Paddle Paddle;
typedef struct Game Game;

/************************************************
 * Prototypes
*************************************************/

Window*     window_create(const char* title, int w, int h, u8 fullscreen);
void        window_destroy(Window* self);
void        window_update(Window* self);

Renderer*   renderer_create(Window* window);
void        renderer_destroy(Renderer* self);
void        renderer_clear(Renderer* self, u32 color);
void        renderer_draw_title(Renderer* self);
void        renderer_draw_score(Renderer* self, u8 scores[], int side);
void        renderer_draw_gameover(Renderer* self, int winner);
void        renderer_draw_background(Renderer* self);
void        renderer_draw_ball(Renderer* self, Ball* b);
void        renderer_draw_paddle(Renderer* self, Paddle* p);
void        renderer_flush(Renderer* self);

void        paddle_update(Game* game);
void        ball_update(Game* game);
int         ball_collision(Ball* b, Paddle* p);

void        game_reset(Game* game);
void        game_render(Game* game);
void        game_init(Game* game, u8 flags);
void        game_run(Game* game);
void        game_shutdown(Game* game);


/************************************************
 * Definitions
*************************************************/

enum
{
    RIGHT   = 0,
    LEFT    = 1,
    UP      = 2,
    DOWN    = 4
};

enum
{
    STATE_TITLE = 0,
    STATE_PLAYING,
    STATE_GAMEOVER
};

struct Window
{
    SDL_Window* window;
    SDL_Event event;

    u8 keyboard[SDL_NUM_SCANCODES];
    
    int w, h;

    u8 should_close;
};

struct Renderer
{
    SDL_Renderer* renderer;

    SDL_Surface*  screen;
    SDL_Surface*  gameover;
    SDL_Surface*  score;
    SDL_Surface*  title;

    SDL_Texture*  buffer;
};

struct Ball
{
    int  x,  y;
    int  w,  h;
    int dx, dy;
};

struct Paddle
{
    int x, y;
    int w, h;
};

struct Game
{
    Window* window;
    Renderer* renderer;

    Paddle paddles[2];
    Ball ball;

    u8 scores[2];
    u8 multiplayer;
    u8 state;
};


/************************************************
 * Window functions
*************************************************/

Window* window_create(const char* title, int width, int height, u8 fullscreen)
{
    Window* window = malloc(sizeof(Window));
    SDL_assert(window);

    window->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_SHOWN);
    SDL_assert(window->window);

    memset(window->keyboard, 0, sizeof(window->keyboard));

    window->w = width;
    window->h = height;

    window->should_close = 0;

    return window;
}

void window_destroy(Window* window)
{
    SDL_DestroyWindow(window->window);

    memset(window->keyboard, 0, sizeof(window->keyboard));

    free(window);
    window = NULL;
}

void window_update(Window* window)
{
    SDL_PumpEvents();
    const u8* keys = SDL_GetKeyboardState(NULL);

    memcpy(window->keyboard, keys, SDL_NUM_SCANCODES);

    if (window->keyboard[SDL_SCANCODE_ESCAPE])
        window->should_close = 1;
}


/************************************************
 * Renderer functions
*************************************************/

Renderer* renderer_create(Window* window)
{
    Renderer* renderer = malloc(sizeof(Renderer));
    SDL_assert(renderer);

    renderer->renderer = SDL_CreateRenderer(window->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_assert(renderer->renderer);

    renderer->screen   = SDL_CreateRGBSurfaceWithFormat(0, window->w, window->h, 32, SDL_PIXELFORMAT_RGBA32);
    renderer->gameover = IMG_Load("res/gameover.png");
    renderer->score    = IMG_Load("res/score.png");
    renderer->title    = IMG_Load("res/title.png");
    renderer->buffer   = SDL_CreateTextureFromSurface(renderer->renderer, renderer->screen);
    SDL_assert(renderer->screen);
    SDL_assert(renderer->gameover);
    SDL_assert(renderer->score);
    SDL_assert(renderer->title);
    SDL_assert(renderer->buffer);

    return renderer;
}

void renderer_destroy(Renderer* renderer)
{
    SDL_FreeSurface(renderer->screen);
    SDL_FreeSurface(renderer->gameover);
    SDL_FreeSurface(renderer->score);
    SDL_FreeSurface(renderer->title);
    SDL_DestroyTexture(renderer->buffer);

    SDL_DestroyRenderer(renderer->renderer);

    free(renderer);
    renderer = NULL;
}

void renderer_clear(Renderer* renderer, u32 color)
{
    SDL_RenderClear(renderer->renderer);
    SDL_FillRect(renderer->screen, NULL, color);
}

void renderer_draw_title(Renderer* renderer)
{
    int x = (renderer->screen->w >> 1) - (renderer->title->w * 1.5f);
    int y = (renderer->screen->h >> 1) - (renderer->title->h * 1.5f);

    SDL_Rect src = {0, 0, renderer->title->w, renderer->title->h};
    SDL_Rect dst = {x, y, src.w * 3, src.h * 3};
    SDL_BlitScaled(renderer->title, &src, renderer->screen, &dst);
}

void renderer_draw_score(Renderer* renderer, u8 scores[], int side)
{
    int x_left  = (renderer->screen->w >> 1) - 50 - 35;
    int x_right = (renderer->screen->w >> 1) + 50;

    SDL_Rect src = {(scores[side] % 10) * 3, 0, 3, 5};
    SDL_Rect dst = {side == LEFT ? x_left : x_right, 0, 35, 100};
    SDL_BlitScaled(renderer->score, &src, renderer->screen, &dst);
}

void renderer_draw_gameover(Renderer* renderer, int winner)
{
    int x = (renderer->screen->w >> 1) - 212;
    int y = (renderer->screen->h >> 1) - 48;
    
    SDL_Rect src = {0, winner * 32, renderer->gameover->w, 32};
    SDL_Rect dst = {x, y, src.w << 2, src.h << 2};
    SDL_BlitScaled(renderer->gameover, &src, renderer->screen, &dst);
}

void renderer_draw_background(Renderer* renderer)
{
    SDL_Rect src = {(renderer->screen->w >> 1) - 3, 10, 6, 15};

    for ( ; src.y <= renderer->screen->h; src.y += 30)
        SDL_FillRect(renderer->screen, &src, WHITE);
}

void renderer_draw_ball(Renderer* self, Ball* b)
{
    SDL_Rect src = {b->x, b->y, b->w, b->h};
    SDL_FillRect(self->screen, &src, WHITE);
}

void renderer_draw_paddle(Renderer* renderer, Paddle* p)
{
    SDL_Rect src = {p->x, p->y, p->w, p->h};
    SDL_FillRect(renderer->screen, &src, WHITE);
}

void renderer_flush(Renderer* renderer)
{
    SDL_UpdateTexture(renderer->buffer, NULL, renderer->screen->pixels, renderer->screen->pitch);
    SDL_RenderCopy(renderer->renderer, renderer->buffer, NULL, NULL);
    SDL_RenderPresent(renderer->renderer);
}


/************************************************
 * Entity functions
*************************************************/

void paddle_update(Game* game)
{
    Ball* b = &game->ball;
    Paddle* rp = &game->paddles[RIGHT];
    Paddle* lp = &game->paddles[LEFT];

    u8* keyboard = game->window->keyboard;

    if (keyboard[SDL_SCANCODE_UP])    rp->y -= 5;
    if (keyboard[SDL_SCANCODE_DOWN])  rp->y += 5;

    if (game->multiplayer)
    {
        if (keyboard[SDL_SCANCODE_W]) lp->y -= 5;
        if (keyboard[SDL_SCANCODE_S]) lp->y += 5;
    }
    else
    {
        int p_center = lp->y + (lp->h >> 1);
        int b_center = b->y + (b->h >> 1);
        int s_center = game->window->h >> 1;
        int b_speed = ABS(b->dy) + (int)(ABS(b->dx) * 0.15f);

        // Ball is moving left
        if (b->dx < 0)
        {
            // The first ternary chooses the speed, the second chooses the direction
            lp->y -= (b->dy ? b_speed : 5) * (b_center < p_center ? 1 : -1);
        }
        // Ball is moving right, move the paddle back to the center
        else
        {
            // Stops the paddle from jittering back and forth
            if (p_center < s_center - 2 || p_center > s_center + 2)
                lp->y += (p_center < s_center) ? 5 : -5;
        }
    }

    // Keep paddles on the screen
    rp->y = CLAMP(rp->y, 0, game->window->h - rp->h);
    lp->y = CLAMP(lp->y, 0, game->window->h - lp->h);
}

void ball_update(Game* game)
{
    Ball* b = &game->ball;

    b->x += b->dx;
    b->y += b->dy;

    if (b->y < 0 || b->y > game->window->h - b->h)
    {
        b->dy = -b->dy;
    }

    if (b->x < 0)
    {
        game->scores[RIGHT]++;
        game_reset(game);
    }
    else if (b->x > game->window->w - b->w)
    {
        game->scores[LEFT]++;
        game_reset(game);
    }

    for (int i = 0; i < 2; ++i)
    {
        Paddle* p = &game->paddles[i];
        if (ball_collision(b, p))
        {
            // Negative if the ball's center hit above the paddles's center, positive if below
            int hit_pos = (b->y + (b->w >> 1)) - (p->y + (p->h >> 1));
            b->dy = hit_pos / 7;
            b->dx += (b->dx > 0) ? 1 : -1;
            b->dx = -b->dx;
            break;
        }
    }
}

int ball_collision(Ball* b, Paddle* p)
{
    if (b->x >= p->x + p->w || p->x >= b->x + b->w) return 0;
    if (b->y >= p->y + p->h || p->y >= b->y + b->h) return 0;
 
    return 1;
}


/************************************************
 * Game functions
*************************************************/

int check_scores(Game* game)
{
    if (game->scores[LEFT] == 10)  return 1;
    if (game->scores[RIGHT] == 10) return 0;
    
    return -1;
}

void game_reset(Game* game)
{
    game->paddles[RIGHT] = (Paddle){game->window->w - 30, (game->window->h >> 1) - 25, 10, 50};
    game->paddles[LEFT]  = (Paddle){20, (game->window->h >> 1) - 25, 10, 50};
    game->ball           = (Ball){(game->window->w >> 1) - 5, (game->window->h >> 1) - 5, 10, 10, 1, 1};
}

void game_render(Game* game)
{
    Renderer* renderer = game->renderer;

    renderer_clear(renderer, BLACK);

    switch (game->state)
    {
        case STATE_TITLE:
            renderer_draw_title(renderer);
            break;

        case STATE_PLAYING:
            renderer_draw_ball(renderer, &game->ball);

            renderer_draw_paddle(renderer, &game->paddles[RIGHT]);
            renderer_draw_paddle(renderer, &game->paddles[LEFT]);
            renderer_draw_score(renderer, game->scores, RIGHT);
            renderer_draw_score(renderer, game->scores, LEFT);
            renderer_draw_background(renderer);
            break;

        case STATE_GAMEOVER:
            renderer_draw_gameover(renderer, check_scores(game));

            renderer_draw_paddle(renderer, &game->paddles[RIGHT]);
            renderer_draw_paddle(renderer, &game->paddles[LEFT]);
            renderer_draw_score(renderer, game->scores, RIGHT);
            renderer_draw_score(renderer, game->scores, LEFT);
            renderer_draw_background(renderer);
            break;
        
        default: break;
    }

    renderer_flush(renderer);
}

void game_init(Game* game, u8 flags)
{
    SDL_assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    game->window      = window_create("Pong", 640, 480, (flags & 1));
    game->renderer    = renderer_create(game->window);
    game->multiplayer = (flags & 2);
    game->state       = STATE_TITLE;
    game_reset(game);
}

void game_run(Game* game)
{
    Window* window = game->window;

    while (!window->should_close)
    {
        switch (game->state)
        {
            case STATE_TITLE:
                if (window->keyboard[SDL_SCANCODE_SPACE])
                {
                    game->state = STATE_PLAYING;
                }
                break;

            case STATE_PLAYING:
                ball_update(game);
                paddle_update(game);
                if (check_scores(game) >= 0)
                {
                    game->state = STATE_GAMEOVER;
                }
                break;

            case STATE_GAMEOVER:
                if (window->keyboard[SDL_SCANCODE_R])
                {
                    game->scores[RIGHT] = game->scores[LEFT] = 0;
                    game->state = STATE_PLAYING;
                    game_reset(game);
                }
                break;
        }

        game_render(game);
        window_update(window);
    }
}

void game_shutdown(Game* game)
{
    renderer_destroy(game->renderer);
    window_destroy(game->window);
    SDL_Quit();
}


/************************************************
 * Main
*************************************************/

int main(int argc, char** argv)
{
    Game game;

    u8 flags = 0;
    for (int i = 0; i < argc; ++i)
    {
        if (strcmp("-f", argv[i]) == 0) flags |= 1;
        if (strcmp("-m", argv[i]) == 0) flags |= 2;
    }

    game_init(&game, flags);
    game_run(&game);
    game_shutdown(&game);
}
