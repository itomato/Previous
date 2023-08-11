/*
  Hatari - statusbar.c

  This file is distributed under the GNU General Public License, version 2
  or at your option any later version. Read the file gpl.txt for details.

  Code to draw statusbar area, floppy leds etc.

  Use like this:
  - Before screen surface is (re-)created Statusbar_SetHeight()
    has to be called with the new screen height. Add the returned
    value to screen height (zero means no statusbar).  After this,
    Statusbar_GetHeight() can be used to retrieve the statusbar size
  - After screen surface is (re-)created, call Statusbar_Init()
    to re-initialize / re-draw the statusbar
  - Call Statusbar_SetFloppyLed() to set floppy drive led ON/OFF,
    or call Statusbar_EnableHDLed() to enabled HD led for a while
  - Whenever screen is redrawn, call Statusbar_Update() to update
    statusbar contents and find out whether and what screen area
    needs to be updated (outside of screen locking)
  - If screen redraws can be partial, Statusbar_OverlayRestore()
    needs to be called before locking the screen for drawing and
    Statusbar_OverlayBackup() needs to be called after screen unlocking,
    but before calling Statusbar_Update().  These are needed for
    hiding the overlay drive led (= restoring the area that was below
    them before LED was shown) when drive leds are turned OFF.
  - If other information shown by Statusbar (TOS version etc) changes,
    call Statusbar_UpdateInfo()
*/
const char Statusbar_fileid[] = "Hatari statusbar.c";

#include <assert.h>
#include "main.h"
#include "configuration.h"
#include "sdlgui.h"
#include "statusbar.h"
#include "screen.h"
#include "video.h"
#include "grab.h"
#include "dimension.hpp"
#include "str.h"

#include <SDL.h>


#define DEBUG 0
#if DEBUG
# include <execinfo.h>
# define DEBUGPRINT(x) printf x
#else
# define DEBUGPRINT(x)
#endif

/* whole statusbar area, for full updates */
static SDL_Rect FullRect;

/* whether drive leds should be ON and their previous shown state */
static struct {
	drive_led_t state;
	drive_led_t oldstate;
	uint32_t expire;	/* when to disable led, valid only if >0 && state=TRUE */
	int offset;	/* led x-pos on screen */
} Led[NUM_DEVICE_LEDS];


/* drive leds size & y-pos */
static SDL_Rect LedRect;

/* overlay led size & pos */
static SDL_Rect OverlayLedRect;

/* screen contents left under overlay led */
static SDL_Surface *OverlayUnderside;

static enum {
	OVERLAY_NONE,
	OVERLAY_DRAWN,
	OVERLAY_RESTORED
} nOverlayState;

static SDL_Rect SystemLedRect;
static bool bSystemLed, bOldSystemLed;

static SDL_Rect DspLedRect;
static bool bDspLed, bOldDspLed;

static SDL_Rect NdLedRect;
static int nNdLed, nOldNdLed;

/* led colors */
static uint32_t LedColor[ MAX_LED_STATE ];
static uint32_t SysColorOn, SysColorOff;
static uint32_t DspColorOn, DspColorOff;
static uint32_t NdColorOn, NdColorCS8, NdColorOff;
static uint32_t GrayBg, LedColorBg;

/* needs to be enough for all messages, but <= MessageRect width / font width */
#define MAX_MESSAGE_LEN 69
typedef struct msg_item {
	struct msg_item *next;
	char msg[MAX_MESSAGE_LEN+1];
	uint32_t timeout;	/* msecs, zero=no timeout */
	uint32_t expire;  /* when to expire message */
	bool shown;
} msg_item_t;

static msg_item_t DefaultMessage;
static msg_item_t *MessageList = &DefaultMessage;
static SDL_Rect MessageRect;

/* screen height above statusbar and height of statusbar below screen */
static int ScreenHeight;
static int StatusbarHeight;


/*-----------------------------------------------------------------------*/
/**
 * Return statusbar height for given width and height
 */
int Statusbar_GetHeightForSize(int width, int height)
{
	if (ConfigureParams.Screen.bShowStatusbar) {
		/* Should check the same thing as SDLGui_SetScreen()
		 * does to decide the font size.
		 */
		if (width >= 640 && height >= (400-24)) {
			return 24;
		} else {
			return 12;
		}
	}
	return 0;
}

/*-----------------------------------------------------------------------*/
/**
 * Set screen height used for statusbar height calculation.
 *
 * Return height of statusbar that should be added to the screen
 * height when screen is (re-)created, or zero if statusbar will
 * not be shown
 */
