#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gmmk.h"

unsigned int setBits(unsigned int bits);
int readNumber();

int main(int argc, char **argv) {
    int i;
    unsigned int j;
    GMMKState *s;
    unsigned int argInt1, argInt2, argInt3;
    unsigned int num;
    GMMKColor c[GMMK_MAX_KEY];

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
                if((devMask & activeDevs) == 0) {
                    fprintf(stderr, "WARNING: No devices selected!\n");
                }
            }
        } else if(strcmp(argv[i], "mode") == 0) {
            if(i < argc - 1) {
                i++;
                argInt1 = atoi(argv[i]);
                for(j = 0; j < GMMK_MAX_DEVS; j++) {
                    if(devMask & (1 << j)) {
                        gmmk_setMode(s, j, argInt1);
                    }
                }
            }
        } else if(strcmp(argv[i], "brightness") == 0) {
            if(i < argc - 1) {
                i++;
                argInt1 = atoi(argv[i]);
                for(j = 0; j < GMMK_MAX_DEVS; j++) {
                    if(devMask & (1 << j)) {
                        gmmk_setBrightness(s, j, argInt1);
                    }
                }
            }
        } else if(strcmp(argv[i], "delay") == 0) {
            if(i < argc - 1) {
                i++;
                argInt1 = atoi(argv[i]);
                for(j = 0; j < GMMK_MAX_DEVS; j++) {
                    if(devMask & (1 << j)) {
                        gmmk_setDelay(s, j, argInt1);
                    }
                }
            }
        } else if(strcmp(argv[i], "left") == 0) {
            for(j = 0; j < GMMK_MAX_DEVS; j++) {
                if(devMask & (1 << j)) {
                    gmmk_setDirLeft(s, j);
                }
            }
        } else if(strcmp(argv[i], "right") == 0) {
            for(j = 0; j < GMMK_MAX_DEVS; j++) {
                if(devMask & (1 << j)) {
                    gmmk_setDirRight(s, j);
                }
            }
        } else if(strcmp(argv[i], "colorful") == 0) {
            for(j = 0; j < GMMK_MAX_DEVS; j++) {
                if(devMask & (1 << j)) {
                    gmmk_setColorful(s, j);
                }
            }
        } else if(strcmp(argv[i], "single") == 0) {
            for(j = 0; j < GMMK_MAX_DEVS; j++) {
                if(devMask & (1 << j)) {
                    gmmk_unsetColorful(s, j);
                }
            }
        } else if(strcmp(argv[i], "rate") == 0) {
            if(i < argc - 1) {
                i++;
                argInt1 = atoi(argv[i]);
                for(j = 0; j < GMMK_MAX_DEVS; j++) {
                    if(devMask & (1 << j)) {
                        gmmk_setRate(s, j, argInt1);
                    }
                }
            }
        } else if(strcmp(argv[i], "color") == 0) {
            if(i < argc - 3) {
                i++;
                argInt1 = atoi(argv[i]);
                i++;
                argInt2 = atoi(argv[i]);
                i++;
                argInt3 = atoi(argv[i]);
                for(j = 0; j < GMMK_MAX_DEVS; j++) {
                    if(devMask & (1 << j)) {
                        gmmk_setAnimationColor(s, j, argInt1, argInt2, argInt3);
                    }
                }
            }
        } else if(strcmp(argv[i], "keys") == 0) {
            argInt1 = readNumber();
            num = readNumber();
            if(num > GMMK_MAX_KEY) {
                fprintf(stderr, "Count too high.\n");
                break;
            }
            for(j = 0; j < num; j++) {
                c[j].r = readNumber();
                c[j].g = readNumber();
                c[j].b = readNumber();
            }
            for(j = 0; j < GMMK_MAX_DEVS; j++) {
                if(devMask & (1 << j)) {
                    gmmk_setKeys(s, j, argInt1, num, c);
                }
            }
        }
    }

    gmmk_close(s);

    exit(EXIT_SUCCESS);
}

unsigned int setBits(unsigned int bits) {
    unsigned int val = 0;
    for(; bits > 0; bits--) {
        val <<= 1;
        val |= 1;
    }

    return(val);
}

int readNumber() {
    char buffer[4];
    int c;
    unsigned int i;

    /* gross */
    for(c = getchar();
        c == ' ' ||
        c == '\r' ||
        c == '\n' ||
        c == '\t';
        c = getchar());
    ungetc(c, stdin);

    for(i = 0; i < sizeof(buffer); i++) {
        c = getchar();
        if(c == EOF) {
            buffer[i] = ' ';
            break;
        }
        buffer[i] = c;
        if(c == ' ' ||
           c == '\r' ||
           c == '\n' ||
           c == '\t') {
            break;
        }
    }
    if((i == sizeof(buffer)) ||
       ((i == sizeof(buffer) - 1) &&
        !(buffer[i] == ' ' ||
          buffer[i] == '\r' ||
          buffer[i] == '\n' ||
          buffer[i] == '\t'))) {
        return(0);
    }
    buffer[i] = '\0';
    return(atoi(buffer));
}
