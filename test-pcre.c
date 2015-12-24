#define PCRE2_CODE_UNIT_WIDTH 8
#define MAX_BUFFER 2^20
#include <pcre2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
int main() {

  pcre2_code * re;
  PCRE2_SPTR pattern;
  PCRE2_SPTR subject;
  PCRE2_SPTR name_table;
//  int crlf_is_newline;
  int errornumber;
//  int find_all = 0;
  int i;
  int namecount;
  int name_entry_size;
  int rc;
//  int utf8;
  char *addr;
  int fd;
  struct stat sb;
  off_t offset, pa_offset;
  size_t length;
  ssize_t s;



  PCRE2_SIZE erroroffset;
  PCRE2_SIZE *ovector;

  size_t subject_length;
  pcre2_match_data *match_data;

  pattern = (PCRE2_SPTR) ".*'''(?<title>.*?)'''(?<summary>.*?)==.+";
//  pattern = (PCRE2_SPTR) "(?<aaa>.+)";
  //subject = (PCRE2_SPTR) "aaatest123bc";
  fd = open("satan.ascii.wiki",O_RDONLY);
  offset = 0;
  pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
  /* offset for mmap() must be page aligned */

  if (offset >= sb.st_size) {
    fprintf(stderr, "offset is past end of file\n");
    exit(EXIT_FAILURE);
  }
  length = sb.st_size - offset;
  addr = mmap(NULL, length + offset - pa_offset, PROT_READ,
              MAP_PRIVATE, fd, pa_offset);
  if (addr == MAP_FAILED)
    perror("mmap");

  s = write(STDOUT_FILENO, addr + offset - pa_offset, length);
  if ((unsigned long)s != length) {
    if (s == -1)
      perror("write");

    fprintf(stderr, "partial write");
    exit(EXIT_FAILURE);
  }
  subject = (PCRE2_SPTR) addr;


  subject_length = strlen((char *)subject);
  re = pcre2_compile(pattern,
                     PCRE2_ZERO_TERMINATED,
                     PCRE2_MULTILINE | PCRE2_DOTALL, //options
                     &errornumber,
                     &erroroffset,
                     NULL); // compile offset

  if(re == NULL) {
    PCRE2_UCHAR buffer[MAX_BUFFER];
    pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
    printf("pcre2 err at %d: %s\n", (int)erroroffset, buffer);
    return 1;
  }
  match_data = pcre2_match_data_create_from_pattern(re,NULL);
  rc = pcre2_match(re,
                   subject,
                   subject_length,
                   0, // offset in subject
                   0, // options
                   match_data,
                   NULL);
  if (rc < 0)
    {
      switch(rc)
        {
        case PCRE2_ERROR_NOMATCH: printf("No match\n"); break;
          /*
            Handle other special cases if you like
          */
        default: printf("Matching error %d\n", rc); break;
        }
      pcre2_match_data_free(match_data);   /* Release memory used for the match */
      pcre2_code_free(re);                 /* data and the compiled pattern. */
      return 1;
    }
  ovector = pcre2_get_ovector_pointer(match_data);
  printf("\nMatch succeeded at offset %d\n", (int)ovector[0]);
  if (rc == 0)
    printf("ovector was not big enough for all the captured substrings\n");

  /* Show substrings stored in the output vector by number. Obviously, in a real
     application you might want to do things other than print them. */

  for (i = 0; i < rc; i++)
    {
      PCRE2_SPTR substring_start = subject + ovector[2*i];
      size_t substring_length = ovector[2*i+1] - ovector[2*i];
      printf("%2d: %.*s\n", i, (int)substring_length, (char *)substring_start);
    }
  (void)pcre2_pattern_info(
                           re,                   /* the compiled pattern */
                           PCRE2_INFO_NAMECOUNT, /* get the number of named substrings */
                           &namecount);          /* where to put the answer */

  if (namecount <= 0) printf("No named substrings\n"); else
    {
      PCRE2_SPTR tabptr;
      printf("Named substrings\n");

      /* Before we can access the substrings, we must extract the table for
         translating names to numbers, and the size of each entry in the table. */

      (void)pcre2_pattern_info(
                               re,                       /* the compiled pattern */
                               PCRE2_INFO_NAMETABLE,     /* address of the table */
                               &name_table);             /* where to put the answer */

      (void)pcre2_pattern_info(
                               re,                       /* the compiled pattern */
                               PCRE2_INFO_NAMEENTRYSIZE, /* size of each entry in the table */
                               &name_entry_size);        /* where to put the answer */

      /* Now we can scan the table and, for each entry, print the number, the name,
         and the substring itself. In the 8-bit library the number is held in two
         bytes, most significant first. */

      tabptr = name_table;
      for (i = 0; i < namecount; i++)
        {
          int n = (tabptr[0] << 8) | tabptr[1];
          printf("(%d) %*s: %.*s\n", n, name_entry_size - 3, tabptr + 2,
                 (int)(ovector[2*n+1] - ovector[2*n]), subject + ovector[2*n]);
          tabptr += name_entry_size;
        }
    }
  pcre2_match_data_free(match_data);  /* Release the memory that was used */
  pcre2_code_free(re);                /* for the match data and the pattern. */

  munmap((void *)subject, length);
  //  return 0;                           /* Exit the program. */


  
}
