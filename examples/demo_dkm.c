/*
 * Legacy Agent DKM Demo - TCP CLI Server with JSON Caching
 * 
 * Features:
 *  - JSON sample data caching from directory
 *  - Parameterized CLI commands for DDS entities
 *  - Type-to-filename mapping (P_NSTEL::C_Type -> P_NSTEL__C_Type.json)
 *
 * Usage in VxWorks Shell:
 *   -> ld < demo_tcp_cli_dkm.out
 *   -> legacyAgentTcpStart(23000, NULL)              # Use default JSON dir
 *   -> legacyAgentTcpStart(23000, "examples/output") # Custom JSON dir
 *   -> # From external: telnet <target_ip> 23000
 *   -> legacyAgentTcpStop()
 *   -> unld "demo_tcp_cli_dkm.out"
 */

#include <vxWorks.h>
#include <taskLib.h>
#include <sysLib.h>
#include <sockLib.h>
#include <inetLib.h>
#include <ioLib.h>
#include <stdioLib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include "legacy_agent.h"

// JSON Cache structures
#define MAX_JSON_CACHE_ENTRIES 64
#define MAX_JSON_SIZE 8192
#define MAX_TYPE_NAME_LEN 128

typedef struct {
    char type_name[MAX_TYPE_NAME_LEN];  // "P_NSTEL::C_Cannon_Actuator_Signal"
    char* json_data;                     // Dynamically allocated JSON string
    size_t json_len;                     // Length of JSON data
    int in_use;                          // Entry is occupied
} JsonCacheEntry;

// Global state
static LEGACY_HANDLE g_agent = NULL;
static int g_server_sock = -1;
static int g_client_sock = -1;
static TASK_ID g_server_task = TASK_ID_ERROR;
static volatile int g_running = 0;
static SEM_ID g_cli_mutex = NULL;

// JSON Cache globals
static JsonCacheEntry g_json_cache[MAX_JSON_CACHE_ENTRIES];
static int g_json_cache_count = 0;
static char g_json_dir_path[256] = "examples/output";
static SEM_ID g_cache_mutex = NULL;

// --- JSON Cache Functions ---

// Convert "P_NSTEL::C_Cannon_Actuator_Signal" -> "P_NSTEL__C_Cannon_Actuator_Signal.json"
static void type_to_filename(const char* type_name, char* filename, size_t filename_size) {
    const char* src = type_name;
    char* dst = filename;
    size_t written = 0;
    
    while (*src && written < filename_size - 6) {  // Leave space for ".json\0"
        if (*src == ':' && *(src+1) == ':') {
            *dst++ = '_';
            *dst++ = '_';
            src += 2;
            written += 2;
        } else {
            *dst++ = *src++;
            written++;
        }
    }
    
    strncpy(dst, ".json", filename_size - written);
    filename[filename_size - 1] = '\0';
}

static int load_json_file(const char* filepath, const char* type_name) {
    int fd = -1;
    char* buffer = NULL;
    int bytes_read = 0;
    int result = -1;
    
    if (g_json_cache_count >= MAX_JSON_CACHE_ENTRIES) {
        printf("[JSON Cache] Cache full, cannot load %s\n", filepath);
        return -1;
    }
    
    fd = open(filepath, O_RDONLY, 0);
    if (fd < 0) {
        printf("[JSON Cache] Failed to open: %s\n", filepath);
        return -1;
    }
    
    buffer = (char*)malloc(MAX_JSON_SIZE);
    if (!buffer) {
        printf("[JSON Cache] Memory allocation failed for %s\n", filepath);
        close(fd);
        return -1;
    }
    
    bytes_read = read(fd, buffer, MAX_JSON_SIZE - 1);
    close(fd);
    
    if (bytes_read <= 0) {
        printf("[JSON Cache] Failed to read: %s\n", filepath);
        free(buffer);
        return -1;
    }
    
    buffer[bytes_read] = '\0';
    
    // Find empty cache slot
    semTake(g_cache_mutex, WAIT_FOREVER);
    for (int i = 0; i < MAX_JSON_CACHE_ENTRIES; i++) {
        if (!g_json_cache[i].in_use) {
            strncpy(g_json_cache[i].type_name, type_name, MAX_TYPE_NAME_LEN - 1);
            g_json_cache[i].type_name[MAX_TYPE_NAME_LEN - 1] = '\0';
            g_json_cache[i].json_data = buffer;
            g_json_cache[i].json_len = bytes_read;
            g_json_cache[i].in_use = 1;
            g_json_cache_count++;
            result = 0;
            break;
        }
    }
    semGive(g_cache_mutex);
    
    if (result == 0) {
        printf("[JSON Cache] Loaded: %s -> %s (%d bytes)\n", type_name, filepath, bytes_read);
    } else {
        free(buffer);
    }
    
    return result;
}

