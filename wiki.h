#include "mla.h"
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <pthread.h>
#include <curl/curl.h>
#include <curses.h>
#include "libstolen.h"
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
thread_fn extract_wiki_document(void * arg);
thread_fn knowledge_query(void * arg);