int Statusbar_SetHeight(int width, int height)
{
#if DEBUG
	/* find out from where the set height is called */
	void *addr[8];
	int count = backtrace(addr, sizeof(addr)/sizeof(*addr));
	backtrace_symbols_fd(addr, count, fileno(stderr));
#endif
	ScreenHeight = height;
	StatusbarHeight = Statusbar_GetHeightForSize(width, height);
	DEBUGPRINT(("Statusbar_SetHeight(%d, %d) -> %d\n", width, height, StatusbarHeight));
	return StatusbarHeight;
}

/*-----------------------------------------------------------------------*/
/**
 * Return height of statusbar set with Statusbar_SetHeight()
 */
int Statusbar_GetHeight(void)
{
	return StatusbarHeight;
}


/*-----------------------------------------------------------------------*/
/**
 * Enable device led, it will be automatically disabled after a while.
 */
void Statusbar_BlinkLed(drive_index_t drive)
{
	/* leds are shown for 1/2 sec after enabling */
	Led[drive].expire = SDL_GetTicks() + 1000/2;
	Led[drive].state = LED_STATE_ON;
}


/*-----------------------------------------------------------------------*/
/**
 * Set system, DSP, CPU and NeXTdimension led state, anything enabling led with
 * this needs also to take care of disabling it.
 */
void Statusbar_SetSystemLed(bool state) {
	bSystemLed = state;
}

void Statusbar_SetDspLed(bool state) {
	bDspLed = state;
}

void Statusbar_SetNdLed(int state) {
	nNdLed = state;
}

/*-----------------------------------------------------------------------*/
/**
 * Set overlay led size/pos on given screen to internal Rect
 * and free previous resources.
 */
static void Statusbar_OverlayInit(const SDL_Surface *surf)
{
	int h;
	/* led size/pos needs to be re-calculated in case screen changed */
	h = surf->h / 50;
	OverlayLedRect.w = 2*h;
	OverlayLedRect.h = h;
	OverlayLedRect.x = surf->w - 5*h/2;
	OverlayLedRect.y = h/2;
	/* free previous restore surface if it's incompatible */
	if (OverlayUnderside &&
	    OverlayUnderside->w == OverlayLedRect.w &&
	    OverlayUnderside->h == OverlayLedRect.h &&
	    OverlayUnderside->format->BitsPerPixel == surf->format->BitsPerPixel)
	{
		SDL_FreeSurface(OverlayUnderside);
		OverlayUnderside = NULL;
	}
	nOverlayState = OVERLAY_NONE;
}

/*-----------------------------------------------------------------------*/
/**
 * (re-)initialize statusbar internal variables for given screen surface
 * (sizes&colors may need to be re-calculated for the new SDL surface)
 * and draw the statusbar background.
 */
