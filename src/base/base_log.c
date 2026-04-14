inline void __log(const char *level, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    __log_va(level, fmt, va);
    va_end(va);
}

inline void log_debug(const char *fmt, ...)
{
#if defined(BUILD_DEBUG)
    va_list va;
    va_start(va, fmt);
    __log_va("DEBUG", fmt, va);
    va_end(va);
#else
    unused(fmt);
#endif
}

inline void __log_va(const char *level, const char *fmt, va_list va)
{
    if (log.arena == NULL) {
        return;
    }

    File file = platform_get_std_file(STDOUT);
    String newline = string_lit(PLATFORM_LINE_SEPARATOR);
    String msg = string_fmt_va(log.arena, fmt, va);
    String log_level = string_from_cstring(level);
    String line = string_fmt(log.arena, "[{}] {}{}", log_level, msg, newline);

    platform_file_write_string(file, line);
}
