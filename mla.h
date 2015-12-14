#define MAX_URL 255
#define WIKI_ENDPOINT "https://en.wikipedia.org/w/api.php\
?action=query&format=xml&exportnowrap=true&export=true&titles=\0"
#define MAX_BUFFER ((MAX_URL - strlen(WIKI_ENDPOINT)) + 1)
// typedef to make thread functions easier to spot
typedef void * thread_fn;
typedef char * string;
