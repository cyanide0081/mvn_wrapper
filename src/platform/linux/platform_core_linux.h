#include <unistd.h> // syscalls
#include <fcntl.h> // open
#include <errno.h> // errno
#include <string.h> // strerror
#include <dirent.h> // opendir
#include <limits.h> // PATH_MAX
#include <sys/stat.h> // stat
#include <sys/wait.h> // wait
#include <sys/mman.h> // mmap

typedef struct {
    usize size;
    i32 descriptor;
} File;

typedef struct {
    i32 pid;
} Process;

typedef struct {
    DIR *dir;
    struct dirent *entry;
    b32 is_done;
} FileIter;

#if !defined(MAP_ANONYMOUS)
#    define MAP_ANONYMOUS MAP_ANON
#endif

#define PLATFORM_PATH_SEPARATOR "/"
#define PLATFORM_LINE_SEPARATOR "\n"
#define PLATFORM_ENV_SEPARATOR ":"

#define PLATFORM_SHELL_NAME "/bin/sh"
#define PLATFORM_SHELL_CMD_FLAG "-c"

#define PLATFORM_MVN_FILE string_lit("mvn")

#define platform_mem_equal(a, b, len) (memcmp(a, b, len) == 0)
#define platform_mem_copy(d, s, len) memcpy(d, s, len)

#define platform_file_is_valid(f) ((f).descriptor != -1)
