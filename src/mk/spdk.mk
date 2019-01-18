DPDK_ENV_LIB = /home/chenxuqiang/spdk18.10/spdk/dpdk/build/lib
SPDK_ENV_LIB = /home/chenxuqiang/spdk18.10/spdk/build/lib

ISPDK += -I/usr/local/include
SPDK_CFLAGS = -Wl,-z,relro,-z,now -Wl,-z,noexecstack \
			-Wl,--whole-archive -lspdk_copy_ioat -lspdk_ioat \
			-Wl,--no-whole-archive  -Wl,--whole-archive -lspdk_bdev_malloc \
			-lspdk_bdev_null -lspdk_bdev_nvme -lspdk_nvme -lspdk_vbdev_passthru \
			-lspdk_vbdev_error -lspdk_vbdev_gpt -lspdk_vbdev_split \
			-lspdk_vbdev_raid -lspdk_bdev_aio -lspdk_bdev_virtio -lspdk_virtio \
			-Wl,--no-whole-archive -laio -Wl,--whole-archive -lspdk_sock \
			-lspdk_sock_posix  -Wl,--no-whole-archive \
			-Wl,--whole-archive -lspdk_event_bdev -lspdk_event_copy \
			-Wl,--no-whole-archive -lspdk_blobfs -lspdk_blob -lspdk_bdev \
			-lspdk_blob_bdev -lspdk_copy -lspdk_event -lspdk_thread \
			-lspdk_util -lspdk_conf -lspdk_trace -lspdk_log \
			-lspdk_jsonrpc -lspdk_json -lspdk_rpc 
DPDK_CFLAGS = $(SPDK_ENV_LIB)/libspdk_env_dpdk.a \
			-Wl,--whole-archive $(DPDK_ENV_LIB)/librte_eal.a \
			$(DPDK_ENV_LIB)/librte_mempool.a \
			$(DPDK_ENV_LIB)/librte_ring.a \
			$(DPDK_ENV_LIB)/librte_mempool_ring.a \
			$(DPDK_ENV_LIB)/librte_pci.a \
			$(DPDK_ENV_LIB)/librte_bus_pci.a \
			$(DPDK_ENV_LIB)/librte_kvargs.a \
			-Wl,--no-whole-archive -lnuma -ldl -lrt -luuid -lcrypto

SPDK_LIB = -L$(SPDK_ENV_LIB)