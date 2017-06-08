/*
Blades of Exile Game/Scenario Editor/Character Editor
    Copyright (C) 1998-2007, Jeff Vogel
    http://www.spiderwebsoftware.com, spidweb@spiderwebsoftware.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

// Blades of Exile for Windows
// Will this horror never end?

#include <windows.h>

#include "stdlib.h"
#include "math.h"

#include "global.h"
#include "graphics.h"
#include "newgraph.h"
#include "fileio.h"
#include "actions.h"
#include "text.h"
#include "party.h"
#include "exlsound.h"
#include "items.h"
#include "fields.h"
#include "town.h"
#include "dlogtool.h"
#include "startup.h"
#include "dlgutils.h"
#include "exlsound.h"
#include "infodlgs.h"
#include "graphutl.h"

void check_game_done();
Boolean handle_menu (short item, HMENU menu);
void load_cursors();
void set_cursor(HCURSOR which_curs);
void change_cursor(POINT where_curs);
void check_colors();
void cursor_go();
void cursor_stay();

/* Mac stuff globals */
Boolean All_Done = FALSE,dialog_not_toast = FALSE;
short had_text_freeze = 0,num_fonts;
Boolean in_startup_mode = TRUE,app_started_normally = FALSE;
Boolean play_sounds = TRUE,first_startup_update = TRUE;
Boolean diff_depth_ok = FALSE,first_sound_played = FALSE,spell_forced = FALSE,startup_loaded = FALSE;
Boolean save_maps = TRUE,party_in_memory = FALSE,in_scen_debug = FALSE;
HWND text_sbar = NULL,item_sbar = NULL,shop_sbar = NULL;
RECT sbar_rect = {545,284,562,421};
RECT shop_sbar_rect = {258,67,274,357};
RECT item_sbar_rect = {545,146,562,254}; /**/
short dialog_answer;
POINT store_anim_ul;

Boolean gInBackground = FALSE;
long start_time;
Boolean allowed_one_erase = FALSE;

Boolean palette_suspect = FALSE,window_in_front = FALSE;

short on_spell_menu[2][62];
short on_monst_menu[256];
Boolean belt_present = FALSE;

// Cursors 
short current_cursor = 120;
HCURSOR arrow_curs[3][3], sword_curs, key_curs, target_curs,talk_curs,look_curs;


// Shareware globals
Boolean registered = TRUE,ed_reg = FALSE;
long register_flag = 0;
long ed_flag = 0,ed_key;
Boolean game_run_before = TRUE;

Boolean debug_on = FALSE,give_intro_hint = TRUE;

/* Adventure globals */
party_record_type far party;
pc_record_type far adven[6];
outdoor_record_type far outdoors[2][2];
current_town_type far c_town;
big_tr_type far t_d;
town_item_list far t_i;
unsigned char far out[96][96];
unsigned char far out_e[96][96];
setup_save_type far setup_save;
unsigned char far misc_i[64][64],sfx[64][64];
location monster_targs[T_M];

/* Display globals */
extern short combat_posing_monster = -1, current_working_monster = -1; // 0-5 PC 100 + x - monster x
Boolean fast_bang = FALSE;
short spec_item_array[60];
short overall_mode = 45,current_spell_range;
Boolean first_update = TRUE,anim_onscreen = FALSE,frills_on = TRUE,sys_7_avail,suppress_stat_screen = FALSE;
short stat_window = 0,store_modifier;
Boolean monsters_going = FALSE,boom_anim_active = FALSE,cartoon_happening = FALSE;
short give_delays = 0;
Boolean modeless_exists[18] = {FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,
								FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,
								FALSE,FALSE,FALSE,FALSE,FALSE,FALSE};
short modeless_key[18] = {1079,1080,1081,1082,1084, 1046,1088,1089,1090,1092, 1095,1072,0,0,0,0,0,0};
HWND modeless_dialogs[18] = {NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	
								NULL,	NULL,	NULL,	NULL,	NULL,	NULL,
								NULL,	NULL,	NULL,	NULL,	NULL,	NULL};

short town_size[3] = {64,48,32};
short which_item_page[6] = {0,0,0,0,0,0}; // Remembers which of the 2 item pages pc looked at
short ulx = 0, uly = 0;
short display_mode = 0; // 0 - center 1- ul 2 - ur 3 - dl 4 - dr 5 - small win
long stored_key;
short pixel_depth,old_depth = 8;
short current_ground = 0,stat_screen_mode = 0;
short anim_step = -1;
long anim_ticks = 0;

