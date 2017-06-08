
#include <Windows.h>
#include <commdlg.h>

#include <string.h>
#include "global.h"
#include "stdio.h"
#include "fileio.h"
#include "text.h"
#include "town.h"
#include "items.h"
#include "graphics.h"
#include "locutils.h"
#include "fields.h"
#include "newgraph.h"
#include "dlgutils.h"
#include "gutils.h"
#include "infodlgs.h"
#include "graphutl.h"


#include "exlsound.h"

#define	DONE_BUTTON_ITEM	1

HWND	the_dialog;
extern party_record_type far	party;
extern pc_record_type far adven[6];
extern outdoor_record_type far outdoors[2][2];
extern unsigned char far out[96][96];
extern unsigned char far out_e[96][96];
extern short overall_mode,give_delays,stat_screen_mode;
extern Boolean in_startup_mode,registered,play_sounds,in_scen_debug,sys_7_avail,save_maps,party_in_memory;
extern current_town_type far	c_town;
extern town_item_list far 	t_i;
extern location center;
extern long register_flag;
extern HWND mainPtr;
extern stored_items_list_type far stored_items[3];
extern stored_outdoor_maps_type far o_maps;
extern big_tr_type far  t_d;
extern short town_size[3];
extern short town_type,current_pc;
extern Boolean web,crate,barrel,fire_barrier,force_barrier,quickfire,force_wall,fire_wall,antimagic,scloud,ice_wall,blade_wall;
extern Boolean sleep_field;
extern setup_save_type far setup_save;
extern unsigned char far misc_i[64][64],sfx[64][64];
extern unsigned char far template_terrain[64][64];
extern town_record_type far anim_town;
extern tiny_tr_type far anim_t_d;
extern Boolean modeless_exists[18];
extern location monster_targs[T_M];
extern HWND modeless_dialogs[18] ;
extern short which_combat_type;
extern Boolean game_run_before;
extern Boolean ed_reg;
extern long ed_flag, ed_key;
extern short display_mode,cur_town_talk_loaded;
extern Boolean give_intro_hint;
extern char terrain_blocked[256];
extern char far scen_strs2[110][256];
extern stored_town_maps_type far town_maps,town_maps2;
extern scenario_data_type far scenario;
 extern unsigned char far combat_terrain[64][64];
extern piles_of_stuff_dumping_type *data_store;
extern piles_of_stuff_dumping_type2 *data_store2;
 extern talking_record_type far talking;

extern short terrain_pic[256];
extern scen_header_type scen_headers[25];
HBITMAP spec_scen_g = NULL;

extern HDC main_dc,main_dc2,main_dc3;
extern piles_of_stuff_dumping_type3 *data_store3;
extern piles_of_stuff_dumping_type4 *data_store4;
extern piles_of_stuff_dumping_type5 *data_store5;

typedef struct {
	char expl[96][96];
	}	out_info_type;

Boolean loaded_yet = FALSE, got_nagged = FALSE,ae_loading = FALSE;
 Boolean cur_scen_is_win = TRUE;


void print_write_position ();
void save_outdoor_maps();
void add_outdoor_maps();

char last_load_file[63] = "blades.save";
char szFileName [128] = "blades.sav";
char szTitleName [128] = "blades.sav";
OPENFILENAME ofn;
OFSTRUCT save_dir,save_dir2;

// Trying this to fix bug. Hope it works.
	tiny_tr_type far tiny_t;
	ave_tr_type far ave_t;

outdoor_record_type  far dummy_out;////
town_record_type  far dummy_town;
short jl;
extern char file_path_name[256];

void file_initialize()
{
static char *szFilter[] = {"Blades of Exile Save Files (*.SAV)","*.sav",
		"Text Files (*.TXT)","*.txt",
		"All Files (*.*)","*.*",
		""};

		OpenFile("outdoor.dat",&save_dir,OF_PARSE);
		OpenFile("town.dat",&save_dir2,OF_PARSE);

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = mainPtr;
		ofn.hInstance = NULL;
		ofn.lpstrFilter = szFilter[0];
		ofn.lpstrCustomFilter = NULL;
		ofn.nMaxCustFilter = 0;
		ofn.nFilterIndex = 0;
		ofn.lpstrFile = NULL;
		ofn.nMaxFile = 128;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 128;
		ofn.lpstrInitialDir = NULL;
		ofn.lpstrTitle = NULL;
		ofn.Flags = 0;
		ofn.nFileOffset = 0;
		ofn.nFileExtension = 0;
		ofn.lpstrDefExt = "txt";
		ofn.lCustData = 0L;
		ofn.lpfnHook = NULL;
		ofn.lpTemplateName = NULL;
}


