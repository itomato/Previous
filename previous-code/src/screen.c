/*
  Previous - screen.c

  This file is distributed under the GNU General Public License, version 2
  or at your option any later version. Read the file gpl.txt for details.

  This file contains the SDL interface for video output.
*/
const char Screen_fileid[] = "Previous screen.c";

#include "main.h"
#include "host.h"
#include "configuration.h"
#include "log.h"
#include "dimension.hpp"
#include "nd_sdl.hpp"
#include "nd_mem.hpp"
#include "paths.h"
#include "screen.h"
#include "statusbar.h"
#include "video.h"
#include "m68000.h"


SDL_Window*   sdlWindow;
SDL_Surface*  sdlscrn = NULL;        /* The SDL screen surface */
static int    nWindowWidth;          /* Width of SDL window in physical pixels */
static int    nWindowHeight;         /* Height of SDL window in physical pixels */
static float  dpiFactor;             /* Factor to convert physical pixels to logical pixels on high-dpi displays */

/* extern for shortcuts */
volatile bool bGrabMouse    = false; /* Grab the mouse cursor in the window */
volatile bool bInFullScreen = false; /* true if in full screen */

static const int NeXT_SCRN_WIDTH  = 1120;
static const int NeXT_SCRN_HEIGHT = 832;
static int width;   /* guest framebuffer */
static int height;  /* guest framebuffer */

static SDL_Renderer* sdlRenderer;
static SDL_Texture*  uiTexture;
static SDL_Texture*  fbTexture;
static SDL_atomic_t  blitUI;
static bool          doUIblit;
static SDL_Rect      statusBar;
static SDL_Rect      screenRect;
static SDL_Rect      saveWindowBounds; /* Window bounds before going fullscreen. Used to restore window size & position. */
static MONITORTYPE   saveMonitorType;  /* Save monitor type to restore on return from fullscreen */
static uint32_t      mask;             /* green screen mask for transparent UI areas */
static void*         uiBuffer;         /* uiBuffer used for user interface texture */
static SDL_SpinLock  uiBufferLock;     /* Lock for concurrent access to UI buffer between m68k thread and repainter */
#ifdef ENABLE_RENDERING_THREAD
static volatile bool doRepaint = true; /* Repaint thread runs while true */
static SDL_Thread*   repaintThread;
#endif


static uint32_t BW2RGB[0x400];
static uint32_t COL2RGB[0x10000];

static uint32_t bw2rgb(SDL_PixelFormat* format, int bw) {
	switch(bw & 3) {
		case 3:  return SDL_MapRGB(format, 0,   0,   0);
		case 2:  return SDL_MapRGB(format, 85,  85,  85);
		case 1:  return SDL_MapRGB(format, 170, 170, 170);
		case 0:  return SDL_MapRGB(format, 255, 255, 255);
		default: return 0;
	}
}

static uint32_t col2rgb(SDL_PixelFormat* format, int col) {
	int r = col & 0xF000; r >>= 12; r |= r << 4;
	int g = col & 0x0F00; g >>= 8;  g |= g << 4;
	int b = col & 0x00F0; b >>= 4;  b |= b << 4;
	return SDL_MapRGB(format, r, g, b);
}

/*
 BW format is 2 bit per pixel
 */
static void blitBW(SDL_Texture* tex) {
	void* pixels;
	uint32_t* dst;
	int src, idx, src_pitch, dst_pitch, x, y;

	src_pitch = (NeXT_SCRN_WIDTH + (ConfigureParams.System.bTurbo ? 0 : 32)) / 4;
	SDL_LockTexture(tex, NULL, &pixels, &dst_pitch);
	for (y = 0; y < NeXT_SCRN_HEIGHT; y++) {
		src = y * src_pitch;
		dst = (uint32_t*)((uint8_t*)pixels + (y * dst_pitch));
		for (x = 0; x < NeXT_SCRN_WIDTH / 4; x++) {
			idx = NEXTVideo[src++] * 4;
			*dst++ = BW2RGB[idx+0];
			*dst++ = BW2RGB[idx+1];
			*dst++ = BW2RGB[idx+2];
			*dst++ = BW2RGB[idx+3];
		}
	}
	SDL_UnlockTexture(tex);
}

/*
 Color format is 4 bit per pixel, big-endian: RGBX
 */
