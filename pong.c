#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Misc
//==========================================================================================================================
#define SCREEN_W   640
#define SCREEN_H   480
#define SCREEN_W_2 320
#define SCREEN_H_2 240

#define BLACK 0x00000000
#define WHITE 0xFFFFFFFF

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define RANDOM(min, max) ((min) + ((float)rand() / ((float)RAND_MAX / ((max) - (min)))))

typedef unsigned char u8;
typedef unsigned int  u32;

enum
{
    LEFT  = 0,
    RIGHT = 1,
    UP    = 2,
    DOWN  = 4
};

typedef union V2
{
    struct { int x, y; };
    struct { int w, h; };
} V2;
//==========================================================================================================================

// Graphics
//==========================================================================================================================
enum
{
    TEXTURE_SCREEN = 0,
    TEXTURE_TITLE,
    TEXTURE_SCORE,
    TEXTURE_GAMEOVER,
    TEXTURE_ENTITIES,

    TEXTURE_LAST
};

typedef struct Window
{
    SDL_Window* handle;
    SDL_Event event;
    V2 size;
    u8 should_close;
} Window;

typedef struct Renderer
{
    SDL_Renderer* handle;
    SDL_Texture* tex[TEXTURE_LAST];

    u32* raw;
    u32  raw_size;
} Renderer;
//==========================================================================================================================

// Game
//==========================================================================================================================
typedef enum State
{
    STATE_TITLE = 0,
    STATE_PLAYING,
    STATE_GAMEOVER
} State;

typedef struct Ball
{
    V2 pos;
    V2 size;
    V2 vel;
} Ball;

typedef struct Paddle
{
    V2 pos;
    V2 size;
} Paddle;

typedef struct Game
{
    Window* window;
    Renderer* renderer;

    Ball ball;
    Paddle paddles[2];

    u8 scores[2];

    State state;

    double dt;
    u8 player2;
} Game;

// Global game
Game game;

//==========================================================================================================================

// V2 functions
//==========================================================================================================================
V2 v2(int x, int y)       { return (V2){{x, y}}; }

V2 v2_add(V2 a, V2 b)     { return v2(a.x + b.x, a.y + b.y); }
V2 v2_sub(V2 a, V2 b)     { return v2(a.x - b.x, a.y - b.y); }
V2 v2_mul(V2 a, V2 b)     { return v2(a.x * b.x, a.y * b.y); }

V2 v2_add_s(V2 a, int b)  { return v2(a.x + b, a.y + b); }
V2 v2_sub_s(V2 a, int b)  { return v2(a.x - b, a.y - b); }
V2 v2_mul_s(V2 a, int b)  { return v2(a.x * b, a.y * b); }

V2 v2_half(V2 a)          { return v2(a.x >> 1, a.y >> 1); }
V2 v2_abs(V2 a)           { return v2(ABS(a.x), ABS(a.y)); }
//==========================================================================================================================

// Utility functions
//==========================================================================================================================
int check_scores()
{
    if (game.scores[LEFT] == 10 || game.scores[RIGHT] == 10)
    {
        game.scores[LEFT] = game.scores[RIGHT] = 0;
        return 1;
    }
    return 0;
}
//==========================================================================================================================

// Window functions
//==========================================================================================================================
void window_init(const char* title, int w, int h, u8 fullscreen)
{
    Window* window = game.window;
    window = malloc(sizeof(Window));
    SDL_assert(window);

    window->handle = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_SHOWN);
    SDL_assert(window->handle);

    SDL_GetWindowSize(window->handle, &window->size.w, &window->size.h);
    window->should_close = 0;
}

void window_destroy()
{
    SDL_DestroyWindow(game.window->handle);

    free(game.window);
    game.window = NULL;
}

void window_poll_events()
{
    SDL_PumpEvents();

    while (SDL_PollEvent(&game.window->event))
    {
        switch (game.window->event.type)
        {
            case SDL_QUIT:
                game.window->should_close = 1;
                break;

            case SDL_KEYDOWN:
                if (game.window->event.key.keysym.sym == SDLK_ESCAPE)
                    game.window->should_close = 1;
                break;

            default: break;
        }
    }
}
//==========================================================================================================================

