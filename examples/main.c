#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "legacy_agent.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

// Global Handle (For Demo)
static LEGACY_HANDLE g_agent = NULL;
static SOCKET g_client_sock = INVALID_SOCKET;

// --- Helper for Output ---
void cli_print(const char* fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (g_client_sock != INVALID_SOCKET) {
        send(g_client_sock, buffer, (int)strlen(buffer), 0);
    } else {
        printf("%s", buffer);
    }
}

// --- Callbacks (Simple Print) ---

void on_hello(LEGACY_HANDLE agent, LegacyRequestId req_id, const LegacySimpleResult* res, const LegacyHelloInfo* info, void* user) {
    cli_print("\n[ASYNC] Hello Response: req_id=%u, ok=%d, proto=%d\n> ", req_id, res->ok, info ? info->proto : -1);
}

void on_generic(LEGACY_HANDLE agent, LegacyRequestId req_id, const LegacySimpleResult* res, void* user) {
    // user parameter can be used to identify the command
    const char* cmd_name = (const char*)user; 
    cli_print("\n[ASYNC] %s Response: req_id=%u, ok=%d, msg=%s\n> ", 
        cmd_name ? cmd_name : "Unknown", req_id, res->ok, res->msg ? res->msg : "null");
}

void on_event(LEGACY_HANDLE agent, const LegacyEvent* evt, void* user) {
    cli_print("\n[ASYNC] Event Received: topic=%s, type=%s\nData: %s\n> ", 
        evt->topic, evt->type, evt->data_json);
}

// --- CLI Helpers ---

void print_help() {
    cli_print("\n=== Legacy Agent CLI ===\n");
    cli_print("  connect <ip> <port> : Initialize agent (default: 127.0.0.1 25000)\n");
    cli_print("  hello               : Send Hello\n");
    cli_print("  create_p            : Create Participant\n");
    cli_print("  create_w            : Create Writer (Cannon_Actuator_Signal)\n");
    cli_print("  create_r            : Create Reader (Cannon_Actuator_Signal)\n");
    cli_print("  write               : Write Sample JSON\n");
    cli_print("  clear               : Clear All Entities\n");
    cli_print("  quit                : Exit\n");
    cli_print("========================\n");
}

void process_command(char* line) {
    char cmd[64];
    char arg1[64];
    int port = 25000;
    LegacyConfig config = { "127.0.0.1", 25000, LEGACY_CODEC_JSON, 100, 64*1024, 100, 64*1024 };

    // Parse
    cmd[0] = 0; arg1[0] = 0;
    int n = sscanf(line, "%s %s %d", cmd, arg1, &port);
    if (n < 1) return;

    if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) {
        // Handled by caller or just close socket
        if (g_client_sock != INVALID_SOCKET) {
            closesocket(g_client_sock);
            g_client_sock = INVALID_SOCKET;
        }
    }
    else if (strcmp(cmd, "help") == 0) {
        print_help();
    }
    else if (strcmp(cmd, "connect") == 0) {
        if (g_agent) {
            cli_print("Already connected. Restart app to reconnect.\n");
            return;
        }
        if (n >= 2) {
                static char ip_buf[64];
                strncpy(ip_buf, arg1, sizeof(ip_buf)-1);
                config.agent_ip = ip_buf;
        }
        if (n >= 3) config.agent_port = port;

        LegacyStatus st = legacy_agent_init(&config, &g_agent);
        if (st == LEGACY_OK) {
            cli_print("Agent initialized to %s:%d\n", config.agent_ip, config.agent_port);
        } else {
            cli_print("Failed to init agent: %d\n", st);
        }
    }
    else if (strcmp(cmd, "hello") == 0) {
        if (!g_agent) { cli_print("Not connected.\n"); return; }
        legacy_agent_hello(g_agent, 1000, on_hello, "Hello");
    }
    else if (strcmp(cmd, "create_p") == 0) {
        if (!g_agent) { cli_print("Not connected.\n"); return; }
        LegacyParticipantConfig pcfg = { 0, "NetLib::BaseUdpOnly" };
        legacy_agent_create_participant(g_agent, &pcfg, 1000, on_generic, "CreateParticipant");
    }
    else if (strcmp(cmd, "create_w") == 0) {
        if (!g_agent) { cli_print("Not connected.\n"); return; }
        LegacyWriterConfig wcfg = { 
            0, // domain
            "pub1", // publisher
            "Cannon_Actuator_Signal", // topic
            "P_NSTEL::C_Cannon_Actuator_Signal", // type
            "NstelCustomQosLib::InitialStateProfile" // qos
        };
        legacy_agent_create_writer(g_agent, &wcfg, 1000, on_generic, "CreateWriter");
    }
    else if (strcmp(cmd, "create_r") == 0) {
        if (!g_agent) { cli_print("Not connected.\n"); return; }
        LegacyReaderConfig rcfg = { 
            0, // domain
            "sub1", // subscriber
            "Cannon_Actuator_Signal", // topic
            "P_NSTEL::C_Cannon_Actuator_Signal", // type
            "NstelCustomQosLib::InitialStateProfile" // qos
        };
        legacy_agent_create_reader(g_agent, &rcfg, 1000, on_generic, "CreateReader");
        
        // Subscribe to event when creating reader
        legacy_agent_subscribe_event(g_agent, "Cannon_Actuator_Signal", "P_NSTEL::C_Cannon_Actuator_Signal", on_event, NULL);
    }
    else if (strcmp(cmd, "write") == 0) {
        if (!g_agent) { cli_print("Not connected.\n"); return; }
        // Sample Data (Hardcoded)
        const char* json_data = "{"
            "\"A_armSafetyMainCannonLock\":\"L_ArmSafetyMainCannonLock_NORMAL\","
            "\"A_autoArmPositionComplement\":\"L_ArmPositionType_NORMAL\","
            "\"A_azAngle\":1.0,"
            "\"A_deckClearance\":\"L_DekClearanceType_OUTSIDE\","
            "\"A_e1AngleVelocity\":1.0,"
            "\"A_energyStorage\":\"L_ChangingStatusType_NORMAL\","
            "\"A_mainCannonFixStatus\":\"L_MainCannonFixStatusType_NORMAL\","
            "\"A_mainCannonRestoreComplement\":\"L_MainCannonReturnStatusType_RUNNING\","
            "\"A_manualArmPositionComple\":\"L_ArmPositionType_NORMAL\","
            "\"A_recipientID\":{\"A_instanceId\":1,\"A_resourceId\":1},"
            "\"A_roundGyro\":1.0,"
            "\"A_shutdown\":\"L_CannonDrivingDeviceShutdownType_UNKNOWN\","
            "\"A_sourceID\":{\"A_instanceId\":1,\"A_resourceId\":1},"
            "\"A_timeOfDataGeneration\":{\"A_nanoseconds\":1,\"A_second\":1},"
            "\"A_upDownGyro\":1.0"
        "}";

        LegacyWriteJsonOptions wopt = { 
            "Cannon_Actuator_Signal", 
            "P_NSTEL::C_Cannon_Actuator_Signal", 
            json_data,
            0, // domain
            "pub1", // publisher
            "NstelCustomQosLib::InitialStateProfile" // qos
        };

        legacy_agent_write_json(g_agent, &wopt, 1000, on_generic, "WriteJson");
    }
    else if (strcmp(cmd, "clear") == 0) {
        if (!g_agent) { cli_print("Not connected.\n"); return; }
        legacy_agent_clear_dds_entities(g_agent, 1000, on_generic, "ClearEntities");
    }
    else {
        cli_print("Unknown command: %s\n", cmd);
    }
}

