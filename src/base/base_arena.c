Arena arena_init(usize reserve_factor, usize commit)
{
    usize page_size = platform_get_page_size();
    usize reserved = align_forward_size(reserve_factor * commit, page_size);
    usize committed = align_forward_size(commit, page_size);
    void *memory = platform_mem_reserve(NULL, reserved);
    if (memory == NULL) {
        reserved = 0;
        committed = 0;
    } else {
        platform_mem_commit(memory, committed);
    }

    return (Arena){
        .reserved = reserved,
        .committed = committed,
        .block_size = committed,
        .memory = memory,
    };
}

inline void arena_release(Arena *arena)
{
    platform_mem_release(arena->memory, arena->reserved);
}

void *arena_push(Arena *arena, usize size)
{
    uptr memory = (uptr)arena->memory;
    uptr cur_addr = memory + (uptr)arena->offset;
    uptr base_addr = align_forward(cur_addr, DEFAULT_ALIGN);
    usize base_offset = (usize)(base_addr - memory);
    if (base_addr > memory + (uptr)arena->reserved) {
        // NOTE(cya): scratch behavior
        base_addr = align_forward(memory, DEFAULT_ALIGN);
        base_offset = (usize)(base_addr - memory);
    } else {
        usize committed = arena->committed;
        if (base_offset + size > committed) {
            // TODO(cya): can we really be sure this will stay in-bounds?
            usize commit = align_forward(size, arena->block_size);
            void *commit_addr = (void*)(memory + (uptr)committed);
            platform_mem_commit(commit_addr, commit);
            arena->committed += commit;
        }
    }

    usize new_offset = base_offset + size;
    arena->offset = new_offset;
    return (void*)base_addr;
}

inline void arena_pop(Arena *arena, usize size)
{
    arena->offset -= size;
}

inline void arena_reset(Arena *arena)
{
    arena->offset = 0;
}

inline void arena_log_stats(Arena *arena)
{
    String used = string_from_u64(arena, arena->offset);
    String committed = string_from_u64(arena, arena->committed);
    String reserved = string_from_u64(arena, arena->reserved);
    String usage = string_from_u64(arena, 100 * arena->offset / arena->reserved);

    const char *fmt = "memory usage: {}% [used={},committed={},reserved={}]";
    log_debug(fmt, usage, used, committed, reserved);
}
