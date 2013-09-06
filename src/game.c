#include <allegro.h>
#include <math.h>

#include "main.h"
#include "game.h"
#include "gfx.h"

#define pi 3.141592654

BITMAP *map;

int go;
float ballx, bally, ballz, balldx, balldy, balldz;
float hills[16][16];

#define ANIMS 5
DATAFILE *animdata[ANIMS];
int anim[ANIMS], animlen[ANIMS];
int animx[ANIMS], animy[ANIMS];
BITMAP *tower;
BITMAP *nuclearegg;
BITMAP *launcher;
SAMPLE *explosion;
SAMPLE *radiating;
SAMPLE *teleport;
SAMPLE *bonus;

float power[2];
float basex[2], basey[2];
int life[2];

BITMAP *bonuspic[3];
#define MAXBONUS 10
float bonusx[MAXBONUS], bonusy[MAXBONUS];
int bonustype[MAXBONUS];
int bonusnum;

int turn;
int ignore[2];
float turndx[2], turndy[2];
int AI[2];
int think[2];
int memorized[2];
float mindist[2];
float memdist[2][3];
float mindir[2][3];
float scan[2];

#define ZZZ 16
#define TW 29
#define TH 29
#define MW 464
#define MH 464
#define WINL ((SCREEN_W - MW) / 2)
#define WINT (SCREEN_H - MH)

float get_height(int x, int y)
{
	float h, h1, h2, h3, h4;
	int mx = x / TW;
	int my = y / TH;
	float hx = (x - mx * TW) / (float)TW;
	float hy = (y - my * TH) / (float)TH;
	mx &= 15;
	my &= 15;
	h1 = hills[my][mx];
	h2 = hills[(my + 1) & 15][mx];
	h3 = hills[(my + 1) & 15][(mx + 1) & 15];
	h4 = hills[my][(mx + 1) & 15];
	h1 = h1 * (1 - hx) * (1 - hy);
	h2 = h2 * (1 - hx) * (hy);
	h3 = h3 * (hx) * (hy);
	h4 = h4 * (hx) * (1 - hy);
	h = h1 + h2 + h3 + h4;
	return h;
}

int dat_length(DATAFILE *dat)
{
	int c;
	for (c = 0; dat[c].type != DAT_END; c++);
	
	return c;
}

#define M(x) (x < 0 ? 0 : x > 255 ? 255 : x)

void draw_back(BITMAP *bmp)
{
	int x, y;
	float h[16][21];
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 21; x++) {
			h[y][x] = rnd();
		}
	}
	for (y = 0; y < bmp->h; y++) {
		for (x = 0; x < bmp->w; x++) {
			int my = y / 32;
			int mx = x / 32;
			float hx = (x - mx * 32) / 32.0;
			float hy = (y - my * 32) / 32.0;
			float h1 = h[my + 0][mx + 0];
			float h2 = h[my + 1][mx + 0];
			float h3 = h[my + 1][mx + 1];
			float h4 = h[my + 0][mx + 1];
			float a;
			int r, g, b;
			h1 = h1 * (1 - hx) * (1 - hy);
			h2 = h2 * (1 - hx) * (hy);
			h3 = h3 * (hx) * (hy);
			h4 = h4 * (hx) * (1 - hy);
			a = h1 + h2 + h3 + h4;
			r = 200 * a;
			g = 150 * a;
			b = 50 * a;
			putpixel(bmp, x, y, makecol32(M(r), M(g), M(b)));
		}
	}
}

void game_end(void)
{
	stop_sample(radiating);
}

