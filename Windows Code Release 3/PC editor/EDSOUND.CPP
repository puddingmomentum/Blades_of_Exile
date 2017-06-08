#include "math.h"
#include <windows.h>
#include <mmsystem.h>

#include "stdio.h"

#include "global.h"
#include "edsound.h"

#define	NUM_SOUNDS	100

HGLOBAL sound_handles[NUM_SOUNDS];
char far *snds[NUM_SOUNDS];

extern HANDLE store_hInstance;

extern Boolean play_sounds,in_startup_mode;
extern HWND mainPtr;
extern Boolean gInBackground;
extern party_record_type far	party;
extern Boolean cat,cow,chicken,dog,sheep,monsters_going;
extern short num_chirps_played,overall_mode;

short last_played = 10000;
short error_beeps = 0;
short store_last_sound_played = 0;

Boolean far always_asynch[100] = {FALSE,FALSE,FALSE,FALSE,FALSE,
							TRUE,TRUE,FALSE,FALSE,FALSE,
							TRUE,FALSE,FALSE,FALSE,FALSE, // 10
							FALSE,FALSE,FALSE,FALSE,FALSE,
							FALSE,FALSE,TRUE,FALSE,TRUE, // 20
							TRUE,FALSE,FALSE,FALSE,FALSE,
							FALSE,FALSE,FALSE,FALSE,TRUE,  // 30
							FALSE,FALSE,TRUE,FALSE,TRUE,
							FALSE,TRUE,TRUE,TRUE,TRUE, // 40
							TRUE,TRUE,TRUE,TRUE,TRUE,
							TRUE,FALSE,FALSE,TRUE,FALSE, // 50
							TRUE,FALSE,FALSE,FALSE,FALSE,
							FALSE,TRUE,FALSE,FALSE,FALSE, // 60
							FALSE,FALSE,FALSE,FALSE,FALSE,
							FALSE,FALSE,FALSE,FALSE,FALSE, // 70
							FALSE,TRUE,TRUE,TRUE,TRUE,
							TRUE,TRUE,TRUE,TRUE,FALSE, // 80
							TRUE,FALSE,FALSE,FALSE,FALSE,
							FALSE,TRUE,FALSE,FALSE,FALSE, // 90
							FALSE,FALSE,FALSE,FALSE,FALSE};
Boolean far load_when_play[100] = {
0,0,1,1,1,0,0,1,1,1,
0,0,0,1,0,1,1,1,1,1,
1,1,0,1,1,1,1,0,1,1,
1,1,1,1,0,1,1,0,1,1,
1,1,1,1,1,1,1,0,0,0,
0,1,1,0,1,0,1,1,1,1, // 50
1,0,1,1,1,1,1,1,1,0,
0,0,0,0,0,0,0,0,0,0, // 70
1,1,1,1,1,0,0,0,0,0,
1,0,1,1,1,1,1,1,1,0
};

short far can_ignore[100] = {
0,0,0,0,0,5,3,0,0,0,
5,5,5,0,5,0,0,0,0,0,
0,0,5,0,0,0,0,0,0,2,
2,2,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,1,1,1,
1,0,0,5,0,1,0,0,0,0, // 50
0,0,0,0,5,0,0,0,0,0,
0,0,0,0,0, 0,5,5,5,5, // 70
4,4,4,4,0,0,0,0,0,0,
0,5,5,0,0,0,0,0,0,0};
// 1 - polite asych, 1 or 2 or 3- lose easily when pref is set for fewer snds
// 3 can be async
// 4 - nostop asynch ... when 4, this sound is NOSTOP, i.e. when played, is played
//     asynch, and refuses all other sounds. Sounds that come in are ignored, and
//     disappear into the ether
// 5 - finally, bold asynch ... when 5, this sound is NOSTOP, i.e. when played, is played
//     asynch, and refuses all other sounds. When a sound is played on top of this, game
//     hangs on until sound is done, and then and only then plays later sound.


short num_devs;
Boolean sounds_fucked = FALSE;
long intro_music_start_time = -1;

