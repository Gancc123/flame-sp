
#ifndef CHUNKSTORE_CHUNKSTORE_PRIV_H
#define CHUNKSTORE_CHUNKSTORE_PRIV_H

#include <iostream>
#include <cstdint>

#define OP_STORE_INIT       0x01
#define OP_STORE_MKFS       0x02
#define OP_STORE_LOCK       0x03
#define OP_STORE_RELEASE    0x04
#define OP_STORE_EXIT       0x05

#define OP_META_GET_TOTAL_SIZE      0x21
#define OP_META_GET_USED_SIZE       0x22
#define OP_META_GET_CHUNK_NUMS      0x23

#define OP_CHUNK_CREATE     0x41
#define OP_CHUNK_READ       0x42
#define OP_CHUNK_WRITE      0x43
#define OP_CHUNK_REMOVE     0x44

#define OP_CHUNK_GETINFO    0x60
#define OP_CHUNK_GETSIZE    0x61
#define OP_CHUNK_GETPOLICY  0x62
#define OP_CHUNK_GETEPOCH   0x63

#define OP_OBJ_READ         0x81
#define OP_OBJ_WRITE        0x82
#define OP_OBJ_FLUSH        0x83
#define OP_OBJ_REMOVE       0x84

#define MD_CHUNK_STATUS_DIRTY   0x01
#define MD_CHUNK_STATUS_CLEAN   0x02

#define OP_TYPE_IO              0x01
#define OP_TYPE_ADMIN           0x02
#define OP_TYPE_STORE_ADMIN     0x03
#define OP_TYPE_CHUNK_ADMIN     0x04
#define OP_TYPE_CHUNK_IO        0x05

struct ciocb {
    uint32_t seq;
    uint64_t cid;
    uint8_t opcode;
    void *buffer;
    uint64_t length;
    uint64_t offset;
    uint64_t complete_nums;
};

struct oiocb {
    int fd;
    int opcode;
    void *buffer;
    uint64_t length;
    uint64_t offset;

public:
    void print_arg() {
        std::cout << "fd = " << fd << std::endl;
        std::cout << "opcode = " << opcode << std::endl;
        std::cout << "length = " << length << std::endl;
        std::cout << "offset = " << offset << std::endl;
    }
};

#endif