inline usize platform_get_page_size(void)
{
    SYSTEM_INFO info = {0};
    GetSystemInfo(&info);
    return info.dwPageSize;
}

inline void *platform_mem_reserve(void *addr, usize size)
{
    return VirtualAlloc(addr, size, MEM_RESERVE, PAGE_READWRITE);
}

inline void *platform_mem_commit(void *addr, usize size)
{
    return VirtualAlloc(addr, size, MEM_COMMIT, PAGE_READWRITE);
}

inline void platform_mem_release(void *addr, usize size)
{
    (void)size; // NOTE(cya): windows doesn't use it
    VirtualFree(addr, 0, MEM_RELEASE);
}

#define win32_wide_char_to_multi_byte(str, len, out_str, out_len) \
    (usize)WideCharToMultiByte( \
        CP_UTF8, \
        WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK, \
        (const u16*)str, \
        (i32)len, \
        out_str, \
        (i32)out_len, \
        (const char*)"?", \
        NULL \
    )

#define win32_multi_byte_to_wide_char(str, len, out_str, out_len) \
    (usize)MultiByteToWideChar( \
        CP_UTF8, \
        MB_COMPOSITE, \
        (const char *)str, \
        (i32)len, \
        out_str, \
        (i32)out_len \
    )

internal inline String win32_utf8_from_utf16(Arena *arena, String16 s)
{
    i32 len_utf8 = win32_wide_char_to_multi_byte(s.str, s.len , NULL, 0);
    char *str_utf8 = arena_push(arena, len_utf8 + 1);
    win32_wide_char_to_multi_byte(s.str, s.len, str_utf8, len_utf8);
    str_utf8[len_utf8] = '\0';

    return string_create(str_utf8, len_utf8);
}

internal inline String16 win32_utf16_from_utf8(Arena *arena, String s)
{
    usize len_utf16 = win32_multi_byte_to_wide_char(s.str, s.len, NULL, 0);
    u16 *str_utf16 = arena_push_array(arena, (len_utf16 + 1), u16);
    win32_multi_byte_to_wide_char(s.str, s.len, str_utf16, len_utf16);
    str_utf16[len_utf16] = '\0';

    return string16_create(str_utf16, len_utf16);
}

String platform_get_process_filename(Arena *arena)
{
    usize len;
    usize buf_size;
    usize buf_len = MAX_PATH;
    u16 *buf;
    for (;;) {
        buf_size = buf_len * sizeof(*buf);
        buf = arena_push(arena, buf_size);
        len = GetModuleFileNameW(NULL, buf, buf_len);
        if (len < buf_len) {
            break;
        }

        arena_pop(arena, buf_size);
        buf_len *= 2;
    }

    String16 filename_16 = string16_create(buf, len);
    return win32_utf8_from_utf16(arena, filename_16);
}

String platform_get_env(Arena *arena, String key)
{
    String16 key_utf16 = win32_utf16_from_utf8(arena, key);
    DWORD var_size_utf16 = GetEnvironmentVariableW(key_utf16.str, NULL, 0);
    u16 *var_utf16 = arena_push_array(arena, var_size_utf16, u16);
    usize len_utf16 = GetEnvironmentVariableW(
        key_utf16.str,
        var_utf16,
        var_size_utf16
    );

    return win32_utf8_from_utf16(arena, string16_create(var_utf16, len_utf16));
}

inline void platform_set_env(Arena *arena, String key, String val)
{
    String16 key_utf16 = win32_utf16_from_utf8(arena, key);
    String16 val_utf16 = win32_utf16_from_utf8(arena, val);
    SetEnvironmentVariableW(key_utf16.str, val_utf16.str);
}

internal inline usize win32_file_size(void *handle)
{
    if (handle == INVALID_HANDLE_VALUE) {
        return 0;
    }

    DWORD size_lo = 0, size_hi = 0;
    size_lo = GetFileSize(handle, &size_hi);
    return size_lo == INVALID_FILE_SIZE ? 0 : size_lo | size_hi;
}

