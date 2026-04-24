typedef struct {
    Arena *arena;
} Log;

global thread_local Log log;

#define log_info(...) __log("INFO", __VA_ARGS__)
#define log_warn(...) __log("WARN", __VA_ARGS__)
#define log_error(...) __log("ERROR", __VA_ARGS__)
#define log_fatal(...) __log("FATAL", __VA_ARGS__)

internal void __log(const char *level, const char *fmt, ...);
internal void log_debug(const char *fmt, ...);
internal void __log_va(const char *level, const char *fmt, va_list va);
