#ifndef BASE_H
#define BASE_H

#include <stdarg.h> // va_args

/***************************
 * NOTE(cya): declarations *
 ***************************/

#define internal static
#define global static
#define local_persist static

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef DEFAULT_ALIGN
#define DEFAULT_ALIGN (uptr)(2 * sizeof(void*))
#endif

#ifdef _WIN32
#define OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define __OS_PATH_SEPARATOR '\\'
#define OS_LINE_SEPARATOR "\r\n"

#define OS_STDIN STD_INPUT_HANDLE
#define OS_STDOUT STD_OUTPUT_HANDLE
#define OS_STDERR STD_ERROR_HANDLE

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef uintptr_t uptr;
typedef intptr_t iptr;

#else
#error unsupported platform
#endif

typedef u64 usize;
typedef i64 isize;

typedef u32 b32;

#ifndef true
#define true (0 == 0)
#endif

#ifndef false
#define false (0 != 0)
#endif

#ifdef NDEBUG
#define MODE_RELEASE
#endif

#ifndef readonly
#ifdef OS_WINDOWS
#pragma section(".rdata$", read)
#define readonly __declspec(allocate(".rdata$"))
#else
#define readonly
#endif // OS_WINDOWS
#endif

#ifndef noop
#define noop() ((void)0)
#endif

#ifndef debug_trap
#ifdef _MSC_VER
#if _MSC_VER < 1300
#define debug_trap() __asm int 3
#else
#define debug_trap() __debugbreak()
#endif // _MSC_VER < 1300
#else
#define debug_trap() __builtin_trap()
#endif // _MSC_VER
#endif

#ifndef stringify
#define stringify(x) #x
#endif // stringify

#ifndef assert_msg
#ifndef MODE_RELEASE
#define assert_msg(cond, ...) {\
    if (!(cond)) { \
        assert_handle( \
            "assertion failed", \
            stringify(cond), \
            __FILE__, \
            stringify(__LINE__), \
            __VA_ARGS__ \
        ); \
        debug_trap(); \
    } \
} noop()
#else
#define assert_msg(cond, ...) (void)(cond)
#endif // MODE_RELEASE
#endif

#ifndef assert
#define assert(cond) assert_msg(cond, NULL)
#endif

#ifndef min
#define min(a, b) ((a) > (b) ? (b) : (a))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

typedef struct {
    usize reserved;
    usize committed;
    usize block_size;
    usize offset;
    void *memory;
} Arena;

typedef struct {
    usize len;
    u8 *str;
} String;

typedef struct {
    usize len;
    u16 *str;
} String16;

typedef struct StringNode StringNode;
struct StringNode {
    StringNode *next;
    String str;
};

typedef struct {
    usize node_count;
    usize total_len;
    StringNode *first;
    StringNode *last;
} StringList;

typedef struct {
    usize size;
    void *handle;
} File;

typedef struct {
    void *handle;
} Process;

typedef enum {
    LOG_ERROR,
    LOG_INFO,
} LogLevel;

#define string_is_empty(s) (s.len == 0)
#define string_create(s, l) ((String){.len = (usize)l, .str = (u8*)s})
#define string_lit(s) string_create(s, sizeof(s) - 1)
#define string_from_char(c) string_create(&c, 1)
#define string_contains_char(s, c) string_contains(s, string_create(&c, 1))
#define string16_create(s, l) ((String16){.len = (usize)l, .str = (u16*)s})
#define arena_alloc_array(a, size, type) arena_alloc(a, (size) * sizeof(type))
#define array_len(a) ((usize)sizeof(a) / sizeof(a[0]))
#define mem_equal(d, s, len) os_mem_equal(d, s, len)
#define mem_copy(d, s, len) os_mem_copy(d, s, len)

readonly global u8 OS_PATH_SEPARATOR = __OS_PATH_SEPARATOR;
readonly global char *__LOG_LEVEL_TO_STRING[] = {
    [LOG_ERROR] = "ERROR",
    [LOG_INFO] = "INFO",
};

