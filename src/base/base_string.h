typedef struct {
    usize len;
    u8 *str;
} String;

typedef struct {
    usize len;
    u16 *str;
} String16;

typedef struct StringNode {
    struct StringNode *next;
    String str;
} StringNode;

typedef struct {
    usize node_count;
    usize total_len;
    StringNode *first;
    StringNode *last;
} StringList;

#define string_is_empty(s) ((s).len == 0)
#define string_create(s, l) ((String){.len = (usize)l, .str = (u8*)(s)})
#define string_lit(s) string_create((s), sizeof(s) - 1)
#define string_contains_char(s, c) string_contains((s), string_create(&(c), 1))
#define string_equals(a, b) ((a).len == (b).len && mem_equal((a).str, (b).str, (a).len))

#define string16_create(s, l) ((String16){.len = (usize)l, .str = (u16*)s})

#define string_list_for_each(arena, list, proc) { \
    StringNode *curr_ = list->first; \
    for (usize i_ = 0; i_ < list->node_count; i_++) { \
        String current_ = curr_->str; \
        String transformed_ = proc(arena, curr_->str); \
        list->total_len += (current_.len - transformed_.len); \
        curr_->str = transformed_; \
        curr_ = curr_->next; \
    } \
} (void)0

internal String string_from_cstring(const char *str);
internal String16 string16_from_wcstring(const u16 *str);

internal char *string_to_cstring(Arena *arena, String s);

internal b32 string_starts_with(String a, String b);
internal b32 string_contains(String haystack, String needle);
internal String string_keep_number(String s);
internal String string_fmt(Arena *arena, const char *fmt, ...);
internal String string_fmt_va(Arena *arena, const char *fmt, va_list va);
internal String string_skip_first_match(String s, String target);
internal String string_skip_nth_match(String s, String target, usize n);
internal String string_trunc(String s, usize len);
internal String string_cut_leading(String s, usize n);
internal String string_trim_leading(String s);
internal String string_trim_trailing(String s);
internal String string_path_append(Arena *arena, String path, String elem);
internal String string_path_pop(String path);
internal String string_from_u64(Arena *arena, u64 val);
internal u64 string_parse_u64(String s);

internal StringNode *string_node_create(Arena *arena, String s);
internal StringList string_split(Arena *arena, String s, String delims);
internal char **string_list_to_cstrings(Arena *arena, StringList *list, int *out_len);

internal void string_list_push_back(Arena *arena, StringList *list, String s);
internal void string_list_push_front(Arena *arena, StringList *list, String s);
internal String string_list_pop_front(StringList *list);
internal String string_list_find_first_match(StringList *list, String needle);
internal void string_list_pop_matches(StringList *list, String needle);
internal String string_list_join(Arena *arena, StringList *list, String delim);
