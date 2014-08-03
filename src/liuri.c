#include "liuri.h"
#include <string.h>

#define ALPHA_BIT                   (1)
#define DIGIT_BIT                   (2)
#define XDIGIT_BIT                  (4)
#define SCHEME_BIT                  (8)
#define UNRESERVED_AND_SUBDELIM_BIT (16)
#define AT_SIGN_BIT                 (32)
#define COLON_BIT                   (64)
#define QUERY_AND_FRAGMENT_BIT      (128)

static unsigned char const cset[256] = {
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
//       !           $       &   '   (   )   *   +   ,   -   .   /
     0, 16,  0,  0, 16,  0, 16, 16, 16, 16, 16, 24, 16, 24, 24,128,
//   0   1   2   3   4   5   6   7   8   9   :   ;       =       ?
     6,  6,  6,  6,  6,  6,  6,  6,  6,  6, 64, 16,  0, 16,  0,128,
//   @   A   C   C   D   E   F   G   H   I   J   K   L   M   N   O
    32,  5,  5,  5,  5,  5,  5,  1,  1,  1,  1,  1,  1,  1,  1,  1,
//   P   Q   R   S   T   U   V   W   Z   Y   Z       \           _
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0, 16,
//       a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
     0,  5,  5,  5,  5,  5,  5,  1,  1,  1,  1,  1,  1,  1,  1,  1,
//   p   q   r   s   t   u   v   w   z   y   z               ~
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0, 16,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
} ;

static int is_alpha(char c) {
    return cset[(unsigned char)c] & (ALPHA_BIT);
}

static int is_digit(char c) {
    return cset[(unsigned char)c] & (DIGIT_BIT);
}

static int is_xdigit(char c) {
    return cset[(unsigned char)c] & (XDIGIT_BIT);
}

static int is_scheme(char c) {
    return cset[(unsigned char)c] & (ALPHA_BIT | DIGIT_BIT | SCHEME_BIT);
}

static int is_unreserved_or_subdelim(char c) {
    return cset[(unsigned char)c] & (ALPHA_BIT | DIGIT_BIT | UNRESERVED_AND_SUBDELIM_BIT);
}

static int is_ipvfuture(char c) {
    return cset[(unsigned char)c] & (ALPHA_BIT | DIGIT_BIT | UNRESERVED_AND_SUBDELIM_BIT | COLON_BIT);
}

static int is_pchar_nc(char c) {
    return cset[(unsigned char)c] & (ALPHA_BIT | DIGIT_BIT | UNRESERVED_AND_SUBDELIM_BIT | AT_SIGN_BIT);
}

static int is_pchar(char c) {
    return cset[(unsigned char)c] & (ALPHA_BIT | DIGIT_BIT | UNRESERVED_AND_SUBDELIM_BIT | AT_SIGN_BIT | COLON_BIT);
}

static int is_query_or_fragment(char c) {
    return cset[(unsigned char)c] & (ALPHA_BIT | DIGIT_BIT | UNRESERVED_AND_SUBDELIM_BIT | AT_SIGN_BIT | COLON_BIT | QUERY_AND_FRAGMENT_BIT);
}

/*
 * Checks for a %-encoded part and skips two (not three) characters.
 */
static int is_pct_encoded(char const **i, char const *end) {
    if (*i + 2 < end && (*i)[0] == '%' && is_xdigit((*i)[1]) && is_xdigit((*i)[2])) {
        *i += 2;
        return 1;
    }

    return 0;
}

static int match_char(char const **str, char const *end, char c) {
    if (*str < end && **str == c) {
        ++*str;
        return 1;
    }

    return 0;
}

static inline char const *match_scheme(char const *str, char const *end, struct liuri_components *components) {
    char const *i = str;

    if (i < end && is_alpha(*i)) {
        for (i = i + 1; i < end; ++i) {
            char c = *i;

            if (is_scheme(c)) {
                continue;
            }

            if (c == ':') {
                components->scheme.string = str;
                components->scheme.length = i - str;
                return i + 1;
            }

            break;
        }
    }

    return str;
}

static char const *match_userinfo(char const *str, char const *end, struct liuri_components *components) {
    for (char const *i = str; i < end; ++i) {
        if (is_unreserved_or_subdelim(*i) || is_pct_encoded(&i, end)) {
            continue;
        }

        if (*i == '@') {
            components->userinfo.string = str;
            components->userinfo.length = i - str;
            return i + 1;
        }

        break;
    }

    return str;
}