internal b32 is_power_of_two(uptr value);
internal usize align_forward_size(usize value, usize align);
internal uptr align_forward(uptr value, uptr align);
internal usize os_get_page_size(void);
internal void *os_mem_reserve(void *addr, usize size);
internal void *os_mem_commit(void *addr, usize size);
internal void os_mem_protect(void *addr, usize size, u8 flags);
internal String os_get_command_line(Arena *arena);
internal String os_get_env(Arena *arena, String var);
internal File os_get_std_file(u32 descriptor, b32 handle_only);
internal void os_set_env(Arena *arena, String key, String value);
internal File os_file_open(Arena *arena, String path);
internal usize os_file_size(void *handle);
internal b32 os_file_close(File file);
internal String os_file_read_into_string(Arena *arena, File file);
internal void os_file_write_string(File file, String s);
internal Process os_process_spawn(Arena *arena, String args);
internal void os_process_await(Process process);

internal Arena arena_init(usize reserve, usize commit);
internal void *arena_alloc(Arena *arena, usize size);

internal b32 char_is_whitespace(char c);
internal b32 char_is_digit(char c);
internal usize cstring_len(const char *str);
internal usize wcstring_len(const u16 *str);

internal String string_from_cstring(const char *str);
internal String16 string16_from_wcstring(const u16 *str);
internal b32 string_starts_with(String a, String b);
internal b32 string_contains(String haystack, String needle);
internal String string_keep_number(String s);
internal String string_fmt(Arena *arena, const char *fmt, ...);
internal String string_fmt_va(Arena *arena, const char *fmt, va_list va);
internal String string_skip_first_match(String s, String target);
internal String string_skip_nth_match(String s, String target, usize n);
internal String string_cut_leading(String s, usize n);
internal String string_trim_leading(String s);
internal String string_path_append(Arena *arena, String path, String elem);
internal String string_path_pop(String path);
internal u64 string_parse_u64(String s);

internal StringList string_split(Arena *arena, String s, String delims);
internal void string_list_push_back(Arena *arena, StringList *list, String s);
internal void string_list_pop_front(StringList *list);
internal String string_list_find_first_match(StringList *list, String needle);
internal String string_list_join(Arena *arena, StringList *list, String delim);

#ifndef MODE_RELEASE
internal void assert_handle(
    const char *_prefix,
    const char *_cond,
    const char *_file,
    const char *_line,
    const char *_msg,
    ...
);
#endif

internal void log_fmt(LogLevel level, const char *fmt, ...);
internal void log_fmt_va(LogLevel level, const char *fmt, va_list va);

/******************************
 * NOTE(cya): implementations *
 ******************************/

b32 is_power_of_two(uptr value)
{
    return (value & (value - 1)) == 0;
}

inline usize align_forward_size(usize value, usize align)
{
    return align_forward((uptr)value, (uptr)align);
}

uptr align_forward(uptr value, uptr align)
{
    assert(is_power_of_two(align));

    uptr modulo = value & (align - 1);
    return modulo == 0 ? value : value + align - modulo;
}

#define BIT_FLAG(n) (1 << (n))
#define PERM_READ BIT_FLAG(0)
#define PERM_WRITE BIT_FLAG(1)

#ifdef OS_WINDOWS
#define os_mem_equal(d, s, len) RtlEqualMemory(d, s, len)
#define os_mem_copy(d, s, len) RtlCopyMemory(d, s, len)

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
    char *str_utf8 = arena_alloc(arena, len_utf8 + 1);
    win32_wide_char_to_multi_byte(s.str, s.len, str_utf8, len_utf8);
    str_utf8[len_utf8] = '\0';

    return string_create(str_utf8, len_utf8);
}

internal inline String16 win32_utf16_from_utf8(Arena *arena, String s)
{
    usize len_utf16 = win32_multi_byte_to_wide_char(s.str, s.len, NULL, 0);
    u16 *str_utf16 = arena_alloc_array(arena, (len_utf16 + 1), u16);
    win32_multi_byte_to_wide_char(s.str, s.len, str_utf16, len_utf16);
    str_utf16[len_utf16] = '\0';

    return string16_create(str_utf16, len_utf16);
}

inline usize os_get_page_size(void)
{
    SYSTEM_INFO info = {0};
    GetSystemInfo(&info);
    return info.dwPageSize;
}

