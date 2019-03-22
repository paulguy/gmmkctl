#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gmmk.h"

int main(int argc, char **argv) {
    int i, j;
    GMMKState *s;
    unsigned int argInt;

    unsigned int activeDevs;
    unsigned int devMask;

    s = gmmk_open();
    if(s == NULL) {
        fprintf(stderr, "Failed to open any device.\n");
        exit(EXIT_SUCCESS);
    }

    fprintf(stderr, "Found %u devices.\n", s->devices);
    activeDevs = devMask = setBits(s->devices);

    for(i = 1; i < argc; i++) {
        if(strcmp(argv[i], "devmask") == 0) {
            if(i < argc - 1) {
                i++;
                devMask = atoi(argv[i]);
                if(devMask & activeDevs == 0) {
                    fprintf(stderr, "WARNING: No devices selected!\n");
                }
            }
        } else if(strcmp(argv[i], "mode") == 0) {
            if(i < argc - 1) {
                i++;
                argInt = atoi(argv[i]);
                for(j = 0; j < GMMK_MAX_DEVS; j++) {
                    if(devMask & (1 << j)) {
                        gmmk_setMode(s, j, argInt);
                    }
                }
            }
        }
    }

    gmmk_close(s);

    exit(EXIT_SUCCESS);
}

unsigned int setBits(unsigned int bits) {
    unsigned int val = 1;
    for(; bits > 0; bits--) {
        val <<= 1;
        val |= 1;
    }

    return(val);
}
