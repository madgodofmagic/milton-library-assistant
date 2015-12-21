#define MAX_BUFFER 1024
#include <stdio.h>
#include <stdlib.h>
#include "mpc/mpc.h"
int main() {
  mpc_parser_t * LowerA = mpc_new("lowera");
  mpc_parser_t * Singlequote = mpc_new("sq");
  mpc_parser_t * Equals = mpc_new("eq");
//  mpc_define(Singlequote, mpc_char('\''));
  mpc_result_t r;
  mpc_parser_t * Article = mpc_new("article");
  mpca_lang(MPCA_LANG_PREDICTIVE,"sq: '\'';" "eq: '=';" "lowera: /[a-zA-Z0-9]*/;" "article:(/[ \|&a-zA-Z0-9{}]*/|';'|'['|']')*<sq><sq><sq><lowera>/$/;",Singlequote,Equals,LowerA,Article,NULL);
  
//  FILE * test_article = fopen("test.txt", "r");
  if(mpc_parse_pipe("stdin", stdin, Article, &r)) {
    mpc_ast_print(r.output);
    mpc_ast_delete(r.output);
    
  } else {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
    }
  mpc_cleanup(4,LowerA,Singlequote,Equals,Article);
}
