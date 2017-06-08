#include <Windows.h>
#include <commdlg.h>

#include <string.h>
#include "stdio.h"

#include "global.h"
#include "edfileio.h"
#include "graphics.h"
#include "edsound.h"
#include "editors.h"


/* Adventure globals */
extern party_record_type far party;
extern pc_record_type far adven[6];
extern outdoor_record_type far outdoors[2][2];
extern current_town_type far c_town;
extern big_tr_type far t_d;
extern town_item_list far	t_i;
extern unsigned char misc_i[64][64],sfx[64][64];
extern unsigned char far out[96][96];
extern unsigned char far out_e[96][96];
extern setup_save_type far setup_save;
extern stored_items_list_type far stored_items[3];
extern stored_town_maps_type far maps;
extern stored_outdoor_maps_type far o_maps;

extern Boolean registered,play_sounds,sys_7_avail,save_blocked,ed_reg,party_in_scen;
extern short current_active_pc;
extern long register_flag,stored_key;
extern HWND mainPtr;
extern long ed_flag,ed_key;

extern Boolean file_in_mem;


typedef struct {
	char expl[96][96];
	}	out_info_type;

extern short store_flags[3];

// Big waste!
out_info_type far store_map;
char last_load_file[63] = "blades.sav";
char szFileName [128] = "blades.sav";
char szTitleName [128] = "blades.sav";
OPENFILENAME ofn;
extern stored_town_maps_type far town_maps,town_maps2;

short give_intro_hint = 1;
short display_mode = 0;
 short store_size;

void file_initialize()
{
static char *szFilter[] = {"Blades of Exile Save Files (*.SAV)","*.sav",
		"Text Files (*.TXT)","*.txt",
		"All Files (*.*)","*.*",
		""};


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
	Boolean in_scen = FALSE;

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
		play_sound(0);
		return;
		}

	file_size = sizeof(party_record_type);

	len = sizeof(flag_type);

//	sprintf((char *) debug, "  Len %d               ", (short) len);
//	add_string_to_buf((char *) debug);

	for (i = 0; i < 3; i++) {
		if ((error = _lread(file_id, (char *) flag_data, len)) == HFILE_ERROR) {
			play_sound(0);
			return;
			}
		flag_ptr = (flag_type *) flag_data;
		flag = *flag_ptr;
		store_flags[i] = (short) flag.i;
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
		play_sound(0);
		FCD(1064,0);
		return;
		}
	for (count = 0; count < store_len; count++)
		party_ptr[count] ^= 0x5C;	

	// LOAD SETUP
	len = (UINT) sizeof(setup_save_type);
	if ((error = _lread(file_id, (char *) &setup_save, len)) == HFILE_ERROR){
		_lclose(file_id);
		play_sound(0);
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
			play_sound(0);
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
		play_sound(0);
		FCD(1064,0);
		return;
		}

	// LOAD TOWN 
	if (town_restore == TRUE) {
		len = (long) sizeof(current_town_type);
		if ((error = FSRead(file_id, &len, (char *) &c_town)) != 0){
				FSClose(file_id);
				play_sound(0);
		FCD(1064,0);
				return;
				}
	
		len = (long) sizeof(big_tr_type);
		if ((error = FSRead(file_id, &len, (char *) &t_d)) != 0){
				FSClose(file_id);
				play_sound(0);
		FCD(1064,0);
				return;
				}

		len = (long) sizeof(town_item_list);
		if ((error = FSRead(file_id, &len, (char *) &t_i))  != 0){
			FSClose(file_id);
			play_sound(0);
		FCD(1064,0);
			return;
			}
	
		}

	// LOAD STORED ITEMS
	for (i = 0; i < 3; i++) {
		len = (long) sizeof(stored_items_list_type);
		if ((error = FSRead(file_id, &len, (char *) &stored_items[i]))  != 0){
				FSClose(file_id);
				play_sound(0);
		FCD(1064,0);
				return;
				}		
		}

	// LOAD SAVED MAPS
	if (maps_there == TRUE) {
		len = (long) sizeof(stored_town_maps_type);
		if ((error = FSRead(file_id, &len, (char *) &(town_maps)))  != 0){
				FSClose(file_id);
				play_sound(0);
				FCD(1064,0);
				return;
				}
		len = (long) sizeof(stored_town_maps_type);
		if ((error = FSRead(file_id, &len, (char *) &(town_maps2)))  != 0){
				FSClose(file_id);
				play_sound(0);
				FCD(1064,0);
				return;
				}
	
		len = (long) sizeof(stored_outdoor_maps_type);
		if ((error = FSRead(file_id, &len, (char *) &o_maps)) != 0) {
				FSClose(file_id);
				play_sound(0);
		FCD(1064,0);
				return;
				}	
		}

	// LOAD SFX & MISC_I
		len = (long) (64 * 64);
		if ((error = FSRead(file_id, &len, (char *) sfx))  != 0){
				FSClose(file_id);
				play_sound(0);
		FCD(1064,0);
				return;
				}	
		if ((error = FSRead(file_id, &len, (char *) misc_i))  != 0){
				FSClose(file_id);
				play_sound(0);
		FCD(1064,0);
				return;
				}	

	} // end if_scen

	if ((error = _lclose(file_id))  == HFILE_ERROR){
		add_string_to_buf("Load: Can't close file.          ");
		play_sound(0);
		return;
		}

	
	for (i = 0; i < 6; i++)
		if (adven[i].main_status > 0) {
			current_active_pc = i;
			i = 6;
			}

	file_in_mem = TRUE;
	save_blocked = FALSE;
	party_in_scen = in_scen;

	redraw_screen();
}

