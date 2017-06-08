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

// Blades of Exile Scenario Editor for Windoze

#include <windows.h>

#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "math.h"


#include "Global.h"
#include "graphics.h"
#include "tactions.h"
#include "tfileio.h"
#include "dlogtool.h"
#include "buttonmg.h"
#include "edsound.h"
#include "townout.h"
#include "scenario.h"
#include "keydlgs.h"
#include "graphutl.h"

void check_game_done();

void check_colors();
void cursor_go();
void cursor_stay();
Boolean verify_restore_quit(short mode);
void max_window(HWND window);
void update_item_menu();
short check_cd_event(HWND hwnd,UINT message,UINT wparam,LONG lparam);



/* Mac stuff globals */
RECT	windRect, Drag_RECT;
Boolean All_Done = FALSE;
short had_text_freeze = 0,num_fonts;
Boolean in_startup_mode = TRUE,app_started_normally = FALSE,file_in_mem = FALSE,mouse_button_held = FALSE;
Boolean play_sounds = TRUE,first_startup_update = TRUE;
Boolean diff_depth_ok = FALSE,first_sound_played = FALSE,spell_forced = FALSE,startup_loaded = FALSE;
Boolean save_maps = FALSE,first_time = FALSE,palette_suspect = FALSE,window_in_front = TRUE;
Boolean change_made = FALSE;

HWND right_sbar;

Boolean bgm_on = FALSE,bgm_init = FALSE;

Boolean gInBackground = FALSE;
long start_time;

// Shareware globals
Boolean registered = FALSE,ed_reg = FALSE;
long register_flag = 0,ed_flag = 0;
Boolean game_run_before = TRUE,save_blocked = FALSE;
short store_flags[3];

short current_active_pc = 0;
short user_given_password = -1;
short given_password;

short cur_town;
location cur_out;
short mode_count = 0;

/* Display globals */
Boolean first_update = TRUE,anim_onscreen = FALSE,frills_on = TRUE,sys_7_avail,suppress_stat_screen = FALSE;
short stat_window = 0,store_modifier;
short give_delays = 0;
Boolean dialog_not_toast = TRUE;

// storage for dialogs
short store_chosen_pc;
short store_choice_mode;
location store_choice_loc;

short town_size[3] = {64,48,24};
short which_item_page[6] = {0,0,0,0,0,0}; // Remembers which of the 2 item pages pc looked at
//short display_mode = 0; // 0 - center 1- ul 2 - ur 3 - dl 4 - dr 5 - small win
long stored_key;
short pixel_depth,dialog_answer;


// Spell casting globals
short store_mage = 0, store_priest = 0;
short store_mage_lev = 0, store_priest_lev = 0;
short store_spell_target = 6,pc_casting;
short num_targets_left = 0;
char file_path_name[256];

/* Windoze stuff globals */
Boolean cursor_shown = TRUE;

short ulx = 0, uly = 0;

HWND	mainPtr;
HWND force_dlog = NULL;
HFONT font,small_bold_font,italic_font,underline_font,bold_font,tiny_font;
HBITMAP bmap = NULL;
HPALETTE hpal;
PALETTEENTRY far ape[256];
HDC main_dc,main_dc2,main_dc3;
HANDLE store_hInstance,accel;
BOOL event_handled;