void Statusbar_Init(SDL_Surface *surf)
{
	msg_item_t *item;
	SDL_Rect ledbox;
	int i, fontw, fonth, xoffset;
	const char *text[NUM_DEVICE_LEDS] = { "EN:", "MO:", "SD:", "FD:" };

	DEBUGPRINT(("Statusbar_Init()\n"));
	assert(surf);

	/* dark green and light green for leds themselves */
	LedColor[ LED_STATE_OFF ]     = SDL_MapRGB(surf->format, 0x00, 0x40, 0x00);
	LedColor[ LED_STATE_ON ]      = SDL_MapRGB(surf->format, 0x00, 0xe0, 0x00);
	LedColor[ LED_STATE_ON_BUSY ] = SDL_MapRGB(surf->format, 0xff, 0xe0, 0x00);
	LedColorBg   = SDL_MapRGB(surf->format, 0x00, 0x00, 0x00);
	SysColorOff  = SDL_MapRGB(surf->format, 0x40, 0x00, 0x00);
	SysColorOn   = SDL_MapRGB(surf->format, 0xe0, 0x00, 0x00);
	DspColorOff  = SDL_MapRGB(surf->format, 0x00, 0x00, 0x40);
	DspColorOn   = SDL_MapRGB(surf->format, 0x00, 0x00, 0xe0);
	NdColorOff   = SDL_MapRGB(surf->format, 0x00, 0x00, 0x40);
	NdColorCS8   = SDL_MapRGB(surf->format, 0xe0, 0x00, 0x00);
	NdColorOn    = SDL_MapRGB(surf->format, 0x00, 0x00, 0xe0);
	GrayBg       = SDL_MapRGB(surf->format, 0xb5, 0xb7, 0xaa);

	/* disable leds */
	for (i = 0; i < NUM_DEVICE_LEDS; i++)
	{
		Led[i].state = Led[i].oldstate = LED_STATE_OFF;
		Led[i].expire = 0;
	}
	Statusbar_OverlayInit(surf);
	
	/* disable statusbar if it doesn't fit to video mode */
	if (surf->h < ScreenHeight + StatusbarHeight)
	{
		StatusbarHeight = 0;
	}
	if (!StatusbarHeight)
	{
		DEBUGPRINT(("Doesn't fit <- Statusbar_Init()\n"));
		return;
	}

	/* prepare fonts */
	SDLGui_Init();
	SDLGui_SetScreen(surf);
	SDLGui_GetFontSize(&fontw, &fonth);

	/* video mode didn't match, need to recalculate sizes */
	if (surf->h > ScreenHeight + StatusbarHeight)
	{
		StatusbarHeight = fonth + 2;
		/* actually statusbar vertical offset */
		ScreenHeight = surf->h - StatusbarHeight;
	}
	else
	{
		assert(fonth+2 < StatusbarHeight);
	}

	/* draw statusbar background gray so that text shows */
	FullRect.x = 0;
	FullRect.y = surf->h - StatusbarHeight;
	FullRect.w = surf->w;
	FullRect.h = StatusbarHeight;
	SDL_FillRect(surf, &FullRect, GrayBg);

	/* led size */
	LedRect.w = fonth/2;
	LedRect.h = fonth - 4;
	LedRect.y = ScreenHeight + StatusbarHeight/2 - LedRect.h/2;

	/* black box for the leds */
	ledbox = LedRect;
	ledbox.y -= 1;
	ledbox.w += 2;
	ledbox.h += 2;

	xoffset = fontw;
	MessageRect.y = LedRect.y - 2;
	/* draw led texts and boxes + calculate box offsets */
	for (i = 0; i < NUM_DEVICE_LEDS; i++)
	{
		SDLGui_Text(xoffset, MessageRect.y, text[i]);
		xoffset += strlen(text[i]) * fontw;
		xoffset += fontw/2;

		ledbox.x = xoffset - 1;
		SDL_FillRect(surf, &ledbox, LedColorBg);

		LedRect.x = xoffset;
		SDL_FillRect(surf, &LedRect, LedColor[ LED_STATE_OFF ]);

		Led[i].offset = xoffset;
		xoffset += LedRect.w + fontw;
	}
	MessageRect.x = xoffset + fontw;
	MessageRect.w = MAX_MESSAGE_LEN * fontw;
	MessageRect.h = fonth;
	for (item = MessageList; item; item = item->next) {
		item->shown = false;
	}

	/* draw i860 led box */
	NdLedRect = LedRect;
	NdLedRect.x = surf->w - 15*fontw - NdLedRect.w;
	ledbox.x = NdLedRect.x - 1;
	SDLGui_Text(ledbox.x - 3*fontw - fontw/2, MessageRect.y, "ND:");
	SDL_FillRect(surf, &ledbox, LedColorBg);
	SDL_FillRect(surf, &NdLedRect, NdColorOff);
	nNdLed = 0;

	/* draw dsp led box */
	DspLedRect = LedRect;
	DspLedRect.x = surf->w - 8*fontw - DspLedRect.w;
	ledbox.x = DspLedRect.x - 1;
	SDLGui_Text(ledbox.x - 4*fontw - fontw/2, MessageRect.y, "DSP:");
	SDL_FillRect(surf, &ledbox, LedColorBg);
	SDL_FillRect(surf, &DspLedRect, DspColorOff);
	bDspLed = false;

	/* draw system led box */
	SystemLedRect = LedRect;
	SystemLedRect.x = surf->w - fontw - SystemLedRect.w;
	ledbox.x = SystemLedRect.x - 1;
	SDLGui_Text(ledbox.x - 4*fontw - fontw/2, MessageRect.y, "LED:");
	SDL_FillRect(surf, &ledbox, LedColorBg);
	SDL_FillRect(surf, &SystemLedRect, SysColorOff);
	bSystemLed = false;

	/* and blit statusbar on screen */
	Screen_UpdateRects(surf, 1, &FullRect);
	DEBUGPRINT(("Drawn <- Statusbar_Init()\n"));
}


/*-----------------------------------------------------------------------*/
/**
 * Queue new statusbar message 'msg' to be shown for 'msecs' milliseconds
 */
