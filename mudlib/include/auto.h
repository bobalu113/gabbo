#ifndef _AUTO_H
#define _AUTO_H

#pragma combine_strings
#pragma strong_types
#pragma save_types
#pragma verbose_errors
#pragma range_check
#pragma warn_deprecated

#define LibDir "/lib"
#define ObjDir "/obj"
#define ZoneDir "/zone"
#define HomeDir "/home"
#define BinDir "/bin"

#define UserLib LibDir "/user"
#define FileLib LibDir "/file"
#define ArrayLib LibDir "/array"
#define StringsLib LibDir "/strings"
#define ArgsLib LibDir "/args"
#define GetoptsLib LibDir "/getopts"

#define CommandCode LibDir "/command"

#define MASTER_OBJECT __MASTER_OBJECT__
#define SIMUL_EFUN_OBJECT "/secure/simul_efun"
#define LOGIN_OBJECT ObjDir "/auth/login"
#define AVATAR ObjDir "/avatar"
#define COMMON_ROOM ZoneDir "/common"

#define THISO this_object()
#define THISP this_player()
#define THISI this_interactive()
#define FINDO(x) find_object(x)
#define FINDP(x) find_player(x)
#define PID THISP->query_id()

#define CAP(x) capitalize(x)

#endif  // _AUTO_H
