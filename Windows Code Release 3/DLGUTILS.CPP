

#include <Windows.h>
#include "string.h"
#include "global.h"

#include "dlgutils.h"
#include "text.h"
#include "town.h"
#include "itemdata.h"
#include "locutils.h"
#include "fields.h"
#include "party.h"
#include "specials.h"
#include "fileio.h"
#include "graphics.h"
#include "townspec.h"
#include "items.h"
#include "exlsound.h"
#include "stdio.h"
#include "dlogtool.h"
#include "newgraph.h"
#include "infodlgs.h"
#include "graphutl.h"

#define	NUM_HINTS	30

extern big_tr_type far  t_d;
extern short stat_window,overall_mode,dialog_answer;
extern current_town_type far c_town;
extern party_record_type far party;
extern piles_of_stuff_dumping_type *data_store;
extern piles_of_stuff_dumping_type2 *data_store2;
extern talking_record_type far talking;
extern scenario_data_type far scenario;

extern Boolean in_startup_mode,play_sounds,sys_7_avail,give_intro_hint;
extern HWND mainPtr;

extern short display_mode,stat_screen_mode,current_pc;
extern long register_flag;
extern long ed_flag,ed_key;
extern Boolean registered,ed_reg,save_maps,give_delays;
extern pc_record_type far adven[6];
extern location center;
extern HWND text_sbar,item_sbar,shop_sbar;
extern Boolean modeless_exists[18];
extern HWND modeless_dialogs[18] ;

extern pascal Boolean cd_event_filter();
extern Boolean dialog_not_toast;

extern Boolean game_run_before;

extern HPALETTE hpal;
extern far PALETTEENTRY ape[256];
extern HDC main_dc,main_dc2,main_dc3;
extern Boolean cursor_shown;
extern piles_of_stuff_dumping_type3 *data_store3;
extern piles_of_stuff_dumping_type4 *data_store4;


HBITMAP pcs_gworld = NULL;

short sign_mode,person_graphic,store_person_graphic,store_sign_mode;
long num_talk_entries;
char null_string[256] = "";
short store_tip_page_on = 0;

// Talking vars
word_rect_type far store_words[50];
short store_pre_talk_mode,store_personality,store_personality_graphic,shop_identify_cost;
HBITMAP talk_gworld = NULL;
Boolean talk_end_forced;
char far old_str1[256];
char far old_str2[256];
char far one_back1[256];
char far one_back2[256]; 
extern word_rect_type preset_words[9];
RECT talk_area_rect = {5,5,284,420}, word_place_rect = {7,44,257,372},talk_help_rect = {254,5,272,21};
/**/
char title_string[50];
unsigned char store_monst_type;
short store_m_num;
RECT dummy_rect = {0,0,0,0};
hold_responses far store_resp[83];
short strnum1,strnum2,oldstrnum1,oldstrnum2;
short store_talk_face_pic,cur_town_talk_loaded = -1;

// Shopping vars

// 1 - 499 ... regular items
// 500 alchemy
// 600-620 ... food
// 700+ i - that healing
// 800 + - mage spells
// 900 + - priest spells
// n000 + i - magic store n item i
short far store_shop_items[30];
short far store_shop_costs[30];
// talk_area_rect and talk_help_rect used for this too
short store_shop_min,store_shop_max,store_pre_shop_mode,store_cost_mult;
char far store_store_name[256];
// 0 - whole area, 1 - active area 2 - graphic 3 - item name
// 4 - item cost 5 - item extra str  6 - item help button
extern RECT shopping_rects[8][7];
RECT bottom_help_rects[4] = {{6,356,250,368},{6,374,270,386},{6,386,250,398},{6,398,250,410}};
RECT shop_name_str = {6,44,200,56};
RECT shop_frame = {10,62,269,352};
RECT shop_done_rect = {212,388,275,411}; /**/

extern short store_shop_type;

char far *heal_types[] = {"Heal Damage","Cure Poison","Cure Disease","Cure Paralysis",
		"Uncurse Items","Cure Stoned Character","Raise Dead","Resurrection","Cure Dumbfounding"};
short heal_costs[9] = {50,30,80,100,250,500,1000,3000,100};
long cost_mult[7] = {5,7,10,13,16,20,25};
short cur_display_mode;

short terrain_pic[256]; 

scen_header_type scen_headers[25];

short store_scen_page_on,store_num_scen;

/*
shop_type:
0 - weapon shop
1 - armor shop
2 - misc shop
3 - healer
4 - food
5-9 - magic shop
10 - mage spells
11 - priest spells
12 alchemy
*/
void start_shop_mode(short shop_type,short shop_min,short shop_max,short cost_adj,char *store_name)
{
	RECT area_rect;
	
	if (shop_max < shop_min)
		shop_max = shop_min; ////
	store_cost_mult = cost_adj;
	if (store_cost_mult > 6)
		store_cost_mult = 6;
	store_shop_type = shop_type;
	store_shop_min = shop_min;
	store_shop_max = shop_max;
		
	area_rect = talk_area_rect;
	strcpy((char *) store_store_name,store_name);

	OffsetRect(&area_rect, -1 * area_rect.left,-1 * area_rect.top);
	if (talk_gworld == NULL)
		 talk_gworld = CreateCompatibleBitmap(main_dc,area_rect.right,area_rect.bottom);
	
	store_pre_shop_mode = overall_mode;
	overall_mode = 21;
	stat_screen_mode = 1;
	create_clip_region();
	
	set_up_shop_array();
	put_background();
	
	draw_shop_graphics(0,area_rect);
	
	put_item_screen(stat_window,0);
	give_help(26,27,0);
}

void end_shop_mode()
{
	RECT dummy_rect = {0,0,0,0};
	
	ShowScrollBar(shop_sbar,SB_CTL,FALSE);
	if (store_pre_shop_mode == 20) {
		sprintf((char *)old_str1,"You conclude your business.");
		sprintf((char *)old_str2,"");
		sprintf((char *)one_back1,"You conclude your business.");
		sprintf((char *)one_back2,"");

		strnum1 = strnum2 = oldstrnum1 = oldstrnum2 = 0;
		place_talk_str((char *)old_str1,"",0,dummy_rect);
		}
		else {
			DisposeGWorld(talk_gworld);
			talk_gworld = NULL;
			}
			
	overall_mode = store_pre_shop_mode;
	create_clip_region();
	if (overall_mode == 2)
		overall_mode = 1;
	if (overall_mode == 1) {
		center = c_town.p_loc;
		update_explored(center);
		}
	stat_screen_mode = 0;
	put_item_screen(stat_window,0);
	put_pc_screen();
	refresh_screen(0);
}

