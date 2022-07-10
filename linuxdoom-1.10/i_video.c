// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//      DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

#include <SDL2/SDL.h>

#include <stdlib.h>
#include <unistd.h>

#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *framebuffer;
SDL_Color color_palette[256];

int X_width;
int X_height;

// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
static int multiply = 1;


//
//  Translates the key currently in X_event
//

int
xlatekey (SDL_Keycode sym)
{
  int rc;

  switch (sym)
    {
    case SDLK_LEFT:
      rc = KEY_LEFTARROW;
      break;
    case SDLK_RIGHT:
      rc = KEY_RIGHTARROW;
      break;
    case SDLK_DOWN:
      rc = KEY_DOWNARROW;
      break;
    case SDLK_UP:
      rc = KEY_UPARROW;
      break;
    case SDLK_ESCAPE:
      rc = KEY_ESCAPE;
      break;
    case SDLK_RETURN:
      rc = KEY_ENTER;
      break;
    case SDLK_TAB:
      rc = KEY_TAB;
      break;
    case SDLK_F1:
      rc = KEY_F1;
      break;
    case SDLK_F2:
      rc = KEY_F2;
      break;
    case SDLK_F3:
      rc = KEY_F3;
      break;
    case SDLK_F4:
      rc = KEY_F4;
      break;
    case SDLK_F5:
      rc = KEY_F5;
      break;
    case SDLK_F6:
      rc = KEY_F6;
      break;
    case SDLK_F7:
      rc = KEY_F7;
      break;
    case SDLK_F8:
      rc = KEY_F8;
      break;
    case SDLK_F9:
      rc = KEY_F9;
      break;
    case SDLK_F10:
      rc = KEY_F10;
      break;
    case SDLK_F11:
      rc = KEY_F11;
      break;
    case SDLK_F12:
      rc = KEY_F12;
      break;

    case SDLK_BACKSPACE:
    case SDLK_DELETE:
      rc = KEY_BACKSPACE;
      break;

    case SDLK_PAUSE:
      rc = KEY_PAUSE;
      break;

    case SDLK_KP_EQUALS:
    case SDLK_EQUALS:
      rc = KEY_EQUALS;
      break;

    case SDLK_KP_MINUS:
    case SDLK_MINUS:
      rc = KEY_MINUS;
      break;

    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
      rc = KEY_RSHIFT;
      break;

    case SDLK_LCTRL:
    case SDLK_RCTRL:
      rc = KEY_RCTRL;
      break;

    case SDLK_LALT:
    case SDLK_RALT:
      rc = KEY_RALT;
      break;

    default:
      rc = sym;
      break;
    }

  return rc;
}

void
I_ShutdownGraphics (void)
{
  SDL_DestroyTexture (framebuffer);
  SDL_DestroyRenderer (renderer);
  SDL_DestroyWindow (window);
  SDL_Quit ();
}



//
// I_StartFrame
//
void
I_StartFrame (void)
{
  // er?

}

int
I_GetEvent (void)
{
  SDL_Event sdl_event;
  event_t event;
  int status;

  // put event-grabbing stuff in here
  status = SDL_PollEvent (&sdl_event);
  switch (sdl_event.type)
    {
    case SDL_KEYDOWN:
      event.type = ev_keydown;
      event.data1 = xlatekey(sdl_event.key.keysym.sym);
      D_PostEvent (&event);
      break;
    case SDL_KEYUP:
      event.type = ev_keyup;
      event.data1 = xlatekey(sdl_event.key.keysym.sym);
      D_PostEvent (&event);
      break;

    case SDL_QUIT:
      I_Quit ();
    }

  return status;
}

//
// I_StartTic
//
void
I_StartTic (void)
{
  // poll all events
  while (I_GetEvent () != 0);
}


//
// I_UpdateNoBlit
//
void
I_UpdateNoBlit (void)
{
  // what is this?
}

//
// I_FinishUpdate
//
void
I_FinishUpdate (void)
{
  int *pixels;
  int pitch;
  register int i;
  SDL_Color color;

  // update the framebuffer with the video screen contents
  SDL_LockTexture (framebuffer, NULL, (void**) &pixels, &pitch);
  for (i = 0; i < SCREENWIDTH*SCREENHEIGHT; i++)
    {
      color = color_palette[screens[0][i]];
      pixels[i] = (color.r << 16) | (color.g << 8) | color.b;
    }
  SDL_UnlockTexture (framebuffer);

  // blit the finished surface to the screen
  SDL_RenderCopy (renderer, framebuffer, NULL, NULL);
  SDL_RenderPresent (renderer);
}


//
// I_ReadScreen
//
void
I_ReadScreen (byte * scr)
{
  memcpy (scr, screens[0], SCREENWIDTH * SCREENHEIGHT);
}

//
// I_SetPalette
//
void
I_SetPalette (byte * palette)
{
  for (int i = 0; i < 256; i++)
    {
      color_palette[i].r = *(palette++);
      color_palette[i].g = *(palette++);
      color_palette[i].b = *(palette++);
    }
}

void
I_InitGraphics (void)
{
  static int firsttime = 1;
  if (!firsttime)
    return;
  firsttime = 0;

  if (M_CheckParm ("-2"))
    multiply = 2;
  if (M_CheckParm ("-3"))
    multiply = 3;
  if (M_CheckParm ("-4"))
    multiply = 4;

  X_width = SCREENWIDTH * multiply;
  X_height = SCREENHEIGHT_4_3 * multiply;

  SDL_Init (SDL_INIT_VIDEO);
  window = SDL_CreateWindow ("DOOM", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, X_width, X_height, SDL_WINDOW_OPENGL);

  renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_ACCELERATED);
  framebuffer = SDL_CreateTexture (renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, SCREENWIDTH, SCREENHEIGHT);

  // check if the user wants to grab the mouse (quite unnice)
  if (M_CheckParm ("-grabmouse"))
    SDL_SetRelativeMouseMode (SDL_TRUE);

  screens[0] = (unsigned char *) malloc (SCREENWIDTH * SCREENHEIGHT);
}
