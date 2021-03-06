/*-----------------------------------------------------------------------------
Copyright 2007 Milan Babuskov
Copyright 2020 David A. Redick

This file is part of Vodovod

Vodovod is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Vodovod is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Vodovod in file COPYING; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
-----------------------------------------------------------------------------*/
#include <string>
#include <vector>
#include "config.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include "njamfont.h"
#include "sutils.h"
#include "menu.h"

extern SDL_Surface *Screen;
//-----------------------------------------------------------------------------
Menu::Menu(NjamFont *font_ptr, int x, int y)
{
    x_pos = x;
    y_pos = y;
    font = font_ptr;
    indicator = 0;
    firstIsTitleM = false;
}
//-----------------------------------------------------------------------------
Menu::~Menu()
{
    if (background)
        SDL_FreeSurface(background);    // release background surface
}
//-----------------------------------------------------------------------------
bool Menu::process_keys()                   // up/down/enter/esc: return false on menu exit
{
    SDLKey key = NjamGetch(false);
    if (key == SDLK_UP   || key > 499 && key == getKey(taUp))
        indicator--;
    if (key == SDLK_DOWN || key > 499 && key == getKey(taDown))
        indicator++;
    if (firstIsTitleM && indicator < 1 || !firstIsTitleM && indicator < 0)
        indicator = options.size()-1;
    if ((unsigned)indicator >= options.size())
        indicator = (firstIsTitleM ? 1 : 0);
    if (key == SDLK_ESCAPE)
        return false;
    if ((key == SDLK_RETURN || key == SDLK_KP_ENTER || key > 499 && key == getKey(taDrop)))
    {
        if (!onEnter())
            return false;
    }
    return true;
}
//-----------------------------------------------------------------------------
void Menu::draw(bool flipit)
{
    if (background)
        SDL_BlitSurface(background, NULL, Screen, NULL);

    render();           // draw some animations if needed, scroll, etc.

    int i=0;            // blit menu items
    for (std::vector<std::string>::iterator it = options.begin(); it != options.end(); ++it)
    {
        font->WriteTextXY(Screen, x_pos, y_pos + i*font->GetCharHeight(), (*it).c_str());
        i++;
    }

    SDL_Rect r;         // blit rectangle around selected item
    int border = 2;
    int length = 0;
    if (options.size() > (unsigned)indicator)
        length = options[indicator].length();
    r.x = x_pos-border;
    r.y = y_pos+indicator*font->GetCharHeight()-2;
    r.w = font->GetCharWidth() * length + 2*border;
    r.h = font->GetCharHeight() + 2*border;
    Box(Screen, r, 1, 255, 190, 50);

    if (firstIsTitleM)  // blit line below title (=first option)
    {
        border = 2;
        length = options[0].length();
        r.x = x_pos;
        r.y = font->GetCharHeight() + y_pos - 1;
        r.w = font->GetCharWidth() * length + 2*border;
        r.h = 1;
        Box(Screen, r, 1, 255, 190, 50);
    }

    if (flipit)
        SDL_Flip(Screen);
}
//-----------------------------------------------------------------------------
void Menu::getDimensions(SDL_Rect& rect, int border, int minWidth)
{
    unsigned int char_width = minWidth;
    for (std::vector<std::string>::iterator it = options.begin(); it != options.end(); ++it)
        if ((*it).length() > char_width)
            char_width = (*it).length();

    unsigned int char_height = options.size();
    rect.w = (char_width+1) * font->GetCharWidth() + 2*border;
    rect.h = char_height * font->GetCharHeight() + 2*border;
    rect.x = x_pos-border;
    rect.y = y_pos-border;
}
//-----------------------------------------------------------------------------
void Menu::start(int minWidth)
{
    background = SDL_DisplayFormat(Screen);     // save screen to buffer, so we can redraw
    if (!background)
        throw Exiter("Cannot make a copy of background");

    SDL_Rect r;
    getDimensions(r, 6, minWidth);
    SurfaceEffect(background, r, fxDarkenAlot);
    Box(background, r, 1, 255, 255, 0);

    while (true)
    {
        Uint32 startTime = SDL_GetTicks();
        draw();             // also waits for vsync
        if (!process_keys())
            return;
        CheckMusic(0);
        while (startTime + 34 > SDL_GetTicks())
            SDL_Delay(1);    // target about 30 fps
    }
};
//-----------------------------------------------------------------------------
SDLKey Menu::Message(std::string text)
{
    const int screenw = 800;
    while (NjamGetch(false) != SDLK_LAST);  // clear the buffer

    int l = text.length();
    int w = font->GetCharWidth();

    draw(false);    // render background

    SDL_Rect dest;
    SDL_Rect_set(dest, (screenw-l*w-50)/2, 195, l*w+50, 55);
    SDL_FillRect(Screen, &dest, 0);
    Uint32 FillColor = SDL_MapRGB(Screen->format, 220, 50, 0);
    SDL_Rect_set(dest, (screenw-l*w-50)/2 + 1, 196, l*w+48, 53);
    SDL_FillRect(Screen, &dest, FillColor);

    SDL_Rect_set(dest, (screenw-l*w-30)/2, 205, l*w+30, 35);
    SDL_FillRect(Screen, &dest, 0);

    font->WriteTextXY(Screen, (screenw-l*w)/2, 215, text.c_str());
    SDL_Flip(Screen);

    return NjamGetch(true);
}
//-----------------------------------------------------------------------------
