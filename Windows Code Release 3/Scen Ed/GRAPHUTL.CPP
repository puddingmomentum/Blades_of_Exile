#define xmin(a,b)	((a) < (b) ?  (a) : (b))

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "global.h"


#include "edsound.h"

#include "graphutl.h"

extern HWND mainPtr;
extern HPALETTE hpal;
extern far PALETTEENTRY ape[256];
extern HDC main_dc,main_dc2,main_dc3;
extern short ulx,uly;
extern HBITMAP mixed_gworld,dialog_pattern_gworld,pattern_gworld,status_pattern_gworld;
extern Boolean suppress_load_file_name;

extern char far scen_strs[160][256];
extern char far scen_strs2[110][256];
extern char far talk_strs[170][256];
extern char far town_strs[180][256];
extern char file_path_name[256];
HPALETTE opening_palette;

BOOL pal_ok = FALSE;
Boolean syscolors_stored = FALSE;
	int elements[5] = {COLOR_ACTIVEBORDER,COLOR_ACTIVECAPTION,
			COLOR_WINDOWFRAME,COLOR_SCROLLBAR,COLOR_BTNFACE};
	COLORREF store_element_colors[5];
short dlog_pat_placed = 0;
short current_pattern = -1;
HPALETTE syspal = NULL;

void init_palette(BYTE huge * lpDib)
{
	HDC hdc;
	short i,red,green,blue;
	LOGPALETTE *plgpl = NULL;
	LOCALHANDLE l;

	BITMAPINFO *store_info;
	BITMAPCOREINFO *store_core_info;
	RGBTRIPLE store_c[256];
	DWORD dwNumColors, dwColorTableSize;
	WORD wBitCount;
  char debug[80];
 HBRUSH hbr;
	if (pal_ok == TRUE)
		return;
	pal_ok = TRUE;

	if (GetDibInfoHeaderSize(lpDib) == sizeof(BITMAPCOREHEADER)) {
		wBitCount = ((BITMAPCOREHEADER huge *) lpDib)->bcBitCount;
		if (wBitCount != 24)
			dwNumColors = 1L << wBitCount;
			else dwNumColors = 0;
	dwColorTableSize = dwNumColors * sizeof(RGBTRIPLE);

	store_core_info = (BITMAPCOREINFO*) lpDib;
	for (i = 0; i < dwNumColors; i++) {
		store_c[i].rgbtRed = store_core_info->bmciColors[i].rgbtRed;
		store_c[i].rgbtGreen = store_core_info->bmciColors[i].rgbtGreen;
		store_c[i].rgbtBlue = store_core_info->bmciColors[i].rgbtBlue;
		}
	}
	else {
		wBitCount = ((BITMAPINFOHEADER huge *) lpDib)->biBitCount;
		if (GetDibInfoHeaderSize(lpDib) >= 36)
			dwNumColors = ((BITMAPINFOHEADER huge *) lpDib)->biClrUsed;
		if (dwNumColors == 0) {
			if (wBitCount != 24)
				dwNumColors = 1L << wBitCount;
				else dwNumColors = 0;
			}
		dwColorTableSize = dwNumColors * sizeof(RGBQUAD);

		store_info = (BITMAPINFO *) lpDib;
		for (i = 0; i < dwNumColors; i++) {
			store_c[i].rgbtRed = store_info->bmiColors[i].rgbRed;
			store_c[i].rgbtGreen = store_info->bmiColors[i].rgbGreen;
			store_c[i].rgbtBlue = store_info->bmiColors[i].rgbBlue;
			}
		}

  hdc = GetDC(mainPtr);

l = LocalAlloc(LHND,
		sizeof(LOGPALETTE) + dwNumColors * sizeof(PALETTEENTRY));
plgpl = (LOGPALETTE*) LocalLock(l);

	plgpl->palNumEntries = (WORD) (dwNumColors);
	plgpl->palVersion = 0x300;

	for (i = 0; i < dwNumColors; i++) {
			ape[i].peRed =
				plgpl->palPalEntry[i].peRed = store_c[i].rgbtRed;
			ape[i].peGreen =
				plgpl->palPalEntry[i].peGreen = store_c[i].rgbtGreen;
			ape[i].peBlue =
				plgpl->palPalEntry[i].peBlue = store_c[i].rgbtBlue;
			ape[i].peFlags =
				plgpl->palPalEntry[i].peFlags = PC_NOCOLLAPSE;

		}

	hpal = CreatePalette(plgpl);

	GetSystemPaletteEntries(hdc,0,255,(PALETTEENTRY FAR*) ape);

	inflict_palette();

	LocalUnlock(l);
	LocalFree(l);

	fry_dc(mainPtr,hdc);
	SendMessage(HWND_BROADCAST,WM_SYSCOLORCHANGE,0,0);
}

