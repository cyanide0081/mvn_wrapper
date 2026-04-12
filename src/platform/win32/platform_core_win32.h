#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct {
    usize size;
    void *handle;
} File;

typedef struct {
    void *handle;
} Process;

#define PLATFORM_PATH_SEPARATOR "\\"
#define PLATFORM_LINE_SEPARATOR "\r\n"
#define PLATFORM_ENV_SEPARATOR ";"

#define PLATFORM_SHELL_NAME "cmd"
#define PLATFORM_SHELL_CMD_FLAG "/C"

#define platform_mem_equal(a, b, len) RtlEqualMemory(a, b, len)
#define platform_mem_copy(d, s, len) RtlCopyMemory(d, s, len)

#define platform_file_is_valid(f) ((f).handle != NULL)