void handle_shop_event(POINT p,Boolean right_button)
{
	short i,j,get_pc,store_what_picked;

	p.x -= 5;
	p.y -= 5;

	if (PtInRect(&talk_help_rect,p)) {
		click_shop_rect(talk_help_rect);
		party.help_received[26] = 0;
		give_help(26,27,0);
		return;
		}
	if (PtInRect(&shop_done_rect, p)) {
		click_shop_rect(shop_done_rect);
		end_shop_mode();
		return;
		}

	for (i = 0; i < 8; i++) {
		store_what_picked = i + GetScrollPos(shop_sbar,SB_CTL);
		if ((PtInRect(&shopping_rects[i][1],p)) && (store_shop_items[store_what_picked] >= 0)) {
			click_shop_rect(shopping_rects[i][1]);
			handle_sale(store_shop_items[store_what_picked],store_shop_costs[store_what_picked]);
			}
		if ((PtInRect(&shopping_rects[i][6],p)) && (store_shop_items[store_what_picked] >= 0)
			&& (store_shop_type != 3) && (store_shop_type != 4)){
			click_shop_rect(shopping_rects[i][6]);
			handle_info_request(store_shop_items[store_what_picked]);
			}
		}
}

void handle_sale(short what_chosen,short cost)
{
	item_record_type base_item;
	short what_magic_shop,what_magic_shop_item,i,j;
	RECT dummy_rect = {0,0,0,0};

	switch (what_chosen / 100) {
		case 0: case 1: case 2: case 3: case 4: 
			base_item = get_stored_item(what_chosen);
			base_item.item_properties = base_item.item_properties | 1;
			//cost = (base_item.charges == 0) ? base_item.value : base_item.value * base_item.charges;
			switch (pc_ok_to_buy(current_pc,cost,base_item)) {
				case 1: play_sound(-38); give_to_pc(current_pc,base_item,TRUE); break;
				case 2: ASB("Can't carry any more items."); break;
				case 3: ASB("Not enough cash."); break;
				case 4: ASB("Item is too heavy."); break;
				case 5: ASB("You own too many of this."); break;
				}
			break;
		case 5:
			base_item = store_alchemy(what_chosen - 500);
			if (party.alchemy[base_item.item_level] == TRUE)
				ASB("You already know that recipe.");
				else if (take_gold(cost,FALSE) == FALSE)
					ASB("Not enough gold.");
					else {
						play_sound(62);
						ASB("You buy an alchemical recipe.");
						party.alchemy[base_item.item_level] = TRUE;
						}
			break;
		case 7:
			what_chosen -= 700;
			if (take_gold(cost,FALSE) == FALSE)
				ASB("Not enough gold.");
				else {
					ASB("You pay the healer.");
					play_sound(68);
					switch (what_chosen) {
						case 0:
							adven[current_pc].cur_health = adven[current_pc].max_health;
							break;
						case 1:
							adven[current_pc].status[2] = 0;
							break;
						case 2:
							adven[current_pc].status[7] = 0; break;
						case 3:
							adven[current_pc].status[12] = 0; break;
						case 4: 
							for (i = 0; i < 24; i++)
								if ((adven[current_pc].equip[i] == TRUE) && 
									(is_cursed(adven[current_pc].items[i])))
										adven[current_pc].items[i].item_properties =
											adven[current_pc].items[i].item_properties & 239;
  							break;
						case 5: case 6: case 7:
							adven[current_pc].main_status = 1; break;
						case 8:
							adven[current_pc].status[9] = 0; break;
						}
					}
			break;
		case 8:
			base_item = store_mage_spells(what_chosen - 800 - 30);
			if ((base_item.item_level < 0) || (base_item.item_level > 61)) {
				SysBeep(50); ASB("Error 102: Report this!"); break;}
			if (adven[current_pc].mage_spells[base_item.item_level] == TRUE)
				ASB("You already have this spell.");
				else if (take_gold(cost,FALSE) == FALSE)
					ASB("Not enough gold.");
					else {
						play_sound(62);
						ASB("You buy a spell.");
						adven[current_pc].mage_spells[base_item.item_level] = TRUE;
						give_help(41,0,0);
						}
			break;
		case 9:
			base_item = store_priest_spells(what_chosen - 900 - 30);
			if ((base_item.item_level < 0) || (base_item.item_level > 61)) {
				SysBeep(50); ASB("Error 101: Report this!"); break;}
			if (adven[current_pc].priest_spells[base_item.item_level] == TRUE)
				ASB("You already have this spell.");
				else if (take_gold(cost,FALSE) == FALSE)
					ASB("Not enough gold.");
					else {
						play_sound(62);
						ASB("You buy a spell.");
						adven[current_pc].priest_spells[base_item.item_level] = TRUE;
						give_help(41,0,0);
						}
			break;
		default:
			what_magic_shop = (what_chosen / 1000) - 1;
			what_magic_shop_item = what_chosen % 1000;
			base_item = party.magic_store_items[what_magic_shop][what_magic_shop_item];
			base_item.item_properties = base_item.item_properties | 1;
			switch (pc_ok_to_buy(current_pc,cost,base_item)) {
				case 1: play_sound(-38); give_to_pc(current_pc,base_item,TRUE); 
					party.magic_store_items[what_magic_shop][what_magic_shop_item].variety = 0;
					break;
				case 2: ASB("Can't carry any more items."); break;
				case 3: ASB("Not enough cash."); break;
				case 4: ASB("Item is too heavy."); break;
				}
			break;
		}
	set_up_shop_array();
	
	if (overall_mode != 21) {
		SysBeep(50);
		ASB("Shop error 1. Report This!");
		}	
	draw_shop_graphics(0,dummy_rect);
	print_buf();
	put_pc_screen();
	put_item_screen(stat_window,0);
}


void handle_info_request(short what_chosen)
{
	item_record_type base_item;
	short what_magic_shop,what_magic_shop_item;
	
	switch (what_chosen / 100) {
		case 0: case 1: case 2: case 3: case 4: 
			base_item = get_stored_item(what_chosen);
			base_item.item_properties = base_item.item_properties | 1;
			display_pc_item(6,0, base_item,0);
			break;
		case 5:
			display_help(0,0);
			break;
		case 8:
			base_item = store_mage_spells(what_chosen - 800 - 30);
			display_spells(0,base_item.item_level,0);
			break;
		case 9:
			base_item = store_priest_spells(what_chosen - 900 - 30);
			display_spells(1,base_item.item_level,0);
			break;
		default:
			what_magic_shop = (what_chosen / 1000) - 1;
			what_magic_shop_item = what_chosen % 1000;
			base_item = party.magic_store_items[what_magic_shop][what_magic_shop_item];
			base_item.item_properties = base_item.item_properties | 1;
			display_pc_item(6,0, base_item,0);
			break;
	}
}

