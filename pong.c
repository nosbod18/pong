#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define WHITE 0xFFFFFFFF
#define BLACK 0x00000000

#define ABS(x)  ((x) < 0 ? -(x) : (x))
#define SIGN(x) (((x) > 0) - ((x) < 0))

// Declarations
//=======================================================================================================================
typedef unsigned char u8;
typedef unsigned int  u32;

typedef struct Window Window;
typedef struct Renderer Renderer;

typedef struct Ball Ball;
typedef struct Paddle Paddle;
typedef struct Game Game;
//=======================================================================================================================

// Prototypes
//=======================================================================================================================
Window*     window_create(const char* title, int w, int h, u8 fullscreen);
void        window_destroy(Window* self);
void        window_update(Window* self);

Renderer*   renderer_create(Window* window);
void        renderer_destroy(Renderer* self);
void        renderer_clear(Renderer* self, u32 color);
void        renderer_draw_gameover(Renderer* self, int winner);
void        renderer_draw_score(Renderer* self, u8 score, int side);
void        renderer_draw_title(Renderer* self);
void        renderer_draw_background(Renderer* self);
void        renderer_draw_ball(Renderer* self, Ball* b);
void        renderer_draw_paddle(Renderer* self, Paddle* p);
void        renderer_flush(Renderer* self);

void        paddle_update(Game* game);
void        paddle_ai(Game* game, int side);

void        ball_update(Game* game);
int         ball_collision(Ball* b, Paddle* p);

void        game_reset(Game* game);
void        game_init(Game* game, u8 flags);
void        game_run(Game* game);
void        game_shutdown(Game* game);
//=======================================================================================================================

// Definitions
//=======================================================================================================================
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
    SDL_Window* handle;
    SDL_Event event;

    u8 keyboard[SDL_NUM_SCANCODES];
    
    int w, h;

    double dt;
    double last_frame;
    double frame_timer;
    u32 frames;

    u8 should_close;
};

struct Renderer
{
    SDL_Renderer* handle;

    SDL_Surface*  screen;
    SDL_Surface*  gameover;
    SDL_Surface*  score;
    SDL_Surface*  title;

    SDL_Texture*  screen_tex;
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
    u8 time_scale;
};
//=======================================================================================================================

// Window functions
//=======================================================================================================================
Window* window_create(const char* title, int w, int h, u8 fullscreen)
{
    Window* self = malloc(sizeof(Window));
    SDL_assert(self);

    self->handle = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_SHOWN);
    SDL_assert(self->handle);

    memset(self->keyboard, 0, sizeof(self->keyboard));

    self->w = w;
    self->h = h;

    self->dt = 0.0;
    self->last_frame = 0.0;
    self->frame_timer = 0.0;
    self->frames = 0;

    self->should_close = 0;

    return self;
}

void window_destroy(Window* self)
{
    SDL_DestroyWindow(self->handle);

    memset(self->keyboard, 0, sizeof(self->keyboard));

    free(self);
    self = NULL;
}

void window_update(Window* self)
{
    SDL_PumpEvents();
    const u8* keys = SDL_GetKeyboardState(NULL);

    for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
        self->keyboard[i] = keys[i];

    if (self->keyboard[SDL_SCANCODE_ESCAPE])
        self->should_close = 1;


    self->frames++;
    self->frame_timer += self->dt;
    if (self->frame_timer >= 1.0)
    {
        char title[32];
        snprintf(title, 31, "Pong | FPS: %u", self->frames);
        SDL_SetWindowTitle(self->handle, title);
        self->frame_timer -= 1.0;
        self->frames = 0;
    }
}