static void blitColor(SDL_Texture* tex) {
	void* pixels;
	uint16_t* src;
	uint32_t* dst;
	int src_pitch, dst_pitch, x, y;

	src_pitch = NeXT_SCRN_WIDTH + (ConfigureParams.System.bTurbo ? 0 : 32);
	SDL_LockTexture(tex, NULL, &pixels, &dst_pitch);
	for (y = 0; y < NeXT_SCRN_HEIGHT; y++) {
		src = (uint16_t*)NEXTVideo + (y * src_pitch);
		dst = (uint32_t*)((uint8_t*)pixels + (y * dst_pitch));
		for (x = 0; x < NeXT_SCRN_WIDTH; x++) {
			*dst++ = COL2RGB[*src++];
		}
	}
	SDL_UnlockTexture(tex);
}

/*
 Dimension format is 8 bit per pixel, big-endian: BBGGRRAA
 */
void Screen_BlitDimension(uint32_t* vram, SDL_Texture* tex) {
	void* src;
	void* dst;
	int src_pitch, dst_pitch;
	uint32_t src_format, dst_format;

#if ND_STEP
	src = &vram[0];
#else
	src = &vram[4];
#endif
	src_pitch  = (NeXT_SCRN_WIDTH + 32) * 4;
	src_format = SDL_PIXELFORMAT_BGRA32;
	SDL_QueryTexture(tex, &dst_format, NULL, NULL, NULL);

	SDL_LockTexture(tex, NULL, &dst, &dst_pitch);
	SDL_ConvertPixels(NeXT_SCRN_WIDTH, NeXT_SCRN_HEIGHT, src_format, src, src_pitch, dst_format, dst, dst_pitch);
	SDL_UnlockTexture(tex);
}

/*
 Blank screen
 */
void Screen_Blank(SDL_Texture* tex) {
	void* pixels;
	int   pitch;
	SDL_LockTexture(tex, NULL, &pixels, &pitch);
	SDL_memset4(pixels, COL2RGB[0], pitch * NeXT_SCRN_HEIGHT / 4);
	SDL_UnlockTexture(tex);
}

/*
 Blit NeXT framebuffer to texture.
 */
static bool blitScreen(SDL_Texture* tex) {
	if (ConfigureParams.Screen.nMonitorType==MONITOR_TYPE_DIMENSION) {
		uint32_t* vram = nd_vram_for_slot(ND_SLOT(ConfigureParams.Screen.nMonitorNum));
		if (vram) {
			if (nd_video_enabled(ND_SLOT(ConfigureParams.Screen.nMonitorNum))) {
				Screen_BlitDimension(vram, tex);
			} else {
				Screen_Blank(tex);
			}
			return true;
		}
	} else {
		if (NEXTVideo) {
			if (Video_Enabled()) {
				if (ConfigureParams.System.bColor) {
					blitColor(tex);
				} else {
					blitBW(tex);
				}
			} else {
				Screen_Blank(tex);
			}
			return true;
		}
	}
	return false;
}

/*
 Blit user interface to texture.
 */
static void blitUserInterface(SDL_Texture* tex) {
	void* pixels;
	int   pitch;
	SDL_LockTexture(tex, NULL, &pixels, &pitch);
	SDL_AtomicLock(&uiBufferLock);
	memcpy(pixels, uiBuffer, height * pitch);
	SDL_AtomicSet(&blitUI, 0);
	SDL_AtomicUnlock(&uiBufferLock);
	SDL_UnlockTexture(tex);
}

/*
 Blits the NeXT framebuffer to the fbTexture, blends with the GUI surface and shows it.
 */
bool Screen_Repaint(void) {
	bool updateScreen = false;

	/* Blit the NeXT framebuffer to texture */
	if (bEmulationActive) {
		updateScreen = blitScreen(fbTexture);
	}

	/* Copy UI surface to texture */
	if (SDL_AtomicGet(&blitUI)) {
		blitUserInterface(uiTexture);
		updateScreen = true;
	}

	if (updateScreen) {
		SDL_RenderClear(sdlRenderer);
		/* Render NeXT framebuffer texture */
		SDL_RenderCopy(sdlRenderer, fbTexture, NULL, &screenRect);
		SDL_RenderCopy(sdlRenderer, uiTexture, NULL, &screenRect);
		/* Sleeps until next VSYNC if enabled in ScreenInit */
		SDL_RenderPresent(sdlRenderer);
	}

	return updateScreen;
}