void game_init(int ai)
{
	static int once = 1;
	int x, y;
	BITMAP *tmp;
	int bc = makecol(200, 150, 50);
	
	clear_to_color(screen, makecol(0, 0, 0));
	textout_centre(screen, font, "Generating&Caching Data", SCREEN_W / 2, 0, -1);
	textout_centre(screen, font, "May take a while", SCREEN_W / 2, SCREEN_H / 2, -1);
	textout_centre(screen, font, "(about 10 minutes for first run,", SCREEN_W / 2, SCREEN_H / 2 + 20, -1);
	textout_centre(screen, font, "about 1 minute every first game,", SCREEN_W / 2, SCREEN_H / 2 + 40, -1);
	textout_centre(screen, font, "few seconds else)", SCREEN_W / 2, SCREEN_H / 2 + 60, -1);
		
	if (once) {
		once = 0;
		
		animdata[0] = load_gfx("gravburn", 48);
		animdata[1] = load_gfx("shock", 48);
		animdata[2] = load_gfx("fireball", 48);
		animdata[3] = load_gfx("wheel", 48);
		animdata[4] = load_gfx("glow", 32);
		{
			int a;
			for (a = 0; a < ANIMS; a++) {
				anim[a] = 0;
				animlen[a] = dat_length(animdata[a]);
			}
		}
		
		tower = dat[TOWER_BMP].dat;
		explosion = dat[EXPLOSION_WAV].dat;
		radiating =  dat[RADIATING_WAV].dat;
		teleport =  dat[TELEPORT_WAV].dat;
		bonus =  dat[BONUS_WAV].dat;
		nuclearegg =  dat[NUCLEAREGG_BMP].dat;
		launcher =  dat[LAUNCHER_BMP].dat;
	
		bonuspic[0] = dat[BONUS0_BMP].dat;
		bonuspic[1] = dat[BONUS1_BMP].dat;
		bonuspic[2] = dat[BONUS2_BMP].dat;
	
		map = create_bitmap(SCREEN_W, SCREEN_H);
	}

	ignore[0] = 0;
	ignore[1] = 0;	
	think[0] = 0;
	think[1] = 0;
	memorized[0] = 0;
	memorized[1] = 0;
	AI[0] = 0;
	if (ai) AI[1] = 1; else AI[1] = 0;
	power[0] = 0;
	power[1] = 0;
	bonusnum = 0;
	life[0] = 100;
	life[1] = 100;
	go = 0;
	
	turn = 0;
	
	tmp = create_bitmap_ex(32, SCREEN_W, SCREEN_H);
	draw_back(tmp);
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			hills[y][x] = rnd();
		}
	}

	basex[0] = 4 * TW + TW / 2;
	basey[0] = 4 * TH + TH / 2;

	basex[1] = 12 * TW + TW / 2;
	basey[1] = 12 * TH + TH / 2;

	hills[4][4] = 1;
	hills[5][4] = 1;
	hills[5][5] = 1;
	hills[4][5] = 1;

	hills[12][12] = 1;
	hills[13][12] = 1;
	hills[13][13] = 1;
	hills[12][13] = 1;

	x = rnd() * 16;
	y = rnd() * 16;

	for (y = 0; y < MH; y++) {
		for (x = 0; x < MW; x++) {
			int c = 0;
			int t;
			float height = get_height(x, y);
			float h = 0, z = height * ZZZ;
			for (h = height, t = 0; z > 0 && t < 2; z--, h -= 1.0 / (float)ZZZ, t++) {
				int r, g, b;
				float l;
				if (h < 0.2) {
					float p = h / 0.2;
					r = 0;
					g = 100 * p;
					b = 155 + 100 * p;
				} else if (h < 0.5) {
					float p = (h - 0.2) / 0.3;
					r = 0;
					g = 100 + 155 * p;
					b = 255 - 155 * p;
				} else if (h < 0.8) {
					float p = (h - 0.5) / 0.3;
					r = 200 * p;
					g = 255 - 155 * p;
					b = 100;
				} else {
					float p = (h - 0.8) / 0.2;
					r = 200;
					g = 100 + 100 * p;
					b = 100 + 100 * p;
				}
				l = get_height(x + 1, y) - get_height(x, y + 1);
				r = r + l * 2000 + (rnd() - 0.5) * 20;
				g = g + l * 2000 + (rnd() - 0.5) * 20;
				b = b + l * 2000 + (rnd() - 0.5) * 20;
				c = makecol32(M(r), M(g), M(b));
				if (y == MH - 1 && h < height) c = bc;
				putpixel(tmp, 88 + x, y + WINT - z, c);
			}
		}
	}
	{
		float filter[3][3] = {
			{0.025, 0.145, 0.025},
			{0.145, 0.320, 0.145},
			{0.025, 0.145, 0.025}
		};
		for (y = 0; y < SCREEN_H; y++) {
			for (x = 0; x < SCREEN_W; x++) {
				int i = 0, j = 0;
				float r = 0, g = 0, b = 0;
				for (i = -1; i <= 1; i++) {
					for (j = -1; j <= 1; j++) {
						int c = getpixel(tmp, x + j, y + i);
						if (c < 0) c = makecol32(0, 0, 0);
						r += getr32(c) * filter[1 + i][1 + j];
						g += getg32(c) * filter[1 + i][1 + j];
						b += getb32(c) * filter[1 + i][1 + j];
					}
				}
				putpixel(map, x, y, makecol(r, g, b));
			}
		}
	}
	destroy_bitmap(tmp);
	ballx = 240;
	bally = 240;
	balldx = 0;
	balldy = 0;
}

