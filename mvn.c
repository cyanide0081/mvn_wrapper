#include "base.h"

#define COMMIT (1024 * 1024)

readonly global String JDK17_FLAGS =
    string_lit("--add-opens java.base/java.lang=ALL-UNNAMED");
readonly global String POM_PATHS[] = {
    string_lit("pom.xml"),
    string_lit("java/pom.xml"),
};

String skip_first_arg(String cmd_line)
{
    String start = string_trim_leading(cmd_line);
    char c = start.str[0];
    if (string_contains_char(string_lit("\"'"), c)) {
        start = string_cut_leading(start, 1);
        return string_skip_first_match(start, string_from_char(c));
    }

    return string_skip_first_match(start, string_lit(" "));
}

String build_command_line(Arena *arena, String mvn_path)
{
    String cmd_line = os_get_command_line(arena);
    String args = skip_first_arg(cmd_line);
    return string_fmt(arena, "cmd.exe /C \"{}\" {}", mvn_path, args);
}

int main(void)
{
    Arena arena = arena_init(128 * COMMIT, COMMIT);
    if (arena.memory == NULL) {
        log_fmt(LOG_ERROR, "fatal memory allocation error");
        return 1;
    }

    String maven_home = os_get_env(&arena, string_lit("MAVEN_HOME"));
    String path = os_get_env(&arena, string_lit("PATH"));
    StringList path_list = string_split(&arena, path, string_lit(";"));
    String mvn_path = {0};
    if (string_is_empty(maven_home)) {
        log_fmt(LOG_INFO, "searching for maven in PATH");
        String pattern = string_lit("apache-maven");
        mvn_path = string_list_find_first_match(&path_list, pattern);
    } else {
        log_fmt(LOG_INFO, "using maven from MAVEN_HOME");
        mvn_path = string_path_append(&arena, maven_home, string_lit("bin"));
    }

    if (string_is_empty(mvn_path)) {
        log_fmt(LOG_ERROR, "maven not found\n");
        return 1;
    }

    File file = {0};
    for (usize i = 0; i < array_len(POM_PATHS) && file.handle == NULL; i++) {
        file = os_file_open(&arena, POM_PATHS[i]);
    }

    String version = {0};
    if (file.handle != NULL) {
        String pom = os_file_read_into_string(&arena, file);
        os_file_close(file);

        String target_tag = string_lit("<maven.compiler.target>");
        String pos = string_skip_first_match(pom, target_tag);
        version = string_keep_number(pos);
    }

    String jdk_path = {0};
    if (string_is_empty(version)) {
        log_fmt(LOG_INFO, "no JDK target property found (using JAVA_HOME)");
    } else {
        log_fmt(LOG_INFO, "found JDK {} target in pom", version);

        String jdk_target = string_fmt(&arena, "jdk-{}", version);
        jdk_path = string_list_find_first_match(&path_list, jdk_target);
        if (string_is_empty(jdk_path)) {
            log_fmt(LOG_INFO, "found no JDK {} installation (using JAVA_HOME)", version);
        } else {
            jdk_path = string_path_pop(jdk_path);
            log_fmt(LOG_INFO, "found JDK {} installation ({})", version, jdk_path);
            String jdk_key = string_lit("JAVA_HOME");
            os_set_env(&arena, jdk_key, jdk_path);
            if (string_parse_u64(version) == 17) {
                String mvn_key = string_lit("MAVEN_OPTS");
                os_set_env(&arena, mvn_key, JDK17_FLAGS);
            }
        }
    }

    mvn_path = string_path_append(&arena, mvn_path, string_lit("mvn.cmd"));
    log_fmt(LOG_INFO, "running maven script ({})", mvn_path);

    String cmd_line = build_command_line(&arena, mvn_path);
    b32 ok = os_spawn_process(&arena, cmd_line);
    return !ok;
}
