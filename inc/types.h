#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>

// Misc
//================================================================================================
typedef unsigned char u8;
typedef unsigned int  u32;

typedef enum Direction
{
    LEFT  = 0,
    RIGHT = 1,
} Direction;

typedef struct V2
{
    float x;
    float y;
} V2;
//================================================================================================

// Graphics
//================================================================================================

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
    SDL_Texture* screen;
    u32* raw;
    u32  raw_size;
    u32  raw_pitch;
} Renderer;

//================================================================================================

// Game
//================================================================================================
typedef struct Entity
{
    V2 pos;
    V2 size;
    V2 vel;
} Entity;

typedef struct Game
{
    Window* window;
    Renderer* renderer;

    Entity ball;
    Entity paddles[2];
    
    int scores[2];

    double dt;
    double last_frame;

    u8 should_reset;
    u8 won;
} Game;
//================================================================================================

#endif /* TYPES_H */
