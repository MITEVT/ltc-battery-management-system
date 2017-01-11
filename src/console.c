#include <string.h>
#include <stdlib.h>
#include "console.h"
#include "board.h"
#include "microrl.h"


static const char *commands[NUMCOMMANDS] = { "get",
                            "set",
                            "help",
                            "config",
                            "bal"};

static const char nargs[NUMCOMMANDS] = {  1 ,
                        2 ,
                        1 ,
                        0 ,
                        1};

static const char *helpstring[NUMCOMMANDS] = {"get a value", "set a value", "this", "flash that sucker", "set ballence current"};

static const EXECUTE_HANDLER handlers[] = {get, set, help, config, bal};

void get(const char * const * argv) {
    Board_Println("get");
    // [TODO]
}
void set(const char * const * argv) {
    Board_Println("set");
}
void help(const char * const * argv) {
    uint32_t command_i = 0;
    for (command_i = 0; command_i < NUMCOMMANDS; ++command_i)
    {
        if (memcmp(argv[1],commands[command_i],3) == 0){
            Board_Println(helpstring[command_i]);  
        }
    }
}
void config(const char * const * argv) {
    Board_Println("config");
    // [TODO]
}
void bal(const char * const * argv) {
    Board_Println("bal");
    // [TODO]
}                       

void executerl(uint32_t argc, const char * const * argv){
    uint32_t command_i = 0;
    bool found_command = false;
    for (command_i = 0; command_i < NUMCOMMANDS; ++command_i)
    {
        if (memcmp(argv[0],commands[command_i],3) == 0){
            found_command = true;  
            break;
        }
    }
    if (found_command) {
        if (nargs[command_i] == argc-1)
        {
            handlers[command_i](argv);
        }
        else {
            Board_Println("inccorrect number of args");
        }
    }
    else{
        Board_Println("Unrecognized command");
    }
}