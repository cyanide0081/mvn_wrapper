#include "base/base.h"
#include "platform/platform.h"

#include "base/base.c"
#include "platform/platform.c"

readonly force_keep char PROGRAM_NAME[] = "mvn wrapper v0.3";
readonly global char USER_PREFIX[] = "SENIOR";
readonly global char JDK17_FLAGS[] = "--add-opens java.base/java.lang=ALL-UNNAMED";
readonly global char *JDK_DIRS[] = {".jdks"};
readonly global char *POM_DIRS[] = {"", "java"};
readonly global char *JDK_VENDOR_PATTERNS[] = {"jdk", "temurin", "coretto"};

CommandLine build_mvn_command_line(Arena *arena, String mvn_path, StringList *arguments)
{
    String mvn_full_path = string_path_append(arena, mvn_path, PLATFORM_MVN_FILE);
    string_list_push_front(arena, arguments, mvn_full_path);
    string_list_push_front(arena, arguments, string_lit(PLATFORM_SHELL_CMD_FLAG));
    return (CommandLine){
        .exe_name = string_lit(PLATFORM_SHELL_NAME),
        .arguments = arguments,
    };
}

void entry_point(Arena *arena, CommandLine *cmd_line)
{
    log_debug("running {}", string_lit(PROGRAM_NAME));

    String curr_user = platform_get_current_username(arena);
    String home = platform_get_home_directory(arena);
    log_debug("user: {} home: {}", curr_user, home);

    String maven_home = platform_get_env(arena, string_lit("MAVEN_HOME"));
    String path = platform_get_env(arena, string_lit("PATH"));
    String delimiter = string_lit(PLATFORM_ENV_SEPARATOR);
    StringList path_list = string_split(arena, path, delimiter);
    String mvn_path = string_lit("");
    if (!string_is_empty(maven_home)) {
        log_info("using maven from MAVEN_HOME");
        mvn_path = string_path_append(arena, maven_home, PLATFORM_MVN_FILE);
    } else {
        log_info("using maven from PATH");

        // NOTE(cya): exclude ourselves from the paths to search for
        String process_exe = platform_get_process_filename(arena);
        String process_dir = string_path_pop_element(process_exe);
        string_list_pop_matches(&path_list, process_dir);

        mvn_path = platform_find_first_file(arena, &path_list, PLATFORM_MVN_FILE);
    }

    if (string_is_empty(mvn_path)) {
        log_error("no maven directory found (check your PATH or MAVEN_HOME)");
        return;
    }

    String mvn_launcher = string_path_append(arena, mvn_path, PLATFORM_MVN_FILE);
    if (!platform_file_exists(arena, mvn_launcher)) {
        log_error("maven launcher not found @ {}", mvn_path);
        return;
    }

    String version = string_lit("");
    for (usize i = 0; i < array_len(POM_DIRS) && string_is_empty(version); i++) {
        String dir = string_from_cstring(POM_DIRS[i]);
        String path = string_path_append(arena, dir, string_lit("pom.xml"));
        File file = platform_file_open(arena, path);
        if (platform_file_is_valid(file)) {
            String pom = platform_file_read_into_string(arena, file);
            platform_file_close(file);

            String target_tag = string_lit("<maven.compiler.target>");
            String pos = string_skip_first_match(pom, target_tag);
            version = string_keep_number(pos);
        }
    }

    String jdk_path = string_lit("");
    if (string_is_empty(version)) {
        log_warn("no JDK target property found (using JAVA_HOME)");
    } else {
        log_info("found JDK {} target in pom.xml", version);

        // NOTE(cya): prepend dirs from known install locations
        for (usize i = 0; i < array_len(JDK_DIRS); i++) {
            String dir = string_from_cstring(JDK_DIRS[i]);
            String full_dir = string_path_append(arena, home, dir);
            if (platform_file_exists(arena, full_dir)) {
                FileInfo info;
                FileIter *iter = platform_file_iter_begin(arena, full_dir);
                while (platform_file_iter_next(arena, iter, &info)) {
                    if (info.is_dir) {
                        String path = string_path_append(arena, full_dir, info.name);
                        string_list_push_front(arena, &path_list, path);
                    }
                }

                platform_file_iter_end(iter);
            }
        }

        // NOTE(cya): build our vendor-specific install-path patterns
        StringList patterns = {0};
        for (usize i = 0; i < array_len(JDK_VENDOR_PATTERNS); i++) {
            String vendor = string_from_cstring(JDK_VENDOR_PATTERNS[i]);
            String pattern = string_fmt(arena, "{}-{}", vendor, version);
            string_list_push_back(arena, &patterns, pattern);
        }

        jdk_path = string_list_find_first_match(&path_list, &patterns);
        if (string_is_empty(jdk_path)) {
            log_warn("found no JDK {} installation (using JAVA_HOME)", version);
        } else {
            String last_element = string_path_get_last_element(jdk_path);
            if (string_equals(last_element, string_lit("bin"))) {
                jdk_path = string_path_pop_element(jdk_path);
            }

            log_info("found JDK {} installation @ {}", version, jdk_path);
            String jdk_key = string_lit("JAVA_HOME");
            platform_set_env(arena, jdk_key, jdk_path);

            u64 version_num = string_parse_u64(version);
            String user_prefix = string_lit(USER_PREFIX);
            if (version_num == 17 && string_starts_with(curr_user, user_prefix)) {
                platform_set_env(arena, string_lit("MAVEN_OPTS"), string_lit(JDK17_FLAGS));
            }
        }
    }

    log_info("running maven launcher @ {}", mvn_path);

    CommandLine mvn_cmd_line = build_mvn_command_line(arena, mvn_path, cmd_line->arguments);
    Process proc = platform_process_spawn(arena, &mvn_cmd_line);
    b32 success = platform_process_await(proc);
    if (!success) {
        String error = platform_get_error_message(platform_get_last_error());
        log_error("unable to launch mvn: {}", error);
        return;
    }
}
