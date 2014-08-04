#pragma once

#define LIURI_HOST_UNKNOWN   (0)
#define LIURI_HOST_NAME      (1)
#define LIURI_HOST_IPV4      (2)
#define LIURI_HOST_IPV6      (3)
#define LIURI_HOST_IPVFUTURE (4)

struct liuri_match {
    char const *string;
    int length;
};

struct liuri_components {
    struct liuri_match scheme;
    struct liuri_match authority;
    struct liuri_match userinfo;
    struct liuri_match host;
    struct liuri_match port;
    struct liuri_match path;
    struct liuri_match query;
    struct liuri_match fragment;
};

int liuri_host_type(char const *host, int size);
int liuri_parse(char const *uri, int size, struct liuri_components *components);
