// TODO(cya): mac(?)
#if defined(PLATFORM_WINDOWS)
#    include "win32/platform_core_win32.h"
#elif defined(PLATFORM_LINUX)
#    include "linux/platform_core_linux.h"
#else
#    error platform layer not implemented for this OS
#endif

// NOTE(cya): these are platform-independent
#define STDIN 0
#define STDOUT 1
#define STDERR 2

global File __platform_std_files[3];

#define platform_get_std_file(d) __platform_std_files[(d)]

internal usize platform_get_page_size(void);
internal void *platform_mem_reserve(void *addr, usize size);
internal void *platform_mem_commit(void *addr, usize size);
internal void platform_mem_release(void *addr, usize size);

internal String platform_get_process_filename(Arena *arena);
internal String platform_get_env(Arena *arena, String var);
internal void platform_set_env(Arena *arena, String key, String value);
internal File platform_file_open(Arena *arena, String path);
internal b32 platform_file_close(File file);
internal b32 platform_file_exists(Arena *arena, String path);
internal String platform_file_read_into_string(Arena *arena, File file);
internal void platform_file_write_string(File file, String s);
internal Process platform_process_spawn(Arena *arena, CommandLine *cmd_line);
internal b32 platform_process_failed(Process process);
internal b32 platform_process_await(Process process);
internal u64 platform_get_last_error(void);
internal String platform_get_error_message(u64 error_code);

// NOTE(cya): the main program entry point (called by the platform layer)
internal void entry_point(CommandLine *cmd_line);
