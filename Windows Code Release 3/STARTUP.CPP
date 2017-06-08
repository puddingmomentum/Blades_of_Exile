
#include <Windows.h>
#include "stdio.h"

#include "global.h"
#include "newgraph.h"
#include "graphics.h"
#include "fileio.h"
#include "actions.h"
#include "dlgutils.h"
#include "text.h"
#include "gutils.h"
#include "items.h"
#include "party.h"
#include "exlsound.h"
#include "startup.h"
#include "exlsound.h"

extern party_record_type far	party;
extern pc_record_type far adven[6];
extern scen_header_type scen_headers[25];
extern Boolean unreg_party_in_scen_not_check;
extern Boolean in_startup_mode,registered,play_sounds,party_in_memory;
extern long register_flag;
extern HWND	mainPtr;	
extern short ulx,uly;
 extern piles_of_stuff_dumping_type *data_store;
extern piles_of_stuff_dumping_type2 *data_store2;


//void start_game();

RECT startup_button[6];

Boolean handle_startup_press(POINT the_point)
{

	short i,scen;
	long dummy;

	the_point.x -= ulx;
	the_point.y -= uly;
	
	for (i = 0; i < 5; i++) 
		if (PtInRect(&startup_button[i],the_point) == TRUE) {
//			flash_round_rect(start_button_rects[i],10);
			//OffsetRect(&start_button_rects[i],ul.h,ul.v);
			//startup_button_click(start_button_rects[i]);
			
		draw_start_button(i,5);
		if (play_sounds == TRUE)
			play_sound(37);
			else Delay(5,&dummy);
		draw_start_button(i,0);			
			switch (i) {
				case 0:
					startup_load();
					break;
					
				case 1:
					draw_startup(0);
					start_new_game();
					update_pc_graphics();
					draw_startup(0);
					break;
			
				case 2:
					give_reg_info();
						draw_startup(0);
					break;

				case 3:
					if (party_in_memory == FALSE) {
						FCD(867,0);
						break;
						}
					scen = pick_prefab_scen();
					if (scen < 0)
						break;
					if ((registered == FALSE) && (scen > 0)) {
						FCD(913,0);
						break;
						}

					switch (scen) {
						case 0: sprintf(party.scen_name,"valleydy.exs"); break;
						// if not reg, rub out
						case 1: sprintf(party.scen_name,"stealth.exs"); break;
						case 2: sprintf(party.scen_name,"zakhazi.exs"); break;
						}
					put_party_in_scen();
					break;

				case 4: // intro
					if (party_in_memory == FALSE) {
						FCD(867,0);
						break;
						}
					if (registered == FALSE) {
						FCD(913,0);
						break;
						}
					// if not reg, rub out

					scen = pick_a_scen();
					if (scen_headers[scen].prog_make_ver[0] >= 2) {
						FCD(912,0);
						break;
						}
					if (scen >= 0) {
						if (registered == FALSE)
							unreg_party_in_scen_not_check = TRUE;
						sprintf(party.scen_name,"%s",data_store2->scen_names[scen]);
						put_party_in_scen();
						}

					break;

				case 5:
					Delay(50,&dummy);
					return TRUE;
					break;
			
				}
			}
	return FALSE;
}
/*
void start_game () 
{
	init_party(0);

	setup_outdoors(party.p_loc);

	load_area_graphics();
	
	draw_main_screen();
	
	in_startup_mode = FALSE;
	
	adjust_monst_menu();
	adjust_spell_menus();
	
}*/

void startup_load()////
{
					load_file();
					update_pc_graphics();
					if (in_startup_mode == FALSE) {
						//end_anim();
						end_startup();
						post_load();
						}
						else {
							menu_activate(0);
							draw_startup(0);
							}

}
