#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct {
    usize size;
    void *handle;
} File;

typedef struct {
    void *handle;
} Process;

#define __PLATFORM_PATH_SEPARATOR '\\'
#define PLATFORM_LINE_SEPARATOR "\r\n"

#define PLATFORM_STDIN STD_INPUT_HANDLE
#define PLATFORM_STDOUT STD_OUTPUT_HANDLE
#define PLATFORM_STDERR STD_ERROR_HANDLE

#define platform_mem_equal(d, s, len) RtlEqualMemory(d, s, len)
#define platform_mem_copy(d, s, len) RtlCopyMemory(d, s, len)

global u8 PLATFORM_PATH_SEPARATOR = __PLATFORM_PATH_SEPARATOR;

internal usize platform_get_page_size(void);
internal void *platform_mem_reserve(void *addr, usize size);
internal void *platform_mem_commit(void *addr, usize size);
internal void platform_mem_protect(void *addr, usize size, u8 flags);
internal String platform_get_command_line(Arena *arena);
internal String platform_get_env(Arena *arena, String var);
internal String platform_get_process_filename(Arena *arena);
internal File platform_get_std_file(u32 descriptor, b32 handle_only);
internal void platform_set_env(Arena *arena, String key, String value);
internal File platform_file_open(Arena *arena, String path);
internal usize platform_file_size(void *handle);
internal b32 platform_file_close(File file);
internal b32 platform_file_exists(Arena *arena, String path);
internal String platform_file_read_into_string(Arena *arena, File file);
internal void platform_file_write_string(File file, String s);
internal Process platform_process_spawn(Arena *arena, String args);
internal void platform_process_await(Process process);
internal u64 platform_get_last_error(void);
internal String platform_get_error_message(u64 error_code);
