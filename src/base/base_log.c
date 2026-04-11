thread_local u8 __msg_buf[4096];

inline void log_info(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    __log_va("INFO", fmt, va);
    va_end(va);
}

inline void log_warn(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    __log_va("WARN", fmt, va);
    va_end(va);
}

inline void log_error(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    __log_va("ERROR", fmt, va);
    va_end(va);
}

inline void log_fatal(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    __log_va("FATAL", fmt, va);
    va_end(va);
}

inline void __log_va(const char *level, const char *fmt, va_list va)
{
    Arena arena = arena_init_from_buffer(__msg_buf, array_len(__msg_buf));
    File file = platform_get_std_file(PLATFORM_STDOUT, true);
    String newline = string_lit(PLATFORM_LINE_SEPARATOR);
    String msg = string_fmt_va(&arena, fmt, va);
    String log_level = string_from_cstring(level);
    String line = string_fmt(&arena, "[{}] {}{}", log_level, msg, newline);

    platform_file_write_string(file, line);
}
