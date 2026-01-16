/**
 * @file shm_log.hpp
 * @brief TransportSHM 로깅 콜백 인터페이스
 *
 * 공용모듈은 Agent/Legacy의 로깅 시스템에 의존하지 않고,
 * 콜백 함수 포인터를 통해 로그를 전달한다.
 */
#pragma once

namespace ddsfw {
namespace transport_shm {

/**
 * @brief 로그 레벨 정의
 */
enum class LogLevel {
    Debug,  ///< 디버그 정보
    Info,   ///< 일반 정보
    Warn,   ///< 경고
    Error   ///< 오류
};

/**
 * @brief 로그 콜백 함수 타입
 *
 * @param level 로그 레벨
 * @param msg 로그 메시지 (null-terminated)
 *
 * @note noexcept 보장 필수 - 로깅 실패가 Transport 동작에 영향을 주면 안 됨
 */
using LogFn = void (*)(LogLevel level, const char* msg) noexcept;

/**
 * @brief 내부 로그 헬퍼 매크로용 함수
 * @details Transport 내부에서 사용하는 로그 래퍼
 */
inline void log_msg(LogFn fn, LogLevel level, const char* msg) noexcept {
    if (fn) {
        fn(level, msg);
    }
}

}  // namespace transport_shm
}  // namespace ddsfw