static int load_json_directory(const char* dir_path) {
    DIR* dir = NULL;
    struct dirent* entry = NULL;
    char filepath[512];
    char type_name[MAX_TYPE_NAME_LEN];
    int loaded_count = 0;
    
    printf("[JSON Cache] Loading JSON files from: %s\n", dir_path);
    
    dir = opendir(dir_path);
    if (!dir) {
        printf("[JSON Cache] Failed to open directory: %s\n", dir_path);
        return -1;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        // Check for .json extension
        int name_len = strlen(entry->d_name);
        if (name_len < 6 || strcmp(entry->d_name + name_len - 5, ".json") != 0) {
            continue;
        }
        
        // Build full path
        snprintf(filepath, sizeof(filepath), "%s/%s", dir_path, entry->d_name);
        
        // Extract type name from filename: "P_NSTEL__C_..." -> "P_NSTEL::C_..."
        strncpy(type_name, entry->d_name, MAX_TYPE_NAME_LEN - 1);
        type_name[MAX_TYPE_NAME_LEN - 1] = '\0';
        
        // Remove .json extension
        char* ext = strstr(type_name, ".json");
        if (ext) *ext = '\0';
        
        // Convert __ back to ::
        for (char* p = type_name; *p && *(p+1); p++) {
            if (*p == '_' && *(p+1) == '_') {
                *p = ':';
                *(p+1) = ':';
                p++;  // Skip the second colon to avoid reprocessing
            }
        }
        
        if (load_json_file(filepath, type_name) == 0) {
            loaded_count++;
        }
    }
    
    closedir(dir);
    printf("[JSON Cache] Loaded %d JSON files\n", loaded_count);
    return loaded_count;
}

static const char* get_cached_json(const char* type_name) {
    const char* result = NULL;
    
    semTake(g_cache_mutex, WAIT_FOREVER);
    for (int i = 0; i < MAX_JSON_CACHE_ENTRIES; i++) {
        if (g_json_cache[i].in_use && 
            strcmp(g_json_cache[i].type_name, type_name) == 0) {
            result = g_json_cache[i].json_data;
            break;
        }
    }
    semGive(g_cache_mutex);
    
    return result;
}

static void clear_json_cache(void) {
    semTake(g_cache_mutex, WAIT_FOREVER);
    for (int i = 0; i < MAX_JSON_CACHE_ENTRIES; i++) {
        if (g_json_cache[i].in_use) {
            free(g_json_cache[i].json_data);
            g_json_cache[i].json_data = NULL;
            g_json_cache[i].in_use = 0;
        }
    }
    g_json_cache_count = 0;
    semGive(g_cache_mutex);
    printf("[JSON Cache] Cleared\n");
}

// --- Helper for Output ---
void cli_print(const char* fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (g_cli_mutex) semTake(g_cli_mutex, WAIT_FOREVER);
    
    if (g_client_sock >= 0) {
        // Send line by line with \r\n for proper Windows terminal display
        char* line_start = buffer;
        char* line_end;
        while (*line_start) {
            line_end = strchr(line_start, '\n');
            if (line_end) {
                // Send line content (without \n)
                if (line_end > line_start) {
                    send(g_client_sock, line_start, line_end - line_start, 0);
                }
                // Send \r\n for proper Windows terminal line ending
                send(g_client_sock, "\r\n", 2, 0);
                line_start = line_end + 1;
                taskDelay(1);  // Small delay between lines for network transmission
            } else {
                // Send remaining text without newline
                if (*line_start) {
                    send(g_client_sock, line_start, strlen(line_start), 0);
                }
                break;
            }
        }
    } else {
        printf("%s", buffer);
    }
    
    if (g_cli_mutex) semGive(g_cli_mutex);
}