/*
 * Matches a decimal number between 0 and 255 with no extra leading zeros.
 */
static int match_dec_octet(char const **str, char const *end) {
    char const *i = *str;

    if (i < end && is_digit(*i)) {
        int value = (*i++ - '0');

        if (value == 0) {
            *str = i;
            return 1;
        }

        /*
         * Unrolling this loop gives a bigger binary
         */
        for (int n = 2; n > 0; --n) {
            if (i < end && is_digit(*i)) {
                value = 10 * value + (*i++ - '0');
            }
        }

        if (value > 255) {
            *str = *str + 2;
            return 1;
        }

        *str = i;
        return 1;
    }

    return 0;
}

static int match_ipv4_address(char const **str, char const *end) {
    char const *i = *str;

    if (match_dec_octet(&i, end)) {
        if (match_char(&i, end, '.')) {
            if (match_dec_octet(&i, end)) {
                if (match_char(&i, end, '.')) {
                    if (match_dec_octet(&i, end)) {
                        if (match_char(&i, end, '.')) {
                            if (match_dec_octet(&i, end)) {
                                *str = i;
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

/*
 * Matches zero to four hexadecimal digits.
 */
static int match_h16(char const **str, char const *end) {
    char const *i = *str;

    if (i < end && is_xdigit(*i)) {
        ++i;

        if (i < end && is_xdigit(*i)) {
            ++i;

            if (i < end && is_xdigit(*i)) {
                ++i;

                if (i < end && is_xdigit(*i)) {
                    ++i;
                }
            }
        }

        *str = i;
        return 1;
    }

    return 0;
}

static int match_ipv6_address(char const **str, char const *end) {
    char const *i = *str;
    char const *j;
    int count = 1;
    int compress = 0;

    if (!match_h16(&i, end)) {
        if (i + 1 < end && i[0] == ':' && i[1] == ':') {
            ++i;
            compress = 1;
        } else {
            return 0;
        }
    }

    while (i < end) {
        if (match_char(&i, end, ':')) {
            j = i;

            if (match_h16(&i, end)) {
                if (++count == 8) {
                    *str = i;
                    return 1;
                }
            } else if (*i == ':') {
                if (compress) {
                    return 0;
                } else {
                    if (++count == 8) {
                        *str = i + 1;
                        return 1;
                    }

                    compress = 1;
                }
            } else if (compress && count == 1) {
                *str = i;
                return 1;
            } else {
                return 0;
            }
        } else if (compress) {
            if (count < 8 && *i == '.' && match_ipv4_address(&j, end)) {
                *str = j;
                return 1;
            } else {
                *str = i;
                return 1;
            }
        } else {
            if (count == 7 && *i == '.' && match_ipv4_address(&j, end)) {
                *str = j;
                return 1;
            } else {
                return 0;
            }
        }
    }

    return 0;
}

static int match_ipvfuture(char const **str, char const *end) {
    char const *i = *str;

    if (match_char(&i, end, 'v')) {
        if (i < end && is_xdigit(*i)) {
            for (i = i + 1; i < end; ++i) {
                if (!is_xdigit(*i)) {
                    break;
                }
            }

            if (match_char(&i, end, '.')) {
                if (i < end && (is_ipvfuture(*i))) {
                    for (i = i + 1; i < end; ++i) {
                        if (!(is_ipvfuture(*i))) {
                            break;
                        }
                    }

                    *str = i;
                    return 1;
                }
            }
        }
    }

    return 0;
}

static int match_ip_literal(char const **str, char const *end, int *type) {
    char const *i = *str;
    int temp = 0;

    if (match_char(&i, end, '[')) {
        if (match_ipv6_address(&i, end)) {
            temp = LIURI_HOST_IPV6;
        }
        else if (match_ipvfuture(&i, end)) {
            temp = LIURI_HOST_IPVFUTURE;
        }

        if (temp != 0 && match_char(&i, end, ']')) {
            *str = i;
            *type = temp;
            return 1;
        }
    }

    return 0;
}

static int match_reg_name(char const **str, char const *end, struct liuri_components *components) {
    char const *i = *str;

    while (i < end && (is_unreserved_or_subdelim(*i) || is_pct_encoded(&i, end))) {
        ++i;
    }

    *str = i;
    return 1;
}

static char const *match_host(char const *str, char const *end, struct liuri_components *components, int *type) {
    char const *i = str;

    if (match_ip_literal(&i, end, type)) {
        components->host.string = str;
        components->host.length = i - str;
    } else if (match_ipv4_address(&i, end)) {
        components->host.string = str;
        components->host.length = i - str;
        *type = LIURI_HOST_IPV4;
    } else if (match_reg_name(&i, end, components)) {
        components->host.string = str;
        components->host.length = i - str;
        *type = LIURI_HOST_NAME;
    }

    return i;
}

static char const *match_port(char const *str, char const *end, struct liuri_components *components) {
    if (match_char(&str, end, ':')) {
        char const *i = str;

        while (i < end && is_digit((unsigned char)*i)) {
            ++i;
        }

        components->port.string = str;
        components->port.length = i - str;
        return i;
    }

    return str;
}

static char const *match_authority(char const *str, char const *end, struct liuri_components *components) {
    if (str + 1 < end && str[0] == '/' && str[1] == '/') {
        str += 2;
        char const *i = str;
        int type = 0;

        i = match_userinfo(i, end, components);
        i = match_host(i, end, components, &type);
        i = match_port(i, end, components);

        components->authority.string = str;
        components->authority.length = i - str;
        return i;
    }

    return str;
}

static int match_pchar(char const **str, char const *end) {
    char const *i = *str;

    if (i < end && (is_pchar(*i) || is_pct_encoded(&i, end))) {
        *str = i + 1;
        return 1;
    }

    return 0;
}

static int match_pchar_nc(char const **str, char const *end) {
    char const *i = *str;

    if (i < end && (is_pchar_nc(*i) || is_pct_encoded(&i, end))) {
        *str = i + 1;
        return 1;
    }

    return 0;
}

static void match_segment(char const **str, char const *end) {
    while (match_pchar(str, end)) {
    }
}

static void match_segment_nc(char const **str, char const *end) {
    while (match_pchar_nc(str, end)) {
    }
}

/*
 * Matches an absolute path that may contain consecutive '/' anywhere.
 */
static void match_path_absolute(char const **str, char const *end) {
    while (match_char(str, end, '/')) {
        match_segment(str, end);
    }
}

static char const *match_path(char const *str, char const *end, struct liuri_components *components) {
    char const *i = str;

    if (components->authority.string) {
        match_path_absolute(&i, end);
    } else if (components->scheme.string) {
        match_segment(&i, end);
        match_path_absolute(&i, end);
    } else {
        match_segment_nc(&i, end);
        match_path_absolute(&i, end);
    }

    components->path.string = str;
    components->path.length = i - str;
    return i;
}

static int match_fragment_or_query(char const **str, char const *end) {
    char const *i = *str;

    if (i < end && (is_query_or_fragment(*i) || is_pct_encoded(&i, end))) {
        *str = i + 1;
        return 1;
    }

    return 0;
}

static char const *match_query(char const *str, char const *end, struct liuri_components *components) {
    if (str < end && str[0] == '?') {
        char const *i = ++str;

        while (match_fragment_or_query(&i, end)) {
        }

        components->query.string = str;
        components->query.length = i - str;
        return i;
    }

    return str;
}

static char const *match_fragment(char const *str, char const *end, struct liuri_components *components) {
    if (str < end && str[0] == '#') {
        char const *i = ++str;

        while (match_fragment_or_query(&i, end)) {
        }

        components->fragment.string = str;
        components->fragment.length = i - str;
        return i;
    }

    return str;
}

int liuri_address_type(char const *host, int size) {
    if (size == -1) {
        size = strlen(host);
    }

    struct liuri_components components;
    int type = LIURI_HOST_UNKNOWN;
    char const *end = host + size;

    if (match_host(host, end, &components, &type) == end) {
        return type;
    }

    return LIURI_HOST_UNKNOWN;
}

int liuri_parse(char const *uri, int size, struct liuri_components *components) {
    if (size == -1) {
        size = strlen(uri);
    }

    *components = (struct liuri_components){{0}};
    char const *end = uri + size;

    uri = match_scheme(uri, end, components);
    uri = match_authority(uri, end, components);
    uri = match_path(uri, end, components);
    uri = match_query(uri, end, components);
    uri = match_fragment(uri, end, components);
    return uri == end;
}
