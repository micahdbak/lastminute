#include <ctype.h>
#include <assert.h>
#include <stdarg.h>

#include "engine.h"

/* Font files, in BMP format, are to be arranged as such:
 * ABCDEFGHIJKL
 * MNOPQRSTUVWX
 * YZabcdefghij
 * klmnopqrstuv
 * wxyz01234567
 * 89!?.,:;'"()
 */

#define toIndex(toIndex_x, toIndex_y)		((toIndex_y * FONT_NCOLS) + toIndex_x)
#define toCoordinate_x(toCoordinate_index)	(toCoordinate_index % FONT_NCOLS)
#define toCoordinate_y(toCoordinate_index)	((toCoordinate_index - (toCoordinate_index % FONT_NCOLS)) / FONT_NCOLS)

#define ENDBOUNDUPPER	26
#define ENDBOUNDLOWER	(26 * 2)
#define ENDBOUNDDIGIT	((26 * 2) + 10)

int charToIndex(char c)
{
	if (isupper(c))
		return c - 'A';
	if (islower(c))
		return c - 'a' + ENDBOUNDUPPER;
	if (isdigit(c))
		return c - '0' + ENDBOUNDLOWER;

	/* [c] is neither uppercase, lowercase, or a digit. */
	switch (c)
	{
	case '!':	return ENDBOUNDDIGIT + 0;	break;
	case '?':	return ENDBOUNDDIGIT + 1;	break;
	case '.':	return ENDBOUNDDIGIT + 2;	break;
	case ',':	return ENDBOUNDDIGIT + 3;	break;
	case ':':	return ENDBOUNDDIGIT + 4;	break;
	case ';':	return ENDBOUNDDIGIT + 5;	break;
	case '\'':	return ENDBOUNDDIGIT + 6;	break;
	case '"':	return ENDBOUNDDIGIT + 7;	break;
	case '(':	return ENDBOUNDDIGIT + 8;	break;
	case ')':	return ENDBOUNDDIGIT + 9;	break;
	default:
		return -1;

		break;
	}
}

void loadFont(const char *path, struct font *font, int leading, int spaceWidth, int defaultWidth, int nModified, ...)
{
	int glyphWidth, glyphHeight,
	    x, y, index,
	    i;
	va_list ap;
	struct modifyLetter modifyLetter;

	font->sheet = SDL_LoadBMP(path);
	assert(font->sheet != NULL);

	font->leading = leading;
	font->spaceWidth = spaceWidth;

	glyphWidth = font->sheet->w / FONT_NCOLS;
	glyphHeight = font->sheet->h / FONT_NROWS;

	for (y = 0; y < FONT_NROWS; ++y)
		for (x = 0; x < FONT_NCOLS; ++x)
		{
			index = toIndex(x, y);

			font->letter[index].x = x * glyphWidth;
			font->letter[index].y = y * glyphHeight;
			font->letter[index].w = defaultWidth;
			font->letter[index].h = glyphHeight;
		}

	va_start(ap, nModified);

	for (i = 0; i < nModified; ++i)
	{
		modifyLetter = va_arg(ap, struct modifyLetter);

		index = charToIndex(modifyLetter.letter);
		font->letter[index].w = modifyLetter.width;
	}

	va_end(ap);
}

#define MAXWORD	100

void drawText(SDL_Surface *surface, SDL_Rect *drawArea, struct font *font, char *text, ...)
{
	char i, c;
	char word[MAXWORD];
	int wordWidth, index,
	    xPos = 0, yPos = 0;
	SDL_Rect letterArea;
	va_list ap;
	int number;
	char *string;

	va_start(ap, text);

	for (;;)
	{
		for (i = 0, c = *text++; c != '%' && !isspace(c) && c != '\0' && i < MAXWORD - 1; c = *text++)
			word[i++] = c;

		if (c == '%')
		{
			switch (c = *text++)
			{
			case 'd':
				number = va_arg(ap, int);
				sprintf(word, "%d", number);

				break;
			case 's':
				string = va_arg(ap, char *);
				sprintf(word, "%s", string);

				break;
			case 'n':
				xPos = 0;
				yPos += font->leading;

				continue;
			default:

				break;
			}
		} else {
			if (i == MAXWORD - 1)
			/* This word will be broken, but it must maintain it's characters. */
				--text;

			word[i] = '\0';
		}

		for (wordWidth = 0, i = 0; word[i] != '\0'; ++i)
		{
			index = charToIndex(word[i]);
			wordWidth += font->letter[index].w;
		}

		if (xPos + wordWidth > drawArea->w)
		{
			xPos = 0;
			yPos += font->leading;
		}

		letterArea.x = drawArea->x + xPos;
		letterArea.y = drawArea->y + yPos;
		letterArea.h = font->letter[0].h;

		for (i = 0; word[i] != '\0'; ++i)
		{
			index = charToIndex(word[i]);
			letterArea.w = font->letter[index].w;

			SDL_BlitSurface(font->sheet, &font->letter[index], surface, &letterArea);

			letterArea.x += letterArea.w;
		}

		xPos += wordWidth + font->spaceWidth;

		if (c == '\0')
			break;
	}
}

void freeFont(struct font *font)
{
	SDL_FreeSurface(font->sheet);
}

void colorArea(SDL_Surface* surface, SDL_Rect *area, Uint32 color)
{
	SDL_FillRect(colorArea_surface, NULL, color);
	SDL_BlitScaled(colorArea_surface, NULL, surface, area);
}