void load_file()
{

	long file_size;
	HFILE file_id;
	short i,j,k;
	Boolean town_restore = FALSE;
	Boolean maps_there = FALSE;
	Boolean map_doh = FALSE;
	
	flag_type fred;
	flag_type *store;
	char flag_data[8];

	town_item_list *item_ptr;	
	long len;
	UINT store_len,count,error;
	out_info_type *explored_ptr;	
	char *party_ptr;
	char *pc_ptr;
	flag_type flag;
	flag_type *flag_ptr;
	stored_items_list_type *items_ptr;
	Boolean in_scen = FALSE;

	short flags[3][2] = {{5790,1342}, // slot 0 ... 5790 - out  1342 - town
					{100,200}, // slot 1 100  in scenario, 200 not in
					{3422,5567}}; // slot 2 ... 3422 - no maps  5567 - maps

	ofn.hwndOwner = mainPtr;
	ofn.lpstrFile = szFileName;
	ofn.lpstrFileTitle = szTitleName;
	ofn.Flags = 0;

	if (GetOpenFileName(&ofn) == 0)
		return;
	file_id = _lopen(szFileName,OF_READ | OF_SHARE_DENY_WRITE);
	if (file_id == -1) {
		beep();
		return;
		}
		
	file_size = sizeof(party_record_type);

	len = sizeof(flag_type);

//	sprintf((char *) debug, "  Len %d               ", (short) len);
//	add_string_to_buf((char *) debug);

	for (i = 0; i < 3; i++) {
		if ((error = _lread(file_id, (char *) flag_data, len)) == HFILE_ERROR) {
			beep();
			return;
			}
		flag_ptr = (flag_type *) flag_data;
		flag = *flag_ptr;
		if ((flag.i != flags[i][0]) && (flag.i != flags[i][1])) { // OK Exile II save file?
			_lclose(file_id);
			FCD(1063,0);
			return;
			} 

		if ((i == 0) && (flag.i == flags[i][1]))
			town_restore = TRUE;
		if ((i == 1) && (flag.i == flags[i][0])) {
			in_scen = TRUE;
			}
		if ((i == 2) && (flag.i == flags[i][1]))
			maps_there = TRUE;
		}
	
	// LOAD PARTY	
	len = (UINT) sizeof(party_record_type);
	store_len = len;
	party_ptr = (char *) &party;
	if ((error = _lread(file_id, (char *) party_ptr, len)) == HFILE_ERROR){
		_lclose(file_id);
		SysBeep(2);
		FCD(1064,0);
		return;
		}
	for (count = 0; count < store_len; count++)
		party_ptr[count] ^= 0x5C;	

	// LOAD SETUP
	len = (UINT) sizeof(setup_save_type);
	if ((error = _lread(file_id, (char *) &setup_save, len)) == HFILE_ERROR){
		_lclose(file_id);
		SysBeep(2);
		FCD(1064,0);
		return;
		}

	// LOAD PCS
	store_len = (UINT) sizeof(pc_record_type);
	for (i = 0; i < 6; i++) {
		len = store_len;
		pc_ptr = (char *) &adven[i];
		if ((error = _lread(file_id,(char *) pc_ptr, len))  == HFILE_ERROR){
			_lclose(file_id);
			SysBeep(2);
		FCD(1064,0);
			return;
			}
		for (count = 0; count < store_len; count++)
			pc_ptr[count] ^= 0x6B;	
		}

	if (in_scen == TRUE) {
	
	// LOAD OUTDOOR MAP
	len = (long) sizeof(out_info_type);
	if ((error = FSRead(file_id, &len, (char *) out_e)) != 0){
		FSClose(file_id);
		SysBeep(2);
		FCD(1064,0);
		return;
		}

	// LOAD TOWN 
	if (town_restore == TRUE) {
		len = (long) sizeof(current_town_type);
		if ((error = FSRead(file_id, &len, (char *) &c_town)) != 0){
				FSClose(file_id);
				SysBeep(2);
		FCD(1064,0);
				return;
				}
	
		len = (long) sizeof(big_tr_type);
		if ((error = FSRead(file_id, &len, (char *) &t_d)) != 0){
				FSClose(file_id);
				SysBeep(2);
		FCD(1064,0);
				return;
				}

		len = (long) sizeof(town_item_list);
		if ((error = FSRead(file_id, &len, (char *) &t_i))  != 0){
			FSClose(file_id);
			SysBeep(2);
		FCD(1064,0);
			return;
			}
	
		}

	// LOAD STORED ITEMS
	for (i = 0; i < 3; i++) {
		len = (long) sizeof(stored_items_list_type);
		if ((error = FSRead(file_id, &len, (char *) &stored_items[i]))  != 0){
				FSClose(file_id);
				SysBeep(2);
		FCD(1064,0);
				return;
				}		
		}

	// LOAD SAVED MAPS
	if (maps_there == TRUE) {
		len = (long) sizeof(stored_town_maps_type);
		if ((error = FSRead(file_id, &len, (char *) &(town_maps)))  != 0){
				FSClose(file_id);
				SysBeep(2);
				FCD(1064,0);
				return;
				}
		len = (long) sizeof(stored_town_maps_type);
		if ((error = FSRead(file_id, &len, (char *) &(town_maps2)))  != 0){
				FSClose(file_id);
				SysBeep(2);
				FCD(1064,0);
				return;
				}
	
		len = (long) sizeof(stored_outdoor_maps_type);
		if ((error = FSRead(file_id, &len, (char *) &o_maps)) != 0) {
				FSClose(file_id);
				SysBeep(2);
		FCD(1064,0);
				return;
				}	
		}

	// LOAD SFX & MISC_I
		len = (long) (64 * 64);
		if ((error = FSRead(file_id, &len, (char *) sfx))  != 0){
				FSClose(file_id);
				SysBeep(2);
		FCD(1064,0);
				return;
				}	
		if ((error = FSRead(file_id, &len, (char *) misc_i))  != 0){
				FSClose(file_id);
				SysBeep(2);
		FCD(1064,0);
				return;
				}	

	} // end if_scen

	if ((error = _lclose(file_id))  == HFILE_ERROR){
		add_string_to_buf("Load: Can't close file.          ");
		SysBeep(2);
		return;
		}

	party_in_memory = TRUE;
                                                                                                                                                                                                                                                                                                                                                                                                                                              
	// now if not in scen, this is it.
	if (in_scen == FALSE) {
		if (in_startup_mode == FALSE) {
			reload_startup();
			in_startup_mode = TRUE;
			draw_startup(0);		
			}
		return;
		}
		
	if (load_scenario() == FALSE)
		return;
	// if at this point, startup must be over, so make this call to make sure we're ready,
	// graphics wise
	end_startup();

	set_up_ter_pics();
	load_outdoors(party.outdoor_corner.x + 1,party.outdoor_corner.y + 1,1,1,0,0,NULL);
	load_outdoors(party.outdoor_corner.x,party.outdoor_corner.y + 1,0,1,0,0,NULL);
	load_outdoors(party.outdoor_corner.x + 1,party.outdoor_corner.y,1,0,0,0,NULL);
	load_outdoors(party.outdoor_corner.x,party.outdoor_corner.y,0,0,0,0,NULL);

	overall_mode = (town_restore == TRUE) ? 1 : 0;
	stat_screen_mode = 0;
	build_outdoors();
	erase_out_specials();
	update_pc_graphics();
	
	//SelectWindow(mainPtr);
	//SetPort(mainPtr);
	//if (modeless_exists[5] == TRUE)
	//	BringToFront(modeless_dialogs[5]);
		
	if (town_restore == FALSE) {
		center = party.p_loc;
		load_area_graphics();
		}
		else {	
			load_town(c_town.town_num,2,-1,NULL);
			load_town(c_town.town_num,1,-1,NULL);
			for (i = 0; i < T_M; i++)
				{monster_targs[i].x = 0;  monster_targs[i].y = 0;}

			town_type = scenario.town_size[c_town.town_num];
			// Set up field booleans
			for (j = 0; j < town_size[town_type]; j++)
				for (k = 0; k < town_size[town_type]; k++) {
					// Set up field booleans
					if (is_web(j,k) == TRUE)
						web = TRUE;
					if (is_crate(j,k) == TRUE)
						crate = TRUE;
					if (is_barrel(j,k) == TRUE)
						barrel = TRUE;
					if (is_fire_barrier(j,k) == TRUE)
						fire_barrier = TRUE;
					if (is_force_barrier(j,k) == TRUE)
						force_barrier = TRUE;
					if (is_quickfire(j,k) == TRUE)
						quickfire = TRUE;
					}		
force_wall = TRUE;fire_wall = TRUE;antimagic = TRUE;scloud = TRUE;ice_wall = TRUE;blade_wall = TRUE;
sleep_field = TRUE;
			center = c_town.p_loc;
			load_area_graphics();
			}

	create_clip_region();
	redraw_screen(0);
		//else flood_bg();

	current_pc = first_active_pc();
	loaded_yet = TRUE;
	

	add_string_to_buf("Load: Game loaded.            ");
		
	// Set sounds, map saving, and speed
	if (((play_sounds == TRUE) && (party.stuff_done[306][1] == 1)) ||
	 ((play_sounds == FALSE) && (party.stuff_done[306][1] == 0))) {
		flip_sound();
		}
	give_delays = party.stuff_done[306][2]; 
	if (party.stuff_done[306][0] == 0)
		save_maps = TRUE;
		else save_maps = FALSE;	

	in_startup_mode = FALSE;
	in_scen_debug = FALSE;
	

}

void save_file(short mode)
//mode;  // 0 - normal  1 - save as
{
	long file_size;
	HFILE file_id;
	Boolean got_error = FALSE,town_save = FALSE;
	UINT error;

	short i,j;

	long len,store_len,count;
	flag_type flag;
	flag_type *store;
	party_record_type *party_ptr;
	setup_save_type	*setup_ptr;
	pc_record_type *pc_ptr;
//	out_info_type store_explored;
	out_info_type *explored_ptr;
	current_town_type *town_ptr;
	big_tr_type *town_data_ptr;
	town_item_list *item_ptr;
	stored_items_list_type *items_ptr;
	stored_town_maps_type *maps_ptr; 
	stored_outdoor_maps_type *o_maps_ptr; 
		
	char *party_encryptor;	
	char debug[60];

	if ((in_startup_mode == FALSE) && (is_town()))
		town_save = TRUE;

	ofn.hwndOwner = mainPtr;
	ofn.lpstrFile = szFileName;
	ofn.lpstrFileTitle = szTitleName;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	
	if ((mode == 1) || (in_startup_mode == TRUE)) {
		if (GetSaveFileName(&ofn) == 0)
			return;
		}

	if (-1 == (file_id = _lopen(szFileName, OF_WRITE | OF_SHARE_EXCLUSIVE)))
		if (-1 == (file_id = _lcreat(szFileName,0))) {
			beep();
			return;
			}
		
	store = &flag;	

	len = sizeof(flag_type);

	flag.i = (town_save == TRUE) ? 1342 : 5790;
	if ((error = _lwrite(file_id, (char *) store, len))  == HFILE_ERROR){
		add_string_to_buf("Save: Couldn't write to file.         ");
		_lclose(file_id);
		SysBeep(2);
		}
	flag.i = (in_startup_mode == FALSE) ? 100 : 200;
	if ((error = _lwrite(file_id, (char *) store, len)) == HFILE_ERROR) {
		add_string_to_buf("Save: Couldn't write to file.         ");
		_lclose(file_id);
		SysBeep(2);
		return;
		}
	flag.i = (save_maps == TRUE) ? 5567 : 3422;
	if ((error = _lwrite(file_id, (char *) store, len))  == HFILE_ERROR){
		add_string_to_buf("Save: Couldn't write to file.         ");
		_lclose(file_id);
		SysBeep(2); 
		return;
		}

	// SAVE PARTY
	party_ptr = &party;	
	len = sizeof(party_record_type);

	store_len = len;
	party_encryptor = (char *) party_ptr;
	for (count = 0; count < store_len; count++)
		party_encryptor[count] ^= 0x5C;
	if ((error = _lwrite(file_id, (char *) party_ptr, len)) == HFILE_ERROR) {
		add_string_to_buf("Save: Couldn't write to file.         ");
		_lclose(file_id);
		for (count = 0; count < store_len; count++)
			party_encryptor[count] ^= 0x5C;
			SysBeep(2); 
		return;
		}
	for (count = 0; count < store_len; count++)
		party_encryptor[count] ^= 0x5C;

	// SAVE SETUP
	setup_ptr = &setup_save;	
	len = sizeof(setup_save_type);
	if ((error = _lwrite(file_id, (char *) setup_ptr, len))  == HFILE_ERROR){
		add_string_to_buf("Save: Couldn't write to file.         ");
		_lclose(file_id);
		SysBeep(2); 
		return;
		}
		
	// SAVE PCS	
	store_len = sizeof(pc_record_type);
	for (i = 0; i < 6; i++) {
		pc_ptr = &adven[i];	

		len = store_len;
		party_encryptor = (char *) pc_ptr;
		for (count = 0; count < store_len; count++)
			party_encryptor[count] ^= 0x6B;
		if ((error = _lwrite(file_id, (char *) pc_ptr, len))  == HFILE_ERROR){
			add_string_to_buf("Save: Couldn't write to file.         ");
			_lclose(file_id);
			for (count = 0; count < store_len; count++)
				party_encryptor[count] ^= 0x6B;
				SysBeep(2); 
			return;
			}
		for (count = 0; count < store_len; count++)
			party_encryptor[count] ^= 0x6B;
		}
		
	if (in_startup_mode == FALSE) {
	
	// SAVE OUT DATA
	len = sizeof(out_info_type);
	if ((error = FSWrite(file_id, &len, (char *) out_e)) != 0) {
		add_string_to_buf("Save: Couldn't write to file.         ");
		FSClose(file_id);
		SysBeep(2); 
		return;
		}

	if (town_save == TRUE) {	
			town_ptr = &c_town;	
			len = sizeof(current_town_type);
			if ((error = FSWrite(file_id, &len, (char *) town_ptr)) != 0) {
				add_string_to_buf("Save: Couldn't write to file.         ");
				FSClose(file_id);
				SysBeep(2); 
				return;
				}
			town_data_ptr = &t_d;	
			len = sizeof(big_tr_type);
			if ((error = FSWrite(file_id, &len, (char *) town_data_ptr)) != 0) {
				add_string_to_buf("Save: Couldn't write to file.         ");
				FSClose(file_id);
				SysBeep(2); 
				return;
				}
			item_ptr = &t_i;	
			len = sizeof(town_item_list);
			if ((error = FSWrite(file_id, &len, (char *) item_ptr)) != 0) {
				add_string_to_buf("Save: Couldn't write to file.         ");
				FSClose(file_id);
				SysBeep(2); 
				return;
			}	
		}
	
	// Save stored items 
	for (i = 0; i < 3; i++) {
		items_ptr = &stored_items[i];
		len = (long) sizeof(stored_items_list_type);
		if ((error = FSWrite(file_id, &len, (char *) items_ptr))  != 0){
			add_string_to_buf("Save: Couldn't write to file.         ");
			FSClose(file_id);
			SysBeep(2); 
			return;
			}
		}
			
	// If saving maps, save maps
	if (save_maps == TRUE) {
		maps_ptr = &(town_maps);
		len = (long) sizeof(stored_town_maps_type);
		if ((error = FSWrite(file_id, &len, (char *) maps_ptr))  != 0){
			add_string_to_buf("Save: Couldn't write to file.         ");
			FSClose(file_id);
			SysBeep(2); 
			return;
			}	
		maps_ptr = &(town_maps2);
		len = (long) sizeof(stored_town_maps_type);
		if ((error = FSWrite(file_id, &len, (char *) maps_ptr))  != 0){
			add_string_to_buf("Save: Couldn't write to file.         ");
			FSClose(file_id);
			SysBeep(2); 
			return;
			}	

		o_maps_ptr = &o_maps;
		len = (long) sizeof(stored_outdoor_maps_type);
		if ((error = FSWrite(file_id, &len, (char *) o_maps_ptr)) != 0) {
			add_string_to_buf("Save: Couldn't write to file.         ");
			FSClose(file_id);
			SysBeep(2); 
			return;
			}
		} 
	
	// SAVE SFX and MISC_I
		len = (long) (64 * 64);
		if ((error = FSWrite(file_id, &len, (char *) sfx))  != 0){
			add_string_to_buf("Save: Couldn't write to file.         ");
			FSClose(file_id);
			SysBeep(2); 
			return;
			}
		if ((error = FSWrite(file_id, &len, (char *) misc_i))  != 0){
			add_string_to_buf("Save: Couldn't write to file.         ");
			FSClose(file_id);
			SysBeep(2); 
			return;
			}
	


	}

	if ((error = _lclose(file_id)) == HFILE_ERROR) {
		add_string_to_buf("Save: Couldn't close file.         ");
		SysBeep(2);
		return;
		}
		
	if (in_startup_mode == FALSE)
		add_string_to_buf("Save: Game saved.              ");
}