// --- Callbacks ---
void on_hello(LEGACY_HANDLE agent, LegacyRequestId req_id, const LegacySimpleResult* res, const LegacyHelloInfo* info, void* user) {
    cli_print("\n[ASYNC] Hello Response: req_id=%u, ok=%d, proto=%d\n> ", req_id, res->ok, info ? info->proto : -1);
}

void on_generic(LEGACY_HANDLE agent, LegacyRequestId req_id, const LegacySimpleResult* res, void* user) {
    const char* cmd_name = (const char*)user;
    cli_print("\n[ASYNC] %s Response: req_id=%u, ok=%d, msg=%s\n> ", 
        cmd_name ? cmd_name : "Unknown", req_id, res->ok, res->msg ? res->msg : "null");
}

void on_event(LEGACY_HANDLE agent, const LegacyEvent* evt, void* user) {
    cli_print("\n[ASYNC] Event Received: topic=%s, type=%s\nData: %s\n> ", 
        evt->topic, evt->type, evt->data_json);
}

void on_log(int level, const char* msg, void* user) {
    // Redirect library logs to CLI
    cli_print("%s\n", msg);
}

// --- CLI Helpers ---

// Simple argument tokenizer for command parsing
static int tokenize_command(char* line, char** tokens, int max_tokens) {
    int count = 0;
    char* token = strtok(line, " \t");
    
    while (token != NULL && count < max_tokens) {
        tokens[count++] = token;
        token = strtok(NULL, " \t");
    }
    
    return count;
}

void print_help() {
    cli_print(
        "\n=== Legacy Agent CLI (DKM) ===\n"
        "Connection:\n"
        "  connect <ip> <port>          : Initialize agent (default: 127.0.0.1 25000)\n"
        "  hello                        : Send Hello\n"
        "\nEntity Management:\n"
        "  create_p [domain] [qos]      : Create Participant (default: domain=0, qos=BaseUdpOnly)\n"
        "  create_w <topic> <type> [entity_name] [qos]\n"
        "                               : Create Writer (default: entity_name=pub1)\n"
        "  create_r <topic> <type> [entity_name] [qos]\n"
        "                               : Create Reader (default: entity_name=sub1)\n"
        "  write <topic> <type> [file_path]\n"
        "                               : Write data (uses cached JSON or custom file)\n"
        "  clear                        : Clear All Entities\n"
        "\nCache Management:\n"
        "  cache_info                   : Show cached JSON files\n"
        "  cache_reload                 : Reload JSON cache from directory\n"
        "\nOther:\n"
        "  quit                         : Disconnect\n"
        "===============================\n"
        "Examples:\n"
        "  create_p\n"
        "  create_p 0 NetLib::BaseUdpOnly\n"
        "  create_w Cannon_Signal P_NSTEL::C_Cannon_Actuator_Signal\n"
        "  write Cannon_Signal P_NSTEL::C_Cannon_Actuator_Signal\n"
        "  write Cannon_Signal P_NSTEL::C_Cannon_Actuator_Signal /tmp/custom.json\n"
        "===============================\n"
    );
}

