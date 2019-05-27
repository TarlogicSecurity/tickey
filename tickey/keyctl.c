#include "keyctl.h"

#include <linux/keyctl.h>
#include <asm/unistd.h>
#include <unistd.h>

long keyctl_get_persistent(uid_t uid, key_serial_t keyring){
	return syscall(__NR_keyctl, KEYCTL_GET_PERSISTENT, uid, keyring);
}

long keyctl_search(key_serial_t keyring, const char *type, const char *description, key_serial_t destination){
	return syscall(__NR_keyctl, KEYCTL_SEARCH, keyring, type, description, destination);
}

long keyctl_read(key_serial_t key, char *buffer, size_t buflen){
	return syscall(__NR_keyctl, KEYCTL_READ, key, buffer, buflen);
}

long keyctl_read_size(key_serial_t key){
	return keyctl_read(key, NULL, 0);
}

long keyctl_describe(key_serial_t key, char *buffer, size_t buflen){
	return syscall(__NR_keyctl, KEYCTL_DESCRIBE, key, buffer, buflen);
}

long keyctl_describe_size(key_serial_t key){
	return keyctl_describe(key, NULL, 0);
}
