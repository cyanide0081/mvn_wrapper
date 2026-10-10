// TODO(cya): test mac build
#if defined(PLATFORM_WINDOWS)
#    include "win32/platform_core_win32.c"
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_MAC)
#    include "posix/platform_core_posix.c"
#else
#    error platform layer not implemented for this OS
#endif

#define platform_path_is_absolute(p) (string_starts_with(p, string_lit(PLATFORM_PATH_SEPARATOR)))

// NOTE(cya): we don't really need more than one for this
inline Arena platform_init_main_arena(void)
{
    Arena arena = arena_init(16, kibibytes(64));
    if (arena.memory == NULL) {
        String error = platform_get_error_message(platform_get_last_error());
        log_fatal("unable to acquire virtual memory: {}", error);
    }

    return arena;
}

inline String platform_find_first_file(Arena *arena, StringList *path_list, String file)
{
    string_list_foreach(path_list, node) {
        String path = node->str;
        String full_path = string_path_append(arena, path, file);
        if (platform_file_exists(arena, full_path)) {
            return path;
        }
    }

    return string_lit("");
}