void set_up_shop_array()
{
	short i,shop_pos = 0;
	Boolean cursed_item = FALSE;
	item_record_type store_i;
	long store_l;
	
	for (i = 0; i < 30; i++)
		store_shop_items[i] = -1;
	switch (store_shop_type) {
		case 0: case 1: case 2:
			for (i = store_shop_min; i < store_shop_max + 1; i++) {
				store_shop_items[shop_pos] = i;
				store_i = get_stored_item(store_shop_items[shop_pos]);
				store_shop_costs[shop_pos] = (store_i.charges == 0) ? 
				  store_i.value : store_i.value * store_i.charges;
				shop_pos++;
				}
			break;		
		case 3:
			if (adven[current_pc].cur_health < adven[current_pc].max_health) {
				store_shop_items[shop_pos] = 700;
				store_shop_costs[shop_pos] = heal_costs[0];
				shop_pos++;
				}
			if (adven[current_pc].status[2] > 0) {
				store_shop_items[shop_pos] = 701;
				store_shop_costs[shop_pos] = heal_costs[1];
				shop_pos++;
				}
			if (adven[current_pc].status[7] > 0) {
				store_shop_items[shop_pos] = 702;
				store_shop_costs[shop_pos] = heal_costs[2];
				shop_pos++;
				}
			if (adven[current_pc].status[12] > 0) {
				store_shop_items[shop_pos] = 703;
				store_shop_costs[shop_pos] = heal_costs[3];
				shop_pos++;
				}
			if (adven[current_pc].status[9] > 0) {
				store_shop_items[shop_pos] = 708;
				store_shop_costs[shop_pos] = heal_costs[8];
				shop_pos++;
				}
			for (i = 0; i < 24; i++)
				if ((adven[current_pc].equip[i] == TRUE) && (is_cursed(adven[current_pc].items[i]) == TRUE))
					cursed_item = TRUE;
  			if (cursed_item) {
				store_shop_items[shop_pos] = 704;
				store_shop_costs[shop_pos] = heal_costs[4];
				shop_pos++;
				}
			if (adven[current_pc].main_status == 4) {
				store_shop_items[shop_pos] = 705;
				store_shop_costs[shop_pos] = heal_costs[5];
				shop_pos++;
				}
			if (adven[current_pc].main_status == 2){
				store_shop_items[shop_pos] = 706;
				store_shop_costs[shop_pos] = heal_costs[6];
				shop_pos++;
				}
			if  (adven[current_pc].main_status == 3){
				store_shop_items[shop_pos] = 707;
				store_shop_costs[shop_pos] = heal_costs[7];
				shop_pos++;
				}
			break;
		case 4:
			//for (i = store_shop_min; i < store_shop_max + 1; i++) {
			//	store_shop_items[shop_pos] = 600 + i;
			//	store_shop_costs[shop_pos] = food_types[i].value;
			//	shop_pos++;
			//	}
			break;
		case 5: case 6: case 7: case 8: case 9:
			for (i = 0; i < 10; i++)
				if (party.magic_store_items[store_shop_type - 5][i].variety != 0) {
					store_shop_items[shop_pos] = (store_shop_type - 4) * 1000 + i;
					store_i = party.magic_store_items[store_shop_type - 5][i];
					store_shop_costs[shop_pos] = (store_i.charges == 0) ? 
					  store_i.value : store_i.value * store_i.charges;
					shop_pos++;
					}
			break;
		case 10:
			for (i = store_shop_min; i < store_shop_max + 1; i++) 
				if (i == minmax(0,31,i)) {
				store_i = store_mage_spells(i);
				store_shop_costs[shop_pos] = store_i.value;
				store_shop_items[shop_pos] = 800 + i + 30;
				shop_pos++;
				}
			break;
		case 11:
			for (i = store_shop_min; i < store_shop_max + 1; i++) 
				if (i == minmax(0,31,i)) {
				store_i = store_priest_spells(i);
				store_shop_costs[shop_pos] = store_i.value;
				store_shop_items[shop_pos] = 900 + i + 30;
				shop_pos++;
				}
			break;
		case 12:
			for (i = store_shop_min; i < store_shop_max + 1; i++) 
				if (i == minmax(0,19,i)) {
				store_i = store_alchemy(i);
				store_shop_costs[shop_pos] = store_i.value;
				store_shop_items[shop_pos] = 500 + i;
				shop_pos++;
				}
			break;
		}
	for (i = 0; i < 30; i++)
		if (store_shop_items[i] >= 0) {
			store_l = store_shop_costs[i];
			store_l = (store_l * cost_mult[store_cost_mult]) / 10;
			store_shop_costs[i] = (short) store_l;
			}
  	i = max(0,shop_pos - 8);
	SetScrollRange(shop_sbar,SB_CTL,0,i,TRUE);

}

void start_talk_mode(short m_num,short personality,unsigned char monst_type,short store_face_pic)////
{
	RECT area_rect;
	char place_string1[256] = "";
	char place_string2[256] = "";
	short i,cash;

	store_personality = personality;
		
	store_monst_type = monst_type;
	store_m_num = m_num;
	store_talk_face_pic = store_face_pic; ////
	area_rect = talk_area_rect;
	OffsetRect(&area_rect, -1 * area_rect.left,-1 * area_rect.top);
	talk_gworld = CreateCompatibleBitmap(main_dc,area_rect.right,area_rect.bottom);

	// first make sure relevant talk strs are loaded in
	if (personality / 10 != cur_town_talk_loaded)
		load_town(personality / 10,1,0,NULL);

	// load all possible responses
	store_responses();

	// Dredge up critter's name
	sprintf((char *) title_string,"%s:",data_store3->talk_strs[personality % 10]);
	
	store_pre_talk_mode = overall_mode;
	overall_mode = 20;
	create_clip_region();
	talk_end_forced = FALSE;
	stat_screen_mode = 1;
	
	// Bring up and place first strings.
	sprintf((char *) place_string1,"%s",data_store3->talk_strs[personality % 10 + 10]);
	strnum1 = personality % 10 + 10;
	strnum2 = 0;
	strcpy((char *) old_str1,(char *) place_string1);
	strcpy((char *) old_str2,(char *) place_string2);
	strcpy((char *) one_back1,(char *) place_string1);
	strcpy((char *) one_back2,(char *) place_string2);
	place_talk_str((char *) place_string1,(char *) place_string2,0,dummy_rect);
	
	put_item_screen(stat_window,0);
	give_help(5,6,0);

}

void end_talk_mode()
{
	DisposeGWorld(talk_gworld);
	talk_gworld = NULL;
	
	overall_mode = store_pre_talk_mode;
	create_clip_region();
	if (overall_mode == 2)
		overall_mode = 1;
	if (overall_mode == 1) {
		center = c_town.p_loc;
		update_explored(center);
		}
	stat_screen_mode = 0;
	put_item_screen(stat_window,0);
	put_pc_screen();
	//refresh_screen(0);
	redraw_screen(0);
}

