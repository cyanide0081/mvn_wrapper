#if defined(__clang__) // NOTE(cya): clang context
#    define COMPILER_CLANG

#    if defined(_WIN32)
#        define PLATFORM_WINDOWS
#    elif defined(__linux__) || defined(__gnu_linux__)
#        define PLATFORM_LINUX
#    elif defined(__MACH__) && defined(__APPLE__)
#        define PLATFORM_MAC
#    else
#        error unsupported compiler/platform combo
#    endif

#    if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#        define ARCH_X64
#    elif defined(__aarch64__)
#        define ARCH_ARM64
#    else
#        error unsupported architecture
#    endif

#elif defined(_MSC_VER) // NOTE(cya): MSVC context
#    define COMPILER_MSVC

#    if defined(_WIN32)
#        define PLATFORM_WINDOWS
#    else
#        error unsupported compiler/platform combo
#    endif

#    if defined(_M_AMD64)
#        define ARCH_X64
#    elif defined(_M_ARM64)
#        define ARCH_ARM64
#    else
#        error unsupported architecture
#    endif
#elif defined(__GNUC__) || defined(__GNUG__) // NOTE(cya): GCC context
#    define COMPILER_GCC

#    if defined(__linux__) || defined(__gnu_linux__)
#        define PLATFORM_LINUX
#    else
#        error unsupported compiler/platform combo
#    endif

#    if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#        define ARCH_X64
#    elif defined(__aarch64__)
#        define ARCH_ARM64
#    else
#        error unsupported architecture
#    endif
#else
#    error unsupported compiler
#endif

#if !defined(NDEBUG)
#    define BUILD_DEBUG
#endif
