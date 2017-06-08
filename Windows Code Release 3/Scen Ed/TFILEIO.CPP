#include <Windows.h>
#include <commdlg.h>
#include "string.h"
#include "Global.h"
#include "stdio.h"
#include "tfileio.h"
#include "keydlgs.h"
#include "graphics.h"
#include "scenario.h"
#include "edsound.h"
#include "graphutl.h"

#define	DONE_BUTTON_ITEM	1
#define	NIL		0L

extern scenario_data_type far scenario;
extern HDC main_dc;

ave_tr_type ave_t;
tiny_tr_type tiny_t;
	scenario_data_type *temp_scenario;

HWND	the_dialog;
extern HWND mainPtr;
extern town_record_type far town;
extern big_tr_type far t_d;
extern short town_type, max_dim[3];  // 0 - big 1 - ave 2 - small
extern short cur_town,overall_mode,given_password,user_given_password;
extern location cur_out;
extern piles_of_stuff_dumping_type *data_store;
extern talking_record_type far talking;
extern outdoor_record_type far current_terrain;
extern unsigned char borders[4][50];
extern Boolean change_made;
extern scen_item_data_type far scen_item_list;
extern char far scen_strs[160][256];
extern char far scen_strs2[110][256];
extern char far talk_strs[170][256];
extern char far town_strs[180][256];
extern HBITMAP spec_scen_g;
extern char current_string[256] ;
short specials_res_id,data_dump_file_id;
char start_name[256];
short start_volume,data_volume,jl = 0;
long start_dir,data_dir;

Boolean cur_scen_is_win = TRUE;
talking_record_type *dummy_talk_ptr;			
town_record_type *dummy_town_ptr;

// Big waste!
char last_load_file[63] = "newscen.exs";
char szFileName [128] = "newscen.exs";
char szFileName2 [128] = "tempscen.exs";
char szTitleName [128] = "newscen.exs";
OPENFILENAME ofn;
	OFSTRUCT store;
Boolean suppress_load_file_name = FALSE;

void print_write_position ();

