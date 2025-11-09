#include "base.h"

#define COMMIT (1024 * 1024)

readonly global String JDK17_FLAGS =
    string_lit("--add-opens java.base/java.lang=ALL-UNNAMED");
readonly global String POM_PATHS[] = {
    string_lit("pom.xml"),
    string_lit("java/pom.xml"),
};

String build_command_line(Arena *arena, String mvn_path)
{
    String script_name = string_lit("mvn.cmd");
    String script = string_path_append(arena, mvn_path, script_name);
    String cmd_line = os_get_command_line(arena);
    String needle = string_lit("\"");
    String cmd_line_trimmed = string_skip_nth_match(cmd_line, needle, 2);
    return string_fmt(arena, "cmd.exe /C \"{}\" {}", script, cmd_line_trimmed);
}

int main(void)
{
    Arena arena = arena_init(128 * COMMIT, COMMIT);
    if (arena.memory == NULL) {
        File stream = os_get_std_file(OS_STDERR, true);
        log(stream, string_lit("[FATAL] memory allocation error"));
        return 1;
    }

    String maven_home = os_get_env(&arena, string_lit("MAVEN_HOME"));
    String path = os_get_env(&arena, string_lit("PATH"));
    StringList path_list = string_split(&arena, path, string_lit(";"));
    String mvn_path = string_is_empty(maven_home) ?
        string_list_find_first_match(&path_list, string_lit("apache-maven")) :
        string_path_append(&arena, maven_home, string_lit("bin"));
    if (string_is_empty(mvn_path)) {
        log_fmt(&arena, LOG_ERROR, "maven not found\n");
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
    if (!string_is_empty(version)) {
        String jdk_target = string_fmt(&arena, "jdk-{}", version);
        jdk_path = string_list_find_first_match(&path_list, jdk_target);
        if (!string_is_empty(jdk_path)) {
            String jdk_key = string_lit("JAVA_HOME");
            os_set_env(&arena, jdk_key, string_path_pop(jdk_path));
            if (string_parse_u64(version) == 17) {
                String mvn_key = string_lit("MAVEN_OPTS");
                os_set_env(&arena, mvn_key, JDK17_FLAGS);
            }
        }
    }

    b32 ok = os_spawn_process(&arena, build_command_line(&arena, mvn_path));
    return !ok;
}
