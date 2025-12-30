#include "DkmRtpIpc.h"
#include "legacy_agent.h"
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <vector>
#include <chrono>
#include <mutex>
#include <cstdarg>

#ifdef _WIN32
// Windows specific includes are in header
#elif defined(_VXWORKS_)
#include <sockLib.h>
#include <inetLib.h>
#include <hostLib.h>
#include <ioLib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) ::close(s)
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) ::close(s)
#endif

// --- Protocol Definitions ---
#pragma pack(push, 1)
struct Header {
    uint32_t magic;   // 0x52495043 ('RIPC')
    uint16_t version; // 0x0001
    uint16_t type;    // Message Type
    uint32_t corr_id; // Correlation ID
    uint32_t length;  // Payload Length
    uint64_t ts_ns;   // Timestamp
};
#pragma pack(pop)

constexpr uint32_t MAGIC_VALUE = 0x52495043;
constexpr uint16_t PROTO_VERSION = 0x0001;
constexpr uint16_t MSG_FRAME_REQ = 0x1000; // Request frame (payload: CBOR/JSON)

// Helper for 64-bit network byte order
#ifndef htonll
#ifdef _WIN32
#define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32))
#define ntohll(x) ((((uint64_t)ntohl(x)) << 32) + ntohl((x) >> 32))
#else
#define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32))
#define ntohll(x) ((((uint64_t)ntohl(x)) << 32) + ntohl((x) >> 32))
#endif
#endif

static uint64_t now_ns() {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}
// ----------------------------

DkmRtpIpc::DkmRtpIpc() : sock_(INVALID_SOCKET), initialized_(false) {
    memset(&dest_addr_, 0, sizeof(dest_addr_));
}

static void dap_log(int level, const char* fmt, ...) {
    LegacyLogCb cb = nullptr; void* user = nullptr; legacy_agent_get_log_callback(&cb, &user);
    if (!cb) return;
    char buf[512];
    va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof(buf), fmt, ap); va_end(ap);
    cb(level, buf, user);
}

DkmRtpIpc::~DkmRtpIpc() {
    close();
}

bool DkmRtpIpc::init(const char* ip, uint16_t port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        dap_log(4, "[DkmRtpIpc] WSAStartup failed");
        return false;
    }
#endif

    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_ == INVALID_SOCKET) {
        dap_log(4, "[DkmRtpIpc] socket creation failed");
        return false;
    }

    // Setup destination address
    dest_addr_.sin_family = AF_INET;
    dest_addr_.sin_port = htons(port);
#ifdef _WIN32
    inet_pton(AF_INET, ip, &dest_addr_.sin_addr);
#else
    inet_pton(AF_INET, ip, &dest_addr_.sin_addr);
#endif

    // Connect to the server (Agent)
    // Note: For UDP, connect() simply sets the default destination address and filters incoming packets.
    // It does NOT perform a handshake or verify the server exists.
    if (connect(sock_, (struct sockaddr*)&dest_addr_, sizeof(dest_addr_)) == SOCKET_ERROR) {
#ifdef _WIN32
        dap_log(4, "[DkmRtpIpc] connect (set default dest) failed. Error: %d", WSAGetLastError());
#else
        dap_log(4, "[DkmRtpIpc] connect (set default dest) failed. Error: %d", errno);
#endif
        closesocket(sock_);
        return false;
    }

    initialized_ = true;
    // Log via global legacy agent callback if present
    {
        LegacyLogCb cb = nullptr;
        void* user = nullptr;
        legacy_agent_get_log_callback(&cb, &user);
        if (cb) {
            char buf[256];
            snprintf(buf, sizeof(buf), "[DkmRtpIpc] Socket Initialized. Default Destination: %s:%u", ip, port);
            cb(2, buf, user); // level=2 INFO
        }
    }
    return true;
}

void DkmRtpIpc::close() {
    if (sock_ != INVALID_SOCKET) {
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
    }
#ifdef _WIN32
    if (initialized_) {
        WSACleanup();
    }
#endif
    initialized_ = false;
}

