#include "wiki.h"
// REMEMBER TO CHECK FOR THIS AND FREE THE MEMORY IN THE CALLER'S CALLER (sic)
thread_fn allocate_errorstatus_and_exit_pthread(int code) {
  int * pthread_errorcode = malloc(sizeof(int));
  *pthread_errorcode = code;
  pthread_exit(pthread_errorcode);
}
thread_fn extract_raw_summary(void * arg) {

  pcre2_code * re_re;
  PCRE2_SPTR re_pattern;
  PCRE2_SPTR re_subject;
  PCRE2_SPTR re_name_table;
  int re_errornumber;
  int re_namecount;
  int re_name_entry_size;
  int re_rc;
  char * re_addr;
  PCRE2_SIZE re_erroroffset;
  PCRE2_SIZE * re_ovector;
  size_t re_subject_length;
  pcre2_match_data * re_match_data;
  re_pattern = (PCRE2_SPTR) "(?<summary>'''(?<title>.*?)'''.*?)==";
  WikiSummary * l_wsummary = (WikiSummary *) arg;
  re_addr = (char *) l_wsummary->unparsed_text;
  re_subject = (PCRE2_SPTR) re_addr;
  re_subject_length = l_wsummary->unparsed_size;
  re_re = pcre2_compile(re_pattern,
                        PCRE2_ZERO_TERMINATED,
                        PCRE2_MULTILINE | PCRE2_DOTALL, //options
                        &re_errornumber,
                        &re_erroroffset,
                        NULL); // compile offset

  if(re_re == NULL) {
    PCRE2_UCHAR buffer[MAX_BUFFER];
    pcre2_get_error_message(re_errornumber, buffer, sizeof(buffer));
    //printf("pcre2 err at %d: %s\n", (int)erroroffset, buffer);

    allocate_errorstatus_and_exit_pthread(re_errornumber);
  }
  re_match_data = pcre2_match_data_create_from_pattern(re_re,NULL);
  re_rc = pcre2_match(re_re,
                      re_subject,
                      re_subject_length,
                      0, // offset in subject
                      0, // options
                      re_match_data,
                      NULL);
  if (re_rc < 0)
    {
      switch(re_rc)
        {
        case PCRE2_ERROR_NOMATCH: break;
          /*
            Handle other special cases if you like
          */
        default: break;
        }
      pcre2_match_data_free(re_match_data);   /* Release memory used for the match */
      pcre2_code_free(re_re);                 /* data and the compiled pattern. */

      allocate_errorstatus_and_exit_pthread(re_rc);
    }
  re_ovector = pcre2_get_ovector_pointer(re_match_data);

  if (re_rc == 0){
    pcre2_match_data_free(re_match_data);
    pcre2_code_free(re_re);
    allocate_errorstatus_and_exit_pthread(re_rc);
  }
  (void)pcre2_pattern_info(
                           re_re,                   /* the compiled pattern */
                           PCRE2_INFO_NAMECOUNT, /* get the number of named substrings */
                           &re_namecount);          /* where to put the answer */

  if (re_namecount <= 0) allocate_errorstatus_and_exit_pthread(re_namecount); else
    {
      //    PCRE2_SPTR tabptr;
  

      /* Before we can access the substrings, we must extract the table for
         translating names to numbers, and the size of each entry in the table. */

      (void)pcre2_pattern_info(
                               re_re,                       /* the compiled pattern */
                               PCRE2_INFO_NAMETABLE,     /* address of the table */
                               &re_name_table);             /* where to put the answer */

      (void)pcre2_pattern_info(
                               re_re,                       /* the compiled pattern */
                               PCRE2_INFO_NAMEENTRYSIZE, /* size of each entry in the table */
                               &re_name_entry_size);        /* where to put the answer */


      pcre2_substring_length_byname(re_match_data, (PCRE2_SPTR) "summary",&l_wsummary->parsed_size);
      l_wsummary->parsed_text = malloc(l_wsummary->parsed_size * sizeof(char));
      pcre2_substring_copy_byname(re_match_data,(PCRE2_SPTR) "summary",l_wsummary->parsed_text,&l_wsummary->parsed_size);
    }
  pcre2_match_data_free(re_match_data);  /* Release the memory that was used */
  pcre2_code_free(re_re);                /* for the match data and the pattern. */

  pthread_exit(NULL);
}
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
    if(result != NULL) { 
      l_wresult->extracted_text = xmlNodeListGetString(doc,
                                                       result
                                                       ->nodesetval
                                                       ->nodeTab[0]
                                                       ->children,
                                                       1);
      l_wresult->extracted_size = atoi((string) xmlGetProp(result->nodesetval->nodeTab[0], (xmlChar * ) "bytes"));

    }
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
    l_wquery->chunk->size = to_parse->extracted_size;
    pthread_mutex_unlock(l_wquery->chunk->chunk_mutex);
    // clean up memory allocated for struct passed to extraction thread
    free(to_parse);
  }
  curl_easy_cleanup(curl);
  pthread_exit(NULL);
}
