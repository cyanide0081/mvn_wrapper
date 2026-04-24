#if defined(COMPILER_MSVC)
#    if _MSC_VER < 1300
#        define debug_trap() __asm int 3
#    else
#        define debug_trap() __debugbreak()
#    endif
#else
#    define debug_trap() __builtin_trap()
#endif

#if defined(BUILD_DEBUG)
#    define assert_msg(cond, ...) { \
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
#    define assert_msg(cond, ...) unused(cond)
#endif

#define assert(cond) assert_msg(cond, NULL)

#if defined(BUILD_DEBUG)
internal void assert_handle(
    const char *_prefix,
    const char *_cond,
    const char *_file,
    const char *_line,
    const char *_msg,
    ...
);
#endif
