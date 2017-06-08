#define  LINES_IN_TEXT_WIN	11
#define	TEXT_BUF_LEN	70

#include <windows.h>

#include "global.h"
#include "stdio.h"
#include "text.h"
#include "string.h"
#include "locutils.h"
#include "fields.h"
#include "exlsound.h"
#include "graphutl.h"
	

char far *m_mage_sp[] = {"Spark","Minor Haste","Strength","Flame Cloud","Flame",
						"Minor Poison","Slow","Dumbfound","Stinking Cloud","Summon Beast",
						"Conflagration","Fireball","Weak Summoning","Web","Poison",
						"Ice Bolt","Slow Group","Major Haste","Firestorm","Summoning",
						"Shockstorm","Major Poison","Kill","Daemon","Major Blessing",
						"Major Summoning","Shockwave"};
char far *m_priest_sp[] = {"Minor Bless","Light Heal","Wrack","Stumble","Bless",
						"Curse","Wound","Summon Spirit","Disease","Heal",
						"Holy Scourge","Smite","Curse All","Sticks to Snakes","Martyr's Shield",
						"Bless All","Major Heal","Flamestrike","Summon Host","Heal All",
						"Unholy Ravaging","Summon Guardian","Pestilence","Revive All","Avatar",
						"Divine Thud"};

typedef struct {
	char line[50];
	} buf_line;

buf_line far text_buffer[TEXT_BUF_LEN];
short buf_pointer = 30, lines_to_print= 0, num_added_since_stop = 0;
char far store_string[256];
char far store_string2[256];
short start_print_point= 0;
short mark_where_printing_long;
Boolean printing_long = FALSE;
char far c_str[256] = "";
Boolean save_mess_given = FALSE;

RECT status_panel_clip_rect = {299, 11,495,175},item_panel_clip_rect = {297,11,463,175};

RECT far item_buttons_from[7] = {{0,12,14,24},{14,12,28,24},{28,12,42,24},{42,12,56,24},
						{0,24,30,36},{30,24,60,36},{0,36,30,48}}; /**/

short store_mode;
Boolean string_added = FALSE;
short store_text_x = 0, store_text_y = 0;
extern short had_text_freeze,stat_screen_mode;

void InsetRect (RECT *rect, short x, short y);

// graphics globals
extern RECT status_panel_rect,status_panel_title_rect;
extern RECT	text_panel_rect;
extern short overall_mode,which_combat_type,stat_window;
extern HWND mainPtr;
extern RECT more_info_button;
extern short which_item_page[6],current_cursor;
extern HCURSOR sword_curs;
extern HWND text_sbar,item_sbar;
extern POINT store_anim_ul;
extern HBRUSH bg[14];
extern short dest_personalities[40];
extern location source_locs[6];
extern location dest_locs[40] ;

extern HBITMAP mixed_gworld, pc_stats_gworld, item_stats_gworld, text_area_gworld,tiny_obj_gworld,party_template_gworld;
extern short terrain_there[9][9];
extern HBITMAP dialog_pattern_gworld,pattern_gworld,status_pattern_gworld,spec_scen_g;

// game globals
extern party_record_type far party;
extern piles_of_stuff_dumping_type *data_store;
extern piles_of_stuff_dumping_type2 *data_store2;
extern talking_record_type far talking;
extern scenario_data_type far  scenario;

extern current_town_type far c_town;
extern outdoor_record_type far outdoors[2][2];
extern town_item_list far 	t_i;
extern unsigned char far out[96][96];
extern pc_record_type far adven[6];
extern big_tr_type far  t_d;
extern short ulx,uly;
extern Boolean play_sounds,suppress_stat_screen,cartoon_happening;
extern RECT item_buttons[8][6];
 // name, use, give, drip, info, sell/id
extern RECT pc_buttons[6][5];
 // name, hp, sp, info, trade
extern Boolean item_area_button_active[8][6];
extern Boolean pc_area_button_active[6][5];
extern RECT item_screen_button_rects[9];
extern short spec_item_array[60];
extern short abil_chart[200],store_anim_type;
// combat globals
extern short item_bottom_button_active[9];

extern location pc_pos[6];
extern unsigned char far combat_terrain[64][64];
extern short current_pc;
extern short shop_identify_cost;
extern short store_selling_values[8];
extern short combat_posing_monster, current_working_monster; // 0-5 PC 100 + x - monster x
extern Boolean supressing_some_spaces;
extern location ok_space[4];

extern HFONT font,italic_font,underline_font,bold_font,small_bold_font;
extern HPALETTE hpal;
extern HDC main_dc;
extern HANDLE store_hInstance;
extern piles_of_stuff_dumping_type5 *data_store5;

short current_item_button[6] = {-1,-1,-1,-1,-1,-1};
short pc_button_state[6] = {-1,-1,-1,-1,-1,-1};

short text_pc_has_abil_equip(short pc_num,short abil)
{
	short i = 0;

	while (((adven[pc_num].items[i].variety == 0) || (adven[pc_num].items[i].ability != abil)
			|| (adven[pc_num].equip[i] == FALSE)) && (i < 24))
				i++;
	return i;

}

