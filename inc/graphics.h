#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL2/SDL.h>
#include "util.h"

// Declarations
//================================================================================================
typedef struct Window Window;
typedef struct Renderer Renderer;
//================================================================================================

// Prototypes
//================================================================================================
Window*     window_create(const char* title, int w, int h);
void        window_destroy(Window* self);
void        window_poll_events(Window* self);

Renderer*   renderer_create(Window* window);
void        renderer_destroy(Renderer* self);
void        renderer_clear(Renderer* self, u32 color);
void        renderer_draw_point(Renderer* self, int x, int y, u32 color);
void        renderer_draw_entity(Renderer* self, Entity* e, u32 color);
void        renderer_flush(Renderer* self);
//================================================================================================

#endif /* GRAPHICS_H */
