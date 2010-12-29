#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "SDL_image.h"
#include "cap32.h"
#include "psp_danzeff.h"
#include "kbd.h"
#include "psp_fmgr.h"

#define false 0
#define true 1

# define PSP_KBD_MAX_SKIN    128

  int    psp_kbd_last_skin  = 0;
  int    psp_kbd_skin       = -1;
  char  *psp_kbd_skin_dir[PSP_KBD_MAX_SKIN];
  int    psp_kbd_skin_first = 1;

static /*bool*/ int holding = false;     //user is holding a button
static /*bool*/ int dirty = true;        //keyboard needs redrawing
static /*bool*/ int shifted = false;     //user is holding shift
static int mode = 0;             //charset selected. (0 - letters or 1 - numbers)
static /*bool*/ int initialized = false; //keyboard is initialized

//Position on the 3-3 grid the user has selected (range 0-2)
static int selected_x = 1;
static int selected_y = 1;

//Variable describing where each of the images is
#define guiStringsSize 12 /* size of guistrings array */
#define PICS_BASEDIR "./graphics/"

static char *guiStrings[] = 
{
	"keys.png", "keys_t.png", "keys_s.png",
	"keys_c.png", "keys_c_t.png", "keys_s_c.png",
	"nums.png", "nums_t.png", "nums_s.png",
	"nums_c.png", "nums_c_t.png", "nums_s_c.png"
};

//amount of modes (non shifted), each of these should have a corresponding shifted mode.
#define MODE_COUNT 2
//this is the layout of the keyboard
static char modeChar[MODE_COUNT*2][3][3][5] = 
{
	{	//standard letters
		{ ",abc",  ".def","!ghi" },
		{ "-jkl", { DANZEFF_DEL, 'm', ' ', 'n', '\0' }, "?opq" },
		{ "(rst",  ":uvw",")xyz" }
	},
	
	{	//capital letters
		{ "^ABC",  "@DEF","*GHI" },
		{ "_JKL", { DANZEFF_DEL, 'M', ' ', 'N', '\0' }, "\"OPQ" },
		{ "=RST",  ";UVW","/XYZ" }
	},
	
	{	//numbers
		{ {   DANZEFF_CONTROL,   DANZEFF_F1,  DANZEFF_LSHIFT   , '1', '\0' }, 
      {   DANZEFF_ESC    ,   DANZEFF_F2,  DANZEFF_CAPSLOCK , '2', '\0' }, 
      {   DANZEFF_COPY   ,   DANZEFF_F3,  DANZEFF_RSHIFT   , '3', '\0' }},

		{ {   DANZEFF_TAB    ,   DANZEFF_F4,  DANZEFF_CLR      , '4', '\0' }, 
      {   DANZEFF_DEL    ,   DANZEFF_F5,                ' ', '5', '\0' }, 
      {   DANZEFF_RETURN ,   DANZEFF_F6,  DANZEFF_ENTER    , '6', '\0' }},

		{ {   DANZEFF_HOMEBL ,   DANZEFF_F7,  DANZEFF_HOMELN   , '7', '\0' }, 
      {   DANZEFF_ENDBL  ,   DANZEFF_F8,  DANZEFF_ENDLN    , '8', '\0' }, 
      {   DANZEFF_RUN    ,   DANZEFF_F9,                '0', '9', '\0' }},
	},

	{	//special characters
		{ ",(.)",  "\"<'>","-[_]" },
		{ "!{?}","\010\0 \0", "+\\=/" },
		{ ":@;#",  "~$`%","*^|&" }
	}
};

int 
danzeff_isinitialized()
{
	return initialized;
}

int 
danzeff_dirty()
{
	return dirty;
}