inline void *os_mem_reserve(void *addr, usize size)
{
    return VirtualAlloc(addr, size, MEM_RESERVE, PAGE_READWRITE);
}

inline void *os_mem_commit(void *addr, usize size)
{
    return VirtualAlloc(addr, size, MEM_COMMIT, PAGE_READWRITE);
}

inline void os_mem_protect(void *addr, usize size, u8 flags)
{
    u32 old_protect;
    u32 read = flags & PERM_READ;
    u32 write = flags & PERM_WRITE;
    VirtualProtect(addr, size, read | write, (DWORD*)&old_protect);
}

inline String os_get_command_line(Arena *arena)
{
    String16 command_line_utf16 = string16_from_wcstring(GetCommandLineW());
    return win32_utf8_from_utf16(arena, command_line_utf16);
}

String os_get_env(Arena *arena, String var)
{
    String16 var_utf16 = win32_utf16_from_utf8(arena, var);
    DWORD str_size_utf16 = GetEnvironmentVariableW(var_utf16.str, NULL, 0);
    u16 *str_utf16 = arena_alloc_array(arena, str_size_utf16, u16);
    usize len_utf16 = GetEnvironmentVariableW(
        var_utf16.str,
        str_utf16,
        str_size_utf16
    );

    return win32_utf8_from_utf16(arena, string16_create(str_utf16, len_utf16));
}

inline File os_get_std_file(u32 descriptor, b32 handle_only)
{
    void *handle = GetStdHandle(descriptor);
    return (File){
        .handle = handle,
        .size = handle_only ? 0 : os_file_size(handle),
    };
}

inline void os_set_env(Arena *arena, String key, String val)
{
    String16 key_utf16 = win32_utf16_from_utf8(arena, key);
    String16 val_utf16 = win32_utf16_from_utf8(arena, val);
    SetEnvironmentVariableW(key_utf16.str, val_utf16.str);
}

File os_file_open(Arena *arena, String path)
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
        .size = handle == INVALID_HANDLE_VALUE ? 0 : os_file_size(handle),
    };
}

inline usize os_file_size(void *handle)
{
    DWORD size_lo = 0, size_hi = 0;
    size_lo = GetFileSize(handle, &size_hi);
    return size_lo == INVALID_FILE_SIZE ? 0 : size_lo | size_hi;
}

b32 os_file_close(File file)
{
    return CloseHandle(file.handle);
}

inline String os_file_read_into_string(Arena *arena, File file)
{
    usize size = file.size;
    u8 *buf = arena_alloc(arena, size + 1);
    ReadFile(file.handle, buf, (DWORD)size, NULL, NULL);
    return string_create(buf, size);
}

inline void os_file_write_string(File file, String s)
{
    WriteFile(file.handle, s.str, (DWORD)s.len, NULL, NULL);
}

inline Process os_process_spawn(Arena *arena, String args)
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

inline void os_process_await(Process process)
{
    WaitForSingleObject(process.handle, INFINITE);
}

#endif // OS_WINDOWS

Arena arena_init(usize reserve, usize commit)
{
    usize page_size = os_get_page_size();
    usize reserved = align_forward_size(reserve, page_size);
    usize committed = align_forward_size(commit, page_size);
    void *memory = os_mem_reserve(NULL, reserved);
    if (memory == NULL) {
        // TODO(cya): error handling?
        reserved = 0;
        committed = 0;
    } else {
        os_mem_commit(memory, committed);
    }

    return (Arena){
        .reserved = reserved,
        .committed = committed,
        .block_size = committed,
        .memory = memory,
    };
}

void *arena_alloc(Arena *arena, usize size)
{
    uptr memory = (uptr)arena->memory;
    uptr cur_addr = memory + (uptr)arena->offset;
    uptr base_addr = align_forward(cur_addr, DEFAULT_ALIGN);
    usize base_offset = (usize)(base_addr - memory);
    if (base_addr > memory + (uptr)arena->reserved) {
        // NOTE(cya): scratch behavior
        base_addr = align_forward(memory, DEFAULT_ALIGN);
        base_offset = (usize)(base_addr - memory);
    } else {
        usize committed = arena->committed;
        if (base_offset > committed) {
            // TODO(cya): can we really be sure this will stay in-bounds?
            usize commit = align_forward(size, arena->block_size);
            void *commit_addr = (void*)(memory + (uptr)committed);
            os_mem_commit(commit_addr, commit);
            arena->committed += commit;
        }
    }

    usize new_offset = base_offset + size;
    arena->offset = new_offset;
    return (void*)base_addr;
}

