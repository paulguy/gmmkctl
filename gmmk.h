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

#ifndef GMMK_H
#define GMMK_H

#include "usb.h"

#define GMMK_MAX_DEVS (8)
#define GMMK_PACKET_SIZE (64)
#define GMMK_MAX_KEY (126) /* highest key value addressed by Windows utility */

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
int gmmk_setKeys(GMMKState *s, int devNum, unsigned int start,
                 unsigned int count, const GMMKColor *c);

#endif