// Draws the pc area in upper right
//void win_draw_string(HWND dest_window,RECT dest_rect,char *str,short mode,short line_height)
void put_pc_screen()
{
	char to_draw[256];
	short dummy,i_num;
	short i = 0,j,pc,weap1 = 16, weap2 = 16,skill_item,hit_adj,dam_adj;
	RECT erase_rect = {2,17,270,99},to_draw_rect,from_rect;
	RECT final_from_draw_rect = {0,0,271,116},final_to_draw_rect = {0,0,271,116};
	RECT small_erase_rects[3] = {{34,101,76,114},{106,101,147,114},{174,101,201,114}};
	Boolean has_type;
	RECT pc_screen_help_button = {251,101,266,113};
	RECT info_from = {1,0,13,12},switch_from = {12,0,25,12}; /**/

	HDC hdc;
	COLORREF colors[6] = {RGB(0,0,0),RGB(255,0,0),RGB(128,0,0),RGB(0,160,0),RGB(0,0,255),RGB(255,255,255)};
	UINT c[6];
	HBITMAP store_bmp;
	Boolean right_buttons_same = TRUE;

	for (i = 0; i < 6; i++)
		if (((adven[i].main_status != 0) && (pc_button_state[i] != 1)) ||
			((adven[i].main_status == 0) && (pc_button_state[i] != 0)))
				right_buttons_same = FALSE;

	// First refresh gworld with the pc info
	//rect_draw_some_item (orig_pc_info_screen_gworld, erase_rect, pc_info_screen_gworld, erase_rect, 0, 0);
	// First clean up gworld with pretty patterns
	// Will rewrite later
	//FillCRECT(&erase_rect,bg[6]);
	erase_rect.right -= 13;
	//if (right_buttons_same == TRUE)
	//	erase_rect.right -= 15;
	from_rect = erase_rect;
	OffsetRect(&from_rect,-1 * from_rect.left,-1 * from_rect.top);
	rect_draw_some_item(status_pattern_gworld,from_rect,
		pc_stats_gworld,erase_rect,0,0);
	//if (right_buttons_same == FALSE) {
		erase_rect.left = erase_rect.right;
		erase_rect.right += 13;
		from_rect = erase_rect;
		OffsetRect(&from_rect,-1 * from_rect.left,-1 * from_rect.top);
		rect_draw_some_item(status_pattern_gworld,from_rect,
			pc_stats_gworld,erase_rect,0,0);
	//	}
	for (i = 0; i < 3; i++) {
		//	FillCRECT(&small_erase_rects[i],bg[7]);
		from_rect = small_erase_rects[i];
		OffsetRect(&from_rect,-1 * from_rect.left + 208,-1 * from_rect.top + 136);
		rect_draw_some_item(mixed_gworld,from_rect,
			pc_stats_gworld,small_erase_rects[i],0,0);
		}

	hdc = CreateCompatibleDC(main_dc);
	//store_text_hdc = hdc;
	SelectPalette(hdc,hpal,0);
	SetBkMode(hdc,TRANSPARENT);
	SelectObject(hdc,small_bold_font);

	for (i = 0; i < 6; i++)
		c[i] = GetNearestPaletteIndex(hpal,colors[i]);
	store_bmp = SelectObject(hdc,pc_stats_gworld);


	// Put food, gold, day
	SetTextColor(hdc,PALETTEINDEX(c[5]));
	sprintf((char *) to_draw, "%d", (short) party.gold);
	win_draw_string(hdc,small_erase_rects[1],
	  to_draw,0,10);
	sprintf((char *) to_draw, "%d", (short) party.food);
	win_draw_string(hdc,small_erase_rects[0],
	  to_draw,0,10);
	i = calc_day();
	sprintf((char *) to_draw, "%d", i);
	win_draw_string(hdc,small_erase_rects[2],
	  to_draw,0,10);
	SetTextColor(hdc,PALETTEINDEX(c[0]));

	for (i = 0; i < 6; i++) {
		if (adven[i].main_status != 0) {
			for (j = 0; j < 5; j++)
				pc_area_button_active[i][j] = 1;
			if (i == current_pc) {
				//TextFace(italic | bold);
				//ForeColor(blueColor);
				SelectObject(hdc,italic_font);
				SetTextColor(hdc,PALETTEINDEX(c[4]));
				}

			sprintf((char *) to_draw, "%d. %-20s             ", i + 1, (char *) adven[i].name);
			win_draw_string(hdc,pc_buttons[i][0],
			 to_draw,0,10);
			//TextFace(bold);
			//ForeColor(blackColor);
			SelectObject(hdc,small_bold_font);
			SetTextColor(hdc,PALETTEINDEX(c[0]));

			to_draw_rect = pc_buttons[i][1];      
			to_draw_rect.right += 20;
			switch (adven[i].main_status) {
				case 1:
					if (adven[i].cur_health == adven[i].max_health)
						SetTextColor(hdc,PALETTEINDEX(c[3]));
						else SetTextColor(hdc,PALETTEINDEX(c[1]));
					sprintf((char *) to_draw, "%-3d              ",adven[i].cur_health);
					win_draw_string(hdc,pc_buttons[i][1],
					  to_draw,0,10);
					if (adven[i].cur_sp == adven[i].max_sp)
						SetTextColor(hdc,PALETTEINDEX(c[4]));
						else SetTextColor(hdc,PALETTEINDEX(c[2]));
					sprintf((char *) to_draw, "%-3d              ",adven[i].cur_sp);
					win_draw_string(hdc,pc_buttons[i][2],
					  to_draw,0,10);
					SetTextColor(hdc,PALETTEINDEX(c[0]));
					SelectObject(hdc,store_bmp);
					draw_pc_effects(i,NULL);
					SelectObject(hdc,pc_stats_gworld);
					break;
				case 2:
					sprintf((char *) to_draw, "Dead");
					break;
				case 3:
					sprintf((char *) to_draw, "Dust");
					break;
				case 4:
					sprintf((char *) to_draw, "Stone");
					break;
				case 5:
					sprintf((char *) to_draw, "Fled");
					break;
				case 6:
					sprintf((char *) to_draw, "Surface");
					break;
				case 7:
					sprintf((char *) to_draw, "Won");
					break;
				default:
					sprintf((char *) to_draw, "Absent");
					break;
				}
			if (adven[i].main_status != 1)
				win_draw_string(hdc,to_draw_rect,
				 to_draw,0,10);

			// Now put trade and info buttons
			//rect_draw_some_item(mixed_gworld,info_from,pc_stats_gworld,pc_buttons[i][3],1,0);
			//rect_draw_some_item(mixed_gworld,switch_from,pc_stats_gworld,pc_buttons[i][4],1,0);
			// do faster!
			to_draw_rect = pc_buttons[i][3];
			to_draw_rect.right = pc_buttons[i][4].right + 1;
			from_rect = info_from;
			from_rect.right = from_rect.left + to_draw_rect.right - to_draw_rect.left;

				pc_button_state[i] = 1;
				SelectObject(hdc,store_bmp);
				rect_draw_some_item(mixed_gworld,from_rect,pc_stats_gworld,to_draw_rect,1,0);
				SelectObject(hdc,pc_stats_gworld);
				
			}
			else {
				for (j = 0; j < 5; j++)
					pc_area_button_active[i][j] = 0;
				pc_button_state[i] = 0;
				}
		}

	// Now put text on window.
	SelectObject(hdc,store_bmp);
	DeleteObject(hdc);

	// Now put text on window.
	OffsetRect(&final_to_draw_rect,PC_WIN_UL_X,PC_WIN_UL_Y);
	rect_draw_some_item (pc_stats_gworld, final_from_draw_rect, pc_stats_gworld, final_to_draw_rect, 0, 1);

	// Sometimes this gets called when character is slain. when that happens, if items for
	// that PC are up, switch item page.
	if ((current_pc < 6) && (adven[current_pc].main_status != 1) && (stat_window == current_pc)) {
		set_stat_window(current_pc);
		}
}

// Draws item area in middle right
// Screen_num is what page is visible in the item menu.
//    0 - 5 pc inventory  6 - special item  7 - missions
// Stat_screen_mode ... 0 - normal, adventuring, all buttons vis
//						1 - in shop, item info only
//						2 - in shop, identifying, shop_identify_cost is cost
//						3 - in shop, selling weapons
//						4 - in shop, selling armor
//						5 - in shop, selling all
//						6 - in shop, augmenting weapon,shop_identify_cost is type
void put_item_screen(short screen_num,short suppress_buttons)
// if suppress_buttons > 0, save time by not redrawing buttons
{
	char pstats[256] = "Party stats:";
	char to_draw[256];
	short dummy,i_num,item_offset;
	short i = 0,j,pc;
	RECT erase_rect = {2,17,255,123},dest_rect,from_rect,to_rect;
	RECT upper_frame_rect = {3,3,268,16};
	Boolean has_type;
	RECT parts_of_area_to_draw[3] = {{0,0,271,17},{0,16,256,123},{0,123,271,144}}; /**/

	HDC hdc;
	COLORREF colors[6] = {RGB(0,0,0),RGB(255,0,0),RGB(128,0,0),RGB(0,160,0),RGB(0,0,255),RGB(255,255,255)};
	UINT c[6];
	HBITMAP store_bmp;

	// First refresh gworld with the pc info
	//rect_draw_some_item (orig_pc_info_screen_gworld, erase_rect, pc_info_screen_gworld, erase_rect, 0, 0);
	// First clean up gworld with pretty patterns
	// Will rewrite later
	//FillCRECT(&erase_rect,bg[6]);
	from_rect = erase_rect;
	OffsetRect(&from_rect,-1 * from_rect.left,-1 * from_rect.top);
	rect_draw_some_item(status_pattern_gworld,from_rect,
		item_stats_gworld,erase_rect,0,0);
	if (suppress_buttons == 0)
		for (i = 0; i < 6; i++)
			if ((adven[i].main_status != 1) && (current_item_button[i] != -1)) {
			current_item_button[i] = -1;
			//FillCRECT(&item_screen_button_rects[i],bg[7]);
			from_rect = item_screen_button_rects[i];
			OffsetRect(&from_rect,-1 * from_rect.left + 208,-1 * from_rect.top + 136);
			rect_draw_some_item(mixed_gworld,from_rect,
				item_stats_gworld,item_screen_button_rects[i],0,0);
			}
	//FillCRECT(&upper_frame_rect,bg[7]);
	to_rect = upper_frame_rect;
	to_rect.right = to_rect.left + 96;
	from_rect = to_rect;
	OffsetRect(&from_rect,-1 * from_rect.left + 208,-1 * from_rect.top + 136);
	rect_draw_some_item(mixed_gworld,from_rect,
		item_stats_gworld,to_rect,0,0);
	OffsetRect(&to_rect,96,0);
	from_rect = to_rect;
	OffsetRect(&from_rect,-1 * from_rect.left + 208,-1 * from_rect.top + 136);
	rect_draw_some_item(mixed_gworld,from_rect,
		item_stats_gworld,to_rect,0,0);


	hdc = CreateCompatibleDC(main_dc);
	//store_text_hdc = hdc;
	SelectPalette(hdc,hpal,0);
	SetBkMode(hdc,TRANSPARENT);
	SelectObject(hdc,small_bold_font);

	for (i = 0; i < 6; i++)
		c[i] = GetNearestPaletteIndex(hpal,colors[i]);


	store_bmp = SelectObject(hdc,item_stats_gworld);

	// Draw buttons at bottom
	if (suppress_buttons == 0) {
		//for (i = 0; i < 6; i++)
		//	FillCRECT(&item_screen_button_rects[i],bg[7]);
		}

	item_offset = GetScrollPos(item_sbar,SB_CTL);
	SetTextColor(hdc,PALETTEINDEX(c[5]));

	for (i = 0; i < 8; i++)
		for (j = 0; j < 6; j++)
			item_area_button_active[i][j] = FALSE;

	switch (screen_num) {
		case 6: // On special items page
			//TextFace(bold);
			SelectObject(hdc,bold_font);
			sprintf((char *) to_draw, "Special items:");
			win_draw_string(hdc,upper_frame_rect,
				to_draw,0,10);
		 	SetTextColor(hdc,PALETTEINDEX(c[0]));
			for (i = 0; i < 8; i++) {
				i_num = i + item_offset;
				if (spec_item_array[i_num] >= 0){
					strcpy((char *) to_draw,data_store5->scen_strs[60 + spec_item_array[i_num] * 2]);
					win_draw_string(hdc,item_buttons[i][0],to_draw,0,10);

					SelectObject(hdc,store_bmp);
					place_item_button(3,i,4,0);
					if ((scenario.special_items[spec_item_array[i_num]] % 10 == 1)
						&& (!(is_combat())))
							place_item_button(0,i,3,0);
					SelectObject(hdc,item_stats_gworld);
					}
				}
			break;
		case 7: // On jobs page
			break;

		default: // on an items page
			pc = screen_num;
			sprintf((char *) to_draw, "%s inventory:",
				(char *) adven[pc].name);
			win_draw_string(hdc,upper_frame_rect,
			  to_draw,0,10);

		 	SetTextColor(hdc,PALETTEINDEX(c[0]));
			for (i = 0; i < 8; i++) {
				i_num = i + item_offset;
				sprintf((char *) to_draw, "%d.",i_num + 1);
				win_draw_string(hdc,item_buttons[i][0],
				  to_draw,0,10);

				dest_rect = item_buttons[i][0];
				dest_rect.left += 36;

				if (adven[pc].items[i_num].variety == 0) {

					}
					else {
						if (adven[pc].equip[i_num] == TRUE) {
							//TextFace(italic | bold);
							SelectObject(hdc,italic_font);
							if (adven[pc].items[i_num].variety < 3)
								SetTextColor(hdc,PALETTEINDEX(c[1]));
								else if ((adven[pc].items[i_num].variety >= 12) && (adven[pc].items[i_num].variety <= 17))
									SetTextColor(hdc,PALETTEINDEX(c[3]));
									else SetTextColor(hdc,PALETTEINDEX(c[4]));
							}
							else SetTextColor(hdc,PALETTEINDEX(c[0]));
					// Place object graphic here
					//if (adven[pc].items[i_num].variety != 0)
					//	draw_obj_graphic(i + ((which_item_page[pc] == 1) ? 1 : 0),adven[pc].items[i_num].graphic_num,text_panel_rect); // rect is space holder

							if (is_ident(adven[pc].items[i_num]) == 0)
								sprintf((char *) to_draw, "%s  ",adven[pc].items[i_num].name);
								else { /// Don't place # of charges when Sell button up and space tight
									if ((adven[pc].items[i_num].charges > 0) && (adven[pc].items[i_num].type != 2)
										&& (stat_screen_mode <= 1))
										sprintf((char *) to_draw, "%s (%d)",adven[pc].items[i_num].full_name,adven[pc].items[i_num].charges);
										else sprintf((char *) to_draw, "%s",adven[pc].items[i_num].full_name);
									}
						dest_rect.left -= 2;
						win_draw_string(hdc,dest_rect,to_draw,0,10);
						//TextFace(0);
						//TextFace(bold);
						//ForeColor(blackColor);
						SelectObject(hdc,small_bold_font);
						SetTextColor(hdc,PALETTEINDEX(c[0]));

						// this is kludgy, awkwark, and has redundant code. Done this way to
						// make go faster, and I got lazy.
					SelectObject(hdc,store_bmp);
						if ((stat_screen_mode == 0) &&
						 ((is_town()) || (is_out()) || ((is_combat()) && (pc == current_pc)))) { // place give and drop and use
							place_item_button(0,i,0,adven[pc].items[i_num].graphic_num); // item_graphic
							if (abil_chart[adven[pc].items[i_num].ability] != 4) // place use if can
								place_item_button(10,i,1,0);
								else place_item_button(11,i,1,0);
							}
							else {
								place_item_button(0,i,0,adven[pc].items[i_num].graphic_num); // item_graphic
								place_item_button(3,i,4,0); // info button
								if ((stat_screen_mode == 0) &&
								 ((is_town()) || (is_out()) || ((is_combat()) && (pc == current_pc)))) { // place give and drop and use
									place_item_button(1,i,2,0);
									place_item_button(2,i,3,0);
									if (abil_chart[adven[pc].items[i_num].ability] != 4) // place use if can
										place_item_button(0,i,1,0);
									}
								}
						if (stat_screen_mode > 1) {
							place_buy_button(i,pc,i_num,hdc);

							}
					SelectObject(hdc,item_stats_gworld);

						} // end of if item is there
					} // end of for (i = 0; i < 8; i++)
			break;
			}

	// Now put text on window.
	SelectObject(hdc,store_bmp);
	DeleteObject(hdc);

	place_item_bottom_buttons();

	// Now put text on window.
	for (i = 0; i < 3; i++) {
		dest_rect = parts_of_area_to_draw[i];
		OffsetRect(&dest_rect,ITEM_WIN_UL_X,ITEM_WIN_UL_Y);
		rect_draw_some_item (item_stats_gworld, parts_of_area_to_draw[i],
		  item_stats_gworld, dest_rect, 0, 1);
		}
}

