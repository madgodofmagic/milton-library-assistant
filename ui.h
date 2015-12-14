#include <curses.h>
#include <unistd.h>
#include "mla.h"
#include "wiki.h"
void blink(WINDOW * win);
void greet_and_prompt(WINDOW * win);
thread_fn milton_ui(__attribute__((unused)) void * arg);