void run_cli_loop(FILE* input_stream) {
    char line[256];
    int line_idx = 0;

    cli_print("Starting CLI Demo...\n");
    print_help();

    while (1) {
        // Safety check: If socket is closed and no stdin fallback, exit loop
        if (g_client_sock == INVALID_SOCKET && input_stream == NULL) {
            break;
        }

        if (line_idx == 0) cli_print("> ");
        
        if (g_client_sock != INVALID_SOCKET) {
            char buf[64];
            int len = recv(g_client_sock, buf, sizeof(buf), 0);
            if (len <= 0) break;
            
            for (int i = 0; i < len; i++) {
                unsigned char c = (unsigned char)buf[i];
                
                // Handle Telnet IAC (0xFF) - extremely naive skip
                if (c == 0xFF) continue; // Skip IAC for now

                if (c == '\n' || c == '\r') {
                    if (line_idx > 0) {
                        line[line_idx] = 0;
                        // Echo newline to client for better UX
                        send(g_client_sock, "\r\n", 2, 0);
                        
                        process_command(line);
                        
                        line_idx = 0;
                        // Check if closed after command (e.g. quit)
                        if (g_client_sock == INVALID_SOCKET) break;
                        cli_print("> ");
                    }
                } 
                else if (c == 0x08 || c == 0x7F) { // Backspace (0x08) or DEL (0x7F)
                    if (line_idx > 0) {
                        line_idx--;
                        // Visual erase: Backspace, Space, Backspace
                        send(g_client_sock, "\b \b", 3, 0);
                    }
                }
                else if (c >= 32 && c <= 126) {
                    if (line_idx < sizeof(line) - 1) {
                        line[line_idx++] = c;
                        // Optional: Local echo if client doesn't do it
                        // send(g_client_sock, (char*)&c, 1, 0); 
                    }
                }
            }
        } else {
            // Read from stdin (Fallback)
            if (!input_stream) break;
            if (!fgets(line, sizeof(line), input_stream)) break;
            line[strcspn(line, "\r\n")] = 0;
            process_command(line);
        }
    }
}

int main(int argc, char* argv[]) {
    int port = 23000; // Default Telnet Port
    
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    // Simple argument check for port
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    printf("Starting Legacy Agent Demo Server on port %d...\n", port);
    
    SOCKET server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) {
        printf("Failed to create socket\n");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("Bind failed\n");
        return 1;
    }

    if (listen(server_sock, 1) == SOCKET_ERROR) {
        printf("Listen failed\n");
        return 1;
    }

    printf("Waiting for connection...\n");

    while (1) {
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);
        SOCKET client = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
        
        if (client == INVALID_SOCKET) {
            printf("Accept failed\n");
            continue;
        }

        printf("Client connected!\n");
        g_client_sock = client;

        // Run CLI Loop
        run_cli_loop(NULL);

        printf("Client disconnected.\n");
        closesocket(client);
        g_client_sock = INVALID_SOCKET;
        
        if (g_agent) {
            legacy_agent_close(g_agent);
            g_agent = NULL;
        }
    }

    closesocket(server_sock);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
