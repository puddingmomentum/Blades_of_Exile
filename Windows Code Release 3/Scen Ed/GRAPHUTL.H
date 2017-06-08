extern void inflict_palette();
extern void reset_palette();
extern void init_palette(BYTE huge * lpDib);
extern DWORD GetDibInfoHeaderSize(BYTE huge * lpDib);

extern WORD GetDibHeight(BYTE huge * lpDib);
extern BYTE huge * GetDibBitsAddr(BYTE huge * lpDib);
extern HBITMAP ReadDib(char * name,HDC hdc);

extern WORD GetDibWidth(BYTE huge *);
HBITMAP load_pict(short pict_num);
extern void rect_draw_some_item(HBITMAP src,RECT src_rect,HBITMAP dest,RECT dest_rect,
	short trans, short main_win);
extern void fry_dc(HWND hwnd,HDC dc);
HBITMAP load_pict(short pict_num,HDC model_hdc);
void DisposeGWorld(HBITMAP bitmap);
void SectRect(RECT *a, RECT *b, RECT *c) ;
Boolean Button();
HBITMAP load_pict(short pict_num,HDC model_hdc);
void paint_pattern(HBITMAP dest,short which_mode,RECT dest_rect,short which_pattern);