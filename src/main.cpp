#define SDL_MAIN_USE_CALLBACKS 1
#define BALL_RADIUS 16
#define BARRIER_RADIUS 8
#define MAX_NUM_BACKGROUND_POINT 64
#define Y_INTERVAL_BARRIER_BEND 32
#define MAX_NUM_BARRIER_BEND 4
#define Y_INTERVAL_BARRIER_CIRCLE 256
#define MAX_NUM_BARRIER_CIRCLE 4

#include <math.h>
#include <fmt/core.h>
#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

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

typedef struct
{
    glm::vec2 position;
    COLOR color;
    float exData[4];
    void (*onFrame)();
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
static Uint64 frame_LastFrameTime;
static float frame_DeltaTime;
static int m_WindowWidth = 0;
static int m_WindowHeight = 0;

// State
static GAME_STATE Game_State = GAME_STATE::WAIT;
static float Game_StateHintUpArrowValue = 100.0f;

// HUD
static TTF_TextEngine *Game_TextEngine = NULL;
static TTF_Text *Game_TextHeight = NULL;
static TTF_Text *Game_TextFPS = NULL;
static SDL_Texture *Game_HintUpArrowTexture = NULL;

// Gaming Object
static glm::vec2 Game_CameraOffset = glm::vec2(0, 100);
static Ball Game_Ball;

static Game_Object Game_BackgroundPoints[MAX_NUM_BACKGROUND_POINT] = {};
static Game_Object *Game_BarrierColorBend[MAX_NUM_BARRIER_BEND][32] = {};
static Game_Object Game_BarrierColorCircle[MAX_NUM_BARRIER_CIRCLE][16] = {};

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

static glm::vec2 Game_GetRenderPosition(glm::vec2 position)
{
    return glm::vec2(position.x - Game_CameraOffset.x + (m_WindowWidth / 2), (m_WindowHeight / 2) - position.y + Game_CameraOffset.y);
}

static void Game_SetText(TTF_Text *text, std::string content)
{
    TTF_DeleteTextString(text, 0, -1);
    TTF_AppendTextString(text, content.c_str(), content.length());
}

static void Game_DrawCross(glm::vec2 center, int length, glm::vec3 color, float alpha)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);

    float x = center.x - length;
    float endX = center.x + length;
    while (x <= endX)
    {
        SDL_RenderPoint(renderer, x, center.y);
        x++;
    }

    float y = center.y - length;
    float endY = center.y + length;
    while (y <= endY)
    {
        SDL_RenderPoint(renderer, center.x, y);
        y++;
    }
}

static void Game_DrawCircle(glm::vec2 center, int radius, glm::vec3 color, float alpha)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);

    float x = center.x - radius;
    float y = center.y - radius;
    float endX = center.x + radius;
    float endY = center.y + radius;
    while (x <= endX)
    {
        while (y <= endY)
        {
            glm::vec2 pointPosition = glm::vec2(x, y);
            float distance = glm::distance(center, pointPosition);
            if (distance <= radius)
            {
                SDL_RenderPoint(renderer, x, y);
            }

            y++;
        }
        y = center.y - radius;
        x++;
    }
}

/**
 * Create
 */
static Game_Object Game_CreateRandomBackgroundPoint(bool withOffset = true)
{
    float y = (float)SDL_rand(m_WindowHeight);
    if (withOffset)
    {
        y += (m_WindowHeight / 2) + Game_CameraOffset.y;
    }
    /**
     * Point ExData (Alpha Control)
     * { Parallex, brightness }
     */
    return {
        glm::vec2((float)SDL_rand(m_WindowWidth) - m_WindowWidth / 2, y),
        Game_RandomColor(true),
        {SDL_rand(100) / 200.0f, SDL_rand(255) / 255.0f},
        Game_VoidFunction};
}

