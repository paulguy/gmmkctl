/*
 * Copyright 2021 paulguy <paulguy119@gmail.com>
 *
 * This file is part of gmmkctl.
 *
 * gmmkctl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gmmkctl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gmmkctl.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gmmk.h"

#define GMMK_VENDOR_ID  (0x0C45)
#define GMMK_PRODUCT_ID (0x652F)
#define GMMK_LED_INTERFACE (1)
#define GMMK_OUT_ENDPOINT (0x03)
#define GMMK_IN_ENDPOINT (0x82)
#define GMMK_REPORT_ID (4)
#define GMMK_TIMEOUT_MS (1000)

#define GMMK_SUM_OFFSET (1)
#define GMMK_COMMAND_OFFSET (3)

#define GMMK_CMD_START (1)
#define GMMK_CMD_END (2)

#define GMMK_CMD_KEYCOLORS (0x11)
#define GMMK_KEYCOLORS_COUNT_OFFSET (4)
#define GMMK_KEYCOLORS_START_OFFSET (5)
#define GMMK_KEYCOLORS_DATA_OFFSET (8)
#define GMMK_KEYCOLORS_DATA_SIZE ((GMMK_PACKET_SIZE-GMMK_KEYCOLORS_DATA_OFFSET)/3*3)

#define GMMK_CMD_SUBCOMMAND (0x06)
#define GMMK_SUBCMD_CMD_OFFSET (4)
#define GMMK_SUBCMD_ARG_OFFSET (8)

const char GMMK_SUBCMD_MODE[] = {0x01, 0x00, 0x00, 0x00};
const char GMMK_SUBCMD_BRIGHTNESS[] = {0x01, 0x01, 0x00, 0x00};
const char GMMK_SUBCMD_DELAY[] = {0x01, 0x02, 0x00, 0x00};
const char GMMK_SUBCMD_DIRECTION[] = {0x01, 0x03, 0x00, 0x00};
const char GMMK_SUBCMD_COLORFUL[] = {0x01, 0x04, 0x00, 0x00};
const char GMMK_SUBCMD_ANIMCOLOR[] = {0x03, 0x05, 0x00, 0x00};
const char GMMK_SUBCMD_RATE[] = {0x01, 0x0f, 0x00, 0x00};


GMMKState *gmmk_open() {
    GMMKState *s;
    enum libusb_error e;

    s = malloc(sizeof(GMMKState));
    if(s == NULL) {
        goto error0;
    }

    if((e = libusb_init(&(s->usb))) < 0) {
        fprintf(stderr, "Couldn't init libusb: %s\n", libusb_strerror(e));
        goto error1;
    }

    s->devices = open_libusb_devices(s->usb, s->usbDevs, GMMK_MAX_DEVS, GMMK_VENDOR_ID,
        GMMK_PRODUCT_ID, GMMK_LED_INTERFACE);

    if(s->devices == 0) {
        goto error2;
    }

    return(s);

error2:
    libusb_exit(s->usb);
error1:
    free(s);
error0:
    return(NULL);
}

void gmmk_close(GMMKState *s) {
    unsigned int i;
    enum libusb_error e;

    for(i = 0; i < s->devices; i++) {
        if((e = libusb_release_interface(s->usbDevs[i], GMMK_LED_INTERFACE)) < 0) {
            fprintf(stderr, "WARNING: Couldn't release interface: %s\n", libusb_strerror(e));
        }
        libusb_close(s->usbDevs[i]);
    }

    libusb_exit(s->usb);

    free(s);
}

void sum(unsigned short int *sum, unsigned char *mem, int size) {
    int i;

    *sum = 0;
    for(i = 0; i < size; i++) {
        *sum += (unsigned short int)mem[i];
    }
}

#define TRANSFER_OR_FAIL(BUFFER, USBDEV) \
    BUFFER[0] = 4; \
    sum((unsigned short int *)&(BUFFER[GMMK_SUM_OFFSET]), \
        &(BUFFER[GMMK_COMMAND_OFFSET]), \
        GMMK_PACKET_SIZE - GMMK_COMMAND_OFFSET); \
    if((e = libusb_interrupt_transfer(USBDEV, \
                                      GMMK_OUT_ENDPOINT, \
                                      BUFFER, \
                                      GMMK_PACKET_SIZE, \
                                      &transferred, \
                                      GMMK_TIMEOUT_MS)) < 0) { \
        fprintf(stderr, "Interrupt out failed: %s\n", libusb_strerror(e)); \
        return(-1); \
    } \
    if(transferred < GMMK_PACKET_SIZE) { \
        fprintf(stderr, "WARNING: Short packet sent.\n"); \
    } /* read a packet back */ \
    if((e = libusb_interrupt_transfer(USBDEV, \
                                      GMMK_IN_ENDPOINT, \
                                      BUFFER, \
                                      GMMK_PACKET_SIZE, \
                                      &transferred, \
                                      GMMK_TIMEOUT_MS)) < 0) { \
        fprintf(stderr, "Interrupt in failed: %s\n", libusb_strerror(e)); \
        return(-1); \
    } \
    if(transferred < GMMK_PACKET_SIZE) { \
        fprintf(stderr, "WARNING: Short packet read.\n"); \
    }

