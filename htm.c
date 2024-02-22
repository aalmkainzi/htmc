#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    size_t nb;
    bool *unused;
    size_t *caps;
    char **buffers;
} HtmcAllocations;

void htmc_cleanup_unused_buffers(HtmcAllocations *ha, char *ret_ptr)
{
    char **current;
    
    for(current = &ha->buffers[0] ; *current != ret_ptr ; current++)
    {
        free(*current);
    }
    for(current++ ; current != &ha->buffers[ha->nb] ; current++)
    {
        free(*current);
    }
    
    free(ha->caps);
    free(ha->unused);
    free(ha->buffers);
    free(ha);
}

#define htmldoc(...) \
({ \
    HtmcAllocations *ha = malloc(sizeof(HtmcAllocations)); \
    const size_t init_cap = 4; \
    *ha = (HtmcAllocations){ \
        .buffers = calloc(16, sizeof(char*)), \
        .nb = init_cap, \
        .caps = calloc(init_cap, sizeof(size_t)), \
        .unused = malloc(init_cap * sizeof(bool))\
    }; \
    memset(ha->unused, 1, init_cap * sizeof(bool)); \
    char *ret = append_strings(ha, ##__VA_ARGS__, NULL); \
    htmc_cleanup_unused_buffers(ha, ret); \
    ret; \
})

#define div(...)     div_tag  (ha, append_strings(ha, ##__VA_ARGS__, NULL))
#define h1(...)      h1_tag   (ha, append_strings(ha, ##__VA_ARGS__, NULL))
#define head(...)    head_tag (ha, append_strings(ha, ##__VA_ARGS__, NULL))
#define body(...)    body_tag (ha, append_strings(ha, ##__VA_ARGS__, NULL))
#define p(...)       p_tag    (ha, append_strings(ha, ##__VA_ARGS__, NULL))
#define title(...)   title_tag(ha, append_strings(ha, ##__VA_ARGS__, NULL))

char **buffers_find(HtmcAllocations *ha, const char *buffer)
{
    for(size_t i = 0 ; i < ha->nb ; i++)
    {
        if(buffer == ha->buffers[i])
            return &ha->buffers[i];
    }
    return NULL;
}

void grow_buffers(HtmcAllocations *ha)
{
    ha->buffers = realloc(ha->buffers, ha->nb * 2 * sizeof(char*));
    memset(ha->buffers + ha->nb, 0, ha->nb * sizeof(char*));

    ha->caps = realloc(ha->caps, ha->nb * 2 * sizeof(size_t));
    memset(ha->caps + ha->nb, 0, ha->nb * sizeof(size_t));

    ha->unused = realloc(ha->unused, ha->nb * 2 * sizeof(bool));
    memset(ha->unused + ha->nb, 1, ha->nb * sizeof(bool));
    
    ha->nb *= 2;
}

void set_unused(HtmcAllocations *ha, const char *str)
{
    char **found = buffers_find(ha, str);
    ha->unused[ found - ha->buffers ] = true;
}

void set_unused_if_alloced(HtmcAllocations *ha, const char *str)
{
    char **found = buffers_find(ha, str);
    if(found != NULL)
    {
        ha->unused[ found - ha->buffers ] = true;
    }
}

char **get_unused(HtmcAllocations *ha)
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

char **get_or_alloc_unused(HtmcAllocations *ha, size_t bytes)
{
    char **unused_str = get_unused(ha);
    if(unused_str == NULL)
    {
        size_t old_nb = ha->nb;
        grow_buffers(ha);
        
        unused_str = &ha->buffers[ old_nb ];
        *unused_str = calloc(bytes, sizeof(char));
        ha->caps[ old_nb ] = bytes;
    }
    else if(bytes > ha->caps[ unused_str - ha->buffers ])
    {
        *unused_str = realloc(*unused_str, bytes);
        ha->caps[ unused_str - ha->buffers ] = bytes;
    }
    
    ha->unused[ unused_str - ha->buffers ] = false;
    
    return unused_str;
}

char *append_strings(HtmcAllocations *ha, ...)
{
    char **unused_str = get_or_alloc_unused(ha, 16);
    
    size_t *cap = &ha->caps[ unused_str - ha->buffers ];
    size_t size = 0;
    
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
        
        set_unused_if_alloced(ha, next_str);
        
        size = size + next_len;
        (*unused_str)[size] = '\0';
    }
    
    va_end(args);
    
    return *unused_str;
}

char *h1_tag(HtmcAllocations *ha, const char *between)
{
    const size_t between_len = strlen(between);
    const size_t len_to_add = sizeof("<h1>") + sizeof("</h1>") - 2 + between_len; // not including '\0'
    
    char **unused_str = get_or_alloc_unused(ha, len_to_add + 1);
    
    memcpy(*unused_str, "<h1>", sizeof("<h1>") - 1);
    memcpy(*unused_str + sizeof("<h1>") - 1, between, between_len);
    memcpy(*unused_str + sizeof("<h1>") - 1 + between_len, "</h1>", sizeof("</h1>") - 1);
    (*unused_str)[len_to_add] = '\0';
    
    char **between_ptr = buffers_find(ha, between);
    ha->unused[ between_ptr - ha->buffers ] = true;
    
    return *unused_str;
}