// extracts and inflicts palette from given dib. WARNING ...
// does NOT do any deleting or cleanup
void extract_given_palette(BYTE huge * lpDib)
{
	HDC hdc;
	short i,red,green,blue;
	LOGPALETTE *plgpl = NULL;
	LOCALHANDLE l;

	BITMAPINFO *store_info;
	BITMAPCOREINFO *store_core_info;
	RGBTRIPLE store_c[256];
	DWORD dwNumColors, dwColorTableSize;
	WORD wBitCount;
	HBRUSH hbr;

	if (GetDibInfoHeaderSize(lpDib) == sizeof(BITMAPCOREHEADER)) {
		wBitCount = ((BITMAPCOREHEADER huge *) lpDib)->bcBitCount;
		if (wBitCount != 24)
			dwNumColors = 1L << wBitCount;
			else dwNumColors = 0;
	dwColorTableSize = dwNumColors * sizeof(RGBTRIPLE);

	store_core_info = (BITMAPCOREINFO*) lpDib;
	for (i = 0; i < dwNumColors; i++) {
		store_c[i].rgbtRed = store_core_info->bmciColors[i].rgbtRed;
		store_c[i].rgbtGreen = store_core_info->bmciColors[i].rgbtGreen;
		store_c[i].rgbtBlue = store_core_info->bmciColors[i].rgbtBlue;
		}
	}
	else {
		wBitCount = ((BITMAPINFOHEADER huge *) lpDib)->biBitCount;
		if (GetDibInfoHeaderSize(lpDib) >= 36)
			dwNumColors = ((BITMAPINFOHEADER huge *) lpDib)->biClrUsed;
		if (dwNumColors == 0) {
			if (wBitCount != 24)
				dwNumColors = 1L << wBitCount;
				else dwNumColors = 0;
			}
		dwColorTableSize = dwNumColors * sizeof(RGBQUAD);

		store_info = (BITMAPINFO *) lpDib;
		for (i = 0; i < dwNumColors; i++) {
			store_c[i].rgbtRed = store_info->bmiColors[i].rgbRed;
			store_c[i].rgbtGreen = store_info->bmiColors[i].rgbGreen;
			store_c[i].rgbtBlue = store_info->bmiColors[i].rgbBlue;
			}
		}

  hdc = GetDC(mainPtr);

	l = LocalAlloc(LHND,
		sizeof(LOGPALETTE) + dwNumColors * sizeof(PALETTEENTRY));
	plgpl = (LOGPALETTE*) LocalLock(l);

	plgpl->palNumEntries = (WORD) (dwNumColors);
	plgpl->palVersion = 0x300;

	for (i = 0; i < dwNumColors; i++) {

				plgpl->palPalEntry[i].peRed = store_c[i].rgbtRed;

				plgpl->palPalEntry[i].peGreen = store_c[i].rgbtGreen;

				plgpl->palPalEntry[i].peBlue = store_c[i].rgbtBlue;

				plgpl->palPalEntry[i].peFlags = PC_NOCOLLAPSE;

		}

	opening_palette = CreatePalette(plgpl);

	//GetSystemPaletteEntries(hdc,0,255,(PALETTEENTRY FAR*) ape);

	//inflict_palette();
	SelectPalette(main_dc,opening_palette,0);
	RealizePalette(main_dc);
	SetSystemPaletteUse(main_dc,SYSPAL_NOSTATIC);
	UnrealizeObject(opening_palette);
	RealizePalette(main_dc);
	LocalUnlock(l);
	LocalFree(l);

	fry_dc(mainPtr,hdc);
	//SendMessage(HWND_BROADCAST,WM_SYSCOLORCHANGE,0,0);
}


