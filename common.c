#define MIN(x, y) ((x) <= (y) ? (x) : (y))
#define MAX(x, y) ((x) >= (y) ? (x) : (y))
#define CLAMP_MAX(x, max) MIN(x, max)
#define CLAMP_MIN(x, min) MAX(x, min)
#define IS_POW2(x) (((x) != 0) && ((x) & ((x)-1)) == 0)
#define ALIGN_DOWN(n, a) ((n) & ~((a) - 1))
#define ALIGN_UP(n, a) ALIGN_DOWN((n) + (a) - 1, (a))
#define ALIGN_DOWN_PTR(p, a) ((void *)ALIGN_DOWN((uintptr_t)(p), (a)))
#define ALIGN_UP_PTR(p, a) ((void *)ALIGN_UP((uintptr_t)(p), (a)))

void *xcalloc(size_t num_elems, size_t elem_size) {
    void *ptr = calloc(num_elems, elem_size);
    if (!ptr) {
        perror("xcalloc failed");
        exit(1);
    }
    return ptr;
}

void *xrealloc(void *ptr, size_t num_bytes) {
    ptr = realloc(ptr, num_bytes);
    if (!ptr) {
        perror("xrealloc failed");
        exit(1);
    }
    return ptr;
}

void *xmalloc(size_t num_bytes) {
    void *ptr = malloc(num_bytes);
    if (!ptr) {
        perror("xmalloc failed");
        exit(1);
    }
    return ptr;
}

char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buf = xmalloc(size + 1);
    if (size != 0) {
        if (fread(buf, size, 1, file) != 1) {
            fclose(file);
            free(buf);
            return NULL;
        }
    }
    fclose(file);
    buf[size] = 0;
    return buf;
}

typedef struct BufHdr_s{
    size_t len;
    size_t cap;
    char buf[];
}BufHdr;

void* buf__grow(const void* buf, size_t new_len, size_t elem_size);
char *buf__printf(char *buf, const char *fmt, ...);

#define BUF(x) x

#define buf__hdr(b) ((BufHdr *)((char *)(b) - offsetof(BufHdr, buf)))

#define buf_len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf_end(b) ((b) + buf_len(b))
#define buf_sizeof(b) ((b) ? buf_len(b)*sizeof(*b) : 0)
#define buf_fits(b,n) (buf_cap(b)-buf_len(b) > (n) ? 1 : 0)

#define buf_free(b) ((b) ? (free(buf__hdr(b)), (b) = NULL) : 0)
#define buf_fit(b, n) ((n) <= buf_cap(b) ? 0 : ((b) = buf__grow((b), (n), sizeof(*(b)))))
#define buf_push(b, ...) (buf_fit((b), 1 + buf_len(b)), (b)[buf__hdr(b)->len++] = (__VA_ARGS__))
#define buf_printf(b, ...) ((b) = buf__printf((b), __VA_ARGS__))
#define buf_clear(b) ((b) ? buf__hdr(b)->len = 0 : 0)


void *buf__grow(const void *buf, size_t new_len, size_t elem_size) {
    assert(buf_cap(buf) <= (SIZE_MAX - 1)/2);
    size_t new_cap = CLAMP_MIN(2*buf_cap(buf), MAX(new_len, 16));
    assert(new_len <= new_cap);
    assert(new_cap <= (SIZE_MAX - offsetof(BufHdr, buf))/elem_size);
    size_t new_size = offsetof(BufHdr, buf) + new_cap*elem_size;
    BufHdr *new_hdr;
    if (buf) {
        new_hdr = xrealloc(buf__hdr(buf), new_size);
    } else {
        new_hdr = xmalloc(new_size);
        new_hdr->len = 0;
    }
    new_hdr->cap = new_cap;
    return new_hdr->buf;
}

char *buf__printf(char *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t cap = buf_cap(buf) - buf_len(buf);
    size_t n = 1 + vsnprintf(buf_end(buf), cap, fmt, args);
    va_end(args);
    if (n > cap) {
        buf_fit(buf, n + buf_len(buf));
        va_start(args, fmt);
        size_t new_cap = buf_cap(buf) - buf_len(buf);
        n = 1 + vsnprintf(buf_end(buf), new_cap, fmt, args);
        assert(n <= new_cap);
        va_end(args);
    }
    buf__hdr(buf)->len += n - 1;
    return buf;
}

// Arena allocator

typedef struct Arena {
    char *ptr;
    char *end;
    char **blocks;
} Arena;

#define EMPTY_ARENA {NULL,NULL,NULL}

#define ARENA_ALIGNMENT 8
#define ARENA_BLOCK_SIZE 1024*1024


void arena_grow(Arena *arena, size_t min_size) {
    size_t size = ALIGN_UP(MAX(ARENA_BLOCK_SIZE, min_size), ARENA_ALIGNMENT);
    arena->ptr = xmalloc(size);
    arena->end = arena->ptr + size;
    buf_push(arena->blocks, arena->ptr);
}

