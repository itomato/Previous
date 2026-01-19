/*
  Previous - nd_sdl.cpp

  This file is distributed under the GNU General Public License, version 2
  or at your option any later version. Read the file gpl.txt for details.
*/

#include "main.h"
#include "nd_sdl.hpp"
#include "configuration.h"
#include "dimension.hpp"
#include "screen.h"
#include "host.h"
#include "cycInt.h"


#ifdef ENABLE_RENDERING_THREAD
NDSDL::NDSDL(int slot, uint32_t* vram) : slot(slot), vram(vram), ndWindow(NULL), ndRenderer(NULL), ndTexture(NULL), doRepaint(true), repaintThread(NULL) {}

int NDSDL::repainter(void *_this) {
    return ((NDSDL*)_this)->repainter();
}

int NDSDL::repainter(void) {
    SDL_SetCurrentThreadPriority(SDL_THREAD_PRIORITY_NORMAL);

    while (doRepaint) {
        if (bEmulationActive && SDL_GetAtomicInt(&blitNDFB)) {
            repaint();
        } else {
            host_sleep_ms(100);
        }
    }

    return 0;
}
#else // !ENABLE_RENDERING_THREAD
NDSDL::NDSDL(int slot, uint32_t* vram) : slot(slot), vram(vram), ndWindow(NULL), ndRenderer(NULL), ndTexture(NULL) {}
#endif // !ENABLE_RENDERING_THREAD

void NDSDL::repaint(void) {
    if (nd_video_enabled(slot)) {
        Screen_BlitDimension(vram, ndTexture);
    } else {
        Screen_Blank(ndTexture);
    }
    SDL_RenderClear(ndRenderer);
    SDL_RenderTexture(ndRenderer, ndTexture, NULL, NULL);
    SDL_RenderPresent(ndRenderer);
}

void NDSDL::init(void) {
    int x, y, w, h;
    char name[32];
    SDL_Rect r = {0,0,1120,832};

    if (!ndWindow) {
        SDL_GetWindowPosition(sdlWindow, &x, &y);
        SDL_GetWindowSize(sdlWindow, &w, &h);
        h = (w * 832) / 1120;
        snprintf(name, sizeof(name), "NeXTdimension (Slot %i)", slot);
        ndWindow = SDL_CreateWindow(name, w, h, SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY);
        
        if (!ndWindow) {
            fprintf(stderr,"[ND] Slot %i: Failed to create window! (%s)\n", slot, SDL_GetError());
            exit(-1);
        }
        SDL_SetWindowPosition(ndWindow, x+14*slot, y+14*slot);
    }
    
    if (ConfigureParams.Screen.nMonitorType == MONITOR_TYPE_DUAL) {
        titlebar(ConfigureParams.Screen.bShowTitlebar);
        if (!ndRenderer) {
            ndRenderer = SDL_CreateRenderer(ndWindow, NULL);
            if (!ndRenderer) {
                fprintf(stderr,"[ND] Slot %i: Failed to create renderer! (%s)\n", slot, SDL_GetError());
                exit(-1);
            }
            SDL_SetRenderLogicalPresentation(ndRenderer, r.w, r.h, SDL_LOGICAL_PRESENTATION_STRETCH);
            ndTexture = SDL_CreateTexture(ndRenderer, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING, r.w, r.h);
            SDL_SetTextureBlendMode(ndTexture, SDL_BLENDMODE_NONE);
#ifdef ENABLE_RENDERING_THREAD
            SDL_SetRenderVSync(ndRenderer, 1);

            snprintf(name, sizeof(name), "[Previous] Screen at slot %d", slot);
            repaintThread = SDL_CreateThread(NDSDL::repainter, name, this);
#endif
        }

        SDL_ShowWindow(ndWindow);
#ifdef ENABLE_RENDERING_THREAD
        SDL_SetAtomicInt(&blitNDFB, 1);
#endif
    } else {
        SDL_HideWindow(ndWindow);
    }
}

void NDSDL::uninit(void) {
#ifdef ENABLE_RENDERING_THREAD
    SDL_SetAtomicInt(&blitNDFB, 0);
#endif
    SDL_HideWindow(ndWindow);
}

void NDSDL::destroy(void) {
#ifdef ENABLE_RENDERING_THREAD
    doRepaint = false; // stop repaint thread
    int s;
    SDL_WaitThread(repaintThread, &s);
#endif
    SDL_DestroyTexture(ndTexture);
    SDL_DestroyRenderer(ndRenderer);
    SDL_DestroyWindow(ndWindow);
}

void NDSDL::resize(float scale) {
    if (ndWindow) {
        SDL_SetWindowSize(ndWindow, (int)SDL_lroundf(scale*1120), (int)SDL_lroundf(scale*832));
    }
}

void NDSDL::titlebar(bool show) {
    if (ndWindow) {
        SDL_SetWindowBordered(ndWindow, show);
    }
}

#ifndef ENABLE_RENDERING_THREAD
void nd_sdl_repaint(void) {
    FOR_EACH_SLOT(slot) {
        IF_NEXT_DIMENSION(slot, nd) {
            nd->sdl.repaint();
        }
    }
}
#endif

void nd_sdl_titlebar(bool show) {
    FOR_EACH_SLOT(slot) {
        IF_NEXT_DIMENSION(slot, nd) {
            nd->sdl.titlebar(show);
        }
    }
}

void nd_sdl_resize(float scale) {
    FOR_EACH_SLOT(slot) {
        IF_NEXT_DIMENSION(slot, nd) {
            nd->sdl.resize(scale);
        }
    }
}

void nd_sdl_show(void) {
    FOR_EACH_SLOT(slot) {
        IF_NEXT_DIMENSION(slot, nd) {
            nd->sdl.init();
        }
    }
}

void nd_sdl_hide(void) {
    FOR_EACH_SLOT(slot) {
        IF_NEXT_DIMENSION(slot, nd) {
            nd->sdl.uninit();
        }
    }
}
