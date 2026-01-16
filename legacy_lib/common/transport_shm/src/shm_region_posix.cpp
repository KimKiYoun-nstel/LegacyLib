/**
 * @file shm_region_posix.cpp
 * @brief SHM Region POSIX 구현 (VxWorks 전용)
 *
 * shm_open/ftruncate/mmap을 사용한 공유 메모리 관리.
 * VxWorks POSIX 호환 계층을 사용한다.
 */

#include "shm_region.hpp"
#include <cstring>
#include <cerrno>

// VxWorks/POSIX 호환 헤더
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace ddsfw {
namespace transport_shm {

ShmRegion::~ShmRegion() {
    close();
}

bool ShmRegion::open(const char* name, size_t size, bool creator) noexcept {
    if (!name || size == 0) {
        return false;
    }

    // 이미 열려있으면 먼저 닫기
    if (ptr_) {
        close();
    }

    // 이름 저장
    std::strncpy(name_, name, sizeof(name_) - 1);
    name_[sizeof(name_) - 1] = '\0';
    creator_ = creator;
    size_ = size;

    int flags = O_RDWR;
    if (creator) {
        flags |= O_CREAT | O_EXCL;
    }

    // shm_open
    fd_ = ::shm_open(name_, flags, 0666);
    if (fd_ < 0) {
        if (creator && errno == EEXIST) {
            // 기존 SHM이 있으면 삭제 후 재생성 시도
            ::shm_unlink(name_);
            fd_ = ::shm_open(name_, O_RDWR | O_CREAT | O_EXCL, 0666);
        }
        if (fd_ < 0) {
            return false;
        }
    }

    // creator인 경우 크기 설정
    if (creator) {
        if (::ftruncate(fd_, static_cast<off_t>(size)) < 0) {
            ::close(fd_);
            ::shm_unlink(name_);
            fd_ = -1;
            return false;
        }
    }

    // mmap
    void* p = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    if (p == MAP_FAILED) {
        ::close(fd_);
        if (creator) {
            ::shm_unlink(name_);
        }
        fd_ = -1;
        return false;
    }

    ptr_ = p;

    // creator인 경우 메모리 초기화
    if (creator) {
        std::memset(ptr_, 0, size_);
    }

    return true;
}

void ShmRegion::close() noexcept {
    if (ptr_) {
        ::munmap(ptr_, size_);
        ptr_ = nullptr;
    }

    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }

    // creator인 경우 SHM 객체 삭제
    if (creator_ && name_[0] != '\0') {
        ::shm_unlink(name_);
        name_[0] = '\0';
    }

    size_ = 0;
    creator_ = false;
}

}  // namespace transport_shm
}  // namespace ddsfw
