#ifndef PARSER_MSHPARSER_H_
#define PARSER_MSHPARSER_H_

#define _out_   //Output pointer func param indicator
#define _opt_   //Opcional pointer func param indicator
#define _in_out_

#define _POSIX_C_SOURCE 200809L

//Main header for mshparser module
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <ctype.h>


#include <grammar.h>
#include <heredoc.h>
#include <errors.h>
#include <ast.h>
#include <tokenizer.h>
#include <parser.h>
#include <redir.h>
#include <heredoc.h>
#include <parser_utils.h>
#include <scanner.h>



#endif // PARSER_MSHPARSER_H_