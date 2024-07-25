PWD := $(shell pwd)

.PHONY:qca-nss-phy clean

qca-nss-phy:
	export SUBDIR=$(PWD)/src && make -C src/
clean:
	rm -f Module.symvers modules.order
