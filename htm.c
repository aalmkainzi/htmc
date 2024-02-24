#include <stdarg.h>
#include "htmc.h"

const char *htmc_tags[] = {
"a", "abbr", "address", "area", "article", "aside", "audio", "b", "base", "bdi", "bdo", "blockquote", "body", "br", "button", "canvas", "caption", "cite", "code", "col", "colgroup", "data", "datalist", "dd", "del", "details", "dfn", "dialog", "div", "dl", "dt", "em", "embed", "fieldset", "figcaption", "figure", "footer", "form", "h1", "h2", "h3", "h4", "h5", "h6", "head", "header", "hgroup", "hr", "html", "i", "iframe", "img", "input", "ins", "kbd", "label", "legend", "li", "link", "main", "map", "mark", "math", "menu", "meta", "meter", "nav", "noscript", "object", "ol", "optgroup", "option", "output", "p", "param", "picture", "pre", "progress", "q", "rp", "rt", "ruby", "s", "samp", "script", "section", "select", "slot", "small", "source", "span", "strong", "style", "sub", "summary", "sup", "svg", "table", "tbody", "td", "template", "textarea", "tfoot", "th", "thead", "time", "title", "tr", "track", "u", "ul", "var", "video", "wbr"
};

const uint32_t htmc_tag_lengths[] = {
sizeof("a") - 1, sizeof("abbr") - 1, sizeof("address") - 1, sizeof("area") - 1, sizeof("article") - 1, sizeof("aside") - 1, sizeof("audio") - 1, sizeof("b") - 1, sizeof("base") - 1, sizeof("bdi") - 1, sizeof("bdo") - 1, sizeof("blockquote") - 1, sizeof("body") - 1, sizeof("br") - 1, sizeof("button") - 1, sizeof("canvas") - 1, sizeof("caption") - 1, sizeof("cite") - 1, sizeof("code") - 1, sizeof("col") - 1, sizeof("colgroup") - 1, sizeof("data") - 1, sizeof("datalist") - 1, sizeof("dd") - 1, sizeof("del") - 1, sizeof("details") - 1, sizeof("dfn") - 1, sizeof("dialog") - 1, sizeof("div") - 1, sizeof("dl") - 1, sizeof("dt") - 1, sizeof("em") - 1, sizeof("embed") - 1, sizeof("fieldset") - 1, sizeof("figcaption") - 1, sizeof("figure") - 1, sizeof("footer") - 1, sizeof("form") - 1, sizeof("h1") - 1, sizeof("h2") - 1, sizeof("h3") - 1, sizeof("h4") - 1, sizeof("h5") - 1, sizeof("h6") - 1, sizeof("head") - 1, sizeof("header") - 1, sizeof("hgroup") - 1, sizeof("hr") - 1, sizeof("html") - 1, sizeof("i") - 1, sizeof("iframe") - 1, sizeof("img") - 1, sizeof("input") - 1, sizeof("ins") - 1, sizeof("kbd") - 1, sizeof("label") - 1, sizeof("legend") - 1, sizeof("li") - 1, sizeof("link") - 1, sizeof("main") - 1, sizeof("map") - 1, sizeof("mark") - 1, sizeof("math") - 1, sizeof("menu") - 1, sizeof("meta") - 1, sizeof("meter") - 1, sizeof("nav") - 1, sizeof("noscript") - 1, sizeof("object") - 1, sizeof("ol") - 1, sizeof("optgroup") - 1, sizeof("option") - 1, sizeof("output") - 1, sizeof("p") - 1, sizeof("param") - 1, sizeof("picture") - 1, sizeof("pre") - 1, sizeof("progress") - 1, sizeof("q") - 1, sizeof("rp") - 1, sizeof("rt") - 1, sizeof("ruby") - 1, sizeof("s") - 1, sizeof("samp") - 1, sizeof("script") - 1, sizeof("section") - 1, sizeof("select") - 1, sizeof("slot") - 1, sizeof("small") - 1, sizeof("source") - 1, sizeof("span") - 1, sizeof("strong") - 1, sizeof("style") - 1, sizeof("sub") - 1, sizeof("summary") - 1, sizeof("sup") - 1, sizeof("svg") - 1, sizeof("table") - 1, sizeof("tbody") - 1, sizeof("td") - 1, sizeof("template") - 1, sizeof("textarea") - 1, sizeof("tfoot") - 1, sizeof("th") - 1, sizeof("thead") - 1, sizeof("time") - 1, sizeof("title") - 1, sizeof("tr") - 1, sizeof("track") - 1, sizeof("u") - 1, sizeof("ul") - 1, sizeof("var") - 1, sizeof("video") - 1, sizeof("wbr") - 1
};

