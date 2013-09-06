error:
	@echo Targets are 'mingw' or 'linux' or 'msvc' or 'djgpp'. See makefile for details.

mingw:
	gcc *.c -o"EggHack.exe" -s -Wall -O3 -lalleg -mwindows

mingw-dev:
	gcc *.c -o"EggHack.exe" ../../dev/icon.o -s -Wall -O3 -DALLEGRO_STATICLINK -lalleg_s -mwindows -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lole32 -ldinput -lddraw -ldxguid -lwinmm -ldsound

linux:
	gcc *.c -o"EggHack" -s -Wall -O3 `allegro-config --libs`

msvc:
	cl -D "WIN32" -MT -W1 -Ox *.c -nologo -o"EggHack.exe" alleg.lib

djgpp:
	gcc *.c -o"EggHack.exe" -s -Wall -O3 -lalleg
