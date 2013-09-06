#include <allegro.h>
#include <string.h>
#include <float.h>
#include <stdio.h>
#include "egg.h"

void make_gfx(char *name, int num)
{
	char filename[1024 * 6];
	char error[80 * 6] = "none";
	BITMAP *pictures[256] = {NULL};
	int n;
	EGG *egg = NULL;
	PACKFILE *file;
	double planes[2];
	
	strcpy(filename, "dat.dat#");
	strcat(filename, name);
	strcat(filename, "_egg");
	egg = load_egg(filename, error);
	if (egg) {
		planes[0] = -DBL_MAX;
		planes[1] = DBL_MAX;
		for (n = 0; n < num; n++) {
			if (update_egg(egg, error) == 0) {
				pictures[n] = create_bitmap_ex(32, 128, 128);
				clear_to_color(pictures[n], makecol32(255, 0, 255));
				lay_egg(egg, &(pictures[n]), 2, planes);
			}
		}
	} else {
		allegro_message("Data corruption detected.\n");
		exit(-1);
	}
	strcpy(filename, name);
	strcat(filename, ".dat");
	file = pack_fopen(filename, "w!");
	if(file) {
		
		int i;
		
		pack_mputl(DAT_MAGIC, file);
		pack_mputl(num, file);
		
		for(i = 0; i < num; i++) {
			PACKFILE *chunk;
			
			pack_mputl(DAT_PROPERTY, file);
			pack_fwrite("NAME", 4, file);			
			{
				char str[10];
				sprintf(str, "%04i", 1 + i);
				pack_mputl(strlen(str), file);
				pack_fwrite(str, strlen(str), file);
			}
			
			pack_mputl(DAT_BITMAP, file);
						
      	chunk = pack_fopen_chunk(file, 1);
      	
      	if(chunk) {      		 		
      		int x, y;
      		pack_mputw(-32, chunk);
      		pack_mputw(pictures[i]->w, chunk);
      		pack_mputw(pictures[i]->h, chunk);
      		
      		for(y = 0; y < pictures[i]->h; y++) {
      			for(x = 0; x < pictures[i]->w; x++) {      				
      				int c = getpixel(pictures[i], x, y);
      				int r = getr32(c);
      				int g = getg32(c);
      				int b = getb32(c);
      				int a = geta32(c);
      		
      				pack_putc(r, chunk);
      				pack_putc(g, chunk);
      				pack_putc(b, chunk);
      				pack_putc(a, chunk);
      			}
      		}
      	
      		pack_fclose_chunk(chunk);
      	}
			
		}
		
		pack_fclose(file);
	}
	
	for (n = 0; n < num; n++) {
		destroy_bitmap(pictures[n]);
	}
}

DATAFILE *load_gfx(char *name, int num)
{
	DATAFILE *dat;
	char filename[1024 * 6];
	strcpy(filename, name);
	strcat(filename, ".dat");
	set_color_conversion(COLORCONV_NONE);
	dat = load_datafile(filename);
	set_color_conversion(COLORCONV_TOTAL);
	if (dat) return dat;
	make_gfx(name, num);
	set_color_conversion(COLORCONV_NONE);
	dat = load_datafile(filename);
	set_color_conversion(COLORCONV_TOTAL);
	return dat;
}