void game_process()
{	
	static float lastdir, dir, bladir[2] = {0, 0};
	int xm, ym;
	int shoot = 0;
	get_mouse_mickeys(&xm ,&ym);
			
	if (!go) {	
		if (AI[turn] == 0) {
			float turnlen;
			turndx[turn] += xm / 100.0;
			turndy[turn] += ym / 100.0;
			if (turndx[turn] == 0 && turndy[turn] == 0) turndx[turn] = 1;
			turnlen = sqrt(turndx[turn] * turndx[turn] + turndy[turn] * turndy[turn]);
			
			turndx[turn] = turndx[turn] / turnlen;
			turndy[turn] = turndy[turn] / turnlen;
			
			if (mouse_b & 1) shoot = 1;
		} else {
			if (!think[turn]) {
				
				int min;
				lastdir = bladir[turn];
				dir = 0;
				
				if (!ignore[turn]) {
					switch (memorized[turn]) {
						case 0:
							scan[turn] = pi * 2.0 / 3.0;
							dir = mindir[turn][0] = pi * 2 * rnd();
							memorized[turn] = 1;
						break;
						case 1:
							memdist[turn][0] = mindist[turn];
							dir = mindir[turn][1] = mindir[turn][0] + scan[turn];
							memorized[turn] = 2;
						break;
						case 2:
							memdist[turn][1] = mindist[turn];
							dir = mindir[turn][2] = mindir[turn][0] - scan[turn];
							memorized[turn] = 3;
						break;
						case 3:
							memdist[turn][2] = mindist[turn];
							
							if (memdist[turn][0] < memdist[turn][1]) {
								if (memdist[turn][0] < memdist[turn][2]) {
									min = 0;
								} else {
									min = 2;
								}
							} else {
								if (memdist[turn][1] < memdist[turn][2]) {
									min = 1;
								} else {
									min = 2;
								}
							}
							
							memdist[turn][0] = memdist[turn][min];
							mindir[turn][0] = mindir[turn][min];
							
							scan[turn] *= 0.5;
							
							dir = mindir[turn][1] = mindir[turn][0] + scan[turn];
							memorized[turn] = 2;
							
							if (scan[turn] < 0.0003) {
								scan[turn] = pi * 2.0 / 3.0;
								dir = mindir[turn][0] = pi * 2 * rnd();
								memorized[turn] = 1;
							}
							
						break;
					}
					bladir[turn] = dir;
				} else {
					dir = bladir[turn];
					ignore[turn] = 0;
				}
				
				if (power[turn]) {
					int r = (rand() >> 8) & 3;
					dir = r * pi / 2 + pi / 4;
					ignore[turn] = 1;
				} else {
					int r = (rand() >> 8) & 3;
					if (r == 0) {
						int b;
						float minbon = 1000 * 1000;
						
						for (b = 0; b < bonusnum; b++) {
							if (bonustype[b] == 0 || (bonustype[b] == 1 && life[turn] < 100)) {
								float dx, dy;
								float dis;
								dx = bonusx[b] - basex[turn];
								dy = bonusy[b] - basey[turn];
								if (dx < -MW / 2) dx += MW;
								if (dy < -MH / 2) dy += MH;
								if (dx > MW / 2) dx -= MW;
								if (dx > MH / 2) dy -= MH;
								dis = dx * dx + dy * dy;
								if (dis < minbon) {
									minbon = dis;
									dir = atan2(dy, dx);
									ignore[turn] = 1;
								}
							}
						}
					}
				}
				
				mindist[turn] = 10000;
				
				think[turn] = FPS + FPS * rnd();
			} else {
				float t;
				think[turn]--;
				t = (think[turn] - 1) / (float)FPS;
				turndx[turn] = cos(dir * (1 - t) + lastdir * t);
				turndy[turn] = sin(dir * (1 - t) + lastdir * t);
				if (think[turn] == 1) shoot = 1;
			}
		}
	}
	
	if (!go && shoot) {
		
		think[turn] = 0;
		
		ballx = basex[turn] + 15 * turndx[turn];
		bally = basey[turn] + 15 * turndy[turn];
		
		balldx = (2 + power[turn]) * turndx[turn];
		balldy = (2 + power[turn]) * turndy[turn];
		power[turn] = 0;
		play_sample(radiating, 100, 128, 1000, 1);
		go = 1;
	}
	
	if (go && life[turn] > 0) {
		int explode = 0;
		float h1 = get_height(ballx, bally);
		float h2 = get_height(ballx, bally + 1);
		float h4 = get_height(ballx + 1, bally);
		
		float r[2], rx, ry;
		float s;
	
		balldx *= 0.997;
		balldy *= 0.997;
	
		balldx += h1 - h4;
		balldy += h1 - h2;
	
		ballx += balldx;
		bally += balldy;
				
		{
			int t;
			for (t = 0; t < 2; t++) {
				rx = basex[t] - ballx;
				ry = basey[t] - bally;
				r[t] = rx * rx + ry * ry;
				
				if (t != turn) {
					if (r[t] < mindist[turn] * mindist[turn]) {
						mindist[turn] = sqrt(r[t]);
					}
				}
				
				if (r[t] < 15 * 15) {
					explode = 1;
					balldx = 0;
					balldy = 0;
					life[t] -= 30;
					if (life[t] < 0) life[t] = 0;
				}
			}
		}
		
		if (ballx < 0) {ballx += MW; play_sample(teleport, 200, 128, 1000, 0);}
		if (bally < 0) {bally += MH; play_sample(teleport, 200, 128, 1000, 0);}
		if (ballx >= MW) {ballx -= MW; play_sample(teleport, 200, 128, 1000, 0);}
		if (bally >= MH) {bally -= MH; play_sample(teleport, 200, 128, 1000, 0);}
		
		{
			int b;
			for (b = 0; b < bonusnum; b++) {
				float q;
				rx = bonusx[b] - ballx;
				ry = bonusy[b] - bally;
				q = rx * rx + ry * ry;
				if (q < 15 * 15) {
					play_sample(bonus, 200, 128, 1000, 0);
					
					if (bonustype[b] == 0) {
						power[turn] += 2;
						anim[1] = 1;
						animx[1] = WINL + ballx - 64;
						animy[1] = WINT + bally - get_height(ballx, bally) * ZZZ - 64;
					}
					if (bonustype[b] == 1) {
						life[turn] += 20;
						if (life[turn] > 100) life[turn] = 100;
						anim[3] = 1;
						animx[3] = WINL + ballx - 64;
						animy[3] = WINT + bally - get_height(ballx, bally) * ZZZ - 64;
					}
					if (bonustype[b] == 2) {
						life[turn] -= 20;
						if (life[turn] < 1) life[turn] = 1;
						anim[2] = 1;
						animx[2] = WINL + ballx - 64;
						animy[2] = WINT + bally - get_height(ballx, bally) * ZZZ - 64;
					}
					bonusnum--;
					bonusx[b] = bonusx[bonusnum];
					bonusy[b] = bonusy[bonusnum];
					bonustype[b] = bonustype[bonusnum];
					if (bonusnum > 0) continue;
				}
			}
		}
		
		s = sqrt(balldx * balldx + balldy * balldy);
		
		adjust_sample(radiating, 100, 128, 400 + 600 * s, 1);
		
		if (s < 0.1) {
			anim[0] = 1;
			animx[0] = WINL + ballx - 64;
			animy[0] = WINT + bally - h1 * ZZZ - 64;
			stop_sample(radiating);
			play_sample(explosion, 200, 128, 1000, 0);
			
			if (!explode) {
				int t;
				for (t = 0; t < 2; t++) {
					if (r[t] < 30 * 30) {
						life[t] -= (900 - r[t]) / 90;
						if (life[t] < 0) life[t] = 0;
					}
				}
			}
			
			turn = !turn;
			
			if (life[0] > 0 && life[1] > 0) go = 0;
			
			if (bonusnum < MAXBONUS) {
				int rx = rnd() * MW;
				int ry = rnd() * MH;
				float rt = rnd();
				bonusx[bonusnum] = rx;
				bonusy[bonusnum] = ry;
				bonustype[bonusnum] = 0;
				if (rt < 0.2) bonustype[bonusnum] = 1;
				if (rt > 0.8) bonustype[bonusnum] = 2;
				bonusnum++;
			}
			
		}
		
	}
	
	{
		int a;
		for (a = 0; a < ANIMS; a++) {
			if (anim[a]) {
				anim[a]++;
				if (anim[a] > animlen[a]) anim[a] = 0;
			}
		}
	}
}