/** Attempts to read a character from the controller
* If no character is pressed then we return 0
* Other special values: 1 = move left, 2 = move right, 3 = select, 4 = start
* Every other value should be a standard ascii value.
* An unsigned int is returned so in the future we can support unicode input
*/
unsigned int 
danzeff_readInput(gp2xCtrlData pspctrl)
{
	//Work out where the analog stick is selecting
	int x = 1;
	int y = 1;
  if (pspctrl.Buttons & GP2X_CTRL_LEFT ) x -= 1;
  if (pspctrl.Buttons & GP2X_CTRL_RIGHT) x += 1;
  if (pspctrl.Buttons & GP2X_CTRL_UP   ) y -= 1;
  if (pspctrl.Buttons & GP2X_CTRL_DOWN)  y += 1;
 
	if (selected_x != x || selected_y != y) //If they've moved, update dirty
	{
		dirty = true;
		selected_x = x;
		selected_y = y;
	}
	//if they are changing shift then that makes it dirty too
	if ((!shifted && (pspctrl.Buttons & GP2X_CTRL_RTRIGGER)) || (shifted && !(pspctrl.Buttons & GP2X_CTRL_RTRIGGER)))
		dirty = true;
	
	unsigned int pressed = 0; //character they have entered, 0 as that means 'nothing'
	shifted = (pspctrl.Buttons & GP2X_CTRL_RTRIGGER)?true:false;
	
	if (!holding)
	{
		if (pspctrl.Buttons& (GP2X_CTRL_CROSS|GP2X_CTRL_CIRCLE|GP2X_CTRL_TRIANGLE|GP2X_CTRL_SQUARE)) //pressing a char select button
		{
			int innerChoice = 0;
			if      (pspctrl.Buttons & GP2X_CTRL_TRIANGLE)
				innerChoice = 0;
			else if (pspctrl.Buttons & GP2X_CTRL_SQUARE)
				innerChoice = 1;
			else if (pspctrl.Buttons & GP2X_CTRL_CROSS)
				innerChoice = 2;
			else //if (pspctrl.Buttons & GP2X_CTRL_CIRCLE)
				innerChoice = 3;
			
			//Now grab the value out of the array
			pressed = modeChar[ mode*2 + shifted][y][x][innerChoice];
		}
		else if (pspctrl.Buttons& GP2X_CTRL_LTRIGGER) //toggle mode
		{
			dirty = true;
			mode++;
			mode %= MODE_COUNT;
		}
		else if (pspctrl.Buttons& GP2X_CTRL_SELECT)
		{
			pressed = DANZEFF_SELECT; //SELECT
		}
		else if (pspctrl.Buttons& GP2X_CTRL_START)
		{
			pressed = DANZEFF_START; //START
		}
	}

//RTRIGGER doesn't set holding
	holding = pspctrl.Buttons & ~(GP2X_CTRL_RTRIGGER|GP2X_CTRL_UP|GP2X_CTRL_DOWN|GP2X_CTRL_LEFT|GP2X_CTRL_RIGHT); 
	
	return pressed;
}

///-----------------------------------------------------------
///These are specific to the implementation, they should have the same behaviour across implementations.
///-----------------------------------------------------------


///This is the original SDL implementation
#ifdef DANZEFF_SDL

static SDL_Surface* keyBits[guiStringsSize];
static int keyBitsSize = 0;
static int moved_x = 0, moved_y = 0; // location that we are moved to

///variable needed for rendering in SDL, the screen surface to draw to, and a function to set it!
static SDL_Surface* danzeff_screen;
static SDL_Rect danzeff_screen_rect;

void 
danzeff_set_screen(SDL_Surface* screen)
{
	danzeff_screen = screen;
	danzeff_screen_rect.x = 0;
	danzeff_screen_rect.y = 0;
	danzeff_screen_rect.h = screen->h;
	danzeff_screen_rect.w = screen->w;

  moved_x = danzeff_screen->w - 150;
  moved_y = danzeff_screen->h - 150;
}


///Internal function to draw a surface internally offset
//Render the given surface at the current screen position offset by screenX, screenY
//the surface will be internally offset by offsetX,offsetY. And the size of it to be drawn will be intWidth,intHeight
void 
surface_draw_offset(SDL_Surface* pixels, int screenX, int screenY, int offsetX, int offsetY, int intWidth, int intHeight)
{
	//move the draw position
	danzeff_screen_rect.x = moved_x + screenX;
	danzeff_screen_rect.y = moved_y + screenY;

	//Set up the rectangle
	SDL_Rect pixels_rect;
	pixels_rect.x = offsetX;
	pixels_rect.y = offsetY;
	pixels_rect.w = intWidth;
	pixels_rect.h = intHeight;
	
	SDL_BlitSurface(pixels, &pixels_rect, danzeff_screen, &danzeff_screen_rect);
}