bool DkmRtpIpc::send(const void* data, size_t len, uint16_t type, uint32_t corr_id) {
    if (!initialized_ || sock_ == INVALID_SOCKET) return false;

    // Prepare Header
    Header h;
    h.magic = htonl(MAGIC_VALUE);
    h.version = htons(PROTO_VERSION);
    h.type = htons(type); 
    h.corr_id = htonl(corr_id);
    h.length = htonl((uint32_t)len);
    h.ts_ns = htonll(now_ns());

#if defined(_WIN32)
    // For Windows use simple send() (WSASend alternative could be used)
#else
    // Use scatter-gather I/O to avoid copying header+payload into a temporary buffer
#endif
#ifdef DEMO_PERF_INSTRUMENTATION
    auto t0 = std::chrono::steady_clock::now();
#endif
    int sent = 0;
#if defined(_WIN32)
    // Fallback: keep existing simple send on Windows
    std::vector<uint8_t> packet(sizeof(Header) + len);
    memcpy(packet.data(), &h, sizeof(Header));
    if (len > 0) memcpy(packet.data() + sizeof(Header), data, len);
    sent = ::send(sock_, (const char*)packet.data(), (int)packet.size(), 0);
#else
    struct msghdr msg;
    struct iovec iov[2];
    memset(&msg, 0, sizeof(msg));
    iov[0].iov_base = (void*)&h;
    iov[0].iov_len = sizeof(Header);
    iov[1].iov_base = (void*)data;
    iov[1].iov_len = len;
    msg.msg_iov = iov;
    msg.msg_iovlen = 2;
    sent = (int)sendmsg(sock_, &msg, 0);
#endif
    
#ifdef DEMO_PERF_INSTRUMENTATION
    auto t1 = std::chrono::steady_clock::now();
    auto send_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    {
        LegacyLogCb cb = nullptr; void* user = nullptr; legacy_agent_get_log_callback(&cb, &user);
        if (cb) {
            char buf[128];
            snprintf(buf, sizeof(buf), "[PERF] DkmRtpIpc::send() took %lld us (sent=%d)", (long long)send_us, sent);
            cb(1, buf, user); // DEBUG
        }
    }
#ifdef _WIN32
    // no-op
#endif
    // Accumulate transport perf counters
    send_us_total_.fetch_add((uint64_t)send_us);
    send_count_.fetch_add(1);
#endif

    if (sent == SOCKET_ERROR) {
#ifdef _WIN32
        dap_log(4, "[DkmRtpIpc] send failed. Error: %d", WSAGetLastError());
#else
        dap_log(4, "[DkmRtpIpc] send failed. Error: %d", errno);
#endif
        return false;
    }
/* timing/print previously duplicated and removed to avoid redefinition */
    {
        LegacyLogCb cb = nullptr; void* user = nullptr; legacy_agent_get_log_callback(&cb, &user);
        if (cb) {
            char buf[128];
            snprintf(buf, sizeof(buf), "[DkmRtpIpc] Sent %d bytes (Header: %zu + Payload: %zu)", sent, sizeof(Header), len);
            cb(2, buf, user);
        }
    }
    return true;
}

void DkmRtpIpc::getPerfStats(uint64_t* out_send_us_total, uint32_t* out_send_count) const {
    if (out_send_us_total) *out_send_us_total = 0;
    if (out_send_count) *out_send_count = 0;
#ifdef DEMO_PERF_INSTRUMENTATION
    if (out_send_us_total) *out_send_us_total = send_us_total_.load();
    if (out_send_count) *out_send_count = send_count_.load();
#endif
}

int DkmRtpIpc::receive(void* buffer, size_t max_len, int timeout_ms) {
    if (!initialized_ || sock_ == INVALID_SOCKET) return -1;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock_, &readfds);

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int ret = select((int)sock_ + 1, &readfds, NULL, NULL, &tv);
    if (ret > 0) {
        // We need to read the full packet (Header + Payload)
        // But the user buffer might be too small for both.
        // For simplicity, let's peek or read into a temp buffer.
        std::vector<uint8_t> recv_buf(sizeof(Header) + max_len); 

        int bytes = ::recv(sock_, (char*)recv_buf.data(), (int)recv_buf.size(), 0);
        
        if (bytes > 0) {
            if (bytes < sizeof(Header)) {
                    dap_log(4, "[DkmRtpIpc] Received packet too small for header: %d", bytes);
                    return 0; // Ignore invalid packet
                }

            Header* h = (Header*)recv_buf.data();
            uint32_t magic = ntohl(h->magic);
            if (magic != MAGIC_VALUE) {
                dap_log(4, "[DkmRtpIpc] Invalid Magic: 0x%08x", magic);
                return 0; // Ignore invalid packet
            }

            uint32_t payload_len = ntohl(h->length);
              if (bytes - sizeof(Header) < payload_len) {
                  dap_log(4, "[DkmRtpIpc] Incomplete payload. Expected: %u, Got: %d", payload_len, (bytes - (int)sizeof(Header)));
                  return 0;
              }

            // Copy payload to user buffer
            size_t copy_len = (payload_len < max_len) ? payload_len : max_len;
            memcpy(buffer, recv_buf.data() + sizeof(Header), copy_len);
            
            {
                LegacyLogCb cb = nullptr; void* user = nullptr; legacy_agent_get_log_callback(&cb, &user);
                if (cb) {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "[DkmRtpIpc] Recv Valid Packet. Payload: %zu bytes", copy_len);
                    cb(2, buf, user);
                }
            }
            return (int)copy_len;

        } else if (bytes == SOCKET_ERROR) {
#ifdef _WIN32
            int err = WSAGetLastError();
            if (err == WSAECONNRESET) {
                dap_log(3, "[DkmRtpIpc] Error: Agent Port Unreachable (WSAECONNRESET). Agent is NOT running or Port is closed.");
            } else {
                dap_log(4, "[DkmRtpIpc] recv failed. Error: %d", err);
            }
#else
            dap_log(4, "[DkmRtpIpc] recv failed. Error: %d", errno);
#endif
            return -1;
        }
        return bytes;
    } else if (ret == 0) {
        return 0; // Timeout
        } else {
    #ifdef _WIN32
        dap_log(4, "[DkmRtpIpc] select error: %d", WSAGetLastError());
    #else
        dap_log(4, "[DkmRtpIpc] select error: %d", errno);
    #endif
        return -1; // Error
        }
}
