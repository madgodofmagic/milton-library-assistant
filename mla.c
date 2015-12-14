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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <curl/curl.h>
#define MAX_URL 255
#define WIKI_ENDPOINT "https://en.wikipedia.org/w/api.php\
?action=query&format=xml&exportnowrap=true&export=true&titles=\0"
#define MAX_BUFFER ((MAX_URL - strlen(WIKI_ENDPOINT)) + 1)
#include <string.h>
#include <curses.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <locale.h>
#include "libstolen.h"
// typedef to make thread functions easier to spot
typedef void * thread_fn;
typedef char * string;
// struct passed to curl to save query result in memory
typedef struct WikiQuery {
  struct MemoryStruct * chunk;
  string arg;
} WikiQuery;
// struct passed to libxml2 to demangle the wikimedia api response
//into .extracted_text
typedef struct WikiResult {
  struct MemoryStruct * raw_result;
  xmlChar * extracted_text;
  } WikiResult;
// extract article text from xml using libxml2.
// most of this crap allocates memory without any clear indication
// that it's on the heap because fuck you I guess
thread_fn extract_wiki_document(void * arg) {
  WikiResult * l_wresult = (WikiResult *) arg;
  xmlChar * xpath;
  xmlXPathContextPtr context;
  xmlXPathObjectPtr result;
  pthread_mutex_lock(l_wresult->raw_result->chunk_mutex);
  xmlDocPtr doc = xmlReadMemory(l_wresult->raw_result->memory,
                                l_wresult->raw_result->size,
                                NULL,
                                NULL,
                                XML_PARSE_HUGE);
  pthread_mutex_unlock(l_wresult->raw_result->chunk_mutex);
  context = xmlXPathNewContext(doc);
  // xpath lookup fails without a namespace declared if they are defined in the document
  xmlXPathRegisterNs(context, (const xmlChar *) "eatshitwikipedia",     \
                     (const xmlChar *) "http://www.mediawiki.org/xml/export-0.10/");
  xpath = (xmlChar *) "//eatshitwikipedia:text";
  result = xmlXPathEvalExpression(xpath, context);
  if(xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    l_wresult->extracted_text = NULL;
    }
  else {
    if(result != NULL) 
      l_wresult->extracted_text = xmlNodeListGetString(doc,
                                                       result
                                                       ->nodesetval
                                                       ->nodeTab[0]
                                                       ->children,
                                                       1);
    else
      l_wresult->extracted_text = NULL;
  }
  

  // clean up libxml2 shite
  xmlXPathFreeContext(context);
  xmlXPathFreeObject(result);
  xmlFreeDoc(doc);
  xmlCleanupParser();
  pthread_exit(NULL);
}
thread_fn knowledge_query(void * arg) {
  CURL *curl;
  CURLcode res;
  char uri[MAX_URL];
  WikiQuery * l_wquery = (WikiQuery *) arg;
  string query = l_wquery->arg;
  pthread_t parser_thread;
  uri[0]=0;
  strcat(uri,WIKI_ENDPOINT);
  strcat(uri,query);
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl,CURLOPT_URL,uri);
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,WriteMemoryCallback);
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,(void *) l_wquery->chunk);
  }
  pthread_mutex_lock(l_wquery->chunk->chunk_mutex);
  res = curl_easy_perform(curl);
  pthread_mutex_unlock(l_wquery->chunk->chunk_mutex);
 
  if(res != CURLE_OK) {
    printw("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    free(l_wquery->chunk->memory);
  } else {
    
    WikiResult * to_parse = malloc(sizeof(WikiResult));
    to_parse->raw_result = l_wquery->chunk;
    pthread_create(&parser_thread, NULL, &extract_wiki_document ,
                   (void *) to_parse);
    pthread_join(parser_thread,NULL);
    pthread_mutex_lock(l_wquery->chunk->chunk_mutex);
    // clean up after curl lookup
    free(l_wquery->chunk->memory);
    // swap the pointer to raw xml (should now be a NULL pointer)
    //for the wikimarkup of the article
    l_wquery->chunk->memory = (string) to_parse->extracted_text;
    pthread_mutex_unlock(l_wquery->chunk->chunk_mutex);
    // clean up memory allocated for struct passed to extraction thread
    free(to_parse);
  }
  curl_easy_cleanup(curl);
  pthread_exit(NULL);
}
void blink(WINDOW * win) {
  mvwprintw(win,0,0,"%s",eye);
  wrefresh(win);
  usleep(200000);
  mvwprintw(win,0,0,"%s",eye2);
  wrefresh(win);
  usleep(200000);
  mvwprintw(win,0,0,"%s",eye3);
  wrefresh(win);
  usleep(200000);
  mvwprintw(win,0,0,"%s",eye2);
  wrefresh(win);
  usleep(200000);
  mvwprintw(win,0,0,"%s",eye);
  wrefresh(win);
}
void greet_and_prompt(WINDOW * win) {
  wclear(win);
  wrefresh(win);
  mvwprintw(win,0,0,"Milton Library Assistant Version 2. ");
  wrefresh(win);
  usleep(200000);
  mvwprintw(win,2,0,"$ ");
  wrefresh(win);
  wmove(win, 2, 3);
  wrefresh(win);
}
thread_fn milton_ui(__attribute__((unused)) void * arg) {
  char * buffer = malloc(sizeof(char) * MAX_BUFFER);
  pthread_t worker_thread;
  WINDOW *top, *bottom;
  int wl1, wl2, wc1, wc2;

  initscr();
  cbreak();
  noecho();
  wl2 = wl1 = LINES/2;
  wc2 = wc1 = COLS/2;
  refresh();
  top = newwin(wl1,wc1,0,0);
  wrefresh(top);
  bottom = newwin(wl2,wc2,wl1,0);
  scrollok(bottom, TRUE);
  idlok(bottom, TRUE);
  wrefresh(bottom);
  blink(top);
  greet_and_prompt(bottom);
  int ch;
  for(;;) {
    int i;
    buffer[0] = 0;
    // read chars until you find a newline, print them back to the user as they come to emulate echo, don't go over maximum buffer size
    for(i = 0;((ch = getch()) != '\n') && ((unsigned long) i <= MAX_BUFFER - 1);i++) {
      buffer[i] = ch;
      waddch(bottom,ch);
      wrefresh(bottom);
    }
    buffer[i] = 0;
    blink(top);
    struct MemoryStruct chunk;
    chunk.size=0;
    chunk.memory = malloc(1);
    chunk.chunk_mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(chunk.chunk_mutex, NULL);
    WikiQuery wquery = {.chunk = &chunk,.arg = buffer};
    pthread_create(&worker_thread,NULL, &knowledge_query, &wquery);
    pthread_join(worker_thread,NULL);
    pthread_mutex_lock(chunk.chunk_mutex);
    if(chunk.memory != NULL) {
      wclear(bottom);
      wrefresh(bottom);
      wprintw(bottom,"%s",chunk.memory);
      wrefresh(bottom);
      getch();
      // finally clean up the xmlString
      xmlFree((xmlChar *) chunk.memory);
      }
    pthread_mutex_unlock(chunk.chunk_mutex);
    pthread_mutex_destroy(chunk.chunk_mutex);
    free(chunk.chunk_mutex);
    greet_and_prompt(bottom);
  }
  free(buffer);

  pthread_exit(NULL);
}
int main() {
  pthread_t ui_thread;
  curl_global_init(CURL_GLOBAL_DEFAULT);
  pthread_create(&ui_thread, NULL , &milton_ui, NULL);
  pthread_join(ui_thread, NULL);
  curl_global_cleanup();
}
/* end mla.c */