const char *const htmc_doctypehtml = "<!DOCTYPE html>";

void htmc_cleanup_unused_buffers(HtmcAllocations *ha, const char *ret_ptr)
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
    free(ha->sizes);
    free(ha->unused);
    free(ha->buffers);
    free(ha);
}

char **htmc_find_buffer(const HtmcAllocations *ha, const char *buffer)
{
    for(size_t i = 0 ; i < ha->nb ; i++)
    {
        if(buffer == ha->buffers[i])
            return &ha->buffers[i];
    }
    return NULL;
}

void htmc_grow_buffers(HtmcAllocations *ha)
{
    ha->buffers = realloc(ha->buffers, ha->nb * 2 * sizeof(char*));
    memset(ha->buffers + ha->nb, 0, ha->nb * sizeof(char*));
    
    ha->caps = realloc(ha->caps, ha->nb * 2 * sizeof(size_t));
    memset(ha->caps + ha->nb, 0, ha->nb * sizeof(size_t));
    
    ha->sizes = realloc(ha->sizes, ha->nb * 2 * sizeof(size_t));
    memset(ha->sizes + ha->nb, 0, ha->nb * sizeof(size_t));
    
    ha->unused = realloc(ha->unused, ha->nb * 2 * sizeof(bool));
    memset(ha->unused + ha->nb, 1, ha->nb * sizeof(bool));
    
    ha->nb *= 2;
}

void htmc_set_unused(HtmcAllocations *ha, const char *str)
{
    char **found = htmc_find_buffer(ha, str);
    ha->unused[ found - ha->buffers ] = true;
}

void htmc_set_unused_if_alloced(HtmcAllocations *ha, const char *str)
{
    char **found = htmc_find_buffer(ha, str);
    if(found != NULL)
    {
        ha->unused[ found - ha->buffers ] = true;
    }
}

