#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curses.h>
#include "mla.h"
struct MemoryStruct {
  char *memory;
  size_t size;
  pthread_mutex_t * chunk_mutex;
};
 
size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
unsigned char eye[1059];
unsigned int eye_len;
unsigned char eye2[1060];
unsigned int eye2_len;
unsigned char eye3[1057];
unsigned int eye3_len;
