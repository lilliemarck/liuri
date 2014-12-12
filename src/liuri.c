#include "liuri.h"
#include <string.h>

#define ALPHA_BIT               1
#define DIGIT_BIT               2
#define XDIGIT_BIT              4
#define PLUS_MINUS_DOT_BIT      8
#define UNRESERVED_SUBDELIM_BIT 16
#define AT_SIGN_BIT             32
#define COLON_BIT               64
#define SLASH_QUESTION_BIT      128

#define ALPHA_SET          (ALPHA_BIT)
#define DIGIT_SET          (DIGIT_BIT)
#define XDIGIT_SET         (XDIGIT_BIT)
#define SCHEME_SET         (ALPHA_BIT | DIGIT_BIT | PLUS_MINUS_DOT_BIT)
#define REG_NAME_SET       (ALPHA_BIT | DIGIT_BIT | UNRESERVED_SUBDELIM_BIT)
#define USERINFO_SET       (ALPHA_BIT | DIGIT_BIT | UNRESERVED_SUBDELIM_BIT | COLON_BIT)
#define IPVFUTURE_SET      (ALPHA_BIT | DIGIT_BIT | UNRESERVED_SUBDELIM_BIT | COLON_BIT)
#define PATH_NC_SET        (ALPHA_BIT | DIGIT_BIT | UNRESERVED_SUBDELIM_BIT | AT_SIGN_BIT)
#define PATH_SET           (ALPHA_BIT | DIGIT_BIT | UNRESERVED_SUBDELIM_BIT | AT_SIGN_BIT | COLON_BIT)
#define QUERY_FRAGMENT_SET (ALPHA_BIT | DIGIT_BIT | UNRESERVED_SUBDELIM_BIT | AT_SIGN_BIT | COLON_BIT | SLASH_QUESTION_BIT)

static unsigned char const sets[256] = {
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

static int in_set(unsigned char set, char c) {
    return sets[(unsigned char)c] & set;
}

static int match_set(char const **str, char const *end, unsigned char set) {
    if (*str < end && in_set(set, **str)) {
        ++*str;
        return 1;
    }

    return 0;
}

static void match_xset(char const **str, char const *end, unsigned char set) {
    while (*str < end && in_set(set, **str)) {
        ++*str;
    }
}

static void match_xset_enc(char const **str, char const *end, unsigned char set) {
    while (*str < end) {
        if (in_set(set, **str)) {
            ++*str;
            continue;
        }

        if (*str + 2 < end && (*str)[0] == '%' && in_set(XDIGIT_SET, (*str)[1]) && in_set(XDIGIT_SET, (*str)[2])) {
            *str += 3;
            continue;
        }

        break;
    }
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

    if (match_set(&i, end, ALPHA_SET)) {
        match_xset(&i, end, SCHEME_SET);

        if (match_char(&i, end, ':')) {
            components->scheme.string = str;
            components->scheme.length = i - str - 1;
            return i;
        }
    }

    return str;
}

static char const *match_userinfo(char const *str, char const *end, struct liuri_components *components) {
    char const *i = str;
    match_xset_enc(&i, end, USERINFO_SET);

    if (match_char(&i, end, '@')) {
        components->userinfo.string = str;
        components->userinfo.length = i - str - 1;
        return i;
    }

    return str;
}

/*
 * Matches a decimal number between 0 and 255 with no extra leading zeros.
 */
static int match_dec_octet(char const **str, char const *end) {
    char const *i = *str;

    if (i < end && in_set(DIGIT_SET, *i)) {
        int value = (*i++ - '0');

        if (value != 0) {
            if (i < end && in_set(DIGIT_SET, *i)) {
                value = 10 * value + (*i++ - '0');

                if (i < end && in_set(DIGIT_SET, *i)) {
                    value = 10 * value + (*i++ - '0');
                }
            }

            if (value > 255) {
                *str += 2;
                return 1;
            }
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
 * Matches one to four hexadecimal digits.
 */
static int match_h16(char const **str, char const *end) {
    char const *i = *str;

    if (match_set(&i, end, XDIGIT_SET)) {
        if (match_set(&i, end, XDIGIT_SET)) {
            if (match_set(&i, end, XDIGIT_SET)) {
                if (match_set(&i, end, XDIGIT_SET)) {
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
        if (match_set(&i, end, XDIGIT_SET)) {
            match_xset(&i, end, XDIGIT_SET);

            if (match_char(&i, end, '.')) {
                if (match_set(&i, end, IPVFUTURE_SET)) {
                    match_xset(&i, end, IPVFUTURE_SET);
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

static char const *match_host(char const *str, char const *end, struct liuri_components *components, int *type) {
    char const *i = str;

    if (match_ip_literal(&i, end, type)) {
    } else if (match_ipv4_address(&i, end)) {
        *type = LIURI_HOST_IPV4;
    } else {
        match_xset_enc(&i, end, REG_NAME_SET);
        *type = LIURI_HOST_NAME;
    }

    components->host.string = str;
    components->host.length = i - str;
    return i;
}

static char const *match_port(char const *str, char const *end, struct liuri_components *components) {
    if (match_char(&str, end, ':')) {
        char const *i = str;
        match_xset(&i, end, DIGIT_SET);
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

/*
 * Matches an absolute path that may contain consecutive '/' anywhere.
 */
static void match_path_absolute(char const **str, char const *end) {
    while (match_char(str, end, '/')) {
        match_xset_enc(str, end, PATH_SET);
    }
}

static char const *match_path(char const *str, char const *end, struct liuri_components *components) {
    char const *i = str;

    if (components->authority.string) {
        match_path_absolute(&i, end);
    } else if (components->scheme.string) {
        match_xset_enc(&i, end, PATH_SET);
        match_path_absolute(&i, end);
    } else {
        match_xset_enc(&i, end, PATH_NC_SET);
        match_path_absolute(&i, end);
    }

    components->path.string = str;
    components->path.length = i - str;
    return i;
}

static char const *match_query_fragment(char const *str, char const *end, char delim, struct liuri_match *match) {
    if (match_char(&str, end, delim)) {
        char const *i = str;
        match_xset_enc(&i, end, QUERY_FRAGMENT_SET);
        match->string = str;
        match->length = i - str;
        return i;
    }

    return str;
}

int liuri_host_type(char const *host, int size) {
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
    uri = match_query_fragment(uri, end, '?', &components->query);
    uri = match_query_fragment(uri, end, '#', &components->fragment);
    return uri == end;
}