#define CLEAR_BUFF(BUFFER) \
    memset(&(BUFFER[GMMK_COMMAND_OFFSET]), 0,\
           GMMK_PACKET_SIZE - GMMK_COMMAND_OFFSET);

#define DO_START(BUFFER, USBDEV) \
    CLEAR_BUFF(BUFFER) \
    BUFFER[GMMK_COMMAND_OFFSET] = GMMK_CMD_START; \
    TRANSFER_OR_FAIL(BUFFER, USBDEV)

#define DO_END(BUFFER, USBDEV) \
    CLEAR_BUFF(BUFFER) \
    BUFFER[GMMK_COMMAND_OFFSET] = GMMK_CMD_END; \
    TRANSFER_OR_FAIL(BUFFER, USBDEV)

#define APPLY_SUBCOMMAND(BUFFER, SCMD, SCARG) \
    BUFFER[GMMK_COMMAND_OFFSET] = GMMK_CMD_SUBCOMMAND; \
    memcpy(&(BUFFER[GMMK_SUBCMD_CMD_OFFSET]), \
           SCMD, \
           sizeof(SCMD)); \
    BUFFER[GMMK_SUBCMD_ARG_OFFSET] = SCARG;

#define DO_SUBCOMMAND(BUFFER, USBDEV, SCMD, SCARG) \
    CLEAR_BUFF(BUFFER) \
    APPLY_SUBCOMMAND(BUFFER, SCMD, SCARG) \
    TRANSFER_OR_FAIL(BUFFER, USBDEV)

#define APPLY_SUBCOMMAND_RGB(BUFFER, SCMD, SCR, SCG, SCB) \
    BUFFER[GMMK_COMMAND_OFFSET] = GMMK_CMD_SUBCOMMAND; \
    memcpy(&(BUFFER[GMMK_SUBCMD_CMD_OFFSET]), \
           SCMD, \
           sizeof(SCMD)); \
    BUFFER[GMMK_SUBCMD_ARG_OFFSET] = SCR; \
    BUFFER[GMMK_SUBCMD_ARG_OFFSET+1] = SCG; \
    BUFFER[GMMK_SUBCMD_ARG_OFFSET+2] = SCB;

#define DO_SUBCOMMAND_RGB(BUFFER, USBDEV, SCMD, SCR, SCG, SCB) \
    CLEAR_BUFF(BUFFER) \
    APPLY_SUBCOMMAND_RGB(BUFFER, SCMD, SCR, SCG, SCB) \
    TRANSFER_OR_FAIL(BUFFER, USBDEV)

int gmmk_setMode(GMMKState *s, int devNum, unsigned char mode) {
    enum libusb_error e;
    int transferred;

    DO_START(s->buffer, s->usbDevs[devNum])
    DO_SUBCOMMAND(s->buffer, s->usbDevs[devNum], GMMK_SUBCMD_MODE, mode)
    DO_END(s->buffer, s->usbDevs[devNum])

    return(0);
}

int gmmk_setBrightness(GMMKState *s, int devNum, unsigned char brightness) {
    enum libusb_error e;
    int transferred;

    DO_START(s->buffer, s->usbDevs[devNum])
    DO_SUBCOMMAND(s->buffer, s->usbDevs[devNum], GMMK_SUBCMD_BRIGHTNESS, brightness)
    DO_END(s->buffer, s->usbDevs[devNum])

    return(0);
}

int gmmk_setDelay(GMMKState *s, int devNum, unsigned char delay) {
    enum libusb_error e;
    int transferred;

    DO_START(s->buffer, s->usbDevs[devNum])
    DO_SUBCOMMAND(s->buffer, s->usbDevs[devNum], GMMK_SUBCMD_DELAY, delay)
    DO_END(s->buffer, s->usbDevs[devNum])

    return(0);
}

