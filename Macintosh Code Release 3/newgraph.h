typedef	struct {	char word[15];	Rect word_rect;	} word_rect_type;typedef struct {	short per1,per2;	char stra[5],strb[5];	short ttype,a,b,c,d;	} hold_responses;	void apply_unseen_mask();void apply_light_mask();void end_anim();void init_anim(short which_anim);void run_anim();void run_dedicated_anim(short mode);void run_anim_0();void run_anim_1();void run_anim_2();void kill_a_monst(short which_m);location anim_get_space(short which);void put_anim_onscreen();void set_anim_str(char *astr,location l);void run_anim_pcs();void set_anim_attacker(short attacker);Boolean try_monst_anim_move(short i,location start,short x,short y);void run_anim_monsts();Boolean try_pc_anim_move(short i,location start,short x,short y);void start_missile_anim();short get_missile_direction(Point origin_point,Point the_point);void end_missile_anim() ;void run_a_missile(location from,location fire_to,short miss_type,short path,short sound_num,short x_adj,short y_adj,short len);void run_a_boom(location boom_where,short type,short x_adj,short y_adj);void mondo_boom(location l,short type);void add_missile(location dest,short missile_type,short path_type,short x_adj,short y_adj);void add_explosion(location dest,short val_to_place,short place_type,short boom_type,short x_adj,short y_adj);void do_missile_anim(short num_steps,location missile_origin,short sound_num) ;void do_explosion_anim(short sound_num,short expand);void click_shop_rect(Rect area_rect);void draw_shop_graphics(short draw_mode,Rect clip_area_rect);void refresh_shopping();item_record_type store_mage_spells(short which_s) ;item_record_type store_priest_spells(short which_s);item_record_type store_alchemy(short which_s);void get_item_interesting_string(item_record_type item,char *message);void click_talk_rect(char *str_to_place,char *str_to_place2,Rect c_rect);void place_talk_str(char *str_to_place,char *str_to_place2,short color,Rect c_rect);short scan_for_response(char *str);void refresh_talking();