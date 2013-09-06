EggHack

Speedhack 2002 B
http://allegro.cc/speedhack

Elias Pschernig

__________
Installing

Type make <platform>, where platform is mingw, msvc, linux or djgpp. Or just
compile all the .c files into an executable, linking with Allegro.

________
The Game

Quickstart:

Esc    ... Title Screen/Exit
F1     ... Singleplayer
F2     ... Multiplayer

Mouse  ... Rotate
Button ... Roll Nuclear Egg

EggHack is a turn based game, where you have to roll a ball over the screen.
In the intro screen, press F1 to start a single player game, F2 to start a
multiplayer game. Press Esc to exit.

The goal of the game is to destroy your opponent's tower. You can aim with
the mouse, and roll a nuclear ball in any direction by clicking the left button.

The bar on your side of the screen shows how much 'health' your tower still has.
When it turns all red, you have lost. There are 3 bonus objects:

Power Shot  ... Cumulative, each one doubles your rolling power next time
Green Cross ... Restores some life
Red X       ... Removes some life, but can never kill you

Mail to elias@users.sf.net if you find bugs, or have suggestions or comments.

__________
The Source

All files starting with 'egg' are copied from Shawn Hargreaves' particle library
"Egg".
