#include "sound_management.h"
#include <nds.h>
#include <stdio.h>
#include "soundbank_bin.h"

typedef enum {PANNING_CENTER = 128, PANNING_LEFT = 0, PANNING_RIGHT = 255} Panning;

//Configures mm_sound_effect pointed by sound to id with panning, max volume and default rate
void setup_sound(mm_word id, mm_byte panning, mm_sound_effect * sound){
	sound->id = id;
	sound->rate    =	1024;	// rate
//	sound->handle  = 0;           // 0 = allocate new handle
	sound->volume  = 255;         //Max volume
	sound->panning = panning;         // centered panning
}

//Configures sound for id with centered panning
void setup_sound_effect(mm_word id){
	mm_sound_effect sound;
	setup_sound(id, PANNING_CENTER, &sound);
}

//Initializes soundbank and load effects and music
void init_sounds(){

	//Init the sound library
	mmInitDefaultMem((mm_addr)soundbank_bin);

	//Load and setup all sounds effects
	mmLoadEffect(SFX_BUTTON);
	setup_sound_effect(SFX_BUTTON);

	mmLoadEffect(SFX_PORTAL_ENTER);
	setup_sound_effect(SFX_PORTAL_ENTER);

	mmLoadEffect(SFX_PORTAL_INVALID_SURFACE);
	setup_sound_effect(SFX_PORTAL_INVALID_SURFACE);

	mmLoadEffect(SFX_PORTAL_OPEN);
	setup_sound_effect(SFX_PORTAL_OPEN);

	mmLoadEffect(SFX_SHOOT_BLUE);
	setup_sound_effect(SFX_SHOOT_BLUE);

	mmLoadEffect(SFX_SHOOT_ORANGE);
	setup_sound_effect(SFX_SHOOT_ORANGE);

	mmLoadEffect(SFX_TICKTOCK);
	setup_sound_effect(SFX_TICKTOCK);

	//Load the music
	mmLoad(MOD_STILL_ALIVE);
}

//Deallocate the sound effects and music
void deallocate_sound(){
	stop_music();
	mmUnloadEffect(SFX_BUTTON);

	mmUnloadEffect(SFX_PORTAL_ENTER);

	mmUnloadEffect(SFX_PORTAL_INVALID_SURFACE);

	mmUnloadEffect(SFX_PORTAL_OPEN);

	mmUnloadEffect(SFX_SHOOT_BLUE);

	mmUnloadEffect(SFX_SHOOT_ORANGE);

	mmUnloadEffect(SFX_TICKTOCK);

	mmUnload(MOD_STILL_ALIVE);
}

//Plays music
void play_music() {
	mmStart(MOD_STILL_ALIVE, MM_PLAY_LOOP);
}

//Stops music
void stop_music(){
	mmStop();
}

//Plays effect id with centered panning
void play_effect(mm_word id){
	mmEffect(id);
}

//Plays effect id with left panning
void play_effect_left(mm_word id){
	mm_sound_effect sound;
	setup_sound(id, PANNING_LEFT, &sound);
	mmEffectEx(&sound);
}

//Plays effect id with right panning
void play_effect_right(mm_word id){
	mm_sound_effect sound;
	setup_sound(id, PANNING_RIGHT, &sound);
	mmEffectEx(&sound);
}
