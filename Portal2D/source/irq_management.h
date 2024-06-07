#include <nds.h>
#include "graphics_sub_engine.h"

void init_IRQ_for_gameplay();
void init_IRQ_for_startup();
void init_IRQ_for_ending();

void ISR_VBlank();
void ISR_key_A();
void ISR_timer0();
void ISR_key_Start();

void disable_timer0();
void disable_IRQ_for_gameplay();