void set_terrain(location l, unsigned char terrain_type)
{
	t_d.terrain[l.x][l.y] = terrain_type;
	combat_terrain[l.x][l.y] = terrain_type;
}

void change_rect_terrain(RECT r,unsigned char terrain_type,short probability,Boolean hollow)
// prob is 0 - 20, 0 no, 20 always
{
location l;
short i,j;

for (i = 0; i < town_size[town_type]; i++)
	for (j = 0; j < town_size[town_type]; j++) {
		l.x = i;
		l.y = j;
		if ((i >= r.left) && (i <= r.right) && (j >= r.top) && (j <= r.bottom)
		&& ((hollow == FALSE) || (i == r.left) || (i == r.right) || (j == r.top) || (j == r.bottom))
		  && ((hollow == TRUE) || (get_ran(1,1,20) <= probability)))
			set_terrain(l,terrain_type);
		}
}

void swap_val(unsigned char *val,short a,short b)
{
	if (*val == a)
		*val = b;
		else if (*val == b)
			*val = a;
}
void change_val_4 (unsigned char *val,short a,short b,short c,short d)
{
	if (*val == a)
		*val = b;
		else if (*val == b)
			*val = c;
			else if (*val == c)
			*val = d;
			else if (*val == d)
				*val = a;
}
void change_val (unsigned char *val,short a,short b)
{
	if (*val == a)
		*val = b;
}

void build_scen_file_name (char *file_n)
{
	short i,last_slash = -1;

	for (i = 0; i < 256; i++)
		if ((file_path_name[i] == 92) || (file_path_name[i] == '/'))
			last_slash = i;
	if (last_slash < 0) {
		sprintf((char *) file_n,"BLADSCEN/%s",party.scen_name);
		return;
		}
	strcpy(file_n,file_path_name);
	file_n += last_slash + 1;
	sprintf((char *) file_n,"BLADSCEN/%s",party.scen_name);
}

void build_scen_ed_name (char *file_n)
{
	short i,last_slash = -1;

	for (i = 0; i < 256; i++)
		if ((file_path_name[i] == 92) || (file_path_name[i] == '/'))
			last_slash = i;
	if (last_slash < 0) {
		sprintf((char *) file_n,"BLSCENED/bladdata.bld");
		return;
		}
	strcpy(file_n,file_path_name);
	file_n += last_slash + 1;
	sprintf((char *) file_n,"BLSCENED/bladdata.bld");
}