void place_buy_button(short position,short pc_num,short item_num,HDC hdc)
{
	RECT dest_rect,source_rect;
	RECT button_sources[3] = {{0,24,30,36},{30,24,60,36},{0,36,30,48}}; /**/
	short val_to_place;
	short aug_cost[10] = {4,7,10,8, 15,15,10, 0,0,0};
	HBITMAP store_bmp;

	if (adven[pc_num].items[item_num].variety == 0)
		return;

	dest_rect = item_buttons[position][5];

	val_to_place = (adven[pc_num].items[item_num].charges > 0) ?
		adven[pc_num].items[item_num].charges * adven[pc_num].items[item_num].value :
		adven[pc_num].items[item_num].value;
	val_to_place = val_to_place / 2;

	switch (stat_screen_mode) {
		case 2:
			if (is_ident(adven[pc_num].items[item_num]) == FALSE) { 
				item_area_button_active[position][5] = TRUE;
				source_rect = button_sources[0];
				val_to_place = shop_identify_cost;
				}
			break;
		case 3: // sell weapons 
			if (((adven[pc_num].items[item_num].variety < 7) || (adven[pc_num].items[item_num].variety == 23) ||
				(adven[pc_num].equip[item_num] == FALSE) &&
				(adven[pc_num].items[item_num].variety == 24)) &&
				(is_ident(adven[pc_num].items[item_num]) == TRUE) && (val_to_place > 0) &&
				 (is_cursed(adven[pc_num].items[item_num]) == FALSE)) { 
				item_area_button_active[position][5] = TRUE;
				source_rect = button_sources[1];
				}
			break;
		case 4: // sell armor
			if ((adven[pc_num].items[item_num].variety >= 12) && (adven[pc_num].items[item_num].variety <= 17) &&
				(adven[pc_num].equip[item_num] == FALSE) &&
				(is_ident(adven[pc_num].items[item_num]) == TRUE) && (val_to_place > 0) &&
				 (is_cursed(adven[pc_num].items[item_num]) == FALSE)) { 
				item_area_button_active[position][5] = TRUE;
				source_rect = button_sources[1];
				}
			break;
		case 5: // sell any
			if ((val_to_place > 0) && (is_ident(adven[pc_num].items[item_num]) == TRUE) && 
				(adven[pc_num].equip[item_num] == FALSE) &&
				 (is_cursed(adven[pc_num].items[item_num]) == FALSE)) { 
				item_area_button_active[position][5] = TRUE;
				source_rect = button_sources[1];
				}
			break;
		case 6: // augment weapons 
			if ((adven[pc_num].items[item_num].variety < 3) &&
				(is_ident(adven[pc_num].items[item_num]) == TRUE) &&
				(adven[pc_num].items[item_num].ability == 0) &&
				(is_magic(adven[pc_num].items[item_num]) == FALSE)) { 
				item_area_button_active[position][5] = TRUE;
				source_rect = button_sources[2];
				val_to_place = max(aug_cost[shop_identify_cost] * 100,adven[pc_num].items[item_num].value * (5 + aug_cost[shop_identify_cost]));
				}
			break;
		}
	if (item_area_button_active[position][5] == TRUE) {
		store_selling_values[position] = val_to_place;
		dest_rect = item_buttons[position][5];
		dest_rect.right = dest_rect.left + 30;
		rect_draw_some_item (mixed_gworld, source_rect,
		  item_stats_gworld, dest_rect, 1, 0);
		sprintf((char *) store_string,"        %d",val_to_place);
		//if (val_to_place >= 10000)
		//	TextFace(0);
		store_bmp = SelectObject(hdc,item_stats_gworld);
		char_win_draw_string(hdc,item_buttons[position][5],
		  store_string,2,10);
		SelectObject(hdc,store_bmp);
		//TextFace(bold);
		}
}

//extern Boolean item_area_button_active[8][6];
 // name, use, give, drop, info, sell/id
