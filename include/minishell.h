#ifndef MINISHELL_H_
#define MINISHELL_H_

#define __DEBUG
#define _out_

//Necesario para sigaction y otras definiciones que dependen de la defincion de este macro.
#define _XOPEN_SOURCE 700 

//Standard
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>    
#include <sys/wait.h>
#include <ctype.h>

#include <parser.h>
#include <macros.h>
#include <prompt.h>
#include <read.h>
#include <init.h>
#include <env.h>
#include <command/execute.h>
#include <command/builtin.h>

#endif // MINISHELL_H_

