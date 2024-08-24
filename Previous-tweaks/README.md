# Tweaks
This is a modification to the Previous SDL GUI to change from an Atari font to a more appropriate font, Apple Monaco.

Instead of the 'X-ed out blob' icon Hatari uses to represent a directory, we have a simplistic 'folder' icon.

It's a more or less complete replacement of all the characters, but as an American, it's difficult for me to ID and test all the characters represented.

![](monaco-sdl-gui.jpg)

The GIMP was used to load and replace the character set. A patched up XCF file is included. It uses a 10x16 grid, and *should* still have the appropriate letter/line spacing to make replacement a type-and-go operation.

I wanted to use Ohlfs, but can't seem to get the font converted for use on the Mac. May still try ;)

## Workflow for the curious:

### Fonts and Icons:

- Import original Hatari bitmap into GIMP
- Apply grid over bitmap
- Monaco (without antialiasing) characters were used where possible (see folder icons for exception)

This is where I'm fuzzy, some 1.5Y later.. I either used GIMP's native C header export or a command pipeline. Time will tell.

### Colors

I have also changed the colors in the SDL GUI to closely reflect the default gray of interface elements as shown on a 24bit display (NeXT-native Gamma).


	< 	Uint32 grey = SDL_MapRGB(pSdlGuiScrn->format,170,170,170);
	---
	> 	Uint32 grey = SDL_MapRGB(pSdlGuiScrn->format,181,183,170);

	< 		upleftc = SDL_MapRGB(pSdlGuiScrn->format,86,86,86);
	---
	> 		upleftc = SDL_MapRGB(pSdlGuiScrn->format,147,145,170);

	< 		downrightc = SDL_MapRGB(pSdlGuiScrn->format,0,0,0);
	---
	> 		downrightc = SDL_MapRGB(pSdlGuiScrn->format,147,145,170);

	<  	Uint32 grey0 = SDL_MapRGB(pSdlGuiScrn->format,170,170,170);
	<  	Uint32 grey1 = SDL_MapRGB(pSdlGuiScrn->format,80,80,80);
	<  	Uint32 grey2 = SDL_MapRGB(pSdlGuiScrn->format, 0, 0, 0);
	---
	>  	Uint32 grey0 = SDL_MapRGB(pSdlGuiScrn->format,147,145,170);
	>  	Uint32 grey1 = SDL_MapRGB(pSdlGuiScrn->format,181,183,170);
	>  	Uint32 grey2 = SDL_MapRGB(pSdlGuiScrn->format, 73, 72, 85);


# Using it

Clone this repo

Run tweak.sh

Run build-mac.sh


