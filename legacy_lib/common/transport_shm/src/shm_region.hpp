/**
 * @file shm_region.hpp
 * @brief SHM Region 관리 인터페이스 (내부용)
 *
 * POSIX shm_open/mmap 기반 공유 메모리 영역 관리.
 * VxWorks POSIX 호환 API를 사용한다.
 */
#pragma once

#include <cstdint>
#include <cstddef>

namespace ddsfw {
namespace transport_shm {

/**
 * @class ShmRegion
 * @brief 공유 메모리 영역 관리 클래스
 *
 * creator 모드: shm_open(O_CREAT) + ftruncate + mmap
 * joiner 모드: shm_open(기존) + mmap
 */
class ShmRegion {
public:
    ShmRegion() = default;
    ~ShmRegion();

    // Non-copyable
    ShmRegion(const ShmRegion&) = delete;
    ShmRegion& operator=(const ShmRegion&) = delete;

    /**
     * @brief SHM 영역 열기/생성
     *
     * @param name SHM 객체 이름 (예: "/dds_fw_data")
     * @param size 필요한 크기 (바이트)
     * @param creator true이면 생성, false이면 기존에 조인
     * @return 성공 시 true
     */
    bool open(const char* name, size_t size, bool creator) noexcept;

    /**
     * @brief SHM 영역 닫기 및 unmap
     * @details creator인 경우 shm_unlink도 수행
     */
    void close() noexcept;

    /**
     * @brief 매핑된 메모리 포인터
     * @return 매핑된 주소 (nullptr이면 열리지 않음)
     */
    void* ptr() const noexcept { return ptr_; }

    /**
     * @brief 매핑된 크기
     */
    size_t size() const noexcept { return size_; }

    /**
     * @brief 열린 상태 확인
     */
    bool is_open() const noexcept { return ptr_ != nullptr; }

    /**
     * @brief creator 여부
     */
    bool is_creator() const noexcept { return creator_; }

private:
    void* ptr_ = nullptr;
    size_t size_ = 0;
    int fd_ = -1;
    bool creator_ = false;
    char name_[64] = {};
};

}  // namespace transport_shm
}  // namespace ddsfw
