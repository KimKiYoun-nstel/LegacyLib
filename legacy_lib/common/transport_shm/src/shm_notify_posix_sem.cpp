/**
 * @file shm_notify_posix_sem.cpp
 * @brief SHM Notify POSIX Named Semaphore 구현 (VxWorks 전용)
 *
 * sem_open/sem_post/sem_timedwait을 사용한 알림 메커니즘.
 * VxWorks POSIX 호환 계층을 사용한다.
 */

#include "shm_notify.hpp"
#include <cstring>
#include <cerrno>

// VxWorks/POSIX 호환 헤더
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>

namespace ddsfw {
namespace transport_shm {

ShmNotify::~ShmNotify() {
    close();
}

bool ShmNotify::open(const char* name, bool creator) noexcept {
    if (!name) {
        return false;
    }

    // 이미 열려있으면 먼저 닫기
    if (sem_) {
        close();
    }

    // 이름 저장
    std::strncpy(name_, name, sizeof(name_) - 1);
    name_[sizeof(name_) - 1] = '\0';
    creator_ = creator;

    sem_t* s = nullptr;

    if (creator) {
        // 기존 세마포어 삭제 시도 (이전 실행 잔재)
        ::sem_unlink(name_);

        // 초기값 0으로 생성 (consumer가 먼저 wait하면 블록)
        s = ::sem_open(name_, O_CREAT | O_EXCL, 0666, 0);
        if (s == SEM_FAILED) {
            return false;
        }
    } else {
        // 기존 세마포어 열기
        s = ::sem_open(name_, 0);
        if (s == SEM_FAILED) {
            return false;
        }
    }

    sem_ = static_cast<void*>(s);
    return true;
}

void ShmNotify::close() noexcept {
    if (sem_) {
        ::sem_close(static_cast<sem_t*>(sem_));
        sem_ = nullptr;
    }

    // creator인 경우 세마포어 삭제
    if (creator_ && name_[0] != '\0') {
        ::sem_unlink(name_);
        name_[0] = '\0';
    }

    creator_ = false;
}

bool ShmNotify::post() noexcept {
    if (!sem_) {
        return false;
    }

    return ::sem_post(static_cast<sem_t*>(sem_)) == 0;
}

bool ShmNotify::wait(uint32_t timeout_ms) noexcept {
    if (!sem_) {
        return false;
    }

    sem_t* s = static_cast<sem_t*>(sem_);

    if (timeout_ms == 0) {
        // 무한 대기
        while (::sem_wait(s) != 0) {
            if (errno != EINTR) {
                return false;
            }
            // EINTR인 경우 재시도
        }
        return true;
    }

    // 타임아웃 대기
    struct timespec ts;
    if (::clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        return false;
    }

    // 밀리초 → 초/나노초 변환
    uint64_t ns = static_cast<uint64_t>(ts.tv_nsec) +
                  static_cast<uint64_t>(timeout_ms) * 1000000ULL;
    ts.tv_sec += static_cast<time_t>(ns / 1000000000ULL);
    ts.tv_nsec = static_cast<long>(ns % 1000000000ULL);

    while (::sem_timedwait(s, &ts) != 0) {
        if (errno == ETIMEDOUT) {
            return false;  // 타임아웃
        }
        if (errno != EINTR) {
            return false;  // 기타 오류
        }
        // EINTR인 경우 재시도
    }

    return true;
}

}  // namespace transport_shm
}  // namespace ddsfw
