NAME = LastMinute
OUT = .
CC = cc
CFLAGS = -lm $(shell pkg-config --cflags --libs SDL2 SDL2_Mixer)
FILES = draw.c\
	object.c\
	world.c\
	player.c\
	enemy.c\
	overWorld.c\
	main.c

clean:
	rm -f $(OUT)/$(NAME)

$(NAME): clean
	$(CC) -o $(OUT)/$(NAME) $(CFLAGS) $(FILES)
