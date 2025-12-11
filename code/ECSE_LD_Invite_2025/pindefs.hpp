#ifndef PINDEFS_HPP
#define PINDEFS_HPP

// LED MATRIX COLUMNS ARE POSITIVE, ROWS ARE NEGATIVE
#define LED_C1 25
#define LED_C2 24
#define LED_C3 23
#define LED_C4 22
#define LED_C5 21

#define MASK_ALL_COLS ((1<<LED_C1)|(1<<LED_C2)|(1<<LED_C3)|(1<<LED_C4)|(1<<LED_C5))

#define LED_R1 20
#define LED_R2 19
#define LED_R3 18
#define LED_R4 17
#define LED_R5 16


#define MASK_ALL_ROWS ((1<<LED_R1)|(1<<LED_R2)|(1<<LED_R3)|(1<<LED_R4)|(1<<LED_R5))

#define PB1 15
#define PB2 14

#endif