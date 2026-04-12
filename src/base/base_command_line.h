typedef struct {
    String exe_name;
    StringList *arguments;
} CommandLine;

internal CommandLine command_line_from_string_list(StringList *arguments);
internal char **command_line_to_argv(Arena *arena, CommandLine *cmd_line, int *out_argc);
internal String command_line_escape_string(Arena *arena, String s);
