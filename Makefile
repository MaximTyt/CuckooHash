TARGET=cuckoo_hash
TARGETDIR=./test
DPDKVER=dpdk-24.11.2
DPDK_DIR=./dpdk/dpdk-stable-24.11.2
PATCH_DIR=./patches
INCLUDE_PATH=./dpdk/lib/include
LIB_PATH=./dpdk/lib/lib/x86_64-linux-gnu
LIBS_NAME= -lrte_hash -lrte_eal -lrte_kvargs -lrte_log -lrte_telemetry -lrte_net -lrte_mbuf -lrte_mempool -lrte_ring -lrte_rcu
CC=gcc
CFLAGS= -Wall -Wextra -Wstrict-prototypes -Wdeclaration-after-statement -Wmissing-declarations -Werror

.phony: all clean dpdk

all: info dpdk apply-patches build run

clean:
	rm *.o $(TARGETDIR)/$(TARGET) -r dpdk

run:
	LD_LIBRARY_PATH=$(LIB_PATH) $(TARGETDIR)/$(TARGET) --no-huge

dpdk:
	@echo "Downloading and building DPDK..."
	mkdir dpdk
	mkdir dpdk/lib
	wget -P ./dpdk/ https://fast.dpdk.org/rel/$(DPDKVER).tar.xz
	tar xf ./dpdk/$(DPDKVER).tar.xz -C ./dpdk	
	meson setup $(DPDK_DIR)/setup $(DPDK_DIR) --prefix=${PWD}/dpdk/lib	
	ninja -C $(DPDK_DIR)/setup
	meson install -C ./$(DPDK_DIR)/setup
	
apply-patches:	
	@echo "Applying DPDK patches..."
	patch -f $(INCLUDE_PATH)/rte_atomic_64.h < $(PATCH_DIR)/rte_atomic_64.patch || true
	patch -f $(INCLUDE_PATH)/rte_bitops.h < $(PATCH_DIR)/rte_bitops.patch || true
	patch -f $(INCLUDE_PATH)/rte_jhash.h < $(PATCH_DIR)/rte_jhash.patch || true

build:
	$(CC) $(TARGETDIR)/cuckoo_hash.c -o $(TARGETDIR)/$(TARGET) $(CFLAGS) -I$(INCLUDE_PATH) -L$(LIB_PATH) $(LIBS_NAME)

info:
	cat /etc/os-release
	$(CC) --version
	python3 --version
