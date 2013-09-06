#include <string.h>
#include <stdio.h>
#include <allegro.h>

#include "main.h"

/* for the starfield */
#define MAX_STARS       128

volatile struct {
	fixed x, y, z;
	int ox, oy;
} star[MAX_STARS];

int star_count = 0;
int star_count_count = 0;

MIDI *mid;

void demo_1(void)
{
	int c;
	
	play_midi((MIDI *)dat[ALLEGRO_MID].dat, 1);
	
	for (c = 0; c < MAX_STARS; c++) {
		star[c].z = 0;
		star[c].ox = star[c].oy = -1;
	}	
}

void demo_2(void)
{
	/* for the starfield */
	fixed x, y;
	int ix, iy;
	int c;
	
/* animate the starfield */
	for (c = 0; c < star_count; c++) {
		if (star[c].z <= itofix(1)) {
			x = itofix(rand() & 0xff);
			y = itofix(((rand() & 3) + 1) * SCREEN_W);

			star[c].x = fixmul(fixcos(x), y);
			star[c].y = fixmul(fixsin(x), y);
			star[c].z = itofix((rand() & 0x1f) + 0x20);
		}

		x = fixdiv(star[c].x, star[c].z);
		y = fixdiv(star[c].y, star[c].z);

		ix = (int) (x >> 16) + SCREEN_W / 2;
		iy = (int) (y >> 16) + SCREEN_H / 2;

		if ((ix >= 0) && (ix < SCREEN_W) && (iy >= 0)
			 && (iy <= SCREEN_H)) {
			star[c].ox = ix;
			star[c].oy = iy;
			star[c].z -= 4096;
		} else {
			star[c].ox = -1;
			star[c].oy = -1;
			star[c].z = 0;
		}
	}

/* wake up new stars */
	if (star_count < MAX_STARS) {
		if (star_count_count++ >= 32) {
			star_count_count = 0;
			star_count++;
		}
	}
}

void demo_3(void)
{
	int c, c2;
/* draw the starfield */
	for (c = 0; c < star_count; c++) {
		int c3;
		c2 = 7 - (int) (star[c].z >> 18);
		c3 = MID(0, c2, 7);
		if (star[c].z > 0) circlefill(page, star[c].ox, star[c].oy, 1 + c3, makecol(200, 200, 150));
	}

}
