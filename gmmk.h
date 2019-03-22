#ifndef GMMK_H
#define GMMK_H

#include "usb.h"

#define GMMK_MAX_DEVS (8)
#define GMMK_PACKET_SIZE (64)

typedef struct {
    libusb_context *usb;
    libusb_device_handle *usbDevs[GMMK_MAX_DEVS];
    unsigned int devices;

    unsigned char buffer[GMMK_PACKET_SIZE];
} GMMKState;

typedef struct {
    unsigned char r, g, b;
} GMMKColor;

GMMKState *gmmk_open();
void gmmk_close(GMMKState *s);
int gmmk_setMode(GMMKState *s, int devNum, unsigned char mode);
int gmmk_setBrightness(GMMKState *s, int devNum, unsigned char brightness);
int gmmk_setDelay(GMMKState *s, int devNum, unsigned char delay);
int gmmk_setDirLeft(GMMKState *s, int devNum);
int gmmk_setDirRight(GMMKState *s, int devNum);
int gmmk_setColorful(GMMKState *s, int devNum);
int gmmk_unsetColorful(GMMKState *s, int devNum);
int gmmk_setAnimationColor(GMMKState *s, int devNum,
                           unsigned char r, unsigned char g, unsigned char b);
int gmmk_setRate(GMMKState *s, int devNum, unsigned char rate);
int gmmk_setKeys(GMMKState *s, int devNum, unsigned char start,
                 unsigned char count, const GMMKColor *c);

#endif
