#ifndef TEMPPARSER_H_
#define TEMPPARSER_H_

//TEEEEMP

typedef struct {
	char * filename;
	int argc;
	char ** argv;
} tcommand;

typedef struct {
	int ncommands;
	tcommand * commands;
	char * redirect_input;
	char * redirect_output;
	char * redirect_error;
	int background;
} tline;

extern tline * tokenize(char *str);

#endif // TEMPPARSER_H_