void save_file(short mode)
//short mode;  // 0 - normal  1 - save as
{

	long file_size;
	HFILE file_id;
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
		Boolean got_error = FALSE,town_save = FALSE,in_scen = FALSE,save_maps = FALSE;

      mode = 1;
	if (file_in_mem == FALSE)
		return;

	if (save_blocked == TRUE) {
		FCD(903,0);
		return;
		}

	if (store_flags[0] == 1342)
		town_save = TRUE;
		else town_save = FALSE;
	if (store_flags[1] == 100)
		in_scen = TRUE;
		else in_scen = FALSE;
	if (store_flags[2] == 5567)  {
		save_maps = TRUE;
  		}
		else save_maps = FALSE;

	ofn.hwndOwner = mainPtr;
	ofn.lpstrFile = szFileName;
	ofn.lpstrFileTitle = szTitleName;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	if (mode == 1) {
		if (GetSaveFileName(&ofn) == 0)
			return;
		}

	if (-1 == (file_id = _lopen(szFileName, OF_WRITE | OF_SHARE_EXCLUSIVE)))
		if (-1 == (file_id = _lcreat(szFileName,0))) {
			play_sound(0);
			return;
			}
		
	store = &flag;	

	len = sizeof(flag_type);

	flag.i = store_flags[0];
	if ((error = _lwrite(file_id, (char *) store, len))  == HFILE_ERROR){
		add_string_to_buf("Save: Couldn't write to file.         ");
		_lclose(file_id);
		play_sound(0);
		}
	flag.i = store_flags[1];
	if ((error = _lwrite(file_id, (char *) store, len)) == HFILE_ERROR) {
		add_string_to_buf("Save: Couldn't write to file.         ");
		_lclose(file_id);
		play_sound(0);
		return;
		}
	flag.i = store_flags[2];
	if ((error = _lwrite(file_id, (char *) store, len))  == HFILE_ERROR){
		add_string_to_buf("Save: Couldn't write to file.         ");
		_lclose(file_id);
		play_sound(0);
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
			play_sound(0); 
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
		play_sound(0); 
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
				play_sound(0); 
			return;
			}
		for (count = 0; count < store_len; count++)
			party_encryptor[count] ^= 0x6B;
		}
		
	if (party_in_scen == TRUE) {
	
	// SAVE OUT DATA
	len = sizeof(out_info_type);
	if ((error = FSWrite(file_id, &len, (char *) out_e)) != 0) {
		add_string_to_buf("Save: Couldn't write to file.         ");
		FSClose(file_id);
		play_sound(0);
		return;
		}

	if (town_save == TRUE) {	
			town_ptr = &c_town;	
			len = sizeof(current_town_type);
			if ((error = FSWrite(file_id, &len, (char *) town_ptr)) != 0) {
				add_string_to_buf("Save: Couldn't write to file.         ");
				FSClose(file_id);
				play_sound(0); 
				return;
				}
			town_data_ptr = &t_d;	
			len = sizeof(big_tr_type);
			if ((error = FSWrite(file_id, &len, (char *) town_data_ptr)) != 0) {
				add_string_to_buf("Save: Couldn't write to file.         ");
				FSClose(file_id);
				play_sound(0); 
				return;
				}
			item_ptr = &t_i;	
			len = sizeof(town_item_list);
			if ((error = FSWrite(file_id, &len, (char *) item_ptr)) != 0) {
				add_string_to_buf("Save: Couldn't write to file.         ");
				FSClose(file_id);
				play_sound(0); 
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
			play_sound(0); 
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
			play_sound(0); 
			return;
			}	
		maps_ptr = &(town_maps2);
		len = (long) sizeof(stored_town_maps_type);
		if ((error = FSWrite(file_id, &len, (char *) maps_ptr))  != 0){
			add_string_to_buf("Save: Couldn't write to file.         ");
			FSClose(file_id);
			play_sound(0); 
			return;
			}	

		o_maps_ptr = &o_maps;
		len = (long) sizeof(stored_outdoor_maps_type);
		if ((error = FSWrite(file_id, &len, (char *) o_maps_ptr)) != 0) {
			add_string_to_buf("Save: Couldn't write to file.         ");
			FSClose(file_id);
			play_sound(0); 
			return;
			}
		} 
	
	// SAVE SFX and MISC_I
		len = (long) (64 * 64);
		if ((error = FSWrite(file_id, &len, (char *) sfx))  != 0){
			add_string_to_buf("Save: Couldn't write to file.         ");
			FSClose(file_id);
			play_sound(0); 
			return;
			}
		if ((error = FSWrite(file_id, &len, (char *) misc_i))  != 0){
			add_string_to_buf("Save: Couldn't write to file.         ");
			FSClose(file_id);
			play_sound(0); 
			return;
			}
	


	}

	if ((error = _lclose(file_id)) == HFILE_ERROR) {
		add_string_to_buf("Save: Couldn't close file.         ");
		play_sound(0);
		return;
		}
}


