#include "memzone/rdma_mz.h"
#include "memzone/MemoryManager.h"
#include "msg/rdma/Infiniband.h"
#include "memzone/RdmaMem.h"

using namespace flame::memory;
using namespace flame::memory::ib;
using namespace flame::msg::ib;

namespace flame {

void RdmaAllocator::init(FlameContext *fct, ProtectionDomain *p, MemoryConfig *_cfg) {
    mmgr = new flame::memory::ib::MemoryManager(fct, p, _cfg);
}

Buffer RdmaAllocator::allocate(size_t sz) {
    RdmaBuffer *rb = mmgr->get_rdma_allocator()->alloc(sz);
    return rb == nullptr ? Buffer() : 
                        Buffer(std::shared_ptr<BufferPtr>(new RdmaBufferPtr(rb, this)));
}

size_t RdmaAllocator::max_size() const {
    return 0;
}

size_t RdmaAllocator::min_size() const {
    return 0;
}

size_t RdmaAllocator::free_size() const {
    return 0;
}

bool RdmaAllocator::empty() const {
    return false;
}

}   //end namespace flame;