// Spell casting globals
short store_mage = 0, store_priest = 0;
short store_mage_lev = 0, store_priest_lev = 0;
short store_spell_target = 6,pc_casting;
short pc_last_cast[2][6] = {{1,1,1,1,1,1},{1,1,1,1,1,1}};
short num_targets_left = 0;
location spell_targets[8];

 long store_mouse;

/* Combat globals */
short which_combat_type,town_type;
location center;
unsigned char far combat_terrain[64][64];
location pc_pos[6];
short current_pc;
short combat_active_pc;
effect_pat_type current_pat;
short monst_target[T_M]; // 0-5 target that pc   6 - no target  100 + x - target monster x
short spell_caster, missile_firer,current_monst_tactic;
short store_current_pc = 0;


stored_items_list_type far stored_items[3];
stored_outdoor_maps_type far o_maps;

// Special stuff booleans
Boolean web,crate,barrel,fire_barrier,force_barrier,quickfire,force_wall,fire_wall,antimagic,scloud,ice_wall,blade_wall;
Boolean sleep_field;

long last_anim_time = 0,last_redraw_time;

/* Windoze stuff globals */
Boolean cursor_shown = TRUE;
short store_pc_being_created;


HWND	mainPtr;
HWND force_dlog = NULL;
HFONT font,fantasy_font,small_bold_font,italic_font,underline_font,bold_font,tiny_font;
FARPROC modeless_dlogprocs[18] = {NULL,	NULL,	NULL,	NULL,	NULL,	NULL,
								NULL,	NULL,	NULL,	NULL,	NULL,	NULL,
								NULL,	NULL,	NULL,	NULL,	NULL,	NULL};
HBITMAP bmap = NULL;
HPALETTE hpal;
PALETTEENTRY far ape[256];
HDC main_dc,main_dc2,main_dc3;
HANDLE store_hInstance,accel;
BOOL event_handled;
scenario_data_type far scenario;
piles_of_stuff_dumping_type *data_store;
piles_of_stuff_dumping_type2 *data_store2;
piles_of_stuff_dumping_type3 *data_store3;
piles_of_stuff_dumping_type4 *data_store4;
piles_of_stuff_dumping_type5 *data_store5;
talking_record_type far talking;
char far scen_strs2[110][256];
stored_town_maps_type far town_maps,town_maps2;

char szWinName[] = "Blades of Exile Dialogs";
char szAppName[] = "Blades of Exile";
char file_path_name[256];

Boolean block_erase = FALSE;

long FAR PASCAL _export WndProc (HWND, UINT, UINT, LONG);
Boolean handle_menu (short, HMENU);

int PASCAL WinMain (hInstance,hPrevInstance,
  lpszCmdParam, nCmdShow)
