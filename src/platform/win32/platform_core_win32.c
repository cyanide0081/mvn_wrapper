#define PERM_READ bit_flag(0)
#define PERM_WRITE bit_flag(1)

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

inline void platform_mem_protect(void *addr, usize size, u8 flags)
{
    u32 old_protect;
    u32 read = flags & PERM_READ;
    u32 write = flags & PERM_WRITE;
    VirtualProtect(addr, size, read | write, (DWORD*)&old_protect);
}

inline String platform_get_command_line(Arena *arena)
{
    String16 command_line_utf16 = string16_from_wcstring(GetCommandLineW());
    return win32_utf8_from_utf16(arena, command_line_utf16);
}

String platform_get_env(Arena *arena, String var)
{
    String16 var_utf16 = win32_utf16_from_utf8(arena, var);
    DWORD str_size_utf16 = GetEnvironmentVariableW(var_utf16.str, NULL, 0);
    u16 *str_utf16 = arena_push_array(arena, str_size_utf16, u16);
    usize len_utf16 = GetEnvironmentVariableW(
        var_utf16.str,
        str_utf16,
        str_size_utf16
    );

    return win32_utf8_from_utf16(arena, string16_create(str_utf16, len_utf16));
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

inline File platform_get_std_file(u32 descriptor, b32 handle_only)
{
    void *handle = GetStdHandle(descriptor);
    return (File){
        .handle = handle,
        .size = handle_only ? 0 : platform_file_size(handle),
    };
}

inline void platform_set_env(Arena *arena, String key, String val)
{
    String16 key_utf16 = win32_utf16_from_utf8(arena, key);
    String16 val_utf16 = win32_utf16_from_utf8(arena, val);
    SetEnvironmentVariableW(key_utf16.str, val_utf16.str);
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
        .size = handle == INVALID_HANDLE_VALUE ? 0 : platform_file_size(handle),
    };
}

inline usize platform_file_size(void *handle)
{
    DWORD size_lo = 0, size_hi = 0;
    size_lo = GetFileSize(handle, &size_hi);
    return size_lo == INVALID_FILE_SIZE ? 0 : size_lo | size_hi;
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

inline String platform_file_read_into_string(Arena *arena, File file)
{
    usize size = file.size;
    u8 *buf = arena_push(arena, size + 1);
    ReadFile(file.handle, buf, (DWORD)size, NULL, NULL);
    return string_create(buf, size);
}

inline void platform_file_write_string(File file, String s)
{
    WriteFile(file.handle, s.str, (DWORD)s.len, NULL, NULL);
}

inline Process platform_process_spawn(Arena *arena, String args)
{
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
    return (Process){
        .handle = process_info.hProcess,
    };
}

inline void platform_process_await(Process process)
{
    WaitForSingleObject(process.handle, INFINITE);
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