scenario_data_type far scenario;
piles_of_stuff_dumping_type *data_store;
RECT right_sbar_rect;
town_record_type far town;
big_tr_type far t_d;
outdoor_record_type far current_terrain;
talking_record_type far talking;
short overall_mode = 61;
scen_item_data_type far scen_item_list;
char far scen_strs[160][256];
char far scen_strs2[110][256];
char far talk_strs[170][256];
char far town_strs[180][256];
unsigned char border1 = 90, border2 = 90; // kludgy thing ... leave right here, before borders
unsigned char borders[4][50];
short max_dim[3] = {64,48,32};
short cen_x, cen_y;
Boolean editing_town = FALSE;
short town_type = 0;  // 0 - big 1 - ave 2 - small
short cur_viewing_mode = 0;
char szWinName[] = "Blades of Exile dialogs";
char szAppName[] = "Blades of Exile Scenario Editor";


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
	HGLOBAL temp_buffer;


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
			"Blades of Exile Scenario Editor",
			WS_OVERLAPPEDWINDOW,
			0,
			0,
			536,
			478,
			NULL,
			NULL,
			hInstance,
			NULL);


	if (!hPrevInstance) { // initialize
		//center_window(mainPtr);
 		GetModuleFileName(hInstance,file_path_name,256);
		store_hInstance = hInstance;
		accel = LoadAccelerators(hInstance, MAKEINTRESOURCE(1));

		seed = (short) GetCurrentTime();
		srand(seed);

		temp_buffer = GlobalAlloc(GMEM_FIXED,sizeof(piles_of_stuff_dumping_type));
		if (temp_buffer == NULL) {
			PostQuitMessage(0);
			}
		data_store = (piles_of_stuff_dumping_type *) (GlobalLock(temp_buffer));
		if (data_store == NULL) {
			PostQuitMessage(0);
			}

		max_window(mainPtr);
		GetWindowRect(mainPtr,&windRect);
		SetTimer(mainPtr,1,20,NULL);

		ShowWindow(mainPtr,nCmdShow);
		Set_up_win ();
		init_lb();
		init_rb();
		init_town(1);
		init_out();

		init_scenario();

		font = CreateFont(12,0,0,0,0, 0,0,0, 0,0,
			0,0,0,"MS Sans Serif");
		small_bold_font = CreateFont(12,0,0,0,700, 0,0,0, 0,0,
			0,0,0,"MS Sans Serif");
		italic_font = CreateFont(12,0,0,0,0, 1,0,0, 0,0,
			0,0,0,"MS Sans Serif");
		underline_font = CreateFont(12,0,0,0,0, 0,1,0, 0,0,
			0,0,0,"MS Sans Serif");
		bold_font = CreateFont(14,0,0,0,700, 0,0,0, 0,0,
			0,0,0,"MS Sans Serif");
		tiny_font = font;
		load_sounds();

		right_sbar_rect.top = RIGHT_AREA_UL_Y;
		right_sbar_rect.left = RIGHT_AREA_UL_X + RIGHT_AREA_WIDTH - 1 - 16;
		right_sbar_rect.bottom = RIGHT_AREA_UL_Y + RIGHT_AREA_HEIGHT;
		right_sbar_rect.right = RIGHT_AREA_UL_X + RIGHT_AREA_WIDTH - 1;
		right_sbar = CreateWindow("scrollbar",NULL,
			WS_CHILD | WS_TABSTOP | SBS_VERT, right_sbar_rect.left + ulx,right_sbar_rect.top + uly,
			right_sbar_rect.right - right_sbar_rect.left,
			right_sbar_rect.bottom - right_sbar_rect.top,
			mainPtr,1,store_hInstance,NULL);

//		ShowWindow(mainPtr,nCmdShow);

		init_screen_locs();
		set_up_start_screen();

		file_initialize();
		check_colors();
		//menu_activate(0);
		cursor_stay();
		//showcursor(TRUE);
		update_item_menu();
		shut_down_menus(0);

		cd_init_dialogs();
		//choice_dialog(0,801);

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
//RECT s_rect = {0,0,30,30},d_rect = {0,0,30,30},d2 = {0,0,420,216},s2 = {0,0,420,216};
POINT press;
short handled = 0,which_sbar,sbar_pos = 0,old_setting;
int min = 0, max = 0;
HMENU menu;
RECT dlg_rect,wind_rect;
POINT p;
RECT r;

	switch (message) {
	case WM_KEYDOWN:
		 if (hwnd != mainPtr) {
			check_cd_event(hwnd,message,wParam,lParam);
			}
			else {
				All_Done = handle_syskeystroke(wParam,lParam,&handled);
				}
		return 0;
		break;

	case WM_CHAR:
		if (hwnd != mainPtr)
			check_cd_event(hwnd,message,wParam,lParam);
			else {
				All_Done = handle_keystroke(wParam,lParam);
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

				All_Done = handle_action(press, wParam,lParam);
				check_game_done();
				}
		return 0;
		break;

	case WM_LBUTTONUP:
		mouse_button_held = FALSE;
		break;

	case WM_RBUTTONDOWN:
		mouse_button_held = FALSE;
		cursor_stay();
		if (hwnd != mainPtr)
			check_cd_event(hwnd,message,wParam,lParam);
			else {
				SetFocus(hwnd);
				press = MAKEPOINT(lParam);

				All_Done = handle_action(press, wParam,-2);
				check_game_done();
				}
		return 0;
		break;

	case WM_TIMER:
		// first, mouse held?
		if ((wParam == 1) && (mouse_button_held == TRUE)) {
			GetCursorPos(&p);
			ScreenToClient(mainPtr,&p);
			All_Done = handle_action(p, 0,0);
			}
		// second, refresh cursor?
		if ((wParam == 1) && (overall_mode < 60) && (GetFocus() == mainPtr)) {
			GetCursorPos(&p);
			ScreenToClient(mainPtr,&p);
			GetClientRect(mainPtr,&r);
			if (PtInRect(&r,p))
				restore_cursor();
			}
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
			if (((wParam == WA_ACTIVE) ||(wParam == WA_CLICKACTIVE)) &&
				(palette_suspect == TRUE)) {
				palette_suspect = FALSE;
				inflict_palette();

				}
			if ((wParam == WA_ACTIVE) ||(wParam == WA_CLICKACTIVE)) {
				window_in_front = TRUE;
				 }
			if (wParam == WA_INACTIVE) {
				window_in_front = FALSE;
				}
			}
		return 0;


	case WM_MOUSEMOVE:
		if ((mouse_button_held == TRUE) && (hwnd == mainPtr)) {
			press = MAKEPOINT(lParam);
			All_Done = handle_action(press, wParam,lParam);
			}
		//if (hwnd == mainPtr)
      	restore_cursor();
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hwnd,&ps);
		EndPaint(hwnd,&ps);
		if (hwnd != mainPtr)
			check_cd_event(hwnd,message,wParam,lParam);
			else redraw_screen();
		return 0;

	case WM_VSCROLL:
		which_sbar = GetWindowWord(HIWORD (lParam), GWW_ID);
		switch (which_sbar) {
			case 1:
				sbar_pos = GetScrollPos(right_sbar,SB_CTL);
				old_setting = sbar_pos;
				GetScrollRange(right_sbar,SB_CTL,&min,&max);
				switch (wParam ) {
					case SB_PAGEDOWN: sbar_pos += NRSONPAGE - 1; break;
					case SB_LINEDOWN: sbar_pos++; break;
					case SB_PAGEUP: sbar_pos -= NRSONPAGE - 1; break;
					case SB_LINEUP: sbar_pos--; break;
					case SB_TOP: sbar_pos = 0; break;
					case SB_BOTTOM: sbar_pos = max; break;
					case SB_THUMBPOSITION:
					case SB_THUMBTRACK:
						sbar_pos = LOWORD(lParam);
						break;
					}
				if (sbar_pos < 0)
					sbar_pos = 0;
				if (sbar_pos > max)
					sbar_pos = max;
				SetScrollPos(right_sbar,SB_CTL,sbar_pos,TRUE);
				if (sbar_pos != old_setting)
					draw_rb();
				break;
			}
	    SetFocus(mainPtr);
		return 0;

	case WM_COMMAND:
		if (hwnd == mainPtr) {

			menu = GetMenu(mainPtr);
			handle_menu((short) wParam, menu);
			check_game_done();
			}
			else {
				check_cd_event(hwnd,message,wParam,lParam);
				}
		return 0;


	case WM_DESTROY:
		if (hwnd == mainPtr) {
		discard_graphics();
		PostQuitMessage(0);
			}
		return 0;
	case WM_CLOSE:
		if (hwnd == mainPtr) {
		discard_graphics();
		PostQuitMessage(0);
			}
		return 0;
	case WM_QUIT:
      if (hwnd == mainPtr)
		discard_graphics();
		break;


	}

