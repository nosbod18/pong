#include <stdio.h>
#include <SDL2/SDL.h>
#include "../inc/entity.h"
#include "../inc/graphics.h"
#include "../inc/types.h"
#include "../inc/util.h"

// Global game
Game game;


void game_reset()
{
    game.paddles[LEFT]  = (Entity){v2(50.0f,  200.0f), v2(10.0f, 50.0f), v2(0.0f, 0.0f)};
    game.paddles[RIGHT] = (Entity){v2(590.0f, 200.0f), v2(10.0f, 50.0f), v2(0.0f, 0.0f)};
    game.ball = (Entity){v2_mul_s(game.window->size, 0.5f), v2(10.0f, 10.0f), v2(RANDOM(0.0f, 1.0f) > 0.5f ? -100.0f : 100.0f, RANDOM(0.0f, 1.0f) > 0.5f ? 50.0f : -50.0f)};
    
    game.scores[LEFT] = game.scores[RIGHT] = 0;
}

void game_init()
{
    SDL_assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    game.window = window_create("Pong", 640, 480);
    game.renderer = renderer_create(game.window);
    
    game.dt = 0.0;
    game.last_frame = 0.0;
    game.should_reset = 0;

    reset();
}

void game_run()
{
    Window* window = game.window;
    Renderer* renderer = game.renderer;

    while (!window->should_close)
    {
        // Frame timing
        const double now = (double)SDL_GetTicks() / 1000.0;
        game.dt = now - game.last_frame;
        game.last_frame = now;

        if (!check_scores())
        {
            update_paddles();
            update_ball();
        }
        else
        {
            if (game.should_reset)
                reset();
        }

        renderer_clear(renderer, BLACK);
        renderer_draw_entity(renderer, &game.paddles[LEFT],  WHITE);
        renderer_draw_entity(renderer, &game.paddles[RIGHT], WHITE);
        renderer_draw_entity(renderer, &game.ball,           WHITE);
        renderer_flush(renderer);

        window_poll_events(window);
    }
}

void game_shutdown()
{
    renderer_destroy(game.renderer);
    window_destroy(game.window);
    SDL_Quit();
}

int main(int argc, char** argv)
{
    game_init();
    game_run();
    game_shutdown();
}