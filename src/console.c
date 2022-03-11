#include "include/console.h"

void Console_Clear(Console *console)
{
    wclear(console->window);
}

void Console_ClearRect(Console *console, Rect2D rect)
{
    for(int y = 0; y < rect.size.height; y++)
    {
        for(int x = 0; x < rect.size.height; x++)
            Console_SetChar(console, rect.position.y + y, rect.position.x + x, ' ', 0, 0);
    }
}

void Console_ClearRow(Console *console, int y)
{
    for(int x = 0; x < console->size.width; x++)
        Console_SetChar(console, y, x, ' ', 0, 0);
}

Console *Console_Create()
{
    setlocale(LC_ALL, "");      // Set the locale to that of the user.
    fwide(stdout, 1);           // Indicate that wide characters will be printed.

    Console *console = malloc(sizeof(Console));
    console->window = initscr();

    noecho();       // Keys typed are not printed.
    cbreak();       // No line buffering.
    curs_set(0);    // Cursor: 0 = none, 1 = baseline, 2 = full

    nodelay(console->window, FALSE);    // Getting a key makes the program wait.
    keypad(console->window, TRUE);      // The keypad can be used.

    console->key = -1;
    console->size = (Size2D){ getmaxx(console->window), getmaxy(console->window) };

    start_color();
    init_pair(0, COLOR_BLACK, COLOR_BLACK);
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_GREEN, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_CYAN, COLOR_BLACK);
    init_pair(8, COLOR_BLACK, COLOR_WHITE);

    console->colorPairsCount = 8;

    return console;
}

void Console_Destroy(Console *console)
{
    delwin(console->window);
    free(console);
}

void Console_DrawBar(Console *console, int y, int x, size_t width, int value, int valueMax, int colorPair, int attributes)
{
    double per = (double)value / (double)valueMax;
    size_t w = (size_t)((double)width * per);

    for(int xx = 0; xx < w; xx++)
        Console_SetChar(console, y, x + xx, '+', colorPair, attributes);
    for(int xx = w; xx < width; xx++)
        Console_SetChar(console, y, x + xx, ' ', colorPair, attributes);
}

void Console_DrawBarW(Console *console, int y, int x, size_t width, int value, int valueMax, int colorPair, int attributes)
{
    double per = (double)value / (double)valueMax;
    size_t w = (size_t)((double)width * per);

    for(int xx = 0; xx < w; xx++)
        Console_SetCharW(console, y, x + xx, L'█', colorPair, attributes);
    for(int xx = w; xx < width; xx++)
        Console_SetCharW(console, y, x + xx, L'░', colorPair, attributes);
}

void Console_DrawRect(Console *console, Rect2D rect, wchar_t wchrs[6], int colorPair, int attributes)
{
    //│┤╡╢╖╕╣║╗╝╜╛┐└┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌
    // wchrs[6] = { L'─', L'│', L'┌', L'┐', L'┘', L'└' };
    // wchrs[6] = { L'═', L'║', L'╔', L'╗', L'╝', L'╚' };

    Console_ClearRect(console, rect);

    for(int x = rect.position.x + 1; x < rect.position.x + rect.size.width - 1; x++)
    {
        Console_SetCharW(console, rect.position.y, rect.position.x + x, wchrs[0], colorPair, attributes);
        Console_SetCharW(console, rect.position.y + rect.size.height - 1, rect.position.x + x, wchrs[0], colorPair, attributes);
    }

    for(int y = rect.position.y + 1; y < rect.position.y + rect.size.height - 1; y++)
    {
        Console_SetCharW(console, rect.position.y + y, rect.position.x, wchrs[1], colorPair, attributes);
        Console_SetCharW(console, rect.position.y + y, rect.position.x + rect.size.width - 1, wchrs[1], colorPair, attributes);
    }

    Console_SetCharW(console, rect.position.y, rect.position.x, wchrs[2], colorPair, attributes);
    Console_SetCharW(console, rect.position.y, rect.position.x + rect.size.width - 1, wchrs[3], colorPair, attributes);
    Console_SetCharW(console, rect.position.y + rect.size.height - 1, rect.position.x + rect.size.width - 1, wchrs[4], colorPair, attributes);
    Console_SetCharW(console, rect.position.y + rect.size.height - 1, rect.position.x, wchrs[5], colorPair, attributes);
}

void Console_FillRandomly(Console *console)
{
    char *basicCharacterSet = "!\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

    for(int y = 0; y < console->size.height; y++)
    {
        for(int x = 0; x < console->size.width; x++)
            Console_SetChar(console, y, x, basicCharacterSet[rand() % strlen(basicCharacterSet)], rand() % console->colorPairsCount, 0);
    }
}