void inflict_palette()
{
	COLORREF x = RGB(0,0,192),y = RGB(0,128,0);
	UINT c;
	COLORREF elem_color[5];

	if (syscolors_stored == FALSE) {
		store_element_colors[0] = GetSysColor(elements[0]);
		store_element_colors[1] = GetSysColor(elements[1]);
		store_element_colors[2] = GetSysColor(elements[2]);
		store_element_colors[3] = GetSysColor(elements[3]);
		store_element_colors[4] = GetSysColor(elements[4]);
		syscolors_stored = TRUE;
		}
	c = GetNearestPaletteIndex(hpal,x);
	elem_color[0] = RGB(192,192,192);
	elem_color[1] = RGB(128,128,128);
	elem_color[2] = RGB(0,0,0);//192,192,192);
	elem_color[3] = RGB(128,128,128);
	elem_color[4] = RGB(192,192,192);
	SetSysColors(5,elements,elem_color);
// 	int elements[5] = {COLOR_ACTIVEBORDER,COLOR_ACTIVECAPTION,
//			COLOR_WINDOWFRAME,COLOR_SCROLLBAR,COLOR_BTNFACE};

	SelectPalette(main_dc,hpal,0);
	RealizePalette(main_dc);
	SetSystemPaletteUse(main_dc,SYSPAL_NOSTATIC);
	UnrealizeObject(hpal);
	RealizePalette(main_dc);
	SendMessage(HWND_BROADCAST,WM_SYSCOLORCHANGE,0,0);
/*
	store_element_colors[0] = GetSysColor(elements[0]);
	store_element_colors[1] = GetSysColor(elements[1]);
	store_element_colors[2] = GetSysColor(elements[2]);
	c = GetNearestPaletteIndex(hpal,x);
	elem_color[0] = RGB(255,0,0);
	elem_color[1] = RGB(255,0,0);
	elem_color[2] = RGB(255,0,0);
	SetSysColors(3,elements,elem_color);
*/
	}

void reset_palette()
{
	LOGPALETTE *plgpl = NULL;
	LOCALHANDLE l;
	short i;

	if (syspal == NULL) {
			l = LocalAlloc(LHND,
		sizeof(LOGPALETTE) + 255 * sizeof(PALETTEENTRY));
	plgpl = (LOGPALETTE*) LocalLock(l);

	plgpl->palNumEntries = (WORD) (255);
	plgpl->palVersion = 0x300;

	for (i = 0; i < 255; i++) {
				plgpl->palPalEntry[i].peRed = ape[i].peRed;
				plgpl->palPalEntry[i].peGreen = ape[i].peGreen;
				plgpl->palPalEntry[i].peBlue = ape[i].peBlue;
				plgpl->palPalEntry[i].peFlags = PC_NOCOLLAPSE;
			}

		syspal = CreatePalette(plgpl);
		}
	SetSystemPaletteUse(main_dc,SYSPAL_STATIC);
	UnrealizeObject(hpal);
	SelectPalette(main_dc,syspal,0);
	RealizePalette(main_dc);
	SendMessage(HWND_BROADCAST,WM_SYSCOLORCHANGE,0,0);
	SetSysColors(5,elements,store_element_colors);


}

DWORD GetDibInfoHeaderSize(BYTE huge * lpDib)
{
return ((BITMAPINFOHEADER huge *) lpDib)->biSize;
}

WORD GetDibWidth(BYTE huge * lpDib)
{
	if (GetDibInfoHeaderSize(lpDib) == sizeof(BITMAPCOREHEADER))
		return (WORD) (((BITMAPCOREHEADER huge *) lpDib)->bcWidth);
		else return (WORD) (((BITMAPINFOHEADER huge *) lpDib)->biWidth);
}