return DefWindowProc(hwnd,message,wParam,lParam);
}

void check_game_done()
{
  //	through_sending();
	if (All_Done == TRUE) {
		discard_graphics();
		//showcursor(TRUE);       
		PostQuitMessage(0);
		}
}

Boolean handle_menu (short item, HMENU menu)
{
	short choice,i,j;
	Boolean to_return = FALSE;
	short item_hit;

	item_hit = item;

	switch (item) {
   	case -1: break;
		case 1: // open
			load_scenario();
			if (overall_mode == 60) {
				update_item_menu();
				set_up_main_screen();
				}
			break;
		case 2: // save
			modify_lists();
			save_scenario();
			break;
		case 3: // new scen
			build_scenario();
			if (overall_mode == 60)
				set_up_main_screen();
			break;

		case 5: // quit
			if (save_check(869) == FALSE)
				break;
			discard_graphics();
			PostQuitMessage(0);
			break;



		case 101:
			if (change_made == TRUE) {
				give_error("You need to save the changes made to your scenario before you can add a new town.",
					"",0);
				return to_return;
				}
			if (scenario.num_towns >= 200) {
				give_error("You have reached the limit of 200 towns you can have in one scenario.",
					"",0);
				return to_return;
				}
			if (new_town(scenario.num_towns) == TRUE)
				set_up_main_screen();
			break;
		case 103:
			edit_scen_details();
			break;
		case 104:
			edit_scen_intro();
			break;
		case 105:
			set_starting_loc();
			break;
		case 106:
			if (check_p(user_given_password) == TRUE)
				given_password = get_password(); break;
		case 109:  SetScrollPos(right_sbar,SB_CTL,0,TRUE); start_special_editing(0,0); break;
		case 110:
			 SetScrollPos(right_sbar,SB_CTL,0,TRUE); start_string_editing(0,0);
			break;
		case 111:
			if (change_made == TRUE) {
				give_error("You need to save the changes made to your scenario before you can add a new town.",
					"",0);
				return to_return;
				}
			i = pick_import_town(841,0);
			if (i >= 0) {
				import_town(i);
				change_made = TRUE;
				redraw_screen();
				}
			break;
		case 112:
			edit_save_rects();
			break;
		case 113:
			edit_horses();
			break;
		case 114:
			edit_boats();
			break;
		case 115:
			edit_add_town();
			break;
		case 116:
			edit_scenario_events();
			break;
		case 117:
			edit_item_placement();
			break;
		case 118:
			if (change_made == TRUE) {
				give_error("You need to save the changes made to your scenario before you can delete a town.",
					"",0);
				return to_return;
				}
			if (scenario.num_towns == 1) {
				give_error("You can't delete the last town in a scenario. All scenarios must have at least 1 town.",
					"",0);
				return to_return;
				}
			if (scenario.num_towns - 1 == cur_town) {
				give_error("You can't delete the last town in a scenario while you're working on it. Load a different town, and try this again.",
					"",0);
				return to_return;
				}
			if (scenario.num_towns - 1 == scenario.which_town_start) {
				give_error("You can't delete the last town in a scenario while it's the town the party starts the scenario in. Change the parties starting point and try this again.",
					"",0);
				return to_return;
				}
			if (fancy_choice_dialog(865,0) == 1)
				delete_last_town();
			break;
		case 119:
			if (fancy_choice_dialog(866,0) == 1)
				start_data_dump();
			break;
		case 120:
			if (change_made == TRUE) {
				give_error("You need to save the changes made to your scenario before you can delete a town.",
					"",0);
				return to_return;
				}
			if (fancy_choice_dialog(871,0) == 1)
				scen_text_dump();
			redraw_screen();
			break;

		case 201: edit_town_details(); break;
		case 202: edit_town_wand(); break;
		case 203: 							overall_mode = 9;
			mode_count = 2;
			set_cursor(5);
			set_string("Set town boundary","Select upper left corner");
			break;
		case 204: frill_up_terrain(); break;
		case 205: unfrill_terrain(); break;
		case 206: edit_town_strs(); break;
		case 208: if (fancy_choice_dialog(863,0) == 2)
					break;
				place_items_in_town();
				 break; // add random
		case 209: for (i = 0; i < 64; i++)
					town.preset_items[i].property = 0;
				fancy_choice_dialog(861,0);
				draw_terrain();
				break; // set not prop
		case 210: if (fancy_choice_dialog(862,0) == 2)
					break;
				for (i = 0; i < 64; i++)
					town.preset_items[i].item_code = -1;
				draw_terrain();
				break; // clear all items
		case 213:  SetScrollPos(right_sbar,SB_CTL,0,TRUE); start_special_editing(2,0); break;
		case 214:  SetScrollPos(right_sbar,SB_CTL,0,TRUE); start_string_editing(2,0); break;
		case 215: edit_advanced_town(); break;
		case 216: edit_town_events(); break;

		case 301: outdoor_details(); break;
		case 302: edit_out_wand(0); break;
		case 303: edit_out_wand(1); break;
		case 304: frill_up_terrain(); break;
		case 305: unfrill_terrain(); break;
		case 306: edit_out_strs(); break;
		case 308: overall_mode = 47;
			set_string("Select party starting location.",""); break;
		case 311: SetScrollPos(right_sbar,SB_CTL,0,TRUE); start_special_editing(1,0); break;
		case 312: SetScrollPos(right_sbar,SB_CTL,0,TRUE);  start_string_editing(1,0); break;
		case 401: fancy_choice_dialog(986,0); break; // started
		case 402: fancy_choice_dialog(1000,0); break; // testing
		case 403: fancy_choice_dialog(1001,0); break; // distributing
		case 405: fancy_choice_dialog(1002,0); break; // contest

		case 410: // help file
			WinHelp(mainPtr,"bladedit.hlp",HELP_CONTENTS,0L);
			break;
		case 411: fancy_choice_dialog(1062,0); break;

		default:
			if ((overall_mode >= 60) || (editing_town == FALSE)) {
				give_error("You can only place items and individual monsters in town sections, not outdoors.","",0);
				break;
				}
			if ((item_hit >= 600) && (item_hit < 1000)) {
				item_hit -= 600;
				if (scen_item_list.scen_items[item_hit].variety == 0) {
					give_error("This item has its Variety set to No Item. You can only place items with a Variety set to an actual item type.","",0);
					return to_return;
					}
				overall_mode = 4;
				set_string("Place the item.","Select item location");
				mode_count = item_hit;
				}
			if ((item_hit >= 1000) && (item_hit <= 1400)) {
				overall_mode = 28;
            item_hit -= 1001;
				set_string("Place the monster.","Select monster location");
				mode_count = item_hit;
				}
			break;

		}
	if ((item >= 100) && (item < 200) && (item != 118) && (item != 119))
		change_made = TRUE;

	return to_return;
}