///Draw a surface at the current moved_x, moved_y
void 
surface_draw(SDL_Surface* pixels)
{
	surface_draw_offset(pixels, 0, 0, 0, 0, pixels->w, pixels->h);
}

void
danzeff_init_skin()
{
  int index;

  psp_kbd_last_skin = psp_fmgr_get_dir_list(PICS_BASEDIR, PSP_KBD_MAX_SKIN, psp_kbd_skin_dir) - 1;
 
  /* Should not happen ! */
  if (psp_kbd_last_skin < 0) {
    fprintf(stdout, "no keyboard skin in %s directory !\n",  PICS_BASEDIR);
    exit(1);
  }

  if ((psp_kbd_skin == -1) || (psp_kbd_skin > psp_kbd_last_skin)) {
    psp_kbd_skin_first = 0;
    for (psp_kbd_skin = 0; psp_kbd_skin <= psp_kbd_last_skin; psp_kbd_skin++) {
      if (!strcasecmp(psp_kbd_skin_dir[psp_kbd_skin], "default/")) break;
    }
    if (psp_kbd_skin > psp_kbd_last_skin) psp_kbd_skin = 0;
  }
}

/* load all the guibits that make up the OSK */
int 
danzeff_load()
{
  char tmp_filename[128];

	if (initialized) return 1;

  if (psp_kbd_skin_first) {
    danzeff_init_skin();
  }
	int a;
	for (a = 0; a < guiStringsSize; a++)
	{
    strcpy(tmp_filename, PICS_BASEDIR);
    strcat(tmp_filename, psp_kbd_skin_dir[psp_kbd_skin] );
    strcat(tmp_filename, guiStrings[a] );
		keyBits[a] = IMG_Load(tmp_filename);
		if (keyBits[a] == NULL)
		{
			//ERROR! out of memory.
			//free all previously created surfaces and set initialized to false
			int b;
			for (b = 0; b < a; b++)
			{
				SDL_FreeSurface(keyBits[b]);
				keyBits[b] = NULL;
			}
			initialized = false;
      fprintf(stdout, "can't load image %s\n", tmp_filename); 
			exit(1);
		}
	}
	initialized = true;
  return 1;
}

/* remove all the guibits from memory */
void 
danzeff_free()
{
	if (!initialized) return;
	
	int a;
	for (a = 0; a < guiStringsSize; a++)
	{
		SDL_FreeSurface(keyBits[a]);
		keyBits[a] = NULL;
	}
	initialized = false;
}

/* draw the keyboard at the current position */
void 
danzeff_render()
{
	dirty = false;
	
	///Draw the background for the selected keyboard either transparent or opaque
	///this is the whole background image, not including the special highlighted area
	//if center is selected then draw the whole thing opaque
	if (selected_x == 1 && selected_y == 1)
		surface_draw(keyBits[6*mode + shifted*3]);
	else
		surface_draw(keyBits[6*mode + shifted*3 + 1]);
	
	///Draw the current Highlighted Selector (orange bit)
	surface_draw_offset(keyBits[6*mode + shifted*3 + 2], 
	//Offset from the current draw position to render at
	selected_x*43, selected_y*43, 
	//internal offset of the image
	selected_x*64,selected_y*64,
	//size to render (always the same)
	64, 64);
}

/* move the position the keyboard is currently drawn at */
void 
danzeff_moveTo(const int newX, const int newY)
{
	moved_x = danzeff_screen->w - 150 + newX;
	moved_y = danzeff_screen->h - 150 + newY;
}

void
danzeff_change_skin()
{
  danzeff_free();
  danzeff_load();
}

#endif //DANZEFF_SDL