void handle_talk_event(POINT p,Boolean right_button)
{
	short i,j,force_special = 0,get_pc,s1 = -1,s2 = -1,s3 = -1;
	char asked[4];
	char place_string1[256] = "";
	char place_string2[256] = "";

	short a,b,c,d,per1,per2,ttype,which_talk_entry = -1;
	char stra[10],strb[10];

	long per1l,ttypel,al,bl,cl,dl,per2l;

	p.x -= 5;
	p.y -= 5;

	if (PtInRect(&talk_help_rect,p)) {
		party.help_received[5] = 0;
		give_help(5,6,0);
		return;
		}

	for (i = 0; i < 9; i++)
		if ((PtInRect(&preset_words[i].word_rect,p)) && ((talk_end_forced == FALSE) || (i == 6) || (i == 5))) {
			click_talk_rect((char *) old_str1,(char *) old_str2,preset_words[i].word_rect);
			switch (i) {
				case 0: case 1: case 2: case 7: case 8:
					force_special = i + 1;
					break;
				case 3: case 4:
					force_special = i + 1;

					//asked[0] = 'p';asked[1] = 'u';asked[2] = 'r';asked[3] = 'c';
					break;
				case 5: // save
					if (strnum1 <= 0) {
						SysBeep(20);
						return;
						}
					for (j = 0; j < 120; j++)
						if ((party.talk_save[j].personality == store_personality) && 
						  (party.talk_save[j].str1 == strnum1) &&
						  (party.talk_save[j].str2 == strnum2)) {
						  	ASB("This is already saved.");
						  	print_buf();
						  	return;
						  	}
					for (j = 0; j < 120; j++)
						if (party.talk_save[j].personality <= 0) {
							give_help(57,0,0);
							play_sound(0);
							party.talk_save[j].personality = store_personality;
							party.talk_save[j].town_num = (unsigned char) c_town.town_num;
							party.talk_save[j].str1 = strnum1;
							party.talk_save[j].str2 = strnum2;
							ASB("Noted in journal.");
							j = 200;
							}		
					if (j < 200) {
						SysBeep(20);
						ASB("No more room in talking journal.");
						}		
					print_buf();	
					return;
					break;
				case 6: // quit
					end_talk_mode();
					return;
					break;
				default:
					for (j = 0; j < 4; j++)
						asked[j] = preset_words[i].word[j];
					break;
				} 		
			i = 100;
			}
	if (i < 100) {
		for (i = 0; i < 50; i++) 
			if ((PtInRect(&store_words[i].word_rect,p)) && (talk_end_forced == FALSE)) {
				click_talk_rect((char *) old_str1,(char *) old_str2,store_words[i].word_rect);
				for (j = 0; j < 4; j++)
					asked[j] = store_words[i].word[j];
			
				i = 100;
				}
		}
	if (i == 50) // no event
		return;
	if (force_special == 9) {
		get_text_response(1017,place_string1,0);
		asked[0] = place_string1[0];
		asked[1] = place_string1[1];
		asked[2] = place_string1[2];
		asked[3] = place_string1[3];
		}
	
	if ((asked[0] == 'n') && (asked[1] == 'a') &&(asked[2] == 'm') &&(asked[3] == 'a')) {
		force_special = 2;
		}
	if ((asked[0] == 'l') && (asked[1] == 'o') &&(asked[2] == 'o') &&(asked[3] == 'k')) {
		force_special = 1;
		}
	if (((asked[0] == 'j') && (asked[1] == 'o') &&(asked[2] == 'b')) ||
		((asked[0] == 'w') && (asked[1] == 'o') &&(asked[2] == 'r')&&(asked[3] == 'k')) ) {
		force_special = 3;	
		}

	if (force_special > 0) {
		switch (force_special) {
			case 1: case 2: case 3:
				get_str(place_string1,120 + ((store_personality - 1) / 10),
				 ((store_personality - 1) % 10) * 3 + 10 + force_special);
				sprintf((char *) place_string1,"%s",data_store3->talk_strs[store_personality % 10 + 10 * force_special]);
					
				oldstrnum1 = strnum1; oldstrnum2 = strnum2;
				strnum1 =  store_personality % 10 + 10 * force_special;
				strnum2 = 0;
				strcpy((char *) one_back1,(char *) old_str1);
				strcpy((char *) one_back2,(char *) old_str2);
				strcpy((char *) old_str1,(char *) place_string1);
				strcpy((char *) old_str2,(char *) place_string2);
				place_talk_str((char *) place_string1,(char *) place_string2,0,dummy_rect);
				return;
				break;
			case 4: // buy button
				asked[0] = 'p';asked[1] = 'u';asked[2] = 'r';asked[3] = 'c';
				if (scan_for_response(asked) >= 0)
					break;
				asked[0] = 's';asked[1] = 'a';asked[2] = 'l';asked[3] = 'e';
				if (scan_for_response(asked) >= 0)
					break;
				asked[0] = 'h';asked[1] = 'e';asked[2] = 'a';asked[3] = 'l';
				if (scan_for_response(asked) >= 0)
					break;
				asked[0] = 'i';asked[1] = 'd';asked[2] = 'e';asked[3] = 'n';
				if (scan_for_response(asked) >= 0)
					break;
				asked[0] = 't';asked[1] = 'r';asked[2] = 'a';asked[3] = 'i';
				if (scan_for_response(asked) >= 0)
					break;
				break;
			case 5: // sell button
				asked[0] = 's';asked[1] = 'e';asked[2] = 'l';asked[3] = 'l';
				if (scan_for_response(asked) >= 0)
					break;
				break;
			case 8: // back 1
				strnum1 = oldstrnum1; strnum2 = oldstrnum2;
				strcpy((char *) place_string1,(char *) one_back1);
				strcpy((char *) place_string2,(char *) one_back2);
				strcpy((char *) one_back1,(char *) old_str1);
				strcpy((char *) one_back2,(char *) old_str2);
				strcpy((char *) old_str1,(char *) place_string1);
				strcpy((char *) old_str2,(char *) place_string2);
				place_talk_str((char *) place_string1,(char *) place_string2,0,dummy_rect);
				return;
				break;
			}
		}
	
	which_talk_entry = scan_for_response(asked);
	if ((which_talk_entry < 0) || (which_talk_entry > 59)) {
		strcpy((char *) one_back1,(char *) old_str1);
		strcpy((char *) one_back2,(char *) old_str2);
		sprintf((char *) old_str2,"");
		sprintf((char *) old_str1,"%s",data_store3->talk_strs[store_personality % 10 + 160]);
		if (strlen((char *) old_str1) < 2)
			sprintf((char *) old_str1,"You get no response.");
		place_talk_str((char *) old_str1,(char *) old_str2,0,dummy_rect);
		strnum1 = -1;
		return;	
		}
			
	ttype = talking.talk_nodes[which_talk_entry].type;
	a = talking.talk_nodes[which_talk_entry].extras[0];
	b = talking.talk_nodes[which_talk_entry].extras[1];
	c = talking.talk_nodes[which_talk_entry].extras[2];
	d = talking.talk_nodes[which_talk_entry].extras[3];

	sprintf((char *) place_string1,"%s",data_store3->talk_strs[40 + which_talk_entry * 2]);
	sprintf((char *) place_string2,"%s",data_store3->talk_strs[40 + which_talk_entry * 2 + 1]);
	
	oldstrnum1 = strnum1; oldstrnum2 = strnum2;
	strnum1 =  40 + which_talk_entry * 2; strnum2 = 40 + which_talk_entry * 2 + 1;

	switch(ttype) {
		case 0:
			break;
		case 1:
			if (PSD[a][b] > c) {
				strnum1 = strnum2;
				strcpy((char *) place_string1,(char *) place_string2);
				}
			sprintf((char *) place_string2,"");					
			strnum2 = 0;
			break;
		case 2:
			PSD[a][b] = 1;
			break;
		case 3:
			if (party.gold < a) {
				strnum1 = strnum2;
				strcpy((char *) place_string1,(char *) place_string2);
				}
				else {
					talk_end_forced = TRUE;
					party.gold -= a;
					put_pc_screen();
					heal_party(30 * b);
					restore_sp_party(25 * b);
					party.age += 700;
					c_town.p_loc.x = c;
					c_town.p_loc.y = d;
					center = c_town.p_loc;
					}
			strnum2 = 0;
			sprintf((char *) place_string2,"");					
			break;
		case 4:
			if (day_reached((unsigned char) a,0) == TRUE) {
				strnum1 = strnum2;
				strcpy((char *) place_string1,(char *) place_string2);
				}
			sprintf((char *) place_string2,"");					
			strnum2 = 0;
			break;
		case 5:
			if (day_reached((unsigned char) a,(unsigned char) b) == TRUE) {
				strnum1 = strnum2;
				strcpy((char *) place_string1,(char *) place_string2);
				}
			sprintf((char *) place_string2,"");					
			strnum2 = 0;
			break;
		case 6:
			if (c_town.town_num != a) {
				strnum1 = strnum2;
				strcpy((char *) place_string1,(char *) place_string2);
				}
			sprintf((char *) place_string2,"");					
			strnum2 = 0;
			break;
		case 7: 
			c = minmax(1,30,c);
			start_shop_mode(2,b,
				b + c - 1,a,(char *)place_string1);
			strnum1 = -1;
			return;
		case 8: 
			if ((get_pc = char_select_pc(1,0,"Train who?")) < 6) {
				strnum1 = -1;
				spend_xp(get_pc,1, 0);
				}
			sprintf((char *) place_string1, "You conclude your training.");  
			return;
		
		case 9: case 10: case 11: 
			c = minmax(1,30,c);
			start_shop_mode(ttype + 1,b,
				b + c - 1,a,(char *)place_string1);
			strnum1 = -1;
			return;
		case 12: //healer
			start_shop_mode(3,c_town.monst.dudes[store_m_num].monst_start.extra1,
				c_town.monst.dudes[store_m_num].monst_start.extra2,a,(char *)place_string1);
			strnum1 = -1;
			return;
			break;
		case 13: // sell weap
			strnum1 = -1;
			stat_screen_mode = 3;
			put_item_screen(stat_window,1);
			give_help(42,43,0);
			break;
		case 14: // sell armor
			strnum1 = -1;
			stat_screen_mode = 4;
			put_item_screen(stat_window,1);
			give_help(42,43,0);
			break;
		case 15: // sell misc
			strnum1 = -1;
			stat_screen_mode = 5;
			put_item_screen(stat_window,1);
			give_help(42,43,0);
			break;
		case 16: case 17: // ident enchant
				strnum1 = -1;
			stat_screen_mode = (ttype == 16) ? 2 : 6;
			shop_identify_cost = a;
			put_item_screen(stat_window,1);
			give_help(ttype - 16 + 44,0,0);
			break;
		case 18:
			if (party.gold < a) {
				strnum1 = strnum2;
				strcpy((char *) place_string1,(char *) place_string2);
				}
				else {
					party.gold -= a;
					put_pc_screen();

					}
			sprintf((char *) place_string2,"");					
			strnum2 = 0;
			break;
		case 19:
			if ((sd_legit(b,c) == TRUE) && (PSD[b][c] == d)) {
				sprintf((char *) place_string1, "You've already learned that.");  
				strnum1 = -1;
				}
			else if (party.gold < a) {
				strnum1 = strnum2;
				strcpy((char *) place_string1,(char *) place_string2);
				}
				else {
					party.gold -= a;
					put_pc_screen();
					if (sd_legit(b,c) == TRUE)
						PSD[b][c] = d;
						else give_error("Invalid Stuff Done flag called in conversation.","",0);
					}
			strnum2 = 0;
			sprintf((char *) place_string2,"");					
			break;
		case 20:
			if (party.gold < a) {
				strnum1 = strnum2;
				strnum2 = 0;
				strcpy((char *) place_string1,(char *) place_string2);
				sprintf((char *) place_string2,"");					
				break;
				}
				else {
					for (i = b; i <= b + c; i++)
						if ((i >= 0) && (i < 30) && (party.boats[i].property == TRUE)) {
							party.gold -= a;
							put_pc_screen();
							party.boats[i].property = FALSE;
							sprintf((char *) place_string2,"");					
							strnum2 = 0;
							i = 1000;
							}
					if (i >= 1000)
						break;
					}
			sprintf((char *) place_string1, "There are no boats left.");  
			sprintf((char *) place_string2,"");					
			strnum1 = -1;
			strnum2 = -1;
			break;
		case 21:
			if (party.gold < a) {
				strnum1 = strnum2;
				strnum2 = 0;
				strcpy((char *) place_string1,(char *) place_string2);
				sprintf((char *) place_string2,"");					
				break;
				}
				else {
					for (i = b; i <= b + c; i++)
						if ((i >= 0) && (i < 30) && (party.horses[i].property == TRUE)) {
							party.gold -= a;
							put_pc_screen();
							party.horses[i].property = FALSE;
							sprintf((char *) place_string2,"");					
							strnum2 = 0;
							i = 1000;
							}
					if (i >= 1000)
						break;
					}
			sprintf((char *) place_string1, "There are no horses left.");  
			sprintf((char *) place_string2,"");					
			strnum1 = -1;
			strnum2 = -1;
			break;
		case 22:
			if (party.spec_items[a] > 0) {
				sprintf((char *) place_string1, "You already have it.");  
				strnum1 = -1;
				}
			else if (party.gold < b) {
				strcpy((char *) place_string1,(char *) place_string2);
				strnum1 = strnum2;
				}
				else {
					party.gold -= b;
					put_pc_screen();
					party.spec_items[a] = 1;
					}
			strnum2 = 0;
			sprintf((char *) place_string2,"");					
			break;
		case 23:
			start_shop_mode(5 + b,0,
				9,a,(char *)place_string1);
			strnum1 = -1;
			return;
		case 24:		
			if (party.can_find_town[b] > 0) {
				}
				else if (party.gold < a) {
					strnum1 = strnum2;
					strcpy((char *) place_string1,(char *) place_string2);
					}
					else {
						party.gold -= a;
						put_pc_screen();
						party.can_find_town[b] = 1;
						}
			strnum2 = 0;
			sprintf((char *) place_string2,"");					
			break;
		case 25:
			talk_end_forced = TRUE;
			break;	
		case 26:
			c_town.monst.dudes[store_m_num].attitude = 1;
			c_town.monst.dudes[store_m_num].mobile = 1;
			talk_end_forced = TRUE;
			break;	
		case 27:
			make_town_hostile();
			talk_end_forced = TRUE;
			break;	
		case 28:
			c_town.monst.dudes[store_m_num].active = 0;
	// Special killing effects
	if (sd_legit(c_town.monst.dudes[store_m_num].monst_start.spec1,c_town.monst.dudes[store_m_num].monst_start.spec2) == TRUE)
		party.stuff_done[c_town.monst.dudes[store_m_num].monst_start.spec1][c_town.monst.dudes[store_m_num].monst_start.spec2] = 1;
			talk_end_forced = TRUE;
			break;	
		case 29: // town special
			 run_special(7,2,a,c_town.p_loc,&s1,&s2,&s3);
			// check s1 & s2 to see if we got diff str, and, if so, munch old strs
			if ((s1 >= 0) || (s2 >= 0)) {
				strnum1 = -1;
				strnum2 = -1;
				sprintf((char *) place_string1,"");
				sprintf((char *) place_string2,"");
				}
			 get_strs((char *) place_string1,(char *) place_string2,2,s1,s2); 
			 //strnum1 = -1;
			 //strnum2 = -1;
			 if (s1 >= 0) strnum1 = 2000 + s1;
			 if (s2 >= 0) strnum2 = 2000 + s2;
			put_pc_screen();
			put_item_screen(stat_window,0);
			 break;	
		case 30: // scen special
			 run_special(7,0,a,c_town.p_loc,&s1,&s2,&s3);
			// check s1 & s2 to see if we got diff str, and, if so, munch old strs
			if ((s1 >= 0) || (s2 >= 0)) {
				strnum1 = -1;
				strnum2 = -1;
				sprintf((char *) place_string1,"");
				sprintf((char *) place_string2,"");
				}
			 get_strs((char *) place_string1,(char *) place_string2,0,s1,s2); 
			 //strnum1 = -1;
			 //strnum2 = -1;
			 if (s1 >= 0) strnum1 = 3000 + s1;
			 if (s2 >= 0) strnum2 = 3000 + s2;
			put_pc_screen();
			put_item_screen(stat_window,0);
			break;	
		
			
			
		}

	strcpy((char *) one_back1,(char *) old_str1);
	strcpy((char *) one_back2,(char *) old_str2);
	strcpy((char *) old_str1,(char *) place_string1);
	strcpy((char *) old_str2,(char *) place_string2);
	place_talk_str((char *) old_str1,(char *) old_str2,0,dummy_rect);
	
}

