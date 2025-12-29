#include "DkmRtpIpc.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <chrono>

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

DkmRtpIpc::~DkmRtpIpc() {
    close();
}

bool DkmRtpIpc::init(const char* ip, uint16_t port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[DkmRtpIpc] WSAStartup failed" << std::endl;
        return false;
    }
#endif

    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_ == INVALID_SOCKET) {
        std::cerr << "[DkmRtpIpc] socket creation failed" << std::endl;
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
        std::cerr << "[DkmRtpIpc] connect (set default dest) failed. Error: " << WSAGetLastError() << std::endl;
#else
        std::cerr << "[DkmRtpIpc] connect (set default dest) failed. Error: " << errno << std::endl;
#endif
        closesocket(sock_);
        return false;
    }

    initialized_ = true;
    std::cout << "[DkmRtpIpc] Socket Initialized. Default Destination: " << ip << ":" << port << std::endl;
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

    // Combine Header + Payload
    std::vector<uint8_t> packet(sizeof(Header) + len);
    memcpy(packet.data(), &h, sizeof(Header));
    if (len > 0) {
        memcpy(packet.data() + sizeof(Header), data, len);
    }

    // Use send() since we are connected
    
#ifdef DEMO_TIMING_INSTRUMENTATION
    auto t0 = std::chrono::steady_clock::now();
#endif
    int sent = ::send(sock_, (const char*)packet.data(), (int)packet.size(), 0);
    
    if (sent == SOCKET_ERROR) {
#ifdef _WIN32
        std::cerr << "[DkmRtpIpc] send failed. Error: " << WSAGetLastError() << std::endl;
#else
        std::cerr << "[DkmRtpIpc] send failed. Error: " << errno << std::endl;
#endif
        return false;
    }
#ifdef DEMO_TIMING_INSTRUMENTATION
    auto t1 = std::chrono::steady_clock::now();
    auto send_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cerr << "[TIMING] DkmRtpIpc::send() took " << send_us << " us (sent=" << sent << ")\n";
#endif
    std::cout << "[DkmRtpIpc] Sent " << sent << " bytes (Header: " << sizeof(Header) << " + Payload: " << len << ")" << std::endl;
    return true;
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
                std::cerr << "[DkmRtpIpc] Received packet too small for header: " << bytes << std::endl;
                return 0; // Ignore invalid packet
            }

            Header* h = (Header*)recv_buf.data();
            uint32_t magic = ntohl(h->magic);
            if (magic != MAGIC_VALUE) {
                std::cerr << "[DkmRtpIpc] Invalid Magic: " << std::hex << magic << std::dec << std::endl;
                return 0; // Ignore invalid packet
            }

            uint32_t payload_len = ntohl(h->length);
            if (bytes - sizeof(Header) < payload_len) {
                 std::cerr << "[DkmRtpIpc] Incomplete payload. Expected: " << payload_len << ", Got: " << (bytes - sizeof(Header)) << std::endl;
                 return 0;
            }

            // Copy payload to user buffer
            size_t copy_len = (payload_len < max_len) ? payload_len : max_len;
            memcpy(buffer, recv_buf.data() + sizeof(Header), copy_len);
            
            std::cout << "[DkmRtpIpc] Recv Valid Packet. Payload: " << copy_len << " bytes" << std::endl;
            return (int)copy_len;

        } else if (bytes == SOCKET_ERROR) {
#ifdef _WIN32
            int err = WSAGetLastError();
            if (err == WSAECONNRESET) {
                std::cerr << "[DkmRtpIpc] Error: Agent Port Unreachable (WSAECONNRESET). Agent is NOT running or Port is closed." << std::endl;
            } else {
                std::cerr << "[DkmRtpIpc] recv failed. Error: " << err << std::endl;
            }
#else
            std::cerr << "[DkmRtpIpc] recv failed. Error: " << errno << std::endl;
#endif
            return -1;
        }
        return bytes;
    } else if (ret == 0) {
        return 0; // Timeout
    } else {
#ifdef _WIN32
        std::cerr << "[DkmRtpIpc] select error: " << WSAGetLastError() << std::endl;
#else
        std::cerr << "[DkmRtpIpc] select error: " << errno << std::endl;
#endif
        return -1; // Error
    }
}
