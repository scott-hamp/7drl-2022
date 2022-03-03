#ifndef CONSOLE_H_yMWGhus96Xse2J2e
#define CONSOLE_H_yMWGhus96Xse2J2e

#include <locale.h>
#include <stdlib.h>

// BUILDINDEX = { 0: linux, 1: win64 }

#if BUILDINDEX == 0
    #define _XOPEN_SOURCE_EXTENDED
    #include <ncursesw/curses.h>
#else
    #include <curses.h>
#endif

typedef struct Size2D
{
    size_t width, height;
} Size2D;

typedef struct Console
{
    size_t colorPairsCount;
    Size2D size;
    WINDOW *window;
} Console;

Console *Console_Create();
void Console_Clear(Console *console);
char Console_Getch(Console *console);
void Console_Destroy(Console *console);
char *Console_GetString(Console *console, size_t size);
void Console_Refresh(Console *console);
void Console_SetChar(Console *console, int y, int x, char chr, int colorPair, int attributes);
void Console_SetCharW(Console *console, int y, int x, wchar_t wchr, int colorPair, int attributes);
void Console_SetCursor(Console *console, int cursor);
void Console_Write(Console *console, int y, int x, char *str, int colorPair, int attributes);
void Console_WriteF(Console *console, int y, int x, int colorPair, int attributes, const char *fmt, ...);
void Console_WriteW(Console *console, int y, int x, wchar_t *wstr, int colorPair, int attributes);
void Console_WriteWF(Console *console, int y, int x, int colorPair, int attributes, const wchar_t *fmt, ...);

#endif