char *p_tag(HtmcAllocations *ha, char *between)
{
    const size_t between_len = strlen(between);
    const size_t len_to_add = sizeof("<p>") + sizeof("</p>") - 2 + between_len; // not including '\0'
    
    char **unused_str = get_or_alloc_unused(ha, len_to_add + 1);
    
    memcpy(*unused_str, "<p>", sizeof("<p>") - 1);
    memcpy(*unused_str + sizeof("<p>") - 1, between, between_len);
    memcpy(*unused_str + sizeof("<p>") - 1 + between_len, "</p>", sizeof("</p>") - 1);
    (*unused_str)[len_to_add] = '\0';
    
    char **between_ptr = buffers_find(ha, between);
    ha->unused[ between_ptr - ha->buffers ] = true;
    
    return *unused_str;
}

char *body_tag(HtmcAllocations *ha, char *between)
{
    const size_t between_len = strlen(between);
    const size_t len_to_add = sizeof("<body>") + sizeof("</body>") - 2 + between_len; // not including '\0'
    
    char **unused_str = get_or_alloc_unused(ha, len_to_add + 1);
    
    memcpy(*unused_str, "<body>", sizeof("<body>") - 1);
    memcpy(*unused_str + sizeof("<body>") - 1, between, between_len);
    memcpy(*unused_str + sizeof("<body>") - 1 + between_len, "</body>", sizeof("</body>") - 1);
    (*unused_str)[len_to_add] = '\0';
    
    char **between_ptr = buffers_find(ha, between);
    ha->unused[ between_ptr - ha->buffers ] = true;
    
    return *unused_str;
}

char *head_tag(HtmcAllocations *ha, char *between)
{
    const size_t between_len = strlen(between);
    const size_t len_to_add = sizeof("<head>") + sizeof("</head>") - 2 + between_len; // not including '\0'
    
    char **unused_str = get_or_alloc_unused(ha, len_to_add + 1);
    
    memcpy(*unused_str, "<head>", sizeof("<head>") - 1);
    memcpy(*unused_str + sizeof("<head>") - 1, between, between_len);
    memcpy(*unused_str + sizeof("<head>") - 1 + between_len, "</head>", sizeof("</head>") - 1);
    (*unused_str)[len_to_add] = '\0';
    
    char **between_ptr = buffers_find(ha, between);
    ha->unused[ between_ptr - ha->buffers ] = true;
    
    return *unused_str;
}

char *title_tag(HtmcAllocations *ha, char *between)
{
    const size_t between_len = strlen(between);
    const size_t len_to_add = sizeof("<title>") + sizeof("</title>") - 2 + between_len; // not including '\0'
    
    char **unused_str = get_or_alloc_unused(ha, len_to_add + 1);
    
    memcpy(*unused_str, "<title>", sizeof("<title>") - 1);
    memcpy(*unused_str + sizeof("<title>") - 1, between, between_len);
    memcpy(*unused_str + sizeof("<title>") - 1 + between_len, "</title>", sizeof("</title>") - 1);
    (*unused_str)[len_to_add] = '\0';
    
    char **between_ptr = buffers_find(ha, between);
    ha->unused[ between_ptr - ha->buffers ] = true;
    
    return *unused_str;
}

char *div_tag(HtmcAllocations *ha, char *between)
{
    const size_t between_len = strlen(between);
    const size_t len_to_add = sizeof("<div>") + sizeof("</div>") - 2 + between_len; // not including '\0'
    
    char **unused_str = get_or_alloc_unused(ha, len_to_add + 1);
    
    memcpy(*unused_str, "<div>", sizeof("<div>") - 1);
    memcpy(*unused_str + sizeof("<div>") - 1, between, between_len);
    memcpy(*unused_str + sizeof("<div>") - 1 + between_len, "</div>", sizeof("</div>") - 1);
    (*unused_str)[len_to_add] = '\0';
    
    char **between_ptr = buffers_find(ha, between);
    ha->unused[ between_ptr - ha->buffers ] = true;
    
    return *unused_str;
}

int main()
{
    char *rt_str = malloc(128);
    fgets(rt_str, 128, stdin);
    
    char *doc = htmldoc(
        head(
            title("AMAZING PAGE FROM C")
        ),
        body(
            h1("big"),
            p("small"),
            p("small"),
            p("small"),
            div(
                h1("INSIDE DIV"),
                p("inside div")
            ),
            h1("MESSAGE: ", rt_str)
        )
    );
    
    puts(doc);
    
    free(doc);
    free(rt_str);
    return 0;
}
