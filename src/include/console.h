#ifndef CONSOLE_H_yMWGhus96Xse2J2e
#define CONSOLE_H_yMWGhus96Xse2J2e

// BUILDINDEX = { 0: linux, 1: win64 }

#if BUILDINDEX == 0
    #define _XOPEN_SOURCE_EXTENDED
    #include <ncursesw/curses.h>
#else
    #if CURSESINDEX == 0
        #include <ncursesw/curses.h>
    #else
        #define PDC_DLL_BUILD
        #include <curses.h>
    #endif
#endif

#include "geometry.h"

#define CONSOLECOLORPAIR_BLACKBLACK     0
#define CONSOLECOLORPAIR_WHITEBLACK     1
#define CONSOLECOLORPAIR_REDBLACK       2
#define CONSOLECOLORPAIR_YELLOWBLACK    3
#define CONSOLECOLORPAIR_BLUEBLACK      4
#define CONSOLECOLORPAIR_GREENBLACK     5
#define CONSOLECOLORPAIR_MAGENTABLACK   6
#define CONSOLECOLORPAIR_CYANBLACK      7
#define CONSOLECOLORPAIR_BLACKWHITE     8

//char *_basicCharacterSet = "!\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
//wchar_t *_CP437 = L" ☺☻♥♦♣♠•◘○◙♂♀♪♫☼►◄↕‼¶§▬↨↑↓→←∟↔▲▼ !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~⌂ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜ¢£¥₧ƒáíóúñÑªº¿⌐¬½¼¡«»░▒▓│┤╡╢╖╕╣║╗╝╜╛┐└┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌█▄▌▐▀αßΓπΣσµτΦΘΩδ∞φε∩≡±≥≤⌠⌡÷≈°∙·√ⁿ²■ ";

typedef struct Console
{
    size_t colorPairsCount;
    int key;
    Size2D size;
    WINDOW *window;
} Console;

Console *Console_Create();
void Console_Clear(Console *console);
void Console_ClearRow(Console *console, int y);
void Console_Destroy(Console *console);
void Console_DrawBar(Console *console, int y, int x, size_t width, int value, int valueMax, int colorPair, int attributes);
void Console_DrawBarW(Console *console, int y, int x, size_t width, int value, int valueMax, int colorPair, int attributes);
void Console_FillRandomly(Console *console);
void Console_FillRandomlyW(Console *console);
char Console_Getch(Console *console);
char *Console_GetString(Console *console, size_t size);
void Console_MoveCursor(Console *console, Point2D point);
void Console_Refresh(Console *console);
void Console_SetChar(Console *console, int y, int x, char chr, int colorPair, int attributes);
void Console_SetCharW(Console *console, int y, int x, wchar_t wchr, int colorPair, int attributes);
void Console_SetCursor(Console *console, int cursor);
void Console_SetNoDelay(Console *console, bool noDelay);
void Console_SetNoEcho(Console *console, bool noEcho);
void Console_Wait(Console *console, size_t ms);
void Console_Write(Console *console, int y, int x, char *str, int colorPair, int attributes);
void Console_WriteF(Console *console, int y, int x, int colorPair, int attributes, const char *fmt, ...);
void Console_WriteW(Console *console, int y, int x, wchar_t *wstr, int colorPair, int attributes);
void Console_WriteWF(Console *console, int y, int x, int colorPair, int attributes, const wchar_t *fmt, ...);

#endif