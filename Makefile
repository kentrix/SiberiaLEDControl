INSTALL = /bin/env install -c
DEL = /bin/rm 
DELDIR = /bin/rm -r


main.o : 
	gcc -I /usr/include/libusb-1.0/ -lusb-1.0 main.c -o sled


.PHONY: install
install : sled
	$(INSTALL) sled /usr/local/bin/sled

.PHONY: uninstall
uninstall : 
	$(DEL) /usr/local/bin/sled

.PHONY: clean

