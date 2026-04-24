inline String string_from_cstring(const char *str)
{
    return string_create(str, cstring_len(str));
}

inline String16 string16_from_wcstring(const u16 *str)
{
    return string16_create(str, wcstring_len(str));
}

inline char *string_to_cstring(Arena *arena, String s)
{
    char *result = arena_push(arena, s.len + 1);
    mem_copy(result, s.str, s.len);
    result[s.len] = '\0';
    return result;
}

inline b32 string_starts_with(String a, String b)
{
    if (b.len > a.len) {
        return false;
    }

    for (usize i = 0; i < b.len; i++) {
        if (a.str[i] != b.str[i]) {
            return false;
        }
    }

    return true;
}

inline b32 string_contains(String haystack, String needle)
{
    for (usize i = 0; i < haystack.len; i++) {
        b32 found_match = haystack.str[i] == needle.str[0] &&
            mem_equal(&haystack.str[i], needle.str, needle.len);
        if (found_match) {
            return true;
        }
    }

    return false;
}

inline String string_keep_number(String s)
{
    usize i = 0;
    for (; i < s.len && !char_is_digit(s.str[i]); i++) {}

    usize j = i;
    for (; j < s.len && char_is_digit(s.str[j]); j++) {}

    return string_create(&s.str[i], j - i);
}

inline String string_fmt(Arena *arena, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    String str = string_fmt_va(arena, fmt, va);
    va_end(va);
    return str;
}

inline String string_fmt_va(Arena *arena, const char *fmt, va_list va)
{
    StringList parts = {0};
    usize cur = 0, i = 0;
    for (; fmt[i] != '\0'; i++) {
        if (fmt[i] == '{' && fmt[i + 1] == '}') {
            if (i > 0 && fmt[i - 1] == '\\') {
                continue;
            }

            usize len = i - cur;
            if (len > 0) {
                string_list_push_back(arena, &parts, string_create(&fmt[cur], len));
            }

            string_list_push_back(arena, &parts, va_arg(va, String));

            cur += len + 2;
            i += 1;
        }
    }

    string_list_push_back(arena, &parts, string_create(&fmt[cur], i - cur));
    return string_list_join(arena, &parts, string_lit(""));
}

String string_skip_first_match(String s, String target)
{
    return string_skip_nth_match(s, target, 1);
}

String string_skip_nth_match(String s, String target, usize n)
{
    usize matches = 0;
    usize len = s.len;
    usize i = 0;
    for (; i < len; i++) {
        if (mem_equal(&s.str[i], target.str, target.len)) {
            matches += 1;
            i += target.len;
            if (matches == n) {
                break;
            }
        }
    }

    return string_create(&s.str[i], len - i);
}

b32 string_contains_whitespace(String s)
{
    for (usize i = 0; i < s.len; i++) {
        if (char_is_whitespace(s.str[i])) {
            return true;
        }
    }

    return false;
}

inline String string_join(Arena *arena, String delim, String a, String b)
{
    usize len = a.len + delim.len + b.len;
    u8 *buf = arena_push(arena, len + 1);

    mem_copy(buf, a.str, a.len);
    mem_copy(&buf[a.len], delim.str, delim.len);
    mem_copy(&buf[a.len + delim.len], b.str, b.len);

    return string_create(buf, len);
}

inline String string_cut_leading(String s, usize n)
{
    usize i = min(n, s.len);
    return string_create(&s.str[i], s.len - i);
}

inline String string_trim_leading(String s)
{
    usize i = 0;
    while (i < s.len && char_is_whitespace(s.str[i])) {
        i += 1;
    }

    return string_create(&s.str[i], s.len - i);
}

inline String string_trim_trailing(String s)
{
    isize i = s.len - 1;
    while (i >= 0 && char_is_whitespace(s.str[i])) {
        i -= 1;
    }

    return string_create(s.str, i);
}

inline String string_path_append(Arena *arena, String path, String elem)
{
    return string_join(arena, string_lit(PLATFORM_PATH_SEPARATOR), path, elem);
}

inline String string_path_get_last_element(String path)
{
    for (usize i = 0; i < path.len; i++) {
        usize index = path.len - i - 1;
        if (PLATFORM_PATH_SEPARATOR[0] == path.str[index]) {
            return string_create(&path.str[index + 1], i);
        }
    }

    return string_lit("");
}

inline String string_path_pop_element(String path)
{
    for (usize i = 0; i < path.len; i++) {
        usize index = path.len - i - 1;
        if (PLATFORM_PATH_SEPARATOR[0] == path.str[index]) {
            return string_create(path.str, index);
        }
    }

    return string_lit("");
}

