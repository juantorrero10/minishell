#ifndef MINISHELL_H_
#define MINISHELL_H_

#define __DEBUG
#define _out_


//Standard
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/wait.h>

#include <parser.h>
#include <macros.h>
#include <prompt.h>
#include <read.h>
#include <init.h>
#include <env.h>
#include <command/execute.h>
#include <command/builtin.h>

#endif // MINISHELL_H_

