#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// maybe we don't need 'unused', cap == 0 if unused (ACTUALLY UNUSED MEANS ALLOCED AND UNUSED. NOT JUST 0)
typedef struct
{
    size_t nb;
    bool *unused;
    size_t *caps;
    char **buffers;
} HtmlAllocator;

#define htmldoc(...) \
({ \
    HtmlAllocator ha = {.buffers = calloc(16, sizeof(char*)), .nb = 16, .caps = calloc(16, sizeof(size_t)), .unused = malloc(16 * sizeof(bool))}; \
    memset(ha.unused, 1, 16 * sizeof(bool)); \
    append_strings(&ha, __VA_ARGS__ __VA_OPT__(,) NULL); \
})

#define div(...)     div_tag  (&ha, append_strings(&ha, __VA_ARGS__ __VA_OPT__(,) NULL))
#define h1(...)      h1_tag   (&ha, append_strings(&ha, __VA_ARGS__ __VA_OPT__(,) NULL))
#define head(...)    head_tag (&ha, append_strings(&ha, __VA_ARGS__ __VA_OPT__(,) NULL))
#define body(...)    body_tag (&ha, append_strings(&ha, __VA_ARGS__ __VA_OPT__(,) NULL))
#define p(...)       p_tag    (&ha, append_strings(&ha, __VA_ARGS__ __VA_OPT__(,) NULL))
#define title(...)   title_tag(&ha, append_strings(&ha, __VA_ARGS__ __VA_OPT__(,) NULL))
// #define notag(...)   __VA_ARGS__,

char *buffers_find(char **buffers, size_t len, char *buffer)
{
    for(size_t i = 0 ; i < len ; i++)
    {
        if(buffer == buffers[i])
            return buffer;
    }
    return NULL;
}

char **get_unused(HtmlAllocator *ha)
{
    int64_t first_unused = -1;
    
    for(size_t i = 0 ; i < ha->nb ; i++)
    {
        if(ha->unused[i])
        {
            first_unused = i;
            goto find_unused_and_alloced;
        }
    }
    
    // no unused buffers were found
    return NULL;
    
    find_unused_and_alloced:
    for(size_t i = first_unused ; i < ha->nb ; i++)
    {
        if(ha->unused[i] && ha->caps[i] != 0)
        {
            return &ha->buffers[i];
        }
    }
    
    return &ha->buffers[first_unused];
}

char *append_strings(HtmlAllocator *ha, ...)
{
    char **unused_str = get_unused(ha);
    
    if(unused_str == NULL)
    {
        ha->buffers = realloc(ha->buffers, ha->nb * 2 * sizeof(char*));
        memset(ha->buffers + ha->nb, 0, ha->nb * sizeof(char*));
        
        ha->caps = realloc(ha->caps, ha->nb * 2 * sizeof(size_t));
        memset(ha->caps + ha->nb, 0, ha->nb * sizeof(size_t));
        
        ha->unused = realloc(ha->unused, ha->nb * 2 * sizeof(bool));
        memset(ha->unused + ha->nb, 1, ha->nb * sizeof(bool));
        
        ha->buffers[ ha->nb ] = calloc(128, sizeof(char));
        ha->caps[ ha->nb ] = 128;
        ha->unused[ ha->nb ] = false;
        
        unused_str = &ha->buffers[ ha->nb ];
        
        ha->nb *= 2;
    }
    
    size_t *cap = &ha->caps[ unused_str - ha->buffers ];
    size_t size = 0;
    **unused_str = '\0';
    
    va_list args;
    va_start(args, ha);
    
    for(char *next_str = va_arg(args, char*); next_str != NULL ; next_str = va_arg(args, char*))
    {
        size_t next_len = strlen(next_str);
        if(size + next_len >= *cap)
        {
            *unused_str = realloc(*unused_str, (next_len + *cap) * 2);
            *cap = (next_len + *cap) * 2;
        }
        memcpy(*unused_str + size, next_str, next_len);
        
        if(buffers_find(ha->buffers, ha->nb, next_str) != NULL)
        {
            ha->unused[ &next_str - ha->buffers ] = true;
        }
        
        size = size + next_len;
        (*unused_str)[size] = '\0';
    }
    
    va_end(args);
    
    return *unused_str;
}