HANDLE hInstance;
HANDLE hPrevInstance;
LPSTR lpszCmdParam;
int nCmdShow;
{

	MSG msg;
	WNDCLASS wndclass,wndclass2;
	short i,seed;
   RECT windRECT;
	HGLOBAL temp_buffer,temp_buffer2;
	HGLOBAL temp_buffer3,temp_buffer4;
	HGLOBAL temp_buffer5,temp_buffer6;

	if (!hPrevInstance) {
		wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNWINDOW;
		wndclass.lpfnWndProc = WndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = hInstance;
		wndclass.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(10));
		wndclass.hCursor = NULL;
		wndclass.hbrBackground = GetStockObject(WHITE_BRUSH);
		wndclass.lpszMenuName = MAKEINTRESOURCE(1);
		wndclass.lpszClassName = szAppName;
		
		RegisterClass(&wndclass);
	
		wndclass2.style = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNWINDOW;
		wndclass2.lpfnWndProc = WndProc;
		wndclass2.cbClsExtra = 0;
		wndclass2.cbWndExtra = 0;
		wndclass2.hInstance = hInstance;
		wndclass2.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(10));
		wndclass2.hCursor = NULL;
		wndclass2.hbrBackground = GetStockObject(WHITE_BRUSH);
		wndclass2.lpszMenuName = NULL;
		wndclass2.lpszClassName = szWinName;
		
		RegisterClass(&wndclass2);
		}

		mainPtr = CreateWindow (szAppName,
			"Blades of Exile",
			WS_OVERLAPPEDWINDOW,
			0,
			0,
			588,
			478,
			NULL,
			NULL,
			hInstance,
			NULL);

	if (!hPrevInstance) { // initialize
		//center_window(mainPtr);
		GetModuleFileName(hInstance,file_path_name,256);
		//ASB(file_path_name);

		store_hInstance = hInstance;
		accel = LoadAccelerators(hInstance, MAKEINTRESOURCE(1));
		init_buf();
		load_cursors();

		temp_buffer = GlobalAlloc(GMEM_FIXED,sizeof(piles_of_stuff_dumping_type));
		if (temp_buffer == NULL) {
			PostQuitMessage(0);
			}
		data_store = (piles_of_stuff_dumping_type *) (GlobalLock(temp_buffer));
		if (data_store == NULL) {
			PostQuitMessage(0);
			}
		temp_buffer2 = GlobalAlloc(GMEM_FIXED,sizeof(piles_of_stuff_dumping_type2));
		if (temp_buffer2 == NULL) {
			PostQuitMessage(0);
			}
		data_store2 = (piles_of_stuff_dumping_type2 *) (GlobalLock(temp_buffer2));
		if (data_store2 == NULL) {
			PostQuitMessage(0);
			}
		temp_buffer3 = GlobalAlloc(GMEM_FIXED,sizeof(piles_of_stuff_dumping_type3));
		if (temp_buffer3 == NULL) {
			PostQuitMessage(0);
			}
		data_store3 = (piles_of_stuff_dumping_type3 *) (GlobalLock(temp_buffer3));
		if (data_store3 == NULL) {
			PostQuitMessage(0);
			}
		temp_buffer4 = GlobalAlloc(GMEM_FIXED,sizeof(piles_of_stuff_dumping_type4));
		if (temp_buffer4 == NULL) {
			PostQuitMessage(0);
			}
		data_store4 = (piles_of_stuff_dumping_type4 *) (GlobalLock(temp_buffer4));
		if (data_store4 == NULL) {
			PostQuitMessage(0);
			}
		temp_buffer5 = GlobalAlloc(GMEM_FIXED,sizeof(piles_of_stuff_dumping_type5));
		if (temp_buffer5 == NULL) {
			PostQuitMessage(0);
			}
		data_store5 = (piles_of_stuff_dumping_type5 *) (GlobalLock(temp_buffer5));
		if (data_store5 == NULL) {
			PostQuitMessage(0);
			}

		last_redraw_time = seed = (short) GetCurrentTime();
		srand(seed);

		get_reg_data();

		if (display_mode != 5)
			max_window(mainPtr);
			else {
					GetWindowRect(GetDesktopWindow(),&windRECT);
					MoveWindow(mainPtr,(windRECT.right - (588 + 10)) / 2,
					(windRECT.bottom - (425 + 52)) / 2 ,
					588 + 10,425 + 52,TRUE);
				}


		load_sounds();

		font = CreateFont(12,0,0,0,0, 0,0,0, 0,0,
			0,0,0,"MS Sans Serif");
		small_bold_font = CreateFont(12,0,0,0,700, 0,0,0, 0,0,
			0,0,0,"MS Sans Serif");
		italic_font = CreateFont(12,0,0,0,700, 1,0,0, 0,0,
			0,0,0,"MS Sans Serif");
		underline_font = CreateFont(12,0,0,0,0, 0,1,0, 0,0,
			0,0,0,"MS Sans Serif");
		bold_font = CreateFont(14,0,0,0,700, 0,0,0, 0,0,
			0,0,0,"MS Sans Serif");
		tiny_font = font;
		fantasy_font = CreateFont(22,0,0,0,0, 0,0,0, 0,0,
			0,0,0,"MaidenWord");
		if (fantasy_font == NULL) {
			fantasy_font = CreateFont(22,0,0,0,0, 0,0,0, 0,0,
				0,0,0,"MS Sans Serif");
			}

//Rect sbar_rect = {546,283,562,421};
//Rect shop_sbar_rect = {258,67,274,357};
//Rect item_sbar_rect = {546,146,562,254}; /**/
      /*
		text_sbar = CreateWindow("scrollbar",NULL,
			WS_CHILD | WS_TABSTOP | SBS_VERT, sbar_rect.left,sbar_rect.top,sbar_rect.right,sbar_rect.bottom,
			mainPtr,1,store_hInstance,NULL);
		SetScrollRange(text_sbar,SB_CTL,0,58,FALSE);
		item_sbar = CreateWindow("scrollbar",NULL,
			WS_CHILD | WS_TABSTOP | SBS_VERT, item_sbar_rect.left,item_sbar_rect.top,item_sbar_rect.right,item_sbar_rect.bottom,
			mainPtr,2,store_hInstance,NULL);
		SetScrollRange(item_sbar,SB_CTL,0,16,FALSE);
		*/
		shop_sbar = CreateWindow("scrollbar",NULL,
			WS_CHILD | WS_TABSTOP | SBS_VERT, shop_sbar_rect.left,shop_sbar_rect.top,shop_sbar_rect.right,shop_sbar_rect.bottom,
			mainPtr,3,store_hInstance,NULL);
		SetScrollRange(shop_sbar,SB_CTL,0,16,FALSE);

		ShowWindow(mainPtr,nCmdShow);

		plop_fancy_startup();

		init_screen_locs();

		fancy_startup_delay();

		SetTimer(mainPtr,1,620,NULL);
		SetTimer(mainPtr,2,200,NULL);
		//SetTimer(mainPtr,3,10,NULL);
		file_initialize();
		check_colors();
		menu_activate(0);
		cursor_stay();
		showcursor(TRUE);
		reset_text_bar();

		adjust_window_mode();

		cd_init_dialogs();

		if (load_blades_data() == FALSE)
			registered = FALSE;

			if (game_run_before == FALSE)
				FCD(986,0);
				else if (give_intro_hint == TRUE)
			 		tip_of_day();
			}

		event_handled = FALSE;
		while(GetMessage(&msg,NULL,0,0)) {
			if (event_handled == FALSE) {
				if (!TranslateAccelerator(mainPtr, accel, &msg)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					}
				}
			}
		return msg.wParam;
}

