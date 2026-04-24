inline CommandLine command_line_from_string_list(StringList *arguments)
{
    return (CommandLine){
        .exe_name = string_list_pop_front(arguments),
        .arguments = arguments,
    };
}

inline char **command_line_to_argv(Arena *arena, CommandLine *cmd_line, int *out_argc)
{
    StringList *arguments = cmd_line->arguments;
    string_list_push_front(arena, arguments, cmd_line->exe_name);
    return string_list_to_cstrings(arena, arguments, out_argc);
}

inline String command_line_escape_string(Arena *arena, String s)
{
    return string_contains_whitespace(s) ? string_fmt(arena, "\"{}\"", s) : s;
}