// shortcuts - if which_button_to_put is 10, all 4 buttons now
//				if which_button_to_put is 11, just right 2
void place_item_button(short which_button_to_put,short which_slot,short which_button_position,short extra_val)
{
	RECT from_rect = {0,0,18,18},to_rect;

	if (which_button_position == 0) { // this means put little item graphic, extra val is which_graphic
		item_area_button_active[which_slot][which_button_position] = TRUE;
		OffsetRect(&from_rect,(extra_val % 10) * 18,(extra_val / 10) * 18);
		to_rect = item_buttons[which_slot][0];
		to_rect.right = to_rect.left + (to_rect.bottom - to_rect.top);
		InsetRect(&to_rect,-1,-1);
		OffsetRect(&to_rect,20,1);
		InsetRect(&from_rect,2,2);
		if (extra_val >= 150) {
			from_rect = get_custom_rect(extra_val - 150);
			rect_draw_some_item (spec_scen_g, from_rect,
			  item_stats_gworld, to_rect, 0, 0);
			}
			else {
				rect_draw_some_item (tiny_obj_gworld, from_rect,
			  item_stats_gworld, to_rect, 1, 0);
				}
		 return;
		}
	if (which_button_to_put < 4) { // this means put a regular item button
		item_area_button_active[which_slot][which_button_position] = TRUE;
		rect_draw_some_item (mixed_gworld, item_buttons_from[which_button_to_put],
		  item_stats_gworld, item_buttons[which_slot][which_button_position], 1, 0);
		}
	if (which_button_to_put == 10) { // this means put all 4
		item_area_button_active[which_slot][1] = TRUE;
		item_area_button_active[which_slot][2] = TRUE;
		item_area_button_active[which_slot][3] = TRUE;
		item_area_button_active[which_slot][4] = TRUE;
		from_rect = item_buttons_from[0]; from_rect.right = item_buttons_from[3].right;
		to_rect = item_buttons[which_slot][1];
		to_rect.right = to_rect.left + from_rect.right - from_rect.left;
		rect_draw_some_item (mixed_gworld, from_rect,
		  item_stats_gworld, to_rect, 1, 0);
		}
	if (which_button_to_put == 11) { // this means put right 3
		item_area_button_active[which_slot][2] = TRUE;
		item_area_button_active[which_slot][3] = TRUE;
		item_area_button_active[which_slot][4] = TRUE;
		from_rect = item_buttons_from[1]; from_rect.right = item_buttons_from[3].right;
		to_rect = item_buttons[which_slot][2];
		to_rect.right = to_rect.left + from_rect.right - from_rect.left;
		rect_draw_some_item (mixed_gworld, from_rect,
		  item_stats_gworld, to_rect, 1, 0);
		}
}

RECT get_custom_rect (short which_rect) ////
{
	RECT store_rect = {0,0,28,36};

	OffsetRect(&store_rect,28 * (which_rect % 10),36 * (which_rect / 10));
	return store_rect;
}

void place_item_bottom_buttons()
{
	RECT pc_from_rect = {0,0,28,36},but_from_rect = {36,85,54,101},to_rect; /**/
	short i;

	for (i = 0; i < 6; i++) {
		if (adven[i].main_status == 1) {
			item_bottom_button_active[i] = TRUE;
			to_rect = item_screen_button_rects[i];
			//if (current_item_button[i] != adven[i].which_graphic) {
				current_item_button[i] = adven[i].which_graphic;
				rect_draw_some_item (mixed_gworld, but_from_rect, item_stats_gworld, to_rect, 0, 0);
				pc_from_rect = get_party_template_rect(i,0);
				InsetRect(&to_rect,2,2);
				rect_draw_some_item (party_template_gworld, pc_from_rect, item_stats_gworld, to_rect, 0, 0);
			//	}

			}
			else item_bottom_button_active[i] = FALSE;
		}
}

RECT get_party_template_rect(short pc_num,short mode)
{
	RECT source_rect;

	source_rect.top = (pc_num % 3) * BITMAP_HEIGHT;
	source_rect.bottom = 36 + (pc_num % 3) * BITMAP_HEIGHT;
	source_rect.left = (pc_num / 3) * BITMAP_WIDTH * 2 + ((mode == 1) ? 28 : 0);
	source_rect.right = source_rect.left + BITMAP_WIDTH;

	return source_rect;
}


void set_stat_window(short new_stat)
{
	short i,array_pos = 0;
	
	stat_window = new_stat;
	if ((stat_window < 6) && (adven[stat_window].main_status != 1))
		stat_window = first_active_pc();
	switch (stat_window) {
		case 6:
			for (i = 0; i < 60; i++)
				spec_item_array[i] = -1;
			for (i = 0; i < 50; i++) ////
				if (party.spec_items[i] > 0) {
					spec_item_array[array_pos] = i;	
					array_pos++;
					}
			array_pos = max(0,array_pos - 8);
			//SetControlMaximum(item_sbar,array_pos);
			SetScrollRange(item_sbar,SB_CTL,0,array_pos,FALSE);
			break;
		case 7:
			SetScrollRange(item_sbar,SB_CTL,0,2,FALSE);
			break;
		default:
			SetScrollRange(item_sbar,SB_CTL,0,16,FALSE);
			break;
		}
	//SetControlValue(item_sbar,0);
	SetScrollPos(item_sbar,SB_CTL,0,TRUE);
	put_item_screen(stat_window,0);

}

short first_active_pc()
{
	short i = 0;
	
	for (i = 0; i < 6; i++)
		if (adven[i].main_status == 1)
			return i;
	return 6;
}


void refresh_stat_areas(short mode)
{
	short i,x;
	RECT dest_rect,parts_of_area_to_draw[3] = {{0,0,271,17},{0,16,256,123},{0,123,271,144}};/**/
	RECT pc_stats_from =  {0,0,271,116},text_area_from = {0,0,256,138};

	x = mode * 10;
	dest_rect = pc_stats_from;
	OffsetRect(&dest_rect,PC_WIN_UL_X,PC_WIN_UL_Y);
	rect_draw_some_item (pc_stats_gworld, pc_stats_from, pc_stats_gworld, dest_rect, x, 1);
	for (i = 0; i < 3; i++) {
		dest_rect = parts_of_area_to_draw[i];
		OffsetRect(&dest_rect,ITEM_WIN_UL_X,ITEM_WIN_UL_Y);
		rect_draw_some_item (item_stats_gworld, parts_of_area_to_draw[i], 
		  item_stats_gworld, dest_rect, x, 1);
		}
	dest_rect = text_area_from;
	OffsetRect(&dest_rect,TEXT_WIN_UL_X,TEXT_WIN_UL_Y);
	rect_draw_some_item (text_area_gworld, text_area_from, text_area_gworld, dest_rect, x, 1);

}


short total_encumberance(short pc_num)
{
	short store = 0,i,what_val;
	
	for (i = 0; i < 24; i++)
		if (adven[pc_num].equip[i] == TRUE) {
			what_val = adven[pc_num].items[i].awkward;
			store += what_val;
			}
	return store;
}

short get_tnl(pc_record_type *pc)
{
	short tnl = 100,i,store_per = 100;
	short rp[3] = {0,12,20};
	short ap[15] = {10,20,8,10,4, 6,10,7,12,15, -10,-8,-8,-20,-8};
	
	tnl = (tnl * (100 + rp[pc->race])) / 100;
	for (i = 0; i < 15; i++)
		if (pc->traits[i] == TRUE) 
			store_per = store_per + ap[i];

	tnl = (tnl * store_per) / 100;	
	
	return tnl;
}



