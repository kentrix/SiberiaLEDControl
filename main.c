#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libusb.h"
#include <unistd.h>

#define VENDOR_ID 0x1038
#define PRODUCT_ID 0x1211
#define INTERFACE_NUM 3
#define VERSION 0.1
#define TIMEOUT 500

//Don't change these
#define DATA_LENGTH 37
#define REQ_FIELD 0x09
#define VAL_FIELD 0x204

int parse(libusb_device_handle *dev_h, int intensity, char flag, char *mode) {
	if(intensity <= 255 && intensity >= 0) {
		send_data(dev_h, 0x89, intensity);
	}
	else if (intensity != -1) {
		printf("Invalid intensity.\n");
	}
	if(flag) {
		char c1, c2;
		c1 = 0x87;
		if(!strcmp("steady", mode)) {
			c2 = 0x02;	
		}
		else if(!strcmp("slow", mode)) {
			c2 = 0x22;	
		}
		else if(!strcmp("medium", mode)) {
			c2 = 0x26;	
		}
		else if(!strcmp("fast", mode)) {
			c2 = 0x2A;	
		}
		else if(!strcmp("trigger", mode)) {
			c2 = 0x12;	
		}
		else {
			printf("Invalid mode.\n");
		}
		send_data(dev_h, c1, c2);
	}
}


void send_data(libusb_device_handle *dev_h, char c1, char c2) {
	const uint8_t mask = 	LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE;

	char data[37] = {0x04, 0x40, 0x01, 0x11, c1, c2, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00,};
	short byte = libusb_control_transfer(dev_h, mask, REQ_FIELD, VAL_FIELD, INTERFACE_NUM, data, DATA_LENGTH , TIMEOUT);
		if(byte != 37) {
			perror("Error, unable to transmit to device.\n");
			exit(1);
		}

}
	
void print_help_msg(int code) {

	printf("LED Control For Siberia 2"
		"\n"
		"Usage:  name [--intensity=<0-255>] [--mode=<steady|fast|medium|slow|trigger>]\n");
		exit(code);
}

int main(int argc, char **argv) {
	if(argc == 1) {
		print_help_msg(1);
	}
	for(int i = 1; i < argc; i++) {
		if(!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h") ) {
			print_help_msg(0);
		}
	}

	int intensity = -1;
	char mode[8];
	short flag = 0;
	for(int i = 1; i < argc; i++) {
		char *arg = argv[i];
		if((sscanf(arg, "--intensity=%d", &intensity)) == 1) {
		}
		else if((sscanf(arg, "--mode=%s", mode)) == 1) {
			flag = 0x01;
		}
		else {
			printf("Error: Invalid Argument %s\n", arg);
			exit(1);
		}
	}

	if(getuid()) {
		printf("Not running as root, make sure you have some udev rules.\n");
	}



	if(libusb_init(NULL)) {
		perror("Error: libusb can't initialise.\n");
		exit(1);
	}

	//TODO check for all err
	libusb_device_handle *dev_h;

	dev_h = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);
	if(dev_h == NULL) {
		perror("Error: Device not found.\n");
		exit(1);
	}


	if(libusb_kernel_driver_active(dev_h, INTERFACE_NUM)) {
		//detach the driver if active
		if(libusb_detach_kernel_driver(dev_h, 3)) {
			perror("Error: Unable to detach kernel driver.\n");
			exit(1);
		}

	}

	if(libusb_claim_interface(dev_h, 3)) {
		perror("Unable to claim interface.\n");
		exit(1);
	}

	parse(dev_h, intensity, flag, mode);



	if(libusb_release_interface(dev_h, 3)) {
		perror("Error: unable to release interface.\n");
		exit(1);
	}

	if(libusb_attach_kernel_driver(dev_h, 3)) {
		perror("Error: unable to attach kernel driver..\n");
		exit(1);
	}

	libusb_close(dev_h);
	libusb_exit(NULL);
}
