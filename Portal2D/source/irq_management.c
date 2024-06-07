#include "irq_management.h"
#include "sprite_management.h"
#include "map.h"
#include "game_state_management.h"
#include "sound_management.h"
#include <stdio.h>

#define TIMER_HZ 2
#define TOTAL_TIME_IN_SECONDS 10
int time_left_door_open = 0;

//Setup the Start interrupt
void init_IRQ_for_startup(){
    REG_KEYCNT = KEY_START | 1<<14;
    irqSet(IRQ_KEYS, &ISR_key_Start);
    irqEnable(IRQ_KEYS);
}

//The start ISR, it disables itself 
//and let's the player move to the next game state.
void ISR_key_Start(){
    REG_KEYCNT = 0;
    irqDisable(IRQ_KEYS);
    go_to_next_state();
}

//Setup the VBlank and A key interrupts.
void init_IRQ_for_gameplay(){
    irqSet(IRQ_VBLANK, &ISR_VBlank); 
    irqEnable(IRQ_VBLANK);

    REG_KEYCNT = KEY_A | 1<<14;
    irqSet(IRQ_KEYS, &ISR_key_A);
    irqEnable(IRQ_KEYS);
}

//Setup for the ending IRQs, 
//disabling previous active interrupts and actitivating Start.
void init_IRQ_for_ending(){
    disable_IRQ_for_gameplay();

    REG_KEYCNT = KEY_START | 1<<14;
    irqSet(IRQ_KEYS, &ISR_key_Start);
    irqEnable(IRQ_KEYS);
}

//Disable the Vblank and keys interrupts.
void disable_IRQ_for_gameplay(){
    irqDisable(IRQ_VBLANK);

    REG_KEYCNT = 0;
    irqDisable(IRQ_KEYS);
}

//VBlank ISR, update the player and the sprites.
void ISR_VBlank(){
    updatePlayer();
    updateSprites();
}

//A key ISR
void ISR_key_A(){
    //Open the door and start the timer
    //if the player presses A near the button.
    if(isPlayerNear(Button)){
        play_effect(SFX_BUTTON);
        time_left_door_open = TIMER_HZ * TOTAL_TIME_IN_SECONDS;
        TIMER_DATA(0) = TIMER_FREQ_256(TIMER_HZ); 
	    TIMER_CR(0) = TIMER_ENABLE | TIMER_DIV_256 | TIMER_IRQ_REQ;
        irqSet(IRQ_TIMER0, &ISR_timer0);
        irqEnable(IRQ_TIMER0);
        openDoor();
    }
    //If the player is near the Opened door,
    //we deallocate all ressources and go to the next state.
    if(isPlayerNear(DoorMiddleOpen)){
        disable_timer0();
        deallocateRessources();
        go_to_next_state();
    }
}

//Timer 0 ISR,
//Countdown the time left, 
//make a sound every second that passes,
//and disable the Timer once it reaches 0.
void ISR_timer0(){
    time_left_door_open -= 1;
    if(time_left_door_open == 0){
    	disable_timer0();

    }else if(time_left_door_open % TIMER_HZ == 0){
        play_effect(SFX_TICKTOCK);
    }
}

//Disables Timer 0 and closes the door.
void disable_timer0(){
    irqDisable(IRQ_TIMER0);
    TIMER_DATA(0) = 0; 
	TIMER_CR(0) = 0;
	time_left_door_open = 0;
    closeDoor();
}