void draw_pc_effects(short pc,HDC dest_dc)
//short pc; // 10 + x -> draw for pc x, but on spell dialog  
{
	RECT source_rects[18] = {{55,0,67,12},{55,12,67,24},{55,24,67,36},
							{67,0,79,12},{67,12,79,24},{67,24,79,36},
							{79,0,91,12},{79,12,91,24},{79,24,91,36},
							{91,0,103,12},{91,12,103,24},{91,24,103,36},
							{103,0,115,12},{103,12,115,24},{103,24,115,36},
							{115,0,127,12},{115,12,127,24},{115,24,127,36}};
	RECT dest_rect = {18,15,30,27},dlog_dest_rect = {66,354,78,366};//rects altered below
	Boolean on_dialog = FALSE;
	short right_limit = 250;
	short dest = 0; // 0 - in gworld  2 - on dialog
	short name_width,i,mode = 1;
	HBITMAP dest_bmp;

	for (i = 0; i < 18; i++)
		alter_rect(&source_rects[i]);
	alter_rect(&dest_rect);
	alter_rect(&dlog_dest_rect);
	
	if (pc >= 10) {
		pc -= 10;
		right_limit = 490;
		dest_rect = dlog_dest_rect;
		dest = 2;
      mode = 0;
		dest_rect.top += pc * 24 + 18;
		dest_rect.bottom += pc * 24 + 18;
		dest_bmp = (HBITMAP) dest_dc;
		}
		else {
			name_width = string_length(adven[pc].name,main_dc);
			right_limit = pc_buttons[0][1].left - 5;
			//dest_rect.left = pc_buttons[i][1].left - 16;
			dest_rect.left = name_width + 33;
			dest_rect.right = dest_rect.left + 12;
			dest_rect.top += pc * 13;
			dest_rect.bottom += pc * 13;
			dest_bmp = pc_stats_gworld;
			}
			
	if (adven[pc].main_status % 10 != 1)
		return;
			
	if ((adven[pc].status[0] > 0) && (dest_rect.right < right_limit)) { 
		rect_draw_some_item (mixed_gworld,source_rects[4],dest_bmp,dest_rect,mode,dest);
		dest_rect.left += 13;
		dest_rect.right += 13;
		}
	if (adven[pc].status[1] > 0) { 
		rect_draw_some_item (mixed_gworld,source_rects[2],dest_bmp,dest_rect,mode,dest);
		dest_rect.left += 13;
		dest_rect.right += 13;
		}
	if (adven[pc].status[1] < 0) { 
		rect_draw_some_item (mixed_gworld,source_rects[3],dest_bmp,dest_rect,mode,dest);
		dest_rect.left += 13;
		dest_rect.right += 13;
		}
	if (adven[pc].status[2] > 0) { 
		rect_draw_some_item (mixed_gworld,source_rects[(adven[pc].status[2] > 4) ? 1 : 0],dest_bmp,dest_rect,mode,dest);
		dest_rect.left += 13;
		dest_rect.right += 13;
		}
	if (adven[pc].status[4] > 0) { 
		rect_draw_some_item (mixed_gworld,source_rects[5],dest_bmp,dest_rect,mode,dest);
		dest_rect.left += 13;
		dest_rect.right += 13;
		}
	if (adven[pc].status[3] > 0) { 
		rect_draw_some_item (mixed_gworld,source_rects[6],dest_bmp,dest_rect,mode,dest);
		dest_rect.left += 13;
		dest_rect.right += 13;
		}
	if (adven[pc].status[3] < 0) { 
		rect_draw_some_item (mixed_gworld,source_rects[8],dest_bmp,dest_rect,mode,dest);
		dest_rect.left += 13;
		dest_rect.right += 13;
		}
	if ((adven[pc].status[5] > 0) && (dest_rect.right < right_limit)) { 
		rect_draw_some_item (mixed_gworld,source_rects[9],dest_bmp,dest_rect,mode,dest);
		dest_rect.left += 13;
		dest_rect.right += 13;
		}
	if ((adven[pc].status[6] > 0) && (dest_rect.right < right_limit)) { 
		rect_draw_some_item (mixed_gworld,source_rects[10],dest_bmp,dest_rect,mode,dest);
		dest_rect.left += 13;
		dest_rect.right += 13;
		}
	if ((adven[pc].status[7] > 0) && (dest_rect.right < right_limit)){ 
		rect_draw_some_item (mixed_gworld,source_rects[11],dest_bmp,dest_rect,mode,dest);
		dest_rect.left += 13;
		dest_rect.right += 13;
		}
	if ((adven[pc].status[8] > 0) && (dest_rect.right < right_limit)){ 
		rect_draw_some_item (mixed_gworld,source_rects[12],dest_bmp,dest_rect,mode,dest);
		dest_rect.left += 13;
		dest_rect.right += 13;
		}
	if ((adven[pc].status[9] > 0) && (dest_rect.right < right_limit)){ 
		rect_draw_some_item (mixed_gworld,source_rects[13],dest_bmp,dest_rect,mode,dest);
		dest_rect.left += 13;
		dest_rect.right += 13;
		}	
	if ((adven[pc].status[10] > 0) && (dest_rect.right < right_limit)){ 
		rect_draw_some_item (mixed_gworld,source_rects[14],dest_bmp,dest_rect,mode,dest);
		dest_rect.left += 13;
		dest_rect.right += 13;
		}	
	if ((adven[pc].status[11] > 0) && (dest_rect.right < right_limit)){ 
		rect_draw_some_item (mixed_gworld,source_rects[15],dest_bmp,dest_rect,mode,dest);
		dest_rect.left += 13;
		dest_rect.right += 13;
		}	
	if ((adven[pc].status[12] > 0) && (dest_rect.right < right_limit)){ 
		rect_draw_some_item (mixed_gworld,source_rects[16],dest_bmp,dest_rect,mode,dest);
		dest_rect.left += 13;
		dest_rect.right += 13;
		}	
	if ((adven[pc].status[13] > 0) && (dest_rect.right < right_limit)){ 
		rect_draw_some_item (mixed_gworld,source_rects[17],dest_bmp,dest_rect,mode,dest);
		dest_rect.left += 13;
		dest_rect.right += 13;
		}	
}


void print_party_stats() {
	add_string_to_buf("PARTY STATS:");
	sprintf((char *) store_string, "  Number of kills: %ld                   ", party.total_m_killed);
	add_string_to_buf((char *) store_string);
	if ((is_town()) || ((is_combat()) && (which_combat_type == 1))) {
		sprintf((char *) store_string, "  Kills in this town: %d                   ", party.m_killed[c_town.town_num]);
		add_string_to_buf((char *) store_string);
		}
	sprintf((char *) store_string, "  Total experience: %ld                   ", party.total_xp_gained);
	add_string_to_buf((char *) store_string);
	sprintf((char *) store_string, "  Total damage done: %ld                   ", party.total_dam_done);
	add_string_to_buf((char *) store_string);
	sprintf((char *) store_string, "  Total damage taken: %ld                   ", party.total_dam_taken);
	add_string_to_buf((char *) store_string);
	print_buf();
}


short do_look(location space)
{
	short i,j,num_items = 0;
	Boolean gold_here = FALSE, food_here = FALSE, is_lit = TRUE;
	location from_where;
		
	from_where = get_cur_loc();
	is_lit = pt_in_light(from_where,space);

	if (((is_out()) && (same_point(space,party.p_loc) == TRUE)) ||
		((is_town()) && (same_point(space,c_town.p_loc))))
			add_string_to_buf("    Your party");
	if (is_combat())
		for (i = 0; i < 6; i++)
			if ((same_point(space,pc_pos[i]) == TRUE) && (adven[i].main_status == 1)
				&& (is_lit == TRUE) && (can_see(pc_pos[current_pc],space,0) < 5)) {
				sprintf((char *) store_string, "    %s", (char *) adven[i].name);
				add_string_to_buf((char *) store_string);
				}

	if (is_out() == FALSE) {
		for (i = 0; i < T_M; i++)
			if ((c_town.monst.dudes[i].active != 0) && (is_lit == TRUE)
				&& (monst_on_space(space,i) == TRUE) &&
				((is_town()) || (can_see(pc_pos[current_pc],space,0) < 5))
				&& (c_town.monst.dudes[i].m_d.picture_num != 0)) {


				get_m_name(store_string2,c_town.monst.dudes[i].number);
				if (c_town.monst.dudes[i].m_d.health < c_town.monst.dudes[i].m_d.m_health) {
					if (c_town.monst.dudes[i].attitude % 2 == 1)
						sprintf((char *) store_string, "    Wounded %s (H)", store_string2);
						else sprintf((char *) store_string, "    Wounded %s (F)", store_string2);
					}
				else {
					if (c_town.monst.dudes[i].attitude % 2 == 1)
						sprintf((char *) store_string, "    %s (H)", (char *) store_string2);
						else sprintf((char *) store_string, "    %s (F)", (char *) store_string2);
					}

				add_string_to_buf((char *) store_string);

				}
		}
	if (is_out()) {
		for (i = 0; i < 10; i++) {
			if ((party.out_c[i].exists == TRUE)
				&& (same_point(space,party.out_c[i].m_loc) == TRUE)) {
					for (j = 0; j < 7; j++)
						if (party.out_c[i].what_monst.monst[j] != 0) {
							get_m_name(store_string2,party.out_c[i].what_monst.monst[j]);
							sprintf((char *) store_string, "    %s", store_string2);
							add_string_to_buf((char *) store_string);
							j = 7;

							}

					}
			}

		if (out_boat_there(space) < 30)
			add_string_to_buf("    Boat                ");
		if (out_horse_there(space) < 30)
			add_string_to_buf("    Horse                ");
		}

	if (is_out() == FALSE) {
		if (town_boat_there(space) < 30)
			add_string_to_buf("    Boat               ");
		if (town_horse_there(space) < 30)
			add_string_to_buf("    Horse               ");

		if (is_web(space.x,space.y))
			add_string_to_buf("    Web               ");
		if (is_crate(space.x,space.y))
			add_string_to_buf("    Crate               ");
		if (is_barrel(space.x,space.y))
			add_string_to_buf("    Barrel               ");
		if (is_fire_barrier(space.x,space.y))
			add_string_to_buf("    Magic Barrier               ");
		if (is_force_barrier(space.x,space.y))
			add_string_to_buf("    Magic Barrier               ");
		if (is_quickfire(space.x,space.y))
			add_string_to_buf("    Quickfire               ");
		if (is_fire_wall(space.x,space.y))
			add_string_to_buf("    Wall of Fire               ");
		if (is_force_wall(space.x,space.y))
			add_string_to_buf("    Wall of Force               ");
		if (is_antimagic(space.x,space.y))
			add_string_to_buf("    Antimagic Field               ");
		if (is_scloud(space.x,space.y))
			add_string_to_buf("    Stinking Cloud               ");
		if (is_ice_wall(space.x,space.y))
			add_string_to_buf("    Ice Wall               ");
		if (is_blade_wall(space.x,space.y))
			add_string_to_buf("    Blade Wall               ");

		if (is_small_blood(space.x,space.y))
			add_string_to_buf("    Blood stain               ");
		if (is_medium_blood(space.x,space.y))
			add_string_to_buf("    Blood stain               ");
		if (is_large_blood(space.x,space.y))
			add_string_to_buf("    Blood stain               ");
		if (is_small_slime(space.x,space.y))
			add_string_to_buf("    Smears of slime               ");
		if (is_big_slime(space.x,space.y))
			add_string_to_buf("    Smears of slime               ");
		if (is_ash(space.x,space.y))
			add_string_to_buf("    Ashes               ");
		if (is_bones(space.x,space.y))
			add_string_to_buf("    Bones               ");
		if (is_rubble(space.x,space.y))
			add_string_to_buf("    Rubble               ");

		for (i = 0; i < NUM_TOWN_ITEMS; i++) {
			if ((t_i.items[i].variety != 0) && (same_point(space,t_i.items[i].item_loc) == TRUE)
				&& (is_lit == TRUE)) {
				if (t_i.items[i].variety == 3)
					gold_here = TRUE;
				else if (t_i.items[i].variety == 11)
					food_here = TRUE;
					else num_items++;
				}
			}
		if (gold_here == TRUE)
			add_string_to_buf("    Gold");
		if (food_here == TRUE)
			add_string_to_buf("    Food");
		if (num_items > 8)
			add_string_to_buf("    Many items");
			else for (i = 0; i < NUM_TOWN_ITEMS; i++) {
				if ((t_i.items[i].variety != 0) && (t_i.items[i].variety != 3) &&(t_i.items[i].variety != 11) &&
					 (same_point(space,t_i.items[i].item_loc) == TRUE) && (is_contained(t_i.items[i]) == FALSE)) {
					if (is_ident(t_i.items[i]) == TRUE)
						sprintf((char *) store_string, "    %s",t_i.items[i].full_name);
						else sprintf((char *) store_string, "    %s",t_i.items[i].name);
					add_string_to_buf((char *) store_string);
					}
				}
		}

	if (is_lit == FALSE) {
		add_string_to_buf("    Dark                ");
		return 0;
		}

	return print_terrain(space);
}