// mode 0 want town and talking, 1 talking only, 2 want a string only, and extra is string num
// Hey's let's be kludgy and overload these value again! If extra is -1, and mode 2, that
// means we want to load all the strings and only the strings
void load_town(short town_num,short mode,short extra,char *str)
{

	HFILE file_id;
	short i,j,k;
	long int num_records_to_offset;
	unsigned char to_put;
	short start_volume;
	long store;
	long start_dir;
	long len;
	OFSTRUCT store_str;
	UINT l,error;
	long len_to_jump = 0;
	short which_town;
	char start_name[256],file_name[256];

	if (town_num != minmax(0,scenario.num_towns - 1,town_num)) {
		give_error("The scenario tried to place you into a non-existant town.","",0);
		return;
		}

	which_town = town_num;

	//HGetVol((StringPtr) start_name,&start_volume,&start_dir);
	build_scen_file_name(file_name);
	file_id = OpenFile(file_name,&store_str,OF_READ | OF_SEARCH);
	if (file_id == HFILE_ERROR) {
		FCD(949,0);
		return;
		}
	len_to_jump = sizeof(scenario_data_type);
	len_to_jump += sizeof(scen_item_data_type);
	for (i = 0; i < 300; i++)
		len_to_jump += (long) scenario.scen_str_len[i];
	store = 0;
	for (i = 0; i < 100; i++)
		for (j = 0; j < 2; j++)
			store += (long) (scenario.out_data_size[i][j]);
	for (i = 0; i < which_town; i++)
		for (j = 0; j < 5; j++)
			store += (long) (scenario.town_data_size[i][j]);
	len_to_jump += store;

	error = SetFPos (file_id, 1, len_to_jump);
	if (error != 0) {FSClose(file_id);oops_error(35);}

	len = sizeof(town_record_type);

	if (mode == 0) {
		error = FSRead(file_id, &len , (char *) &c_town.town);
		port_town();
		}
		else error = FSRead(file_id, &len , (char *) &dummy_town);
	if (error != 0) {FSClose(file_id);oops_error(36);}

	switch (scenario.town_size[which_town]) {
		case 0:
			len =  sizeof(big_tr_type);
			if (mode == 0) {
				FSRead(file_id, &len, (char *) &t_d);
				port_t_d();
				}
				else error = SetFPos (file_id, 3, len);

			break;
			
		case 1:
				len = sizeof(ave_tr_type);
				if (mode == 0) {
					FSRead(file_id, &len, (char *) &ave_t);
					for (i = 0; i < 48; i++)
						for (j = 0; j < 48; j++) {
							t_d.terrain[i][j] = ave_t.terrain[i][j];
							t_d.lighting[i / 8][j] = ave_t.lighting[i / 8][j];					
							}
					//print_nums(5555,5555,5555);
					//for (i = 0; i < 8; i++)
					//	for (j = 0; j < 48; j++)
					//		if (t_d.lighting[i][j] != 0) {
					//			print_nums(i,j,t_d.lighting[i][j]);
					//			}

					for (i = 0; i < 16; i++) {
						t_d.room_rect[i] = ave_t.room_rect[i];
						}
					for (i = 0; i < 40; i++) {
						t_d.creatures[i] = ave_t.creatures[i];
						}
					for (i = 40; i < 60; i++) {
						t_d.creatures[i].number = 0;
						}
					port_t_d();
					}
					else error = SetFPos (file_id, 3, len);

			break;
			
		case 2:
			len = sizeof(tiny_tr_type);
			if (mode == 0) {
				FSRead(file_id,&len , (char *) &tiny_t);
				for (i = 0; i < 32; i++)
					for (j = 0; j < 32; j++) {
						t_d.terrain[i][j] = tiny_t.terrain[i][j];
						t_d.lighting[i / 8][j] = tiny_t.lighting[i / 8][j];					
						}
				for (i = 0; i < 16; i++) {
					t_d.room_rect[i] = tiny_t.room_rect[i];
				}
				for (i = 0; i < 30; i++) {
					t_d.creatures[i] = tiny_t.creatures[i];
					}
				for (i = 30; i < 60; i++) {
					t_d.creatures[i].number = 0;
					}
				port_t_d();
				}
				else error = SetFPos (file_id, 3, len);
			break;
		}
		
	for (i = 0; i < 140; i++) {
		len = (mode == 0) ? (long) (c_town.town.strlens[i]) : (long) (dummy_town.strlens[i]);
		switch (mode) {
			case 0:
				FSRead(file_id, &len, (char *) &(data_store->town_strs[i]));
				data_store->town_strs[i][len] = 0;
				break;
			case 1:
				SetFPos (file_id, 3, len);
				break;
			case 2:
				if (extra < 0) {
					FSRead(file_id, &len, (char *) &(data_store->town_strs[i]));
					data_store->town_strs[i][len] = 0;
					}
				else if (i == extra) {
					FSRead(file_id, &len, (char *) str);
					str[len] = 0;
					}
					else SetFPos (file_id, 3, len);
				break;
			}
		}

	if (mode < 2) {
		len = sizeof(talking_record_type);
		error = FSRead(file_id, &len , (char *) &talking);
		port_talk_nodes();
		if (error != 0) {FSClose(file_id);oops_error(37);}
	
		for (i = 0; i < 170; i++) {
			len = (long) (talking.strlens[i]);
			FSRead(file_id, &len, (char *) &(data_store3->talk_strs[i]));
			data_store3->talk_strs[i][len] = 0;
			}
		cur_town_talk_loaded = town_num;
		}
	if (mode == 0)
		town_type = scenario.town_size[which_town];
	error = FSClose(file_id);
	if (error != 0) {FSClose(file_id);oops_error(38);}
	
	// Now more initialization is needed. First need to properly create the misc_i array.
	
	// Initialize barriers, etc. Note non-sfx gets forgotten if this is a town recently visited.
	if (mode == 0) {
		for (i = 0; i < 64; i++)
			for (j = 0; j < 64; j++) {
				misc_i[i][j] = 0;
				sfx[i][j] = 0;
				}
		for (i = 0; i < 50; i++)
			if ((c_town.town.spec_id[i] >= 0) && (c_town.town.special_locs[i].x < 100)){
				make_special(c_town.town.special_locs[i].x,c_town.town.special_locs[i].y);
				}
	for (i = 0; i < 50; i++) {
			if ((c_town.town.preset_fields[i].field_type > 0) && (c_town.town.preset_fields[i].field_type < 9))
				misc_i[c_town.town.preset_fields[i].field_loc.x][c_town.town.preset_fields[i].field_loc.y] = 
				 misc_i[c_town.town.preset_fields[i].field_loc.x][c_town.town.preset_fields[i].field_loc.y] | 
			 	(unsigned char) (s_pow(2,c_town.town.preset_fields[i].field_type - 1));
			if ((c_town.town.preset_fields[i].field_type >= 14) && (c_town.town.preset_fields[i].field_type <= 21))
				sfx[c_town.town.preset_fields[i].field_loc.x][c_town.town.preset_fields[i].field_loc.y] = 
				 sfx[c_town.town.preset_fields[i].field_loc.x][c_town.town.preset_fields[i].field_loc.y] | 
				 (unsigned char) (s_pow(2,c_town.town.preset_fields[i].field_type - 14));

			}
		}

}

void shift_universe_left()
{
	short i,j;
	char create_line[60];
	
	make_cursor_watch();
	
	save_outdoor_maps();
	party.outdoor_corner.x--;
	party.i_w_c.x++;
	party.p_loc.x += 48;
	outdoors[1][0] = outdoors[0][0];
	outdoors[1][1] = outdoors[0][1];
	data_store4->outdoor_text[1][0] = data_store4->outdoor_text[0][0];
	data_store4->outdoor_text[1][1] = data_store4->outdoor_text[0][1];

	for (i = 48; i < 96; i++)
		for (j = 0; j < 96; j++)
			out_e[i][j] = out_e[i - 48][j];

	for (i = 0; i < 48; i++)
		for (j = 0; j < 96; j++)
			out_e[i][j] = 0;

	for (i = 0; i < 10; i++) {
		if (party.out_c[i].m_loc.x > 48)
			party.out_c[i].exists = FALSE;
		if (party.out_c[i].exists == TRUE) 
			party.out_c[i].m_loc.x += 48;
		}

	load_outdoors(party.outdoor_corner.x,party.outdoor_corner.y,0,0,0,0,NULL);
	load_outdoors(party.outdoor_corner.x,party.outdoor_corner.y + 1,0,1,0,0,NULL);
	build_outdoors();


	// reload graphics -- who knows what we added
	load_area_graphics();		
	make_cursor_sword();

}

void shift_universe_right()
{
	short i,j;
	
	make_cursor_watch();
	save_outdoor_maps();
	party.outdoor_corner.x++;
	party.i_w_c.x--;
	party.p_loc.x -= 48;
	outdoors[0][0] = outdoors[1][0];
	outdoors[0][1] = outdoors[1][1];
	data_store4->outdoor_text[0][0] = data_store4->outdoor_text[1][0];
	data_store4->outdoor_text[0][1] = data_store4->outdoor_text[1][1];
	for (i = 0; i < 48; i++)
		for (j = 0; j < 96; j++)
			out_e[i][j] = out_e[i + 48][j];
	for (i = 48; i < 96; i++)
		for (j = 0; j < 96; j++)
			out_e[i][j] = 0;


	for (i = 0; i < 10; i++) {
		if (party.out_c[i].m_loc.x < 48)
			party.out_c[i].exists = FALSE;
		if (party.out_c[i].exists == TRUE) 
			party.out_c[i].m_loc.x -= 48;
		}
	load_outdoors(party.outdoor_corner.x + 1,party.outdoor_corner.y,1,0,0,0,NULL);
	load_outdoors(party.outdoor_corner.x + 1,party.outdoor_corner.y + 1,1,1,0,0,NULL);
	build_outdoors();

	// reload graphics -- who knows what we added
	load_area_graphics();		
	make_cursor_sword();

}

void shift_universe_up()
{
	short i,j;
	
	make_cursor_watch();
	save_outdoor_maps();
	party.outdoor_corner.y--;
	party.i_w_c.y++;
	party.p_loc.y += 48;
	outdoors[0][1] = outdoors[0][0];
	outdoors[1][1] = outdoors[1][0];

	data_store4->outdoor_text[0][1] = data_store4->outdoor_text[0][0];
	data_store4->outdoor_text[1][1] = data_store4->outdoor_text[1][0];
		for (i = 0; i < 96; i++)
			for (j = 48; j < 96; j++)
				out_e[i][j] = out_e[i][j - 48];
		for (i = 0; i < 96; i++)
			for (j = 0; j < 48; j++)
				out_e[i][j] = 0;	 

	for (i = 0; i < 10; i++) {
		if (party.out_c[i].m_loc.y > 48)
			party.out_c[i].exists = FALSE;
		if (party.out_c[i].exists == TRUE)
			party.out_c[i].m_loc.y += 48; 
		}
	load_outdoors(party.outdoor_corner.x,party.outdoor_corner.y,0,0,0,0,NULL);
	load_outdoors(party.outdoor_corner.x + 1,party.outdoor_corner.y,1,0,0,0,NULL);

	build_outdoors();

	// reload graphics -- who knows what we added
	load_area_graphics();		
	make_cursor_sword();

}

void shift_universe_down()
{
	short i,j;

	make_cursor_watch();
	
	save_outdoor_maps();
	party.outdoor_corner.y++;
	party.i_w_c.y--;
	party.p_loc.y = party.p_loc.y - 48;
	outdoors[0][0] = outdoors[0][1];
	outdoors[1][0] = outdoors[1][1];

	data_store4->outdoor_text[0][0] = data_store4->outdoor_text[0][1];
	data_store4->outdoor_text[1][0] = data_store4->outdoor_text[1][1];
		for (i = 0; i < 96; i++)
			for (j = 0; j < 48; j++)
				out_e[i][j] = out_e[i][j + 48];
		for (i = 0; i < 96; i++)
			for (j = 48; j < 96; j++)
				out_e[i][j] = 0;	

	for (i = 0; i < 10; i++) {
		if (party.out_c[i].m_loc.y < 48)
			party.out_c[i].exists = FALSE;
		if (party.out_c[i].exists == TRUE) 
			party.out_c[i].m_loc.y = party.out_c[i].m_loc.y - 48;
		}
	load_outdoors(party.outdoor_corner.x,party.outdoor_corner.y + 1,0,1,0,0,NULL);
	load_outdoors(party.outdoor_corner.x + 1,party.outdoor_corner.y + 1,1,1,0,0,NULL);
	
	build_outdoors();

	// reload graphics -- who knows what we added
	load_area_graphics();		
	make_cursor_sword();

}
void position_party(short out_x,short out_y,short pc_pos_x,short pc_pos_y)
{
	short i,j;

	save_outdoor_maps();
	party.p_loc.x = pc_pos_x;
	party.p_loc.y = pc_pos_y;
	party.loc_in_sec = global_to_local(party.p_loc);
	
	if ((party.outdoor_corner.x != out_x) || (party.outdoor_corner.y != out_y)) {
		party.outdoor_corner.x = out_x;
		party.outdoor_corner.y = out_y;	
		load_outdoors(party.outdoor_corner.x + 1,party.outdoor_corner.y + 1,1,1,0,0,NULL);
		load_outdoors(party.outdoor_corner.x,party.outdoor_corner.y + 1,0,1,0,0,NULL);
		load_outdoors(party.outdoor_corner.x + 1,party.outdoor_corner.y,1,0,0,0,NULL);
		load_outdoors(party.outdoor_corner.x,party.outdoor_corner.y,0,0,0,0,NULL);
  		}
	party.i_w_c.x = (party.p_loc.x > 47) ? 1 : 0;
	party.i_w_c.y = (party.p_loc.y > 47) ? 1 : 0;
	for (i = 0; i < 10; i++)
		party.out_c[i].exists = FALSE;
	for (i = 0; i < 96; i++)
		for (j = 0; j < 96; j++) 	
			out_e[i][j] = 0;
	build_outdoors();	
}


