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

#ifndef CG_USB_H
#define CG_USB_H

#include <libusb.h>

int open_libusb_devices(libusb_context *ctx,
                        libusb_device_handle **devs,
                        unsigned int maxDevs,
                        unsigned short int idVendor,
                        unsigned short int idProduct,
                        int interface);

#endif