// Renderer functions
//=======================================================================================================================
Renderer* renderer_create(Window* window)
{
    Renderer* self = malloc(sizeof(Renderer));
    SDL_assert(self);

    self->handle = SDL_CreateRenderer(window->handle, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_assert(self->handle);

    self->screen     = SDL_CreateRGBSurfaceWithFormat(0, window->w, window->h, 32, SDL_PIXELFORMAT_RGBA32);
    self->gameover   = IMG_Load("res/gameover.png");
    self->score      = IMG_Load("res/score.png");
    self->title      = IMG_Load("res/title.png");
    self->screen_tex = SDL_CreateTextureFromSurface(self->handle, self->screen);
    SDL_assert(self->screen);
    SDL_assert(self->gameover);
    SDL_assert(self->score);
    SDL_assert(self->title);
    SDL_assert(self->screen_tex);

    u32 colorkey = SDL_MapRGB(self->title->format, 255, 0, 255);
	SDL_SetColorKey(self->title, SDL_TRUE, colorkey);
	SDL_SetColorKey(self->score, SDL_TRUE, colorkey);

    return self;
}

void renderer_destroy(Renderer* self)
{
    SDL_FreeSurface(self->screen);
    SDL_FreeSurface(self->gameover);
    SDL_FreeSurface(self->score);
    SDL_FreeSurface(self->title);
    SDL_DestroyTexture(self->screen_tex);

    SDL_DestroyRenderer(self->handle);

    free(self);
    self = NULL;
}

void renderer_clear(Renderer* self, u32 color)
{
    SDL_RenderClear(self->handle);
    SDL_FillRect(self->screen, NULL, color);
}

void renderer_draw_gameover(Renderer* self, int winner)
{
    int x = (self->screen->w >> 1) - (self->title->w >> 1);
    int y = (self->screen->h >> 1) - (37 >> 1);

    SDL_Rect src = {0, 0, self->gameover->w, winner * 37};
    SDL_Rect dst = {x, y, src.w, src.h};
    SDL_BlitSurface(self->gameover, &src, self->screen, &dst);
}

void renderer_draw_score(Renderer* self, u8 score, int side)
{
    int x = (self->screen->w >> 1) - ((side == LEFT) ? self->score->w - 12 : -12);

    SDL_Rect src = {score * 3, 0, 3, self->score->h};
    SDL_Rect dst = {x, 0, 64, 64};
    SDL_BlitSurface(self->score, &src, self->screen, &dst);
}

void renderer_draw_title(Renderer* self)
{
    int x = (self->screen->w >> 1) - (self->title->w >> 1);
    int y = (self->screen->h >> 1) - (self->title->h >> 1);

    SDL_Rect src = {0, 0, self->title->w, self->title->h};
    SDL_Rect dst = {x, y, src.w, src.h};
    SDL_BlitSurface(self->title, &src, self->screen, &dst);
}

void renderer_draw_background(Renderer* self)
{
    SDL_Rect src = {self->screen->w >> 1, 15, 5, 15};

    for (int i = 0; i < 15; ++i)
    {
        SDL_FillRect(self->screen, &src, WHITE);
        src.y += 30;
    }
}

void renderer_draw_ball(Renderer* self, Ball* b)
{
    SDL_Rect src = {b->x, b->y, b->w, b->h};
    SDL_FillRect(self->screen, &src, WHITE);
}

void renderer_draw_paddle(Renderer* self, Paddle* p)
{
    SDL_Rect src = {p->x, p->y, p->w, p->h};
    SDL_FillRect(self->screen, &src, WHITE);
}

void renderer_flush(Renderer* self)
{
    SDL_UpdateTexture(self->screen_tex, NULL, self->screen->pixels, self->screen->pitch);
    SDL_RenderCopy(self->handle, self->screen_tex, NULL, NULL);
    SDL_RenderPresent(self->handle);

    renderer_clear(self, BLACK);
}
//=======================================================================================================================

// Paddle functions
//=======================================================================================================================
void paddle_update(Game* game)
{
    u8* keyboard = game->window->keyboard;

    if (keyboard[SDL_SCANCODE_UP])   game->paddles[RIGHT].y -= 5;
    if (keyboard[SDL_SCANCODE_DOWN]) game->paddles[RIGHT].y += 5;

    if (game->multiplayer)
    {
        if (keyboard[SDL_SCANCODE_W])    game->paddles[LEFT].y -= 5;
        if (keyboard[SDL_SCANCODE_S])    game->paddles[LEFT].y += 5;
    }
    else
    {
        Ball* b = &game->ball;
        Paddle* p = &game->paddles[LEFT];
        int p_center = p->y + (p->h >> 1);
        int b_center = b->y + (b->h >> 1);
        int s_center = game->window->h >> 1;
        int b_speed = ABS(b->dy) + ABS(b->dx) * 0.05f;

        // Ball is moving left
        if (b->dx < 0)
        {
            // I wanted to see if i could make this as compact as possible, I think i succeeded :)
            p->y -= (b->dy ? b_speed : 5) * (b_center < p_center ? 1 : -1);
        }
        // Ball is moving right
        else
        {
            // Stops the jittering
            if (p_center < s_center - 2 || p_center > s_center + 2)
                p->y += (p_center < s_center) ? 5 : -5;
        }  
    }
}
//=======================================================================================================================

// Ball functions
//=======================================================================================================================
int ball_collision(Ball* b, Paddle* p)
{
    if (b->x >= p->x + p->w || p->x >= b->x + b->w) return 0;
    if (b->y >= p->y + p->h || p->y >= b->y + b->h) return 0;
 
    return 1;
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
        if (ball_collision(b, &game->paddles[i]))
        {
            // Negative if the ball's center hit above the paddles's center, positive if below
            int hit_pos = (game->paddles[i].y + game->paddles[i].h) - b->y;
            b->dy = hit_pos / 7;
            b->dx += (b->dx > 0) ? -(b->dx + 1) : -(b->dx - 1);
        }
    }
}
//=======================================================================================================================