WORD GetDibHeight(BYTE huge * lpDib)
{
	if (GetDibInfoHeaderSize(lpDib) == sizeof(BITMAPCOREHEADER))
		return (WORD) (((BITMAPCOREHEADER huge *) lpDib)->bcHeight);
		else return (WORD) (((BITMAPINFOHEADER huge *) lpDib)->biHeight);
}
BYTE huge * GetDibBitsAddr(BYTE huge * lpDib)
{
	DWORD dwNumColors, dwColorTableSize;
	WORD wBitCount;
	short i;

	if (GetDibInfoHeaderSize(lpDib) == sizeof(BITMAPCOREHEADER)) {
		wBitCount = ((BITMAPCOREHEADER huge *) lpDib)->bcBitCount;
		if (wBitCount != 24)
			dwNumColors = 1L << wBitCount;
			else dwNumColors = 0;
	dwColorTableSize = dwNumColors * sizeof(RGBTRIPLE);
	}
	else {
		wBitCount = ((BITMAPINFOHEADER huge *) lpDib)->biBitCount;
		if (GetDibInfoHeaderSize(lpDib) >= 36)
			dwNumColors = ((BITMAPINFOHEADER huge *) lpDib)->biClrUsed;
		if (dwNumColors == 0) {
			if (wBitCount != 24)
				dwNumColors = 1L << wBitCount;
				else dwNumColors = 0;
			}
		dwColorTableSize = dwNumColors * sizeof(RGBQUAD);
		}


	return lpDib + GetDibInfoHeaderSize(lpDib) + dwColorTableSize;
	}

HBITMAP ReadDib(char * name,HDC hdc) {
BITMAPFILEHEADER bmfh;
BYTE huge * lpDib;
DWORD dwDibSize, dwOffset, dwHeaderSize;
int hFile;
WORD wDibRead;
BYTE huge * lpDibBits;
HBITMAP bmap;
OFSTRUCT store;
char real_name[256] = "",*name_ptr;
short i,last_slash = -1;

	if (suppress_load_file_name == FALSE) {
	for (i = 0; i < 256; i++)
		if ((file_path_name[i] == 92) || (file_path_name[i] == '/'))
			last_slash = i;
	if (last_slash < 0) {
		strcpy((char *) real_name,name);
		}
		else {
			strcpy(real_name,file_path_name);
			name_ptr = (char *) real_name;
			name_ptr += last_slash + 1;
			sprintf((char *) name_ptr,"%s",name);
			//real_name -= last_slash + 1;
			//ASB(real_name);
			}
		}
		else  strcpy(real_name,name);
	hFile = OpenFile(real_name,&store,OF_READ | OF_SEARCH | OF_SHARE_DENY_WRITE);

	if (hFile == HFILE_ERROR)
		return NULL;
//	if (-1 == (hFile = _lopen(name,OF_READ|OF_SHARE_DENY_WRITE)))
//		return NULL;
	if (_lread(hFile, (LPSTR) &bmfh, sizeof(BITMAPFILEHEADER))
	!= sizeof(BITMAPFILEHEADER)) {
		_lclose(hFile);
		return NULL;
		}

//	if (bmfh.bfType != * (WORD *) "BM") {
//		_lclose(hFile);
//		return NULL;
//		}
	dwDibSize = bmfh.bfSize - sizeof(BITMAPFILEHEADER);
	lpDib = (BYTE huge *) GlobalAllocPtr(GMEM_MOVEABLE, dwDibSize);
	if (lpDib == NULL){
		_lclose(hFile);
		return NULL;
		}
	dwOffset = 0;
	while (dwDibSize > 0) {
		wDibRead = (WORD) xmin(32768ul,dwDibSize);
		if (wDibRead != _lread(hFile, (LPSTR) (lpDib + dwOffset),
			wDibRead)) {
				_lclose(hFile);
				GlobalFreePtr(lpDib);
				return NULL;
				}
			dwDibSize -= wDibRead;
			dwOffset += wDibRead;
		}
	_lclose(hFile);
	dwHeaderSize = GetDibInfoHeaderSize(lpDib);
	if ((dwHeaderSize < 12) || ((dwHeaderSize > 12) && (dwHeaderSize < 16))) {
		GlobalFreePtr(lpDib);
		return NULL;
		}

	init_palette(lpDib);
	SelectPalette(hdc,hpal,0);

	if ((name[0] == 'S') && (name[1] == 'T') && (name[5] == '.'))  {
		extract_given_palette(lpDib);
		}
	lpDibBits = GetDibBitsAddr(lpDib);
	bmap = CreateDIBitmap(hdc,(LPBITMAPINFOHEADER) lpDib, CBM_INIT,
		(LPSTR) lpDibBits, (LPBITMAPINFO) lpDib, DIB_RGB_COLORS);
	GlobalFreePtr(lpDib);
	return bmap;
}