void process_command(char* line) {
    static LegacyConfig config = { "127.0.0.1", 25000, 100, 64*1024, 100, 64*1024, on_log, NULL };
    
    char* tokens[16];
    int token_count = tokenize_command(line, tokens, 16);
    
    if (token_count == 0) return;
    
    char* cmd = tokens[0];
    
    if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) {
        if (g_client_sock >= 0) {
            close(g_client_sock);
            g_client_sock = -1;
        }
    }
    else if (strcmp(cmd, "help") == 0) {
        print_help();
    }
    else if (strcmp(cmd, "connect") == 0) {
        if (g_agent) {
            cli_print("Already connected. Disconnect first.\n");
            return;
        }
        if (token_count >= 2) {
            static char ip_buf[64];
            strncpy(ip_buf, tokens[1], sizeof(ip_buf)-1);
            ip_buf[sizeof(ip_buf)-1] = '\0';
            config.agent_ip = ip_buf;
        }
        if (token_count >= 3) config.agent_port = atoi(tokens[2]);

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
        
        int domain = (token_count >= 2) ? atoi(tokens[1]) : 0;
        const char* qos = (token_count >= 3) ? tokens[2] : "NetLib::BaseUdpOnly";
        
        LegacyParticipantConfig pcfg = { domain, qos };
        legacy_agent_create_participant(g_agent, &pcfg, 1000, on_generic, "CreateParticipant");
    }
    else if (strcmp(cmd, "create_w") == 0) {
        if (!g_agent) { cli_print("Not connected.\n"); return; }
        
        if (token_count < 3) {
            cli_print("Usage: create_w <topic> <type> [entity_name] [qos]\n");
            return;
        }
        
        const char* topic = tokens[1];
        const char* type = tokens[2];
        const char* entity_name = (token_count >= 4) ? tokens[3] : "pub1";
        const char* qos = (token_count >= 5) ? tokens[4] : "NstelCustomQosLib::InitialStateProfile";
        
        LegacyWriterConfig wcfg = { 0, entity_name, topic, type, qos };
        legacy_agent_create_writer(g_agent, &wcfg, 1000, on_generic, "CreateWriter");
    }
    else if (strcmp(cmd, "create_r") == 0) {
        if (!g_agent) { cli_print("Not connected.\n"); return; }
        
        if (token_count < 3) {
            cli_print("Usage: create_r <topic> <type> [entity_name] [qos]\n");
            return;
        }
        
        const char* topic = tokens[1];
        const char* type = tokens[2];
        const char* entity_name = (token_count >= 4) ? tokens[3] : "sub1";
        const char* qos = (token_count >= 5) ? tokens[4] : "NstelCustomQosLib::InitialStateProfile";
        
        LegacyReaderConfig rcfg = { 0, entity_name, topic, type, qos };
        legacy_agent_create_reader(g_agent, &rcfg, 1000, on_generic, "CreateReader");
        legacy_agent_subscribe_event(g_agent, topic, type, on_event, NULL);
    }
    else if (strcmp(cmd, "write") == 0) {
        if (!g_agent) { cli_print("Not connected.\n"); return; }
        
        if (token_count < 3) {
            cli_print("Usage: write <topic> <type> [file_path]\n");
            return;
        }
        
        const char* topic = tokens[1];
        const char* type = tokens[2];
        const char* json_data = NULL;
        char* custom_buffer = NULL;
        
        if (token_count >= 4) {
            // Load from custom file path
            const char* file_path = tokens[3];
            int fd = open(file_path, O_RDONLY, 0);
            if (fd < 0) {
                cli_print("Failed to open file: %s\n", file_path);
                return;
            }
            
            custom_buffer = (char*)malloc(MAX_JSON_SIZE);
            if (!custom_buffer) {
                cli_print("Memory allocation failed\n");
                close(fd);
                return;
            }
            
            int bytes = read(fd, custom_buffer, MAX_JSON_SIZE - 1);
            close(fd);
            
            if (bytes <= 0) {
                cli_print("Failed to read file: %s\n", file_path);
                free(custom_buffer);
                return;
            }
            
            custom_buffer[bytes] = '\0';
            json_data = custom_buffer;
        } else {
            // Use cached JSON
            json_data = get_cached_json(type);
            if (!json_data) {
                cli_print("No cached JSON for type: %s\n", type);
                cli_print("Use: write <topic> <type> <file_path>\n");
                return;
            }
        }
        
        LegacyWriteJsonOptions wopt = { 
            topic, type, json_data, 0, "pub1",
            "NstelCustomQosLib::InitialStateProfile"
        };
        
        legacy_agent_write_json(g_agent, &wopt, 1000, on_generic, "WriteJson");
        
        if (custom_buffer) {
            free(custom_buffer);
        }
    }
    else if (strcmp(cmd, "cache_info") == 0) {
        cli_print("\n=== JSON Cache Info ===\n");
        cli_print("Directory: %s\n", g_json_dir_path);
        cli_print("Entries: %d / %d\n\n", g_json_cache_count, MAX_JSON_CACHE_ENTRIES);
        
        semTake(g_cache_mutex, WAIT_FOREVER);
        for (int i = 0; i < MAX_JSON_CACHE_ENTRIES; i++) {
            if (g_json_cache[i].in_use) {
                cli_print("  [%2d] %s (%zu bytes)\n", 
                    i, g_json_cache[i].type_name, g_json_cache[i].json_len);
            }
        }
        semGive(g_cache_mutex);
        cli_print("========================\n");
    }
    else if (strcmp(cmd, "cache_reload") == 0) {
        clear_json_cache();
        load_json_directory(g_json_dir_path);
    }
    else if (strcmp(cmd, "clear") == 0) {
        if (!g_agent) { cli_print("Not connected.\n"); return; }
        legacy_agent_clear_dds_entities(g_agent, 1000, on_generic, "ClearEntities");
    }
    else {
        cli_print("Unknown command: %s (type 'help' for commands)\n", cmd);
    }
}