#ifdef ENABLE_RENDERING_THREAD
static int repainter(void* unused) {
	SDL_SetThreadPriority(SDL_THREAD_PRIORITY_NORMAL);

	/* Enter repaint loop */
	while (doRepaint) {
		if (!Screen_Repaint()) {
			host_sleep_ms(10);
		}
	}
	return 0;
}
#endif

/*-----------------------------------------------------------------------*/
/**
 * Init Screen, creates window, renderer and textures
 */
void Screen_Init(void) {
	uint32_t format;
	uint32_t r, g, b, a;
	int      d, i;

#ifdef ENABLE_RENDERING_THREAD
	SDL_RendererFlags vsync_flag = SDL_RENDERER_PRESENTVSYNC;
#else
	uint32_t vsync_flag = 0;
#endif

	/* Set initial window resolution */
	width  = NeXT_SCRN_WIDTH;
	height = NeXT_SCRN_HEIGHT;
	bInFullScreen = false;

	/* Statusbar */
	Statusbar_SetHeight(width, height);
	statusBar.x = 0;
	statusBar.y = height;
	statusBar.w = width;
	statusBar.h = Statusbar_GetHeight();
	/* Grow to fit statusbar */
	height += Statusbar_GetHeight();

	/* Screen */
	screenRect.x = 0;
	screenRect.y = 0;
	screenRect.h = height;
	screenRect.w = width;

	/* Set new video mode */
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	fprintf(stderr, "SDL screen request: %d x %d (%s)\n", width, height, bInFullScreen ? "fullscreen" : "windowed");

	sdlWindow = SDL_CreateWindow(PROG_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	if (!sdlWindow) {
		Main_ErrorExit("Failed to create window:", SDL_GetError(), -1);
	}

	SDL_GetWindowSizeInPixels(sdlWindow, &nWindowWidth, &nWindowHeight);
	if (nWindowWidth > 0) {
		dpiFactor = (float)width / nWindowWidth;
	} else {
		fprintf(stderr, "Failed to set screen scale\n");
		dpiFactor = 1.0;
	}
	fprintf(stderr, "SDL screen scale: %.3f\n", dpiFactor);

	sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | vsync_flag);
	if (!sdlRenderer) {
		fprintf(stderr, "Failed to create accelerated renderer: %s!\n", SDL_GetError());
		sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, vsync_flag);
		if (!sdlRenderer) {
			Main_ErrorExit("Failed to create renderer:", SDL_GetError(), -1);
		}
	}

	SDL_RenderSetLogicalSize(sdlRenderer, width, height);

	format = SDL_PIXELFORMAT_BGRA32;

	uiTexture = SDL_CreateTexture(sdlRenderer, format, SDL_TEXTUREACCESS_STREAMING, width, height);
	fbTexture = SDL_CreateTexture(sdlRenderer, format, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (!uiTexture || !fbTexture) {
		Main_ErrorExit("Failed to create texture:", SDL_GetError(), -1);
	}
	SDL_SetTextureBlendMode(uiTexture, SDL_BLENDMODE_BLEND);
	SDL_SetTextureBlendMode(fbTexture, SDL_BLENDMODE_NONE);

	SDL_PixelFormatEnumToMasks(format, &d, &r, &g, &b, &a);

	sdlscrn = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, d, r, g, b, a);

	/* Exit if we can not open a screen */
	if (!sdlscrn) {
		Main_ErrorExit("Could not set video mode:", SDL_GetError(), -2);
	}

	/* Clear UI with mask */
	mask = g | a;
	SDL_FillRect(sdlscrn, NULL, mask);

	/* Allocate buffers for copy routines */
	uiBuffer = malloc(sdlscrn->h * sdlscrn->pitch);

	/* Initialize statusbar */
	Statusbar_Init(sdlscrn);

	/* Setup lookup tables */
	SDL_PixelFormat* pformat = SDL_AllocFormat(format);
	/* initialize BW lookup table */
	for (i = 0; i < 0x100; i++) {
		BW2RGB[i*4+0] = bw2rgb(pformat, i>>6);
		BW2RGB[i*4+1] = bw2rgb(pformat, i>>4);
		BW2RGB[i*4+2] = bw2rgb(pformat, i>>2);
		BW2RGB[i*4+3] = bw2rgb(pformat, i>>0);
	}
	/* initialize color lookup table */
	for (i = 0; i < 0x10000; i++)
		COL2RGB[SDL_BYTEORDER == SDL_BIG_ENDIAN ? i : SDL_Swap16(i)] = col2rgb(pformat, i);

	SDL_FreeFormat(pformat);

	/* Start with blank screen */
	Screen_Blank(fbTexture);

