#ifndef SIGNALS_H_
#define SIGNALS_H_

void sigint_handler(int sig);
void sigchld_handler(int sig);

#endif // SIGNALS_H_