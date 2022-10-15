FILES = draw.c\
	object.c\
	player.c\
	world.c\
	main.c

make explorer:
	cc -o Explorer $(shell pkg-config --cflags --libs SDL2 SDL2_Mixer) $(FILES)
