#include "../inc/entity.h"
#include "../inc/types.h"
#include "../inc/util.h"

extern Game game;

void update_paddles()
{
    Entity* lp = &game.paddles[LEFT];
    Entity* rp = &game.paddles[RIGHT];
    const u8* keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_UP])   rp->pos.y -= 4.0f;
    if (keys[SDL_SCANCODE_DOWN]) rp->pos.y += 4.0f;
    
    if (keys[SDL_SCANCODE_W]) lp->pos.y -= 4.0f;
    if (keys[SDL_SCANCODE_S]) lp->pos.y += 4.0f;

    if (rp->pos.y < 0.0f) rp->pos.y = 0.0f;
    if (lp->pos.y < 0.0f) lp->pos.y = 0.0f;
    if (rp->pos.y + rp->size.y > game.window->size.y) rp->pos.y = game.window->size.y - rp->size.y;
    if (lp->pos.y + lp->size.y > game.window->size.y) lp->pos.y = game.window->size.y - lp->size.y;
}

void update_ball()
{
    Entity* b  = &game.ball;
    Entity* lp = &game.paddles[LEFT];
    Entity* rp = &game.paddles[RIGHT];

    if (entity_collision(b, lp))
    {
        // Distance between the centers of the ball and paddle
        float diff = (lp->pos.y + lp->size.y * 0.5f) - (b->pos.y + b->size.y * 0.5f);

        b->vel.x = -b->vel.x;
        b->vel.y = -b->vel.y;
    }
    else if (entity_collision(b, rp))
    {
        // Distance between the centers of the ball and paddle
        float diff = (rp->pos.y + rp->size.y * 0.5f) - (b->pos.y + b->size.y * 0.5f);

        b->vel.x = -b->vel.x;
        b->vel.y = -b->vel.y;
    }
    else if (b->pos.y <= 0.0f || b->pos.y + b->size.y >= game.window->size.y)
    {
        b->vel.y = -b->vel.y;
    }

    b->pos = v2_add(b->pos, v2_mul_s(b->vel, game.dt));
}

int check_scores()
{
    Entity* b = &game.ball;

    if (v2_add(b->pos, b->size).x > game.window->size.x)
    {
        // printf("%d\n", ++game.scores[LEFT]);
        game.scores[LEFT] += 10;
        game.ball.pos = v2_sub(v2_mul_s(game.window->size, 0.5f), v2_mul_s(game.ball.size, 0.5f));
        game.ball.vel = v2(-100.0f, RANDOM(0.0f, 1.0f) > 0.5f ? 50.0f : -50.0f);
        if (game.scores[LEFT] == 10)
        {
            printf("Congradulations player 1, you won!\n");
            game.won = 1;
        }
    }
    else if (b->pos.x < 0.0f)
    {
        // printf("%d\n", ++game.scores[RIGHT]);
        game.scores[RIGHT] += 10;
        game.ball.pos = v2_sub(v2_mul_s(game.window->size, 0.5f), v2_mul_s(game.ball.size, 0.5f));
        game.ball.vel = v2(100.0f, RANDOM(0.0f, 1.0f) > 0.5f ? 50.0f : -50.0f);
        if (game.scores[RIGHT] == 10)
        {
            printf("Congradulations player 2, you won!\n");
            game.won = 1;
        }
    }

    return game.won;
}

int entity_collision(Entity* e1, Entity* e2)
{
    V2 e1min = e1->pos, e1max = v2_add(e1->pos, e1->size);
    V2 e2min = e2->pos, e2max = v2_add(e2->pos, e2->size);

    if(e1max.x < e2min.x || e1min.x > e2max.x) return 0;
    if(e1max.y < e2min.y || e1min.y > e2max.y) return 0;

    return 1;
}