File platform_file_open(Arena *arena, String path)
{
    String16 path_utf16 = win32_utf16_from_utf8(arena, path);
    void *handle = CreateFileW(
        path_utf16.str,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    return (File){
        .handle = handle,
        .size = win32_file_size(handle),
    };
}

b32 platform_file_close(File file)
{
    return CloseHandle(file.handle);
}

b32 platform_file_exists(Arena *arena, String path)
{
    String16 path_16 = win32_utf16_from_utf8(arena, path);
    DWORD attributes = GetFileAttributesW(path_16.str);
    return attributes != INVALID_FILE_ATTRIBUTES;
}

FileIter *platform_file_iter_begin(Arena *arena, String path, u32 flags)
{
    String path_with_wildcard = string_path_append(arena, path, string_lit("*"));
    String16 path_utf16 = win32_utf16_from_utf8(arena, path_with_wildcard);
    FileIter *iter = arena_push_array(arena, 1, FileIter);
    iter->flags = flags;
    iter->data.handle = FindFirstFileExW(
        (WCHAR*)path_utf16.str,
        FindExInfoBasic,
        &iter->data.find_data,
        FindExSearchNameMatch,
        0,
        FIND_FIRST_EX_LARGE_FETCH
    );
    return iter;
}

b32 platform_file_iter_next(Arena *arena, FileIter *iter, FileInfo *info)
{
    if (iter->is_done) {
        return false;
    }

    do {
        WCHAR *name = iter->data.find_data.cFileName;
        DWORD attributes = iter->data.find_data.dwFileAttributes;
        b32 is_dir = (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        b32 skip = ((iter->flags & FILE_ITER_SKIP_DIRS) && is_dir) ||
            ((iter->flags & FILE_ITER_SKIP_FILES) && !is_dir) ||
            ((iter->flags & FILE_ITER_SKIP_HIDDEN) && name[0] == '.');
        if (skip) {
            continue;
        }

        if (!FindNextFileW(iter->data.handle, &iter->data.find_data)) {
            iter->is_done = true;
        }

        info->name = win32_utf8_from_utf16(arena, string16_from_wcstring(name));
        return true;
    } while (FindNextFileW(iter->data.handle, &iter->data.find_data));

    iter->is_done = true;
    return false;
}

inline void platform_file_iter_end(FileIter *iter)
{
    FindClose(iter->data.handle);
}

inline String platform_file_read_into_string(Arena *arena, File file)
{
    usize size = file.size;
    u8 *buf = arena_push(arena, size);
    ReadFile(file.handle, buf, (DWORD)size, NULL, NULL);
    return string_create(buf, size);
}

inline void platform_file_write_string(File file, String s)
{
    WriteFile(file.handle, s.str, (DWORD)s.len, NULL, NULL);
}

inline Process platform_process_spawn(Arena *arena, CommandLine *cmd_line)
{
    // NOTE(cya): windows expects a single command-line string
    string_list_push_front(arena, cmd_line->arguments, cmd_line->exe_name);
    string_list_foreach(cmd_line->arguments, node) {
        String argument = node->str;
        String escaped = command_line_escape_string(arena, argument);
        cmd_line->arguments->total_len += (escaped.len - argument.len);
        node->str = escaped;
    }

    String args = string_list_join(arena, cmd_line->arguments, string_lit(" "));

    String16 args_utf16 = win32_utf16_from_utf8(arena, args);
    PROCESS_INFORMATION process_info = {0};
    STARTUPINFOW startup_info = {.cb = sizeof(startup_info)};
    CreateProcessW(
        NULL,
        args_utf16.str,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &startup_info,
        &process_info
    );
    return (Process){.handle = process_info.hProcess};
}

inline b32 platform_process_failed(Process process)
{
    return process.handle == INVALID_HANDLE_VALUE;
}

inline b32 platform_process_await(Process process)
{
    if (platform_process_failed(process)) {
        return false;
    }

    return WaitForSingleObject(process.handle, INFINITE) != WAIT_FAILED;
}

thread_local u16 __win32_error_buf[4096];

inline u64 platform_get_last_error(void)
{
    return (u64)GetLastError();
}

String platform_get_error_message(u64 error_code)
{
    u16 *msg = (u16*)&__win32_error_buf;
    usize buf_size = array_len(__win32_error_buf);
    usize size = buf_size / sizeof(*msg);
    usize msg_len = FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        (DWORD)error_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)msg,
        (DWORD)size,
        NULL
    );
    if (msg_len == 0) {
        return string_lit("");
    }

    Arena arena = arena_init_from_buffer(__win32_error_buf, buf_size);
    arena.offset = (msg_len + 1) * sizeof(*msg);

    String result =  win32_utf8_from_utf16(&arena, string16_create(msg, msg_len));
    return string_trim_trailing(result);
}

inline String platform_get_current_username(Arena *arena)
{
    u16 buf[UNLEN + 1];
    DWORD buf_size = array_len(buf);
    GetUserNameW(buf, &buf_size);

    String16 curr_user = string16_create(buf, buf_size);
    return win32_utf8_from_utf16(arena, curr_user);
}

readonly global char *HOME_ENVS[] = {"USERPROFILE", "HOME"};

inline String platform_get_home_directory(Arena *arena)
{
    u16 *path;
    if (SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_Profile, 0, NULL, &path))) {
        String home = win32_utf8_from_utf16(arena, string16_from_wcstring(path));
        CoTaskMemFree(path);
        return home;
    }

    // NOTE(cya): environment vars fallback
    for (usize i = 0; i < array_len(HOME_ENVS); i++) {
        String home = platform_get_env(arena, string_lit(HOME_ENVS[i]));
        if (!string_is_empty(home)) {
            return home;
        }
    }

    return string_lit("");
}

// NOTE(cya): windows's wide entry point for unicode strings
int wmain(int argc, wchar_t *argv[])
{
    PLATFORM_PAGE_SIZE = platform_get_page_size();

    __platform_std_files[STDIN].handle = GetStdHandle(STD_INPUT_HANDLE);
    __platform_std_files[STDOUT].handle = GetStdHandle(STD_OUTPUT_HANDLE);
    __platform_std_files[STDERR].handle = GetStdHandle(STD_ERROR_HANDLE);

    Arena arena = platform_init_main_arena();
    if (arena.memory == NULL) {
        return 1;
    }

    StringList arguments = {0};
    for (int i = 0; i < argc; i++) {
        String16 argument_16 = string16_from_wcstring(argv[i]);
        String argument = win32_utf8_from_utf16(&arena, argument_16);
        string_list_push_back(&arena, &arguments, argument);
    }

    log.arena = &arena;

    CommandLine cmd_line = command_line_from_string_list(&arguments);
    entry_point(&arena, &cmd_line);

#if defined(BUILD_DEBUG)
    arena_log_stats(&arena);
#endif

    return 0;
}
