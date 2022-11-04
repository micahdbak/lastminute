FILES = draw.c\
	object.c\
	world.c\
	player.c\
	enemy.c\
	overWorld.c\
	main.c

make:
	cc -o Explorer $(shell pkg-config --cflags --libs SDL2 SDL2_Mixer) $(FILES)
