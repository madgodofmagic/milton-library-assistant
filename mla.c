/* begin mla.c */
/* TODO: verbs ('commands')
   follow redirects,
   parse wiki response, (xml and wikimarkup)
   steal nlp library (to translate nl into 'verbs' into callbacks). 
   capitalise wiki title automatically
   ai conversation
   remember looked-up terms, build a tree of past lookups
   cache wiki lookups
   create 'links' (suggestions) for lookups based on prefious queries
   BUG (maybe): ncurses seems to truncate long results, could be wide chars
   DONE: move from stdio to a terminal-aware library
   DONE: unfucked use of globals
   IN PROGRESS: Parsing wikipedia response (xml done)
*/
#include <pthread.h>
#include <curl/curl.h>
#include "ui.h"


int main() {
  pthread_t ui_thread;
  curl_global_init(CURL_GLOBAL_DEFAULT);
  pthread_create(&ui_thread, NULL , &milton_ui, NULL);
  pthread_join(ui_thread, NULL);
  curl_global_cleanup();
}
/* end mla.c */