// NOTE(cya): ASCII only
inline b32 char_is_whitespace(char c)
{
    switch (c) {
    case ' ':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
        return true;
    default:
        return false;
    }
}

inline b32 char_is_digit(char c)
{
    return (u8)(c - '0') < 10;
}

inline usize cstring_len(const char *str)
{
    usize len = 0;
    while (*str++ != '\0') {
        len += 1;
    }

    return len;
}

inline usize wcstring_len(const u16 *str)
{
    usize len = 0;
    while (*str++ != '\0') {
        len += 1;
    }

    return len;
}

inline String string_from_cstring(const char *str)
{
    return string_create(str, cstring_len(str));
}

inline String16 string16_from_wcstring(const u16 *str)
{
    return string16_create(str, wcstring_len(str));
}

inline b32 string_starts_with(String a, String b)
{
    if (b.len > a.len) {
        return false;
    }

    for (usize i = 0; i < b.len; i++) {
        if (a.str[i] != b.str[i]) {
            return false;
        }
    }

    return true;
}

inline b32 string_contains(String haystack, String needle)
{
    for (usize i = 0; i < haystack.len; i++) {
        if (mem_equal(&haystack.str[i], needle.str, needle.len)) {
            return true;
        }
    }

    return false;
}

inline String string_keep_number(String s)
{
    usize i;
    for (i = 0; i < s.len; i++) {
        if (!char_is_digit(s.str[i])) {
            break;
        }
    }

    s.len = i;
    return s;
}

inline String string_fmt(Arena *arena, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    String str = string_fmt_va(arena, fmt, va);
    va_end(va);
    return str;
}

inline String string_fmt_va(Arena *arena, const char *fmt, va_list va)
{
    StringList parts = {0};
    usize cur = 0, i = 0;
    for (; fmt[i] != '\0'; i++) {
        if (fmt[i] == '{' && fmt[i + 1] == '}') {
            usize len = i - cur;
            string_list_push_back(arena, &parts, string_create(&fmt[cur], len));
            string_list_push_back(arena, &parts, va_arg(va, String));

            cur += len + 2;
            i += 1;
        }
    }

    string_list_push_back(arena, &parts, string_create(&fmt[cur], i - cur));
    return string_list_join(arena, &parts, string_lit(""));
}

inline String string_skip_first_match(String s, String target)
{
    return string_skip_nth_match(s, target, 1);
}

String string_skip_nth_match(String s, String target, usize n)
{
    usize matches = 0;
    usize len = s.len;
    usize i = 0;
    for (; i < len; i++) {
        if (mem_equal(&s.str[i], target.str, target.len)) {
            matches += 1;
            i += target.len;
            if (matches == n) {
                break;
            }
        }
    }

    return string_create(&s.str[i], len - i);
}

inline String string_cut_leading(String s, usize n)
{
    if (s.len <= n) {
        s.str += n;
        s.len -= n;
    }

    return s;
}

inline String string_trim_leading(String s)
{
    usize i = 0;
    while (i < s.len && char_is_whitespace(s.str[i])) {
        i += 1;
    }

    return string_create(&s.str[i], s.len - i);
}

inline String string_path_append(Arena *arena, String path, String elem)
{
    String separator = string_from_char(OS_PATH_SEPARATOR);
    return string_fmt(arena, "{}{}{}", path, separator, elem);
}

inline String string_path_pop(String path)
{
    String result = {0};
    for (usize i = path.len - 1; i >= 0; i--) {
        char c = path.str[i];
        if (c == OS_PATH_SEPARATOR) {
            result = string_create(path.str, i);
            break;
        }
    }

    return result;
}

readonly global u8 __u8_from_symbol[128] = {
    [48] = 0x00, [49] = 0x01, [50] = 0x02, [51] = 0x03, [52] = 0x04,
    [53] = 0x05, [54] = 0x06, [55] = 0x07, [56] = 0x08, [57] = 0x09,
};

