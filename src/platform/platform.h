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

global usize PLATFORM_PAGE_SIZE;
global File __platform_std_files[3];

typedef enum {
    FILE_ITER_SKIP_DIRS = 1 << 0,
    FILE_ITER_SKIP_FILES = 1 << 1,
    FILE_ITER_SKIP_HIDDEN = 1 << 2,
} FileIterFlags;

typedef struct {
    String name;
} FileInfo;

typedef struct {
    u32 flags;
    b32 is_done;
    PlatformFileIter data;
} FileIter;

#define platform_get_std_file(d) __platform_std_files[(d)]

internal Arena platform_init_main_arena(void);
internal String platform_find_first_file(Arena *arena, StringList *path_list, String file);

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
internal FileIter *platform_file_iter_begin(Arena *arena, String path, u32 flags);
internal b32 platform_file_iter_next(Arena *arena, FileIter *iter, FileInfo *info);
internal void platform_file_iter_end(FileIter *iter);
internal String platform_file_read_into_string(Arena *arena, File file);
internal void platform_file_write_string(File file, String s);
internal Process platform_process_spawn(Arena *arena, CommandLine *cmd_line);
internal b32 platform_process_failed(Process process);
internal b32 platform_process_await(Process process);
internal u64 platform_get_last_error(void);
internal String platform_get_error_message(u64 error_code);
internal String platform_get_current_username(Arena *arena);
internal String platform_get_home_directory(Arena *arena);

// NOTE(cya): the main program entry point (called by the platform layer)
internal void entry_point(Arena *arena, CommandLine *cmd_line);