// Renderer functions
//==========================================================================================================================
void renderer_init()
{
    game.renderer = malloc(sizeof(Renderer));
    SDL_assert(game.renderer);

    game.renderer->handle = SDL_CreateRenderer(game.window->handle, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    SDL_assert(game.renderer->handle);

    game.renderer->tex[TEXTURE_SCREEN]   = SDL_CreateTexture(game.renderer->handle, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_W, SCREEN_H);
    game.renderer->tex[TEXTURE_TITLE]    = IMG_LoadTexture(game.renderer->handle, "res/title.png");
    game.renderer->tex[TEXTURE_SCORE]    = IMG_LoadTexture(game.renderer->handle, "res/score.png");
    game.renderer->tex[TEXTURE_GAMEOVER] = IMG_LoadTexture(game.renderer->handle, "res/gameover.png");
    game.renderer->tex[TEXTURE_ENTITIES] = SDL_CreateTexture(game.renderer->handle, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_W, SCREEN_H);
    SDL_assert(game.renderer->tex[TEXTURE_TITLE]);
    SDL_assert(game.renderer->tex[TEXTURE_SCORE]);
    SDL_assert(game.renderer->tex[TEXTURE_GAMEOVER]);
    SDL_assert(game.renderer->tex[TEXTURE_ENTITIES]);

    game.renderer->raw_size = SCREEN_W * SCREEN_H * sizeof(u32);
    game.renderer->raw = malloc(game.renderer->raw_size);
    SDL_assert(game.renderer->raw);

}

void renderer_destroy()
{
    for (int i = 0; i < TEXTURE_LAST; ++i)
        SDL_DestroyTexture(game.renderer->tex[i]);

    SDL_DestroyRenderer(game.renderer->handle);

    free(game.renderer->raw);
    game.renderer->raw = NULL;

    free(game.renderer);
    game.renderer = NULL;
}

void renderer_clear(u32 color)
{
    for (u32 i = 0; i < game.renderer->raw_size; ++i)
        game.renderer->raw[i] = color;
}

void renderer_draw_point(int x, int y)
{
    game.renderer->raw[y * SCREEN_W + x] = WHITE;
}

void renderer_fill_rect(V2 pos, V2 size)
{
    V2 min = pos;
    V2 max = v2_add(pos, size);

    for (int x = min.x; x < max.x; ++x)
        for (int y = min.y; y < max.y; ++y)
            renderer_draw_point(x, y);
}

void renderer_draw_ball()
{
    renderer_fill_rect(game.ball.pos, game.ball.size);
}

void renderer_draw_paddles()
{
    renderer_fill_rect(game.paddles[LEFT].pos, game.paddles[LEFT].size);
    renderer_fill_rect(game.paddles[RIGHT].pos, game.paddles[RIGHT].size);
}

void renderer_draw_background()
{
    V2 pos  = v2(SCREEN_W_2 - 2, 8);
    V2 size = v2(5, 15);

	for(int i = 0; i < 16; ++i)
    {	
		renderer_fill_rect(pos, size);
        pos.y += 30;
	}
}

void renderer_draw_title()
{
    SDL_Rect dst;
    dst.x = (game.window->size.w >> 1) - 40;
	dst.y = (game.window->size.h >> 1) - 32;
	dst.w = 80;
	dst.h = 64;
    SDL_RenderCopy(game.renderer->handle, game.renderer->tex[TEXTURE_TITLE], NULL, &dst);
}

void renderer_draw_scores()
{
    SDL_Rect src1, src2;
    SDL_Rect dst1, dst2;

    src1.x = src1.y = src2.x = src2.y = 0;
    src1.w = src2.w = 

    SDL_RenderCopy(game.renderer->handle, game.renderer->tex[TEXTURE_SCORE], &src1, &dst1);
    SDL_RenderCopy(game.renderer->handle, game.renderer->tex[TEXTURE_SCORE], &src2, &dst2);
}

void renderer_draw_gameover(u8 winner)
{
    SDL_Rect src = {0, 0, 128, 37};
    
    if (winner != RIGHT)
        src.h *= game.player2 ? 2 : 3;

    SDL_RenderCopy(game.renderer->handle, game.renderer->tex[TEXTURE_GAMEOVER], &src, NULL);
}

void renderer_flush()
{
    SDL_UpdateTexture(game.renderer->tex[TEXTURE_ENTITIES], NULL, game.renderer->raw, SCREEN_W * sizeof(u32));
    SDL_RenderCopy(game.renderer->handle, game.renderer->tex[TEXTURE_ENTITIES], NULL, NULL);
    SDL_RenderPresent(game.renderer->handle);
}
//==========================================================================================================================

// Paddle functions
//==========================================================================================================================
void paddle_move(int side, int dir)
{
    Paddle* p = &game.paddles[side];

    if ((dir & UP) == UP)
    {
        p->pos.y -= 5;

        if (p->pos.y < 0)
            p->pos.y = 0;
    }
    
    if ((dir & DOWN) == DOWN)
    {
        p->pos.y += 5;

        if (p->pos.y > SCREEN_H - p->size.h)
            p->pos.y = SCREEN_H - p->size.h;
    }
}

void paddle_ai(int side)
{
    Paddle* p = &game.paddles[side];
    Ball* b   = &game.ball;

    int p_center = p->pos.y + (p->size.h >> 1);
	int b_center = b->pos.y + (b->size.h >> 1);
	int s_center = game.window->size.h >> 1;
	int b_speed  = ABS(b->vel.y) + ABS(b->vel.x) * 0.25f;

    // Ball is moving left
    if (b->vel.x < 0)
    {
		// Ball is moving up or down
		if (b->vel.y)
			p->pos.y -= (b_center < p_center) ? b_speed : -b_speed;
		else
			p->pos.y -= (b_center < p_center) ? 5 : -5;
    }
    // Ball is moving right
    else
	{
		// Stops the jittering
		if (p_center < s_center - 2 || p_center > s_center + 2)
			p->pos.y += (p_center < s_center) ? 5 : -5;
	}

    if (p->pos.y < 0)                       p->pos.y = 0;
    if (p->pos.y > SCREEN_H - p->size.h)    p->pos.y = SCREEN_H - p->size.h;
}

void paddles_update()
{
    const u8* keys = SDL_GetKeyboardState(NULL);

    // 1 if the respective up key is pressed, 2 if the respective down key is pressed, 3 if they are both pressed
    u8 keys_right = (keys[SDL_SCANCODE_UP] << UP) | (keys[SDL_SCANCODE_DOWN] << DOWN);
    u8 keys_left  = (keys[SDL_SCANCODE_W]  << UP) | (keys[SDL_SCANCODE_S]    << DOWN);

    paddle_move(RIGHT, keys_right);

    if (game.player2)
        paddle_move(LEFT, keys_left);
    else
        paddle_ai(LEFT);
}
//==========================================================================================================================

// Ball functions
//==========================================================================================================================
int ball_collide_with(Paddle* p)
{
    Ball* b = &game.ball;

    V2 bmax = v2_add(b->pos, v2_half(b->size));
    V2 pmax = v2_add(p->pos, v2_half(p->size));

    if(bmax.x < p->pos.x || b->pos.x > pmax.x) return 0;
    if(bmax.y < p->pos.y || b->pos.y > pmax.y) return 0;

    return 1;
}

void ball_update()
{
    Ball* b = &game.ball;
    b->pos = v2_add(b->pos, b->vel);

    for (int i = 0; i < 2; ++i)
    {
        if (ball_collide_with(&game.paddles[i]))
        {
            b->vel.x += b->vel.x < 0 ? -1 : 1;
            b->vel.x = -b->vel.x;

            // Positive if the ball hits below the paddle's center, negative if above
            int hit_pos = (b->pos.y + (b->size.h >> 1)) - (game.paddles[i].pos.y + (game.paddles[i].size.h >> 1));

            b->vel.y = hit_pos / 7;
        }
    }
}
//==========================================================================================================================

// Game functions
//==========================================================================================================================
void game_reset()
{
    game.ball           = (Ball){v2(SCREEN_W_2 - 5, SCREEN_H_2 - 5), v2(10, 10), v2(1, RANDOM(0.0f, 1.0f) > 0.5f ? 1 : -1)};
    game.paddles[LEFT]  = (Paddle){v2(20, SCREEN_H_2 - 50), v2(10, 50)};
    game.paddles[RIGHT] = (Paddle){v2(SCREEN_W - 30, SCREEN_H_2 - 50), v2(10, 50)};
    game.scores[LEFT]   = 0;
    game.scores[RIGHT]  = 0;
}

void game_render()
{
    renderer_draw_ball();
    renderer_draw_paddles();

    if (game.state == STATE_TITLE)
    {
        renderer_draw_title();
    }
    else if (game.state == STATE_PLAYING)
    {
        renderer_draw_background();
        renderer_draw_scores();
    }
    else if (game.state == STATE_GAMEOVER)
    {
        renderer_draw_gameover(game.scores[LEFT] == 10 ? LEFT : RIGHT);
    }

    SDL_UpdateTexture(game.renderer->tex[TEXTURE_ENTITIES], NULL, game.renderer->raw, SCREEN_W * sizeof(u32));
    SDL_RenderCopy(game.renderer->handle, game.renderer->tex[TEXTURE_ENTITIES], NULL, NULL);
    SDL_RenderPresent(game.renderer->handle);
}

void game_init(int argc, char** argv)
{
    SDL_assert(SDL_Init(SDL_INIT_VIDEO) == 0);

    u8 fullscreen = 0;
    game.player2 = 0;
    
    for (int i = 0; i < argc; ++i)
    {
        if (strcmp("-f", argv[i]) == 0) fullscreen = 1;
        if (strcmp("-m", argv[i]) == 0) game.player2 = 1;
    }

    window_init("Pong | FPS: ", SCREEN_W, SCREEN_H, fullscreen);
    renderer_init();


    game_reset();

    game.state = STATE_TITLE;
    game.dt = 1.0 / 60.0; // Target FPS = 60
}

void game_run()
{
    u32 frames = 0;
    double last_frame = 0.0;
    double frame_timer = 0.0;

    while (!game.window->should_close)
    {
        const double now = (double)SDL_GetTicks();
        const double elapsed_ms = now - last_frame;
        last_frame = now;

        // Frame time in seconds
        const double dt = elapsed_ms / 1000.0;

        if (game.state == STATE_TITLE)
        {
            const u8* keys = SDL_GetKeyboardState(NULL);

            ball_update();
            paddle_ai(LEFT);
            paddle_ai(RIGHT);

            if (keys[SDL_SCANCODE_SPACE]) 
                game.state = STATE_PLAYING;
        }
        else if (game.state == STATE_PLAYING)
        {
            paddles_update();
            ball_update();

            if (check_scores())
                game.state = STATE_GAMEOVER;
        }
        else if (game.state == STATE_GAMEOVER)
        {
            const u8* keys = SDL_GetKeyboardState(NULL);

            if (keys[SDL_SCANCODE_R])
            {
                game_reset();
                game.state = STATE_PLAYING;
            }
        }

        game_render();
        window_poll_events();

        frame_timer += dt;
        frames++;
        if (frame_timer >= 1.0)
        {
            char title[32];
            snprintf(title, 31, "Pong | FPS: %u", frames);
            SDL_SetWindowTitle(game.window->handle, title);
            frame_timer -= 1.0;
            frames = 0;
        }
    }
}

void game_shutdown()
{
    renderer_destroy();
    window_destroy();
    SDL_Quit();
}

int main(int argc, char** argv)
{
    game_init(argc, argv);
    game_run();
    game_shutdown();
}
//==========================================================================================================================