void load_sounds ()
{
	short i,t,err;
	HRSRC h;
	char snd_name[20],str[60];
	WAVEOUTCAPS wavecaps;

	t = waveOutGetNumDevs();
	if (t == 0) {
		sounds_fucked = TRUE;
		return;
		}
	err = waveOutGetDevCaps(0,&wavecaps,sizeof(WAVEOUTCAPS));
if (err != 0) {
	sounds_fucked = TRUE;
	switch (err) {
		case MMSYSERR_BADDEVICEID:
		MessageBox(mainPtr,"Cannot initialize sounds - No sound device detected. Game can still be played, but quietly.",
	  "Sound Error",MB_OK | MB_ICONEXCLAMATION);
		return;
		case MMSYSERR_NODRIVER:
		MessageBox(mainPtr,"Cannot initialize sounds - No driver installed. Game can still be played, but quietly.",
	  "Sound Error",MB_OK | MB_ICONEXCLAMATION);
		return;
		case MMSYSERR_NOMEM :
		MessageBox(mainPtr,"Cannot initialize sounds - can't find enough memory. Game can still be played, but quietly.",
	  "Sound Error",MB_OK | MB_ICONEXCLAMATION);
		return;
		case MMSYSERR_ALLOCATED:
		MessageBox(mainPtr,"Cannot initialize sounds - sound card already allocated. Game can still be played, but quietly.",
	  "Sound Error",MB_OK | MB_ICONEXCLAMATION);
		return;
		case MMSYSERR_ERROR:
		MessageBox(mainPtr,"Cannot initialize sounds - internal error. Game can still be played, but quietly.",
	  "Sound Error",MB_OK | MB_ICONEXCLAMATION);
		return;
		default:
		MessageBox(mainPtr,"Cannot initialize sounds - unidentified error. Game can still be played, but quietly.",
	  "Sound Error",MB_OK | MB_ICONEXCLAMATION);
		return;

		}

	}

	for (i = 0; i < NUM_SOUNDS; i++) {
		sound_handles[i] = NULL;
		load_when_play[i] = TRUE;
		if (load_when_play[i] == FALSE) {
		sprintf((char *)snd_name,"#%d",i + 1);
		h = FindResource(store_hInstance,snd_name,"#100");

		sound_handles[i] = LoadResource(store_hInstance,h);
		snds[i] = LockResource(sound_handles[i]);
		}
		}

}

void play_sound(short which)  // if < 0, play asynch
{
	if (PSD[304][9] > 0)
		return;
	if (in_startup_mode == TRUE) {
		// put some in foreground check here
		}
	if (play_sounds == TRUE)
		force_play_sound(which);
}


