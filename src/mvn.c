#include "base/base.h"
#include "platform/platform.h"

#include "base/base.c"
#include "platform/platform.c"

readonly force_keep char PROGRAM_NAME[] = "mvn wrapper v0.2";
readonly global char JDK17_FLAGS[] = "--add-opens java.base/java.lang=ALL-UNNAMED";
readonly global char *POM_DIRS[] = {"", "java"};

CommandLine build_mvn_command_line(Arena *arena, String mvn_path, StringList *arguments)
{
    String mvn_full_path = string_path_append(arena, mvn_path, string_lit("mvn"));
    string_list_push_front(arena, arguments, mvn_full_path);
    string_list_push_front(arena, arguments, string_lit(PLATFORM_SHELL_CMD_FLAG));
    return (CommandLine){
        .exe_name = string_lit(PLATFORM_SHELL_NAME),
        .arguments = arguments,
    };
}

void entry_point(CommandLine *cmd_line)
{
    Arena arena = arena_init(16, mebibytes(1));
    if (arena.memory == NULL) {
        String error = platform_get_error_message(platform_get_last_error());
        log_fatal("unable to acquire virtual memory: {}", error);
        return;
    }

    log_debug("running {}", string_lit(PROGRAM_NAME));

    String maven_home = platform_get_env(&arena, string_lit("MAVEN_HOME"));
    String path = platform_get_env(&arena, string_lit("PATH"));
    String delimiter = string_lit(PLATFORM_ENV_SEPARATOR);
    StringList path_list = string_split(&arena, path, delimiter);
    String mvn_path = string_lit("");
    if (!string_is_empty(maven_home)) {
        log_info("using maven from MAVEN_HOME");
        mvn_path = string_path_append(&arena, maven_home, string_lit("bin"));
    } else {
        log_info("using maven from PATH");

        // NOTE(cya): exclude ourselves from the paths to search for
        String process_exe = platform_get_process_filename(&arena);
        String process_dir = string_path_pop(process_exe);
        string_list_pop_matches(&path_list, process_dir);

        String pattern = string_lit("apache-maven");
        mvn_path = string_list_find_first_match(&path_list, pattern);
    }

    if (string_is_empty(mvn_path)) {
        log_error("no maven directory found (check your PATH or MAVEN_HOME)");
        return;
    }

    String mvn_launcher = string_path_append(&arena, mvn_path, string_lit("mvn"));
    if (!platform_file_exists(&arena, mvn_launcher)) {
        log_error("maven launcher not found @ {}", mvn_path);
        return;
    }

    File file = {0};
    String version = string_lit("");
    for (usize i = 0; i < array_len(POM_DIRS) && string_is_empty(version); i++) {
        String dir = string_from_cstring(POM_DIRS[i]);
        String path = string_path_append(&arena, dir, string_lit("pom.xml"));
        file = platform_file_open(&arena, path);
        if (platform_file_is_valid(file)) {
            String pom = platform_file_read_into_string(&arena, file);
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

        String jdk_target = string_fmt(&arena, "jdk-{}", version);
        jdk_path = string_list_find_first_match(&path_list, jdk_target);
        if (string_is_empty(jdk_path)) {
            log_info("found no JDK {} installation (using JAVA_HOME)", version);
        } else {
            jdk_path = string_path_pop(jdk_path);
            log_info("found JDK {} installation @ {}", version, jdk_path);
            String jdk_key = string_lit("JAVA_HOME");
            platform_set_env(&arena, jdk_key, jdk_path);
            if (string_parse_u64(version) == 17) {
                String mvn_key = string_lit("MAVEN_OPTS");
                platform_set_env(&arena, mvn_key, string_from_cstring(JDK17_FLAGS));
            }
        }
    }

    log_info("running maven launcher @ {}", mvn_path);

    CommandLine mvn_cmd_line = build_mvn_command_line(&arena, mvn_path, cmd_line->arguments);
    Process proc = platform_process_spawn(&arena, &mvn_cmd_line);
    b32 success = platform_process_await(proc);
    if (!success) {
        String error = platform_get_error_message(platform_get_last_error());
        log_error("unable to launch mvn: {}", error);
        return;
    }
}
