#ifndef _BJ_UTILS_H
#define _BJ_UTILS_H

#include <string>
#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "define.h"

void Game_VoidFunction();
glm::vec2 Game_GetRenderPosition(glm::vec2 worldPosition);
glm::vec3 Game_GetColorV3(COLOR color);
COLOR Game_RandomColor(bool withAny);
void Game_SetText(TTF_Text *text, std::string content);
void Game_DrawCross(SDL_Renderer *renderer, glm::vec2 center, int length, glm::vec3 color, float alpha);
void Game_DrawCircle(SDL_Renderer *renderer, glm::vec2 center, int radius, glm::vec3 color, float alpha);

#endif