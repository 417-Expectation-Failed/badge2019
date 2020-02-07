/*
 * This file is part of badge2019.
 * Copyright 2019 Niels Kristensen <niels@kristensen.io>
 * Copyright 2019 Emil Renner Berthing <esmil@labitat.dk>
 *
 * badge2019 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * badge2019 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with badge2019. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>

#include "leds.h"
#include "timer.h"
#include "events.h"
#include "buttons.h"
#include "display.h"
#include "menu.h"
#include "power.h"
#include "font.h"

enum events {
	EV_UP = 1,
	EV_DOWN,
	EV_LEFT,
	EV_RIGHT,
	EV_CENTER_PUSH,
	EV_TICK,
	EV_SMID_PUSH,
};

enum direction {
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT,
};

static const struct button_config game_of_life_buttons[BTN_MAX] = {
	[BTN_UP]    = { .press = EV_UP, },
	[BTN_DOWN]  = { .press = EV_DOWN, },
	[BTN_LEFT]  = { .press = EV_LEFT, },
	[BTN_RIGHT] = { .press = EV_RIGHT, },
	[BTN_CENTER]   = { .press = EV_CENTER_PUSH, },
	[BTN_SMID]   = { .press = EV_SMID_PUSH, },
};

void
game_of_life(unsigned int res, uint8_t seed)
{
	unsigned int count = 0;
    uint8_t frametime_ms = 150;
    uint8_t spawn_probability_percent = 11;
    struct ticker tick;
    //const int birth_rules_length = 1;
    #define BIRTH_RULES_LENGTH 1
    int birth_rules[BIRTH_RULES_LENGTH] = {3};
    //const int survival_rules_length = 2;
    #define SURVIVAL_RULES_LENGTH 2
    int survival_rules[SURVIVAL_RULES_LENGTH] = {2, 3};
    const uint8_t display_width  = 240;
    const uint8_t display_height = 240;
    uint8_t amount_tiles = display_width/res;
    int     alive_color = 0xCB0;
    int     dead_color  = 0x000;
    uint8_t grid[amount_tiles][amount_tiles];

	/* '4' guaranteed to be random
	 * -- chosen by fair dice
	 */
	srand(seed);

	/* clear screen */
	dp_fill(0, 0, display_width, display_height, 0x000);

	buttons_config(game_of_life_buttons);

	ticker_start(&tick, 100, EV_TICK);

	// initialize grid
    for(int y=0; y < amount_tiles; y++) {
        for(int x=0; x < amount_tiles; x++) {
            grid[y][x] = (rand()%100+1) < spawn_probability_percent;
            // set color according to state of life
            int color = 0x000;
            if(grid[y][x] != 0) {
                color = 0xCB0;
            }
            dp_fill(x*res, y*res, res, res, color);
        }
    }

    // main loop
	while (1) {
        switch ((enum events)event_get()) {
            case EV_TICK:
                if (power_pressed()) {
                    count += 1;
                    if (count == 3) {
                        led1_off();
                        led2_off();
                        led3_off();
                        ticker_stop(&tick);
                        return;
                    }
                } else
                    count = 0;
                break;
        }
        for(int y = 0; y < amount_tiles; y++) {
            for(int x = 0; x < amount_tiles; x++) {

                int min_range_y = -1;
                int max_range_y = 1;
                int min_range_x = -1;
                int max_range_x = 1;
                if(y == 0)
                    min_range_y = 0;
                if(y == amount_tiles - 1)
                    max_range_y = 0;
                if(x == 0)
                    min_range_x = 0;
                if(x == amount_tiles -1)
                    max_range_x = 0;
                    
                int amount_of_neighbours = 0;
                for(int i = min_range_y; i <= max_range_y; i++) {
                    for(int j = min_range_x; j <= max_range_x; j++) {
                        if((y+i == y && x+j == x)) {
                        } else {
                            if(grid[y+i][x+j] == true) {
                                amount_of_neighbours++;
                            }
                        }
                    }
                }
                if(grid[y][x]) {
                    bool survives = false;
                    for(int i = 0; i < SURVIVAL_RULES_LENGTH; i++) {
                        if(amount_of_neighbours == survival_rules[i]) {
                            survives=true;
                        }
                    }
                    grid[y][x] = survives;
                } else {
                    for(int i = 0; i < BIRTH_RULES_LENGTH; i++) {
                        if(amount_of_neighbours == birth_rules[i]) {
                            grid[y][x] = true;
                        }
                    }
                } 

                // draw
                int color = dead_color;
                if(grid[y][x] != 0) {
                    color = alive_color;
                }
                dp_fill(x*res, y*res, res, res, color);

            }
        }
        // delay
        timer_msleep(frametime_ms);
	}
}

static void game_of_life4(void)  { game_of_life_seed(4); }
static void game_of_life5(void)  { game_of_life_seed(5); }
static void game_of_life6(void)  { game_of_life_seed(6); }
static void game_of_life8(void)  { game_of_life_seed(8); }
static void game_of_life10(void) { game_of_life_seed(10); }

void game_of_life_seed(uint8_t res) {
    dp_clear(0x000);
    dp_puts(5, font.height*2, 0xCB0, 0x000, "press buttons to");
    dp_puts(5, font.height*3, 0xCB0, 0x000, "set seed");
    dp_puts(5, font.height*4+5, 0xCB0, 0x000, "hold power to start");
    struct ticker tick;
    uint8_t power_count = 0;
    uint8_t seed = 0;
    char seed_str_buff[5];
    ticker_start(&tick, 100, EV_TICK);
    while(1) {
        uint8_t latest_event = event_get();
        if((enum events) latest_event != EV_TICK) {
            seed += latest_event;
        }
        switch((enum events)latest_event) {
            case EV_TICK:
                if(power_pressed()) {
                    power_count += 1;
                    if(power_count == 3) {
                        ticker_stop(&tick);
                        dp_clear(0x000);
                        game_of_life(res, seed);
                        return;
                    }
                } else {
                    led1_off();
                }
        }
        sprintf(seed_str_buff, "%d", seed);
        dp_puts(5, font.height * 6, 0xCB0, 0x000, "seed: ");
        dp_puts(5 + font.width*6, font.height * 6, 0xCB0, 0x000, seed_str_buff);
        timer_msleep(100);
    }
    dp_clear(0x000);
}

void
game_of_lifemenu(void)
{
    //dp_puts(0, 0, 0xCB0, 0x000, 'resolution:');
	static const struct menuitem game_of_lifemenu[] = {
		{ .label = "4", .cb = game_of_life4,  },
		{ .label = "5", .cb = game_of_life5,  },
		{ .label = "6", .cb = game_of_life6,  },
		{ .label = "8", .cb = game_of_life8,  },
		{ .label = "10",.cb = game_of_life10, },
	};

	menu(game_of_lifemenu, ARRAY_SIZE(game_of_lifemenu), 0xCB0, 0x000);
}
