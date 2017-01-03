#ifndef _LOGIN_H
#define _LOGIN_H

#define LOCALHOST              "127.0.0.1"
#define WELCOME_FILE           PlatformEtcDir "/issue"
#define TERMINAL_MAX_TRIES     3
#define TIMEOUT_SECS           (10 * 60)
#define DEFAULT_TERMINAL_TYPE  "vt100"
#define CLEAR_SCREEN           1
#define InsecureWarning        "Warning: You are on an insecure connection. " \
                               "Act accordingly.\n"
#define DefaultTermWarning     "Unable to detect terminal type. " \
                               "Using default.\n"
#define TimeoutMessage         "Timeout exceeded, disconnecting...\n"

#endif  // _LOGIN_H