void handle_talk_spec(short ttype,char *place_string1,char *place_string2) ////
{
	short s1 = 1, s2 = 0,num_horns = 0, num_items = 0,i,j;
	item_record_type store_i;
	
	strnum1 = 1000 + s1;
	get_str(place_string1,15,s1);
	if (s2 > 0) {
		strnum2 = 1000 + s2;
		get_str(place_string2,15,s2);
		}
	print_buf();
	put_pc_screen();
}



void store_responses()
{}



void sign_event_filter (short item_hit)
{
	dialog_not_toast = FALSE;
}

void do_sign(short town_num, short which_sign, short sign_type,location sign_loc)
//town_num; // Will be 0 - 200 for town, 200 - 290 for outdoors
//short sign_type; // terrain type
{

	short item_hit;
	char sign_text[256];
	location view_loc;

	view_loc = (is_out()) ? party.p_loc : c_town.p_loc;
	make_cursor_sword();
	
	cd_create_dialog(1014,mainPtr);
	
	store_sign_mode = sign_type;
	if (terrain_pic[sign_type] < 1000)
		csp(1014,3,terrain_pic[sign_type]);
		else csp(1014,3,94);
	
	if (town_num >= 200) {
		town_num -= 200;
		load_outdoors(town_num % scenario.out_width, town_num / scenario.out_width,party.i_w_c.x,party.i_w_c.y,
			1,which_sign + 100,(char *) sign_text);
		}
		else {
			sprintf((char *) sign_text,"%s",data_store->town_strs[120 + which_sign]);
			}
	csit(1014,2,(char *) sign_text);
	
	while (dialog_not_toast)
		ModalDialog();	
	cd_kill_dialog(1014,0);
}



