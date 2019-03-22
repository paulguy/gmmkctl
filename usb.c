#include <stdio.h>

#include "usb.h"

int open_libusb_devices(libusb_context *ctx,
                        libusb_device_handle **devs,
                        unsigned int maxDevs,
                        unsigned short int idVendor,
                        unsigned short int idProduct,
                        int interface) {
    enum libusb_error e;
    libusb_device **devList;
    struct libusb_device_descriptor desc;
    int i;
    unsigned int devNum;

    if((e = libusb_get_device_list(ctx, &devList)) < 0) {
        fprintf(stderr, "Couldn't get device list: %s\n", libusb_strerror(e));
        return(0);
    }

    devNum = 0;
    for(i = 0; devList[i] != NULL; i++) {
        if((e = libusb_get_device_descriptor(devList[i], &desc)) < 0) {
            fprintf(stderr, "Couldn't get device descriptor: %s\n", libusb_strerror(e));
            continue;
        }

        if(desc.idVendor == idVendor && desc.idProduct == idProduct) {
            if((e = libusb_open(devList[i], &(devs[devNum]))) < 0) {
                fprintf(stderr, "Couldn't open device: %s\n", libusb_strerror(e));
                continue;
            }

            if((e = libusb_detach_kernel_driver(devs[devNum], interface)) < 0) {
                if(e == LIBUSB_ERROR_NOT_FOUND) {
                    /* don't do anything, there's no driver to detach. */
                } else if(e == LIBUSB_ERROR_NOT_SUPPORTED) {
                    fprintf(stderr, "WARNING: Couldn't try to detach kernel driver!\n");
                } else {
                    fprintf(stderr, "Couldn't detach driver: %s\n", libusb_strerror(e));
                    libusb_close(devs[devNum]);
                    continue;
                }
            }

            if((e = libusb_claim_interface(devs[devNum], interface)) < 0) {
                fprintf(stderr, "Couldn't claim interface: %s\n", libusb_strerror(e));
                libusb_close(devs[devNum]);
                continue;
            }

            devNum++;
            if(devNum == maxDevs) {
                break;
            }
        }
    }

    libusb_free_device_list(devList, 1);

    return(devNum);
}
