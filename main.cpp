#define SDL_MAIN_USE_CALLBACKS 1
#define MAXFPS 60
#include <math.h>
#include <fmt/core.h>
#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef enum
{
    RED = 10,
    GREEN = 11,
    BLUE = 12,
    YELLOW = 13,
    ANY = 14
} COLOR;

typedef struct
{
    glm::vec2 position;
    glm::vec2 originPosition;
    COLOR color;
    float exData[4];
    void (*renderUpdate)();
} Game_Object;

typedef struct
{
    COLOR color;
    float y;
    float yVelocity;
} Ball;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static TTF_Font *font = NULL;
static Uint64 m_LastFrameTime;
static float m_DeltaTime;
static int m_WindowWidth = 0;
static int m_WindowHeight = 0;
static glm::vec2 Game_CameraOffset = glm::vec2(0, 100);
static Ball Game_Ball;
static TTF_TextEngine *Game_TextEngine = NULL;
static TTF_Text *Game_TextHeight = NULL;

static Game_Object Game_BackgroundPoints[128] = {};

/**
 * Utils
 */

static void Game_VoidFunction()
{
}

static glm::vec3 Game_GetColorV3(COLOR color)
{
    switch (color)
    {
    case COLOR::RED:
        return glm::vec3(255, 0, 0);
        break;

    case COLOR::GREEN:
        return glm::vec3(0, 255, 0);
        break;

    case COLOR::BLUE:
        return glm::vec3(0, 0, 255);
        break;

    case COLOR::YELLOW:
        return glm::vec3(255, 255, 0);
        break;

    default:
        return glm::vec3(255, 255, 255);
    }
}

static COLOR Game_RandomColor(bool hasAny)
{
    Sint32 n = SDL_rand(hasAny ? 4 : 3);
    switch (n)
    {
    case 0:
        return COLOR::RED;
        break;
    case 1:
        return COLOR::GREEN;
        break;
    case 2:
        return COLOR::BLUE;
        break;
    case 3:
        return COLOR::YELLOW;
        break;
    case 4:
        return COLOR::ANY;
        break;

    default:
        return COLOR::RED;
        break;
    }
}

static Game_Object Game_RandomBackgroundPoint()
{
    /**
     * Point ExData (Alpha Control)
     * { Delta, Speed }
     */
    return {glm::vec2((float)SDL_rand(m_WindowWidth) - m_WindowHeight / 2, (float)SDL_rand(m_WindowHeight) + (m_WindowHeight / 2) + Game_CameraOffset.y), glm::vec2(0, 0), Game_RandomColor(true), {0, (SDL_rand(9) + 1.0f) / 20}, Game_VoidFunction};
}

/**
 * Init
 */

static void Game_InitBall()
{
    Game_Ball.y = 0;
    Game_Ball.yVelocity = 0;
    Game_Ball.color = COLOR::YELLOW;
}

static void Game_InitBackgroundPoints()
{
    for (int i = 0; i < 128; i++)
    {
        Game_BackgroundPoints[i] = Game_RandomBackgroundPoint();
    }
}

