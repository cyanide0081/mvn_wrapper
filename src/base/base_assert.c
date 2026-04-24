#if defined(BUILD_DEBUG)
void assert_handle(
    const char *_prefix,
    const char *_cond,
    const char *_file,
    const char *_line,
    const char *_msg,
    ...
) {
    if (log.arena == NULL) {
        return;
    }

    Arena *arena = log.arena;
    String prefix = string_lit(_prefix);
    String cond = string_lit(_cond);
    String file = string_lit(_file);
    String line = string_lit(_line);
    String msg = {0};
    if (_msg != NULL) {
        va_list va;
        va_start(va, _msg);
        msg = string_fmt(arena, _msg, va);
        va_end(va);
    }

    String postfix = {0};
    if (_cond != NULL) {
        postfix = string_fmt(arena, ": `{}`", cond);
    }

    log_error("{}({}): {}{} {}", file, line, prefix, postfix, msg);
}
#endif
