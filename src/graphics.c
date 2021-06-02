#include "../inc/graphics.h"

#include <SDL2/SDL.h>
#include "../inc/types.h"
#include "../inc/util.h"

extern Game game;

Window* window_create(const char* title, int w, int h)
{
    Window* self = malloc(sizeof(Window));
    SDL_assert(self);

    self->handle = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_SHOWN);
    SDL_assert(self->handle);

    self->size = v2((float)w, (float)h);
    self->should_close = 0;

    return self;
}

void window_destroy(Window* self)
{
    SDL_DestroyWindow(self->handle);

    free(self);
    self = NULL;
}

void window_poll_events(Window* self)
{
    SDL_PumpEvents();

    while (SDL_PollEvent(&self->event))
    {
        switch (self->event.type)
        {
            case SDL_QUIT:
                self->should_close = 1;
                break;

            case SDL_KEYDOWN:
                switch (self->event.key.keysym.sym)
                {
                    case SDLK_r:
                        game.should_reset = 1;
                        break;
                
                    default: break;
                }
        
            default: break;
        }
    }
}

Renderer* renderer_create(Window* window)
{
    Renderer* self = malloc(sizeof(Renderer));
    SDL_assert(self);

    self->handle = SDL_CreateRenderer(window->handle, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_assert(self->handle);

    self->screen = SDL_CreateTexture(self->handle, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, (int)window->size.x, (int)window->size.y);
    SDL_assert(self->screen);

    self->raw_size = window->size.x * window->size.y * sizeof(u32);
    self->raw_pitch = window->size.x * sizeof(u32);
    self->raw = malloc(self->raw_size);
    SDL_assert(self->raw);

    return self;
}

void renderer_destroy(Renderer* self)
{
    SDL_DestroyTexture(self->screen);
    SDL_DestroyRenderer(self->handle);

    free(self->raw);
    self->raw = NULL;

    free(self);
    self = NULL;
}

void renderer_clear(Renderer* self, u32 color)
{
    memset(self->raw, color, self->raw_size);
}

void renderer_draw_point(Renderer* self, int x, int y, u32 color)
{
    if (x < 0 || y < 0 || x > game.window->size.x || y > game.window->size.y)
        return;

    self->raw[(int)(y * game.window->size.x + x)] = color;
}

// Since the entities are just rectangles (or squares), this function is very simple
void renderer_draw_entity(Renderer* self, Entity* e, u32 color)
{
    V2 min = e->pos;
    V2 max = v2_add(e->pos, e->size);

    for (int x = (int)min.x; x < max.x; ++x)
        for (int y = (int)min.y; y < max.y; ++y)
            renderer_draw_point(self, x, y, color);
}

void renderer_flush(Renderer* self)
{
    SDL_UpdateTexture(self->screen, NULL, self->raw, self->raw_pitch);
    SDL_RenderCopy(self->handle, self->screen, NULL, NULL);
    SDL_RenderPresent(self->handle);
}