#define SDL_MAIN_USE_CALLBACKS 1
#include <math.h>
#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

typedef enum
{
    RED = 10,
    GREEN = 11,
    BLUE = 12,
    YELLOW = 13
} COLOR;

typedef struct
{
    glm::vec2 position;
    glm::vec2 originPosition;
    char color;
    void (*renderUpdate)();
} Game_Object;

typedef struct
{
    char color;
    float height;
} Ball;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static Uint64 m_LastFrameTime;
static float m_DeltaTime;
static Ball Game_Ball;
static int m_WindowWidth = 0;
static int m_WindowHeight = 0;

static void Game_InitBall()
{
    Game_Ball.height = 0;
    Game_Ball.color = COLOR::YELLOW;
}

// Init App
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("BallJump", "0.1.0", "com.bcmray.balljump");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Ball Jump", 640, 480, 0, &window, &renderer))
    {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Init Game
    Game_InitBall();
    SDL_GetWindowSize(window, &m_WindowWidth, &m_WindowHeight);

    SDL_Log("Game start");

    m_LastFrameTime = SDL_GetPerformanceCounter();

    return SDL_APP_CONTINUE;
}

static void Game_UpdateDeltaTime()
{
    Uint64 m_CurrentFrameTime = SDL_GetPerformanceCounter();
    m_DeltaTime = (m_CurrentFrameTime - m_LastFrameTime) / (float)SDL_GetPerformanceFrequency();
    m_LastFrameTime = m_CurrentFrameTime;
}

static void Game_LimitFPS()
{
    int MaxFPS = 60;
    if (m_DeltaTime < 1.0f / MaxFPS * 1000.0f)
        SDL_Delay(Uint32(floor(1.0 / MaxFPS * 1000.0 - m_DeltaTime)));
}

static glm::vec2 Game_GetRenderPosition(glm::vec2 position)
{
    return glm::vec2(floor(position.x + (m_WindowWidth / 2)), floor(position.y + (m_WindowHeight / 2)));
}

static void Game_RenderBall()
{
    int radius = 16;
    glm::vec2 renderPosition = Game_GetRenderPosition(glm::vec2(0, 0));
    // SDL_FRect rect = {renderPosition.x - 16, renderPosition.y - 16, 32, 32};
    // SDL_FRect rect = {100, 100, 32, 32};

    float x = renderPosition.x - radius;
    float endX = renderPosition.x + radius;
    float y = renderPosition.y - radius;
    float endY = renderPosition.y + radius;

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    while (x <= endX)
    {
        while (y <= endY)
        {
            glm::vec2 currentPointPosition = glm::vec2(x, y);
            float distance = glm::distance(renderPosition, currentPointPosition);
            if (distance <= radius)
            {
                SDL_RenderPoint(renderer, x, y);
            }

            y++;
        }
        y = (int)renderPosition.y - radius;
        x++;
    }

    // SDL_RenderRect(renderer, &rect);
}

static void Game_RenderBackground()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE_FLOAT);
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    Game_UpdateDeltaTime();
    Game_LimitFPS();

    SDL_RenderClear(renderer);

    Game_RenderBall();
    Game_RenderBackground();

    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    SDL_Log("Game quit");
}