void give_reg_info_event_filter (short item_hit)
{
	short i;
	char place_str[256];
	
			switch (item_hit) {
				case 1: 
					dialog_not_toast = FALSE;
					break;
				}
}

void give_reg_info()
{
	long val_for_text;

	short i,item_hit;
	char place_str[256];
	

	make_cursor_sword();

	cd_create_dialog_parent_num(1073,0);

	while (dialog_not_toast)
		ModalDialog();	cd_kill_dialog(1073,0);

}




void do_registration_event_filter (short item_hit)
{
	char get_text[256];
	long dummy;
	
	cd_get_text_edit_str(1075,(char *) get_text);
	dialog_answer = 0;
	sscanf((char *) get_text,"%d",&dialog_answer);
	dialog_not_toast = FALSE;
}

void do_registration()
{

	short item_hit;
	char sign_text[256];
	location view_loc;

	make_cursor_sword();
	
	cd_create_dialog(1075,mainPtr);
		
	cdsin(1075,7,(short) register_flag);	
	cd_set_edit_focus();

	while (dialog_not_toast)
		ModalDialog();	
	cd_kill_dialog(1075,0);
	
	if (dialog_answer == 1)
		SysBeep(30);
	
	if (dialog_answer == (short) init_data(register_flag)) {
		play_sound(40);
		registered = TRUE;
		build_data_file(2);
		FCD(1078,0);
		//save_prefs();
		//check_pref_file();
		}
		else {
			SysBeep(30);
			FCD(1077,0);
			}
	
	
}