void Statusbar_AddMessage(const char *msg, uint32_t msecs)
{
	msg_item_t *item;

	if (!ConfigureParams.Screen.bShowStatusbar)
	{
		/* no sense in queuing messages that aren't shown */
		return;
	}
	item = calloc(1, sizeof(msg_item_t));
	assert(item);

	item->next = MessageList;
	MessageList = item;

	Str_Copy(item->msg, msg, sizeof(item->msg));
	DEBUGPRINT(("Add message: '%s'\n", item->msg));

	if (msecs)
	{
		item->timeout = msecs;
	}
	else
	{
		/* show items by default for 2.5 secs */
		item->timeout = 2500;
	}
	item->shown = false;
}

/*-----------------------------------------------------------------------*/
/**
 * Write given 'more' string to 'buffer' and return new end of 'buffer'
 */
static char *Statusbar_AddString(char *buffer, const char *more)
{
	while (*more)
	{
		*buffer++ = *more++;
	}
	return buffer;
}

/*-----------------------------------------------------------------------*/
/**
 * Retrieve/update default statusbar information
 */
void Statusbar_UpdateInfo(void)
{
	char *end = DefaultMessage.msg;
	char memsize[16];
	char slot[16];
	
	/* Recording in progress */
	if (bRecordingAiff)
	{
		end = Statusbar_AddString(end, "Recording sound");
		*end = '\0';
		assert(end - DefaultMessage.msg < MAX_MESSAGE_LEN);
		DefaultMessage.shown = false;
		return;
	}
	
	/* Message for NeXTdimension */
	if (ConfigureParams.Screen.nMonitorType==MONITOR_TYPE_DIMENSION)
	{
		end = Statusbar_AddString(end, "33MHz/i860XR/");
		snprintf(memsize, sizeof(memsize), "%iMB/",
		         Configuration_CheckDimensionMemory(ConfigureParams.Dimension.board[ConfigureParams.Screen.nMonitorNum].nMemoryBankSize));
		end = Statusbar_AddString(end, memsize);
		end = Statusbar_AddString(end, "NeXTdimension/");
		snprintf(slot, sizeof(slot), "Slot%i", ND_SLOT(ConfigureParams.Screen.nMonitorNum));
		end = Statusbar_AddString(end, slot);
		*end = '\0';
		assert(end - DefaultMessage.msg < MAX_MESSAGE_LEN);
		DefaultMessage.shown = false;
		return;
	}
	
	/* CPU MHz */
	end = Statusbar_AddString(end, Main_SpeedMsg());

	/* CPU type */
	if(ConfigureParams.System.nCpuLevel > 0)
	{
		*end++ = '6';
		*end++ = '8';
		*end++ = '0';
		switch (ConfigureParams.System.nCpuLevel)
		{
			case 0: *end++ = '0'; break;
			case 1: *end++ = '1'; break;
			case 2: *end++ = '2'; break;
			case 3: *end++ = '3'; break;
			case 4: *end++ = '4'; break;
			case 5: *end++ = '6'; break;
			default: break;
		}
		*end++ = '0';
		*end++ = '/';
	}

	/* amount of memory */
	snprintf(memsize, sizeof(memsize), "%iMB/",
	         Configuration_CheckMemory(ConfigureParams.Memory.nMemoryBankSize));
	end = Statusbar_AddString(end, memsize);

	/* machine type */
	switch (ConfigureParams.System.nMachineType)
	{
		case NEXT_CUBE030:
			end = Statusbar_AddString(end, "NeXT Computer");
			break;
		case NEXT_CUBE040:
			end = Statusbar_AddString(end, "NeXTcube");
			break;
		case NEXT_STATION:
			end = Statusbar_AddString(end, "NeXTstation");
			break;
			
		default:
			break;
	}
	if (ConfigureParams.System.bTurbo)
	{
		end = Statusbar_AddString(end, (ConfigureParams.System.nCpuFreq==40)?" Nitro":" Turbo");
	}

	if (ConfigureParams.System.bColor)
	{
		end = Statusbar_AddString(end, " Color");
	}

	*end = '\0';

	assert(end - DefaultMessage.msg < MAX_MESSAGE_LEN);
	DEBUGPRINT(("Set default message: '%s'\n", DefaultMessage.msg));
	/* make sure default message gets (re-)drawn when next checked */
	DefaultMessage.shown = false;
}

/*-----------------------------------------------------------------------*/
/**
 * Draw 'msg' centered to the message area
 */
