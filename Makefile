ALLEGRO_VERSION=5.0.10
PATH_ALLEGRO=C:\allegro-$(ALLEGRO_VERSION)-mingw-4.7.0
LIB_ALLEGRO=\lib\liballegro-$(ALLEGRO_VERSION)-monolith-mt.a
INCLUDE_ALLEGRO=\include

all: game.exe

game.exe: game.o 
	gcc -o game.exe game.o $(PATH_ALLEGRO)$(LIB_ALLEGRO)

game.o: game.c 
	gcc -I $(PATH_ALLEGRO)$(INCLUDE_ALLEGRO) -c game.c

clean:
	del game.o
	del game.exe