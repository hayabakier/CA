#include <stdio.h>
#include "memory.h"
#include "parser.h"
#include "FD.c"

int main() {
    parseFile("program.txt");
    while (pc < NumberofInstructions)
        fetch();
    return 0;
}