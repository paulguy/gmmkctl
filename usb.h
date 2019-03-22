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
