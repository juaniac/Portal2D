#include "irq_management.h"
#include "sprite_management.h"
#include "graphics_main_engine.h"
#include "graphics_sub_engine.h"
#include "sound_management.h"

typedef enum {STARTUP, LEVEL1, LEVEL2, LEVEL3, LEVEL4, ENDING} Game_states;
#define NUMBER_OF_STATES 6
Game_states cur_game_state;

//Setup for ending state
void init_ending(){
    //Startup the main and sub engines.
    init_main_ending();
    init_sub_ending();

    //Startup the IRQs.
    init_IRQ_for_ending();

    //Deallocate the sound as it is no longer used.    
    deallocate_sound();
}

//Setup for any level state
void init_level(){

    //Only allocate and start playing the sound for the first level,
    //as every subsequent level will have it active.
	if(cur_game_state == LEVEL1){
		init_sounds();
		play_music();
	}

    //Load the main and sub engines,
    //we use the game state to figure out which level we load to the main engine.
	init_sub_for_gameplay();
	init_main_for_gameplay(cur_game_state);

    //Startup the IRQs.
	init_IRQ_for_gameplay();

	//Setup game objects.
	configureSprites();
	init_player();

	//Flush inputs at the beginning.
	scanKeys();
}

//Setup for startup state.
void init_startup(){
    cur_game_state = STARTUP;
    init_main_startup();
    init_sub_startup();
    init_IRQ_for_startup();
}

//Make the transition to the next game state.
void go_to_next_state(){
    cur_game_state = (cur_game_state + 1) % NUMBER_OF_STATES;
    if(cur_game_state == STARTUP){
        init_startup();
    }else if(cur_game_state == ENDING){
        init_ending();
    }else{
    	init_level();
    }
}