void Console_FillRandomlyW(Console *console)
{
    wchar_t *CP437 = L" ☺☻♥♦♣♠•◘○◙♂♀♪♫☼►◄↕‼¶§▬↨↑↓→←∟↔▲▼ !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~⌂ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜ¢£¥₧ƒáíóúñÑªº¿⌐¬½¼¡«»░▒▓│┤╡╢╖╕╣║╗╝╜╛┐└┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌█▄▌▐▀αßΓπΣσµτΦΘΩδ∞φε∩≡±≥≤⌠⌡÷≈°∙·√ⁿ²■ ";

    for(int y = 0; y < console->size.height; y++)
    {
        for(int x = 0; x < console->size.width; x++)
            Console_SetCharW(console, y, x, CP437[rand() % 256], rand() % console->colorPairsCount, 0);
    }
}

char *Console_GetString(Console *console, size_t size)
{
    echo();
    nocbreak();
    Console_SetCursor(console, 2);

    char *str = malloc(sizeof(char) * (size + 1));
    memset(str, 0, size);
    wgetnstr(console->window, str, size);

    noecho();
    cbreak();
    Console_SetCursor(console, 0);

    return str;
}

char Console_Getch(Console *console)
{
    console->key = wgetch(console->window);
    return console->key;
}

void Console_MoveCursor(Console *console, Point2D point)
{
    wmove(console->window, point.y, point.x);
}

void Console_Refresh(Console *console)
{
    wrefresh(console->window);
}

void Console_SetChar(Console *console, int y, int x, char chr, int colorPair, int attributes)
{
    chtype color = COLOR_PAIR(colorPair);

    attron(color | attributes);
    mvwaddch(console->window, y, x, chr);
    attroff(color | attributes);
}

void Console_SetCharW(Console *console, int y, int x, wchar_t wchr, int colorPair, int attributes)
{
    chtype color = COLOR_PAIR(colorPair);

    attron(color | attributes);

    wchar_t wstr[2];
    wstr[0] = wchr;
    wstr[1] = 0;
    mvwaddnwstr(console->window, y, x, wstr, 1);

    /*
    #if BUILDINDEX == 0
        wchar_t wstr[2];
        wstr[0] = wchr;
        wstr[1] = 0;
        mvwaddnwstr(console->window, y, x, wstr, 1);
    #else
        mvwaddch(console->window, y, x, wchr); 
    #endif
    */

    attroff(color | attributes);
}

void Console_SetCursor(Console *console, int cursor)
{
    curs_set(cursor);
}

void Console_SetNoDelay(Console *console, bool noDelay)
{
    nodelay(console->window, noDelay);
}

void Console_SetNoEcho(Console *console, bool noEcho)
{
    if(noEcho) noecho(); else echo();
}

void Console_Wait(Console *console, size_t ms)
{
    double start = (double)clock() / (double)(CLOCKS_PER_SEC / 1000);

    while ((double)clock() / (double)(CLOCKS_PER_SEC / 1000) < start + (double)ms);
}

void Console_Write(Console *console, int y, int x, char *str, int colorPair, int attributes)
{
    for(int i = 0; i < strlen(str); i++)
        Console_SetChar(console, y, x + i, str[i], colorPair, attributes);

    /*
    chtype color = COLOR_PAIR(colorPair);

    attron(color | attributes);
    mvwaddstr(console->window, y, x, str);
    attroff(color | attributes);
    */
}

void Console_WriteF(Console *console, int y, int x, int colorPair, int attributes, const char *fmt, ...)
{
    va_list args;        
    va_start(args, fmt);

    char str[256];
    memset(str, 0, 256);
    vsnprintf(str, 256, fmt, args);

    Console_Write(console, y, x, str, colorPair, attributes);

    va_end(args);
}

void Console_WriteW(Console *console, int y, int x, wchar_t *wstr, int colorPair, int attributes)
{
    for(int i = 0; i < wcslen(wstr); i++)
        Console_SetCharW(console, y, x + i, wstr[i], colorPair, attributes);

    /*
    chtype color = COLOR_PAIR(colorPair);

    attron(color | attributes);
    mvwaddwstr(console->window, y, x, wstr);
    attroff(color | attributes);
    */
}

void Console_WriteWF(Console *console, int y, int x, int colorPair, int attributes, const wchar_t *wfmt, ...)
{
    va_list args;        
    va_start(args, wfmt);

    wchar_t wstr[256];
    memset(wstr, 0, 256);
    vswprintf(wstr, 256, wfmt, args);

    Console_WriteW(console, y, x, wstr, colorPair, attributes);

    va_end(args);
}
