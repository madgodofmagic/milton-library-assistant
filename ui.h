#define _XOPEN_SOURCE_EXTENDED
#include <ncursesw/curses.h>
#include <unistd.h>
#include <locale.h>
#include <wchar.h>
#include "mla.h"
#include "wiki.h"
void blink(WINDOW * win);
void greet_and_prompt(WINDOW * win);
thread_fn milton_ui(__attribute__((unused)) void * arg);
wchar_t greet[36];
wchar_t prompt[3];
