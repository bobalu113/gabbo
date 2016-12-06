#ifndef _USER_H
#define _USER_H

#include <sys/tls.h>

#define PASSWD_FILE             _EtcDir "/passwd.val"
#define PASSWD_HASH_METHOD      TLS_HASH_MD5
#define PASSWD_HASH_ITERATIONS  1

#endif  // _USER_H