char *h1_tag(HtmlAllocator *ha, const char *between)
{
    const size_t between_len = strlen(between);
    const size_t len_to_add = sizeof("<h1>") + sizeof("</h1>") - 2 + between_len;
    
    char **unused_str = get_unused(ha);
    if(unused_str == NULL)
    {
        ha->buffers = realloc(ha->buffers, ha->nb * 2 * sizeof(char*));
        memset(ha->buffers + ha->nb, 0, ha->nb * sizeof(char*));
        
        ha->caps = realloc(ha->caps, ha->nb * 2 * sizeof(size_t));
        memset(ha->caps + ha->nb, 0, ha->nb * sizeof(size_t));
        
        ha->unused = realloc(ha->unused, ha->nb * 2 * sizeof(bool));
        memset(ha->unused + ha->nb, 1, ha->nb * sizeof(bool));
        
        ha->buffers[ ha->nb ] = calloc(128, sizeof(char));
        ha->caps[ ha->nb ] = 128;
        ha->unused[ ha->nb ] = false;
        
        unused_str = &ha->buffers[ ha->nb ];
        
        ha->nb *= 2;
    }
    
    size_t *cap = &ha->caps[ unused_str - ha->buffers ];
    size_t size = 0;
    **unused_str = '\0';
        
    if(len_to_add >= *cap)
    {
        *unused_str = realloc(*unused_str, len_to_add + 1);
        *cap = len_to_add + 1;
    }
    
    memcpy(*unused_str, "<h1>", sizeof("<h1>") - 1);
    memcpy(*unused_str + sizeof("<h1>") - 1, between, between_len);
    memcpy(*unused_str + sizeof("<h1>") - 1 + between_len, "</h1>", sizeof("</h1>") - 1);
    
    return *unused_str;
}

char *p_tag(HtmlAllocator *ha, char *between)
{
    const size_t between_len = strlen(between);
    const size_t len_to_add = sizeof("<p>") + sizeof("</p>") - 2 + between_len;
    
    char **unused_str = get_unused(ha);
    if(unused_str == NULL)
    {
        ha->buffers = realloc(ha->buffers, ha->nb * 2 * sizeof(char*));
        memset(ha->buffers + ha->nb, 0, ha->nb * sizeof(char*));
        
        ha->caps = realloc(ha->caps, ha->nb * 2 * sizeof(size_t));
        memset(ha->caps + ha->nb, 0, ha->nb * sizeof(size_t));
        
        ha->unused = realloc(ha->unused, ha->nb * 2 * sizeof(bool));
        memset(ha->unused + ha->nb, 1, ha->nb * sizeof(bool));
        
        ha->buffers[ ha->nb ] = calloc(128, sizeof(char));
        ha->caps[ ha->nb ] = 128;
        ha->unused[ ha->nb ] = false;
        
        unused_str = &ha->buffers[ ha->nb ];
        
        ha->nb *= 2;
    }
    
    size_t *cap = &ha->caps[ unused_str - ha->buffers ];
    size_t size = 0;
    **unused_str = '\0';
        
    if(len_to_add >= *cap)
    {
        *unused_str = realloc(*unused_str, len_to_add + 1);
        *cap = len_to_add + 1;
    }
    
    memcpy(*unused_str, "<p>", sizeof("<p>") - 1);
    memcpy(*unused_str + sizeof("<p>") - 1, between, between_len);
    memcpy(*unused_str + sizeof("<p>") - 1 + between_len, "</p>", sizeof("</p>") - 1);
    
    return *unused_str;
}

char *body_tag(HtmlAllocator *ha, char *between)
{
    const size_t between_len = strlen(between);
    const size_t len_to_add = sizeof("<body>") + sizeof("</body>") - 2 + between_len;
    
    char **unused_str = get_unused(ha);
    if(unused_str == NULL)
    {
        ha->buffers = realloc(ha->buffers, ha->nb * 2 * sizeof(char*));
        memset(ha->buffers + ha->nb, 0, ha->nb * sizeof(char*));
        
        ha->caps = realloc(ha->caps, ha->nb * 2 * sizeof(size_t));
        memset(ha->caps + ha->nb, 0, ha->nb * sizeof(size_t));
        
        ha->unused = realloc(ha->unused, ha->nb * 2 * sizeof(bool));
        memset(ha->unused + ha->nb, 1, ha->nb * sizeof(bool));
        
        ha->buffers[ ha->nb ] = calloc(128, sizeof(char));
        ha->caps[ ha->nb ] = 128;
        ha->unused[ ha->nb ] = false;
        
        unused_str = &ha->buffers[ ha->nb ];
        
        ha->nb *= 2;
    }
    
    size_t *cap = &ha->caps[ unused_str - ha->buffers ];
    size_t size = 0;
    **unused_str = '\0';
        
    if(len_to_add >= *cap)
    {
        *unused_str = realloc(*unused_str, len_to_add + 1);
        *cap = len_to_add + 1;
    }
    
    memcpy(*unused_str, "<body>", sizeof("<body>") - 1);
    memcpy(*unused_str + sizeof("<body>") - 1, between, between_len);
    memcpy(*unused_str + sizeof("<body>") - 1 + between_len, "</body>", sizeof("</body>") - 1);
    
    return *unused_str;
}