static SDL_Rect* Statusbar_DrawMessage(SDL_Surface *surf, const char *msg)
{
	int fontw, fonth, offset;
	SDL_FillRect(surf, &MessageRect, GrayBg);
	if (*msg)
	{
		SDLGui_GetFontSize(&fontw, &fonth);
		offset = (MessageRect.w - strlen(msg) * fontw) / 2;
		SDLGui_Text(MessageRect.x + offset, MessageRect.y, msg);
	}
	DEBUGPRINT(("Draw message: '%s'\n", msg));
	return &MessageRect;
}

/*-----------------------------------------------------------------------*/
/**
 * If message's not shown, show it.  If message's timed out,
 * remove it and show next one.
 * 
 * Return updated area, or NULL if nothing drawn
 */
static SDL_Rect* Statusbar_ShowMessage(SDL_Surface *surf, uint32_t ticks)
{
	msg_item_t *next;

	if (MessageList->shown)
	{
		if (!MessageList->expire)
		{
			/* last/default message never expires */
			return NULL;
		}
		if (MessageList->expire > ticks)
		{
			/* not timed out yet */
			return NULL;
		}
		assert(MessageList->next); /* last message shouldn't end here */
		next = MessageList->next;
		free(MessageList);
		/* show next */
		MessageList = next;
	}
	/* not shown yet, show */
	MessageList->shown = true;
	if (MessageList->timeout && !MessageList->expire)
	{
		MessageList->expire = ticks + MessageList->timeout;
	}
	return Statusbar_DrawMessage(surf, MessageList->msg);
}


/*-----------------------------------------------------------------------*/
/**
 * Save the area that will be left under overlay led
 */
void Statusbar_OverlayBackup(SDL_Surface *surf)
{
	if ((StatusbarHeight && ConfigureParams.Screen.bShowStatusbar)
	    || !ConfigureParams.Screen.bShowDriveLed)
	{
		/* overlay not used with statusbar */
		return;
	}
	assert(surf);
	if (!OverlayUnderside)
	{
		SDL_Surface *bak;
		SDL_PixelFormat *fmt = surf->format;
		bak = SDL_CreateRGBSurface(surf->flags,
					   OverlayLedRect.w, OverlayLedRect.h,
					   fmt->BitsPerPixel,
					   fmt->Rmask, fmt->Gmask, fmt->Bmask,
					   fmt->Amask);
		assert(bak);
		OverlayUnderside = bak;
	}
	SDL_BlitSurface(surf, &OverlayLedRect, OverlayUnderside, NULL);
}

/*-----------------------------------------------------------------------*/
/**
 * Restore the area left under overlay led
 * 
 * State machine for overlay led handling will return from
 * Statusbar_Update() call the area that is restored (if any)
 */
void Statusbar_OverlayRestore(SDL_Surface *surf)
{
	if ((StatusbarHeight && ConfigureParams.Screen.bShowStatusbar)
	    || !ConfigureParams.Screen.bShowDriveLed)
	{
		/* overlay not used with statusbar */
		return;
	}
	if (nOverlayState == OVERLAY_DRAWN && OverlayUnderside)
	{
		assert(surf);
		SDL_BlitSurface(OverlayUnderside, NULL, surf, &OverlayLedRect);
		/* this will make the draw function to update this the screen */
		nOverlayState = OVERLAY_RESTORED;
	}
}

/*-----------------------------------------------------------------------*/
/**
 * Draw overlay led
 */
static void Statusbar_OverlayDrawLed(SDL_Surface *surf, uint32_t color)
{
	SDL_Rect rect;
	if (nOverlayState == OVERLAY_DRAWN)
	{
		/* some led already drawn */
		return;
	}
	nOverlayState = OVERLAY_DRAWN;

	/* enabled led with border */
	rect = OverlayLedRect;
	rect.x += 1;
	rect.y += 1;
	rect.w -= 2;
	rect.h -= 2;
	SDL_FillRect(surf, &OverlayLedRect, LedColorBg);
	SDL_FillRect(surf, &rect, color);
}

/*-----------------------------------------------------------------------*/
/**
 * Draw overlay led onto screen surface if any drives are enabled.
 * 
 * Return updated area, or NULL if nothing drawn
 */