int gmmk_setDirLeft(GMMKState *s, int devNum) {
    enum libusb_error e;
    int transferred;

    DO_START(s->buffer, s->usbDevs[devNum])
    DO_SUBCOMMAND(s->buffer, s->usbDevs[devNum], GMMK_SUBCMD_DIRECTION, 0xFF)
    DO_END(s->buffer, s->usbDevs[devNum])

    return(0);
}

int gmmk_setDirRight(GMMKState *s, int devNum) {
    enum libusb_error e;
    int transferred;

    DO_START(s->buffer, s->usbDevs[devNum])
    DO_SUBCOMMAND(s->buffer, s->usbDevs[devNum], GMMK_SUBCMD_DIRECTION, 0)
    DO_END(s->buffer, s->usbDevs[devNum])

    return(0);
}

int gmmk_setColorful(GMMKState *s, int devNum) {
    enum libusb_error e;
    int transferred;

    DO_START(s->buffer, s->usbDevs[devNum])
    DO_SUBCOMMAND(s->buffer, s->usbDevs[devNum], GMMK_SUBCMD_COLORFUL, 1)
    DO_END(s->buffer, s->usbDevs[devNum])

    return(0);
}

int gmmk_unsetColorful(GMMKState *s, int devNum) {
    enum libusb_error e;
    int transferred;

    DO_START(s->buffer, s->usbDevs[devNum])
    DO_SUBCOMMAND(s->buffer, s->usbDevs[devNum], GMMK_SUBCMD_COLORFUL, 0)
    DO_END(s->buffer, s->usbDevs[devNum])

    return(0);
}

int gmmk_setAnimationColor(GMMKState *s, int devNum,
                           unsigned char r, unsigned char g, unsigned char b) {
    enum libusb_error e;
    int transferred;

    DO_START(s->buffer, s->usbDevs[devNum])
    DO_SUBCOMMAND_RGB(s->buffer, s->usbDevs[devNum], GMMK_SUBCMD_ANIMCOLOR, r, g, b)
    DO_END(s->buffer, s->usbDevs[devNum])

    return(0);
}

int gmmk_setRate(GMMKState *s, int devNum, unsigned char rate) {
    enum libusb_error e;
    int transferred;

    DO_START(s->buffer, s->usbDevs[devNum])
    DO_SUBCOMMAND(s->buffer, s->usbDevs[devNum], GMMK_SUBCMD_RATE, rate)
    DO_END(s->buffer, s->usbDevs[devNum])

    return(0);
}

int gmmk_setKeys(GMMKState *s, int devNum, unsigned int start,
                 unsigned int count, const GMMKColor *c) {
    enum libusb_error e;
    int transferred;
    unsigned int i, j;

/*    fprintf(stderr, "%u %u\n", start, count);
*/
    if(start > GMMK_MAX_KEY) {
        return(0);
    }

    if(start + count > GMMK_MAX_KEY) {
        count = GMMK_MAX_KEY - start;
    }

    DO_START(s->buffer, s->usbDevs[devNum])

    for(i = 0; i < count; i += GMMK_KEYCOLORS_DATA_SIZE / 3) {
        CLEAR_BUFF(s->buffer)
        s->buffer[GMMK_COMMAND_OFFSET] = GMMK_CMD_KEYCOLORS;
        s->buffer[GMMK_KEYCOLORS_COUNT_OFFSET] = (count - i) * 3 > GMMK_KEYCOLORS_DATA_SIZE ?
                                                 GMMK_KEYCOLORS_DATA_SIZE :
                                                 (count - i) * 3;
        *(unsigned short int *)(&(s->buffer[GMMK_KEYCOLORS_START_OFFSET])) =
            (start * 3) + (i * 3);
/*        fprintf(stderr, "%u %u\n",
            *(unsigned short int *)(&(s->buffer[GMMK_KEYCOLORS_START_OFFSET])),
            s->buffer[GMMK_KEYCOLORS_COUNT_OFFSET]);
*/
        for(j = 0; j < s->buffer[GMMK_KEYCOLORS_COUNT_OFFSET]; j+=3) {
/*            fprintf(stderr, "%u %u %u\n", c[i+(j/3)].r, c[i+(j/3)].g, c[i+(j/3)].b);
*/            s->buffer[GMMK_KEYCOLORS_DATA_OFFSET+j+0] = c[i+(j/3)].r;
            s->buffer[GMMK_KEYCOLORS_DATA_OFFSET+j+1] = c[i+(j/3)].g;
            s->buffer[GMMK_KEYCOLORS_DATA_OFFSET+j+2] = c[i+(j/3)].b;
        }
        TRANSFER_OR_FAIL(s->buffer, s->usbDevs[devNum])
    }

    DO_END(s->buffer, s->usbDevs[devNum])

    return(0);
}
