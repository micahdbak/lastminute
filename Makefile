FILES = sprite.c\
	world.c\
	main.c

make explorer:
	cc -o Explorer $(shell sdl2-config --cflags --libs) -framework OpenGL $(FILES)
