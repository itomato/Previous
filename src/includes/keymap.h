/*
  Hatari - keymap.h

  This file is distributed under the GNU General Public License, version 2
  or at your option any later version. Read the file gpl.txt for details.
*/

#ifndef HATARI_KEYMAP_H
#define HATARI_KEYMAP_H

#include <SDL.h>

extern void Keymap_Init(void);

extern void Keymap_MouseWheel(const SDL_MouseWheelEvent *sdlwheel);
extern void Keymap_MouseMove(int dx, int dy);
extern void Keymap_MouseDown(bool left);
extern void Keymap_MouseUp(bool left);

extern void Keymap_KeyDown(const SDL_Keysym *sdlkey);
extern void Keymap_KeyUp(const SDL_Keysym *sdlkey);

extern int Keymap_GetKeyFromName(const char *name);
extern const char *Keymap_GetKeyName(int keycode);

#endif
