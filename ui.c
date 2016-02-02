
#include "ui.h"

void blink(WINDOW * win) {
  mvwprintw(win,0,0,"%ls",weye);
  wrefresh(win);
  usleep(200000);
  mvwprintw(win,0,0,"%ls",weye2);
  wrefresh(win);
  usleep(200000);
  mvwprintw(win,0,0,"%ls",weye3);
  wrefresh(win);
  usleep(200000);
  mvwprintw(win,0,0,"%ls",weye2);
  wrefresh(win);
  usleep(200000);
  mvwprintw(win,0,0,"%ls",weye);
  wrefresh(win);
}
void greet_and_prompt(WINDOW * win) {
  wclear(win);
  wrefresh(win);
  mvwprintw(win,0,0,"%ls",greet);
  wrefresh(win);
  usleep(200000);
  mvwprintw(win,2,0,"%ls",prompt);
  wrefresh(win);
  wmove(win, 2, 3);
  wrefresh(win);
}
thread_fn milton_ui(__attribute__((unused)) void * arg) {
  char * buffer = malloc(sizeof(char) * MAX_BUFFER);
  pthread_t worker_thread;
  WINDOW *top, *bottom;
  int wl1, wl2, wc1, wc2;
  mbstowcs(weye, (string) eye, eye_len);
  mbstowcs(weye2, (string) eye2, eye2_len);
  mbstowcs(weye3, (string) eye3, eye3_len);
  mbstowcs(greet, "Milton Library Assistant Version 2.", sizeof(greet));
  mbstowcs(prompt, "$ ", sizeof(prompt));
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
      wchar_t * expanded_extracted_result = malloc(sizeof(wchar_t) * chunk.size);
      mbstowcs(expanded_extracted_result, chunk.memory, chunk.size);
      
      wprintw(bottom,"%ls",expanded_extracted_result);
      wrefresh(bottom);
      getch();
      // finally clean up the xmlString
//      xmlFree((xmlChar *) chunk.memory);
      free(chunk.memory);
      free(expanded_extracted_result);
    }
    pthread_mutex_unlock(chunk.chunk_mutex);
    pthread_mutex_destroy(chunk.chunk_mutex);
    free(chunk.chunk_mutex);
    greet_and_prompt(bottom);
  }
  free(buffer);

  pthread_exit(NULL);
}
