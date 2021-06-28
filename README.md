# Pong

Simple pong clone made in C using SDL2.

## Dependencies
- SDL2
- SDL Image

## Compiling
`gcc pong.c -o pong -lsdl2 -lsdl2_image`

## Using
Use `./pong [args]` to run, where `args` can be any combination of

- `-f` to run the game in fullscreen
- `-m` to run the game in multiplayer mode

(e.g: `./pong -f -m` to run in fullscreen and multiplayer mode)

### Controls
In singleplayer mode, use the up and down arrow keys to move the paddle on the right. In multiplayer mode, player 2 can use the W and S keys to move the left paddle (player 1 controls are the same).

Get to 10 points to win. Have fun!