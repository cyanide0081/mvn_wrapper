b32 is_power_of_two(uptr value)
{
    return (value & (value - 1)) == 0;
}

inline usize align_forward_size(usize value, usize align)
{
    return align_forward((uptr)value, (uptr)align);
}

uptr align_forward(uptr value, uptr align)
{
    assert(is_power_of_two(align));

    uptr modulo = value & (align -     1);
    return modulo == 0 ? value : value + align - modulo;
}

// NOTE(cya): ASCII only
inline b32 char_is_whitespace(char c)
{
    switch (c) {
    case ' ':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
        return true;
    default:
        return false;
    }
}

inline b32 char_is_digit(char c)
{
    return (u8)(c - '0') < 10;
}

inline usize cstring_len(const char *str)
{
    usize len = 0;
    while (*str++ != '\0') {
        len += 1;
    }

    return len;
}

inline usize wcstring_len(const u16 *str)
{
    usize len = 0;
    while (*str++ != '\0') {
        len += 1;
    }

    return len;
}
