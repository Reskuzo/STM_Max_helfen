/*
 * snake.c
 *
 *  Created on: Feb 15, 2026
 *      Author: yannick Pahlke
 */
#include "snake.h"

/* Define global variables */
uint8_t snake_board[SNAKE_BOARD_SIZE] = {0};
uint8_t snake_head_rot = 1;
uint8_t snake_head_row = 7;
uint8_t snake_head_col = 3;
uint16_t snake_tail_end_pos = 7*SNAKE_COLS + 2;
uint8_t snake_score = 0;
int snake_skip_tail_movement = 0;
uint8_t snake_joy_pressed = 5;

static int snake_generate_start_board(){
	/* Clear board */
	for (int entry = 0; entry < SNAKE_BOARD_SIZE; entry++) snake_board[entry] = 0;

	/* Place snake head and tail */
	snake_board[snake_head_pos()] = 2;
	snake_board[snake_tail_end_pos] = 4;

	/* Place first fruit randomly on right side */
	snake_board[(rand() % SNAKE_ROWS) * SNAKE_COLS + (rand() % (SNAKE_COLS/2)) + SNAKE_COLS/2] = 1;
	return 1;
}

void snake_game(){
	{

		/* Initialize random seed */
		srand(HAL_GetTick());

		/* Reset game state */
		snake_head_rot = 1;
		snake_head_row = 7;
		snake_head_col = 3;
		snake_tail_end_pos = 7*SNAKE_COLS + 2;
		snake_score = 0;
		snake_skip_tail_movement = 0;

		snake_generate_start_board();
		uint16_t game_tick_time = 300;

		while(1){
			snake_render_board();

			/* Display score */
			UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
			UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLACK);
			UTIL_LCD_SetFont(&Font24);
			char out_str[16];
			sprintf(out_str, "Score: %d", snake_score);
			UTIL_LCD_DisplayStringAt(500, 10, (uint8_t*)out_str, LEFT_MODE);

  			/* Get input during tick time */
			HAL_Delay(game_tick_time);
			uint8_t input = snake_get_controller_input();
			/* Process input */
			if (input == 4){
				/* Pause on SELECT */
				continue;
			}
			else if(input == 5){
				/* No input - continue moving */
				snake_move_snake();
			}
			else {
				/* Change direction */
				snake_head_rot = input;
				snake_move_snake();
			}
		}
	}

}