readonly global u8 __SYMBOL_FROM_U8[10] = {
    [0] = 48, [1] = 49, [2] = 50, [3] = 51, [4] = 52,
    [5] = 53, [6] = 54, [7] = 55, [8] = 56, [9] = 57
};
readonly global u8 __U8_FROM_SYMBOL[128] = {
    [48] = 0x00, [49] = 0x01, [50] = 0x02, [51] = 0x03, [52] = 0x04,
    [53] = 0x05, [54] = 0x06, [55] = 0x07, [56] = 0x08, [57] = 0x09,
};

thread_local u8 __u64_buffer[20 + 1];

inline String string_from_u64(Arena *arena, u64 val)
{
    u64 radix = 10;
    u64 reduced = val;
    usize digits = 0;
    do {
        reduced /= radix;
        digits += 1;
    } while (reduced != 0);
    u8* buf = arena == NULL ? __u64_buffer : arena_push(arena, digits + 1);
    for (usize i = 0; i < digits; i++) {
        buf[digits - i - 1] = __SYMBOL_FROM_U8[val % radix];
        val /= radix;
    }

    return string_create(buf, digits);
}

inline u64 string_parse_u64(String s)
{
    u64 val = 0;
    for (usize i = 0; i < s.len; i++) {
        val = val * 10 + __U8_FROM_SYMBOL[s.str[i] & 0x7F];
    }

    return val;
}

inline StringNode *string_node_create(Arena *arena, String s)
{
    StringNode *result = arena_push_array(arena, 1, StringNode);
    result->str = s;
    return result;
}

StringList string_split(Arena *arena, String s, String delims)
{
    StringList result = {0};
    u8 *base, *str;
    base = str = s.str;
    for (usize i = 0; i < s.len; i++) {
        if (i == s.len - 1 || string_contains_char(delims, str[i])) {
            String elem = string_create(base, i - (base - str));
            string_list_push_back(arena, &result, elem);
            base = &str[i + 1];
        }
    }

    return result;
}

inline char **string_list_to_cstrings(Arena *arena, StringList *list, int *out_len)
{
    usize len = list->node_count;
    char **result = arena_push_array(arena, len + 1, char*);
    usize i = 0;
    string_list_foreach(list, node) {
        String str = node->str;
        result[i++] = string_to_cstring(arena, str);
    }

    *out_len = len;
    result[len] = NULL;
    return result;
}

inline void string_list_push_node_back(StringList *list, StringNode *node)
{
    sll_queue_push_back(list->first, list->last, node);
    list->node_count += 1;
    list->total_len += node->str.len;
}

inline void string_list_push_node_front(StringList *list, StringNode *node)
{
    sll_queue_push_front(list->first, list->last, node);
    list->node_count += 1;
    list->total_len += node->str.len;
}

inline void string_list_push_back(Arena *arena, StringList *list, String s)
{

    StringNode *node = string_node_create(arena, s);
    string_list_push_node_back(list, node);
}

inline void string_list_push_front(Arena *arena, StringList *list, String s)
{
    StringNode *node = string_node_create(arena, s);
    string_list_push_node_front(list, node);
}

inline String string_list_pop_front(StringList *list)
{
    if (list->node_count == 0) {
        return string_lit("");
    }

    String result = list->first->str;
    list->node_count -= 1;
    list->total_len -= result.len;
    sll_queue_pop_front(list->first, list->last);

    return result;
}

inline String string_list_find_first_match(StringList *list, StringList *needles)
{
    string_list_foreach(list, node) {
        String str = node->str;
        string_list_foreach(needles, needle_node) {
            String needle = needle_node->str;
            if (string_contains(str, needle)) {
                return str;
            }
        }
    }

    return string_lit("");
}

inline void string_list_pop_matches(StringList *list, String needle)
{
    StringNode *prev = NULL;
    string_list_foreach(list, node) {
        String str = node->str;
        if (string_contains(str, needle)) {
            list->node_count -= 1;
            if (node == list->first) {
                sll_queue_pop_front(list->first, list->last);
            } else {
                prev->next = node->next;
                if (node == list->last) {
                    list->last = prev;
                }
            }
        }

        prev = node;
    }
}

String string_list_join(Arena *arena, StringList *list, String delim)
{
    if (list->node_count == 0) {
        return string_lit("");
    }

    usize total_len = list->total_len + (delim.len * (list->node_count - 1));
    u8 *buf = arena_push(arena, total_len + 1);
    u8 *cur = buf;
    string_list_foreach(list, node) {
        String str = node->str;
        usize len = str.len;
        mem_copy(cur, str.str, str.len);
        if (delim.len > 0 && node != list->last) {
            mem_copy(&cur[len], delim.str, delim.len);
            len += delim.len;
        }

        cur += len;
    }

    return string_create(buf, total_len);
}