char *head_tag(HtmlAllocator *ha, char *between)
{
    const size_t between_len = strlen(between);
    const size_t len_to_add = sizeof("<head>") + sizeof("</head>") - 2 + between_len;
    
    char **unused_str = get_unused(ha);
    if(unused_str == NULL)
    {
        ha->buffers = realloc(ha->buffers, ha->nb * 2 * sizeof(char*));
        memset(ha->buffers + ha->nb, 0, ha->nb * sizeof(char*));
        
        ha->caps = realloc(ha->caps, ha->nb * 2 * sizeof(size_t));
        memset(ha->caps + ha->nb, 0, ha->nb * sizeof(size_t));
        
        ha->unused = realloc(ha->unused, ha->nb * 2 * sizeof(bool));
        memset(ha->unused + ha->nb, 1, ha->nb * sizeof(bool));
        
        ha->buffers[ ha->nb ] = calloc(128, sizeof(char));
        ha->caps[ ha->nb ] = 128;
        ha->unused[ ha->nb ] = false;
        
        unused_str = &ha->buffers[ ha->nb ];
        
        ha->nb *= 2;
    }
    
    size_t *cap = &ha->caps[ unused_str - ha->buffers ];
    size_t size = 0;
    **unused_str = '\0';
        
    if(len_to_add >= *cap)
    {
        *unused_str = realloc(*unused_str, len_to_add + 1);
        *cap = len_to_add + 1;
    }
    
    memcpy(*unused_str, "<head>", sizeof("<head>") - 1);
    memcpy(*unused_str + sizeof("<head>") - 1, between, between_len);
    memcpy(*unused_str + sizeof("<head>") - 1 + between_len, "</head>", sizeof("</head>") - 1);
    
    return *unused_str;
}

char *title_tag(HtmlAllocator *ha, char *between)
{
    const size_t between_len = strlen(between);
    const size_t len_to_add = sizeof("<title>") + sizeof("</title>") - 2 + between_len;
    
    char **unused_str = get_unused(ha);
    if(unused_str == NULL)
    {
        ha->buffers = realloc(ha->buffers, ha->nb * 2 * sizeof(char*));
        memset(ha->buffers + ha->nb, 0, ha->nb * sizeof(char*));
        
        ha->caps = realloc(ha->caps, ha->nb * 2 * sizeof(size_t));
        memset(ha->caps + ha->nb, 0, ha->nb * sizeof(size_t));
        
        ha->unused = realloc(ha->unused, ha->nb * 2 * sizeof(bool));
        memset(ha->unused + ha->nb, 1, ha->nb * sizeof(bool));
        
        ha->buffers[ ha->nb ] = calloc(128, sizeof(char));
        ha->caps[ ha->nb ] = 128;
        ha->unused[ ha->nb ] = false;
        
        unused_str = &ha->buffers[ ha->nb ];
        
        ha->nb *= 2;
    }
    
    size_t *cap = &ha->caps[ unused_str - ha->buffers ];
    size_t size = 0;
    **unused_str = '\0';
        
    if(len_to_add >= *cap)
    {
        *unused_str = realloc(*unused_str, len_to_add + 1);
        *cap = len_to_add + 1;
    }
    
    memcpy(*unused_str, "<title>", sizeof("<title>") - 1);
    memcpy(*unused_str + sizeof("<title>") - 1, between, between_len);
    memcpy(*unused_str + sizeof("<title>") - 1 + between_len, "</title>", sizeof("</title>") - 1);
    
    return *unused_str;
}

char *div_tag(HtmlAllocator *ha, char *between)
{
    const size_t between_len = strlen(between);
    const size_t len_to_add = sizeof("<div>") + sizeof("</div>") - 2 + between_len;
    
    char **unused_str = get_unused(ha);
    if(unused_str == NULL)
    {
        ha->buffers = realloc(ha->buffers, ha->nb * 2 * sizeof(char*));
        memset(ha->buffers + ha->nb, 0, ha->nb * sizeof(char*));
        
        ha->caps = realloc(ha->caps, ha->nb * 2 * sizeof(size_t));
        memset(ha->caps + ha->nb, 0, ha->nb * sizeof(size_t));
        
        ha->unused = realloc(ha->unused, ha->nb * 2 * sizeof(bool));
        memset(ha->unused + ha->nb, 1, ha->nb * sizeof(bool));
        
        ha->buffers[ ha->nb ] = calloc(128, sizeof(char));
        ha->caps[ ha->nb ] = 128;
        ha->unused[ ha->nb ] = false;
        
        unused_str = &ha->buffers[ ha->nb ];
        
        ha->nb *= 2;
    }
    
    size_t *cap = &ha->caps[ unused_str - ha->buffers ];
    size_t size = 0;
    **unused_str = '\0';
        
    if(len_to_add >= *cap)
    {
        *unused_str = realloc(*unused_str, len_to_add + 1);
        *cap = len_to_add + 1;
    }
    
    memcpy(*unused_str, "<div>", sizeof("<div>") - 1);
    memcpy(*unused_str + sizeof("<div>") - 1, between, between_len);
    memcpy(*unused_str + sizeof("<div>") - 1 + between_len, "</div>", sizeof("</div>") - 1);
    
    return *unused_str;
}

int main()
{
    char *rt_str = malloc(128);
    gets(rt_str);
    
    char *doc = htmldoc(
        body(
            h1("H1 TAG WOW"),
            h1(rt_str)
        )
    );
    
    puts(doc);
    
    free(doc);
    return 0;
}