// TCP Server Task
void tcpServerTask(void) {
    struct sockaddr_in addr;
    
    printf("[DKM Demo] TCP Server Task started\n");
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(23000); // Hardcoded for now, can be parameterized
    
    if (bind(g_server_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("[DKM Demo] Bind failed\n");
        g_running = 0;
        return;
    }
    
    if (listen(g_server_sock, 1) < 0) {
        printf("[DKM Demo] Listen failed\n");
        g_running = 0;
        return;
    }
    
    printf("[DKM Demo] Listening on port 23000...\n");
    
    while (g_running) {
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);
        
        printf("[DKM Demo] Waiting for client...\n");
        g_client_sock = accept(g_server_sock, (struct sockaddr*)&client_addr, (socklen_t*)&addr_len);
        
        if (g_client_sock < 0) {
            if (g_running) {
                printf("[DKM Demo] Accept failed\n");
            }
            break;
        }
        
        printf("[DKM Demo] Client connected!\n");
        
        // CLI Loop
        char line[256];
        int line_idx = 0;
        
        // Combined welcome message
        cli_print(
            "Legacy Agent DKM Demo\n"
            "\n=== Legacy Agent CLI (DKM) ===\n"
            "  connect <ip> <port> : Initialize agent (default: 127.0.0.1 25000)\n"
            "  hello               : Send Hello\n"
            "  create_p            : Create Participant\n"
            "  create_w            : Create Writer (Cannon_Actuator_Signal)\n"
            "  create_r            : Create Reader (Cannon_Actuator_Signal)\n"
            "  write               : Write Sample JSON\n"
            "  clear               : Clear All Entities\n"
            "  quit                : Disconnect\n"
            "===============================\n"
            "> "
        );
        
        while (g_running && g_client_sock >= 0) {
            char buf[64];
            int len = recv(g_client_sock, buf, sizeof(buf), 0);
            
            if (len <= 0) break;
            
            for (int i = 0; i < len; i++) {
                unsigned char c = (unsigned char)buf[i];
                
                if (c == 0xFF) continue; // Skip Telnet IAC
                
                if (c == '\n' || c == '\r') {
                    if (line_idx > 0) {
                        line[line_idx] = 0;
                        send(g_client_sock, "\r\n", 2, 0);
                        
                        process_command(line);
                        
                        line_idx = 0;
                        if (g_client_sock < 0) break;
                        cli_print("> ");
                    }
                }
                else if (c == 0x08 || c == 0x7F) { // Backspace
                    if (line_idx > 0) {
                        line_idx--;
                        send(g_client_sock, "\b \b", 3, 0);
                    }
                }
                else if (c >= 32 && c <= 126) {
                    if (line_idx < sizeof(line) - 1) {
                        line[line_idx++] = c;
                    }
                }
            }
        }
        
        printf("[DKM Demo] Client disconnected\n");
        if (g_client_sock >= 0) {
            close(g_client_sock);
            g_client_sock = -1;
        }
        
        if (g_agent) {
            legacy_agent_close(g_agent);
            g_agent = NULL;
        }
    }
    
    printf("[DKM Demo] TCP Server Task exiting\n");
}