void *arena_alloc(Arena *arena, size_t size) {
    if (size > (size_t)(arena->end - arena->ptr)) {
        arena_grow(arena, size);
        assert(size <= (size_t)(arena->end - arena->ptr));
    }
    void *ptr = arena->ptr;
    arena->ptr = ALIGN_UP_PTR(arena->ptr + size, ARENA_ALIGNMENT);
    assert(arena->ptr <= arena->end);
    assert(ptr == ALIGN_DOWN_PTR(ptr, ARENA_ALIGNMENT));
    return ptr;
}

void* arena_calloc(Arena* arena,size_t count,size_t size){
    void* ptr = arena_alloc(arena,size*count);
    memset(ptr,0,count*size);
    return ptr;
}

void arena_free(Arena *arena) {
    for (char **it = arena->blocks; it != buf_end(arena->blocks); it++) {
        free(*it);
    }
}

char* arena_strdup(Arena* arena,const char* src, size_t length){
    char* dup = arena_alloc(arena,length+1);
    strncpy(dup,src,length);
    dup[length] = '\0';
    return dup;
}



typedef struct MapEntry {
    void *key;
    void *val;
    uint64_t hash;
} MapEntry;

typedef struct Map {
    MapEntry *entries;
    size_t len;
    size_t cap;
} Map;


uint64_t uint64_hash(uint64_t x) {
    x *= 0xff51afd7ed558ccdul;
    x ^= x >> 32;
    return x;
}

uint64_t ptr_hash(void *ptr) {
    return uint64_hash((uintptr_t)ptr);
}

uint64_t _str_hash(const char *str, size_t len) {
    uint64_t fnv_init = 14695981039346656037ull;
    uint64_t fnv_mul = 1099511628211ull;
    uint64_t h = fnv_init;
    for (size_t i = 0; i < len; i++) {
        h ^= str[i];
        h *= fnv_mul;
    }
    return h;
}

#define str_hash(str,len) _str_hash(str,len) | 1

void *map_get_hashed(Map *map, void *key, uint64_t hash) {
    if (map->len == 0) {
        return NULL;
    }
    assert(IS_POW2(map->cap));
    uint32_t i = (uint32_t)(hash & (map->cap - 1));
    assert(map->len < map->cap);
    for (;;) {
        MapEntry *entry = map->entries + i;
        if (entry->key == key) {
            return entry->val;
        } else if (!entry->key) {
            return NULL;
        }
        i++;
        if (i == map->cap) {
            i = 0;
        }
    }
    return NULL;
}

void **map_put_hashed(Map *map, void *key, void *val, uint64_t hash);

void map_grow(Map *map, size_t new_cap) {
    new_cap = MAX(16, new_cap);
    Map new_map = {
            .entries = xcalloc(new_cap, sizeof(MapEntry)),
            .cap = new_cap
    };
    for (size_t i = 0; i < map->cap; i++) {
        MapEntry *entry = map->entries + i;
        if (entry->key) {
            map_put_hashed(&new_map, entry->key, entry->val, entry->hash);
        }
    }
    free(map->entries);
    *map = new_map;
}

void **map_put_hashed(Map *map, void *key, void *val, uint64_t hash) {
    assert(key);
    assert(val);
    if (2*map->len >= map->cap) {
        map_grow(map, 2*map->cap);
    }
    assert(2*map->len < map->cap);
    assert(IS_POW2(map->cap));
    uint32_t i = (uint32_t)(hash & (map->cap - 1));
    for (;;) {
        MapEntry *entry = map->entries + i;
        if (!entry->key) {
            map->len++;
            entry->key = key;
            entry->val = val;
            entry->hash = hash;
            return &entry->val;
        } else if (entry->key == key) {
            entry->val = val;
            return &entry->val;
        }
        i++;
        if (i == map->cap) {
            i = 0;
        }
    }
}

void **map_put(Map *map, void *key, void *val) {
    return map_put_hashed(map, key, val, ptr_hash(key));
}

void *map_get(Map *map, void *key) {
    return map_get_hashed(map, key, ptr_hash(key));
}

typedef struct Intern {
    struct Intern* next;
    int len;
    char str[];
}Intern;

Arena str_arena;
Map interns;

const char *str_intern_range(const char *start, const char *end) {
    size_t len = end - start;
    uint64_t hash = str_hash(start, len);
    Intern *intern = map_get_hashed(&interns, (void *)hash, hash);
    for (Intern *it = intern; it; it = it->next) {
        if (it->len == len && strncmp(it->str, start, len) == 0) {
            return it->str;
        }
    }
    Intern *new_intern = arena_alloc(&str_arena, offsetof(Intern, str) + len + 1);
    new_intern->len = len;
    new_intern->next = intern;
    memcpy(new_intern->str, start, len);
    new_intern->str[len] = 0;
    map_put_hashed(&interns, (void *)hash, new_intern, hash);
    return new_intern->str;
}


const char *str_intern(const char *str) {
    return str_intern_range(str, str + strlen(str));
}
