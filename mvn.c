#include "base.h"

readonly global char JDK17_FLAGS[] = "--add-opens java.base/java.lang=ALL-UNNAMED";
readonly global char *POM_DIRS[] = {".", "java"};

String skip_first_arg(String cmd_line)
{
    String start = string_trim_leading(cmd_line);
    char c = start.str[0];
    if (string_contains_char(string_lit("\"'"), c)) {
        start = string_cut_leading(start, 1);
        return string_skip_first_match(start, string_from_char(c));
    }

    return string_trim_leading(string_skip_first_match(start, string_lit(" ")));
}

String build_command_line(Arena *arena, String mvn_path)
{
    String cmd_line = os_get_command_line(arena);
    String args = skip_first_arg(cmd_line);
    return string_fmt(arena, "cmd.exe /C \"{}\" {}", mvn_path, args);
}

#define COMMIT (usize)(1024 * 1024)

int main(void)
{
    Arena arena = arena_init(128 * COMMIT, COMMIT);
    if (arena.memory == NULL) {
        String error = os_get_error_message(os_get_last_error());
        log_fatal("unable to acquire virtual memory: {}", error);
        return 1;
    }

    String maven_home = os_get_env(&arena, string_lit("MAVEN_HOME"));
    String path = os_get_env(&arena, string_lit("PATH"));
    StringList path_list = string_split(&arena, path, string_lit(";"));
    String mvn_path = string_lit("");
    if (string_is_empty(maven_home)) {
        log_info("using maven from PATH");
        String pattern = string_lit("apache-maven");
        mvn_path = string_list_find_first_match(&path_list, pattern);
    } else {
        log_info("using maven from MAVEN_HOME");
        mvn_path = string_path_append(&arena, maven_home, string_lit("bin"));
    }

    if (string_is_empty(mvn_path)) {
        log_error("no maven installation found");
        return 1;
    }

    String mvn_launcher = string_path_append(&arena, mvn_path, string_lit("mvn"));
    if (!os_file_exists(&arena, mvn_launcher)) {
        log_error("maven launcher not found in {}", mvn_path);
        return 1;
    }

    File file = {0};
    String version = string_lit("");
    for (usize i = 0; i < array_len(POM_DIRS) && string_is_empty(version); i++) {
        String dir = string_from_cstring(POM_DIRS[i]);
        String path = string_path_append(&arena, dir, string_lit("pom.xml"));
        file = os_file_open(&arena, path);
        if (file.handle != NULL) {
            String pom = os_file_read_into_string(&arena, file);
            os_file_close(file);

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
            log_info("found JDK {} installation ({})", version, jdk_path);
            String jdk_key = string_lit("JAVA_HOME");
            os_set_env(&arena, jdk_key, jdk_path);
            if (string_parse_u64(version) == 17) {
                String mvn_key = string_lit("MAVEN_OPTS");
                os_set_env(&arena, mvn_key, string_from_cstring(JDK17_FLAGS));
            }
        }
    }

    log_info("running maven launcher ({})", mvn_path);

    String cmd_line = build_command_line(&arena, mvn_launcher);
    Process proc = os_process_spawn(&arena, cmd_line);
    if (proc.handle == NULL) {
        String error = os_get_error_message(os_get_last_error());
        log_error("unable to launch mvn: {}", error);
        return 1;
    }

    os_process_await(proc);
    return 0;
}