// Exported Functions
STATUS legacyAgentTcpStart(int port, const char* json_dir_optional) {
    if (g_running) {
        printf("[DKM Demo] Already running\n");
        return ERROR;
    }
    
    // Create CLI mutex for thread-safe printing
    if (!g_cli_mutex) {
        g_cli_mutex = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE | SEM_DELETE_SAFE);
        if (!g_cli_mutex) {
            printf("[DKM Demo] Failed to create CLI mutex\n");
            return ERROR;
        }
    }
    
    // Create cache mutex
    if (!g_cache_mutex) {
        g_cache_mutex = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE | SEM_DELETE_SAFE);
        if (!g_cache_mutex) {
            printf("[DKM Demo] Failed to create cache mutex\n");
            semDelete(g_cli_mutex);
            g_cli_mutex = NULL;
            return ERROR;
        }
    }
    
    // Set JSON directory path
    if (json_dir_optional && *json_dir_optional) {
        strncpy(g_json_dir_path, json_dir_optional, sizeof(g_json_dir_path) - 1);
        g_json_dir_path[sizeof(g_json_dir_path) - 1] = '\0';
    }
    // else use default "examples/output" already initialized
    
    // Load JSON cache
    printf("[DKM Demo] Loading JSON cache from: %s\n", g_json_dir_path);
    load_json_directory(g_json_dir_path);
    
    g_server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (g_server_sock < 0) {
        printf("[DKM Demo] Failed to create socket\n");
        return ERROR;
    }
    
    // Allow port reuse
    int opt = 1;
    setsockopt(g_server_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    // Disable Nagle's algorithm for immediate transmission
    // TCP_NODELAY = 1 (socket option for IPPROTO_TCP)
    int nodelay = 1;
    setsockopt(g_server_sock, 6, 1, (char*)&nodelay, sizeof(nodelay));  // 6=IPPROTO_TCP, 1=TCP_NODELAY
    
    g_running = 1;
    
    g_server_task = taskSpawn(
        "tLegacyTcp",
        100,
        0,
        32768,
        (FUNCPTR)tcpServerTask,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    );
    
    if (g_server_task == TASK_ID_ERROR) {
        printf("[DKM Demo] Failed to spawn server task\n");
        close(g_server_sock);
        g_server_sock = -1;
        g_running = 0;
        return ERROR;
    }
    
    printf("[DKM Demo] TCP CLI Server started (taskId=%d)\n", g_server_task);
    return OK;
}

STATUS legacyAgentTcpStop(void) {
    if (!g_running) {
        printf("[DKM Demo] Not running\n");
        return ERROR;
    }
    
    printf("[DKM Demo] Stopping TCP server...\n");
    g_running = 0;
    
    // Close sockets to unblock accept/recv
    if (g_client_sock >= 0) {
        close(g_client_sock);
        g_client_sock = -1;
    }
    
    if (g_server_sock >= 0) {
        close(g_server_sock);
        g_server_sock = -1;
    }
    
    // Wait for task to exit
    if (g_server_task != TASK_ID_ERROR) {
        taskDelay(sysClkRateGet() / 10); // 100ms
        
        if (taskIdVerify(g_server_task) == OK) {
            printf("[DKM Demo] Deleting server task\n");
            taskDelete(g_server_task);
        }
        g_server_task = TASK_ID_ERROR;
    }
    
    if (g_agent) {
        legacy_agent_close(g_agent);
        g_agent = NULL;
    }
    
    // Cleanup JSON cache
    clear_json_cache();
    
    if (g_cli_mutex) {
        semDelete(g_cli_mutex);
        g_cli_mutex = NULL;
    }
    
    if (g_cache_mutex) {
        semDelete(g_cache_mutex);
        g_cache_mutex = NULL;
    }
    
    printf("[DKM Demo] Stopped\n");
    return OK;
}