Boolean prefs_event_filter (short item_hit)
{
	Boolean done_yet = FALSE,did_cancel = FALSE;
	short i;
	 RECT windRECT;

		switch (item_hit) {
			case 1:
				done_yet = TRUE;
				dialog_not_toast = FALSE;
				break;

			case 2:
				done_yet = TRUE;
				dialog_not_toast = FALSE;
				did_cancel = TRUE;
				break;

			case 50: case 52: case 18: case 20: case 22: case 24: case 27: case 38: case 40: case 43: case 45:
				cd_set_led(1099,item_hit,1 - cd_get_led(1099,item_hit));
				break;
			
			case 29:
				cd_set_led(1099,29,1);
				break;
			
			case 32: case 34: case 36: case 47:
				cd_set_led(1099,32,(item_hit == 32) ? 1 : 0);
				cd_set_led(1099,34,(item_hit == 34) ? 1 : 0);
				cd_set_led(1099,36,(item_hit == 36) ? 1 : 0);
				cd_set_led(1099,47,(item_hit == 47) ? 1 : 0);
				break;
				
			
			default:
				cd_set_led(1099,4 + cur_display_mode,0);
				cur_display_mode = item_hit - 4;
				cd_set_led(1099,4 + cur_display_mode,1);
				break;
			}
	if (done_yet== TRUE) {
		if (did_cancel == FALSE) {
			display_mode = cur_display_mode;
			party.stuff_done[306][0] = cd_get_led(1099,18);
			party.stuff_done[306][1] = cd_get_led(1099,20);
			play_sounds = 1 - party.stuff_done[306][1];
			party.stuff_done[306][2] = cd_get_led(1099,22);
			party.stuff_done[306][3] = cd_get_led(1099,24);
			party.stuff_done[306][4] = cd_get_led(1099,27);
			party.stuff_done[306][7] = cd_get_led(1099,38);
			party.stuff_done[306][8] = cd_get_led(1099,40);
			party.stuff_done[306][9] = cd_get_led(1099,43);
			party.stuff_done[296][0] = cd_get_led(1099,45);
			party.stuff_done[305][6] = cd_get_led(1099,50);
			party.stuff_done[305][5] = cd_get_led(1099,52);
			if (cd_get_led(1099,32) == 1)
				PSD[306][6] = 0;
			if (cd_get_led(1099,34) == 1)
				PSD[306][6] = 1;
			if (cd_get_led(1099,36) == 1)
				PSD[306][6] = 2;
			if (cd_get_led(1099,47) == 1)
				PSD[306][6] = 3;
			if (cd_get_led(1099,29) == 1)
				for (i = 0; i < 120; i++)
					party.help_received[i] = 0;
			if (display_mode < 5)
				max_window(mainPtr);
				else {
					GetWindowRect(GetDesktopWindow(),&windRECT);
					MoveWindow(mainPtr,(windRECT.right - (588 + 10)) / 2,
					(windRECT.bottom - (425 + 52)) / 2 ,
					588  + 10,425 + 52,TRUE);
					}
			}
		//play_sounds = 1 - party.stuff_done[306][1];
		save_maps = 1 - party.stuff_done[306][0];
		give_delays = party.stuff_done[306][2];
		build_data_file(2);
		}

	return FALSE;
}

void pick_preferences()
{
	short item_hit;

	cur_display_mode = display_mode;

	make_cursor_sword();

	cd_create_dialog(1099,mainPtr);

	cd_set_led(1099,4 + cur_display_mode,1);

	cd_set_led(1099,18,(party.stuff_done[306][0] != 0) ? 1 : 0);
	cd_set_led(1099,20,(play_sounds == FALSE) ? 1 : 0);
	cd_set_led(1099,22,(party.stuff_done[306][2] != 0) ? 1 : 0);
	cd_set_led(1099,24,(party.stuff_done[306][3] != 0) ? 1 : 0);
	cd_set_led(1099,27,(party.stuff_done[306][4] != 0) ? 1 : 0);
	cd_set_led(1099,38,(party.stuff_done[306][7] != 0) ? 1 : 0);
	cd_set_led(1099,40,(party.stuff_done[306][8] != 0) ? 1 : 0);
	cd_set_led(1099,43,(party.stuff_done[306][9] != 0) ? 1 : 0);
	cd_set_led(1099,45,(party.stuff_done[296][0] != 0) ? 1 : 0);
	cd_set_led(1099,50,(party.stuff_done[305][6] != 0) ? 1 : 0);
	cd_set_led(1099,52,(party.stuff_done[305][5] != 0) ? 1 : 0);
		cd_set_flag(1099,3,1);
	if (PSD[306][6] == 3) 
		cd_set_led(1099,47,1);
		else cd_set_led(1099,32 + PSD[306][6] * 2,1);

	if (party.help_received[55] == 0) {
		cd_initial_draw(1099);
		give_help(55,0,1099);
		}

	while (dialog_not_toast)
		ModalDialog();
	cd_kill_dialog(1099,0);
	adjust_window_mode();
}

void put_party_stats()
{
	short i;
	char pc_str[256];


	for (i = 0; i < 6; i++) {
		if (adven[i].main_status > 0) {
			cd_set_item_text(989,5 + 5 * i,adven[i].name);
			cd_activate_item(989,7 + 5 * i,1);
			cd_activate_item(989,8 + 5 * i,1);
			cd_set_flag(989,6 + 5 * i,96);
			cd_activate_item(989,35 + i,1);
			cd_activate_item(989,42 + i,1);
			csp(989,42 + i,800 + adven[i].which_graphic);
			}
			else {
				cd_set_item_text(989,5 + 5 * i,"Empty.");
				cd_activate_item(989,7 + 5 * i,0);
				cd_activate_item(989,8 + 5 * i,0);
				cd_set_flag(989,6 + 5 * i,98);
				cd_activate_item(989,35 + i,0);
				cd_activate_item(989,42 + i,0);
				}

	}
}

void edit_party_event_filter (short item_hit)
{
	short i = 0,j = 0,which_pc;

		switch (item_hit) {
			case 1:
				dialog_not_toast = FALSE;
				break;

			case 41:
				party.help_received[22] = 0;
				give_help(222,23,989);
				return;

			case 5: case 10: case 15: case 20: case 25: case 30:
				which_pc = (item_hit - 5) / 5;
				if (adven[which_pc].main_status == 0) {
					//give_help(56,0,989);
					//create_pc(which_pc,989);
					//cd_initial_draw(989); // extra redraw, just in case
					}
					else pick_pc_name(which_pc,989);
				put_party_stats();
				break;
			case 7: case 12: case 17: case 22: case 27: case 32:
				which_pc = (item_hit - 7) / 5;
				pick_race_abil(&adven[which_pc],0,989);
				put_party_stats();
				break;
			case 8: case 13: case 18: case 23: case 28: case 33:
				which_pc = (item_hit - 8) / 5;
				spend_xp(which_pc,0,989);
				put_party_stats();
				break;
			case 35: case 36: case 37: case 38: case 39: case 40:
				which_pc = item_hit - 35;

				if (adven[which_pc].main_status <= 0)
					break;
				if (adven[which_pc].main_status != 0)
					pick_pc_graphic(which_pc,1,989);
				put_party_stats();
				break;
			case 6: case 11: case 16: case 21: case 26: case 31:
				which_pc = (item_hit - 6) / 5;
				if (adven[which_pc].main_status != 0) {
					if (FCD(1053,989) == 2)
						adven[which_pc].main_status = 0;
					put_party_stats();
					}
					else {
						give_help(56,0,989);
						create_pc(which_pc,989);
						put_party_stats();
						cd_initial_draw(989); // extra redraw, just in case
						}
				break;
//			case 7: case 12: case 17: case 22: case 27: case 32:

			}

}

