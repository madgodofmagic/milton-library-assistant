#include "wiki.h"

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
