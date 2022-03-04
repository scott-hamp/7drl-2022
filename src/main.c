#include "include/console.h"

int main(int argc, char **argv)
{
    Console *_console = Console_Create();

    char *_basicCharacterSet = "!\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    wchar_t *_CP437 = L" ☺☻♥♦♣♠•◘○◙♂♀♪♫☼►◄↕‼¶§▬↨↑↓→←∟↔▲▼ !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~⌂ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜ¢£¥₧ƒáíóúñÑªº¿⌐¬½¼¡«»░▒▓│┤╡╢╖╕╣║╗╝╜╛┐└┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌█▄▌▐▀αßΓπΣσµτΦΘΩδ∞φε∩≡±≥≤⌠⌡÷≈°∙·√ⁿ²■ ";
    char _key = 0;

    Console_Write(_console, 0, 0, "Press any key to continue...\n", 0, 0);
    Console_Refresh(_console);

    _key = Console_Getch(_console);

    while(_key != 27) // 'Esc.'
    {
        Console_Clear(_console);

        char sizeKeyStr[50];
        memset(sizeKeyStr, 0, 50);
        sprintf(sizeKeyStr, "SIZE = (W %d, H %d)\t KEY = %d", _console->size.width, _console->size.height, _key);
        
        Console_Write(_console, 0, 0, sizeKeyStr, 0, A_BOLD);

        for(int y = 1; y < _console->size.height; y++)
        {
            for(int x = 0; x < _console->size.width; x++)
            {
                wchar_t wchr = _CP437[rand() % 256];

                Console_SetCharW(_console, y, x, wchr, (rand() % _console->colorPairsCount) + 1, 0);
            }
        }

        Console_Refresh(_console);
        _key = Console_Getch(_console);
    }

    Console_Destroy(_console);
    exit(1);

    return 1;
}