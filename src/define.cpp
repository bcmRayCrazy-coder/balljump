#include "utils.h"
#include "define.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
TTF_Font *font = NULL;
Uint64 frame_LastFrameTime;
float frame_DeltaTime;
int m_WindowWidth = 0;
int m_WindowHeight = 0;
glm::vec2 Game_CameraOffset = glm::vec2(0, 100);
Ball Game_Ball;

Game_Object::Game_Object()
{
    position = glm::vec2(0.0f, 0.0f);
    color = COLOR::ANY;
}

Game_Object::Game_Object(glm::vec2 _position, COLOR _color)
{
    position = _position;
    color = _color;
}

void Game_Object::onFrame()
{
}

Game_BarrierBendObject::Game_BarrierBendObject()
{
    speed = 0;
    startX = 0;
}

Game_BarrierBendObject::Game_BarrierBendObject(glm::vec2 _position, COLOR _color, float _speed) : Game_Object(_position, _color)
{
    speed = _speed;
    startX = _position.x;
}

void Game_BarrierBendObject::onFrame()
{
    float x = speed * frame_DeltaTime + position.x;
    while (x > m_WindowWidth / 2)
    {
        x -= m_WindowWidth;
    }
    position.x = x;

    glm::vec2 renderPosition = Game_GetRenderPosition(position);
    Game_DrawCircle(renderer, renderPosition, BARRIER_RADIUS, Game_GetColorV3(color), 255);

    // if(glm::distance(glm::vec2(0,Game_Ball.y), position) < (BALL_RADIUS))
}