#ifdef ENABLE_RENDERING_THREAD
	/* Start repaint thread */
	repaintThread = SDL_CreateThread(repainter, "[Previous] Screen at slot 0", NULL);
#endif

	/* Configure some SDL stuff: */
	Main_ShowCursor(false);
	Main_SetMouseGrab(bGrabMouse);

	if (ConfigureParams.Screen.bFullScreen) {
		Screen_EnterFullScreen();
	}
}

/*-----------------------------------------------------------------------*/
/**
 * Free screen bitmap and allocated resources
 */
void Screen_UnInit(void) {
#ifdef ENABLE_RENDERING_THREAD
	int s;
	doRepaint = false; /* stop repaint thread */
	SDL_WaitThread(repaintThread, &s);
#endif
	nd_sdl_destroy();
	free(uiBuffer);
	SDL_FreeSurface(sdlscrn);
	SDL_DestroyTexture(uiTexture);
	SDL_DestroyTexture(fbTexture);
	SDL_DestroyRenderer(sdlRenderer);
	SDL_DestroyWindow(sdlWindow);
}

/*-----------------------------------------------------------------------*/
/**
 * Enter Full screen mode
 */
void Screen_EnterFullScreen(void) {
	bool bWasRunning;

	if (!bInFullScreen) {
		/* Hold things... */
		bWasRunning = Main_PauseEmulation(false);
		bInFullScreen = true;

		SDL_GetWindowPosition(sdlWindow, &saveWindowBounds.x, &saveWindowBounds.y);
		SDL_GetWindowSize(sdlWindow, &saveWindowBounds.w, &saveWindowBounds.h);
		SDL_SetWindowFullscreen(sdlWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
		SDL_Delay(100);                  /* To give monitor time to change to new resolution */

		/* If using multiple screen windows, save and go to single window mode */
		saveMonitorType = ConfigureParams.Screen.nMonitorType;
		if (ConfigureParams.Screen.nMonitorType == MONITOR_TYPE_DUAL) {
			ConfigureParams.Screen.nMonitorType = MONITOR_TYPE_CPU;
			Screen_ModeChanged();
		}

		if (bWasRunning) {
			/* And off we go... */
			Main_UnPauseEmulation();
		}

		/* Always grab mouse pointer in full screen mode */
		Main_SetMouseGrab(true);

		/* Make sure screen is painted in case emulation is paused */
		SDL_AtomicSet(&blitUI, 1);
	}
}

/*-----------------------------------------------------------------------*/
/**
 * Return from Full screen mode back to a window
 */
void Screen_ReturnFromFullScreen(void) {
	bool bWasRunning;

	if (bInFullScreen) {
		/* Hold things... */
		bWasRunning = Main_PauseEmulation(false);
		bInFullScreen = false;

		SDL_SetWindowFullscreen(sdlWindow, 0);
		SDL_Delay(100);                /* To give monitor time to switch resolution */
		SDL_SetWindowSize(sdlWindow, saveWindowBounds.w, saveWindowBounds.h);
		SDL_SetWindowPosition(sdlWindow, saveWindowBounds.x, saveWindowBounds.y);

		/* Return to windowed monitor mode */
		if (saveMonitorType == MONITOR_TYPE_DUAL) {
			ConfigureParams.Screen.nMonitorType = saveMonitorType;
			Screen_ModeChanged();
		}

		if (bWasRunning) {
			/* And off we go... */
			Main_UnPauseEmulation();
		}

		/* Go back to windowed mode mouse grab settings */
		Main_SetMouseGrab(bGrabMouse);

		/* Make sure screen is painted in case emulation is paused */
		SDL_AtomicSet(&blitUI, 1);
	}
}

/*-----------------------------------------------------------------------*/
/**
 * Show main window
 */
void Screen_ShowMainWindow(void) {
	if (!bInFullScreen) {
		SDL_RestoreWindow(sdlWindow);
		SDL_RaiseWindow(sdlWindow);
	}
}

/*-----------------------------------------------------------------------*/
/**
 * Force things associated with changing screen size
 */
void Screen_SizeChanged(void) {
	float scale;

	if (!bInFullScreen) {
		SDL_RenderGetScale(sdlRenderer, &scale, &scale);
		SDL_SetWindowSize(sdlWindow, width*scale*dpiFactor, height*scale*dpiFactor);

		nd_sdl_resize(scale*dpiFactor);
	}

	/* Make sure screen is painted in case emulation is paused */
	SDL_AtomicSet(&blitUI, 1);
}


/*-----------------------------------------------------------------------*/
/**
 * Force things associated with changing between fullscreen/windowed
 */
void Screen_ModeChanged(void) {
	if (!sdlscrn) {
		/* screen not yet initialized */
		return;
	}

	/* Do not use multiple windows in full screen mode */
	if (ConfigureParams.Screen.nMonitorType == MONITOR_TYPE_DUAL && bInFullScreen) {
		saveMonitorType = ConfigureParams.Screen.nMonitorType;
		ConfigureParams.Screen.nMonitorType = MONITOR_TYPE_CPU;
	}
	if (ConfigureParams.Screen.nMonitorType == MONITOR_TYPE_DUAL && !bInFullScreen) {
		nd_sdl_show();
	} else {
		nd_sdl_hide();
	}
}


/*-----------------------------------------------------------------------*/
/**
 * Force things associated with changing statusbar visibility
 */
void Screen_StatusbarChanged(void) {
	float scale;

	if (!sdlscrn) {
		/* screen not yet initialized */
		return;
	}

	/* Get new heigt for our window */
	height = NeXT_SCRN_HEIGHT + Statusbar_SetHeight(NeXT_SCRN_WIDTH, NeXT_SCRN_HEIGHT);

	if (bInFullScreen) {
		saveWindowBounds.h = (height * saveWindowBounds.w) / width;
		SDL_RenderSetLogicalSize(sdlRenderer, width, height);
	} else {
		SDL_RenderGetScale(sdlRenderer, &scale, &scale);
		SDL_SetWindowSize(sdlWindow, width*scale*dpiFactor, height*scale*dpiFactor);
		SDL_RenderSetLogicalSize(sdlRenderer, width, height);
		SDL_RenderSetScale(sdlRenderer, scale, scale);
	}

	/* Make sure screen is painted in case emulation is paused */
	SDL_AtomicSet(&blitUI, 1);
}


/*-----------------------------------------------------------------------*/
/**
 * Draw screen to window/full-screen - (SC) Just status bar updates. Screen redraw is done in repaint thread.
 */
static void statusBarUpdate(void) {
	SDL_LockSurface(sdlscrn);
	SDL_AtomicLock(&uiBufferLock);
	memcpy(&((uint8_t*)uiBuffer)[statusBar.y*sdlscrn->pitch], &((uint8_t*)sdlscrn->pixels)[statusBar.y*sdlscrn->pitch], statusBar.h * sdlscrn->pitch);
	SDL_AtomicSet(&blitUI, 1);
	SDL_AtomicUnlock(&uiBufferLock);
	SDL_UnlockSurface(sdlscrn);
}

/*
 Copy UI SDL surface to uiBuffer and replace mask pixels with transparent pixels for
 UI blending with framebuffer texture.
*/
static void uiUpdate(void) {
	SDL_LockSurface(sdlscrn);
	int     count = sdlscrn->w * sdlscrn->h;
	uint32_t* dst = (uint32_t*)uiBuffer;
	uint32_t* src = (uint32_t*)sdlscrn->pixels;
	SDL_AtomicLock(&uiBufferLock);
	/* poor man's green-screen - would be nice if SDL had more blending modes... */
	for(int i = count; --i >= 0; src++)
		*dst++ = *src == mask ? 0 : *src;
	SDL_AtomicSet(&blitUI, 1);
	SDL_AtomicUnlock(&uiBufferLock);
	SDL_UnlockSurface(sdlscrn);
}

void Screen_UpdateRects(SDL_Surface *screen, int numrects, SDL_Rect *rects) {
	while(numrects--) {
		if(rects->y < NeXT_SCRN_HEIGHT) {
			uiUpdate();
			doUIblit = true;
		} else {
			if(doUIblit) {
				uiUpdate();
				doUIblit = false;
			} else {
				statusBarUpdate();
			}
		}
	}
#ifndef ENABLE_RENDERING_THREAD
	if (!bEmulationActive) {
		Screen_Repaint();
	}
#endif
}

void Screen_UpdateRect(SDL_Surface *screen, int32_t x, int32_t y, int32_t w, int32_t h) {
	SDL_Rect rect = { x, y, w, h };
	Screen_UpdateRects(screen, 1, &rect);
}