HBITMAP load_pict(short pict_num)
{
	HBITMAP got_bitmap;
	HDC model_hdc;

	model_hdc = main_dc;

	switch(pict_num) {
		case 700: case 701: case 702: got_bitmap = ReadDib("STATAREA.BMP",model_hdc); break;
		case 703: got_bitmap = ReadDib("TEXTBAR.BMP",model_hdc); break;
		case 704: got_bitmap = ReadDib("BUTTONS.BMP",model_hdc); break;
		case 705: got_bitmap = ReadDib("TERSCRN.BMP",model_hdc); break;
		case 800: got_bitmap = ReadDib("TER1.BMP",model_hdc); break;
		case 801: got_bitmap = ReadDib("TER2.BMP",model_hdc); break;
		case 802: got_bitmap = ReadDib("TER3.BMP",model_hdc); break;
		case 803: got_bitmap = ReadDib("TER4.BMP",model_hdc); break;
		case 804: got_bitmap = ReadDib("TER5.BMP",model_hdc); break;
		case 805: got_bitmap = ReadDib("TER6.BMP",model_hdc); break;
		case 820: got_bitmap = ReadDib("TERANIM.BMP",model_hdc); break;
		case 821: got_bitmap = ReadDib("FIELDS.BMP",model_hdc); break;
		case 830: got_bitmap = ReadDib("STARTUP.BMP",model_hdc); break;
		case 850: got_bitmap = ReadDib("DLOGPICS.BMP",model_hdc); break;
		case 851: got_bitmap = ReadDib("SCENPICS.BMP",model_hdc); break;
		case 860: got_bitmap = ReadDib("TALKPORT.BMP",model_hdc); break;
		case 880: got_bitmap = ReadDib("MISSILES.BMP",model_hdc); break;
		case 900: got_bitmap = ReadDib("TINYOBJ.BMP",model_hdc); break;
		case 901: got_bitmap = ReadDib("OBJECTS.BMP",model_hdc); break;
		case 902: got_bitmap = ReadDib("PCS.BMP",model_hdc); break;
		case 905: got_bitmap = ReadDib("PCS.BMP",model_hdc); break;
		case 903: case 904: got_bitmap = ReadDib("MIXED.BMP",model_hdc); break;
		case 906: got_bitmap = ReadDib("EDBTNS.BMP",model_hdc); break;
		case 910: case 911: case 912: got_bitmap = ReadDib("BIGSCEN.BMP",model_hdc); break;
		case 1100: case 1200: got_bitmap = ReadDib("MONST1.BMP",model_hdc); break;
		case 1101: case 1201: got_bitmap = ReadDib("MONST2.BMP",model_hdc); break;
		case 1102: case 1202: got_bitmap = ReadDib("MONST3.BMP",model_hdc); break;
		case 1103: case 1203: got_bitmap = ReadDib("MONST4.BMP",model_hdc); break;
		case 1104: case 1204: got_bitmap = ReadDib("MONST5.BMP",model_hdc); break;
		case 1105: case 1205: got_bitmap = ReadDib("MONST6.BMP",model_hdc); break;
		case 1106: case 1206: got_bitmap = ReadDib("MONST7.BMP",model_hdc); break;
		case 1107: case 1207: got_bitmap = ReadDib("MONST8.BMP",model_hdc); break;
		case 1108: case 1208: got_bitmap = ReadDib("MONST9.BMP",model_hdc); break;
		case 1109: case 1209: got_bitmap = ReadDib("MONST10.BMP",model_hdc); break;
		case 1400: got_bitmap = ReadDib("STSCICON.BMP",model_hdc); break;
		case 1401: got_bitmap = ReadDib("HELPPICS.BMP",model_hdc); break;
		case 1402: got_bitmap = ReadDib("APPIC.BMP",model_hdc); break;
		case 1500: case 1501: case 1502: case 1503: case 1504: case 1505: case 1506: case 1507: 
			got_bitmap = ReadDib("BIGMAPS.BMP",model_hdc); break;
		case 2000: got_bitmap = ReadDib("DLOGBTNS.BMP",model_hdc); break;
		case 3000: got_bitmap = ReadDib("START.BMP",model_hdc); break;
		case 3001: got_bitmap = ReadDib("SPIDLOGO.BMP",model_hdc); break;
		case 3002: got_bitmap = ReadDib("EDSTART.BMP",model_hdc); break;
		case 5000: got_bitmap = ReadDib("E3EDTITL.BMP",model_hdc); break;
		
		default: got_bitmap = NULL;
		}
	return got_bitmap;
}

