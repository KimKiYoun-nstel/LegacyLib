/**
 * @file data_transport_factory.cpp
 * @brief DataTransport 팩토리 함수 구현
 *
 * Transport 종류에 따라 적절한 구현체를 생성하여 반환한다.
 */

#include "data_transport_iface.hpp"
#include "data_transport_udp.hpp"

#if DDSFW_ENABLE_SHM
#include "data_transport_shm.hpp"
#endif

namespace legacy {
namespace transport {

IDataTransport* create_data_transport(DataTransportKind kind) noexcept {
    switch (kind) {
        case DataTransportKind::UDP:
            return new (std::nothrow) DataTransportUdp();

        case DataTransportKind::SHM:
#if DDSFW_ENABLE_SHM
            return new (std::nothrow) DataTransportShm();
#else
            // SHM은 VxWorks에서만 지원
            return nullptr;
#endif

        default:
            return nullptr;
    }
}

}  // namespace transport
}  // namespace legacy