char **htmc_find_unused(const HtmcAllocations *ha)
{
    size_t first_unused;
    
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

char **htmc_get_unused(HtmcAllocations *ha, size_t with_size)
{
    char **unused_str = htmc_find_unused(ha);
    if(unused_str == NULL)
    {
        size_t old_nb = ha->nb;
        htmc_grow_buffers(ha);
        
        unused_str = &ha->buffers[ old_nb ];
        *unused_str = calloc(with_size, sizeof(char));
        ha->caps[ old_nb ] = with_size;
    }
    else if(with_size > ha->caps[ unused_str - ha->buffers ])
    {
        *unused_str = realloc(*unused_str, with_size);
        ha->caps[ unused_str - ha->buffers ] = with_size;
    }
    
    ha->unused[ unused_str - ha->buffers ] = false;
    
    return unused_str;
}

char *htmc_concat_strings(HtmcAllocations *ha, ...)
{
    char **unused_str = htmc_get_unused(ha, 16);
    
    size_t *cap = &ha->caps[ unused_str - ha->buffers ];
    size_t size = 0;
    
    va_list args;
    va_start(args, ha);
    
    char *next_str = va_arg(args, char*);
    
    // if no strings were provided, return an empty string
    if(next_str == NULL)
    {
        if(*cap == 0)
        {
            *unused_str = calloc(1, sizeof(char));
        }
        **unused_str = '\0';
        return *unused_str;
    }
    
    for( ; next_str != NULL ; next_str = va_arg(args, char*))
    {
        size_t next_len = strlen(next_str);
        if(size + next_len >= *cap)
        {
            *unused_str = realloc(*unused_str, (next_len + *cap) * 2);
            *cap = (next_len + *cap) * 2;
        }
        memcpy(*unused_str + size, next_str, next_len);
        
        htmc_set_unused_if_alloced(ha, next_str);
        
        size = size + next_len;
        (*unused_str)[size] = '\0';
    }
    
    va_end(args);
    
    ha->sizes[ unused_str - ha->buffers ] = size;
    return *unused_str;
}

static char *htmc_vconcat_strings(HtmcAllocations *ha, va_list args)
{
    char **unused_str = htmc_get_unused(ha, 16);
    
    size_t *cap = &ha->caps[ unused_str - ha->buffers ];
    size_t size = 0;
    
    char *next_str = va_arg(args, char*);
    
    // if no strings were provided, return an empty string
    if(next_str == NULL)
    {
        if(*cap == 0)
        {
            *unused_str = calloc(1, sizeof(char));
        }
        **unused_str = '\0';
        return *unused_str;
    }
    
    for( ; next_str != NULL ; next_str = va_arg(args, char*))
    {
        size_t next_len = strlen(next_str);
        if(size + next_len >= *cap)
        {
            *unused_str = realloc(*unused_str, (next_len + *cap) * 2);
            *cap = (next_len + *cap) * 2;
        }
        memcpy(*unused_str + size, next_str, next_len);
        
        htmc_set_unused_if_alloced(ha, next_str);
        
        size = size + next_len;
        (*unused_str)[size] = '\0';
    }
    
    ha->sizes[ unused_str - ha->buffers ] = size;
    
    return *unused_str;
}

char *htmc_surround_by_tag(HtmcAllocations *ha, uint16_t tag_id, char *between)
{
    const size_t between_len = strlen(between);
    const size_t tag_len = htmc_tag_lengths[tag_id];
    const size_t needed_cap = 1 + tag_len + 1 + between_len + 1 + 1 + tag_len + 1 + 1;
    const char *tag = htmc_tags[tag_id];
    
    char **unused_str = htmc_get_unused(ha, needed_cap);
    
    memcpy(*unused_str, "<", 1);
    memcpy(*unused_str + 1, tag, tag_len);
    memcpy(*unused_str + 1 + tag_len, ">", 1);
    memcpy(*unused_str + 1 + tag_len + 1, between, between_len);
    memcpy(*unused_str + 1 + tag_len + 1 + between_len, "</", 2);
    memcpy(*unused_str + 1 + tag_len + 1 + between_len + 1 + 1, tag, tag_len);
    memcpy(*unused_str + 1 + tag_len + 1 + between_len + 1 + 1 + tag_len, ">", 1);
    (*unused_str)[needed_cap - 1] = '\0';
    
    ha->sizes[ unused_str - ha->buffers ] = needed_cap - 1;
    
    htmc_set_unused(ha, between);
    
    return *unused_str;
}

char *htmc_surround_by_tag_with_attrs(HtmcAllocations *ha, uint16_t tag_id, char *attrs[], size_t nb_attrs, char *between)
{
    const size_t between_len = strlen(between);
    const size_t tag_len = htmc_tag_lengths[tag_id];
    const size_t needed_cap = 1 + tag_len + 1 + between_len + 1 + 1 + tag_len + 1 + 1;
    const char *tag = htmc_tags[tag_id];
    
    char **unused_str = htmc_get_unused(ha, needed_cap);
    size_t *cap = &ha->caps[ unused_str - ha->buffers ];
    size_t size = 0;
    
    memcpy(*unused_str, "<", 1);
    size += 1;
    
    memcpy(*unused_str + size, tag, tag_len);
    size += tag_len;
    
    // insert attributes here:
    for(size_t i = 0 ; i < nb_attrs; i++)
    {
        size_t attr_len = strlen(attrs[i]);
        if(*cap <= size + attr_len + 1) // 1 for the spaces between each attribute
        {
            *unused_str = realloc(*unused_str, size + (attr_len * 2));
            *cap = size + (attr_len * 2);
        }
        memcpy(*unused_str + size, " ", 1);
        size += 1;
        memcpy(*unused_str + size, attrs[i], attr_len);
        size += attr_len;
    }
    
    if(*cap <= size + 1 + between_len + 2 + tag_len + 1)
    {
        *unused_str = realloc(*unused_str, size + 1 + between_len + 2 + tag_len + 1 + 1);
        *cap = size + 1 + between_len + 2 + tag_len + 1 + 1;
    }
    
    memcpy(*unused_str + size, ">", 1);
    size += 1;
    
    memcpy(*unused_str + size, between, between_len);
    size += between_len;
    
    memcpy(*unused_str + size, "</", 2);
    size += 2;
    
    memcpy(*unused_str + size, tag, tag_len);
    size += tag_len;
    
    memcpy(*unused_str + size, ">", 1);
    size += 1;
    
    (*unused_str)[size] = '\0';
    
    ha->sizes[ unused_str - ha->buffers ] = size;
    
    htmc_set_unused(ha, between);
    
    return *unused_str;
}

// this is uselss. void tags should take no args.
char *htmc_make_tag(HtmcAllocations *ha, uint16_t tag_id)
{
    const char *tag = htmc_tags[tag_id];
    const size_t tag_len = htmc_tag_lengths[tag_id];
    
    char **unused = htmc_get_unused(ha, 1 + tag_len + 1 + 1);
    
    memcpy(*unused, "<", 1);
    memcpy(*unused + 1, tag, tag_len);
    memcpy(*unused + 1 + tag_len, ">", 1);
    (*unused)[1 + tag_len + 1] = '\0';
    
    ha->sizes[ unused - ha->buffers ] = tag_len + 2;
    
    return *unused;
}

char *htmc_repeat_(HtmcAllocations *ha, uint32_t nb, ...)
{
    va_list args;
    va_start(args, nb);
    
    char *combined_str = htmc_vconcat_strings(ha, args);
    
    va_end(args);
    
    char **combined_str_ptr = htmc_find_buffer(ha, combined_str);
    size_t *cap = &ha->caps[ combined_str_ptr - ha->buffers ];
    const size_t combined_strlen = ha->sizes[ combined_str_ptr - ha->buffers ];
    size_t *size = &ha->sizes[ combined_str_ptr - ha->buffers ];
    
    if(combined_strlen * nb >= *cap)
    {
        *combined_str_ptr = realloc(*combined_str_ptr, combined_strlen * nb + 1);
        *cap = combined_strlen * nb + 1;
    }
    
    for(uint32_t i = 1 ; i < nb ; i++)
    {
        memcpy(*combined_str_ptr + *size, *combined_str_ptr, combined_strlen);
        *size += combined_strlen;
    }
    
    (*combined_str_ptr)[*size] = '\0';
    
    return *combined_str_ptr;
}

char **htmc_strdup(HtmcAllocations *ha, char **str)
{
    size_t len = ha->sizes[ str - ha->buffers ];
    char **dup = htmc_get_unused(ha, len + 1);
    memcpy(*dup, *str, len + 1);
    
    ha->sizes[ dup - ha->buffers ] = len;
    
    return dup;
}

char *htmc_repeat_modify_(HtmcAllocations *ha, uint32_t nb, void(*mod)(const char *before_mod, size_t len, char **buffer, size_t *cap, uint32_t idx), ...)
{
    va_list args;
    va_start(args, mod);
    
    char *combined_str = htmc_vconcat_strings(ha, args);
    
    va_end(args);
    
    char **combined_str_ptr = htmc_find_buffer(ha, combined_str);
    size_t *cap = &ha->caps[ combined_str_ptr - ha->buffers ];
    size_t *size = &ha->sizes[ combined_str_ptr - ha->buffers ];
    
    char **iter_copy_ptr = htmc_strdup(ha, combined_str_ptr);
    const char *iter_copy = *iter_copy_ptr;
    const size_t copy_len = *size;
    
    char **unused_buffer = htmc_get_unused(ha, copy_len + 1);
    **unused_buffer = '\0';
    size_t *unused_cap = &ha->caps[ unused_buffer - ha->buffers ];
    
    for(uint32_t i = 0 ; i < nb ; i++)
    {
        mod(iter_copy, copy_len, unused_buffer, unused_cap, i);
        size_t modified_len = strlen(*unused_buffer);
        
        if(modified_len + *size >= *cap)
        {
            *combined_str_ptr = realloc(*combined_str_ptr, *size + (modified_len * 2));
        }
        
        memcpy(*combined_str_ptr + *size, *unused_buffer, modified_len);
        
        *size += modified_len;
    }
    
    ha->unused[ iter_copy_ptr - ha->buffers ] = true;
    ha->unused[ unused_buffer - ha->buffers ] = true;
    
    (*combined_str_ptr)[*size] = '\0';
    
    return *combined_str_ptr;
}

char *htmc_repeat_modify_r_(HtmcAllocations *ha, uint32_t nb, void(*mod)(const char *before_mod, size_t len, char **buffer, size_t *cap, uint32_t idx, void *arg), void *arg, ...)
{
    va_list args;
    va_start(args, arg);
    
    char *combined_str = htmc_vconcat_strings(ha, args);
    
    va_end(args);
    
    char **combined_str_ptr = htmc_find_buffer(ha, combined_str);
    size_t *cap = &ha->caps[ combined_str_ptr - ha->buffers ];
    size_t *size = &ha->sizes[ combined_str_ptr - ha->buffers ];
    
    char **iter_copy_ptr = htmc_strdup(ha, combined_str_ptr);
    const char *iter_copy = *iter_copy_ptr;
    const size_t copy_len = *size;
    
    char **unused_buffer = htmc_get_unused(ha, copy_len + 1);
    **unused_buffer = '\0';
    size_t *unused_cap = &ha->caps[ unused_buffer - ha->buffers ];
    
    for(uint32_t i = 0 ; i < nb ; i++)
    {
        mod(iter_copy, copy_len, unused_buffer, unused_cap, i, arg);
        size_t modified_len = strlen(*unused_buffer);
        
        if(modified_len + *size >= *cap)
        {
            *combined_str_ptr = realloc(*combined_str_ptr, *size + (modified_len * 2));
        }
        
        memcpy(*combined_str_ptr + *size, *unused_buffer, modified_len);
        
        *size += modified_len;
    }
    
    ha->unused[ iter_copy_ptr - ha->buffers ] = true;
    ha->unused[ unused_buffer - ha->buffers ] = true;
    
    (*combined_str_ptr)[*size] = '\0';
    
    return *combined_str_ptr;
}

// we could take void* as va args and check if its in range 'ha->buffers >= <= ha->buffers + nb' if so its char** if not its char*
char *htmc_fmt_(HtmcAllocations *ha, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    
    va_list args_copy1;
    va_copy(args_copy1, args);
    
    int str_len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    
    char **unused = htmc_get_unused(ha, str_len + 1);
    ha->sizes[ unused - ha->buffers ] = str_len;
    
    vsnprintf(*unused, str_len + 1, fmt, args_copy1);
    va_end(args_copy1);
    
    return *unused;
}

void htmc_append_to_buffer_idx(HtmcAllocations *ha, size_t buffer_idx, ...)
{
    char **buffer = &ha->buffers[ buffer_idx ];
    size_t *cap = &ha->caps[ buffer - ha->buffers ];
    size_t *len = &ha->sizes[ buffer - ha->buffers ];
    
    va_list list;
    va_start(list, buffer_idx);
    
    char *concated = htmc_vconcat_strings(ha, list);
    char **concated_ptr = htmc_find_buffer(ha, concated);
    size_t *concated_len = &ha->sizes[ concated_ptr - ha->buffers ];
    
    va_end(list);
    
    htmc_gurantee_cap(buffer, cap, *concated_len + *len + 1);
    len = &ha->sizes[ buffer - ha->buffers ]; // this is because htmc_gurantee_cap might realloc and len will be dangling
    
    memcpy(*buffer + *len, concated, *concated_len + 1);
    *len += *concated_len;
    
    ha->unused[ concated_ptr - ha->buffers ] = true;
}

void htmc_gurantee_cap(char **buffer, size_t *cap, size_t new_cap)
{
    if(*cap < new_cap)
    {
        *buffer = realloc(*buffer, new_cap);
        *cap = new_cap;
    }
}
