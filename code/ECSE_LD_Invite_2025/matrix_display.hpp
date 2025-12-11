#ifndef MATRIX_DISPLAY_HPP
#define MATRIX_DISPLAY_HPP
#include <stdio.h>
#include <pico/stdlib.h>

#include "pindefs.hpp"


extern uint8_t scroll_buff[15];
const uint8_t* char_to_matrix(const char charIn);
void disp_char(const uint8_t * character);
void scroll_chars(void);
void add_char_to_scroll(const uint8_t * character);
void add_char_to_scroll_start(const uint8_t * character);
void print_matrix(const uint8_t * character);
void print_print_buff(void);
void push_display(const uint8_t character[5], uint8_t brightness);

#endif