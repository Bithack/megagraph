#ifndef _PLOTTER__H_
#define _PLOTTER__H_

#include <stdio.h>

#define MG_NAME "MegaGraph"
#define MG_VERSION "0.9"

#define LOG_I(x, ...) fprintf(stdout, x "\n", ##__VA_ARGS__)
#define LOG_E(x, ...) fprintf(stderr, x "\n", ##__VA_ARGS__)

#endif
