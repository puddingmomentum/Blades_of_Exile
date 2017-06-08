#include <Windows.h>
#include "string.h"
#include "stdio.h"
#include "Global.h"
#include "graphics.h"
#include "dlogtool.h"
#include "keydlgs.h"
#include "scenario.h"
#include "edsound.h"

extern short cen_x, cen_y, overall_mode;
extern Boolean mouse_button_held,dialog_not_toast;
extern short cur_viewing_mode;
extern town_record_type far town;
extern big_tr_type far t_d;
extern short town_type;  // 0 - big 1 - ave 2 - small
extern short max_dim[3],mode_count,to_create;
extern unsigned char template_terrain[64][64];
extern short_item_record_type item_list[400];
extern scenario_data_type far scenario;
extern special_node_type null_spec_node;
extern talking_node_type null_talk_node;
extern piles_of_stuff_dumping_type *data_store;
extern outdoor_record_type far current_terrain;
extern HANDLE store_hInstance;

extern char far scen_strs[160][256];
extern char far scen_strs2[110][256];
extern char far talk_strs[170][256];
extern char far town_strs[180][256];

extern short dialog_answer;
short store_which_string_dlog;
short store_first_g ;
short store_last_g ;
short store_cur_pic ;
short which_page ;
short store_res_list;
short store_first_t ;
short store_last_t ;
short store_cur_t ;
short store_which_str ;
short store_str_mode ;
short store_which_mode,*store_str1,*store_str2;
short store_which_node,store_spec_str_mode,store_spec_mode;
short last_node[256];
special_node_type store_spec_node;
short num_specs[3] = {256,60,100};
HCURSOR cursors[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
short current_cursor = 0;

	short ex1a_choose[12] = {19,50,55,56,57,58,59,60,182,229,-1,-1};
	short ex2a_choose[18] = {55,56,57,-1,-1, -1,135,136,171,172, 181,192,226,-1,-1, -1,-1,-1};
	short ex1b_choose[40] = {13,20,55,56,57,24,-1,-1,80,130, 131,132,133,137,138,140,141,142,143,145, 
146,147,148,149,150,151,152,153,154,184, 188,195,186,-1,-1,-1,-1,-1,-1,155};
short ex2b_choose[20] = {19,50,55,56,57,58,59,60,130,134, 
			135,136,139,144,154,-1,-1,-1,-1,-1};

char edit_spec_stuff_done_mess[256] = {
	0,1,1,0,0,0,1,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,3,1,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1, // 50
	1,1,1,1,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0, // 100
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	1,0,0,0,1,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,1,0,0,0,0, // 150
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	2,2,2,2,2,2,2,2,2,2, // 200
	7,8,4,0,5,6,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0};
char edit_spec_mess_mess[256] = {
	0,1,1,1,0,1,1,0,0,0,
	0,1,1,0,0,0,0,1,1,1,
	1,0,0,0,0,1,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	1,1,0,0,1,4,4,4,5,5, // 50
	5,1,1,1,0,0,0,0,0,0,	
	0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,0,0,0, // 100
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0, // 150
	0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,0,0,0,0,2,2,
	2,1,1,1,1,1,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1, // 200
	1,1,1,1,1,1,1,1,1,0,
	0,0,0,0,0,0,1,1,1,3,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0};					
char edit_pict_mess[256] = {
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,1,2,3,1,2, // 50
	3,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,4,0,
	0,0,0,0,0,0,0,0,0,0, // 100
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,5,0,0,0,0,0, // 150
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,6,0,0,0,
	0,0,0,0,0,0,0,0,2,1,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0, // 200
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0};
char edit_jumpto_mess[256] = {
	0,0,0,0,0,0,0,1,1,1,
	1,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,4,4,4,0,0, // 50
	0,0,0,2,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0, // 100
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	3,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,0,0,0,0,0, // 150
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0, // 200
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0};
			
	
void fancy_choice_dialog_event_filter (short item_hit)
{
	dialog_not_toast = FALSE;
	dialog_answer = item_hit;
}

short fancy_choice_dialog(short which_dlog,short parent)
// ignore parent in Mac version
{
	short item_hit,i,store_dialog_answer;
	char temp_str[256];
	
	store_dialog_answer = dialog_answer;
	//make_cursor_sword();
	
	cd_create_dialog_parent_num(which_dlog,parent);


	while (dialog_not_toast)
		ModalDialog();

	cd_kill_dialog(which_dlog,0);


	i = dialog_answer;
	dialog_answer = store_dialog_answer;
	
	return i;
}



//cre = check range error
Boolean cre(short val,short min,short max,char *text1, char *text2,short parent_num) 
{
	if ((val < min) || (val > max)) {
		give_error(text1,text2,parent_num);
		return TRUE;
		}		
	return FALSE;
}

void give_error(char *text1, char *text2,short parent_num)
{
	display_strings(text1,text2,"Error!",57,716,parent_num);
}

void display_strings_event_filter (short item_hit)
{
	short i;
	Boolean had1 = FALSE, had2 = FALSE;
	
	switch (item_hit) {
		case 1:
			dialog_not_toast = FALSE;
			break;

		}
}

void display_strings(char *text1, char *text2,
	char *title,short sound_num,short graphic_num,short parent_num)
{

	short item_hit;
	char sign_text[256];
	location view_loc;
	Boolean sound_done = FALSE;

	//make_cursor_sword();
		
	if ((text1 == NULL) && (text2 == NULL))
		return;
	store_which_string_dlog = 970;
	if (strlen(title) > 0)
		store_which_string_dlog += 2;
	if ((text2 != NULL) && (text2[0] != 0))
		store_which_string_dlog++;
	cd_create_dialog_parent_num(store_which_string_dlog,parent_num);
	
	csp(store_which_string_dlog,store_which_string_dlog,graphic_num);
	
	csit(store_which_string_dlog,4,(char *) text1);
	if (text2 != NULL) {
		csit(store_which_string_dlog,5,(char *) text2);
		}
	if (strlen(title) > 0)
		csit(store_which_string_dlog,6,title);
	csp(store_which_string_dlog,3,graphic_num);
	//if (sound_num >= 0)
	//	play_sound(sound_num);
	while (dialog_not_toast)
		ModalDialog();	
	
	cd_kill_dialog(store_which_string_dlog,0);
}

void choose_graphic_event_filter (short item_hit)
{
	short i;
	Boolean had1 = FALSE, had2 = FALSE;
	
	switch (item_hit) {
		case 1:
			dialog_answer = store_cur_pic;
			dialog_not_toast = FALSE;
			break;
		case 4:
			dialog_answer = -1;
			dialog_not_toast = FALSE;
			break;
		case 78:
			if (which_page == 0)
				which_page = (store_last_g - store_first_g) / 36;
				else which_page--;
			put_choice_pics();
			break;
		case 79:
			if (which_page == (store_last_g - store_first_g) / 36)
				which_page = 0;
				else which_page++;
			put_choice_pics();		
			break;
		default:
			if ((item_hit >= 5) && (item_hit <= 40)) {
				store_cur_pic = 36 * which_page + item_hit - 5 + store_first_g;
				for (i = 5; i <= 40; i++) 
					cd_set_led(819,i,(i == item_hit) ? 1 : 0);
				}
			break;
		}
}

void put_choice_pics()
{
	short item_hit,i;
	
	for (i = 0; i < 36; i++) {
		if (store_first_g + which_page * 36 + i > store_last_g) {
			csp(819,41 + i,950);
			cd_activate_item(819,5 + i,0);
			}
			else {
				csp(819,41 + i,store_first_g + 36 * which_page + i);
				cd_activate_item(819,5 + i,1);				
				}
		if (which_page * 36 + i == store_cur_pic - store_first_g)
			cd_set_led(819,5 + i,1);
			else cd_set_led(819,5 + i,0);
		}

}

short choose_graphic(short first_g,short last_g,short cur_choice,short parent_num)
{

	short item_hit;
	location view_loc;
	Boolean sound_done = FALSE;

	//make_cursor_sword();
	
	store_first_g = first_g;
	store_last_g = last_g;
	if ((cur_choice >= first_g) && (cur_choice <= last_g))
		store_cur_pic = cur_choice;
		else store_cur_pic = first_g;
	which_page = (store_cur_pic - store_first_g) / 36;

	cd_create_dialog_parent_num(819,parent_num);

	if (last_g - first_g < 36) {
		cd_activate_item(819,79,0);
		cd_activate_item(819,78,0);
		}
	put_choice_pics();
		
	while (dialog_not_toast)
		ModalDialog();	
	
	cd_kill_dialog(819,0);
	
	return dialog_answer;
}

void choose_text_res_event_filter (short item_hit)
{
	short i;
	Boolean had1 = FALSE, had2 = FALSE;
	
	switch (item_hit) {
		case 2:
			dialog_answer = store_cur_t;
			dialog_not_toast = FALSE;
			break;
		case 7:
			dialog_answer = -1;
			dialog_not_toast = FALSE;
			break;
		case 5:
			if (which_page == 0)
				which_page = (store_last_t - store_first_t) / 40;
				else which_page--;
			put_text_res();
			break;
		case 6:
			if (which_page == (store_last_t - store_first_t) / 40)
				which_page = 0;
				else which_page++;
			put_text_res();		
			break;
		default:
			if ((item_hit >= 9) && (item_hit <= 87)) {
				store_cur_t = 40 * which_page + (item_hit - 9) / 2 + store_first_t;
				for (i = 9; i <= 87; i += 2) 
					cd_set_led(820,i,(i == item_hit) ? 1 : 0);
				}
			break;
		}
}

void put_text_res()
{
	short item_hit,i;
	char str[256];
	
	for (i = 0; i < 40; i++) {
		if (store_first_t + which_page * 40 + i > store_last_t) {
			csit(820,8 + i * 2,"");
			cd_activate_item(820,9 + i * 2,0);
			}
			else {
				get_str(str,store_res_list,store_first_t + 40 * which_page + i);
				csit(820,8 + i * 2,(char *) str);
				cd_activate_item(820,9 + i * 2,1);
				}
		if (which_page * 40 + i == store_cur_t - store_first_t)
			cd_set_led(820,9 + i * 2,1);
			else cd_set_led(820,9 + i * 2,0);
		}

}

short choose_text_res(short res_list,short first_t,short last_t,short cur_choice,short parent_num,char *title)
{

	short item_hit;
	char sign_text[256];
	location view_loc;
	Boolean sound_done = FALSE;

	//make_cursor_sword();
	store_res_list = res_list;
	store_first_t = first_t;
	store_last_t = last_t;
	if ((cur_choice >= first_t) && (cur_choice <= last_t))
		store_cur_t = cur_choice;
		else store_cur_t = first_t;
	which_page = (store_cur_t - store_first_t) / 40;

	cd_create_dialog_parent_num(820,parent_num);

	csit(820,4,title);
	if (last_t - first_t < 40) {
		cd_activate_item(820,5,0);
		cd_activate_item(820,6,0);
		}
	put_text_res();
		
	while (dialog_not_toast)
		ModalDialog();	
	
	cd_kill_dialog(820,0);
	
	return dialog_answer;
}

void edit_text_event_filter (short item_hit)
{
	char str[256];
	short i;
	short num_strs[3] = {260,108,140};
	
			if (store_str_mode == 0)
				CDGT(816,2,(store_which_str >= 160) ? 
					scen_strs2[store_which_str - 160] : scen_strs[store_which_str]);
			if (store_str_mode == 1)
				CDGT(816,2,data_store->out_strs[store_which_str]);
			if (store_str_mode == 2)
				CDGT(816,2,town_strs[store_which_str]);
	switch (item_hit) {
		case 9:
			dialog_not_toast = FALSE; 
			break;

		case 4:
		case 3:
			if (item_hit == 3)
				store_which_str--;
				else store_which_str++;
			if (store_which_str < 0) store_which_str = num_strs[store_str_mode] - 1;
			if (store_which_str >= num_strs[store_str_mode]) store_which_str = 0;
			break;
		}
	cdsin(816,5,store_which_str);
	if (store_str_mode == 0)
		CDST(816,2,(store_which_str >= 160) ? 
					scen_strs2[store_which_str - 160] : scen_strs[store_which_str]);
	if (store_str_mode)
		CDST(816,2,data_store->out_strs[store_which_str]);
	if (store_str_mode == 2)
		CDST(816,2,town_strs[store_which_str]);
}

// mode 0 - scen 1 - out 2 - town
void edit_text_str(short which_str,short mode)
// ignore parent in Mac version
{
	short text_hit,i,store_dialog_answer,item_hit;
	char temp_str[256];
	
	store_which_str = which_str;
	store_str_mode = mode;
	
	cd_create_dialog_parent_num(816,0);
	
	cdsin(816,5,store_which_str);
	if (mode == 0)
		CDST(816,2,(which_str >= 160) ? 
					scen_strs2[which_str - 160] : scen_strs[which_str]);
	if (mode == 1)
		CDST(816,2,data_store->out_strs[which_str]);
	if (mode == 2)
		CDST(816,2,town_strs[which_str]);
	cd_attach_key(816,3,0);
	cd_attach_key(816,4,0);
	while (dialog_not_toast)
		ModalDialog();	

	cd_kill_dialog(816,0);
}

void edit_area_rect_event_filter (short item_hit)
{
	char str[256];
	short i;
	
	switch (item_hit) {
		case 6:
			dialog_answer = TRUE;
			dialog_not_toast = FALSE; 
			CDGT(840,2,(char *) str);
			if (store_str_mode == 0)
				sprintf(data_store->out_strs[store_which_str + 1],"%-29.29s",(char *) str);
				else sprintf(town_strs[store_which_str + 1],"%-29.29s",(char *) str);
			break;

		case 3:
			dialog_answer = FALSE;
			dialog_not_toast = FALSE; 
			break;
		}
}

// mode 0 - out 1 - town
Boolean edit_area_rect_str(short which_str,short mode)
// ignore parent in Mac version
{
	short area_rect_hit,i,store_dialog_answer,item_hit;
	char temp_str[256];
	
	store_which_str = which_str;
	store_str_mode = mode;
	
	cd_create_dialog_parent_num(840,0);
	
	if (store_str_mode == 0)
		CDST(840,2,data_store->out_strs[store_which_str + 1]);
		else CDST(840,2,town_strs[store_which_str + 1]);
		
	while (dialog_not_toast)
		ModalDialog();	

	cd_kill_dialog(840,0);
	
	return dialog_answer;
}

Boolean save_spec_enc()
{
	char str[256];
	short i;
	
	store_spec_node.sd1 = CDGN(822,2);
	store_spec_node.sd2 = CDGN(822,3);
	store_spec_node.m1 = CDGN(822,4);
	store_spec_node.m2 = CDGN(822,5);
	store_spec_node.pic = CDGN(822,6);
	if (store_spec_node.pic < 0)
		store_spec_node.pic = 0;
	store_spec_node.ex1a = CDGN(822,7);
	store_spec_node.ex1b = CDGN(822,8);
	store_spec_node.ex2a = CDGN(822,9);
	store_spec_node.ex2b = CDGN(822,10);
	store_spec_node.jumpto = CDGN(822,11);

	if (edit_spec_stuff_done_mess[store_spec_node.type] == 1) {
		if (cre(store_spec_node.sd1,-1,299,"The first part of a Stuff Done flag must be from 0 to 299 (or -1 if the Stuff Done flag is ignored.",
			"",822) > 0) return FALSE;
		if (cre(store_spec_node.sd2,-1,9,"The second part of a Stuff Done flag must be from 0 to 9 (or -1 if the Stuff Done flag is ignored.",
			"",822) > 0) return FALSE;
		}
	
	if (store_which_mode == 0)
		scenario.scen_specials[store_which_node] = store_spec_node;
	if (store_which_mode == 1)
		current_terrain.specials[store_which_node] = store_spec_node;
	if (store_which_mode == 2)
		town.specials[store_which_node] = store_spec_node;
	return TRUE;
}

void put_spec_enc_in_dlog()
{
	char str[256];
	short i;

	cdsin(822,15,store_which_node);
	get_str(str,22,store_spec_node.type + 1);
	csit(822,27,(char *) str);
	
	if (last_node[0] == -1)
		cd_activate_item(822,14,0);
		else cd_activate_item(822,14,1);
		
	CDSN(822,2,store_spec_node.sd1);
	CDSN(822,3,store_spec_node.sd2);
	switch (edit_spec_stuff_done_mess[store_spec_node.type]) {
		case 0: csit(822,20,"Unused."); csit(822,21,"Unused."); break;
		case 1: csit(822,20,"Stuff Done Flag Part A"); csit(822,21,"Stuff Done Flag Part B"); break;
		case 2: csit(822,20,"Chance of placing (0 - 100)"); csit(822,21,"Unused"); break;
		case 3: csit(822,20,"Stuff Done Flag Part A"); csit(822,21,"Unused"); break;
		case 4: csit(822,20,"X of space to move to"); csit(822,21,"Y of space to move to"); break;
		case 5: csit(822,20,"Terrain to change to"); csit(822,21,"Chance of changing (0 - 100)"); break;
		case 6: csit(822,20,"Switch this ter. type"); csit(822,21,"with this ter. type"); break;
		case 7: csit(822,20,"Chance of placing (0 - 100)"); csit(822,21,"What to place (see docs.)"); break;
		case 8: csit(822,20,"Chance of placing (0 - 100)"); csit(822,21,"0 - web, 1 - barrel, 2 - crate"); break;
		}	

	CDSN(822,4,store_spec_node.m1);
	CDSN(822,5,store_spec_node.m2);
	switch (edit_spec_mess_mess[store_spec_node.type]) {
		case 0: csit(822,22,"Unused."); csit(822,23,"Unused."); cd_activate_item(822,49,0); break;
		case 1: csit(822,22,"First part of message"); csit(822,23,"Second part of message"); 
					cd_activate_item(822,49,1); break;
		case 2: csit(822,22,"Number of first message in dialog"); csit(822,23,"Unused"); 
					cd_activate_item(822,49,1); break;
		case 3: csit(822,22,"Name of Store"); csit(822,23,"Unused"); 
					cd_activate_item(822,49,1); break;
		case 4: csit(822,22,"Number of first message in dialog"); 
				csit(822,23,"1 - add 'Leave'/'OK' button, else no"); 
					cd_activate_item(822,49,1); break;
		case 5: csit(822,22,"Number of first message in dialog"); 
				csit(822,23,"Num. of spec. item to give (-1 none)"); 
					cd_activate_item(822,49,1); break;
		}

	CDSN(822,6,store_spec_node.pic);
	switch (edit_pict_mess[store_spec_node.type]) {
		case 0: csit(822,24,"Unused."); cd_activate_item(822,46,0); break;
		case 1: csit(822,24,"Dialog Picture number"); cd_activate_item(822,46,1); break;
		case 2: csit(822,24,"Terrain Picture number"); cd_activate_item(822,46,1); break;
		case 3: csit(822,24,"Monster Picture number"); cd_activate_item(822,46,1); break;
		case 4: csit(822,24,"Chance of changing (0 - 100)"); cd_activate_item(822,46,0); break;
		case 5: csit(822,24,"Number of letters to match"); cd_activate_item(822,46,0); break;
		case 6: csit(822,24,"Radius of explosion"); cd_activate_item(822,46,0); break;
		}

	CDSN(822,7,store_spec_node.ex1a);
	get_str(str,30,store_spec_node.type + 1);
	csit(822,25,(char *) str);
	cd_activate_item(822,47,0);
	for (i = 0 ; i < 12; i++)
		if (store_spec_node.type == ex1a_choose[i])
			cd_activate_item(822,47,1);
	
	CDSN(822,8,store_spec_node.ex1b);
	get_str(str,31,store_spec_node.type + 1);
	csit(822,26,(char *) str);
	cd_activate_item(822,43,0);
	for (i = 0 ; i < 40; i++)
		if (store_spec_node.type == ex1b_choose[i])
			cd_activate_item(822,43,1);

	CDSN(822,9,store_spec_node.ex2a);
	get_str(str,32,store_spec_node.type + 1);
	csit(822,28,(char *) str);
	cd_activate_item(822,48,0);
	for (i = 0 ; i < 18; i++)
		if (store_spec_node.type == ex2a_choose[i])
			cd_activate_item(822,48,1);
	
	CDSN(822,10,store_spec_node.ex2b);
	get_str(str,33,store_spec_node.type + 1);
	csit(822,29,(char *) str);
	cd_activate_item(822,44,0);
	for (i = 0 ; i < 20; i++)
		if (store_spec_node.type == ex2b_choose[i])
			cd_activate_item(822,44,1);

	CDSN(822,11,store_spec_node.jumpto);
	switch (edit_jumpto_mess[store_spec_node.type]) {
		case 0: csit(822,30,"Special to Jump To"); break;
		case 1: csit(822,30,"Special node if not blocked"); break;
		case 2: csit(822,30,"Special after trap finished"); break;
		case 3: csit(822,30,"Otherwise call this special"); break;
		case 4: csit(822,30,"Special if OK/Leave picked"); break;
		}
}

void edit_spec_enc_event_filter (short item_hit)
{
	char str[256];
	short i,node_to_change_to = -1,spec;
	
	switch (item_hit) {
		case 12:
			if (save_spec_enc() == TRUE)
				 dialog_not_toast = FALSE; 
			break;
		case 14:  //go_back
			if (save_spec_enc() == FALSE)
				 break; 
			for (i = 0; i < 256; i++) 
				if (last_node[i] < 0) {
					node_to_change_to = last_node[i - 1];
					if (i > 0)
						last_node[i - 1] = -1;
					i = 256;
					}
			break;
		case 13: 
			if (last_node[0] != -1) {
				give_error("You can't cancel out of making a special until you're back at the beginning of the special chain.",
					"Press the Go Back button until it disappears.",822);
				break;
				}
			dialog_not_toast = FALSE; break;
		case 43: case 44: case 45: // 1b, 2b, jump to spec
			if (save_spec_enc() == FALSE)
				 break; 
			if (item_hit == 43)
				spec = CDGN(822,8);
			if (item_hit == 44)
				spec = CDGN(822,10);
			if (item_hit == 45)
				spec = CDGN(822,11);
			if ((spec < 0) || (spec >= num_specs[store_which_mode])) {
				spec = -1;
				//CDSN(822,8,-1)
				if ((item_hit == 43) && (store_spec_node.type == 13))
					spec = get_fresh_spec(0);
					else if ((item_hit == 45) && (store_spec_node.type == 21))
						spec = get_fresh_spec(0);
						else spec = get_fresh_spec(store_which_mode);
				if (spec < 0) {
					give_error("You can't create a new special encounter because there are no more free special nodes.",
						"To free a special node, set its type to No Special and set its Jump To special to -1.",822);
					break;
					}
				if (item_hit == 43)
					CDSN(822,8,spec);
				if (item_hit == 44)
					CDSN(822,10,spec);
				if (item_hit == 45)
					CDSN(822,11,spec);
				/*
				if (item_hit == 43)
					store_spec_node.ex1b = spec;
				if (item_hit == 44)
					store_spec_node.ex2b = spec;
				if (item_hit == 45)
					store_spec_node.jumpto = spec;
				*/
				}
			if (save_spec_enc() == FALSE)
				 break; 
			if ((item_hit == 43) && (store_spec_node.type == 13))
				node_to_change_to = spec;
				else if ((item_hit == 45) && (store_spec_node.type == 21))
					node_to_change_to = spec;
					else node_to_change_to = store_which_mode * 1000 + spec;
			for (i = 0; i < 256; i++) 
				if (last_node[i] < 0) {
					last_node[i] = store_which_mode * 1000 + store_which_node;
					i = 256;
					}								
			break;
		case 47: // 1a choose
			switch (store_spec_node.type) {
				case 19: case 50: case 58: case 59: case 60:  
					i = choose_text_res(-2,0,399,store_spec_node.ex1a,822,"Give which item?"); break;
				case 229:  
					i = choose_text_res(-2,0,399,store_spec_node.ex1a,822,"First item in store?"); break;
				case 55: case 56: case 57:
					i = choose_text_res(-3,0,NUM_DLOG_B - 1,store_spec_node.ex1a,822,"Which button label?"); break;
				case 182: 
					i = choose_text_res(-1,1,255,store_spec_node.ex1a,822,"Choose Which Monster:"); break;
				}
			store_spec_node.ex1a = i;
			CDSN(822,7,store_spec_node.ex1a);
			break;
		case 48: // 2a choose
			switch (store_spec_node.type) {
				case 19: case 50:  case 192:  case 229: 
					i = choose_text_res(-2,0,399,store_spec_node.ex2a,822,"Give which item?"); break;
				case 55: case 56: case 57: case 58: case 59: case 60: 
					i = choose_text_res(-3,0,NUM_DLOG_B - 1,store_spec_node.ex2a,822,"Which button label?"); break;
				case 181: 
					i = choose_text_res(-1,1,255,store_spec_node.ex2a,822,"Choose Which Monster:"); break;
				case 135: case 136: case 171: case 172: case 226:
					i = choose_text_res(-4,0,255,store_spec_node.ex2a,822,"Which Terrain?"); break;
				}
			store_spec_node.ex2a = i;
			CDSN(822,9,store_spec_node.ex2a);
			break;
		case 49: // message
			if (save_spec_enc() == TRUE)
				 dialog_not_toast = FALSE; 
			if ((edit_spec_mess_mess[store_spec_node.type] == 2) ||
				(edit_spec_mess_mess[store_spec_node.type] == 4) ||
				(edit_spec_mess_mess[store_spec_node.type] == 5)) {
				edit_dialog_text(store_which_mode,&store_spec_node.m1,822);
				put_spec_enc_in_dlog();	
				}
				else if ((edit_spec_mess_mess[store_spec_node.type] == 1) ||
					(edit_spec_mess_mess[store_spec_node.type] == 3)) {
					edit_spec_text(store_which_mode,&store_spec_node.m1,
						&store_spec_node.m2,822);
					put_spec_enc_in_dlog();	
					}
			break;
		case 46: // pict
			if (save_spec_enc() == TRUE)
				 dialog_not_toast = FALSE; 
			i = -1;
			switch (edit_pict_mess[store_spec_node.type]) {
				case 1: i = choose_graphic(700,731,store_spec_node.pic,822); break;
				case 2: i = choose_graphic(0,252,store_spec_node.pic,822); break;
				case 3: i = choose_graphic(400,572,store_spec_node.pic,822); break;
				}
			if (i >= 0) {
				store_spec_node.pic = i;
				put_spec_enc_in_dlog();	
				}
			break;
		
		case 37: // 1st spec type
			if (save_spec_enc() == TRUE)
				 dialog_not_toast = FALSE; 
			i = choose_text_res(22,1,28,store_spec_node.type + 1,822,"Choose General Use Special:");
			if (i >= 0) {
				store_spec_node.type = i - 1;
				}
			put_spec_enc_in_dlog();	
			break;
		case 38: // 2 spec type
			if (save_spec_enc() == TRUE)
				 dialog_not_toast = FALSE; 
			i = choose_text_res(22,51,64,store_spec_node.type + 1,822,"Choose One-Shot Special:");
			if (i >= 0) {
				store_spec_node.type = i - 1;
				store_spec_node.sd1 = -1; store_spec_node.sd2 = -1; 
				if ((store_spec_node.type >= 55) && (store_spec_node.type <= 57))
					store_spec_node.m2 = 1;
				}
			put_spec_enc_in_dlog();	
			break;
		case 39: // 3 spec type
			if (save_spec_enc() == TRUE)
				 dialog_not_toast = FALSE; 
			i = choose_text_res(22,81,107,store_spec_node.type + 1,822,"Choose Affect Party Special:");
			if (i >= 0) store_spec_node.type = i - 1;
			put_spec_enc_in_dlog();	
			break;
		case 40: // 4 spec type
			if (save_spec_enc() == TRUE)
				 dialog_not_toast = FALSE; 
			i = choose_text_res(22,131,156,store_spec_node.type + 1,822,"Choose If-Then Special:");
			if (i >= 0) {
				store_spec_node.type = i - 1;
				}
			put_spec_enc_in_dlog();	
			break;
		case 41: // 5 spec type
			if (save_spec_enc() == TRUE)
				 dialog_not_toast = FALSE; 
			i = choose_text_res(22,171,219,store_spec_node.type + 1,822,"Choose Town Special:");
			if (i >= 0) store_spec_node.type = i - 1;
			put_spec_enc_in_dlog();	
			break;
		case 42: // 6 spec type
			if (save_spec_enc() == TRUE)
				 dialog_not_toast = FALSE; 
			i = choose_text_res(22,226,230,store_spec_node.type + 1,822,"Choose Outdoor Special:");
			if (i >= 0) store_spec_node.type = i - 1;
			put_spec_enc_in_dlog();	
			break;
		}
	/*if ((item_hit >= 37) && (item_hit <= 42)) {
		if (cd_get_active(822,43) == 0)
			CDSN(822,8,0);
		if (cd_get_active(822,44) == 0)
			CDSN(822,10,0);
		}*/ // Might be useful, but I forget what I was thinking when I added it.
	if (node_to_change_to >= 0) {
		store_which_mode = node_to_change_to / 1000;
		store_which_node = node_to_change_to % 1000;
		if (store_which_mode == 0)
			store_spec_node = scenario.scen_specials[store_which_node];
		if (store_which_mode == 1)
			store_spec_node = current_terrain.specials[store_which_node];
		if (store_which_mode == 2)
			store_spec_node = town.specials[store_which_node];
		if (store_spec_node.pic < 0)
			store_spec_node.pic = 0;
		put_spec_enc_in_dlog();
		}	
}

// mode - 0 scen 1 - out 2 - town
void edit_spec_enc(short which_node,short mode,short parent_num)
// ignore parent in Mac version
{
	short spec_enc_hit,i,store_dialog_answer;
	char temp_str[256];
	
	store_which_mode = mode;
	store_which_node = which_node;
	for (i = 0; i < 256; i++)
		last_node[i] = -1;
	//last_node[0] = store_which_mode * 1000 + store_which_node;
	if (mode == 0)
		store_spec_node = scenario.scen_specials[store_which_node];
	if (mode == 1)
		store_spec_node = current_terrain.specials[store_which_node];
	if (mode == 2)
		store_spec_node = town.specials[store_which_node];
	if (store_spec_node.pic < 0)
		store_spec_node.pic = 0;
		
	cd_create_dialog_parent_num(822,parent_num);
	
	cd_activate_item(822,14,0);
	put_spec_enc_in_dlog();
	
	while (dialog_not_toast)
		ModalDialog();
	cd_kill_dialog(822,0);
}

short get_fresh_spec(short which_mode)
{
	short i;
	special_node_type store_node;
	
	for (i = 0; i < num_specs[which_mode]; i++) {
		if (which_mode == 0)
			store_node = scenario.scen_specials[i];
		if (which_mode == 1)
			store_node = current_terrain.specials[i];
		if (which_mode == 2)
			store_node = town.specials[i];
		if ((store_node.type == 0) && (store_node.jumpto == -1) && (store_node.pic == -1))
			return i;
		}
		
	return -1;
}

void edit_spec_text_event_filter (short item_hit)
{
	char str[256];
	short i;
	
	switch (item_hit) {
		case 7:
			dialog_not_toast = FALSE; 
			break;

		case 6:
			CDGT(826,2,(char *) str);
			if (strlen((char *) str) > 0) {
				if (*store_str1 < 0) {
					switch (store_spec_str_mode) {
						case 0:
							for (i = 0; i < 100; i++)
								if (scen_strs2[i][0] == '*') {
									*store_str1 = i;
									i = 500;
									}
							break;
						case 1:
							for (i = 10; i < 100; i++)
								if (data_store->out_strs[i][0] == '*') {
									*store_str1 = i - 10;
									i = 500;
									}
							break;
						case 2:
							for (i = 20; i < 120; i++)
								if (town_strs[i][0] == '*') {
									*store_str1 = i - 20;
									i = 500;
									}
							break;
					
						}
					if (i < 500) {
						give_error("There are no more free message slots.",
							"To free up some slots, go into Edit Town/Out/Scenario Text to clear some messages.", 826);
						break;
						}
					}
				if (*store_str1 >= 0) {
					switch (store_spec_str_mode) {
						case 0:
							strcpy(scen_strs2[*store_str1],(char *) str);
							break;				
						case 1:
							strcpy(data_store->out_strs[*store_str1 + 10],(char *) str);
							break;				
						case 2:
							strcpy(town_strs[*store_str1 + 20],(char *) str);
							break;				
						}
					}
				}
			CDGT(826,3,(char *) str);
			if (strlen((char *) str) > 0) {
				if (*store_str2 < 0) {
					switch (store_spec_str_mode) {
						case 0:
							for (i = 0; i < 100; i++)
								if (scen_strs2[i][0] == '*') {
									*store_str2 = i;
									i = 500;
									}
							break;
						case 1:
							for (i = 10; i < 100; i++)
								if (data_store->out_strs[i][0] == '*') {
									*store_str2 = i - 10;
									i = 500;
									}
							break;
						case 2:
							for (i = 20; i < 120; i++)
								if (town_strs[i][0] == '*') {
									*store_str2 = i - 20;
									i = 500;
									}
							break;
					
						}
					if (i < 500) {
						give_error("There are no more free message slots.",
							"To free up some slots, go into Edit Town/Out/Scenario Text to clear some messages.", 826);
						break;
						}
					}
				if (*store_str2 >= 0) {
					switch (store_spec_str_mode) {
						case 0:
							strcpy(scen_strs2[*store_str2],(char *) str);
							break;				
						case 1:
							strcpy(data_store->out_strs[*store_str2 + 10],(char *) str);
							break;				
						case 2:
							strcpy(town_strs[*store_str2 + 20],(char *) str);
							break;				
						}
					}
				}
			dialog_not_toast = FALSE;
			break;
		}
}

// mode 0 - scen 1 - out 2 - town
void edit_spec_text(short mode,short *str1,short *str2,short parent)
{
	short text_hit,i,store_dialog_answer,item_hit;
	char temp_str[256];
	short num_s_strs[3] = {100,90,100};
	
	store_str1 = str1;
	store_str2 = str2;
	store_spec_str_mode = mode;
	
	cd_create_dialog_parent_num(826,parent);
	
	if (*str1 >= num_s_strs[mode])
		*str1 = -1;
	if (*str1 >= 0){
		if (mode == 0)
			CDST(826,2,scen_strs2[*str1]);
		if (mode == 1)
			CDST(826,2,data_store->out_strs[*str1 + 10]);
		if (mode == 2)
			CDST(826,2,town_strs[*str1 + 20]);
		}
	if (*str2 >= num_s_strs[mode])
		*str2 = -1;
	if (*str2 >= 0){
		if (mode == 0)
			CDST(826,3,scen_strs2[*str2]);
		if (mode == 1)
			CDST(826,3,data_store->out_strs[*str2 + 10]);
		if (mode == 2)
			CDST(826,3,town_strs[*str2 + 20]);
		}
		
	while (dialog_not_toast)
		ModalDialog();	

	cd_kill_dialog(826,0);
}

void edit_dialog_text_event_filter (short item_hit)
{
	char str[256];
	short i;
	
	switch (item_hit) {
		case 9:
			dialog_not_toast = FALSE; 
			break;

		case 8:
			for (i = 0; i < 6; i++) {
				CDGT(842,2 + i,(char *) str);
				switch (store_spec_str_mode) {
					case 0:
						strcpy(scen_strs2[*store_str1 + i],(char *) str);
						break;				
					case 1:
						strcpy(data_store->out_strs[*store_str1 + 10 + i],(char *) str);
						break;				
					case 2:
						strcpy(town_strs[*store_str1 + 20 + i],(char *) str);
						break;				
					}
				}
			dialog_not_toast = FALSE;
			break;
		}
}

// mode 0 - scen 1 - out 2 - town
void edit_dialog_text(short mode,short *str1,short parent)
{
	short text_hit,i,j,store_dialog_answer,item_hit;
	char temp_str[256];
	short num_s_strs[3] = {100,90,100};
	
	store_str1 = str1;
	store_spec_str_mode = mode;
	
	if (*str1 >= num_s_strs[mode] - 6)
		*str1 = -1;
	// first, assign the 6 strs for the dialog.
	if (*str1 < 0) {
					switch (store_spec_str_mode) {
						case 0:
							for (i = 0; i < 100 - 6; i++) {
								for (j = i; j < i + 6; j++)
									if (scen_strs2[j][0] != '*') {
										j = 500;
										}
								if (j < 500) {
									*str1 = i;
									i = 500;
									}
								}
							break;
						case 1:
							for (i = 10; i < 100 - 6; i++) {
								for (j = i; j < i + 6; j++)
									if (data_store->out_strs[j][0] != '*') {
										j = 500;
										}
								if (j < 500) {
									*str1 = i - 10;
									i = 500;
									}
								}
							break;
						case 2:
							for (i = 20; i < 120 - 6; i++) {
								for (j = i; j < i + 6; j++)
									if (town_strs[j][0] != '*') {
										j = 500;
										}
								if (j < 500) {
									*str1 = i - 20;
									i = 500;
									}
								}
							break;
					
						}
		if (*str1 >= 0)
			for (i = *str1; i < *str1 + 6; i++) {
				switch (store_spec_str_mode) {
					case 0:
						sprintf(scen_strs2[i],"");
						break;				
					case 1:
						sprintf(data_store->out_strs[10 + i],"");
						break;				
					case 2:
						sprintf(town_strs[20 + i],"");
						break;				
					}			
				}
		}
	if (*str1 < 0) {
		give_error("To create a dialog, you need 6 consecutive unused messages. To free up 6 messages, select Edit Out/Town/Scenario Text from the menus.","",parent);
		return;
		}
		
	cd_create_dialog_parent_num(842,parent);
	
	if (*str1 >= 0){
		for (i = 0; i < 6; i++) {
			if (mode == 0)
				CDST(842,2 + i,scen_strs2[*str1 + i]);
			if (mode == 1)
				CDST(842,2 + i,data_store->out_strs[*str1 + 10 + i]);
			if (mode == 2)
				CDST(842,2 + i,town_strs[*str1 + 20 + i]);
			}
		}
		
	while (dialog_not_toast)
		ModalDialog();	

	cd_kill_dialog(842,0);
}

void edit_special_num_event_filter (short item_hit)
{
	char str[256];
	short i;
	
	switch (item_hit) {
		case 8:
			dialog_answer = -1;
			dialog_not_toast = FALSE; 
			break;

		case 4:
			i = CDGN(825,2);
			if ((i < 0) || (i >= num_specs[store_spec_mode])) {
				give_error("There is no special node with that number. Legal ranges are 0 to 255 for scenario specials, 0 to 59 for outdoor specials, and 0 to 99 for town specials.","",825);
				break;
				}
			dialog_answer = i;
			dialog_not_toast = FALSE;
			break;
		}
}

short edit_special_num(short mode,short what_start)
{
	short text_hit,i,j,store_dialog_answer,item_hit;
	char temp_str[256];
	
	store_spec_mode = mode;
		
	cd_create_dialog_parent_num(825,0);
	
	CDSN(825,2,what_start);
		
	while (dialog_not_toast)
		ModalDialog();	

	cd_kill_dialog(825,0);
	
	return dialog_answer;
}

void edit_scen_intro_event_filter (short item_hit)
{
	char str[256];
	short i;
	
	switch (item_hit) {
		case 9:
			scenario.intro_pic = CDGN(804,8);
			if ((scenario.intro_pic < 0) || (scenario.intro_pic > 29)) {
				give_error("Intro picture number is out of range.","",804);
				break;
				}
			for (i = 0; i < 6; i++)
				CDGT(804, 2 + i,scen_strs[4 + i]);
			dialog_not_toast = FALSE; 
			break;
		case 10: 
			dialog_not_toast = FALSE; 
			break;

		case 16:
			i = CDGN(804,8);
			i = choose_graphic(1600,1629,1600 + i,804);
			if (i >= 0) {
				CDSN(804,8,i - 1600);
				csp(804,11,i );
				}
			break;
		}
}

void edit_scen_intro()
{
	short text_hit,i,j,store_dialog_answer,item_hit;
	char temp_str[256];
	short num_s_strs[3] = {100,90,100};
	
		
	cd_create_dialog_parent_num(804,0);
	
	CDSN(804,8,scenario.intro_pic);
	for (i = 0; i < 6; i++)
		CDST(804, 2 + i,scen_strs[4 + i]);
	csp(804,11,scenario.intro_pic + 1600);
	
	while (dialog_not_toast)
		ModalDialog();	

	cd_kill_dialog(804,0);
}


void make_cursor_sword() 
{
	set_cursor(0);
}

void set_cursor(short which_c) 
{
	short i;

	if (cursors[0] == NULL) {
		for (i = 0; i < 8; i++)
			cursors[i] = LoadCursor(store_hInstance,MAKEINTRESOURCE(130 + i));
		}
	current_cursor = which_c;
	SetCursor(cursors[current_cursor]);
}
void restore_cursor()
{
	set_cursor(current_cursor);
}