long FAR PASCAL _export WndProc (hwnd,message,wParam,lParam)
HWND hwnd;
UINT message;
UINT wParam;
LONG lParam;
{
HDC hdc;
PAINTSTRUCT ps;
RECT rect;
short cxDib, cyDib;
BYTE huge * lpDibBits;
RECT s_rect = {0,0,30,30},d_rect = {0,0,30,30},d2 = {0,0,420,216},s2 = {0,0,420,216};
POINT press;
short handled = 0;
HMENU menu;
RECT dlg_rect,wind_rect,draw_rect;
short store_ulx,store_uly,sbar_pos;
short which_sbar;

short old_setting,new_setting;
	short smin,smax;

// First, handle window size
	GetWindowRect(mainPtr,&wind_rect);
	switch (display_mode) {
		case 1:
			store_ulx = 0;  store_uly = 0; break;
		case 2: store_ulx = wind_rect.right - 550 - 21; store_uly = 0;	break;
		case 3: store_ulx = 0; store_uly = wind_rect.bottom - 422 - 24 - 21; break;
		case 4: store_ulx = wind_rect.right - 550 - 21; store_uly = wind_rect.bottom - 422 - 24 - 21; break;
		case 5: store_ulx = 0; store_uly = 0;
		default:
			store_ulx = ((wind_rect.right - wind_rect.left) - 536) / 2 - 30;
			store_uly = ((wind_rect.bottom - wind_rect.top) - 478) / 2 + 8;
			break;
		}
	store_ulx = store_ulx - (store_ulx % 8);
	store_uly = store_uly - (store_uly % 8);
	if ((store_ulx != ulx) || (store_uly != uly)) {
		ulx = store_ulx;
		uly = store_uly;
		adjust_window_mode();
		}

	switch (message) {
	/*case WM_SIZE: case WM_MOVE:
		if (hwnd == mainPtr) {
			print_nums((message == WM_SIZE) ? 500 : 0,0,0);
			GetClientRect(mainPtr,&draw_rect);
			InvalidateRect(mainPtr,&draw_rect,TRUE);
			}
		break;*/
	case WM_KEYDOWN:
		if (hwnd != mainPtr)
			check_cd_event(hwnd,message,wParam,lParam);
			else {
				cursor_go();
				All_Done = handle_syskeystroke(wParam,lParam,&handled);
				check_game_done();
				}
		if (handled == 1)
			FlushEvents(0);
		return 0;
		break;

	case WM_CHAR:
		if (hwnd != mainPtr)
			check_cd_event(hwnd,message,wParam,lParam);
			else {
				cursor_go();
				All_Done = handle_keystroke(wParam,lParam);
				check_game_done();
				}
		return 0;
		break;

	case WM_LBUTTONDOWN:
		cursor_stay();
		if (hwnd != mainPtr)
			check_cd_event(hwnd,message,wParam,lParam);
			else {
				SetFocus(hwnd);
				press = MAKEPOINT(lParam);
				if (in_startup_mode == FALSE)
					All_Done = handle_action(press, wParam,lParam);
					else All_Done = handle_startup_press(press);
				change_cursor(press);
				check_game_done();
				}
		return 0;
		break;

	case WM_RBUTTONDOWN:
		cursor_stay();
		if (hwnd != mainPtr)
			check_cd_event(hwnd,message,wParam,lParam);
			else {
				SetFocus(hwnd);
				press = MAKEPOINT(lParam);
				if (in_startup_mode == FALSE)
					All_Done = handle_action(press, wParam,-2);
					else All_Done = handle_startup_press(press);
				change_cursor(press);
				check_game_done();
				}
		return 0;
		break;

	case WM_PALETTECHANGED:
		 palette_suspect = TRUE;
		 return 0;

	case WM_ACTIVATEAPP:
		if ((hwnd == mainPtr) && (wParam == 0)) {
			reset_palette();
			}
		if ((hwnd == mainPtr) && (wParam != 0)) {
			inflict_palette();
			}
		break;
	case WM_ACTIVATE:
		if (hwnd == mainPtr) {
			if ((wParam == WA_ACTIVE) ||(wParam == WA_CLICKACTIVE)) {
				if (palette_suspect == TRUE) {
					inflict_palette();
					palette_suspect = FALSE;
					GetClientRect(hwnd,&dlg_rect);
					InvalidateRect(hwnd,&dlg_rect,FALSE);
					}
				}
			if ((wParam == WA_ACTIVE) ||(wParam == WA_CLICKACTIVE)) {
				window_in_front = TRUE;
				GetClientRect(hwnd,&dlg_rect);
				 InvalidateRect(hwnd,&dlg_rect,FALSE);
				 }
			if (wParam == WA_INACTIVE) {
				cursor_stay();
				anim_onscreen = FALSE;
				window_in_front = FALSE;
				}
			}
		return 0;

	case WM_ERASEBKGND:
		if (hwnd != mainPtr)
			break;
		if (block_erase == TRUE) {
			block_erase = FALSE;
			return 1;
			}
		//if (allowed_one_erase == TRUE)
			return 1;
		allowed_one_erase = TRUE;
		break;
	case WM_MOUSEMOVE:
		if (store_mouse != lParam)
			cursor_stay();
		store_mouse = lParam;
		press = MAKEPOINT(lParam);
		if (hwnd == mainPtr) {
			change_cursor(press);
			if ((overall_mode > 10) && (overall_mode < 20) && (party.stuff_done[181][6] == 0))
				draw_targeting_line(press);
			}
		return 0;

	case WM_PAINT:
		if ((block_erase == TRUE) && (hwnd == mainPtr)) {
			block_erase = FALSE;
			}
		//if (hwnd == mainPtr) {
		//	GetClientRect(mainPtr,&draw_rect);
		//	ValidateRect(mainPtr,&draw_rect);
		//	}
		hdc = BeginPaint(hwnd,&ps);
		EndPaint(hwnd,&ps);

		if (hwnd != mainPtr)
			check_cd_event(hwnd,message,wParam,lParam);
			else {
				force_reprint();
				reset_text_bar();
				if (in_startup_mode == TRUE) {
					draw_startup(0);
					}
				else if (first_update == TRUE) {
					first_update = FALSE;
					if (overall_mode == 0)
						redraw_screen(0);
					if ((overall_mode > 0) & (overall_mode < 10))
						redraw_screen(1);
				// 1st update never combat
				}
			else refresh_screen(overall_mode);
			}


		return 0;

	case WM_TIMER:
		switch(wParam) {
			case 1:
				if ((in_startup_mode == FALSE) && (anim_onscreen == TRUE)
				 && (window_in_front == TRUE) && (give_delays == FALSE)) {
					//if (GetCurrentTime() - last_redraw_time > 600) {
						anim_ticks++;
						initiate_redraw();
					//	}
					}
				break;
			case 2:
				if ((overall_mode > 10) && (overall_mode < 20) && (party.stuff_done[181][6] == 0)) {
					GetCursorPos(&press);
					press.x -= wind_rect.left + 3;
					press.y -= wind_rect.top + 41;
					draw_targeting_line(press);
					}
				if ((in_startup_mode == TRUE) && (window_in_front == TRUE)) {
					draw_startup_anim();
					}
				break;
			}
		return 0;

	case WM_COMMAND:    // 1006 is dummy event
		// creation events 2000 spend xp 2001 pick graphic 2002 finish up
		if (hwnd == mainPtr) {
			menu = GetMenu(mainPtr);
			All_Done = handle_menu((short) wParam, menu);
			check_game_done();
			}
			else {
				check_cd_event(hwnd,message,wParam,lParam);
				}
		return 0;

	case WM_VSCROLL:
		which_sbar = GetWindowWord(HIWORD (lParam), GWW_ID);
		switch (which_sbar) {// 1 text  2 items  3 shop
			case 1:
				sbar_pos = GetScrollPos(text_sbar,SB_CTL);
				switch (wParam ) {
					case SB_PAGEDOWN: sbar_pos += 11; break;
					case SB_LINEDOWN: sbar_pos++; break;
					case SB_PAGEUP: sbar_pos -= 11; break;
					case SB_LINEUP: sbar_pos--; break;
					case SB_TOP: sbar_pos = 0; break;
					case SB_BOTTOM: sbar_pos = 57; break;
					case SB_THUMBPOSITION:
					case SB_THUMBTRACK:
						sbar_pos = LOWORD(lParam);
						break;
					}
				if (sbar_pos < 0)
					sbar_pos = 0;
				if (sbar_pos > 58)
					sbar_pos = 58;
				SetScrollPos(text_sbar,SB_CTL,sbar_pos,TRUE);
				force_reprint();
				print_buf();
			break;
			case 2:
				old_setting = sbar_pos = GetScrollPos(item_sbar,SB_CTL);
				GetScrollRange(item_sbar,SB_CTL,(int *) &smin,(int *) &smax);
				switch (wParam ) {
					case SB_PAGEDOWN: sbar_pos += (stat_window == 7) ? 2 : 8; break;
					case SB_LINEDOWN: sbar_pos++; break;
					case SB_PAGEUP: sbar_pos -= (stat_window == 7) ? 2 : 8; break;
					case SB_LINEUP: sbar_pos--; break;
					case SB_TOP: sbar_pos = 0; break;
					case SB_BOTTOM: sbar_pos = smax; break;
					case SB_THUMBPOSITION:
					case SB_THUMBTRACK:
						sbar_pos = LOWORD(lParam);
						break;
					}
				if (sbar_pos < 0)
					sbar_pos = 0;
				if (sbar_pos > smax)
		      	sbar_pos = smax;
				SetScrollPos(item_sbar,SB_CTL,sbar_pos,TRUE);
				if (sbar_pos != old_setting)
					put_item_screen(stat_window,1);
			break;
			case 3:
				old_setting = sbar_pos = GetScrollPos(shop_sbar,SB_CTL);
				GetScrollRange(shop_sbar,SB_CTL,(int *) &smin,(int *) &smax);
				switch (wParam ) {
					case SB_PAGEDOWN: sbar_pos += 8; break;
					case SB_LINEDOWN: sbar_pos++; break;
					case SB_PAGEUP: sbar_pos -= 8; break;
					case SB_LINEUP: sbar_pos--; break;
					case SB_TOP: sbar_pos = 0; break;
					case SB_BOTTOM: sbar_pos = smax; break;
					case SB_THUMBPOSITION:
					case SB_THUMBTRACK:
						sbar_pos = LOWORD(lParam);
						break;
					}
				if (sbar_pos < 0)
					sbar_pos = 0;
				if (sbar_pos > smax)
		   	   		sbar_pos = smax;
				SetScrollPos(shop_sbar,SB_CTL,sbar_pos,TRUE);
				if (sbar_pos != old_setting)
					draw_shop_graphics(0,shop_sbar_rect);			
			break;
			}
	    SetFocus(mainPtr);
		return 0;

	case WM_DESTROY:
		if (hwnd == mainPtr) {
			lose_graphics();
			PostQuitMessage(0);
			}
		return 0;
	case WM_CLOSE:
		if (hwnd == mainPtr) {
			lose_graphics();
			PostQuitMessage(0);
			}
		return 0;
	case WM_QUIT:
		if (hwnd == mainPtr)
			lose_graphics();
		break;


	}

return DefWindowProc(hwnd,message,wParam,lParam);
}

