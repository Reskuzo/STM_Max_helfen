/*
 * wavefront_page.h
 *
 *  Created on: Feb 20, 2026
 *      Author: yannick
 */

#ifndef SRC_PAGES_WAVEFRONT_PAGE_H_
#define SRC_PAGES_WAVEFRONT_PAGE_H_

#include <stdint.h>

extern uint16_t noise_threshold;
extern int threashold_raise_increase;


/*
 * @brief Should be run once before the first tick. (Re)Sets the display theme and starts audio recording */
void wavefront_init();

/*
 * @brief run this every tick to display the last 512 16b audio Intensities as a wavefront
 * */
void wavefront_onTick();

void raise_noise_threshold();
void lower_noise_threshold();
void toggle_thres_vol();
void raise_inc();
void lower_inc();


#endif /* SRC_PAGES_WAVEFRONT_PAGE_H_ */
