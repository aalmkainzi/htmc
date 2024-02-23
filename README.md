# a library for creating HTML documents in C code

```C
#include "htm.c"

int main()
{
    char *doc =
    htmldoc(
        html(
            head(
                title("my html page")
            ),
            body(
                h1("BIG TITLE"),
                p("small text")
            )
        )
    );
    
    free(doc);
}
```

## HTML tag attributes

```C
attr(tag_name, ...list of attributes)
```

```C
#include "htm.c"

int main()
{
    char *doc =
    htmldoc(
        html(
            head(
                title("my html page")
            ),
            body(
                attr(h1, "font-size: 16px;", "color: blue;")("BIG TITLE"),
                p("small text")
            )
        )
    );
    
    free(doc);
}
```
or
```C
#include "htm.c"

int main()
{
    char *doc =
    htmldoc(
        html(
            head(
                title("my html page")
            ),
            body(
                attr(h1, htmc_str(font-size: 16px; color: blue;))("BIG TITLE"),
                p("small text")
            )
        )
    );
    
    free(doc);
}
```
## Runtime Strings

```C
#include "htm.c"
#include <stdio.h>

int main()
{
    char myname[16];
    fgets(myname, 16, stdout);
    
    char *doc =
    htmldoc(
        html(
            head(
                title(myname, "'s html page")
            ),
            body(
                h1("this is ", myname, "'s HTML page")
            )
        )
    );
    
    free(doc);
}
```

## Namespacing
If the HMTL tags pollute your namespace, you can choose to prefix them all with `htmc_`:

```C
#include "htm.c"

int main()
{
#define HTMC_PREFIX
#include "htmc.h"
    
    char *doc =
    htmc_htmldoc(
        htmc_html(
            htmc_head(
                htmc_title("my html page")
            ),
            htmc_body(
                htmc_h1("BIG TITLE"),
                htmc_p("small text")
            )
        )
    );
    
    free(doc);
}
```
This can be toggled on and off by re-including the header without defining `HTMC_PREFIX`

## how to include:

in one translation unit do:
```C
#include "htm.c"
```
and in all other TUs:
```C
#include "htmc.h"
```

Or alternatively you can add `htm.c` to your build and only include `htmc.h`