void force_play_sound(short which)
{
	short i,err,num_fails = 0;
	char snd_name[30],str[60];
	Boolean asyn = FALSE,a_sound_did_get_played = FALSE,sound_played_nostop = FALSE;
	Boolean check_sound;
	long dummy;
	HRSRC h;
	char far *snd;
	DWORD size;

	if ((sounds_fucked == TRUE) || (play_sounds == FALSE))
		return;
	if (which < 0) {
		asyn = TRUE;
		which = which * -1;
		}

	if (which >= 100)
		return;

	if ((always_asynch[which] == TRUE) &&
	((can_ignore[which] == 1) || (can_ignore[which] >= 3)))
		asyn = TRUE;
	if ((can_ignore[which] > 0) && (can_ignore[which] < 5) && (party.stuff_done[305][5] == 1))
		return;
	if ((can_ignore[which] != 1) && (can_ignore[which] < 3))
		asyn = FALSE;
	if ((party.stuff_done[305][5] == 1) && (can_ignore[which] < 5))
		asyn = FALSE;

	//print_nums(1000 + can_ignore[which],which,(short) asyn);

	if ((load_when_play[which] == TRUE) && (sound_handles[which] == NULL)) {
	  //	if (can_ignore[which] != 4)
			asyn = FALSE;
		sprintf((char *)snd_name,"#%d",which + 1);
		h = FindResource(store_hInstance,snd_name,"#100");

		sound_handles[which] = LoadResource(store_hInstance,h);
		snds[which] = LockResource(sound_handles[which]);

	  //	ASB("Loaded sound:");
	  //	print_nums(0,0,which);
		}

	if (store_last_sound_played == 6) {
		//ASB("Interrupted snd.");
		sndPlaySound(NULL,0);
		}

	if (asyn == TRUE) {
		if (can_ignore[which] >= 4)
			check_sound = sndPlaySound(snds[which],SND_ASYNC | SND_MEMORY | SND_NOSTOP);
			else check_sound = sndPlaySound(snds[which],SND_ASYNC | SND_MEMORY);

		while (check_sound == FALSE) {

			if (can_ignore[store_last_sound_played] == 4) {// then sound goes away
				//ASB("Sound overruled by asynch sound.");
				return;
				}


			num_fails++;
			if (num_fails < 40)
				sound_pause(25);
				else {
					MessageBox(mainPtr,"Cannot play sounds - Sounds stuck error a. Game can still be played, but quietly.",
					  "Sound Error",MB_OK | MB_ICONEXCLAMATION);
					//print_nums(111,which,num_fails);
					//sounds_fucked = TRUE;
					return;
					}
			//ASB ("Asynch clearing buffer!!!");
			//check_sound = sndPlaySound(snds[99],SND_SYNC | SND_MEMORY);
			sndPlaySound(NULL,0);

			if (can_ignore[which] >= 4)
				check_sound = sndPlaySound(snds[which],SND_ASYNC | SND_MEMORY | SND_NOSTOP);
				else check_sound = sndPlaySound(snds[which],SND_ASYNC | SND_MEMORY);
			}
	  a_sound_did_get_played = TRUE;
	  }
		else {
		if (can_ignore[which] >= 4)
			check_sound = sndPlaySound(snds[which],SND_SYNC | SND_MEMORY | SND_NOSTOP);
			else check_sound = sndPlaySound(snds[which],SND_SYNC | SND_MEMORY);
		while (check_sound == FALSE) {
			if (can_ignore[store_last_sound_played] == 4) {// then sound goes away
				//ASB("Sound overruled by asynch sound.");
				return;
				}


			num_fails++;
			if (num_fails < 40)
				sound_pause(25);
				else {
					MessageBox(mainPtr,"Cannot play sounds - Sounds stuck error b. Game can still be played, but quietly.",
					 "Sound Error",MB_OK | MB_ICONEXCLAMATION);
					//print_nums(222,which,num_fails);
					//sounds_fucked = TRUE;
					return;
					}
			//ASB ("Asynch clearing buffer!!!");
			//check_sound = sndPlaySound(snds[99],SND_SYNC | SND_MEMORY);
			sndPlaySound(NULL,0);

			if (can_ignore[which] >= 4)
				check_sound = sndPlaySound(snds[which],SND_SYNC | SND_MEMORY | SND_NOSTOP);
				else check_sound = sndPlaySound(snds[which],SND_SYNC | SND_MEMORY);
			}
		a_sound_did_get_played = TRUE;
	  }

	store_last_sound_played = which;

	if ((load_when_play[which] == TRUE) && (asyn == FALSE)) {
		// deleted a seemingly extraneous LoadResource here
		//ASB("Dumped a sound.");
		UnlockResource(sound_handles[which]);
		FreeResource(sound_handles[which]);
		sound_handles[which] = NULL;
		}
		//else if (load_when_play[which] == TRUE) ASB("Kept a temp. sound.");
	for (i = 0; i < NUM_SOUNDS; i++)
		if ((load_when_play[which] == TRUE) && (sound_handles[which] != NULL)
			&& (a_sound_did_get_played == TRUE) && (i != which)){
		// deleted a seemingly extraneous LoadResource here
		UnlockResource(sound_handles[i]);
		FreeResource(sound_handles[i]);
		sound_handles[i] = NULL;
		//ASB("Kept sound dumped:");
		//print_nums(0,0,i);
		}

}

void kill_sound()
{
			sndPlaySound(NULL,0);
}
void one_sound(short which)
{
	if (which == last_played)
		return;
	play_sound(which);
	last_played = which;
}

void clear_sound_memory()
{
	last_played = 100;
}

void flip_sound()
{
	play_sounds = (play_sounds == TRUE) ? FALSE : TRUE;
}


void sound_pause(long len) {
	long t1,t2;

	t1 = (long) GetCurrentTime();
	t2 = t1;
	while (t2 - t1 < len) {
		t2 = (long)GetCurrentTime();
		}
}