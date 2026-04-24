#include <stdarg.h> // va_args
#include <stdint.h> // sized integers

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uintptr_t uptr;
typedef intptr_t iptr;

typedef u64 usize;
typedef i64 isize;

typedef i32 b32;

#define internal static
#define global static
#define local_persist static

#define true (0 == 0)
#define false (0 != 0)

#define DEFAULT_ALIGN (uptr)(2 * sizeof(void*))

#if defined(COMPILER_MSVC) || (defined(COMPILER_CLANG) && defined(PLATFORM_WINDOWS))
#    pragma section(".rdata$", read)
#    define readonly __declspec(allocate(".rdata$"))
#    define force_keep __declspec(selectany)
#elif defined(COMPILER_CLANG)
#    define readonly __attribute__((section(".rodata")))
#    define force_keep __attribute__((used))
#else
#    define readonly
#    define force_keep
#endif

#if defined(COMPILER_MSVC)
#    define thread_local __declspec(thread)
#else
#    define thread_local __thread
#endif

#define noop() ((void)0)
#define unused(x) (void)sizeof(x)
#define stringify(x) #x
#define min(a, b) ((a) > (b) ? (b) : (a))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define array_len(a) ((usize)sizeof(a) / sizeof(a[0]))
#define mem_equal(d, s, len) platform_mem_equal(d, s, len)
#define mem_copy(d, s, len) platform_mem_copy(d, s, len)
#define bit_flag(n) (1 << (n))
#define is_power_of_two(v) (((v) & ((v) - 1)) == 0)

#define kibibytes(n) (1024 * (n))
#define mebibytes(n) (1024 * kibibytes(n))
#define gibibytes(n) (1024 * mebibytes(n))

#define check_null(v) ((v) == NULL)
#define set_null(v) ((v) = NULL)

// NOTE(cya): singly-linked, doubly-headed lists -> queues
#define sll_queue_push_back(f, l, n) ( \
    check_null(f) ? \
        ((f) = (l) = (n), set_null((n)->next)) : \
        ((l)->next = (n), (l) = (n), set_null((n)->next)) \
)
#define sll_queue_push_front(f, l, n) ( \
    check_null(f) ? \
        ((f) = (l) = (n), set_null((n)->next)) : \
        ((n)->next = (f), (f) = (n)) \
)
#define sll_queue_pop_front(f, l) ( \
    ((f) == (l)) ? \
        (set_null(f), set_null(l)) : \
        ((f) = (f)->next) \
)

internal usize align_forward_size(usize value, usize align);
internal uptr align_forward(uptr value, uptr align);

internal b32 char_is_whitespace(char c);
internal b32 char_is_digit(char c);

internal usize cstring_len(const char *str);
internal usize wcstring_len(const u16 *str);