void leave_town()
{
	store_flags[0] = 5790;
}


void remove_party_from_scen()
{
	store_flags[1] = 200;
	party_in_scen = FALSE;
}



long do_waterfall(long flag)
{
	long k = 0;
		
	k = flag;
	k = k * k;
	k = k + 50;
	k = k % 3000;
	k = k * 81;
	k = k % 10000;
	k = k + 10000;
	
	return k;

}

long init_data(long flag)
{
	long k = 0;
		
	k = flag;
	k = k * k;
	k = k + 50;
	k = k % 3000;
	k = k * 81;
	k = k % 10000;
	k = k + 10000;
	
	return k;
}
void get_reg_data()
{
	HFILE f;
	short i,choice;
	long *val_store,vals[10],len = 4;
	OFSTRUCT store;

	return;
	f = OpenFile("ex3misc.dat",&store,OF_READ | OF_SEARCH);

	if (f == HFILE_ERROR) {
		build_data_file(1);
		f = OpenFile("ex3misc.dat",&store,OF_READ | OF_SEARCH);

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
		_lclose (f);
		build_data_file(1);
		return;
		}
		else {
			if (vals[9] == init_data(vals[8])) 
				ed_reg = TRUE;
			if (vals[7] == init_data(vals[6])) 
				registered = TRUE;
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

	return;
	f = OpenFile("ex3misc.dat",&store,OF_READWRITE | OF_SEARCH);
	if (f == HFILE_ERROR)
		f = OpenFile("ex3misc.dat",&store,OF_WRITE | OF_CREATE | OF_SEARCH);
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
			val_store = (long) (get_ran(1,5000,25000));
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

void reg_alert()
{
	MessageBox(mainPtr,"Cannot create/read file misc.dat! This disk may be locked. Exile III can be still be played.",
	  "File Error",MB_OK | MB_ICONEXCLAMATION);
	MessageBox(mainPtr,"If you keep getting this error, make sure Exile III is not on a CD or other locked disk.",
	  "File Error",MB_OK | MB_ICONEXCLAMATION);
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