#ifndef INIT_H_
#define INIT_H_


extern const char *banner;


#ifdef _DEF_BANNER
#ifndef __DEBUG
const char* banner = 
"                                      _       _     _          _ _ \n"
"                                /\\/\\ (_)_ __ (_)___| |__   ___| | |\n"
"                               /    \\| | '_ \\| / __| '_ \\ / _ \\ | |\n"
"                              / /\\/\\ \\ | | | | \\__ \\ | | |  __/ | |\n"
"                              \\/    \\/_|_| |_|_|___/_| |_|\\___|_|_|\n"
"                                                                     \n";
#endif
#endif
/**
 * @brief Funcion de inicialización del programa.
 */
void init_minishell(int argc, char** argv);

#endif // INIT_H_