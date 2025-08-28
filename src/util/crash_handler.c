#define _GNU_SOURCE
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <execinfo.h>
#include <ucontext.h>
#include <errno.h>
#include <dlfcn.h>

#include "util/data_structure/dynamic_string.h"
#include "util/io/logger.h"
#include "util/system.h"


// Maximum number of stack frames to capture
#define MAX_STACK_FRAMES 40

// Original signal handlers for restoration
static struct sigaction s_old_handlers[NSIG];
static const int s_crash_signals[] = {SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGBUS};

#ifdef __cplusplus
    #include <cxxabi.h>

    // Function to demangle C++ symbols
    static char* demangle_symbol(const char* symbol) {
        int status;
        char* demangled = abi::__cxa_demangle(symbol, NULL, NULL, &status);
        return (status == 0) ? demangled : NULL;
    }
#else
    static char* demangle_symbol(const char* symbol) {
        (void)symbol;
        return NULL;
    }
#endif

// Function to get symbol information for an address
static void get_symbol_info(void* addr, char** symbol_name, void** symbol_addr, char** file_name, long* offset) {

    Dl_info info;
    if (dladdr(addr, &info)) {
        *symbol_name = info.dli_sname ? strdup(info.dli_sname) : NULL;
        *symbol_addr = info.dli_saddr;
        *file_name = info.dli_fname ? strdup(info.dli_fname) : NULL;
        *offset = (char*)addr - (char*)info.dli_saddr;
    } else {
        *symbol_name = NULL;
        *symbol_addr = NULL;
        *file_name = NULL;
        *offset = 0;
    }
}

// Function to resolve address to file and line number using addr2line
static void resolve_address(void* addr, const char* executable, char** file, int* line) {
    *file = NULL;
    *line = 0;
    
    // Create command for addr2line
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "addr2line -e %s -f -C -p %p 2>/dev/null", 
             executable, addr);
    
    FILE* fp = popen(cmd, "r");
    if (!fp) return;
    
    char result[256];
    if (fgets(result, sizeof(result), fp)) {
        // Parse the result: "function_name at file:line"
        char* at_pos = strstr(result, " at ");
        if (at_pos) {
            *at_pos = '\0'; // Separate function name from file:line
            at_pos += 4;    // Skip " at "
            
            char* colon_pos = strrchr(at_pos, ':');
            if (colon_pos) {
                *colon_pos = '\0'; // Separate file from line
                *file = strdup(at_pos);
                *line = atoi(colon_pos + 1);
            }
        }
    }
    
    pclose(fp);
}

