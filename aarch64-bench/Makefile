# SPDX-License-Identifier: Apache-2.0

.PHONY: clean

TARGET = bench
CC  ?= gcc
LD  := $(CC)

SRC_C := hal/hal.c bench.c ntt_zetas.c group7/mul512_kara_asm.c group7/mod.c group7/mul512_kara.c
SRC_S := ntt.S group7/schoolbook256_mul.S   # <── 新增 .S

SOURCES := $(SRC_C) $(SRC_S)

CFLAGS := \
	-Wall \
	-Wextra \
	-Werror=unused-result \
	-Wpedantic \
	-Werror \
	-Wmissing-prototypes \
	-Wshadow \
	-Wpointer-arith \
	-Wredundant-decls \
	-Wno-long-long \
	-Wno-unknown-pragmas \
	-Wno-unused-command-line-argument \
	-O3 \
	-fomit-frame-pointer \
	-std=c99 \
	-pedantic \
	-Ihal \
	-MMD \
	$(CFLAGS)

ifeq ($(C_KARA),1)
	CFLAGS += -DC_KARATSUBA
endif

ifeq ($(CYCLES),PMU)
	CFLAGS += -DPMU_CYCLES
endif

ifeq ($(CYCLES),PERF)
	CFLAGS += -DPERF_CYCLES
endif

ifeq ($(CYCLES),MAC)
	CFLAGS += -DMAC_CYCLES
endif

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) -lgmp

clean:
	-$(RM) -rf $(TARGET) *.d