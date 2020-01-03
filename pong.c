/*
 * This file is part of badge2019.
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

#include "geckonator/gpio.h"

#include "leds.h"
#include "timer.h"
#include "events.h"
#include "buttons.h"
#include "display.h"
#include "power.h"

enum events {
	EV_SUP_PUSH = 1,
	EV_SUP_RELEASE,
	EV_SMID_PUSH,
	EV_SMID_RELEASE,
	EV_SDOWN_PUSH,
	EV_SDOWN_RELEASE,
	EV_UP_PUSH,
	EV_UP_RELEASE,
	EV_DOWN_PUSH,
	EV_DOWN_RELEASE,
	EV_LEFT_PUSH,
	EV_LEFT_RELEASE,
	EV_RIGHT_PUSH,
	EV_RIGHT_RELEASE,
	EV_CENTER_PUSH,
	EV_CENTER_RELEASE,
	EV_TICK,
};

static const struct button_config testbuttons[BTN_MAX] = {
	[BTN_SUP]    = { .press = EV_SUP_PUSH,    .release = EV_SUP_RELEASE,    },
	[BTN_SMID]   = { .press = EV_SMID_PUSH,   .release = EV_SMID_RELEASE,   },
	[BTN_SDOWN]  = { .press = EV_SDOWN_PUSH,  .release = EV_SDOWN_RELEASE,  },
	[BTN_UP]     = { .press = EV_UP_PUSH,     .release = EV_UP_RELEASE,     },
	[BTN_DOWN]   = { .press = EV_DOWN_PUSH,   .release = EV_DOWN_RELEASE,   },
	[BTN_LEFT]   = { .press = EV_LEFT_PUSH,   .release = EV_LEFT_RELEASE,   },
	[BTN_RIGHT]  = { .press = EV_RIGHT_PUSH,  .release = EV_RIGHT_RELEASE,  },
	[BTN_CENTER] = { .press = EV_CENTER_PUSH, .release = EV_CENTER_RELEASE, },
};

void
pongmenu(void)
{
	struct ticker tick250;
	unsigned int count = 0;

	dp_fill(0, 0, 240, 240, 0x000);
	buttons_config(testbuttons);

	ticker_start(&tick250, 250, EV_TICK);


	uint8_t ndir=0; //player direction, saving position from buttonpress 1 is left, 2 is right
	uint8_t res = 10; //resolution of ball and platform
	int pcurrentx = 10 * res; //player startingposition
	dp_fill(pcurrentx, 195, res*5, res, 0xF00);
	int maxX = 240 - (6*res);
	uint8_t bxdir=0; //0=left, 1 = right
	uint8_t bydir=0; //0=down, 1 = up
	int bcurrentx = 100; //current ball x pos
	int bcurrenty = 20; //current ball ypos
	int blastx = 10; //current ball x pos
	int blasty = 10; //current ball ypos

	//Drawing border
	//dp_fill(0,         0, 240,         res, 0xCB0); //top border
	//dp_fill(0,   240-res, 240,         res, 0xCB0);
	dp_fill(0,       5, 5, 240 - 2*5, 0xCB0);
	dp_fill(240-5, 5, 5, 240 - 2*5, 0xCB0); //right border

	while (1) {
		//ndir = 0;
		switch ((enum events)event_wait()) {
		case EV_SUP_PUSH:
			led1_on();
			dp_fill(5, 195, 10, 10, 0xF00);
			break;
		case EV_SUP_RELEASE:
			led1_off();
			dp_fill(5, 195, 10, 10, 0x000);
			break;
		case EV_SMID_PUSH:
			led2_on();
			dp_fill(5, 210, 10, 10, 0x0F0);
			break;
		case EV_SMID_RELEASE:
			led2_off();
			dp_fill(5, 210, 10, 10, 0x000);
			break;
		case EV_SDOWN_PUSH:
			led3_on();
			dp_fill(5, 225, 10, 10, 0x00F);
			break;
		case EV_SDOWN_RELEASE:
			led3_off();
			dp_fill(5, 225, 10, 10, 0x000);
			break;
		case EV_UP_PUSH:
			dp_fill(115, 190, 10, 10, 0xCB0);
			break;
		case EV_UP_RELEASE:
			dp_fill(115, 190, 10, 10, 0x000);
			break;
		case EV_DOWN_PUSH:
			dp_fill(115, 220, 10, 10, 0xCB0);
			break;
		case EV_DOWN_RELEASE:
			dp_fill(115, 220, 10, 10, 0x000);
			break;
		case EV_LEFT_PUSH:
			ndir=1;
			dp_fill(105, 205, 10, 10, 0xCB0);
			break;
		case EV_LEFT_RELEASE:
			ndir=0;
			dp_fill(105, 205, 10, 10, 0x000);
			break;
		case EV_RIGHT_PUSH:
			dp_fill(125, 205, 10, 10, 0xCB0);
			ndir=2;
			break;
		case EV_RIGHT_RELEASE:
			ndir=0;
			dp_fill(125, 205, 10, 10, 0x000);
			break;
		case EV_CENTER_PUSH:
			dp_fill(115, 205, 10, 10, 0x00F);
			break;
		case EV_CENTER_RELEASE:
			dp_fill(115, 205, 10, 10, 0x000);
			break;
		case EV_TICK:
			if (power_pressed()) {
				count += 1;
				if (count == 3) {
					led1_off();
					led2_off();
					led3_off();
					ticker_stop(&tick250);
					return;
				}
			} else
				count = 0;
			break;
		}
		/* Drawing and calculating the player */
		if (ndir !=0){
			if(ndir==1 && pcurrentx > res){
				pcurrentx = pcurrentx - res;
				dp_fill((pcurrentx+(5*res)), 195, res, res, 0x000);
				dp_fill(pcurrentx, 195, res*5, res, 0xF01);
			}else if (ndir == 2 && pcurrentx < maxX){
				pcurrentx = pcurrentx + res;
				dp_fill((pcurrentx-res), 195, res, res, 0x000);
				dp_fill(pcurrentx, 195, res*5, res, 0xF01);
			}
		}

		/*Drawing and calculating the ball */

		/*uint8_t bxdir = 0; //0=left, 1 = right
		uint8_t bydir = 0; //0=down, 1 = up
		uint8_t bcurrentx = 20; //current ball x pos
		uint8_t bcurrenty = 20; //current ball ypos
		*/
		blastx = bcurrentx;
		blasty = bcurrenty;
		if(bxdir == 0){
			bcurrentx-=res;
		}else {
			bcurrentx+=res;
		}

		if(bydir==0){
			bcurrenty +=res;
		}else{
			bcurrenty -=res;
		}
		if(bcurrentx==(240-(res*2))){ //ball hit right border
			bxdir =0;
		}else if(bcurrentx==res+1){ //ball hit left corner
			bxdir =0;
		}
		dp_fill(blastx, blasty, res, res, 0x000);
		dp_fill(bcurrentx, bcurrenty, res, res, 0x00F);
		timer_msleep(250);

	}
}
