#ifndef ALLOCS_COMMON_H
#define ALLOCS_COMMON_H

#define rvalue_ptr(type,value) &((type){value})
#define arr_len(arr) (sizeof(arr)/sizeof(*(arr)))

#define MAX(x, y) ((x) >= (y) ? (x) : (y))
#define minus(i) (~(i-1))

#define xassert(expr,text) (expr ? assert(text) : 0)

#define xto_string(token) #token
#define to_string(token) xto_string(token)

#define for_each_node(type,i,in_list) for(type* i = in_list.head; i != list.tail; i=i->next)

#define Struct(name,...) typedef struct name __VA_ARGS__ name
#define Union(name,...) typedef union name __VA_ARGS__ name
#define Enum(name,...) typedef enum name __VA_ARGS__ name

#endif //ALLOCS_COMMON_H