void edit_party(short can_create,short can_cancel)
{
	short item_hit;
	Boolean munch_pc_graphic = FALSE;
	
	make_cursor_sword();

	if (pcs_gworld == NULL) {
		munch_pc_graphic = TRUE;
		pcs_gworld = load_pict(902,main_dc);
		}	

	cd_create_dialog(989,mainPtr);

	put_party_stats();
	if (party.help_received[22] == 0) {
		cd_initial_draw(989);
		give_help(22,23,989);
		}
	while (dialog_not_toast)
		ModalDialog();
	cd_kill_dialog(989,0);

	if (munch_pc_graphic == TRUE) {
		DisposeGWorld(pcs_gworld);
		pcs_gworld = NULL;
		}

	if (adven[current_pc].main_status != 1)
		current_pc = first_active_pc();


}

void tip_of_day_event_filter (short item_hit)
{
	short i;
	char place_str[256];
	
			switch (item_hit) {
				case 1: 
					dialog_not_toast = FALSE;
					break;
				
				case 5:
					store_tip_page_on++;
					if (store_tip_page_on == NUM_HINTS)
						store_tip_page_on = 0;
					get_str(place_str,12,50 + store_tip_page_on);
					csit(958,3,(char *) place_str);
					break;

				case 7:
					give_intro_hint = 1 - give_intro_hint;
					cd_set_led(958,7,give_intro_hint);
					break;
			}

}

void tip_of_day()
{
	long val_for_text;

	short i,item_hit;
	char place_str[256];
	
	store_tip_page_on = get_ran(1,0,NUM_HINTS - 1);
	
	make_cursor_sword();

	cd_create_dialog_parent_num(958,0);

	get_str(place_str,12,50 + store_tip_page_on);
	csit(958,3,(char *) place_str);

	cd_set_led(958,7,give_intro_hint);

	while (dialog_not_toast)
		ModalDialog();	
		
	cd_kill_dialog(958,0);
	build_data_file(2);

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

void put_scen_info()
{
	short i;
	char place_str[256];
	char *ratings[] = {"G","PG","R","NC-17"};
	char *difficulty[] = {"Low","Medium","High","Very High"};

	for (i = 0; i < 3; i++)
		if (scen_headers[store_scen_page_on * 3 + i].flag1 != 0) {
			cd_set_pict(947, 6 + i * 3,1600 + scen_headers[store_scen_page_on * 3 + i].intro_pic);
			sprintf((char *) place_str,
				"%s v%d.%d.%d - |  Difficulty: %s, Rating: %s |%s |%s",
				data_store2->scen_header_strs[store_scen_page_on * 3 + i][0],
				(short) scen_headers[store_scen_page_on * 3 + i].ver[0],
				(short) scen_headers[store_scen_page_on * 3 + i].ver[1],
				(short) scen_headers[store_scen_page_on * 3 + i].ver[2],
				difficulty[scen_headers[store_scen_page_on * 3 + i].difficulty],
				ratings[scen_headers[store_scen_page_on * 3 + i].default_ground],
				data_store2->scen_header_strs[store_scen_page_on * 3 + i][1],
				data_store2->scen_header_strs[store_scen_page_on * 3 + i][2]); 
			csit(947,7 + i * 3,(char *) place_str);
			cd_activate_item(947,8 + i * 3,1);			
			}
			else {
				cd_set_pict(947, 6 + i * 3,950);
				csit(947,7 + i * 3,"");
				cd_activate_item(947,8 + i * 3,0);
				}
}

void pick_a_scen_event_filter (short item_hit)
{
	short i;
	char place_str[256];
	
			switch (item_hit) {
				case 1: 
					dialog_answer = -1;
					dialog_not_toast = FALSE;
					break;
				
				case 3: case 4:
					if (item_hit == 3) {
						if (store_scen_page_on == 0)
							store_scen_page_on = (store_num_scen - 1) / 3;
							else store_scen_page_on--;
						}
						else {
							if (store_scen_page_on == (store_num_scen - 1) / 3)
								store_scen_page_on = 0;
								else store_scen_page_on++;
							}					
					put_scen_info();
					break;
				
				case 8: case 11: case 14:
					dialog_answer = ((item_hit - 8) / 3) + store_scen_page_on * 3;
					dialog_not_toast = FALSE;
					break;
				}

}

short pick_a_scen()
{
	long val_for_text;

	short i,item_hit;
	char place_str[256];
	
	build_scen_headers();
	
	store_num_scen = 0;
	for (i = 0; i < 25; i++)
		if (scen_headers[i].flag1 != 0)
			store_num_scen++;
	store_scen_page_on = 0;
	
	if (store_num_scen == 0) {
		FCD(868,0);
		return -1;
		}
	make_cursor_sword();

	cd_create_dialog_parent_num(947,0);

	put_scen_info();
	
	if (store_num_scen <= 3) {
		cd_activate_item(947,3,0);
		cd_activate_item(947,4,0);
		}
	while (dialog_not_toast)
		ModalDialog();
	cd_kill_dialog(947,0);
	return dialog_answer;
}


void pick_prefab_scen_event_filter (short item_hit)
{
	short i;
	char place_str[256];
	
			switch (item_hit) {
				case 1: 
					dialog_answer = -1;
					dialog_not_toast = FALSE;
					break;
				
				case 6: case 9: case 12:
					dialog_answer = ((item_hit - 6) / 3);
					dialog_not_toast = FALSE;
					break;
				}

}

short pick_prefab_scen()
{
	long val_for_text;

	short i,item_hit;
	char place_str[256];
	
	make_cursor_sword();

	cd_create_dialog_parent_num(869,0);
	cd_activate_item(869,2,0);

	while (dialog_not_toast)
		ModalDialog();
	cd_kill_dialog(869,0);
	return dialog_answer;
}

void give_password_filter (short item_hit)
{
	char str[256];
	short i;
	
	switch (item_hit) {
		default:

			dialog_not_toast = FALSE; 
			break;

		}
}

Boolean enter_password()
// ignore parent in Mac version
{
	short town_strs_hit,i,store_dialog_answer;
	char temp_str[256],str2[256];
		
	cd_create_dialog_parent_num(823,0);
	
	cd_set_text_edit_str(823,"");
	
	while (dialog_not_toast)
		ModalDialog();

	cd_get_text_edit_str(823,(char *) temp_str);
	i = wd_to_pwd(temp_str);
	
	cd_kill_dialog(823,0);
	
	return check_p(i);
}

short wd_to_pwd(char *str)
{
	char pwd[8] = "aaaaaa";
	short i;
	long val = 0,pow[6] = {1,10,100,1000,9999,99999};
	
	for (i = 0; i < 6; i++) {
		if (str[i] == 0) 
			i = 6;
			else {
				if ((str[i] >= 65) && (str[i] <= 90))
					pwd[i] = str[i] + 32;
				else if ((str[i] >= 48) && (str[i] <= 57))
					pwd[i] = str[i] + 49;
				else if ((str[i] >= 97) && (str[i] <= 122))
					pwd[i] = str[i];
				}
		}
	for (i = 0; i < 6; i++)
		val = val + pow[i] * (long) (pwd[i] - 97);
	val = val % 30000;
	return (short) val;
}
