internal void log_info(const char *fmt, ...);
internal void log_warn(const char *fmt, ...);
internal void log_error(const char *fmt, ...);
internal void log_fatal(const char *fmt, ...);
internal void log_debug(const char *fmt, ...);
internal void __log_va(const char *level, const char *fmt, va_list va);