short town_boat_there(location where)
{
	short i;

	// Num boats stores highest # of boat in town
	for (i = 0; i < 30; i++)
		if ((party.boats[i].exists == TRUE) && (party.boats[i].which_town == c_town.town_num)
		 && (same_point(where,party.boats[i].boat_loc) == TRUE))
			return i;
	return 30;
}
short out_boat_there(location where)
{
	short i;
	
	for (i = 0; i < 30; i++)
		if ((party.boats[i].exists == TRUE) && (same_point(where,party.boats[i].boat_loc) == TRUE)
			&& (party.boats[i].which_town == 200))
			return i;
	return 30;
}

short town_horse_there(location where)
{
	short i;
	
	// Num boats stores highest # of boat in town
	for (i = 0; i < 30; i++)
		if ((party.horses[i].exists == TRUE) && (party.horses[i].which_town == c_town.town_num) 
		 && (same_point(where,party.horses[i].horse_loc) == TRUE))
			return i;
	return 30;
}
short out_horse_there(location where)
{
	short i;
	
	for (i = 0; i < 30; i++)
		if ((party.horses[i].exists == TRUE) && (same_point(where,party.horses[i].horse_loc) == TRUE)
			&& (party.horses[i].which_town == 200))
			return i;
	return 30;
}
void notify_out_combat_began(out_wandering_type encounter,short *nums) 
{
	short i;

	sprintf((char *) store_string, "COMBAT!                 ");				
	add_string_to_buf((char *) store_string);	

	for (i = 0; i < 6; i++)
		if (encounter.monst[i] != 0) {
			switch (encounter.monst[i]) {
				////
				
				default:
				get_m_name(store_string2,encounter.monst[i]);
				sprintf((char *) store_string, "  %d x %s        ",nums[i],store_string2);
				break;		
			}				
			add_string_to_buf((char *) store_string);	
			}
	if (encounter.monst[6] != 0) {
			get_m_name(store_string2,encounter.monst[6]);
			sprintf((char *) store_string, "  %s        ",store_string2);
			add_string_to_buf((char *) store_string);		
		}
}

void get_m_name(char *str,unsigned char num)
{
	char store_name[256];
	
	strcpy((char *) str,(char *) data_store2->scen_item_list.monst_names[num]);
}
void get_ter_name(char *str,unsigned char num)
{
	char store_name[256];
	
	////
	if ((num == 90) && ((is_out()) || (is_town()) || ((is_combat()) && (which_combat_type == 1))))
		sprintf((char *) store_name,"Pit");
		else {
			strcpy((char *) store_name,(char *) data_store2->scen_item_list.ter_names[num]);
			}
	strcpy((char *) str,(char *) store_name);
}

void print_monst_name(unsigned char m_type)
{
	get_m_name(store_string2,m_type);
	sprintf ((char *) store_string, "%s:",store_string2);
	add_string_to_buf((char *) store_string);
}

void print_monst_attacks(unsigned char m_type,short target)
//short target; // < 100 - pc  >= 100  monst
{
	char store_string3[60];
	
	get_m_name(store_string2,m_type);
	if (target < 100)
		sprintf ((char *) store_string, "%s attacks %s",
			store_string2,(char *) adven[target].name);
		else {
			get_m_name(store_string3,c_town.monst.dudes[target - 100].number);
			sprintf ((char *) store_string, "%s attacks %s",
			store_string2,store_string3);
			}
	add_string_to_buf((char *) store_string);
}

void damaged_message(short damage,short type)
{
	char str[256];
	
	get_str(str,20,130 + type);
	sprintf ((char *) store_string, "  %s for %d",
			(char *) str,damage);
	add_string_to_buf((char *) store_string);	
}

// This prepares the monster's string for the text bar
void print_monster_going(char *combat_str,unsigned char m_num,short ap)
{
	get_m_name(store_string2,m_num);
	sprintf ((char *) combat_str, "%s (ap: %d)",
		store_string2,ap);
}

void monst_spell_note(unsigned char number,short which_mess)
{
	get_m_name(store_string2,number);
	switch (which_mess) {
		case 1:
	sprintf ((char *) store_string, "  %s scared. ",store_string2);break;
	
		case 2:
	sprintf ((char *) store_string, "  %s slowed. ",store_string2);break;
	
		case 3:
	sprintf ((char *) store_string, "  %s weakened.",store_string2);break;
	
		case 4:
	sprintf ((char *) store_string, "  %s poisoned.",store_string2);break;
	
		case 5:
	sprintf ((char *) store_string, "  %s cursed.",store_string2);break;

		case 6:
	sprintf ((char *) store_string, "  %s ravaged.",store_string2);break;

		case 7:
	sprintf ((char *) store_string, "  %s undamaged.",store_string2);break;

		case 8:
	sprintf ((char *) store_string, "  %s is stoned.",store_string2);break;
		case 9:
	sprintf ((char *) store_string, "  Gazes at %s.",store_string2);break;
		case 10:
	sprintf ((char *) store_string, "  %s resists.",store_string2);break;		
		case 11:
	sprintf ((char *) store_string, "  Drains %s.",store_string2);break;	
		case 12:
	sprintf ((char *) store_string, "  Shoots at %s.",store_string2);break;	
		case 13:
	sprintf ((char *) store_string, "  Throws spear at %s.",
		store_string2);
			break;	
		case 14:
	sprintf ((char *) store_string, "  Throws rock at %s.",
		store_string2);
			break;	
		case 15:
	sprintf ((char *) store_string, "  Throws razordisk at %s.",
		store_string2);
			break;
		case 16:
	sprintf ((char *) store_string, "  Hits %s.",
		store_string2);
			break;
		case 17:
	sprintf ((char *) store_string, "%s disappears.",
		store_string2);
			break;
		case 18:
	sprintf ((char *) store_string, "  Misses %s.",
		store_string2);
			break;
		case 19:
	sprintf ((char *) store_string, "  %s is webbed.",store_string2);break;
		case 20:
	sprintf ((char *) store_string, "  %s chokes.",store_string2);break;
		case 21:
	sprintf ((char *) store_string, "  %s summoned.",store_string2);break;
		case 22:
	sprintf ((char *) store_string, "  %s is dumbfounded.",store_string2);break;
		case 23:
	sprintf ((char *) store_string, "  %s is charmed.",store_string2);break;
		case 24:
	sprintf ((char *) store_string, "  %s is recorded.",store_string2);break;
		case 25:
	sprintf ((char *) store_string, "  %s is diseased.",store_string2);break;
		case 26:
	sprintf ((char *) store_string, "  %s is an avatar!",store_string2);break;
		case 27:
	sprintf ((char *) store_string, "  %s splits!",store_string2);break;
		case 28:
	sprintf ((char *) store_string, "  %s falls asleep.",store_string2);break;
		case 29:
	sprintf ((char *) store_string, "  %s wakes up.",store_string2);break;
		case 30:
	sprintf ((char *) store_string, "  %s paralyzed.",store_string2);break;
		case 31:
	sprintf ((char *) store_string, "  %s covered with acid.",store_string2);break;
		case 32:
	sprintf ((char *) store_string, "  Fires spines at %s.",store_string2);break;
		}

	if (which_mess > 0)
		add_string_to_buf((char *) store_string);
}