void build_outdoors()
{
	short i,j,k,l,m;
	unsigned char exit_g_type[12] = {0,0,2,2,2, 28,26,6,30,2, 2,0};
		for (i = 0; i < 48; i++)
			for (j = 0; j < 48; j++) {
				out[i][j] = outdoors[0][0].terrain[i][j];
				out[48 + i][j] = outdoors[1][0].terrain[i][j];
				out[i][48 + j] = outdoors[0][1].terrain[i][j];
				out[48 + i][48 + j] = outdoors[1][1].terrain[i][j];	
			}
			
	fix_boats();
	add_outdoor_maps(); 
	make_out_trim();
	if (in_startup_mode == FALSE)
		erase_out_specials();
	
	for (i = 0; i < 10; i++)
		if (party.out_c[i].exists == TRUE)
			if ((party.out_c[i].m_loc.x < 0) || (party.out_c[i].m_loc.y < 0) ||
				(party.out_c[i].m_loc.x > 95) || (party.out_c[i].m_loc.y > 95))
				party.out_c[i].exists = FALSE;
	
}

short onm(char x_sector,char y_sector)
{
	short i;

	i = y_sector * scenario.out_width + x_sector;
	return i;
}


// This adds the current outdoor map info to the saved outdoor map info
void save_outdoor_maps()
{
	short i,j;
	
	for (i = 0; i < 48; i++)
		for (j = 0; j < 48; j++) {
			if (out_e[i][j] > 0) 
				o_maps.outdoor_maps[onm(party.outdoor_corner.x,party.outdoor_corner.y)][i / 8][j] =
				 o_maps.outdoor_maps[onm(party.outdoor_corner.x,party.outdoor_corner.y)][i / 8][j] |
				 (char) (s_pow(2,i % 8));	
			if (party.outdoor_corner.x + 1 < scenario.out_width) {
				if (out_e[i + 48][j] > 0) 
					o_maps.outdoor_maps[onm(party.outdoor_corner.x + 1,party.outdoor_corner.y)][i / 8][j] =
					 o_maps.outdoor_maps[onm(party.outdoor_corner.x + 1,party.outdoor_corner.y)][i / 8][j] |
					 (char) (s_pow(2,i % 8));
				}
			if (party.outdoor_corner.y + 1 < scenario.out_height) {
				if (out_e[i][j + 48] > 0) 
					o_maps.outdoor_maps[onm(party.outdoor_corner.x,party.outdoor_corner.y + 1)][i / 8][j] =
					 o_maps.outdoor_maps[onm(party.outdoor_corner.x,party.outdoor_corner.y + 1)][i / 8][j] |
					 (char) (s_pow(2,i % 8));
				}
			if ((party.outdoor_corner.y + 1 < scenario.out_height) &&
				(party.outdoor_corner.x + 1 < scenario.out_width)) {
				if (out_e[i + 48][j + 48] > 0) 
					o_maps.outdoor_maps[onm(party.outdoor_corner.x + 1,party.outdoor_corner.y + 1)][i / 8][j] =
					 o_maps.outdoor_maps[onm(party.outdoor_corner.x + 1,party.outdoor_corner.y + 1)][i / 8][j] |
					 (char) (s_pow(2,i % 8));
				}
			}
}

void add_outdoor_maps()  // This takes the existing outdoor map info and supplements it
						// with the saved map info
{
	short i,j;
	
	for (i = 0; i < 48; i++)
		for (j = 0; j < 48; j++) {
			if ((out_e[i][j] == 0) && 
			 ((o_maps.outdoor_maps[onm(party.outdoor_corner.x,party.outdoor_corner.y)][i / 8][j] &
			  (char) (s_pow(2,i % 8))) != 0))
			 	out_e[i][j] = 1;
			if (party.outdoor_corner.x + 1 < scenario.out_width) {
				if ((out_e[i + 48][j] == 0) && 
				 ((o_maps.outdoor_maps[onm(party.outdoor_corner.x + 1,party.outdoor_corner.y)][i / 8][j] &
				  (char) (s_pow(2,i % 8))) != 0))
				 	out_e[i + 48][j] = 1;		
			 		}	 	
			if (party.outdoor_corner.y + 1 < scenario.out_height) {
				if ((out_e[i][j + 48] == 0) && 
				 ((o_maps.outdoor_maps[onm(party.outdoor_corner.x,party.outdoor_corner.y + 1)][i / 8][j] &
				  (char) (s_pow(2,i % 8))) != 0))
				 	out_e[i][j + 48] = 1;
				}
			if ((party.outdoor_corner.y + 1 < scenario.out_height) &&
				(party.outdoor_corner.x + 1 < scenario.out_width)) {
				if ((out_e[i + 48][j + 48] == 0) && 
				 ((o_maps.outdoor_maps[onm(party.outdoor_corner.x + 1,party.outdoor_corner.y + 1)][i / 8][j] &
				  (char) (s_pow(2,i % 8))) != 0))
				 	out_e[i + 48][j + 48] = 1;
			 	}
			 }
} 



void fix_boats()
{
	short i;

	for (i = 0; i < 30; i++)
		if ((party.boats[i].exists == TRUE) && (party.boats[i].which_town == 200)) {
			if (party.boats[i].boat_sector.x == party.outdoor_corner.x)
				party.boats[i].boat_loc.x = party.boats[i].boat_loc_in_sec.x;
				else if (party.boats[i].boat_sector.x == party.outdoor_corner.x + 1)
				party.boats[i].boat_loc.x = party.boats[i].boat_loc_in_sec.x + 48;
					else party.boats[i].boat_loc.x = 500;
			if (party.boats[i].boat_sector.y == party.outdoor_corner.y) 
				party.boats[i].boat_loc.y = party.boats[i].boat_loc_in_sec.y;
				else if (party.boats[i].boat_sector.y == party.outdoor_corner.y + 1)
				party.boats[i].boat_loc.y = party.boats[i].boat_loc_in_sec.y + 48;
					else party.boats[i].boat_loc.y = 500;
			}
	for (i = 0; i < 30; i++)
		if ((party.horses[i].exists == TRUE) && (party.horses[i].which_town == 200)) {
			if (party.horses[i].horse_sector.x == party.outdoor_corner.x)
				party.horses[i].horse_loc.x = party.horses[i].horse_loc_in_sec.x;
				else if (party.horses[i].horse_sector.x == party.outdoor_corner.x + 1)
				party.horses[i].horse_loc.x = party.horses[i].horse_loc_in_sec.x + 48;
					else party.horses[i].horse_loc.x = 500;
			if (party.horses[i].horse_sector.y == party.outdoor_corner.y) 
				party.horses[i].horse_loc.y = party.horses[i].horse_loc_in_sec.y;
				else if (party.horses[i].horse_sector.y == party.outdoor_corner.y + 1)
				party.horses[i].horse_loc.y = party.horses[i].horse_loc_in_sec.y + 48;
					else party.horses[i].horse_loc.y = 500;
			}
}


void load_outdoors(short to_create_x, short to_create_y, short targ_x, short targ_y,
	short mode,short extra,char *str)
	//short	to_create_x, to_create_y; // actual sector being loaded