void rect_draw_some_item(HBITMAP src,RECT src_rect,HBITMAP dest,RECT dest_rect,
	short trans, short main_win) {
	HDC hdcMem,hdcMem2,hdcMem3,destDC,storeDC;
	HBITMAP transbmp;
	COLORREF white = RGB(255,255,255),black = RGB(0,0,0),oldcolor;
	RECT debug = {0,0,200,20};
	HBRUSH hbrush,old_brush;
	COLORREF x = RGB(17,17,17);
	UINT c;
	HBITMAP store,store2;
	Boolean dlog_draw = FALSE;

	main_dc2 = CreateCompatibleDC(main_dc);
	SetMapMode(main_dc2,GetMapMode(mainPtr));
	SelectPalette(main_dc2,hpal,0);
	main_dc3 = CreateCompatibleDC(main_dc);
	SetMapMode(main_dc3,GetMapMode(mainPtr));
	SelectPalette(main_dc3,hpal,0);
	SetStretchBltMode(main_dc2,STRETCH_DELETESCANS);
	SetStretchBltMode(main_dc3,STRETCH_DELETESCANS);

	if (main_win == 2) {
		destDC = (HDC) dest;
		main_win = 1;
		dlog_draw = TRUE;
		hdcMem = CreateCompatibleDC(destDC);
		SelectObject(hdcMem, src);
		SetMapMode(hdcMem,GetMapMode(mainPtr));
		SelectPalette(hdcMem,hpal,0);
		SetStretchBltMode(hdcMem,STRETCH_DELETESCANS);
		}
		else {
			destDC = main_dc;
			hdcMem = main_dc2;
			store = SelectObject(hdcMem,src);
			}


	SetStretchBltMode(destDC,STRETCH_DELETESCANS);
	SetStretchBltMode(hdcMem,STRETCH_DELETESCANS);
	if (trans != 1) {
		if (main_win == 0) { // Not transparent, into bitmap
			hdcMem2 = main_dc3;
			store2 = SelectObject(hdcMem2, dest);
			/*CreateCompatibleDC(hdcMem);
			SelectObject(hdcMem2, dest);
			SetMapMode(hdcMem2,GetMapMode(mainPtr));
			SelectPalette(hdcMem2,hpal,0);  */

			SetStretchBltMode(hdcMem2,STRETCH_DELETESCANS);

				StretchBlt(hdcMem2,dest_rect.left,dest_rect.top,dest_rect.right - dest_rect.left,
				dest_rect.bottom - dest_rect.top,
				hdcMem,src_rect.left,src_rect.top,src_rect.right - src_rect.left,
				src_rect.bottom - src_rect.top,(trans >= 0) ? SRCCOPY : SRCAND);
			SelectObject(hdcMem2,store2);
			//DeleteDC(hdcMem2);
			}

		else { // Not transparent, onto screen
		if (trans == 2) {
			c = GetNearestPaletteIndex(hpal,x);
			hbrush = CreateSolidBrush(PALETTEINDEX(c));
			old_brush = SelectObject(destDC,hbrush);
			//SelectObject(hdcMem,hbrush);
			}
		if (dlog_draw == FALSE)
			SetViewportOrg(destDC,ulx,uly);

		SetStretchBltMode(destDC,STRETCH_DELETESCANS);
		SetStretchBltMode(hdcMem,STRETCH_DELETESCANS);
		StretchBlt(destDC,dest_rect.left,dest_rect.top,dest_rect.right - dest_rect.left,
			dest_rect.bottom - dest_rect.top,
			hdcMem,src_rect.left,src_rect.top,src_rect.right - src_rect.left,
			src_rect.bottom - src_rect.top,(trans == 0) ? SRCCOPY : MERGECOPY);
			if (trans == 2) {
				SelectObject(destDC,old_brush);
				if (DeleteObject(hbrush) == 0)
					play_sound(1);
				}
		if (dlog_draw == FALSE)
			SetViewportOrg(destDC,0,0);


		}
		} // end of non-transparent draws
		else {
		if (main_win == 0) {
			hdcMem3 = CreateCompatibleDC(hdcMem);
			SelectObject(hdcMem3, dest);
			SetMapMode(hdcMem3,GetMapMode(mainPtr));
			SelectPalette(hdcMem3,hpal,0);
			transbmp = CreateBitmap(src_rect.right - src_rect.left,
						src_rect.bottom - src_rect.top,1,1,NULL);
		hdcMem2 = CreateCompatibleDC(destDC);
		SelectObject(hdcMem2, transbmp);
		oldcolor = SetBkColor(hdcMem, white);
		StretchBlt(hdcMem2,0,0,dest_rect.right - dest_rect.left,
			dest_rect.bottom - dest_rect.top,
			hdcMem,src_rect.left,src_rect.top,src_rect.right - src_rect.left,
			src_rect.bottom - src_rect.top,SRCCOPY);
		SetBkColor(hdcMem, oldcolor);

		StretchBlt(hdcMem3,dest_rect.left,dest_rect.top,dest_rect.right - dest_rect.left,
			dest_rect.bottom - dest_rect.top,
			hdcMem,src_rect.left,src_rect.top,src_rect.right - src_rect.left,
			src_rect.bottom - src_rect.top,SRCINVERT);
		StretchBlt(hdcMem3,dest_rect.left,dest_rect.top,dest_rect.right - dest_rect.left,
			dest_rect.bottom - dest_rect.top,
			hdcMem2,0,0,src_rect.right - src_rect.left,
			src_rect.bottom - src_rect.top,SRCAND);
		StretchBlt(hdcMem3,dest_rect.left,dest_rect.top,dest_rect.right - dest_rect.left,
			dest_rect.bottom - dest_rect.top,
			hdcMem,src_rect.left,src_rect.top,src_rect.right - src_rect.left,
			src_rect.bottom - src_rect.top,SRCINVERT);
		DeleteDC(hdcMem3);
		DeleteDC(hdcMem2);

			DeleteObject(transbmp);
		}
		else {
			if (dlog_draw == FALSE)
				SetViewportOrg(destDC,ulx,uly);
			transbmp = CreateBitmap(src_rect.right - src_rect.left,
						src_rect.bottom - src_rect.top,1,1,NULL);
			hdcMem2 = CreateCompatibleDC(destDC);
			SelectObject(hdcMem2, transbmp);
			oldcolor = SetBkColor(hdcMem, white);
			StretchBlt(hdcMem2,0,0,dest_rect.right - dest_rect.left,
				dest_rect.bottom - dest_rect.top,
				hdcMem,src_rect.left,src_rect.top,src_rect.right - src_rect.left,
				src_rect.bottom - src_rect.top,SRCCOPY);

			SetBkColor(hdcMem, oldcolor);

			StretchBlt(destDC,dest_rect.left,dest_rect.top,dest_rect.right - dest_rect.left,
				dest_rect.bottom - dest_rect.top,
				hdcMem,src_rect.left,src_rect.top,src_rect.right - src_rect.left,
				src_rect.bottom - src_rect.top,SRCINVERT);
			StretchBlt(destDC,dest_rect.left,dest_rect.top,dest_rect.right - dest_rect.left,
				dest_rect.bottom - dest_rect.top,
				hdcMem2,0,0,src_rect.right - src_rect.left,
				src_rect.bottom - src_rect.top,SRCAND);
			StretchBlt(destDC,dest_rect.left,dest_rect.top,dest_rect.right - dest_rect.left,
				dest_rect.bottom - dest_rect.top,
				hdcMem,src_rect.left,src_rect.top,src_rect.right - src_rect.left,
				src_rect.bottom - src_rect.top,SRCINVERT);
			if (dlog_draw == FALSE)
				SetViewportOrg(destDC,0,0);
			DeleteDC(hdcMem2);

				DeleteObject(transbmp);


			}
			}
	if (dlog_draw == TRUE)
		DeleteDC(hdcMem);
		else SelectObject(hdcMem,store);
	DeleteDC(main_dc2);
	DeleteDC(main_dc3);
}

 void fry_dc(HWND hwnd,HDC dc) {
	if (ReleaseDC(hwnd,dc) == 0)
	  //	add_string_to_buf("xx");
		PostQuitMessage(0);
	if (DeleteDC(dc) == 0)
		//add_string_to_buf("yy");
		PostQuitMessage(0);
 }

