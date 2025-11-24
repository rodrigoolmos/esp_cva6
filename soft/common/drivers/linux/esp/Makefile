# Copyright (c) 2011-2025 Columbia University, System Level Design Group
# SPDX-License-Identifier: Apache-2.0
include ../driver.mk

KBUILD_EXTRA_SYMBOLS := $(shell pwd)/../esp_cache/Module.symvers $(shell pwd)/../contig_alloc/Module.symvers
export KBUILD_EXTRA_SYMBOLS

CFLAGS := -Wall -Werror -O3 -I$(DRIVERS)/include -I$(DRIVERS)/../common/include -L.

Module.symvers:
	$(MAKE) -C ../esp_cache ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) DRIVERS=$(DRIVERS)
	$(MAKE) -C ../contig_alloc ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) DRIVERS=$(DRIVERS)
	cat ../esp_cache/$@ > $@
	cat ../contig_alloc/$@ >> $@