//short 	targ_x, targ_y; // 0 or 1
{
	HFILE file_id;
	short error;
	short i,j,k,out_sec_num;
	long int num_records_to_offset;
	char to_put;
	char store_name[256],file_name[256];
	OFSTRUCT store_str;
	long len;
	LONG l;
	long store_dir,len_to_jump = 0,store = 0;

	if ((to_create_x != minmax(0,scenario.out_width - 1,to_create_x)) ||
		(to_create_y != minmax(0,scenario.out_height - 1,to_create_y))) { // not exist
			for (i = 0; i < 48; i++)
				for (j = 0; j < 48; j++)
					outdoors[targ_x][targ_y].terrain[i][j] = 5;
			for (i = 0; i < 18; i++) {
				outdoors[targ_x][targ_y].special_id[i] = -1;
				outdoors[targ_x][targ_y].special_locs[i].x = 100;
				}
			return;
			}

	build_scen_file_name(file_name);
	file_id = OpenFile(file_name,&store_str,OF_READ | OF_SEARCH);
	if (file_id == HFILE_ERROR) {
		outdoor_alert();
		PostQuitMessage(0);
		}

	out_sec_num = scenario.out_width * to_create_y + to_create_x;

	len_to_jump = sizeof(scenario_data_type);
	len_to_jump += sizeof(scen_item_data_type);
	for (i = 0; i < 300; i++)
		len_to_jump += (long) scenario.scen_str_len[i];
	store = 0;
	for (i = 0; i < out_sec_num; i++)
		for (j = 0; j < 2; j++)
			store += (long) (scenario.out_data_size[i][j]);
	len_to_jump += store;

	error = SetFPos (file_id, 1, len_to_jump);
	if (error != 0) {FSClose(file_id);oops_error(32);}

	len = sizeof(outdoor_record_type);
	if (mode == 0) {
		error = FSRead(file_id, &len, (char *) &outdoors[targ_x][targ_y]);
		port_out(&outdoors[targ_x][targ_y]);
		if (error != 0) {FSClose(file_id);oops_error(33);}
		}
		else error = FSRead(file_id, &len, (char *) &dummy_out);
		
	if (mode == 0) {
		for (i = 0; i < 9; i++) {
			len = (long) (outdoors[targ_x][targ_y].strlens[i]);
			FSRead(file_id, &len, (char *) &(data_store4->outdoor_text[targ_x][targ_y].out_strs[i]));
			data_store4->outdoor_text[targ_x][targ_y].out_strs[i][len] = 0;
			}
		}
	if (mode == 1) {
		for (i = 0; i < 120; i++) {
			len = (long) (dummy_out.strlens[i]);
			if (i == extra) {
				FSRead(file_id, &len, (char *) str);
				str[len] = 0;
				}
				else SetFPos (file_id, 3, len);	
			}
	
		}

	error = FSClose(file_id);
	if (error != 0) {FSClose(file_id);oops_error(33);}
}


void get_reg_data()
{
	HFILE f;
	short i,choice;
	long *val_store,vals[10],len = 4;
	OFSTRUCT store;

	f = OpenFile("bladmisc.dat",&store,OF_READ | OF_SEARCH);

	if (f == HFILE_ERROR) {
		game_run_before = FALSE;
		build_data_file(1);
		f = OpenFile("bladmisc.dat",&store,OF_READ | OF_SEARCH);

		if (f == HFILE_ERROR) {
			reg_alert();
			register_flag = -1;
			return;
			}
			else {
				_lclose(f);
				return;
				}
		}
	_llseek(f,0,0);

	for (i = 0; i < 10; i++) {
		_hread(f,(char *) &(vals[i]), 4);
		if (i == 2)
      	give_intro_hint = vals[i];
		if (i == 3)
			play_sounds = (vals[i] == 0) ? FALSE : TRUE;
		if (i == 4)
			display_mode = (short) vals[i];
		if ((display_mode < 0) || (display_mode > 5))
      	display_mode = 0;
		if (i == 6)
			register_flag = vals[i];
		if (i == 8)
			ed_flag = vals[i];
		}
	if (vals[5] == 2000) {
		game_run_before = FALSE;
		_lclose (f);
		build_data_file(1);
		return;
		}
		else {
			//if (vals[9] == init_data(vals[8]))
			//	ed_reg = TRUE;
			//if (vals[7] == init_data(vals[6]))
			//	registered = TRUE;
			}
	_lclose (f);

}

void build_data_file(short mode)
//mode; // 0 - make first time file  1 - customize  2 - new write
{
	short i,j,k,file_id;
	long val_store,to_return = 0,len = 4,s_vals[10] = {0,0,0,0,0, 0,0,0,0,0};
	OFSTRUCT store;
	HFILE f;
	char debug_str[60];

	f = OpenFile("bladmisc.dat",&store,OF_READWRITE | OF_SEARCH);
	if (f == HFILE_ERROR)
		f = OpenFile("bladmisc.dat",&store,OF_WRITE | OF_CREATE | OF_SEARCH);
		else {
			_llseek(f,0,0);
			for (i = 0; i < 10; i++)
				_hread(f,(char *) &(s_vals[i]), 4);

			//sprintf(debug_str,"Starting %d: %d",i,(short) s_vals[i]);
			//add_string_to_buf(debug_str);
			}

	if (f == HFILE_ERROR) {
		reg_alert();
		register_flag = -1;
		ed_flag = -1;
		return;
		}
	_llseek(f,0,0);

	for (i = 0; i < 10; i++) {
		if (mode < 2)
			val_store = (long) (get_ran(1,20000,30000));
			else val_store = s_vals[i];
		switch (i) {
			case 2: // tip of day
				if (mode < 2)
					val_store = 1;
					else val_store = give_intro_hint;
				break;
			case 3: // sounds
				if (mode < 2)
					val_store = 1;
					else if (play_sounds == TRUE)
						val_store = 1;
						else val_store = 0;
				break;
			case 4:  // display_mode
				if (mode < 2)
					val_store = 0;
					else val_store = display_mode;
				break;
			case 5:
				val_store = (mode == 0) ? 2000 : 1000;
				break;
			case 6:
				if (mode == 2)
					val_store = s_vals[i];
				register_flag = val_store;
				break;
			case 7:
				if ((mode == 2) && (registered == TRUE))
					val_store = init_data(register_flag);
					else val_store = val_store + 50000;
				break;
			case 8:
				if (mode < 2)
					val_store = (long) (get_ran(1,1000,5000));
					else val_store = s_vals[i];
				ed_flag = val_store;
				break;
			case 9:
				if ((mode == 2) && (ed_reg == TRUE))
					val_store = init_data(ed_flag);
					else val_store = val_store + 50000;
				break;
			}
		_lwrite(f, (char *) &val_store, 4);
		}

	_lclose(f);

}

// expecting party record to contain name of proper scenario to load
Boolean load_scenario()
{

	short i,j,k,l,error;
	HFILE file_id;
	Boolean file_ok = FALSE;
	long len;
	char file_name[256];
	 OFSTRUCT store;

	build_scen_file_name(file_name);
	
	file_id = OpenFile(file_name,&store,OF_READ | OF_SEARCH);
	if (file_id == HFILE_ERROR) {
		oops_error(10000);
		SysBeep(2);	return FALSE;
		}
		
	len = (long) sizeof(scenario_data_type);
	if ((error = FSRead(file_id, &len, (char *) &scenario)) != 0){
		FSClose(file_id); oops_error(29); return FALSE;
		}
	if ((scenario.flag1 == 10) && (scenario.flag2 == 20)
	 && (scenario.flag3 == 30)
	  && (scenario.flag4 == 40)) {
	  	file_ok = TRUE;
		cur_scen_is_win = FALSE;
		port_scenario();
		}
	if ((scenario.flag1 == 20) && (scenario.flag2 == 40)
	 && (scenario.flag3 == 60)
	  && (scenario.flag4 == 80)) {
		file_ok = TRUE;
		cur_scen_is_win = TRUE;
		}
	 if (file_ok == FALSE) {
		FSClose(file_id); 
		give_error("This is not a legitimate Blades of Exile scenario.","",0);
		return FALSE;	 
	 	}
	len = sizeof(scen_item_data_type); // item data
	if ((error = FSRead(file_id, &len, (char *) &(data_store2->scen_item_list))) != 0){
		FSClose(file_id); oops_error(30); return FALSE;
		}
	port_item_list();
	for (i = 0; i < 270; i++) {
		len = (long) (scenario.scen_str_len[i]);
		if (i < 160) {
			FSRead(file_id, &len, (char *) &(data_store5->scen_strs[i]));
			data_store5->scen_strs[i][len] = 0;
			}
			else {
				FSRead(file_id, &len, (char *) &(scen_strs2[i - 160]));
				scen_strs2[i - 160][len] = 0;
				}
		}
	
	FSClose(file_id);
	load_spec_graphics();
	set_up_ter_pics();
	return TRUE;
}

void set_up_ter_pics()
{
	short i;
	
	set_terrain_blocked();
	for (i = 0; i < 256; i++)
		terrain_pic[i] = scenario.ter_types[i].picture;
}
void oops_error(short error)
{
	char error_str[256];
	
		SysBeep(50);
		SysBeep(50);
		SysBeep(50);
	sprintf((char *) error_str,"Giving the scenario editor more memory might also help. Be sure to back your scenario up often. Error number: %d.",error);
	give_error("The program encountered an error while loading/saving/creating the scenario. To prevent future problems, the program will now terminate. Trying again may solve the problem.",(char *) error_str,0);
	//ExitToShell();
}

