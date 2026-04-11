typedef struct {
    usize reserved;
    usize committed;
    usize block_size;
    usize offset;
    void *memory;
} Arena;

#define arena_push_array(a, size, type) arena_push(a, (size) * sizeof(type))

internal Arena arena_init(usize reserve, usize commit);
internal Arena arena_init_from_buffer(void *buffer, usize size);
internal void *arena_push(Arena *arena, usize size);
internal void arena_pop(Arena *arena, usize size);
internal void arena_reset(Arena *arena);
