#ifndef _BJ_DEFINE_H
#define _BJ_DEFINE_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/glm.hpp>

#define BALL_RADIUS 16
#define BARRIER_RADIUS 8
#define MAX_NUM_BACKGROUND_POINT 64
#define Y_INTERVAL_BARRIER_BEND 32
#define MAX_NUM_BARRIER_BEND 4
#define Y_INTERVAL_BARRIER_CIRCLE 256
#define MAX_NUM_BARRIER_CIRCLE 4

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern TTF_Font *font;
extern Uint64 frame_LastFrameTime;
extern float frame_DeltaTime;
extern int m_WindowWidth;
extern int m_WindowHeight;
extern glm::vec2 Game_CameraOffset;

typedef enum
{
    RED = 10,
    GREEN = 11,
    BLUE = 12,
    YELLOW = 13,
    ANY = 14
} COLOR;

typedef enum
{
    MENU,
    WAIT,
    GAME,
    END
} GAME_STATE;

class Game_Object
{
public:
    Game_Object();
    Game_Object(glm::vec2 position, COLOR color);
    glm::vec2 position;
    COLOR color;
    float exData[4];
    void onFrame();
};

class Game_BarrierBendObject : public Game_Object
{
public:
    Game_BarrierBendObject(glm::vec2 position, COLOR color, float speed);
    float speed;
    float startX;
    void onFrame();
};

typedef struct
{
    COLOR color;
    float y;
    float yVelocity;
} Ball;

#endif