void file_initialize()
{

static char *szFilter[] = {"Blades of Exile Scenarios (*.EXS)","*.exs",
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

// Here we go. this is going to hurt.
// Note no save as is available for scenarios.
//At this point, szFileName MUST contain the name for the currently edited scen.
// Strategy ... assemble a big Dummy file containing the whole scenario 
//chunk by chunk, copy the dummy over the original, and delete the dummy
// the whole scenario is too big be be shifted around at once
void save_scenario()
{
	short i,j,k,num_outdoors;
	HFILE dummy_f,scen_f;
	char *buffer = NULL;
	DWORD buf_len = 100005;
	short error;
	short out_num;
	long len,scen_ptr_move = 0,save_town_size = 0,save_out_size = 0;
	outdoor_record_type *dummy_out_ptr;
	HGLOBAL temp_buffer;
	long total_file_size = 0;

	if (check_p(user_given_password) == FALSE) {
		fancy_choice_dialog(868,0);
		return;
		}
	user_given_password = given_password;

	//OK. FIrst find out what file name we're working with, and make the dummy file
	// which we'll build the new scenario in
	if (-1 == (dummy_f = _lopen(szFileName2, OF_WRITE | OF_SHARE_EXCLUSIVE)))
		if (-1 == (dummy_f = _lcreat(szFileName2,0))) {
			oops_error(11);
			return;
			}

	if (HFILE_ERROR == (scen_f = _lopen(szFileName, OF_READ | OF_SHARE_EXCLUSIVE))) {
		oops_error(12);
		return;
		}

	// Now we need to set up a buffer for moving the data over to the dummy
	temp_buffer = GlobalAlloc(GMEM_FIXED,buf_len);
	if (temp_buffer == NULL) {
		_lclose(scen_f); _lclose(dummy_f); oops_error(14);
		return;
		}
	buffer = GlobalLock(temp_buffer);
	if (buffer == NULL) {
		_lclose(scen_f); _lclose(dummy_f); oops_error(14);
		return;
		}
	scenario.prog_make_ver[0] = 1;
	scenario.prog_make_ver[1] = 0;
	scenario.prog_make_ver[2] = 0;

	// Now, the pointer in scen_f needs to move along, so that the correct towns are sucked in.
	// To do so, we'll remember the size of the saved town and out now.
	out_num = cur_out.y * scenario.out_width + cur_out.x;
	save_out_size = (long) (scenario.out_data_size[out_num][0]) + (long) (scenario.out_data_size[out_num][1]);
	save_town_size = (long) (scenario.town_data_size[cur_town][0]) + (long) (scenario.town_data_size[cur_town][1])
					+ (long) (scenario.town_data_size[cur_town][2]) + (long) (scenario.town_data_size[cur_town][3])
					+ (long) (scenario.town_data_size[cur_town][4]);
	scen_ptr_move = sizeof(scenario_data_type);
	scen_ptr_move += sizeof(scen_item_data_type);
	for (i = 0; i < 270; i++)  // scenario strings
		scen_ptr_move += scenario.scen_str_len[i];



	// We're finally set up. Let's first set up the new scenario field
	// We need the new info for the current town and outdoors, which may have been changed
	scenario.town_data_size[cur_town][0] = sizeof(town_record_type);
	if (scenario.town_size[cur_town] == 0) 
		scenario.town_data_size[cur_town][0] += sizeof(big_tr_type);
		else if (scenario.town_size[cur_town] == 1) 
			scenario.town_data_size[cur_town][0] += sizeof(ave_tr_type);
			else scenario.town_data_size[cur_town][0] += sizeof(tiny_tr_type);
	scenario.town_data_size[cur_town][1] = 0;
	for (i = 0; i < 60; i++)
		scenario.town_data_size[cur_town][1] += strlen(town_strs[i]);
	scenario.town_data_size[cur_town][2] = 0;
	for (i = 60; i < 140; i++)
		scenario.town_data_size[cur_town][2] += strlen(town_strs[i]);
	scenario.town_data_size[cur_town][3] = sizeof(talking_record_type);
	for (i = 0; i < 80; i++)
		scenario.town_data_size[cur_town][3] += strlen(talk_strs[i]);
	scenario.town_data_size[cur_town][4] = 0;
	for (i = 80; i < 170; i++)
		scenario.town_data_size[cur_town][4] += strlen(talk_strs[i]);

	scenario.out_data_size[out_num][0] = sizeof(outdoor_record_type);
	scenario.out_data_size[out_num][1] = 0;
	for (i = 0; i < 120; i++)
		scenario.out_data_size[out_num][1] += strlen(data_store->out_strs[i]);
		
	for (i = 0; i < 300; i++) 
		scenario.scen_str_len[i] = 0;
	for (i = 0; i < 160; i++)
		scenario.scen_str_len[i] = strlen(scen_strs[i]);
	for (i = 160; i < 270; i++)
		scenario.scen_str_len[i] = strlen(scen_strs2[i - 160]);
	scenario.last_town_edited = cur_town;
	scenario.last_out_edited = cur_out;
	
	// now write scenario data
	scenario.flag1 = 20;
	scenario.flag2 = 40;
	scenario.flag3 = 60;
	scenario.flag4 = 80; /// these mean made on PC
	
	// now flags
	scenario.flag_a = sizeof(scenario_data_type) + get_ran(1,-1000,1000);
	scenario.flag_b = town_s(user_given_password);
	scenario.flag_c = out_s(user_given_password);
	scenario.flag_e = str_size_1(user_given_password);
	scenario.flag_f = str_size_2(user_given_password);
	scenario.flag_h = str_size_3(user_given_password);
	scenario.flag_g = 10000 + get_ran(1,0,5000);
	scenario.flag_d = init_data(user_given_password);
	

	len = sizeof(scenario_data_type); // scenario data
	if ((error = FSWrite(dummy_f, &len, (char *) &scenario)) != 0) {
		SysBeep(2); _lclose(scen_f); _lclose(dummy_f);oops_error(15);
		return;
		}
		
	len = sizeof(scen_item_data_type); // item data
	if ((error = FSWrite(dummy_f, &len, (char *) &(scen_item_list))) != 0) {
		SysBeep(2); _lclose(scen_f); _lclose(dummy_f);oops_error(16);
		return;
		}	
	for (i = 0; i < 270; i++) { // scenario strings
		len = (long) scenario.scen_str_len[i];
		if ( i < 160) {
			if ((error = FSWrite(dummy_f, &len, (char *) &(scen_strs[i]))) != 0) {
				SysBeep(2); _lclose(scen_f); _lclose(dummy_f);oops_error(17);
				return;
				}		
			}
			else {
				if ((error = FSWrite(dummy_f, &len, (char *) &(scen_strs2[i - 160]))) != 0) {
					SysBeep(2); _lclose(scen_f); _lclose(dummy_f);oops_error(17);
					return;
					}		
				}	
		}
	
	SetFPos(scen_f,1,scen_ptr_move);
	
	// OK ... scenario written. Now outdoors.
	num_outdoors = scenario.out_width * scenario.out_height;
	for (i = 0; i < num_outdoors; i++)
		if (i == out_num) {
			// write outdoors
			for (j = 0; j < 180; j++)
				current_terrain.strlens[j] = 0;
			for (j = 0; j < 120; j++)
				current_terrain.strlens[j] = strlen(data_store->out_strs[j]);
			len = sizeof(outdoor_record_type);
			error = FSWrite(dummy_f, &len, (char *) &current_terrain);
			if (error != 0) {_lclose(scen_f); _lclose(dummy_f);oops_error(18);}
			
			for (j = 0; j < 120; j++) {
				len = (long) current_terrain.strlens[j];
				FSWrite(dummy_f, &len, (char *) &(data_store->out_strs[j]));
				}

			SetFPos(scen_f,3,save_out_size);
			}
			else {
				len = (long) (scenario.out_data_size[i][0]) + (long) (scenario.out_data_size[i][1]);
				error = FSRead(scen_f, &len, buffer);
				dummy_out_ptr = (outdoor_record_type *) buffer;
				port_out(dummy_out_ptr);
				if (error != 0) {_lclose(scen_f); _lclose(dummy_f);oops_error(19);}
				if ((error = FSWrite(dummy_f, &len, buffer)) != 0) {
					SysBeep(2); _lclose(scen_f); _lclose(dummy_f);oops_error(20);
					return;
					}			
				}
				
	// now, finally, write towns.
	for (k = 0; k < scenario.num_towns; k++)
		if (k == cur_town) {
			for (i = 0; i < 180; i++)
				town.strlens[i] = 0;
			for (i = 0; i < 140; i++)
				town.strlens[i] = strlen(town_strs[i]);
			
			// write towns
			len = sizeof(town_record_type);
			error = FSWrite(dummy_f, &len, (char *) &town); 
			if (error != 0) {_lclose(scen_f); _lclose(dummy_f);oops_error(21);}

			switch (scenario.town_size[cur_town]) {
				case 0:
					len = sizeof(big_tr_type);
					FSWrite(dummy_f, &len, (char *) &t_d);
					break;
			
				case 1:
					for (i = 0; i < 48; i++)
						for (j = 0; j < 48; j++) {
							ave_t.terrain[i][j] = t_d.terrain[i][j];
							ave_t.lighting[i / 8][j] = t_d.lighting[i / 8][j];					
							}					
					for (i = 0; i < 16; i++) {
						ave_t.room_rect[i] = t_d.room_rect[i];
						}
					for (i = 0; i < 40; i++) {
						ave_t.creatures[i] = t_d.creatures[i];
						}
					len = sizeof(ave_tr_type);
					FSWrite(dummy_f, &len, (char *) &ave_t);
				break;
			
			
				case 2:
					for (i = 0; i < 32; i++)
						for (j = 0; j < 32; j++) {
							tiny_t.terrain[i][j] = t_d.terrain[i][j];
							tiny_t.lighting[i / 8][j] = t_d.lighting[i / 8][j];					
							}
					for (i = 0; i < 16; i++) {
						tiny_t.room_rect[i] = t_d.room_rect[i];
						}
					for (i = 0; i < 30; i++) {
						tiny_t.creatures[i] = t_d.creatures[i];
						}
					len = sizeof(tiny_tr_type);
					FSWrite(dummy_f, &len, (char *) &tiny_t);
					break;
				}
			for (j = 0; j < 140; j++) {
				len = (long) town.strlens[j];
				FSWrite(dummy_f, &len, (char *) &(town_strs[j]));
				}

			for (i = 0; i < 200; i++)
				talking.strlens[i] = 0;
			for (i = 0; i < 170; i++)
				talking.strlens[i] = strlen(talk_strs[i]);
			len = sizeof(talking_record_type);
			error = FSWrite(dummy_f, &len, (char *) &talking); 
			if (error != 0) {_lclose(scen_f); _lclose(dummy_f);oops_error(22);}
			for (j = 0; j < 170; j++) {
				len = (long) talking.strlens[j];
				FSWrite(dummy_f, &len, (char *) &(talk_strs[j]));
				}
			
			SetFPos(scen_f,3,save_town_size);
			}
			else { /// load unedited town into buffer and save, doing translataions when necessary

				len = (long) (sizeof(town_record_type));
				error = FSRead(scen_f, &len, buffer);
				if (error != 0) {_lclose(scen_f); _lclose(dummy_f);oops_error(24);}
				dummy_town_ptr = (town_record_type *) buffer;
				port_dummy_town();
				if ((error = FSWrite(dummy_f, &len, buffer)) != 0) {_lclose(scen_f); _lclose(dummy_f);oops_error(23);return;}						
		
				if (scenario.town_size[k] == 0) 
					len = (long) ( sizeof(big_tr_type));
					else if (scenario.town_size[k] == 1) 
						len = (long) ( sizeof(ave_tr_type));
						else len = (long) ( sizeof(tiny_tr_type));
				error = FSRead(scen_f, &len, buffer);
				if (error != 0) {_lclose(scen_f); _lclose(dummy_f);oops_error(24);}
				port_dummy_t_d(scenario.town_size[k],buffer);
				if ((error = FSWrite(dummy_f, &len, buffer)) != 0) {_lclose(scen_f); _lclose(dummy_f);oops_error(23);return;}						
				
				len = (long) (scenario.town_data_size[k][1])
					+ (long) (scenario.town_data_size[k][2]);
				error = FSRead(scen_f, &len, buffer);
				if (error != 0) {_lclose(scen_f); _lclose(dummy_f);oops_error(24);}
				if ((error = FSWrite(dummy_f, &len, buffer)) != 0) {_lclose(scen_f); _lclose(dummy_f);oops_error(23);return;}						
			
				len = (long) (scenario.town_data_size[k][3]);
				error = FSRead(scen_f, &len, buffer);
				if (error != 0) {_lclose(scen_f); _lclose(dummy_f);oops_error(24);}
				dummy_talk_ptr = (talking_record_type *) buffer;
				port_dummy_talk_nodes();
				if ((error = FSWrite(dummy_f, &len, buffer)) != 0) {_lclose(scen_f); _lclose(dummy_f);oops_error(23);return;}						
				len = (long) (scenario.town_data_size[k][4]);
				error = FSRead(scen_f, &len, buffer);
				if (error != 0) {_lclose(scen_f); _lclose(dummy_f);oops_error(24);}
				if ((error = FSWrite(dummy_f, &len, buffer)) != 0) {_lclose(scen_f); _lclose(dummy_f);oops_error(23);return;}
				}
	
	change_made = FALSE;


	// now, everything is moved over. Delete the original, and move everything from the
	// dummy into a new original
	error = _lclose(scen_f); 
	if (error != 0) {_lclose(scen_f); _lclose(dummy_f);oops_error(25);}
	cur_scen_is_win = TRUE;
	
	if (OpenFile(szFileName,&store,OF_DELETE) == HFILE_ERROR) {
		oops_error(101);
		return;
		}
	if (-1 == (scen_f = _lcreat(szFileName,0))) {
		oops_error(100);
		return;
		}
	total_file_size = _llseek(dummy_f,0,2);
	SetFPos(dummy_f,1,0);

	while (total_file_size > 0) {
		if (total_file_size >= 60000)
			len = 60000;
			else len = total_file_size;
		error = FSRead(dummy_f, &len, buffer);
		if (error != 0) {_lclose(scen_f); _lclose(dummy_f);oops_error(102);}
		if ((error = FSWrite(scen_f, &len, buffer)) != 0) {_lclose(scen_f); _lclose(dummy_f);oops_error(103);return;}
		total_file_size -= len;
		}
	error = _lclose(dummy_f);
	error = _lclose(scen_f);
	if (error != 0) {_lclose(scen_f); _lclose(dummy_f);oops_error(26);}
	OpenFile(szFileName2,&store,OF_DELETE);
	GlobalUnlock(temp_buffer);
	GlobalFree(temp_buffer);
	
}

void load_scenario()
{
	short i,j,k,l;
	Boolean file_ok = FALSE;
	HFILE file_id;
	short error;
	long len;

	ofn.hwndOwner = mainPtr;
	ofn.lpstrFile = szFileName;
	ofn.lpstrFileTitle = szTitleName;
	ofn.Flags = 0;

	if (GetOpenFileName(&ofn) == 0)
		return;
	file_id = _lopen(szFileName,OF_READ | OF_SHARE_DENY_WRITE);
	if (file_id == HFILE_ERROR) {
		oops_error(28);
		beep();
		return;
		}

	len = (long) sizeof(scenario_data_type);
	if ((error = FSRead(file_id, &len, (char *) &scenario)) != 0){
		_lclose(file_id); oops_error(29); return;
		}
	if ((scenario.flag1 == 10) && (scenario.flag2 == 20)
	 && (scenario.flag3 == 30)
	  && (scenario.flag4 == 40)) {
		cur_scen_is_win = FALSE;
		file_ok = TRUE;
		port_scenario();
		}
	if ((scenario.flag1 == 20) && (scenario.flag2 == 40)
	 && (scenario.flag3 == 60)
	  && (scenario.flag4 == 80)) {

		cur_scen_is_win = TRUE;
		file_ok = TRUE;
		}
	 if (file_ok == FALSE) {
		_lclose(file_id);
		give_error("This is not a legitimate Blades of Exile scenario.","",0);
		return;
		}
	len = sizeof(scen_item_data_type); // item data
	if ((error = FSRead(file_id, &len, (char *) &(scen_item_list))) != 0){
		_lclose(file_id); oops_error(30); return;
		}
	port_item_list();
	for (i = 0; i < 270; i++) {
		len = (long) (scenario.scen_str_len[i]);
		if (i < 160) {
			FSRead(file_id, &len, (char *) &(scen_strs[i]));
			scen_strs[i][len] = 0;
			}
			else {
				FSRead(file_id, &len, (char *) &(scen_strs2[i - 160]));
				scen_strs2[i - 160][len] = 0;
				}
		}
	
	_lclose(file_id);
	
	// Now check password
	if (check_p(0) == FALSE) {
		user_given_password = enter_password();
		if (check_p(user_given_password) == FALSE) {
			fancy_choice_dialog(868,0);
			if (overall_mode != 61) {
				user_given_password = enter_password();
				if (check_p(user_given_password) == FALSE) 
					PostQuitMessage(0)
;
				}
				else return;
			}
		}
		else user_given_password = 0;
	//given_password = user_given_password;
	given_password = user_given_password;
	
	//store_file_reply = file_to_load;
	overall_mode = 60;
	change_made = FALSE;
	load_town(scenario.last_town_edited);
	//load_town(0);
	load_outdoors(scenario.last_out_edited,0);
	load_spec_graphics();
	augment_terrain(scenario.last_out_edited);
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
	sprintf(file_name,"%s",szFileName);
	for (i = 0; i < 256; i++) {
		if (file_name[i] == '.') {
  			file_name[i + 1] = 'b';
			file_name[i + 2] = 'm';
			file_name[i + 3] = 'p';
			i = 256;
			}
		}
	suppress_load_file_name = TRUE;
	spec_scen_g = ReadDib(file_name,main_dc);
	suppress_load_file_name = FALSE;

}


void augment_terrain(location to_create)
{
	location to_load;
	short i,j;
	
	for (i = 0 ; i < 4; i++)
		for (j = 0 ; j < 50; j++)
			borders[i][j] = 90;
			
	to_load = to_create;
	if (to_create.y > 0) {
		to_load.y--;
		load_outdoors(to_load, 1);
		}
	to_load = to_create;
	if (to_create.y < scenario.out_height - 1) {
		to_load.y++;
		load_outdoors(to_load, 3);
		}

	to_load = to_create;
	if (to_create.x < scenario.out_width - 1) {
		to_load.x++;
		load_outdoors(to_load, 2);
		}
	to_load = to_create;
	if (to_create.x > 0) {
		to_load.x--;
		load_outdoors(to_load, 4);
		}
}

//mode -> 0 - primary load  1 - add to top  2 - right  3 - bottom  4 - left
void load_outdoors(location which_out,short mode)
{
	short i,j,k,file_id;
	long len,len_to_jump,store;
	short out_sec_num;
	outdoor_record_type store_out;
	short error;
	
	if (overall_mode == 61)
		return;

	file_id = _lopen(szFileName,OF_READ | OF_SHARE_DENY_WRITE);
	if (file_id == HFILE_ERROR) {
		oops_error(31);
		beep();
		return;
		}	
	
	out_sec_num = scenario.out_width * which_out.y + which_out.x;
	
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
	if (error != 0) {_lclose(file_id);oops_error(32);}

	len = sizeof(outdoor_record_type);
	error = FSRead(file_id, &len, (char *) &store_out);
	if (error != 0) {_lclose(file_id);oops_error(33);}

	if (mode == 0) {
		current_terrain = store_out;
		port_out(&current_terrain);
		for (i = 0; i < 120; i++) {
			len = (long) (current_terrain.strlens[i]);
			FSRead(file_id, &len, (char *) &(data_store->out_strs[i]));
			data_store->out_strs[i][len] = 0;
			}

		}
	
	if (mode == 0) 	{
		cur_out = which_out;
		}
		
	if (mode == 1) 
		for (j = 0; j < 48; j++) {				
				borders[0][j] = store_out.terrain[j][47];
				}
	if (mode == 2) 
		for (j = 0; j < 48; j++) {				
				borders[1][j] = store_out.terrain[0][j];
				}
	if (mode == 3) 
		for (j = 0; j < 48; j++) {				
				borders[2][j] = store_out.terrain[j][0];
				}
	if (mode == 4) 
		for (j = 0; j < 48; j++) {				
				borders[3][j] = store_out.terrain[47][j];
				}
				

	error = _lclose(file_id);
	if (error != 0) {_lclose(file_id);oops_error(33);}


}

void load_town(short which_town)
{
	short i,j,k,file_id;
	long len,len_to_jump = 0,store;
	short error;
	
	
	if (overall_mode == 61)
		return;
		
	file_id = _lopen(szFileName,OF_READ | OF_SHARE_DENY_WRITE);
	if (file_id == HFILE_ERROR) {
		oops_error(34);
		beep();
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
	if (error != 0) {_lclose(file_id);oops_error(35);}
	
	len = sizeof(town_record_type);
	
	error = FSRead(file_id, &len , (char *) &town);
	port_town();
	if (error != 0) {_lclose(file_id);oops_error(36);}

	switch (scenario.town_size[which_town]) {
		case 0:
			len =  sizeof(big_tr_type);
			FSRead(file_id, &len, (char *) &t_d);
			port_t_d();
			break;

		case 1:
				len = sizeof(ave_tr_type);
				FSRead(file_id, &len, (char *) &ave_t);
				for (i = 0; i < 48; i++)
					for (j = 0; j < 48; j++) {
						t_d.terrain[i][j] = ave_t.terrain[i][j];
						t_d.lighting[i / 8][j] = ave_t.lighting[i / 8][j];
						}
				for (i = 0; i < 16; i++) {
					t_d.room_rect[i] = ave_t.room_rect[i];
					}
				for (i = 0; i < 40; i++) {
					t_d.creatures[i] = ave_t.creatures[i];
					}
				port_t_d();
				for (i = 40; i < 60; i++) {
					t_d.creatures[i].number = 0;
					}
			break;

		case 2:
			len = sizeof(tiny_tr_type);
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
			port_t_d();
			for (i = 30; i < 60; i++) {
				t_d.creatures[i].number = 0;
				}
			break;
		}


	for (i = 0; i < 140; i++) {
		len = (long) (town.strlens[i]);
		FSRead(file_id, &len, (char *) &(town_strs[i]));
		town_strs[i][len] = 0;
		}

	len = sizeof(talking_record_type);
	error = FSRead(file_id, &len , (char *) &talking);
	if (error != 0) {_lclose(file_id);oops_error(37);}
	port_talk_nodes();
	
	for (i = 0; i < 170; i++) {
		len = (long) (talking.strlens[i]);
		FSRead(file_id, &len, (char *) &(talk_strs[i]));
		talk_strs[i][len] = 0;
		}
			
	town_type = scenario.town_size[which_town];
	cur_town = which_town;
	error = _lclose(file_id);
	if (error != 0) {_lclose(file_id);oops_error(38);}
}

// if which_town is -1, load town from base
void import_town(short which_town)
{
	short i,j,k,l;
	HFILE file_id;
	Boolean file_ok = FALSE;
	short error;
	long len,len_to_jump = 0,store;
	DWORD buf_len = 100000;
	char *buffer = NULL;
	short import_user_given_password;
	char szFileName3 [128] = "scen.exs";
	HGLOBAL temp_buffer;
	
	if (which_town >= 0) {
		ofn.hwndOwner = mainPtr;
		ofn.lpstrFile = szFileName3;
		ofn.lpstrFileTitle = szTitleName;
		ofn.Flags = 0;

		if (GetOpenFileName(&ofn) == 0)
			return;
		}
		else {
			sprintf((char *) szFileName3,"BLADBASE.EXS");
			which_town = 0;
			}

	file_id = _lopen(szFileName3,OF_READ | OF_SEARCH | OF_SHARE_DENY_WRITE);
	if (file_id == HFILE_ERROR) {
		oops_error(42); beep();
		return;
		}
	
	temp_buffer = GlobalAlloc(GMEM_FIXED,buf_len);
	if (temp_buffer == NULL) {
		_lclose(file_id); oops_error(41);
		return;
		}
	buffer = GlobalLock(temp_buffer);
	if (buffer == NULL) {
		_lclose(file_id); oops_error(41);
		return;
		}

	len = (long) sizeof(scenario_data_type);
	if ((error = FSRead(file_id, &len, buffer)) != 0){
		_lclose(file_id); oops_error(43); return;
		}
	temp_scenario = (scenario_data_type *) buffer;
	
	if (temp_scenario->town_size[which_town] != scenario.town_size[cur_town]) {
		give_error("Oops ... the town in the selected scenario and the current town are different sizes. Import failed.","",0);
		GlobalUnlock(temp_buffer); GlobalFree(temp_buffer); _lclose(file_id);
		return;
		}
	if (which_town >= temp_scenario->num_towns) {
		give_error("Oops ... the selected scenario doesn't have enough towns. The town you selected doesn't exist inside this scenario.","",0);
		GlobalUnlock(temp_buffer); GlobalFree(temp_buffer); _lclose(file_id); 
		return;
		}
	
	if ((temp_scenario->flag1 == 20) && (temp_scenario->flag2 == 40)
	 && (temp_scenario->flag3 == 60)
	  && (temp_scenario->flag4 == 80)) {
	  	file_ok = TRUE;
	  	}
	 if (file_ok == FALSE) {
		GlobalUnlock(temp_buffer); GlobalFree(temp_buffer); _lclose(file_id); give_error("This is not a legitimate Blades of Exile scenario. If it is a scenario, note that it needs to have been saved by the Windows scenario editor.","",0); return;	 
	 	}

	
	// Now check password
	if (import_check_p(0) == FALSE) {
		import_user_given_password = enter_password();
		if (import_check_p(import_user_given_password) == FALSE) {
			fancy_choice_dialog(868,0);
			if (overall_mode != 61) {
				import_user_given_password = enter_password();
				if (import_check_p(import_user_given_password) == FALSE) 
					PostQuitMessage(0)
;
				}
				else return;
			}
		}
		
	len_to_jump = sizeof(scenario_data_type);
	len_to_jump += sizeof(scen_item_data_type);
	for (i = 0; i < 300; i++)
		len_to_jump += (long) temp_scenario->scen_str_len[i];
	store = 0;
	for (i = 0; i < 100; i++)
		for (j = 0; j < 2; j++)
			store += (long) (temp_scenario->out_data_size[i][j]);
	for (i = 0; i < which_town; i++)
		for (j = 0; j < 5; j++)
			store += (long) (temp_scenario->town_data_size[i][j]);
	len_to_jump += store;
	error = SetFPos (file_id, 1, len_to_jump);
	if (error != 0) {_lclose(file_id);oops_error(44);}
	
	len = sizeof(town_record_type);
	error = FSRead(file_id, &len , (char *) &town);
	if (error != 0) {_lclose(file_id);oops_error(45);}

	switch (temp_scenario->town_size[which_town]) {
		case 0:
			len =  sizeof(big_tr_type);
			FSRead(file_id, &len, (char *) &t_d);
			break;
			
		case 1:
				len = sizeof(ave_tr_type);
				FSRead(file_id, &len, (char *) &ave_t);
				for (i = 0; i < 48; i++)
					for (j = 0; j < 48; j++) {
						t_d.terrain[i][j] = ave_t.terrain[i][j];
						t_d.lighting[i / 8][j] = ave_t.lighting[i / 8][j];					
						}
				for (i = 0; i < 16; i++) {
					t_d.room_rect[i] = ave_t.room_rect[i];
					}
				for (i = 0; i < 40; i++) {
					t_d.creatures[i] = ave_t.creatures[i];
					}
				for (i = 40; i < 60; i++) {
					t_d.creatures[i].number = 0;
					}
			break;
			
		case 2:
			len = sizeof(tiny_tr_type);
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
			break;
		}

	for (i = 0; i < 140; i++) {
		len = (long) (town.strlens[i]);
		FSRead(file_id, &len, (char *) &(town_strs[i]));
		town_strs[i][len] = 0;
		}

	len = sizeof(talking_record_type);
	error = FSRead(file_id, &len , (char *) &talking);
	if (error != 0) {_lclose(file_id);oops_error(46);}
	
	for (i = 0; i < 170; i++) {
		len = (long) (talking.strlens[i]);
		FSRead(file_id, &len, (char *) &(talk_strs[i]));
		talk_strs[i][len] = 0;
		}

	error = _lclose(file_id);
	if (error != 0) {_lclose(file_id);oops_error(47);}
	GlobalUnlock(temp_buffer);
	GlobalFree(temp_buffer);	
}

// When this is called, the current town is the town to make town 0.
void make_new_scenario(char *file_name,short out_width,short out_height,short making_warriors_grove,
	short use_grass,char *title)
{
	short i,j,k,num_outdoors;
	HFILE dummy_f,file_id;
	DWORD buf_len = 100000;
	short error;
	short out_num;
	long len,scen_ptr_move = 0,save_town_size = 0,save_out_size = 0;
	location loc;
	short x,y;
	HGLOBAL temp_buffer;
	char *buffer;

	// Step 1 - load scenario file from scenario base. It contains all the monsters and
	// items done up properly!
	if (HFILE_ERROR == (file_id = _lopen("BLADBASE.EXS", OF_READ | OF_SEARCH | OF_SHARE_EXCLUSIVE))) {
		//give_error("Could not find the file 'Blades of Exile Base', which needs to be in the Scenario Editor folder. If you no longer have it, you need to redownload Blades of Exile.","",0);
			give_error("The Blades Editor is having trouble finding the file 'bladbase.exs'. It needs to be in the BLSCENED folder.",
				"If it is in that folder, select Open from the file menu, open 'bladbase.exs', and then try making a new scenario again.",0);
			file_name[0] = 0;
			return;
			}
	strcpy((char *) szFileName,file_name);

	temp_buffer = GlobalAlloc(GMEM_FIXED,buf_len);
	if (temp_buffer == NULL) {
		_lclose(file_id); oops_error(114);
		return;
		}
	buffer = GlobalLock(temp_buffer);
	if (buffer == NULL) {
		_lclose(file_id); oops_error(114);
		return;
		}
	
	len = (long) sizeof(scenario_data_type);
	if ((error = FSRead(file_id, &len, (char *) &scenario)) != 0){
		_lclose(file_id); oops_error(82); return;
		}

	len = sizeof(scen_item_data_type); // item data
	if ((error = FSRead(file_id, &len, (char *) &(scen_item_list))) != 0){
		_lclose(file_id); oops_error(83); return;
		}
	for (i = 0; i < 270; i++) {
		len = (long) (scenario.scen_str_len[i]);
		if (i < 160) {
			FSRead(file_id, &len, (char *) &(scen_strs[i]));
			scen_strs[i][len] = 0;
			}
			else {
				FSRead(file_id, &len, (char *) &(scen_strs2[i - 160]));
				scen_strs2[i - 160][len] = 0;
				}
		}
	strcpy((char *) scen_strs[0],(char *) title);
	scenario.scen_str_len[0] = strlen(title);
	scen_strs[0][scenario.scen_str_len[0]] = 0;
	_lclose(file_id);

	//OK. FIrst find out what file name we're working with, and make the dummy file
	// which we'll build the new scenario in   Blades of Exile Base
	if (HFILE_ERROR == (dummy_f = _lcreat(szFileName, 0))) {
			oops_error(111);
			return;
			}

		
	scenario.prog_make_ver[0] = 1;
	scenario.prog_make_ver[1] = 0;
	scenario.prog_make_ver[2] = 0;
	cur_town = 0;
	town_type = 1;
	scenario.num_towns = 1;
	scenario.town_size[0] = 1;
	scenario.out_width = out_width;
	scenario.out_height = out_height;	
	cur_out.x = 0;
	cur_out.y = 0;
	scenario.last_out_edited = cur_out;

	// We're finally set up. Let's first set up the new scenario field
	// We need the new info for the current town and outdoors, which may have been changed
	scenario.town_data_size[cur_town][0] = sizeof(town_record_type);
	if (scenario.town_size[cur_town] == 0) 
		scenario.town_data_size[cur_town][0] += sizeof(big_tr_type);
		else if (scenario.town_size[cur_town] == 1) 
			scenario.town_data_size[cur_town][0] += sizeof(ave_tr_type);
			else scenario.town_data_size[cur_town][0] += sizeof(tiny_tr_type);
	scenario.town_data_size[cur_town][1] = 0;
	for (i = 0; i < 60; i++)
		scenario.town_data_size[cur_town][1] += strlen(town_strs[i]);
	scenario.town_data_size[cur_town][2] = 0;
	for (i = 60; i < 140; i++)
		scenario.town_data_size[cur_town][2] += strlen(town_strs[i]);
	scenario.town_data_size[cur_town][3] = sizeof(talking_record_type);
	for (i = 0; i < 80; i++)
		scenario.town_data_size[cur_town][3] += strlen(talk_strs[i]);
	scenario.town_data_size[cur_town][4] = 0;
	for (i = 80; i < 170; i++)
		scenario.town_data_size[cur_town][4] += strlen(talk_strs[i]);

	num_outdoors = scenario.out_width * scenario.out_height;
	for (i = 0; i < num_outdoors; i++) {
		scenario.out_data_size[i][0] = sizeof(outdoor_record_type);
		scenario.out_data_size[i][1] = 0;
		for (j = 0; j < 120; j++)
			scenario.out_data_size[i][1] += strlen(data_store->out_strs[j]);
		}
		
	for (i = 0; i < 300; i++) 
		scenario.scen_str_len[i] = 0;
	for (i = 0; i < 160; i++)
		scenario.scen_str_len[i] = strlen(scen_strs[i]);
	for (i = 160; i < 270; i++)
		scenario.scen_str_len[i] = strlen(scen_strs2[i - 160]);
	scenario.last_town_edited = cur_town;
	scenario.last_out_edited = cur_out;
	
	// now write scenario data
	scenario.flag1 = 20;
	scenario.flag2 = 40;
	scenario.flag3 = 60;
	scenario.flag4 = 80; /// these mean made on mac
	// now flags
	scenario.flag_a = sizeof(scenario_data_type) + get_ran(1,-1000,1000);
	scenario.flag_b = town_s(user_given_password);
	scenario.flag_c = out_s(user_given_password);
	scenario.flag_e = str_size_1(user_given_password);
	scenario.flag_f = str_size_2(user_given_password);
	scenario.flag_h = str_size_3(user_given_password);
	scenario.flag_g = 10000 + get_ran(1,0,5000);
	scenario.flag_d = init_data(user_given_password);
	
	len = sizeof(scenario_data_type); // scenario data
	scen_ptr_move += len;
	if ((error = FSWrite(dummy_f, &len, (char *) &scenario)) != 0) {
		_lclose(dummy_f);
				oops_error(3);
		return;
		}	
	len = sizeof(scen_item_data_type); // item data
	scen_ptr_move += len;
	if ((error = FSWrite(dummy_f, &len, (char *) &(scen_item_list))) != 0) {
 		_lclose(dummy_f);
		oops_error(4);
		return;
		}	
	for (i = 0; i < 270; i++) { // scenario strings
		len = (long) scenario.scen_str_len[i];
		scen_ptr_move += len;
		if (i < 160) {
			if ((error = FSWrite(dummy_f, &len, (char *) &(scen_strs[i]))) != 0) {
				_lclose(dummy_f);oops_error(5);
				return;
				}		
			}	
			else {
				if ((error = FSWrite(dummy_f, &len, (char *) &(scen_strs2[i - 160]))) != 0) {
					_lclose(dummy_f);oops_error(5);
					return;
					}		
				}
		}
	
	
	// OK ... scenario written. Now outdoors.
	num_outdoors = scenario.out_width * scenario.out_height;
	for (i = 0; i < num_outdoors; i++)
		{
			loc.x = i % scenario.out_width;
			loc.y = i / scenario.out_width;
			
			for (x = 0; x < 48; x++)
				for (y = 0; y < 48; y++) {
					current_terrain.terrain[x][y] = (use_grass > 0) ? 2 : 0;
					
					if ((x < 4) && (loc.x == 0))
						current_terrain.terrain[x][y] = (use_grass > 0) ? 22 : 5;
					if ((y < 4) && (loc.y == 0))
						current_terrain.terrain[x][y] = (use_grass > 0) ? 22 : 5;
					if ((x > 43) && (loc.x == scenario.out_width - 1))
						current_terrain.terrain[x][y] = (use_grass > 0) ? 22 : 5;
					if ((y > 43) && (loc.y == scenario.out_height - 1))
						current_terrain.terrain[x][y] = (use_grass > 0) ? 22 : 5;
					if ((i == 0) && (making_warriors_grove > 0)) {
						current_terrain.terrain[24][24] = (use_grass > 0) ? 234 : 232;
						current_terrain.exit_locs[0].x = 24;
						current_terrain.exit_locs[0].y = 24;
						current_terrain.exit_dests[0] = 0;
						}
						else current_terrain.exit_locs[0].x = 100;
					}
					
			// write outdoors
			for (j = 0; j < 180; j++)
				current_terrain.strlens[j] = 0;
			for (j = 0; j < 120; j++)
				current_terrain.strlens[j] = strlen(data_store->out_strs[j]);
			len = sizeof(outdoor_record_type);
			error = FSWrite(dummy_f, &len, (char *) &current_terrain); 
			if (error != 0) {_lclose(dummy_f);oops_error(6);}

			for (j = 0; j < 120; j++) {
				len = (long) current_terrain.strlens[j];
				error = FSWrite(dummy_f, &len, (char *) &(data_store->out_strs[j]));
				if (error != 0) {_lclose(dummy_f);oops_error(7);}
				}
			
			}


	// now, finally, write towns.
	for (k = 0; k < scenario.num_towns; k++)
		{
			for (i = 0; i < 180; i++)
				town.strlens[i] = 0;
			for (i = 0; i < 140; i++)
				town.strlens[i] = strlen(town_strs[i]);
			
			// write towns
			len = sizeof(town_record_type);
			error = FSWrite(dummy_f, &len, (char *) &town); 
			if (error != 0) {_lclose(dummy_f);oops_error(8);}

			switch (scenario.town_size[cur_town]) {
				case 0:
					len = sizeof(big_tr_type);
					FSWrite(dummy_f, &len, (char *) &t_d);
					break;
			
				case 1:
					for (i = 0; i < 48; i++)
						for (j = 0; j < 48; j++) {
							ave_t.terrain[i][j] = t_d.terrain[i][j];
							ave_t.lighting[i / 8][j] = t_d.lighting[i / 8][j];					
							}					
					for (i = 0; i < 16; i++) {
						ave_t.room_rect[i] = t_d.room_rect[i];
						}
					for (i = 0; i < 40; i++) {
						ave_t.creatures[i] = t_d.creatures[i];
						}
					len = sizeof(ave_tr_type);
					FSWrite(dummy_f, &len, (char *) &ave_t);
				break;
			
			
				case 2:
					for (i = 0; i < 32; i++)
						for (j = 0; j < 32; j++) {
							tiny_t.terrain[i][j] = t_d.terrain[i][j];
							tiny_t.lighting[i / 8][j] = t_d.lighting[i / 8][j];					
							}
					for (i = 0; i < 16; i++) {
						tiny_t.room_rect[i] = t_d.room_rect[i];
						}
					for (i = 0; i < 30; i++) {
						tiny_t.creatures[i] = t_d.creatures[i];
						}
					len = sizeof(tiny_tr_type);
					FSWrite(dummy_f, &len, (char *) &tiny_t);
					break;
				}
			for (j = 0; j < 140; j++) {
				len = (long) town.strlens[j];
				FSWrite(dummy_f, &len, (char *) &(town_strs[j]));
				}

			for (i = 0; i < 200; i++)
				talking.strlens[i] = 0;
			for (i = 0; i < 170; i++)
				talking.strlens[i] = strlen(talk_strs[i]);
			len = sizeof(talking_record_type);
			error = FSWrite(dummy_f, &len, (char *) &talking); 
			if (error != 0) {_lclose(dummy_f);oops_error(9);}
			for (j = 0; j < 170; j++) {
				len = (long) talking.strlens[j];
				FSWrite(dummy_f, &len, (char *) &(talk_strs[j]));
				}
			
			}

	
	change_made = TRUE;
	// now, everything is moved over. Delete the original, and rename the dummy
	error = _lclose(dummy_f);		
	if (error != 0) {_lclose(dummy_f);oops_error(10);}

	GlobalUnlock(temp_buffer);
	GlobalFree(temp_buffer);
}

void oops_error(short error)
{
	char error_str[256];
	
		SysBeep(50);
		SysBeep(50);
		SysBeep(50);
	sprintf((char *) error_str,"You may need more memory ... run the editor with no other programs running. Be sure to back your scenario up often. Error number: %d.",error);
	give_error("The program encountered an error while loading/saving/creating the scenario. To prevent future problems, the program will now terminate. Trying again may solve the problem.",(char *) error_str,0);
	PostQuitMessage(0);
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
Boolean check_p2 (short pword)
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
}Boolean check_p3 (short pword)
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

Boolean import_check_p (short pword)
{

	if (temp_scenario->flag_d != init_data(pword))
		return FALSE;

	return TRUE;
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

void reset_pwd()
{
	// now flags
	scenario.flag_a = sizeof(scenario_data_type) + get_ran(1,-1000,1000);
	scenario.flag_b = town_s(user_given_password);
	scenario.flag_c = out_s(user_given_password);
	scenario.flag_e = str_size_1(user_given_password);
	scenario.flag_f = str_size_2(user_given_password);
	scenario.flag_h = str_size_3(user_given_password);
	scenario.flag_g = 10000 + get_ran(1,0,5000);
	scenario.flag_d = init_data(user_given_password);
}

void start_data_dump()
{
	short i;
	short j,k;
	long val_store,to_return = 0;
	short the_type;
	RECT the_rect;
	char the_string[256],store_name[256];
	char get_text[280];
	HFILE data_dump_file_id;
	short error;
	long len,empty_len;

	if (HFILE_ERROR == (data_dump_file_id = _lcreat("SCENDATA.TXT", 0))) {
			SysBeep(50);
			return;
			}

	SetFPos (data_dump_file_id, 2, 0);

	sprintf((char *)get_text,"Scenario data for %s:\n",scen_strs[0]);
	len = (long) (strlen((char *)get_text));
	FSWrite(data_dump_file_id, &len, (char *) get_text);
	sprintf((char *)get_text,"\n");
	len = (long) (strlen((char *)get_text));
	FSWrite(data_dump_file_id, &len, (char *) get_text);

	sprintf((char *)get_text,"Terrain types for %s:\n",scen_strs[0]);
	len = (long) (strlen((char *)get_text));
	FSWrite(data_dump_file_id, &len, (char *) get_text);


	for (i = 0; i < 256; i++) {
		sprintf((char *)get_text,"  Terrain type %d: %s\n",i,scen_item_list.ter_names[i]);
		len = (long) (strlen((char *)get_text));
		FSWrite(data_dump_file_id, &len, (char *) get_text);

		}

	sprintf((char *)get_text,"\n");
	len = (long) (strlen((char *)get_text));
	FSWrite(data_dump_file_id, &len, (char *) get_text);

	sprintf((char *)get_text,"Monster types for %s:\n",scen_strs[0]);
	len = (long) (strlen((char *)get_text));
	FSWrite(data_dump_file_id, &len, (char *) get_text);

	for (i = 0; i < 256; i++) {
		sprintf((char *)get_text,"  Monster type %d: %s\n",i,scen_item_list.monst_names[i]);
		len = (long) (strlen((char *)get_text));
		FSWrite(data_dump_file_id, &len, (char *) get_text);

		}

	sprintf((char *)get_text,"\n");
	len = (long) (strlen((char *)get_text));
	FSWrite(data_dump_file_id, &len, (char *) get_text);

	sprintf((char *)get_text,"Item types for %s:\n",scen_strs[0]);
	len = (long) (strlen((char *)get_text));
	FSWrite(data_dump_file_id, &len, (char *) get_text);

	for (i = 0; i < 400; i++) {
		sprintf((char *)get_text,"  Item type %d: %s\n",i,scen_item_list.scen_items[i].full_name);
		len = (long) (strlen((char *)get_text));
		FSWrite(data_dump_file_id, &len, (char *) get_text);

		}
	_lclose(data_dump_file_id);

}

void scen_text_dump()
{
	short i;
	short j,k;
	long val_store,to_return = 0;
	short the_type;
	HFILE data_dump_file_id;
	RECT the_rect;
	char the_string[256],store_name[256];
	char get_text[300];
	short error;
	long len,empty_len;
	location out_sec;
	
	if (HFILE_ERROR == (data_dump_file_id = _lcreat("SCENTEXT.TXT", 0))) {
			SysBeep(50);
			return;
			}


//	sprintf((char *)empty_line,"\r");
//	empty_len = (long) (strlen((char *)empty_line));

	SetFPos (data_dump_file_id, 2, 0);

	sprintf((char *)get_text,"Scenario text for %s:\n",scen_strs[0]);
	len = (long) (strlen((char *)get_text));
	FSWrite(data_dump_file_id, &len, (char *) get_text);
	sprintf((char *)get_text,"\n");
	len = (long) (strlen((char *)get_text));
	FSWrite(data_dump_file_id, &len, (char *) get_text);

	sprintf((char *)get_text,"Scenario Text:\n",scen_strs[0]);
	len = (long) (strlen((char *)get_text));
	FSWrite(data_dump_file_id, &len, (char *) get_text);
	sprintf((char *)get_text,"\n");
	len = (long) (strlen((char *)get_text));
	FSWrite(data_dump_file_id, &len, (char *) get_text);
	for (i = 0; i < 260; i++)
		if (((i < 160) ? scen_strs[i][0] : scen_strs2[i - 160][0]) != '*') {
			if (i < 160)
				sprintf((char *)get_text,"  Message %d: %s\n",i,scen_strs[i]);
				else sprintf((char *)get_text,"  Message %d: %s\n",i,scen_strs2[i - 160]);
			len = (long) (strlen((char *)get_text));
			FSWrite(data_dump_file_id, &len, (char *) get_text);
			}

		sprintf((char *)get_text,"\n");
		len = (long) (strlen((char *)get_text));
		FSWrite(data_dump_file_id, &len, (char *) get_text);
	sprintf((char *)get_text,"Outdoor Sections Text:\n",scen_strs[0]);
	len = (long) (strlen((char *)get_text));
	FSWrite(data_dump_file_id, &len, (char *) get_text);
	sprintf((char *)get_text,"\n");
	len = (long) (strlen((char *)get_text));
	FSWrite(data_dump_file_id, &len, (char *) get_text);
	for (out_sec.x = 0; out_sec.x < scenario.out_width ; out_sec.x++)
		for (out_sec.y = 0; out_sec.y < scenario.out_height ; out_sec.y++) {
			sprintf((char *)get_text,"  Section X = %d, Y = %d:\n",(short) out_sec.x,(short) out_sec.y);
			len = (long) (strlen((char *)get_text));
			FSWrite(data_dump_file_id, &len, (char *) get_text);
			sprintf((char *)get_text,"\n");
			len = (long) (strlen((char *)get_text));
			FSWrite(data_dump_file_id, &len, (char *) get_text);

			load_outdoors(out_sec,0);
			for (i = 0; i < 108; i++)
				if (data_store->out_strs[i][0] != '*') {
					sprintf((char *)get_text,"  Message %d: %s\n",i,data_store->out_strs[i]);
					len = (long) (strlen((char *)get_text));
					FSWrite(data_dump_file_id, &len, (char *) get_text);
					}
			sprintf((char *)get_text,"\n");
			len = (long) (strlen((char *)get_text));
			FSWrite(data_dump_file_id, &len, (char *) get_text);
		}
	augment_terrain(out_sec);

	sprintf((char *)get_text,"Town Text:\n",scen_strs[0]);
	len = (long) (strlen((char *)get_text));
	FSWrite(data_dump_file_id, &len, (char *) get_text);
	sprintf((char *)get_text,"\n");
	len = (long) (strlen((char *)get_text));
	FSWrite(data_dump_file_id, &len, (char *) get_text);
	for (j = 0; j < scenario.num_towns; j++) {
		load_town(j);

		sprintf((char *)get_text,"  Town: %s\n",town_strs[0]);
		len = (long) (strlen((char *)get_text));
		FSWrite(data_dump_file_id, &len, (char *) get_text);
		sprintf((char *)get_text,"\n");
		len = (long) (strlen((char *)get_text));
		FSWrite(data_dump_file_id, &len, (char *) get_text);
		sprintf((char *)get_text,"  Town Messages:",town_strs[j]);
		len = (long) (strlen((char *)get_text));
		FSWrite(data_dump_file_id, &len, (char *) get_text);
		sprintf((char *)get_text,"\n");
		len = (long) (strlen((char *)get_text));
		FSWrite(data_dump_file_id, &len, (char *) get_text);

			for (i = 0; i < 135; i++)
				if (town_strs[i][0] != '*') {
					sprintf((char *)get_text,"  Message %d: %s\n",i,town_strs[i]);
					len = (long) (strlen((char *)get_text));
					FSWrite(data_dump_file_id, &len, (char *) get_text);
					}

		sprintf((char *)get_text,"  Town Dialogue:",town_strs[j]);
		len = (long) (strlen((char *)get_text));
		FSWrite(data_dump_file_id, &len, (char *) get_text);
		sprintf((char *)get_text,"\n");
		len = (long) (strlen((char *)get_text));
		FSWrite(data_dump_file_id, &len, (char *) get_text);

		for (i = 0; i < 10; i++) {
			sprintf((char *)get_text,"  Personality %d name: %s\n",j * 10 + i,talk_strs[i]);
			len = (long) (strlen((char *)get_text));
			FSWrite(data_dump_file_id, &len, (char *) get_text);
			sprintf((char *)get_text,"  Personality %d look: %s\n",j * 10 + i,talk_strs[i + 10]);
			len = (long) (strlen((char *)get_text));
			FSWrite(data_dump_file_id, &len, (char *) get_text);
			sprintf((char *)get_text,"  Personality %d ask name: %s\n",j * 10 + i,talk_strs[i + 20]);
			len = (long) (strlen((char *)get_text));
			FSWrite(data_dump_file_id, &len, (char *) get_text);
			sprintf((char *)get_text,"  Personality %d ask job: %s\n",j * 10 + i,talk_strs[i + 30]);
			len = (long) (strlen((char *)get_text));
			FSWrite(data_dump_file_id, &len, (char *) get_text);
			sprintf((char *)get_text,"  Personality %d confused: %s\n",j * 10 + i,talk_strs[i + 160]);
			len = (long) (strlen((char *)get_text));
			FSWrite(data_dump_file_id, &len, (char *) get_text);
			}

			for (i = 40; i < 160; i++)
				if (strlen((char *) (talk_strs[i])) > 0) {
					sprintf((char *)get_text,"  Node %d: %s\n",(i - 40) / 2,talk_strs[i]);
					len = (long) (strlen((char *)get_text));
					FSWrite(data_dump_file_id, &len, (char *) get_text);
					}

			sprintf((char *)get_text,"\n");
			len = (long) (strlen((char *)get_text));
			FSWrite(data_dump_file_id, &len, (char *) get_text);

		}

	_lclose(data_dump_file_id);

}
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
	flip_short(&town.town_chop_time);
	flip_short(&town.town_chop_key);
	flip_short(&town.lighting);
	for (i =0 ; i < 4; i++)
		flip_short(&town.exit_specs[i]);
	flip_rect(&town.in_town_rect);
	for (i =0 ; i < 64; i++) {
		flip_short(&town.preset_items[i].item_code);
		flip_short(&town.preset_items[i].ability);
		}
	for (i =0 ; i < 50; i++) {
		flip_short(&town.preset_fields[i].field_type);
		}
	flip_short(&town.max_num_monst);
	flip_short(&town.spec_on_entry);
	flip_short(&town.spec_on_entry_if_dead);
	for (i =0 ; i < 8; i++)
		flip_short(&town.timer_spec_times[i]);
	for (i =0 ; i < 8; i++)
		flip_short(&town.timer_specs[i]);
	flip_short(&town.difficulty);
	for (i =0 ; i < 100; i++)
		flip_spec_node(&town.specials[i]);

}

void port_dummy_town()
{
	short i,j,k,l;

	if (cur_scen_is_win == TRUE)
		return;
	flip_short(&dummy_town_ptr->town_chop_time);
	flip_short(&dummy_town_ptr->town_chop_key);
	flip_short(&dummy_town_ptr->lighting);
	for (i =0 ; i < 4; i++)
		flip_short(&dummy_town_ptr->exit_specs[i]);
	flip_rect(&dummy_town_ptr->in_town_rect);
	for (i =0 ; i < 64; i++) {
		flip_short(&dummy_town_ptr->preset_items[i].item_code);
		flip_short(&dummy_town_ptr->preset_items[i].ability);
		}
	for (i =0 ; i < 50; i++) {
		flip_short(&dummy_town_ptr->preset_fields[i].field_type);
		}
	flip_short(&dummy_town_ptr->max_num_monst);
	flip_short(&dummy_town_ptr->spec_on_entry);
	flip_short(&dummy_town_ptr->spec_on_entry_if_dead);
	for (i =0 ; i < 8; i++)
		flip_short(&dummy_town_ptr->timer_spec_times[i]);
	for (i =0 ; i < 8; i++)
		flip_short(&dummy_town_ptr->timer_specs[i]);
	flip_short(&dummy_town_ptr->difficulty);
	for (i =0 ; i < 100; i++)
		flip_spec_node(&dummy_town_ptr->specials[i]);

}

void port_dummy_t_d(short size,char *buffer)
{
	short i,j,k,l;
	big_tr_type *d1;
	ave_tr_type *d2;
	tiny_tr_type *d3;

	if (cur_scen_is_win == TRUE)
		return;

	switch (size) {
		case 0:
			d1 = (big_tr_type *) buffer;
			for (i =0 ; i < 16; i++)
				flip_rect(&d1->room_rect[i]);
			for (i =0 ; i < 60; i++) {
				flip_short(&d1->creatures[i].spec1);
				flip_short(&d1->creatures[i].spec2);
				flip_short(&d1->creatures[i].monster_time);
				flip_short(&d1->creatures[i].personality);
				flip_short(&d1->creatures[i].special_on_kill);
				flip_short(&d1->creatures[i].facial_pic);
				}
			break;
		case 1:
			d2 = (ave_tr_type *) buffer;
			for (i =0 ; i < 16; i++)
				flip_rect(&d2->room_rect[i]);
			for (i =0 ; i < 40; i++) {
				flip_short(&d2->creatures[i].spec1);
				flip_short(&d2->creatures[i].spec2);
				flip_short(&d2->creatures[i].monster_time);
				flip_short(&d2->creatures[i].personality);
				flip_short(&d2->creatures[i].special_on_kill);
				flip_short(&d2->creatures[i].facial_pic);
				}
			break;
		case 2:
			d3 = (tiny_tr_type *) buffer;
			for (i =0 ; i < 16; i++)
				flip_rect(&d3->room_rect[i]);
			for (i =0 ; i < 30; i++) {
				flip_short(&d3->creatures[i].spec1);
				flip_short(&d3->creatures[i].spec2);
				flip_short(&d3->creatures[i].monster_time);
				flip_short(&d3->creatures[i].personality);
				flip_short(&d3->creatures[i].special_on_kill);
				flip_short(&d3->creatures[i].facial_pic);
				}
			break;
		}
}

void port_dummy_talk_nodes()
{
	short i,j,k,l;

	if (cur_scen_is_win == TRUE)
		return;
	for (i = 0; i < 60; i++) {
		flip_short(&dummy_talk_ptr->talk_nodes[i].personality);
		flip_short(&dummy_talk_ptr->talk_nodes[i].type);
		flip_short(&dummy_talk_ptr->talk_nodes[i].extras[0]);
		flip_short(&dummy_talk_ptr->talk_nodes[i].extras[1]);
		flip_short(&dummy_talk_ptr->talk_nodes[i].extras[2]);
		flip_short(&dummy_talk_ptr->talk_nodes[i].extras[3]);
		}
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
		flip_short(&(scen_item_list.scen_items[i].variety));
		flip_short(&(scen_item_list.scen_items[i].item_level));
		flip_short(&(scen_item_list.scen_items[i].value));
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

void alter_rect(RECT *r)
{
	short a;

	a = r->top;
	r->top = r->left;
	r->left = a;
	a = r->bottom;
	r->bottom = r->right;
	r->right = a;
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