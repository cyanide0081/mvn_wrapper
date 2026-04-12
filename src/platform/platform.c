// TODO(cya): mac(?)
#if defined(PLATFORM_WINDOWS)
#    include "win32/platform_core_win32.c"
#elif defined(PLATFORM_LINUX)
#    include "linux/platform_core_linux.c"
#else
#    error platform layer not implemented for this OS
#endif

inline String platform_find_first_file(Arena *arena, StringList *path_list, String file)
{
    string_list_foreach(path_list, path) {
        String full_path = string_path_append(arena, path, file);
        if (platform_file_exists(arena, full_path)) {
            return path;
        }
    }

    return string_lit("");
}