static SDL_Rect* Statusbar_OverlayDraw(SDL_Surface *surf)
{
	uint32_t currentticks = SDL_GetTicks();
	int i;

	for (i = 0; i < NUM_DEVICE_LEDS; i++)
	{
		if (Led[i].state)
		{
			if (Led[i].expire && Led[i].expire < currentticks)
			{
				Led[i].state = LED_STATE_OFF;
				continue;
			}
			Statusbar_OverlayDrawLed(surf, LedColor[ Led[i].state ]);
			break;
		}
	}
	/* possible state transitions:
	 *   NONE -> DRAWN -> RESTORED -> DRAWN -> RESTORED -> NONE
	 * Other than NONE state needs to be updated on screen
	 */
	switch (nOverlayState)
	{
	case OVERLAY_RESTORED:
		nOverlayState = OVERLAY_NONE;
		/* fall through */
	case OVERLAY_DRAWN:
		DEBUGPRINT(("Overlay LED = %s\n", nOverlayState==OVERLAY_DRAWN?"ON":"OFF"));
		return &OverlayLedRect;
	case OVERLAY_NONE:
		break;
	}
	return NULL;
}


/*-----------------------------------------------------------------------*/
/**
 * Update statusbar information (leds etc) if/when needed.
 * 
 * May not be called when screen is locked (SDL limitation).
 * 
 * Return updated area, or NULL if nothing is drawn.
 */
void Statusbar_Update(SDL_Surface *surf)
{
	uint32_t color, currentticks;
	static SDL_Rect rect;
	SDL_Rect *last_rect;
	int i, updates;

	assert(surf);
	if (!(StatusbarHeight && ConfigureParams.Screen.bShowStatusbar))
	{
		last_rect = NULL;
		/* not enabled (anymore), show overlay led instead? */
		if (ConfigureParams.Screen.bShowDriveLed)
		{
			last_rect = Statusbar_OverlayDraw(surf);
			if (last_rect)
			{
				Screen_UpdateRects(surf, 1, last_rect);
				last_rect = NULL;
			}
		}
		return;
	}

	/* Statusbar_Init() not called before this? */
#if DEBUG
	if (surf->h != ScreenHeight + StatusbarHeight)
	{
		printf("DEBUG: %d != %d + %d\n", surf->h, ScreenHeight, StatusbarHeight);
	}
#endif
	assert(surf->h == ScreenHeight + StatusbarHeight);

	currentticks = SDL_GetTicks();
	last_rect = Statusbar_ShowMessage(surf, currentticks);
	updates = last_rect ? 1 : 0;

	rect = LedRect;
	for (i = 0; i < NUM_DEVICE_LEDS; i++)
	{
		if (Led[i].expire && Led[i].expire < currentticks)
		{
			Led[i].state = LED_STATE_OFF;
		}
		if (Led[i].state == Led[i].oldstate)
		{
			continue;
		}
		Led[i].oldstate = Led[i].state;
		color = LedColor[ Led[i].state ];
		rect.x = Led[i].offset;
		SDL_FillRect(surf, &rect, color);
		DEBUGPRINT(("LED[%d] = %d\n", i, Led[i].state));
		last_rect = &rect;
		updates++;
	}

	Statusbar_ShowMessage(surf, currentticks);

	/* Draw DSP LED */
	if (bDspLed != bOldDspLed)
	{
		bOldDspLed = bDspLed;
		if (bDspLed)
		{
			color = DspColorOn;
		}
		else
		{
			color = DspColorOff;
		}
		SDL_FillRect(surf, &DspLedRect, color);
		last_rect = &DspLedRect;
		updates++;
	}
	
	/* Draw SCR2 LED */
	if (bSystemLed != bOldSystemLed)
	{
		bOldSystemLed = bSystemLed;
		if (bSystemLed)
		{
			color = SysColorOn;
		}
		else
		{
			color = SysColorOff;
		}
		SDL_FillRect(surf, &SystemLedRect, color);
		last_rect = &SystemLedRect;
		updates++;
	}

	/* Draw NeXTdimension LED */
	if (nNdLed != nOldNdLed)
	{
		nOldNdLed = nNdLed;
		switch(nNdLed)
		{
			case 0:  color = NdColorOff; break;
			case 1:  color = NdColorCS8; break;
			case 2:  color = NdColorOn;  break;
			default: color = NdColorOff; break;
		}
		SDL_FillRect(surf, &NdLedRect, color);
		last_rect = &NdLedRect;
		updates++;
	}

	if (updates > 1)
	{
		/* multiple items updated -> update whole statusbar */
		last_rect = &FullRect;
	}
	if (last_rect)
	{
		Screen_UpdateRects(surf, 1, last_rect);
		last_rect = NULL;
	}
}