void DisposeGWorld(HBITMAP bitmap)
{
	DeleteObject(bitmap);
}

void SectRect(RECT *a, RECT *b, RECT *c) 
	{
	IntersectRect(c,a,b);
	}

Boolean Button()
{
	MSG msg;

				if (PeekMessage(&msg,mainPtr,WM_MOUSEFIRST,WM_MOUSELAST,PM_REMOVE) > 0)
					if ((msg.message == WM_LBUTTONDOWN) || (msg.message == WM_CHAR)
					|| (msg.message == WM_KEYDOWN))
						return TRUE;
				if (PeekMessage(&msg,mainPtr,WM_KEYFIRST,WM_KEYLAST,PM_REMOVE) > 0)
					if ((msg.message == WM_LBUTTONDOWN) || (msg.message == WM_CHAR)
					|| (msg.message == WM_KEYDOWN))
						return TRUE;
	return FALSE;
}

// which_mode is 0 ... dest is a bitmap
// is 1 ... ignore dest ... paint on mainPtr
// is 2 ... dest is a dialog, use the dialog pattern
// both pattern gworlds are 192 x 256
void paint_pattern(HBITMAP dest,short which_mode,RECT dest_rect,short which_pattern)
{
	HBITMAP source_pat;
	RECT pattern_source = {32,168,96,232}, pat_dest_orig = {0,0,64,64},pat_dest;
	short i,j;

	RECT draw_from_orig = {0,0,192,256},draw_from,draw_to;
	short store_ulx,store_uly;

	if (which_mode == 1)
		OffsetRect(&dest_rect,ulx, uly);
		else if (which_mode == 3)
      	which_mode = 1;
	if (which_mode == 2) {
		source_pat = dialog_pattern_gworld;
		if (dlog_pat_placed == 0) {
			dlog_pat_placed = 1;
			OffsetRect(&pattern_source, 64 * 2,0);
			for (i = 0; i < 3; i++)
				for (j = 0; j < 4; j++) {
					pat_dest = pat_dest_orig;
					OffsetRect(&pat_dest,64 * i, 64 * j);
					rect_draw_some_item(mixed_gworld,pattern_source,
						dialog_pattern_gworld,pat_dest,0,0);
					}
			}
		}
		else {
			source_pat = pattern_gworld;
			if (current_pattern != which_pattern) {
				current_pattern = which_pattern;
				OffsetRect(&pattern_source, 64 * (which_pattern % 5),
					64 * (which_pattern / 5));
				for (i = 0; i < 3; i++)
					for (j = 0; j < 4; j++) {
						pat_dest = pat_dest_orig;
						OffsetRect(&pat_dest,64 * i, 64 * j);
						rect_draw_some_item(mixed_gworld,pattern_source,
							pattern_gworld,pat_dest,0,0);
						}
				}
			}

	// now patterns are loaded, so have fun
	// first nullify ul shifting
	store_ulx = ulx;
	store_uly = uly;
	ulx = uly = 0;
	for (i = 0; i < (dest_rect.right / 192) + 1; i++)
		for (j = 0; j < (dest_rect.bottom / 256) + 1; j++) {
			draw_to = draw_from_orig;
			OffsetRect(&draw_to,192 * i, 256 * j);
			IntersectRect(&draw_to,&draw_to,&dest_rect);
			if (draw_to.right != 0) {
				draw_from = draw_to;
				OffsetRect(&draw_from, -192 * i, -256 * j);
				switch (which_mode) {
					case 0:
						rect_draw_some_item(source_pat,draw_from,
							dest,draw_to,0,0); break;
					case 1:
						rect_draw_some_item(source_pat,draw_from,
							source_pat,draw_to,0,1); break;
					case 2:
						rect_draw_some_item(source_pat,draw_from,
							dest,draw_to,0,2); break;
					}
				}
			}
	ulx = store_ulx;
   uly = store_uly;
}