//
static void crash_handler(int sig, siginfo_t* info, void* ucontext) {
    // Prevent recursive crashes in handler
    static volatile sig_atomic_t in_handler = 0;
    if (in_handler) {
        _exit(EXIT_FAILURE);
    }
    in_handler = 1;
    
    // Get program counter
    ucontext_t* uc = (ucontext_t*)ucontext;
    void* caller_address = NULL;
#ifdef __x86_64__
    caller_address = (void*)uc->uc_mcontext.gregs[REG_RIP];
#elif __i386__
    caller_address = (void*)uc->uc_mcontext.gregs[REG_EIP];
#elif __arm__
    caller_address = (void*)uc->uc_mcontext.arm_pc;
#elif __aarch64__
    caller_address = (void*)uc->uc_mcontext.pc;
#endif

    // Log crash information to stderr
    const char* sig_name = "UNKNOWN";
    switch(sig) {
        case SIGSEGV: sig_name = "SIGSEGV (Segmentation Fault)"; break;
        case SIGABRT: sig_name = "SIGABRT (Abort)"; break;
        case SIGFPE:  sig_name = "SIGFPE (Floating Point Exception)"; break;
        case SIGILL:  sig_name = "SIGILL (Illegal Instruction)"; break;
        case SIGBUS:  sig_name = "SIGBUS (Bus Error)"; break;
    }

    const char* exe_path = get_executable_path();
    
    dyn_str crash_msg;
    ds_init(&crash_msg);

    ds_append_str(&crash_msg, "\n=================== CRASH DETECTED ===================\n");
    ds_append_fmt(&crash_msg, "Signal: %s (%d)\n", sig_name, sig);
    ds_append_fmt(&crash_msg, "Fault address: %p\n", info->si_addr);
    ds_append_fmt(&crash_msg, "Fault instruction: %p\n", caller_address);

    // Get backtrace
    void* buffer[MAX_STACK_FRAMES];
    int frames = backtrace(buffer, MAX_STACK_FRAMES);
    
    // Overwrite the first frame (this function) with the actual fault address
    if (caller_address)
        buffer[1] = caller_address;
    
    ds_append_fmt(&crash_msg, "-------------- Stack trace (%d frames) --------------\n", frames);
    
    for (int i = 1; i < frames; i++) {
        void* addr = buffer[i];
        
        // Get symbol information
        char* symbol_name = NULL;
        void* symbol_addr = NULL;
        char* file_name = NULL;
        long offset = 0;
        get_symbol_info(addr, &symbol_name, &symbol_addr, &file_name, &offset);
        
        // Try to demangle the symbol name
        char* demangled_name = NULL;
        if (symbol_name) {
            demangled_name = demangle_symbol(symbol_name);
        }
        
        // Try to resolve to source file and line
        char* source_file = NULL;
        int source_line = 0;
        resolve_address(addr, exe_path, &source_file, &source_line);
        
        // Print frame information
        ds_append_fmt(&crash_msg, "#%-2d %p", i, addr);
        ds_append_fmt(&crash_msg, " in [%s]", file_name);
        ds_append_fmt(&crash_msg, " name [%s]", demangled_name ? demangled_name : "null");
        ds_append_fmt(&crash_msg, " symbol [%s]", symbol_name ? symbol_name : "null");
        ds_append_fmt(&crash_msg, " offset [0x%lx]", offset);
        ds_append_fmt(&crash_msg, " file:line [%s:%d]\n", source_file ? source_file : "null", source_line);

        if (demangled_name)
            free(demangled_name);
        if (source_file && source_line > 0)
            free(source_file);
        
        // Clean up
        if (symbol_name) free(symbol_name);
        if (file_name) free(file_name);
    }
    
    ds_append_str(&crash_msg, "------------------------------------------------------\n");
    ds_append_str(&crash_msg, "Note: Install debug symbols for more detailed information\n");
    
    LOG(Error, "%s", crash_msg.data);
    ds_free(&crash_msg);
    logger_shutdown();
    
    // Restore default handler and re-raise signal to trigger core dump
    sigaction(sig, &s_old_handlers[sig], NULL);
    raise(sig);
}

//
bool crash_handler_init() {

    LOG_INIT

    // Signals to catch
    const int num_signals = sizeof(s_crash_signals) / sizeof(s_crash_signals[0]);
    struct sigaction sa;
    sa.sa_sigaction = crash_handler;
    sigemptyset(&sa.sa_mask);
    
    for (int i = 0; i < num_signals; i++)               // Add all crash signals to the mask to prevent nested handling
        sigaddset(&sa.sa_mask, s_crash_signals[i]);
    
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_RESTART;
    
    // Register signal handlers
    for (int i = 0; i < num_signals; i++) {
        if (sigaction(s_crash_signals[i], &sa, &s_old_handlers[s_crash_signals[i]]) == -1) {
            perror("sigaction failed");
            return false;
        }
    }
    
    signal(SIGPIPE, SIG_IGN);           // Ignore SIGPIPE to prevent crashes from broken pipes
    
    return true;
}

//
void crash_handler_shutdown() {

    LOG_SHUTDOWN

    const int num_signals = sizeof(s_crash_signals) / sizeof(s_crash_signals[0]);
    for (int i = 0; i < num_signals; i++)                                           // Restore original signal handlers
        sigaction(s_crash_signals[i], &s_old_handlers[s_crash_signals[i]], NULL);

}