void check_game_done()
{
	through_sending();
	if (All_Done == TRUE) {
		lose_graphics();
		showcursor(TRUE);
		PostQuitMessage(0);
		}
}

Boolean handle_menu (short item, HMENU menu)
{
	short choice,i;
	POINT x = {1001,0},pass_point;
	Boolean to_return = FALSE;

	switch (item) {
		case 1:
			if (in_startup_mode == TRUE)
				startup_load();
				else do_load();
			break;
		case 2:
			do_save(0);
			break;
		case 3:
			if (in_startup_mode == TRUE)
				save_file(1);
				else do_save(1);
			break;
		case 4:
			if (in_startup_mode == FALSE) {
				choice = FCD(1091,0);
				if (choice == 1)
					return FALSE;
				for (i = 0; i < 6; i++)
					adven[i].main_status = 0;
				party_in_memory = FALSE;
				reload_startup();
				in_startup_mode = TRUE;
				draw_startup(0);
				}
			start_new_game();
			update_pc_graphics();
			draw_startup(0);
			break;
		case 6:
			pick_preferences();
			break;
		case 8:

			if (in_startup_mode == TRUE) {
				to_return = All_Done = TRUE;
				break;
				}
			if (overall_mode > 1) {
				choice = FCD(1067,0);
				if (choice == 1)
					return All_Done;
				}
				else {
					choice = FCD(1066,0);
					if (choice == 3)
						break;
					if (choice == 1)
						save_file(0);
					}
			to_return = All_Done = TRUE;
			break;

		// options menu
		case 21:
			choice = char_select_pc(0,0,"New graphic for who?");
			if (choice < 6)
				pick_pc_graphic(choice,1,0);
			update_pc_graphics();
			initiate_redraw();
			break;
			
		case 24:
			if (prime_time() == FALSE) {
				ASB("Finish what you're doing first.");
				print_buf();
				}
				else {
					choice = char_select_pc(0,0,"Delete who?");
					if (choice < 6) {
						if ((i = FCD(1053,0)) == 2)
							kill_pc(choice,0);
						}
					update_pc_graphics();
					initiate_redraw();
					}
			break;
			
			
		case 22:
			choice = select_pc(0,0);
			if (choice < 6)
				pick_pc_name(choice,0);
			put_pc_screen();
			put_item_screen(stat_window,0);
			break;
			
			
		case 23:
			if (!(is_town())) {
				add_string_to_buf("Add PC: Town mode only.");
				print_buf();
				break;
				}
			for (i = 0; i < 6; i++)
				if (adven[i].main_status == 0)
					i = 20;
			if (i == 6) {
				ASB("Add PC: You already have 6 PCs.");
				print_buf();
				}
			if (c_town.town_num == scenario.which_town_start) {
				give_help(56,0,0);
				create_pc(6,0);
				}
				else {
					add_string_to_buf("Add PC: You can only make new");
					add_string_to_buf("  characters in the town you ");
					add_string_to_buf("  started in.");
					}
			print_buf();
			put_pc_screen();
			put_item_screen(stat_window,0);
			break;	
		case 27:	
			if (overall_mode == 20) {
				ASB("Talking notes: Can't read while talking.");
				print_buf();
				return to_return;
				}
			talk_notes();
			break;
		case 28:
			adventure_notes();
			break;
		case 29:
			if (in_startup_mode == FALSE)
				print_party_stats();
			break;

		// help menu
		case 41: FCD(1079,0);
				break;
		case 42: FCD(1080,0); break;
		case 43: FCD(1081,0); break;
		case 44: FCD(1072,0); break; // magic barriers
		case 46: FCD(1084,0); break;
		case 47: FCD(1088,0); break;

		// library
		case 61: display_spells(0,100,0);
				break;
		case 62: display_spells(1,100,0);
				break;
		case 63: display_skills(100,0);
			    break;
		case 64:
				display_help(0,0);
				break;
		case 65: tip_of_day(); break;
		case 67: FCD(986,0); break;

		// actions
		case 81:
			if (overall_mode != 1) {
				ASB("Alchemy: In town mode only.");
				print_buf();
				break;
            }
				pass_point.x = 1000;
				pass_point.y = 405;
			to_return = handle_action(pass_point,(UINT) 0,(LONG)-1);
			break;
		case 82:
			//handle_keystroke('w',0,event);
			to_return = handle_action(x,(UINT) 0,(LONG)-1);
			break;
		case 84:
			if (prime_time() == FALSE) {
				ASB("Finish what you're doing first.");
				print_buf();
				}
				else {
					give_help(62,0,0);
					display_map();
					}
			make_cursor_sword();
			break;

		// mage is 399
		case 399: give_help(209,0,0);
			party.help_received[9] = FALSE; break;

		// priest is 499
		case 499: give_help(209,0,0); 
			party.help_received[9] = FALSE; break;
		
		// monsters is 599
		case 599: give_help(212,0,0); break;
		
		case 100: // index
			WinHelp(mainPtr,"bladhelp.hlp",HELP_CONTENTS,0L);
			break;
		case 200: // about
			FCD(1062,0);
			break;

		default:
			if ((item >= 400) && (item < 500))  { // mage spell
				if (prime_time() == FALSE) {
					ASB("Finish what you're doing first.");
					print_buf();
					}
					else handle_menu_spell(item - 400,0);
				break;
				}
			if ((item >= 500) && (item < 600))  { // priest spell
				if (prime_time() == FALSE) {
					ASB("Finish what you're doing first.");
					print_buf();
					}
					else handle_menu_spell(item - 500,1);
				break;
				}
			if ((item >= 600) && (item < 700))  { // monster spell
				display_monst(item - 600,(creature_data_type *) NULL,1);
				break;
				}
			break;
		}
	if (in_startup_mode == TRUE)
		menu_activate(0);
		else menu_activate(1);
	return to_return;
}

