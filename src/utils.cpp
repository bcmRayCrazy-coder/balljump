#include <string>
#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "define.h"

void Game_VoidFunction()
{
}

glm::vec2 Game_GetRenderPosition(glm::vec2 position)
{
    return glm::vec2(position.x - Game_CameraOffset.x + (m_WindowWidth / 2), (m_WindowHeight / 2) - position.y + Game_CameraOffset.y);
}

glm::vec3 Game_GetColorV3(COLOR color)
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

COLOR Game_RandomColor(bool hasAny)
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

void Game_SetText(TTF_Text *text, std::string content)
{
    TTF_DeleteTextString(text, 0, -1);
    TTF_AppendTextString(text, content.c_str(), content.length());
}

void Game_DrawCross(SDL_Renderer *renderer, glm::vec2 center, int length, glm::vec3 color, float alpha)
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

void Game_DrawCircle(SDL_Renderer *renderer, glm::vec2 center, int radius, glm::vec3 color, float alpha)
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

Game_Object Game_CreateRandomBackgroundPoint(bool withOffset)
{
    float y = (float)SDL_rand(m_WindowHeight);
    if (withOffset)
    {
        y += (m_WindowHeight / 2) + Game_CameraOffset.y;
    }
    /**
     * Point ExData
     * { Parallex, brightness }
     */
    Game_Object object(glm::vec2((float)SDL_rand(m_WindowWidth) - m_WindowWidth / 2, y), Game_RandomColor(true));
    object.exData[0] = SDL_rand(100) / 200.0f;
    object.exData[1] = SDL_rand(255) / 255.0f;
    return object;
}

Game_BarrierBendObject *Game_CreateBarrierBendLine(float y, float speed)
{
    static Game_BarrierBendObject objectList[32];

    const COLOR colorList[4] = {COLOR::RED, COLOR::GREEN, COLOR::BLUE, COLOR::YELLOW};
    int colorIndex = 0;
    int colorCount = 1;
    int index = 0;

    float x = -m_WindowWidth / 2;
    while (x <= m_WindowWidth / 2)
    {
        objectList[index] = Game_BarrierBendObject(glm::vec2(x, y), colorList[colorIndex], speed);

        if (++colorCount > 4)
        {
            colorCount = 1;
            if (++colorIndex >= 4)
            {
                colorIndex = 0;
            }
        }

        x += BARRIER_RADIUS * 4;

        if (++index > 32)
        {
            return objectList;
        }
    }

    return objectList;
}