/*
 * state_machiene.c – Page state machine for STM32H735G-DK
 *   No hardware joystick: navigation is via UART remote_input (JOY_* values)
 *   and the single USER button (toggles nav mode).
 */
#include "stm32h735g_discovery.h"
#include "state_machine.h"
#include "../Display/display_main.h"

#include "../Pages/wavefront_page.h"
#include "../Pages/fourier_page.h"
#include "../Pages/noise_filter_page.h"

uint8_t navigation_mode = 0;
uint8_t game_state      = 0;
uint8_t state           = 0;

void blank(void) { return; }

state_t pages[] = {
    { "Wavefront",     wavefront_init,          wavefront_onTick,           raise_inc,                lower_inc,                 raise_noise_threshold,       lower_noise_threshold,        toggle_thres_vol,          "+inc", "-inc", "+threshold", "-threshold", "thr/vol" },
    { "Fourier",       fourier_init,             fourier_onTick,             fourier_cursor_left,      fourier_cursor_right,      fourier_toggle_hold,         blank,                        fourier_toggle_scale,      "-frq", "+frq", "hold",       "",           "log/lin" },
    { "Noise Filters", noise_filter_page_init,   noise_filter_page_onTick,   blank,                    blank,                     noise_filter_page_cursor_up, noise_filter_page_cursor_down, noise_filter_page_toggle, "",     "",     "up",         "down",       "toggle" }
};
uint8_t num_pages = 3;

game_t games[] = {
    {"snake", snake_game, snake_joy_input}
};

void onProgramStart(void)
{
    pages[state].onInit();
    display_toolbar(pages[state].textJoyLeft, pages[state].textJoyRight,
                    pages[state].textJoyUp, pages[state].textJoyDown,
                    pages[state].textJoyCenter);
}

void joy_forwarding(uint32_t joypin)
{
    if (game_state > 0) {
        games[game_state - 1].joy_input_forewarding(joypin);
        return;
    }

    if (navigation_mode) {
        if (joypin == JOY_LEFT || joypin == JOY_DOWN) {
            if (state > 0) state--;
            pages[state].onInit();
        } else if (joypin == JOY_RIGHT || joypin == JOY_UP) {
            if (state < num_pages - 1) state++;
            pages[state].onInit();
        } else {
            game_state++;
        }
        display_navigation(pages[state].name,
                           state > 0           ? pages[state - 1].name : "        ",
                           state < num_pages-1 ? pages[state + 1].name : "        ");
    } else {
        if      (joypin == JOY_LEFT)  pages[state].onJoyLeft();
        else if (joypin == JOY_RIGHT) pages[state].onJoyRight();
        else if (joypin == JOY_UP)    pages[state].onJoyUp();
        else if (joypin == JOY_DOWN)  pages[state].onJoyDown();
        else if (joypin == JOY_SEL)   pages[state].onJoyCenter();
    }
}

void toggle_nav_mode(void)
{
    navigation_mode = !navigation_mode;
    if (navigation_mode)
        display_navigation(pages[state].name,
                           state > 0           ? pages[state - 1].name : "        ",
                           state < num_pages-1 ? pages[state + 1].name : "        ");
    else
        display_toolbar(pages[state].textJoyLeft, pages[state].textJoyRight,
                        pages[state].textJoyUp, pages[state].textJoyDown,
                        pages[state].textJoyCenter);
}

void onTick(void)
{
    if (game_state > 0) {
        games[game_state - 1].start_func();
        game_state = 0;
        pages[state].onInit();
    }
    pages[state].onTick();
}