static bool Game_InitText()
{
    font = TTF_OpenFont("assets/fonts/VonwaonBitmap.ttf", 16);

    if (font == NULL)
    {
        return false;
    }
    SDL_Log("Font loaded");

    Game_TextEngine = TTF_CreateRendererTextEngine(renderer);
    if (Game_TextEngine == NULL)
    {
        return false;
    }

    return true;
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

    if (!TTF_Init())
    {
        SDL_Log("Couldn't initialise SDL_ttf: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Ball Jump", 640, 480, 0, &window, &renderer))
    {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Init Game
    SDL_GetWindowSize(window, &m_WindowWidth, &m_WindowHeight);
    Game_InitBall();
    Game_InitBackgroundPoints();
    if (!Game_InitText())
    {
        SDL_Log("Could not load font:\n%s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Log("Game start");

    std::string _tip = std::string("Press Mouse to start");
    Game_TextHeight = TTF_CreateText(Game_TextEngine, font, _tip.c_str(), _tip.length());

    m_LastFrameTime = SDL_GetPerformanceCounter();

    return SDL_APP_CONTINUE;
}

/**
 * Render
 */

static void Game_UpdateDeltaTime()
{
    Uint64 m_CurrentFrameTime = SDL_GetPerformanceCounter();
    m_DeltaTime = (m_CurrentFrameTime - m_LastFrameTime) / (float)SDL_GetPerformanceFrequency();
    m_LastFrameTime = m_CurrentFrameTime;
}

static void Game_LimitFPS()
{
    if (m_DeltaTime < 1.0f / MAXFPS * 1000.0f)
        SDL_Delay(Uint32(floor(1.0 / MAXFPS * 1000.0 - m_DeltaTime)));
}

static void Game_UpdateTextHeight()
{
    std::string str = fmt::format("Height: {}m", Game_Ball.y / 10);
    Game_TextHeight = TTF_CreateText(Game_TextEngine, font, str.c_str(), str.length());
}

static void Game_UpdateBall()
{
    Game_Ball.y += Game_Ball.yVelocity * m_DeltaTime;
    if (Game_Ball.yVelocity > 0)
    {
        Game_Ball.yVelocity -= 300 * m_DeltaTime;
        if (Game_Ball.yVelocity < 0)
        {
            Game_Ball.yVelocity = 0;
        }
        Game_UpdateTextHeight();
    }
}

static void Game_UpdateCamera()
{
    Game_CameraOffset.y = Game_Ball.y + 100;
}

/**
 * Render
 */

static glm::vec2 Game_GetRenderPosition(glm::vec2 position)
{
    return glm::vec2(position.x - Game_CameraOffset.x + (m_WindowWidth / 2), (m_WindowHeight / 2) - position.y + Game_CameraOffset.y);
}

static void Game_RenderBall()
{
    int radius = 16;
    glm::vec2 renderPosition = Game_GetRenderPosition(glm::vec2(0, Game_Ball.y));
    float x = renderPosition.x - radius;
    float endX = renderPosition.x + radius;
    float y = renderPosition.y - radius;
    float endY = renderPosition.y + radius;

    glm::vec3 ballColor = Game_GetColorV3(Game_Ball.color);

    SDL_SetRenderDrawColor(renderer, ballColor.r, ballColor.g, ballColor.b, 255);

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
}

static void Game_RenderBackground()
{
    for (int i = 0; i < 128; i++)
    {
        const Game_Object point = Game_BackgroundPoints[i];

        const glm::vec3 pointColor = Game_GetColorV3(point.color);
        const glm::vec2 pointPosition = Game_GetRenderPosition(point.position);
        const float brightness = SDL_sinf(point.exData[0] * point.exData[1]);

        SDL_SetRenderDrawColor(renderer, pointColor.r * brightness, pointColor.g * brightness, pointColor.b * brightness, 0);
        SDL_RenderPoint(renderer, pointPosition.x, pointPosition.y);

        // Update exData Delta
        Game_BackgroundPoints[i].exData[0] += m_DeltaTime;

        if (Game_BackgroundPoints[i].exData[0] > 31.4)
        {
            Game_BackgroundPoints[i].exData[0] = 0;
        }

        // Respawm outbound point
        if (pointPosition.y > m_WindowHeight)
        {
            Game_BackgroundPoints[i] = Game_RandomBackgroundPoint();
        }
    }
}

static void Game_RenderTextHeight()
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    TTF_DrawRendererText(Game_TextHeight, 16, 8);
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;
    }
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        Game_Ball.yVelocity = 200;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    Game_UpdateDeltaTime();
    Game_UpdateBall();
    Game_UpdateCamera();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(renderer);

    Game_RenderBackground();
    Game_RenderBall();
    Game_RenderTextHeight();

    SDL_RenderPresent(renderer);
    Game_LimitFPS();

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    TTF_DestroyText(Game_TextHeight);
    TTF_DestroyRendererTextEngine(Game_TextEngine);
    TTF_CloseFont(font);
    TTF_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    SDL_Log("Game quit");
}
