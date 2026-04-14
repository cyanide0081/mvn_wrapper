inline usize platform_get_page_size(void)
{
    return sysconf(_SC_PAGESIZE);
}

inline void *platform_mem_reserve(void *addr, usize size)
{
    void *result = mmap(addr, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return result == MAP_FAILED ? NULL : result;
}

inline void *platform_mem_commit(void *addr, usize size)
{
    mprotect(addr, size, PROT_READ | PROT_WRITE);
    return addr;
}

inline void platform_mem_release(void *addr, usize size)
{
    munmap(addr, size);
}

String platform_get_process_filename(Arena *arena)
{
    usize buf_size = PATH_MAX;
    char *buf = arena_push(arena, buf_size);
    i32 len = readlink("/proc/self/exe", buf, buf_size - 1);
    if (len == -1) {
        return string_lit("");
    }

    buf[len] = '\0';
    return string_create(buf, len);
}

String platform_get_env(Arena *arena, String key)
{
    unused(arena); // NOTE(cya): no need to allocate in POSIX

    char **variables = __environ;
    while (*variables != NULL) {
        String current = string_from_cstring(*variables++);
        String current_key = string_trunc(current, key.len);
        if (string_equals(current_key, key)) {
            String result = string_skip_first_match(current, string_lit("="));
            return result;
        }
    }

    return string_lit("");
}

inline void platform_set_env(Arena *arena, String key, String val)
{
    char **variables = __environ;
    while (*variables != NULL) {
        String current = string_trunc(string_from_cstring(*variables), key.len);
        if (string_equals(current, key)) {
            String new = string_fmt(arena, "{}={}", key, val);
            *variables = string_to_cstring(arena, new);
            return;
        }

        variables += 1;
    }
}

internal inline usize linux_file_size(i32 descriptor)
{
    if (descriptor == -1) {
        return 0;
    }

    struct stat st;
    fstat(descriptor, &st);
    return st.st_size;
}

File platform_file_open(Arena *arena, String path)
{
    int descriptor = open(string_to_cstring(arena, path), O_RDONLY);
    return (File){
        .descriptor = descriptor,
        .size = linux_file_size(descriptor),
    };
}

b32 platform_file_close(File file)
{
    return close(file.descriptor) == 0;
}

b32 platform_file_exists(Arena *arena, String path)
{
    return access(string_to_cstring(arena, path), F_OK) == 0;
}

inline String platform_file_read_into_string(Arena *arena, File file)
{
    usize size = file.size;
    u8 *buf = arena_push(arena, size);
    read(file.descriptor, buf, size);
    return string_create(buf, size);
}

FileIter *platform_file_iter_begin(Arena *arena, String path)
{
    FileIter *iter = arena_push_array(arena, 1, FileIter);
    iter->dir = opendir(string_to_cstring(arena, path));
    return iter;
}

b32 platform_file_iter_next(Arena *arena, FileIter *iter, FileInfo *info)
{
    unused(arena);
    
    while (iter->dir != NULL) {
        iter->entry = readdir(iter->dir);
        if (iter->entry == NULL) {
            break;
        } else if (iter->entry->d_name[0] == '.') {
            continue;
        }
        
        const char *name = iter->entry->d_name;
        struct stat st;
        stat(name, &st);
        
        info->name = string_from_cstring(iter->entry->d_name);
        info->is_dir = (st.st_mode & S_IFMT) == S_IFDIR;
        return true;
    }

    iter->is_done = true;
    return false;
}

void platform_file_iter_end(FileIter *iter)
{
    closedir(iter->dir);
}

inline void platform_file_write_string(File file, String s)
{
    write(file.descriptor, s.str, s.len);
}

#define ERROR_STATUS 255

inline Process platform_process_spawn(Arena *arena, CommandLine *cmd_line)
{
    pid_t pid = fork();
    if (pid == 0) {
        // NOTE(cya): child process branch
        int argc;
        char **argv = command_line_to_argv(arena, cmd_line, &argc);
        execve(argv[0], argv, __environ);
        _exit(ERROR_STATUS); // NOTE(cya): if we get down here it's because exec failed
    }

    return (Process){.pid = pid};
}

inline b32 platform_process_failed(Process process)
{
    return process.pid == -1;
}

inline b32 platform_process_await(Process process)
{
    if (platform_process_failed(process)) {
        return false;
    }

    int w_status;
    do {
        pid_t status = waitpid(process.pid, &w_status, WUNTRACED | WCONTINUED);
        if (status == -1) {
            return false;
        }
    } while (!WIFEXITED(w_status));

    return WEXITSTATUS(w_status) != ERROR_STATUS;
}

thread_local u8 __linux_error_buf[4096];

inline u64 platform_get_last_error(void)
{
    return (u64)errno;
}

String platform_get_error_message(u64 error_code)
{
    char *msg = strerror((int)error_code);
    return string_from_cstring(msg);
}

inline String platform_get_current_username(Arena *arena)
{
    String username = string_from_cstring(getlogin());
    return !string_is_empty(username) ? username :
        platform_get_env(arena, string_lit("LOGNAME"));
}

inline String platform_get_home_directory(Arena *arena)
{
    return platform_get_env(arena, string_lit("HOME"));
}

int main(int argc, char *argv[])
{
    __platform_std_files[STDIN].descriptor = STDIN;
    __platform_std_files[STDOUT].descriptor = STDOUT;
    __platform_std_files[STDERR].descriptor = STDERR;

    Arena arena = platform_init_main_arena();
    if (arena.memory == NULL) {
        return 1;
    }

    StringList arguments = {0};
    for (int i = 0; i < argc; i++) {
        String argument = string_from_cstring(argv[i]);
        string_list_push_back(&arena, &arguments, argument);
    }

    log.arena = &arena;

    CommandLine cmd_line = command_line_from_string_list(&arguments);
    entry_point(&arena, &cmd_line);

#if defined(BUILD_DEBUG)
    arena_log_stats(&arena);
#endif
}
