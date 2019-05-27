
#ifndef KEYCTL_C
#define KEYCTL_C

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <linux/keyctl.h>

typedef int32_t key_serial_t;


long keyctl_get_persistent(uid_t uid, key_serial_t keyring);

long keyctl_search(key_serial_t keyring, const char *type, const char *description, key_serial_t destination);

long keyctl_read(key_serial_t key, char *buffer, size_t buflen);

long keyctl_read_size(key_serial_t key);

long keyctl_describe(key_serial_t key, char *buffer, size_t buflen);

long keyctl_describe_size(key_serial_t key);

#endif
