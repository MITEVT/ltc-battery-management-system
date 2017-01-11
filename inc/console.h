#include <string.h>
#include <stdlib.h>
#include "board.h"
#include "microrl.h"


#define NUMCOMMANDS  5


// microrl object
static microrl_t rl;

typedef void (* const EXECUTE_HANDLER)(const char * const *);



static void get(const char * const *);
static void set(const char * const *);
static void help(const char * const *);
static void config(const char * const *);
static void bal(const char * const *);


void executerl(uint32_t argc, const char * const * argv);                    