/*

typedef struct {
	unsigned char flag1, flag2, flag3, flag4;
	unsigned char ver[3],min_run_ver,prog_make_ver[3],num_towns;
	unsigned char out_width,out_height,difficulty,intro_pic,default_ground;
	} scen_header_type;
*/
void build_scen_headers()
{
	short i,index = 1;
	short cur_entry = 0;
	HWND listbox;
	WORD count;
	char filename[256],filename2[256];
	
	for (i = 0; i < 25; i++)
		scen_headers[i].flag1 = 0;
	listbox = CreateWindow("listbox", NULL,
		WS_CHILDWINDOW, 0,0,0,0,
		mainPtr, 1, GetWindowWord(mainPtr, GWW_HINSTANCE), NULL);
	SendMessage(listbox,LB_DIR,0x0,(LONG) (LPSTR) "BLADSCEN/*.EXS");
	count = (WORD) SendMessage(listbox,LB_GETCOUNT,0,0L);

	count = min(count,20);

	for (i = 0; i < count; i++) {
		SendMessage(listbox,LB_GETTEXT,i,(LONG) (LPSTR) filename2);
		sprintf((char *) filename,"BLADSCEN/%s",filename2);

		if (load_scenario_header(filename,cur_entry) == TRUE) {
			// now we need to store the file name, first stripping any path that occurs
			// before it
			strcpy((char *) data_store2->scen_names[cur_entry],(char *) filename2);
			cur_entry++;
			}
		}

	DestroyWindow(listbox);
}

// This is only called at startup, when bringing headers of active scenarios.
// This wipes out the scenario record, so be sure not to call it while in an active scenario.
Boolean load_scenario_header(char *filename,short header_entry)
{

	short i,j,k,l;
	HFILE file_id;
	short store;
	Boolean file_ok = FALSE;
	long len;
	char load_str[256];
	Boolean mac_header = TRUE;
	OFSTRUCT store_str;
	short error;

	file_id = OpenFile(filename,&store_str,OF_READ | OF_SEARCH);
	if (file_id == HFILE_ERROR) {
		ASB(filename);
		return FALSE;
		}	
		
	len = (long) sizeof(scen_header_type);
	if ((error = FSRead(file_id, &len, (char *) &(scen_headers[header_entry]))) != 0){
		FSClose(file_id); return FALSE;
		}
	if ((scen_headers[header_entry].flag1 == 10) && (scen_headers[header_entry].flag2 == 20)
	 && (scen_headers[header_entry].flag3 == 30)
	  && (scen_headers[header_entry].flag4 == 40)) {
	  	file_ok = TRUE;
	  	mac_header = TRUE;
	  	}
	if ((scen_headers[header_entry].flag1 == 20) && (scen_headers[header_entry].flag2 == 40)
	 && (scen_headers[header_entry].flag3 == 60)
	  && (scen_headers[header_entry].flag4 == 80)) {
	  	file_ok = TRUE;
	  	mac_header = FALSE;
	  	}
	 if (file_ok == FALSE) {
		scen_headers[header_entry].flag1 = 0;
		FSClose(file_id); 
		return FALSE;	 
	 	}

	// So file is OK, so load in string data and close it.
	SetFPos(file_id,1,0);
	len = (long) sizeof(scenario_data_type);
	if ((error = FSRead(file_id, &len, (char *) &scenario)) != 0){
		FSClose(file_id); oops_error(29); return FALSE;
		}
	store = scenario.rating;
	if (mac_header == TRUE)
		flip_short(&store);
	scen_headers[header_entry].default_ground = store;
	
	len = sizeof(scen_item_data_type);
	if (SetFPos(file_id,3,len) != 0)
		return FALSE;
		
	for (i = 0; i < 3; i++) {
		store = (short) scenario.scen_str_len[i];
		len = (long) (store);
		FSRead(file_id, &len, (char *) load_str);
		load_str[len] = 0;
		if (i == 0)
			load_str[29] = 0;
			else load_str[59] = 0;
		strcpy(data_store2->scen_header_strs[header_entry][i],(char *) load_str);
		}
	
	FSClose(file_id);

	return TRUE;
}

//extern GWorldPtr spec_scen_g;
void load_spec_graphics()
{
	short i,file_num;
	char file_name[256];
	
	if (spec_scen_g != NULL) {
		DeleteObject(spec_scen_g);
		spec_scen_g = NULL;
		}
	//build_scen_file_name(file_name);
	sprintf(file_name,"bladscen/%s",party.scen_name);
	for (i = 0; i < 256; i++) {
		if (file_name[i] == '.') {
			file_name[i + 1] = 'b';
			file_name[i + 2] = 'm';
			file_name[i + 3] = 'p';
			i = 256;
			}
		}
	spec_scen_g = ReadDib(file_name,main_dc);
}


short init_data(short flag)
{
	long k = 0;
		
	k = (long) flag;
	k = k * k;
	jl = jl * jl + 84 + k;
	k = k + 51;
		jl = jl * 2 + 1234 + k;
k = k % 3000;
	jl = jl * 54;
	jl = jl * 2 + 1234 + k;
	k = k * 82;
	k = k % 10000;
	k = k + 10000;
	
	return (short) k;
}

short town_s(short flag)
{
	long k = 0;
		
	k = (long) flag;
	k = k * k * k;
	jl = jl * 54;
	jl = jl * 2 + 1234 + k;
	k = k + 51;
	k = k % 3000;
	jl = jl * 2 + 1234 + k;
	k = k * scenario.num_towns;
	k = k % 10000;
	jl = jl * jl + 84 + k;
	k = k + 10000;
	
	return (short) k;
}

short out_s(short flag)
{
	long k = 0;
		
	k = (long) flag;
	k = k * k * k;
	jl = jl * jl + 84 + k;
	k = k + scenario.out_data_size[0][1];
	k = k % 3000;
	k = k * 4;
	jl = jl * 2 + 1234 + k;
	jl = jl * 54;
	jl = jl * 2 + 1234 + k;
	k = k % 10000;
	k = k + 4;
	
	return (short) k;
}

short str_size_1(short flag)
{
	long k = 0;
		
	k = (long) flag;
	k = k * k;
	jl = jl * 2 + 1234 + k;
	jl = jl * jl + 84 + k;
	k = k + scenario.scen_str_len[0] + scenario.scen_str_len[1] + scenario.scen_str_len[2];
	jl = jl * 2 + 1234 + k;
	k = k % 3000;
	jl = jl * 54;
	jl = jl * jl + 84 + k;
	k = k * 4;
	k = k % 5000;
	k = k - 9099;
	
	return (short) k;
}

short str_size_2(short flag)
{
	long k = 0;
		
	k = (long) flag;
	jl = jl * jl + 84 + k;
	k = k * k * k * k;
	jl = jl * 54;
	k = k + 80;
	k = k % 3000;
	jl = jl * 2 + 1234 + k;
	k = k * scenario.out_width * scenario.out_height;
	jl = jl * jl + 84 + k;
	k = k % 3124;
	k = k - 5426;
	
	return (short) k;
}

short str_size_3(short flag)
{
	long k = 0;
		
	k = (long) flag;
	k = k * (scenario.town_data_size[0][0] +  scenario.town_data_size[0][1] +  scenario.town_data_size[0][2] +  scenario.town_data_size[0][3]);
	k = k + 80;
	jl = jl * jl + 84 + k;
	k = k % 3000;
	jl = jl * 2 + 1234 + k;
	k = k * 45;
	jl = jl * 54;
	jl = jl * jl + 84 + k;
	k = k % 887;
	k = k + 9452;
	
	return (short) k;
}

short get_buf_ptr(short flag)
{
	long k = 0;
		
	k = (long) flag;
	jl = jl * jl + 84 + k;
	k = k * (scenario.out_width +  scenario.out_width +  scenario.out_height +  scenario.town_data_size[0][3]);
	k = k + 80;
	jl = jl * jl + 84 + k;
	k = k % 2443;
	jl = jl * 54;
	k = k * 322;
	jl = jl * 2 + 1234 + k;
	k = k % 2542;
	jl = jl * jl + 84 + k;
	k = k + 234;
	
	return (short) k;
}

Boolean check_p (short pword)
{
	if (scenario.flag_b != town_s(pword))
		return FALSE;
	if (scenario.flag_c != out_s(pword))
		return FALSE;
	if (scenario.flag_e != str_size_1(pword))
		return FALSE;
	if (scenario.flag_f != str_size_2(pword))
		return FALSE;
	if (scenario.flag_h != str_size_3(pword))
		return FALSE;
	if (scenario.flag_d != init_data(pword))
		return FALSE;

	return TRUE;
}


void town_alert()
{
	MessageBox(mainPtr,"Cannot find or read file town.dat! Please make sure it is in the same folder as Exile III."	,
	  "File Error",MB_OK | MB_ICONEXCLAMATION);
	ExitToShell();
}
void outdoor_alert()
{
	MessageBox(mainPtr,"Cannot find or read file outdoor.dat! Please make sure it is in the same folder as Exile III.",
	  "File Error",MB_OK | MB_ICONEXCLAMATION);
	ExitToShell();
}
void reg_alert()
{
	MessageBox(mainPtr,"Cannot create/read file misc.dat! This volume may be locked. Exile III can be still be played.",
	  "File Error",MB_OK | MB_ICONEXCLAMATION);
}

 //	MessageBox(mainPtr,"A","Debug note",MB_OK | MB_ICONEXCLAMATION);

void port_talk_nodes()
{
	short i,j,k,l;

	if (cur_scen_is_win == TRUE)
		return;
	for (i = 0; i < 60; i++) {
		flip_short(&talking.talk_nodes[i].personality);
		flip_short(&talking.talk_nodes[i].type);
		flip_short(&talking.talk_nodes[i].extras[0]);
		flip_short(&talking.talk_nodes[i].extras[1]);
		flip_short(&talking.talk_nodes[i].extras[2]);
		flip_short(&talking.talk_nodes[i].extras[3]);
		}
}