void monst_cast_spell_note(unsigned char number,short spell,short type)
//short type; // 0 - mage 1- priest
{
	get_m_name(store_string2,number);
	sprintf ((char *) store_string, "%s casts:",
			(char *) store_string2);
	add_string_to_buf((char *) store_string);
	sprintf ((char *) store_string, "  %s",
			(type == 1) ? (char *) m_priest_sp[spell - 1] : (char *) m_mage_sp[spell - 1]);
	add_string_to_buf((char *) store_string);
}

void monst_breathe_note(unsigned char number)
{
	get_m_name(store_string2,number);
	sprintf ((char *) store_string, "%s breathes.",
			(char *) store_string2);
	add_string_to_buf((char *) store_string);

}

void monst_damaged_mes(short which_m,short how_much,short how_much_spec)
{
	get_m_name(store_string2,c_town.monst.dudes[which_m].number);
	if (how_much_spec > 0)
		sprintf ((char *) store_string, "  %s takes %d+%d",
			store_string2, how_much,how_much_spec);
		else sprintf ((char *) store_string, "  %s takes %d",
			store_string2, how_much);
 
	add_string_to_buf((char *) store_string);
}

void monst_killed_mes(short which_m)
{
	get_m_name(store_string2,c_town.monst.dudes[which_m].number);
	sprintf ((char *) store_string, "  %s dies.",
		(char *) store_string2);
	add_string_to_buf((char *) store_string);
}

void print_nums(short a,short b,short c)
{
	sprintf((char *) store_string, "debug: %d %d %d", a,b,c);
	add_string_to_buf((char *) store_string);

}

short print_terrain(location space)
{
	unsigned char which_terrain;

	if (is_out()) {
		which_terrain = out[space.x][space.y];
		}
	if (is_town()) {
		which_terrain = t_d.terrain[space.x][space.y];
		}
	if (is_combat()) {
		which_terrain = combat_terrain[space.x][space.y];
		}
	get_ter_name(store_string2,which_terrain);
	sprintf((char *) store_string, "    %s", store_string2);
	add_string_to_buf((char *) store_string);

	return (short) which_terrain;
}


void add_string_to_buf(char *string)
{
	//SetControlValue(text_sbar,58);
	SetScrollPos(text_sbar,SB_CTL,58,TRUE);
	string_added = TRUE;
	if (buf_pointer == mark_where_printing_long) {
		printing_long = TRUE;
		print_buf();
		through_sending();
		}
	sprintf((char *)text_buffer[buf_pointer].line, "%-49.49s", string);
   text_buffer[buf_pointer].line[49] = 0;
//	c2pstr((char *)text_buffer[buf_pointer].line);
	if (buf_pointer == (TEXT_BUF_LEN - 1))
		buf_pointer = 0;
		else buf_pointer++;
}

void init_buf()
{
	short i;
	
	for (i = 0; i < TEXT_BUF_LEN; i++)
		sprintf((char *) text_buffer[buf_pointer].line, " ");
}




void print_buf () 
{
	short num_lines_printed = 0,ctrl_val;
	short line_to_print;
	short start_print_point;
	Boolean end_loop = FALSE;
	RECT store_text_rect = {0,0,256,138},dest_rect,erase_rect = {1,1,255,137}; /**/
	RECT from_rect,to_rect;
	HDC hdc;
	HBITMAP store_bmp;


	if (string_added == TRUE) {

	// First clean up gworld with pretty patterns
	//FillCRECT(&erase_rect,bg[6]);
	InsetRect(&erase_rect,1,1); ////
	erase_rect.right++;
	to_rect = erase_rect;
	to_rect.bottom = to_rect.top + 128;
	from_rect = to_rect;
	OffsetRect(&from_rect,-1 * from_rect.left,-1 * from_rect.top);
	rect_draw_some_item(status_pattern_gworld,from_rect,
		text_area_gworld,to_rect,0,0);
	to_rect = erase_rect;
	to_rect.top = to_rect.bottom - 8;
	from_rect = to_rect;
	OffsetRect(&from_rect,-1 * from_rect.left,-1 * from_rect.top);
	rect_draw_some_item(status_pattern_gworld,from_rect,
		text_area_gworld,to_rect,0,0);


	hdc = CreateCompatibleDC(main_dc);
	//store_text_hdc = hdc;
	SelectPalette(hdc,hpal,0);
	SetBkMode(hdc,TRANSPARENT);
	SelectObject(hdc,small_bold_font);

	store_bmp = SelectObject(hdc,text_area_gworld);

	//ctrl_val = 58 - GetControlValue(text_sbar);
	ctrl_val = 58 - GetScrollPos(text_sbar,SB_CTL);
	start_print_point = buf_pointer - LINES_IN_TEXT_WIN - ctrl_val;
	if (start_print_point< 0)
		start_print_point= TEXT_BUF_LEN + start_print_point;
	line_to_print= start_print_point;
	
	while ((line_to_print!= buf_pointer) && (num_lines_printed < LINES_IN_TEXT_WIN)) {
		//MoveTo(4, 13 + 12 * num_lines_printed);
		//drawstring((char *) text_buffer[line_to_print].line);
		DrawString((char *) text_buffer[line_to_print].line,4,
			 2 + 12 * num_lines_printed,hdc);
		num_lines_printed++;
		line_to_print++;
		if (line_to_print== TEXT_BUF_LEN) {
			line_to_print= 0;
			}
			
		if ((num_lines_printed == LINES_IN_TEXT_WIN - 1) && (printing_long == TRUE)) {
			end_loop = FALSE;
			line_to_print= buf_pointer;
			}
	
		}
			

	// Now put text on window.
	SelectObject(hdc,store_bmp);
	DeleteObject(hdc);
		}

	dest_rect = store_text_rect;

	OffsetRect(&dest_rect,TEXT_WIN_UL_X,TEXT_WIN_UL_Y);
 	// Now put text on window.
	rect_draw_some_item (text_area_gworld, store_text_rect, text_area_gworld, dest_rect, 0, 1);
	string_added = FALSE;	
}

void restart_printing()
{
	lines_to_print = 0;
	//clear_text_panel();
}

void restore_mode()
{
	overall_mode = store_mode;
}

void through_sending()
{
	mark_where_printing_long = buf_pointer + LINES_IN_TEXT_WIN - 1;
	if (mark_where_printing_long > TEXT_BUF_LEN - 1)
		mark_where_printing_long -= TEXT_BUF_LEN;
	printing_long = FALSE;
}

void Display_String(char *str)
{
	char str2[256];

//	//c2pstr((char *) str);
// 	sprintf((char *)str2," %s",str);
//	str2[0] = (char) strlen((char *)str);
//	DrawString(str2);
}

void display_string(char *str)
{
//	c2pstr(str);
//	drawstring(str);
}

/* Draw a bitmap in the world window. hor in 0 .. 8, vert in 0 .. 8,
	object is ptr. to bitmap to be drawn, and masking is for Copybits. */
void Draw_Some_Item (HBITMAP src_gworld, RECT src_rect, HBITMAP targ_gworld, 
location target, char masked, short main_win)
{
RECT	destrec = {0,0,28,36};
	
	if ((target.x < 0) || (target.y < 0) || (target.x > 8) || (target.y > 8))
		return;
	if (src_gworld == NULL)
		return;
	if ((supressing_some_spaces == TRUE) && 
		(same_point(target,ok_space[0]) == FALSE) &&
		(same_point(target,ok_space[1]) == FALSE) &&
		(same_point(target,ok_space[2]) == FALSE) &&
		(same_point(target,ok_space[3]) == FALSE))
			return;
	terrain_there[target.x][target.y] = -1;
	
	destrec = coord_to_rect(target.x,target.y);
	rect_draw_some_item ( src_gworld,  src_rect,   targ_gworld, 
		destrec,   masked,  main_win);

}