void game_render()
{
	blit(map, page, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	
	if (go && life[0] > 0 && life[1] > 0) {
		int x = ballx + WINL;
		int y = bally + WINT - get_height(ballx, bally) * ZZZ;
		draw_sprite(page, nuclearegg, x - 7, y - 7);
		draw_trans_sprite(page, animdata[4][frames & 31].dat, x - 64, y - 64);
	} else {
		
		pivot_sprite(page, launcher, WINL + basex[turn], WINT + basey[turn] - ZZZ,
			7, 30, ftofix(atan2(turndx[turn], -turndy[turn]) * 128.0 / 3.1415));
		
	}
	
	{
		int b;
		for (b = 0; b < bonusnum; b++) {
			draw_sprite(page, bonuspic[bonustype[b]], WINL + bonusx[b] - 7,
				WINT + bonusy[b] - get_height(bonusx[b], bonusy[b]) * ZZZ - 7);
		}
	}

	if (life[0] > 0) draw_sprite(page, tower, WINL + 4 * TW - 5, WINT + 4 * TH - 50);
	else textout_centre(page, font, "You Win!", WINL + basex[1], WINT + basey[1], -1);
	if (life[1] > 0) draw_sprite(page, tower, WINL + 12 * TW - 5, WINT + 12 * TH - 50);
	else textout_centre(page, font, "You Win!", WINL + basex[0], WINT + basey[0], -1);

	{
		int t;
		int X[2] = {0, SCREEN_W - 80};
		for (t = 0; t < 2; t++) {
			
			if (power[t] > 0) draw_sprite(page, bonuspic[0], X[t] + 15, 15);
			if (power[t] > 2) draw_sprite(page, bonuspic[0], X[t] + 35, 15);
			if (power[t] > 4) draw_sprite(page, bonuspic[0], X[t] + 55, 15);

			rectfill(page, X[t] + 30, SCREEN_H - 95 - 100 * 3,
				X[t] + 30 + 16, SCREEN_H - 95 - life[t] * 3, makecol(255, 0, 0));
			rectfill(page, X[t] + 30, SCREEN_H - 95,
				X[t] + 30 + 16, SCREEN_H - 95 - life[t] * 3, makecol(0, 155, 0));
				
		}
	}

	{
		int a;
		for (a = 0; a < ANIMS; a++) {
			if (anim[a]) {
				draw_trans_sprite(page, animdata[a][anim[a] - 1].dat, animx[a], animy[a]);
			}
		}
	}
	
}