inline u64 string_parse_u64(String s)
{
    u64 val = 0;
    for (usize i = 0; i < s.len; i++) {
        val = val * 10 + __u8_from_symbol[s.str[i] & 0x7F];
    }

    return val;
}

StringList string_split(Arena *arena, String s, String delims)
{
    StringList result = {0};
    u8 *base, *str;
    base = str = s.str;
    for (usize i = 0; i < s.len; i++) {
        if (i == s.len - 1 || string_contains_char(delims, str[i])) {
            String elem = string_create(base, i - (base - str));
            string_list_push_back(arena, &result, elem);
            base = &str[i + 1];
        }
    }

    return result;
}

inline void string_list_push_back(Arena *arena, StringList *list, String s)
{
    StringNode *node = arena_alloc_array(arena, 1, StringNode);
    node->str = s;
    if (list->first == NULL) {
        list->first = node;
    } else {
        list->last->next = node;
    }

    list->node_count += 1;
    list->total_len += s.len;
    list->last = node;
}

inline void string_list_pop_front(StringList *list)
{
    if (list->node_count > 0) {
        StringNode *first = list->first;

        list->node_count -= 1;
        list->total_len -= first->str.len;
        list->first = first->next;
    }
}

inline String string_list_find_first_match(StringList *list, String needle)
{
    String result = {0};
    StringNode *node = list->first;
    for (usize i = 0; i < list->node_count; i++) {
        String str = node->str;
        if (string_contains(str, needle)) {
            result = str;
            break;
        }

        node = node->next;
    }

    return result;
}

inline String string_list_join(Arena *arena, StringList *list, String delim)
{
    if (list->node_count == 0) {
        return (String){0};
    }

    usize total_len = list->total_len + (delim.len * (list->node_count - 1));
    u8 *buf = arena_alloc(arena, total_len + 1);
    u8 *cur = buf;
    StringNode *cur_node = list->first;
    for (usize i = 0; i < list->node_count; i++) {
        String str = cur_node->str;
        usize len = str.len;
        mem_copy(cur, str.str, str.len);
        if (delim.len > 0 && cur_node != list->last) {
            mem_copy(&cur[len], delim.str, delim.len);
            len += delim.len;
        }

        cur_node = cur_node->next;
        cur += len;
    }

    return string_create(buf, total_len);
}

global u8 __log_buf[0x1000];
global Arena __log_arena = {
    .reserved = array_len(__log_buf),
    .committed = array_len(__log_buf),
    .memory = __log_buf,
};

#ifndef MODE_RELEASE
void assert_handle(
    const char *_prefix,
    const char *_cond,
    const char *_file,
    const char *_line,
    const char *_msg,
    ...
) {
    String prefix = string_lit(_prefix);
    String cond = string_lit(_cond);
    String file = string_lit(_file);
    String line = string_lit(_line);
    String msg = {0};
    if (_msg != NULL) {
        va_list va;
        va_start(va, _msg);
        msg = string_fmt(&__log_arena, _msg, va);
        va_end(va);
    }

    String postfix = {0};
    if (_cond != NULL) {
        postfix = string_fmt(&__log_arena, ": `{}`", cond);
    }

    log_fmt(LOG_ERROR, "{}({}): {}{} {}", file, line, prefix, postfix, msg);
}
#endif

inline void log_fmt(LogLevel level, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    log_fmt_va(level, fmt, va);
    va_end(va);
}

inline void log_fmt_va(LogLevel level, const char *fmt, va_list va)
{
    File file = {0};
    switch (level) {
    case LOG_INFO: {
        file = os_get_std_file(OS_STDOUT, true);
    } break;
    case LOG_ERROR: {
        file = os_get_std_file(OS_STDERR, true);
    } break;
    }
    if (file.handle == NULL) {
        return;
    }

    String newline = string_lit(OS_LINE_SEPARATOR);
    String level_str = string_lit(__LOG_LEVEL_TO_STRING[level]);
    String msg = string_fmt_va(&__log_arena, fmt, va);
    String line = string_fmt(&__log_arena, "[{}] {}{}", level_str, msg, newline);

    os_file_write_string(file, line);
}
#endif // BASE_H
