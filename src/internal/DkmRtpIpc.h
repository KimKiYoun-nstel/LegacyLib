#pragma once
#include <cstdint>
#include <cstddef>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef SOCKET DkmSocket;
#elif defined(_VXWORKS_)
#include <sockLib.h>
#include <inetLib.h>
#include <in.h>
#include <netinet/in.h>
typedef int DkmSocket;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int DkmSocket;
#endif

class DkmRtpIpc {
public:
    DkmRtpIpc();
    ~DkmRtpIpc();

    bool init(const char* ip, uint16_t port);
    void close();
    
    bool send(const void* data, size_t len, uint16_t type = 0x1000, uint32_t corr_id = 0);
    int receive(void* buffer, size_t max_len, int timeout_ms);

private:
    DkmSocket sock_;
    struct sockaddr_in dest_addr_;
    bool initialized_;
};
