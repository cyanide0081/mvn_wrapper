// TODO(cya): mac(?)
#if defined(PLATFORM_WINDOWS)
#    include "win32/platform_core_win32.c"
#elif defined(PLATFORM_LINUX)
#    include "linux/platform_core_linux.c"
#else
#    error platform layer not implemented for this OS
#endif