void port_town()
{
	short i,j,k,l;

	if (cur_scen_is_win == TRUE)
		return;
	flip_short(&c_town.town.town_chop_time);
	flip_short(&c_town.town.town_chop_key);
	flip_short(&c_town.town.lighting);
	for (i =0 ; i < 4; i++)
		flip_short(&c_town.town.exit_specs[i]);
	flip_rect(&c_town.town.in_town_rect);
	for (i =0 ; i < 64; i++) {
		flip_short(&c_town.town.preset_items[i].item_code);
		flip_short(&c_town.town.preset_items[i].ability);
		}
	for (i =0 ; i < 50; i++) {
		flip_short(&c_town.town.preset_fields[i].field_type);
		}
	flip_short(&c_town.town.max_num_monst);
	flip_short(&c_town.town.spec_on_entry);
	flip_short(&c_town.town.spec_on_entry_if_dead);
	for (i =0 ; i < 8; i++) 
		flip_short(&c_town.town.timer_spec_times[i]);
	for (i =0 ; i < 8; i++) 
		flip_short(&c_town.town.timer_specs[i]);
	flip_short(&c_town.town.difficulty);
	for (i =0 ; i < 100; i++) 
		flip_spec_node(&c_town.town.specials[i]);

}


void port_t_d()
{
	short i,j,k,l;
	if (cur_scen_is_win == TRUE)
		return;

	for (i =0 ; i < 16; i++)
		flip_rect(&t_d.room_rect[i]);
	for (i =0 ; i < 60; i++) {
		flip_short(&t_d.creatures[i].spec1);
		flip_short(&t_d.creatures[i].spec2);
		flip_short(&t_d.creatures[i].monster_time);
		flip_short(&t_d.creatures[i].personality);
		flip_short(&t_d.creatures[i].special_on_kill);
		flip_short(&t_d.creatures[i].facial_pic);

		}
}

void port_scenario()
{
	short i,j,k,l;

	if (cur_scen_is_win == TRUE)
		return;

	flip_short(&scenario.flag_a);
	flip_short(&scenario.flag_b);
	flip_short(&scenario.flag_c);
	flip_short(&scenario.flag_d);
	flip_short(&scenario.flag_e);
	flip_short(&scenario.flag_f);
	flip_short(&scenario.flag_g);
	flip_short(&scenario.flag_h);
	flip_short(&scenario.flag_i);
	flip_short(&scenario.intro_mess_pic);
	flip_short(&scenario.intro_mess_len);
	flip_short(&scenario.which_town_start);
	for (i = 0; i < 200; i++)
		for (j = 0; j < 5; j++)
			flip_short(&scenario.town_data_size[i][j]);
	for (i = 0; i < 10; i++)
		flip_short(&scenario.town_to_add_to[i]);
	for (i = 0; i < 10; i++)
		for (j = 0; j < 2; j++)
			flip_short(&scenario.flag_to_add_to_town[i][j]);
	for (i = 0; i < 100; i++)
		for (j = 0; j < 2; j++)
			flip_short(&scenario.out_data_size[i][j]);
	for (i = 0; i < 3; i++)
		flip_rect(&scenario.store_item_rects[i]);
	for (i = 0; i < 3; i++)
		flip_short(&scenario.store_item_towns[i]);
	for (i = 0; i < 50; i++)
		flip_short(&scenario.special_items[i]);
	for (i = 0; i < 50; i++)
		flip_short(&scenario.special_item_special[i]);
	flip_short(&scenario.rating);
	flip_short(&scenario.uses_custom_graphics);
	for (i = 0; i < 256; i++) {
		flip_short(&scenario.scen_monsters[i].health);
		flip_short(&scenario.scen_monsters[i].m_health);
		flip_short(&scenario.scen_monsters[i].max_mp);
		flip_short(&scenario.scen_monsters[i].mp);
		flip_short(&scenario.scen_monsters[i].a[1]);
		flip_short(&scenario.scen_monsters[i].a[0]);
		flip_short(&scenario.scen_monsters[i].a[2]);
		flip_short(&scenario.scen_monsters[i].morale);
		flip_short(&scenario.scen_monsters[i].m_morale);
		flip_short(&scenario.scen_monsters[i].corpse_item);
		flip_short(&scenario.scen_monsters[i].corpse_item_chance);
		flip_short(&scenario.scen_monsters[i].picture_num);
		}

	for (i = 0; i < 256; i++) {
		flip_short(&scenario.ter_types[i].picture);
		}
	for (i = 0; i < 30; i++) {
		flip_short(&scenario.scen_boats[i].which_town);
		}
	for (i = 0; i < 30; i++) {
		flip_short(&scenario.scen_horses[i].which_town);
		}
	for (i = 0; i < 20; i++)
		flip_short(&scenario.scenario_timer_times[i]);
	for (i = 0; i < 20; i++)
		flip_short(&scenario.scenario_timer_specs[i]);
	for (i = 0; i < 256; i++) {
		flip_spec_node(&scenario.scen_specials[i]);
		}
	for (i = 0; i < 10; i++)  {
		flip_short(&scenario.storage_shortcuts[i].ter_type);
		flip_short(&scenario.storage_shortcuts[i].property);
		for (j = 0; j < 10; j++)  {
			flip_short(&scenario.storage_shortcuts[i].item_num[j]);
			flip_short(&scenario.storage_shortcuts[i].item_odds[j]);
			}
		}
	flip_short(&scenario.last_town_edited);
}


void port_item_list()
{
	short i,j,k,l;

	if (cur_scen_is_win == TRUE)
		return;

	for (i = 0; i < 400; i++) {
		flip_short(&(data_store2->scen_item_list.scen_items[i].variety));
		flip_short(&(data_store2->scen_item_list.scen_items[i].item_level));
		flip_short(&(data_store2->scen_item_list.scen_items[i].value));
		}
}

void port_out(outdoor_record_type *out)
{
	short i,j,k,l;

	if (cur_scen_is_win == TRUE)
		return;

	for (i = 0; i < 4; i++) {
		flip_short(&(out->wandering[i].spec_on_meet));
		flip_short(&(out->wandering[i].spec_on_win));
		flip_short(&(out->wandering[i].spec_on_flee));
		flip_short(&(out->wandering[i].cant_flee));
		flip_short(&(out->wandering[i].end_spec1));
		flip_short(&(out->wandering[i].end_spec2));
		flip_short(&(out->special_enc[i].spec_on_meet));
		flip_short(&(out->special_enc[i].spec_on_win));
		flip_short(&(out->special_enc[i].spec_on_flee));
		flip_short(&(out->special_enc[i].cant_flee));
		flip_short(&(out->special_enc[i].end_spec1));
		flip_short(&(out->special_enc[i].end_spec2));
		}
	for (i = 0; i < 8; i++)
		flip_rect(&(out->info_rect[i]));
	for (i = 0; i < 60; i++)
		flip_spec_node(&(out->specials[i]));
}

void flip_spec_node(special_node_type *spec)
{
	flip_short(&(spec->type));
	flip_short(&(spec->sd1));
	flip_short(&(spec->sd2));
	flip_short(&(spec->pic));
	flip_short(&(spec->m1));
	flip_short(&(spec->m2));
	flip_short(&(spec->ex1a));
	flip_short(&(spec->ex1b));
	flip_short(&(spec->ex2a));
	flip_short(&(spec->ex2b));
	flip_short(&(spec->jumpto));
}

void flip_short(short *s)
{
	char store,*s1, *s2;

	s1 = (char *) s;
	s2 = s1 + 1;
	store = *s1;
	*s1 = *s2;
	*s2 = store;

}


void flip_rect(RECT *s)
{
	flip_short((short *) &(s->top));
	flip_short((short *) &(s->bottom));
	flip_short((short *) &(s->left));
	flip_short((short *) &(s->right));
	alter_rect(s);
	}

short FSWrite(HFILE file,long *len,char *buffer)
{
	long error = 0;

	if ((error = _lwrite(file, (char *) buffer, (UINT) (*len)))  == HFILE_ERROR)
		return -1;
	return 0;
}

short FSRead(HFILE file,long *len,char *buffer)
{
	long error = 0;

	if ((error = _lread(file, (char *) buffer, (UINT) (*len)))  == HFILE_ERROR)
		return -1;
	return 0;
		
}

short FSClose(HFILE file)
{
	_lclose(file);
}

short SetFPos(HFILE file, short mode, long len)
{
	long error = 0; 
	
	switch (mode) {
		case 1: error = _llseek(file,len,0); break; 
		case 2: error = _llseek(file,len,2); break; 
		case 3: error = _llseek(file,len,1); break; 
		}

	if (error == HFILE_ERROR)
		return -1;
	return 0;
}

Boolean load_blades_data()
{
	short i,j,k,l,error;
	HFILE file_id;
	Boolean file_ok = FALSE;
	long len;
	char file_name[256];
	 OFSTRUCT store;

	build_scen_ed_name(file_name);

	file_id = OpenFile(file_name,&store,OF_READ | OF_SEARCH);
	if (file_id == HFILE_ERROR) {
		return FALSE;
		}
	FSClose(file_id);
   return TRUE;
}