void check_colors()
{
	short a,b;

	a = GetDeviceCaps(main_dc,BITSPIXEL);
	b = GetDeviceCaps(main_dc,PLANES);
	if (a * b < 8) {
		MessageBox(mainPtr,"The Blades of Exile editor is designed for 256 colors. The current graphics device is set for less. Exile is playable with less colors, but will look somewhat odd."	,
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

void max_window(HWND window)
{
	RECT main_rect,wind_rect;
	short width,height;

	cursor_shown = TRUE;
	showcursor(TRUE);

	GetWindowRect(GetDesktopWindow(),&main_rect);
	GetWindowRect(window,&wind_rect);
	width = main_rect.right - main_rect.left;
	height = main_rect.bottom - main_rect.top;
	MoveWindow(window,0,0,width,height,TRUE);

}

short check_cd_event(HWND hwnd,UINT message,UINT wparam,LONG lparam)
{
	POINT press;
	Boolean action_done = FALSE;
	short wind_hit = -1,item_hit = -1;

	switch (message) {
		case WM_COMMAND:
			if ((wparam >= 150) && (wparam <= 250))  {

				if (HIWORD(lparam) == EN_ERRSPACE)
					play_sound(0);

				return 0;
				}
			cd_find_dlog(hwnd,&wind_hit,&item_hit); // item_hit is dummy
			item_hit = (short) wparam;
			break;
		case WM_KEYDOWN:
			if ((wparam >= 150) && (wparam <= 250)) {
					return -1;
				}
			wind_hit = cd_process_syskeystroke(hwnd, wparam, lparam,&item_hit);
			break;

		case WM_CHAR:
			wind_hit = cd_process_keystroke(hwnd, wparam, lparam,&item_hit);
			break;

		case WM_LBUTTONDOWN:
			press = MAKEPOINT(lparam);
			wind_hit = cd_process_click(hwnd,press, wparam, lparam,&item_hit);
			break;
		case WM_RBUTTONDOWN:
			press = MAKEPOINT(lparam);
			wparam = wparam | MK_CONTROL;
			wind_hit = cd_process_click(hwnd,press, wparam, lparam,&item_hit);
			break;
		case WM_MOUSEMOVE:
			restore_cursor();
			break;
		case WM_PAINT:
			cd_redraw(hwnd);
			break;
		}
	if (wind_hit < 0)
		return 0;
	switch (wind_hit) {
		case -1: break;
		case 970: case 971: case 972: case 973: display_strings_event_filter(item_hit); break;
		case 800: edit_make_scen_1_event_filter(item_hit); break;
		case 801: edit_make_scen_2_event_filter(item_hit); break;
		case 802: user_password_filter(item_hit); break;
		case 803: edit_scen_details_event_filter(item_hit); break;
		case 804: edit_scen_intro_event_filter(item_hit); break;
		case 805: set_starting_loc_filter(item_hit); break;
		case 806: edit_spec_item_event_filter(item_hit); break;
		case 807: edit_save_rects_event_filter(item_hit); break;
		case 808: edit_horses_event_filter(item_hit); break;
		case 809: edit_boats_event_filter(item_hit); break;
		case 810: edit_add_town_event_filter(item_hit); break;
		case 811: edit_scenario_events_event_filter(item_hit); break;
		case 812: edit_item_placement_event_filter(item_hit); break;
		case 813: edit_ter_type_event_filter(item_hit); break;
		case 814: edit_monst_type_event_filter(item_hit); break;
		case 815: edit_monst_abil_event_filter(item_hit); break;
		case 816: edit_text_event_filter(item_hit); break;
		case 817: edit_talk_node_event_filter(item_hit); break;
		case 818: edit_item_type_event_filter(item_hit); break;
		case 819: choose_graphic_event_filter(item_hit); break;
		case 820: choose_text_res_event_filter(item_hit); break;
		case 821: edit_basic_dlog_event_filter(item_hit); break;
		case 822: edit_spec_enc_event_filter(item_hit); break;
		case 823: give_password_filter(item_hit); break;
		case 824: edit_item_abil_event_filter(item_hit); break;
		case 825: edit_special_num_event_filter(item_hit); break;
		case 826: edit_spec_text_event_filter(item_hit); break;
		case 830: new_town_event_filter(item_hit); break;
		case 831: edit_sign_event_filter(item_hit); break;
		case 832: edit_town_details_event_filter(item_hit); break;
		case 833: edit_town_events_event_filter(item_hit); break;
		case 834: edit_advanced_town_event_filter(item_hit); break;
		case 835: edit_town_wand_event_filter(item_hit); break;
		case 836: edit_placed_item_event_filter(item_hit); break;
		case 837: edit_placed_monst_event_filter(item_hit); break;
		case 838: edit_placed_monst_adv_event_filter(item_hit); break;
		case 839: edit_town_strs_event_filter(item_hit); break;
		case 840: edit_area_rect_event_filter(item_hit); break;
		case 841: pick_import_town_event_filter(item_hit); break;
		case 842: edit_dialog_text_event_filter(item_hit); break;
		case 850: edit_out_strs_event_filter(item_hit); break;
		case 851: outdoor_details_event_filter(item_hit); break;
		case 852: edit_out_wand_event_filter(item_hit); break;
		case 854: pick_out_event_filter(item_hit); break;
		case 855: case 856: pick_town_num_event_filter(item_hit); break;
		case 857: change_ter_event_filter(item_hit); break;
		default: fancy_choice_dialog_event_filter (item_hit); break;
		}
	return 0;
}