// Game functions
//=======================================================================================================================
int check_scores(Game* game)
{
    if (game->scores[LEFT] == 10)
        return 0;
    else if (game->scores[RIGHT] == 10)
        return (game->multiplayer) ? 1 : 2;
    else
        return -1;
}

void game_reset(Game* game)
{
    game->paddles[RIGHT] = (Paddle){game->window->w - 30, (game->window->h >> 1) - 50, 10, 50};
    game->paddles[LEFT]  = (Paddle){20, (game->window->h >> 1) - 50, 10, 50};
    game->ball           = (Ball){game->window->w >> 1, game->window->h >> 1, 10, 10, 1, 1};
    game->scores[RIGHT]  = 0;
    game->scores[LEFT]   = 0;
}

void game_init(Game* game, u8 flags)
{
    SDL_assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    game->window = window_create("Pong | FPS: 0", 640, 480, ((flags & 1) == 1));
    game->renderer = renderer_create(game->window);
    game->multiplayer = ((flags & 2) == 2);
    game->state = STATE_TITLE;
    game_reset(game);
}

void game_run(Game* game)
{
    Window* window = game->window;
    Renderer* renderer = game->renderer;

    while (!window->should_close)
    {
        const double now = (double)SDL_GetTicks();
        const double elapsed_ms = now - window->last_frame;
        window->last_frame = now;

        // Frame time in seconds
        window->dt = elapsed_ms / 1000.0;

        if (game->state == STATE_TITLE)
        {
            renderer_draw_title(renderer);
            
            if (game->window->keyboard[SDL_SCANCODE_SPACE])
                game->state = STATE_PLAYING;

        }
        else if (game->state == STATE_PLAYING)
        {
            ball_update(game);
            paddle_update(game);

            renderer_draw_ball(renderer, &game->ball);
            renderer_draw_paddle(renderer, &game->paddles[RIGHT]);
            renderer_draw_paddle(renderer, &game->paddles[LEFT]);
            renderer_draw_score(renderer, game->scores[RIGHT], RIGHT);
            renderer_draw_score(renderer, game->scores[LEFT], LEFT);
            renderer_draw_background(renderer);

            if (check_scores(game) >= 0)
                game->state = STATE_GAMEOVER;

        }
        else if (game->state == STATE_GAMEOVER)
        {
            renderer_draw_gameover(renderer, check_scores(game));

            if (game->window->keyboard[SDL_SCANCODE_R])
            {
                game_reset(game);
                game->state = STATE_PLAYING;
            }
        }

        renderer_flush(renderer);
        window_update(window);
    }
}

void game_shutdown(Game* game)
{
    renderer_destroy(game->renderer);
    window_destroy(game->window);
    SDL_Quit();
}

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
//=======================================================================================================================
