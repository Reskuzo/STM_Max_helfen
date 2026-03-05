/*
 * snake_adapted.h
 * Adapted for STM32H735G-DK board (480x272)
 *
 *  Created on: Apr 24, 2024
 *      Author: yannick pahlke
 *  Adapted: Feb 14, 2026
 */

#ifndef SNAKE_ADAPTED_H_
#define SNAKE_ADAPTED_H_

#include "main.h"
#include "stm32h735g_discovery_lcd.h"
#include "stdlib.h"
#include "time.h"
#include <stdio.h>

/* Display constants */
#define SNAKE_CELL_SIZE   30
#define SNAKE_COLS        16   /* 480 / 30 */
#define SNAKE_ROWS         9   /* 272 / 30 */
#define SNAKE_BOARD_SIZE  (SNAKE_COLS * SNAKE_ROWS)  /* 416 */
#define SNAKE_OFFSET_X    0
#define SNAKE_OFFSET_Y    0

/* Game board */
extern uint8_t snake_board[SNAKE_BOARD_SIZE];
/*
 * 0 = empty
 * 1 = fruit
 * 2 = head
 * 3 = tail to top
 * 4 = tail to right
 * 5 = tail to bottom
 * 6 = tail to left
 * */

extern uint8_t snake_head_rot;
/*
 * 0 UP
 * 1 RIGHT
 * 2 DOWN
 * 3 LEFT
 * */
extern uint8_t snake_head_row;
extern uint8_t snake_head_col;
extern uint16_t snake_tail_end_pos;
extern uint8_t snake_score;
extern int snake_skip_tail_movement;
extern uint8_t snake_joy_pressed;






/**
 * Main method and entry Point for the snake game
 */
void snake_game();

static inline uint16_t snake_head_pos(){
	return snake_head_row * SNAKE_COLS + snake_head_col;
}

static inline int snake_render_board(){
	UTIL_LCD_Clear(UTIL_LCD_COLOR_BLACK);
	
	for (uint8_t row = 0; row < SNAKE_ROWS; row++){
		for (uint8_t col = 0; col < SNAKE_COLS; col++){
			uint8_t cell = snake_board[row*SNAKE_COLS+col];
			if (cell == 0) continue;
			
			uint16_t x = SNAKE_OFFSET_X + col * SNAKE_CELL_SIZE;
			uint16_t y = SNAKE_OFFSET_Y + row * SNAKE_CELL_SIZE;
			
			if (cell == 1) {
				/* Fruit - red circle */
				UTIL_LCD_FillCircle(x + SNAKE_CELL_SIZE/2, y + SNAKE_CELL_SIZE/2, SNAKE_CELL_SIZE/2 - 2, UTIL_LCD_COLOR_RED);
			}
			else if (cell == 2){
				/* Head - light green square */
				UTIL_LCD_FillRect(x, y, SNAKE_CELL_SIZE, SNAKE_CELL_SIZE, UTIL_LCD_COLOR_LIGHTGREEN);
			}
			else{
				/* Body - dark green square */
				UTIL_LCD_FillRect(x, y, SNAKE_CELL_SIZE, SNAKE_CELL_SIZE, UTIL_LCD_COLOR_DARKGREEN);
			}
		}
	}
	return 1;
}

static inline int snake_you_died(){
	UTIL_LCD_Clear(UTIL_LCD_COLOR_WHITE);
	UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_RED);
	UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_WHITE);
	UTIL_LCD_SetFont(&Font24);

	UTIL_LCD_DisplayStringAt(0, 200, (uint8_t*)"GAME OVER!", CENTER_MODE);
	char out_str[32];
	sprintf(out_str, "Score: %d", snake_score);
	UTIL_LCD_DisplayStringAt(0, 230, (uint8_t*)out_str, CENTER_MODE);
	
	/* Wait for joystick press to restart */
	HAL_Delay(2000);
	while (1){
		HAL_Delay(100);
		/* Check for any button press to exit */
	}
	return 1;
}

static inline int snake_move_snake(){
	snake_board[snake_head_pos()] = snake_head_rot + 3;
	
	/* Move head */
	if (snake_head_rot == 0 && snake_head_row > 0) snake_head_row -= 1;
	else if (snake_head_rot == 1 && snake_head_col < SNAKE_COLS - 1) snake_head_col += 1;
	else if (snake_head_rot == 2 && snake_head_row < SNAKE_ROWS - 1) snake_head_row += 1;
	else if (snake_head_rot == 3 && snake_head_col > 0) snake_head_col -= 1;
	else snake_you_died();

	/* Move tail */
	int tail_pos_change = 0;
	switch (snake_board[snake_tail_end_pos]-3) {
	case 0:
		tail_pos_change = -SNAKE_COLS;
		break;
	case 1:
		tail_pos_change = 1;
		break;
	case 2:
		tail_pos_change = SNAKE_COLS;
		break;
	default:
		tail_pos_change = -1;
		break;
	}
	
	if (snake_skip_tail_movement) 
		snake_skip_tail_movement = 0;
	else{
		snake_board[snake_tail_end_pos] = 0;
		snake_tail_end_pos += tail_pos_change;
	}

	/* Check collision */
	switch(snake_board[snake_head_pos()]){
	case 0:
		break;
	case 1:
	{
		/* Ate fruit */
		snake_score++;
		snake_skip_tail_movement = 1;
		uint16_t new_fruit_pos = rand() % SNAKE_BOARD_SIZE;
		while(snake_board[new_fruit_pos] != 0) new_fruit_pos = rand() % SNAKE_BOARD_SIZE;
		snake_board[new_fruit_pos] = 1;
		break;
	}
	default:
		/* Hit self */
		snake_you_died();
		break;
	}
	
	snake_board[snake_head_pos()] = 2;
	return 1;
}

static inline uint8_t snake_get_controller_input(){
	uint32_t joy_dir = snake_joy_pressed;
	snake_joy_pressed = 5;
	
	return joy_dir;
}

static inline void snake_joy_input(uint32_t joy_state){
	if (joy_state == JOY_UP) snake_joy_pressed = 0;
	if (joy_state == JOY_RIGHT) snake_joy_pressed = 1;
	if (joy_state == JOY_DOWN) snake_joy_pressed = 2;
	if (joy_state == JOY_LEFT) snake_joy_pressed = 3;
	if (joy_state == JOY_SEL) snake_joy_pressed = 4;
}


#endif /* SNAKE_ADAPTED_H_ */