static Game_Object *Game_CreateBarrierBend(float y, float speed)
{
    /**
     * TODO: Create Bend
     * Radius: BARRIER_RADIUS
     * Interval: BALL_RADIUS / 2
     * exData: { Speed }
     */
    float x = -m_WindowWidth / 2;
    while (x < m_WindowWidth / 2)
    {
        x += BALL_RADIUS / 2;
    }
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
    for (int i = 0; i < MAX_NUM_BACKGROUND_POINT; i++)
    {
        Game_BackgroundPoints[i] = Game_CreateRandomBackgroundPoint(false);
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

static void Game_InitTexture()
{
    Game_HintUpArrowTexture = IMG_LoadTexture(renderer, "assets/images/up_arrow.png");
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
    SDL_srand((unsigned) time(NULL));
    if (!Game_InitText())
    {
        SDL_Log("Could not load font:\n%s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    Game_InitBall();
    Game_InitBackgroundPoints();
    Game_InitTexture();

    SDL_Log("Game start");

    std::string _empty_text = std::string("");
#ifdef TARGET_MOBILE
    std::string _tip = std::string("Touch Screen to start");
#else
    std::string _tip = std::string("Press Mouse to start");
#endif
    Game_TextHeight = TTF_CreateText(Game_TextEngine, font, _tip.c_str(), _tip.length());
    Game_TextFPS = TTF_CreateText(Game_TextEngine, font, _empty_text.c_str(), 0);

    frame_LastFrameTime = SDL_GetPerformanceCounter();

    return SDL_APP_CONTINUE;
}

/**
 * Update
 */
static void Game_UpdateDeltaTime()
{
    Uint64 m_CurrentFrameTime = SDL_GetPerformanceCounter();
    frame_DeltaTime = (m_CurrentFrameTime - frame_LastFrameTime) / (float)SDL_GetPerformanceFrequency();
    frame_LastFrameTime = m_CurrentFrameTime;
}

static void Game_UpdateTextHeight()
{
    std::string str = fmt::format("Height: {:.3f}m", Game_Ball.y / 10);
    Game_SetText(Game_TextHeight, str);
}

static void Game_LimitFPS()
{
#ifdef LIMIT_FPS
    if (frame_DeltaTime < 1.0f / LIMIT_FPS * 1000.0f)
        SDL_Delay(Uint32(floor(1.0 / LIMIT_FPS * 1000.0 - frame_DeltaTime)));
#endif
}

/**
 * Frame
 */

static void Game_FrameCamera()
{
    Game_CameraOffset.y = Game_Ball.y + 100;
}

static void Game_FrameBall()
{
    Game_Ball.y += Game_Ball.yVelocity * frame_DeltaTime;
    if (Game_Ball.yVelocity > 0)
    {
        Game_Ball.yVelocity -= 300 * frame_DeltaTime;
        if (Game_Ball.yVelocity < 0)
        {
            Game_Ball.yVelocity = 0;
        }
        Game_UpdateTextHeight();
    }

    glm::vec2 renderPosition = Game_GetRenderPosition(glm::vec2(0, Game_Ball.y));
    glm::vec3 ballColor = Game_GetColorV3(Game_Ball.color);
    Game_DrawCircle(renderPosition, BALL_RADIUS, ballColor, 255);
}

static void Game_FrameBackground()
{
    for (int i = 0; i < MAX_NUM_BACKGROUND_POINT; i++)
    {
        const Game_Object point = Game_BackgroundPoints[i];
        const float brightness = point.exData[1];
        const float parallexY = (Game_CameraOffset.y - point.position.y) * point.exData[0];
        const glm::vec3 pointColor = Game_GetColorV3(point.color) * glm::vec3(brightness, brightness, brightness);
        const glm::vec2 pointPosition = Game_GetRenderPosition(point.position) + glm::vec2(0, parallexY);

        Game_DrawCross(pointPosition, 2, pointColor, 255);

        // Respawm outbound point
        if (pointPosition.y > m_WindowHeight)
        {
            Game_BackgroundPoints[i] = Game_CreateRandomBackgroundPoint();
        }
    }
}

static void Game_FrameTextHeight()
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    TTF_DrawRendererText(Game_TextHeight, 16, 28);
}

static void Game_FrameTextFPS()
{
    std::string str = fmt::format("FPS: {:.0f}", 1 / frame_DeltaTime);
    Game_SetText(Game_TextFPS, str);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    TTF_DrawRendererText(Game_TextFPS, 16, 8);
}

static void Game_FrameHintUpArrow()
{
    if (Game_StateHintUpArrowValue > 0 && Game_HintUpArrowTexture != NULL)
    {
        SDL_FRect dst;

        SDL_GetTextureSize(Game_HintUpArrowTexture, &dst.w, &dst.h);
        dst.x = (m_WindowWidth - dst.w) / 2;
        dst.y = (m_WindowHeight - dst.h) / 2;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_SetTextureColorModFloat(Game_HintUpArrowTexture, Game_StateHintUpArrowValue / 100.0f, Game_StateHintUpArrowValue / 100.0f, Game_StateHintUpArrowValue / 100.0f);
        SDL_RenderTexture(renderer, Game_HintUpArrowTexture, NULL, &dst);

        if (Game_State == GAME_STATE::GAME)
        {
            Game_StateHintUpArrowValue -= 400 * frame_DeltaTime;
        }
        if (Game_StateHintUpArrowValue <= 0)
        {
            SDL_DestroyTexture(Game_HintUpArrowTexture);
        }
    }
}

static void Game_FrameHUD()
{
    Game_FrameTextHeight();
    Game_FrameTextFPS();
    Game_FrameHintUpArrow();
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
        if (Game_State != GAME_STATE::GAME)
        {
            Game_State = GAME_STATE::GAME;
        }
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    Game_UpdateDeltaTime();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(renderer);

    Game_FrameCamera();
    Game_FrameBackground();
    Game_FrameBall();
    Game_FrameHUD();

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