void load_cursors()
{
	short i,j;

	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			arrow_curs[i][j] = LoadCursor(store_hInstance,MAKEINTRESOURCE(100 + (i - 1) + 10 * (j - 1)));
	sword_curs = LoadCursor(store_hInstance,MAKEINTRESOURCE(120));

	key_curs = LoadCursor(store_hInstance,MAKEINTRESOURCE(122));
	talk_curs = LoadCursor(store_hInstance,MAKEINTRESOURCE(126));
	target_curs = LoadCursor(store_hInstance,MAKEINTRESOURCE(124));
	look_curs = LoadCursor(store_hInstance,MAKEINTRESOURCE(129));

	set_cursor(sword_curs);
	current_cursor = 124;

}

void set_cursor(HCURSOR which_curs)
{
	SetCursor (which_curs);
}

void change_cursor(POINT where_curs)
{
	short curs_types[50] = {0,0,126,124,122,122,0,0,0,0,
					0,124,124,124,124,122,0,0,0,0,
					120,120,0,0,0,0,0,0,0,0,
					0,0,0,0,0,129,129,129,0,0,
					0,0,0,0,0,120,0,0,0,0},cursor_needed;
	location cursor_direction;
	RECT world_screen = {23, 23, 274, 346};

	where_curs.x -= ulx;
	where_curs.y -= uly;

	if (PtInRect(&world_screen,where_curs) == FALSE)
		cursor_needed = 120;
		else cursor_needed = curs_types[overall_mode];

	if (in_startup_mode == TRUE)
		cursor_needed = 120;
		else if (cursor_needed == 0) {
		if ((PtInRect(&world_screen,where_curs) == FALSE) || (in_startup_mode == TRUE))
			cursor_needed = 120;
			else {
				cursor_direction = get_cur_direction(where_curs);
				cursor_needed = 100 + (cursor_direction.x) + 10 * ( cursor_direction.y);
				if (cursor_needed == 100)
					cursor_needed = 120;
				}
		}
	current_cursor = cursor_needed;

	//cursor_needed = 120;
   //play_sound(0);

	switch (cursor_needed) {
		case 120:
			set_cursor(sword_curs);
			break;
		case 121:

		case 122:
			set_cursor(key_curs);
			break;
		case 124:
			set_cursor(target_curs);
			break;
		case 126:
			set_cursor(talk_curs);
			break;
		case 129:
			set_cursor(look_curs);
			break;

		default:  // an arrow
			set_cursor(arrow_curs[cursor_direction.x + 1][cursor_direction.y + 1]);
			break;
		}
		
}

void check_colors()
{
	short a,b;

	a = GetDeviceCaps(main_dc,BITSPIXEL);
	b = GetDeviceCaps(main_dc,PLANES);
	if (a * b < 8) {
		MessageBox(mainPtr,"Exile III is designed for 256 colors. The current graphics device is set for less. Exile 3 is playable with less colors, but will look somewhat odd."	,
	  "Not 256 colors!",MB_OK | MB_ICONEXCLAMATION);
		MessageBox(mainPtr,"For tips on how to get 256 colors, hit F1 for help, and then select 'Getting 256 Colors' from the table of contents."	,
	  "Not 256 colors!",MB_OK | MB_ICONEXCLAMATION);
	  }
}

void cursor_go()
{
	if (in_startup_mode == TRUE)
		return;
	if (cursor_shown == TRUE) {
		cursor_shown = FALSE;
		showcursor(FALSE);
		}
}

void cursor_stay()
{
	if ((cursor_shown == FALSE) || (in_startup_mode == TRUE)) {
		cursor_shown = TRUE;
		showcursor(TRUE);
      }
}