RECT coord_to_rect(short i,short j)
{
	RECT to_return;
	
	to_return.left = 13 + BITMAP_WIDTH * i;
	to_return.right = to_return.left + BITMAP_WIDTH;
	to_return.top = 13 + BITMAP_HEIGHT * j;
	to_return.bottom = to_return.top + BITMAP_HEIGHT;		

	return to_return;
}


void c2p(char *str) 
{
}

void p2c(char *str)
{
}

void get_str(char *str,short i, short j)
{
	GetIndString(str, i, j);
	p2c(str);
}

short string_length(char *str,HDC hdc)
{
	short text_len[257];
	short total_width = 0,i,len;
	char p_str[256];
	
	for (i = 0; i < 257; i++)
		text_len[i]= 0;
	
	strcpy((char *) p_str,str);
	MeasureText(256,p_str,text_len,hdc);
	len = strlen((char *)str);

	//print_nums(text_len[1],text_len[2],text_len[3]);
   //print_nums(text_len[10],text_len[20],text_len[30]);
	for (i = 0; i < 257; i++)
		if ((text_len[i] > total_width) && (i <= len))
			total_width = text_len[i];
	return total_width;
}


void char_win_draw_string(HDC dest_window,RECT dest_rect,char *str,short mode,short line_height)
{
	char store_s[256];
	
	strcpy((char *) store_s,str);
	win_draw_string( dest_window, dest_rect,store_s, mode, line_height);

}

// mode: 0 - align up and left, 1 - center on one line
// str is a c string, 256 characters
// uses current font
void win_draw_string(HDC dest_hdc,RECT dest_rect,char *str,short mode,short line_height)
{
	short i;

// this will need formatting for '|' line breaks
	for (i = 0; i < 256; i++)  {
		if (str[i] == 0)
			i = 256;
			else {
			if (str[i] == '|') {
				str[i] = 13;
            }
			if (str[i] == '_')
				str[i] = 34;
			}
		}
	// if dest is main window, add ulx, uly
	if (dest_hdc == main_dc)
   	OffsetRect(&dest_rect,ulx,uly);
	switch (mode) {
		case 0:
         dest_rect.bottom += 6;
			DrawText(dest_hdc,str,strlen((char *)str),&dest_rect,DT_LEFT | DT_NOPREFIX | DT_WORDBREAK); break;
		case 1:
			dest_rect.bottom += 6; dest_rect.top -= 6;
			DrawText(dest_hdc,str,strlen((char *)str),&dest_rect,
			DT_CENTER | DT_NOPREFIX | DT_VCENTER | DT_NOCLIP | DT_SINGLELINE); break;
		case 2: case 3:
			dest_rect.bottom += 6; dest_rect.top -= 6;
			DrawText(dest_hdc,str,strlen((char *)str),&dest_rect,
			DT_LEFT | DT_NOPREFIX | DT_VCENTER | DT_NOCLIP | DT_SINGLELINE); break;
		}
	// not yet done adjusts for 1, 2, 3
	
}

short calc_day()
{
	return (short) ((party.age) / 3700) + 1;
}

Boolean day_reached(unsigned char which_day, unsigned char which_event)
// which_day is day event should happen
// which_event is the party.key_times value to cross reference with. 
// if the key_time is reached before which_day, event won't happen
// if it's 8, event always happens
// which_day gets an extra 20 days to give party bonus time
{
	short what_day;
	
	what_day = (short) (which_day) + 20;
	
	if ((which_event != 8) && (party.key_times[which_event] < what_day))
		return FALSE;
	if (calc_day() >= what_day)
		return TRUE;
		else return FALSE;
		
}

// BEGIN EXTRA WINDOWS STUFF

void WinDrawString(char *string,short x,short y)
{
	HDC hdc;
	COLORREF colors[2] = {RGB(0,0,0),RGB(255,255,255)};
	UINT c[2];

	c[0] = GetNearestPaletteIndex(hpal,colors[0]);
	c[1] = GetNearestPaletteIndex(hpal,colors[1]);

	hdc = GetDC(mainPtr);
	SelectPalette(hdc,hpal,0);
 	SetViewportOrg(hdc,ulx,uly);
	SelectObject(hdc,small_bold_font);
	SetBkMode(hdc,TRANSPARENT);
	SetTextColor(hdc,PALETTEINDEX(c[1]));
	DrawString(string,x,y,hdc);
	fry_dc(mainPtr,hdc);
}

void WinBlackDrawString(char *string,short x,short y)
{
	HDC hdc;

	hdc = GetDC(mainPtr);
	SelectPalette(hdc,hpal,0);
 	SetViewportOrg(hdc,ulx,uly);
	SelectObject(hdc,small_bold_font);
	SetBkMode(hdc,TRANSPARENT);
	DrawString(string,x,y,hdc);
	fry_dc(mainPtr,hdc);
}


void DrawString(char *string,short x,short y,HDC hdc)
{
	RECT text_r = {0,0,450,20};

	OffsetRect(&text_r,x,y);
	DrawText(hdc,string,-1,&text_r,DT_LEFT | DT_SINGLELINE | DT_TOP | DT_NOCLIP);

}

//void Display_String(char *string) {
	//DrawString(string,store_x,store_y,store_text_hdc);
	//}

void FlushEvents(short mode)
// mode... 0 - keystrokes   1 - mouse presses  2 - both
{
	MSG msg;

	if ((mode == 0) || (mode == 2)) {
		while ((PeekMessage(&msg, mainPtr, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)) != 0)
			;
		}
	if ((mode == 1) || (mode == 2)) {
		while ((PeekMessage(&msg, mainPtr, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) != 0)
			;
		}

}

void ExitToShell()
{
	PostQuitMessage(0);
}

void undo_clip()
{
 //	RECT overall_rect = {0,0,530,435};
	HRGN rgn;

	rgn = CreateRectRgn(0,0,5000,5000);
	SelectClipRgn(main_dc,rgn);
	DeleteObject(rgn);
}

void ClipRect(RECT *rect)
{
	HRGN rgn;
	HDC hdc;
	RECT rect2;

	rect2 = *rect;
	OffsetRect(&rect2,ulx,uly);

	rgn = CreateRectRgn(rect2.left,rect2.top,rect2.right,rect2.bottom);
	SelectClipRgn(main_dc,rgn);
	DeleteObject(rgn);
}

void beep()
{
	long dummy;

	MessageBeep(MB_OK);
   Delay(30,&dummy);
	}

void SysBeep(short a)
{
	MessageBeep(MB_OK);
}

void make_cursor_sword() 
{
	SetCursor(sword_curs);

}

void GetIndString(char *str,short i, short j) {
	UINT resnum = 0,len;
	short k;

	resnum = i * 300 + j;

	len = LoadString(store_hInstance,resnum,str,256);
	if (len == 0) {
		sprintf(str,"");
		return;
		}
	for (k = 0; k < 256; k++)  {
		if (str[k] == '|')
			str[k] = 13;
		if (str[k] == '_')
			str[k] = 34;
		}
}

void StringToNum(char *the_string,long *number_given) {
	long store_short;
	sscanf(the_string,"%ld",&store_short);
	*number_given = (long) store_short;
	}

void InsetRect(RECT *rect,short x, short y) {
	InflateRect(rect,-1 * x, -1 * y);
	}

void force_reprint()
{
	string_added = TRUE;
}

// Note ... this expects a str len of at most 256 and
// len_array pointing to a 256 long array of shorts
void MeasureText(short str_len,char *str, short *len_array,HDC hdc)
{
	short text_len[257];
	short total_width = 0,i,len;
	char p_str[257];
	DWORD val_returned;
	char *store_array;
	short *store2;

	store_array = (char *) len_array;
	//text_len = len_array;
	for (i = 0; i < 256; i++)
		text_len[i] = 0;
	for (i = 1; i < str_len; i++) {
		strncpy(p_str,str,i);
		p_str[i] = 0;
		val_returned = GetTextExtent(hdc,p_str,i);
		text_len[i] = LOWORD(val_returned);
		}
	for (i = 0; i < 256; i++) {
		store2 = (short *) store_array;
		*store2 = text_len[i];
		store_array += 2;
		}
	}

// kludgy annoyance
void MoveTo(short x, short y)
{
	store_text_x = x;
	store_text_y = y - 16;
}
void MoveToDrawString(char *string,HDC hdc)
{
	DrawString(string,store_text_x,store_text_y, hdc);
}
////
Boolean is_ident(item_record_type item)
{
	if (item.item_properties & 1)
		return TRUE;
		else return FALSE;
}
Boolean is_magic(item_record_type item)
{
	if (item.item_properties & 4)
		return TRUE;
		else return FALSE;
}
Boolean is_contained(item_record_type item)
{
	if (item.item_properties & 8)
		return TRUE;
		else return FALSE;
}
Boolean is_cursed(item_record_type item)
{
	if (item.item_properties & 16)
		return TRUE;
		else return FALSE;
}
Boolean is_property(item_record_type item)
{
	if (item.item_properties & 2)
		return TRUE;
		else return FALSE;
}
