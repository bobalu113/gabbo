#ifndef _AUTO_H
#define _AUTO_H

#pragma combine_strings
#pragma strong_types
#pragma save_types
#pragma verbose_errors
#pragma range_check
#pragma warn_deprecated

#define MASTER_OBJECT        __MASTER_OBJECT__
#define SIMUL_EFUN_OBJECT    "/secure/simul_efun"

#define LibDir               "/lib"
#define ModulesDir           "/modules"
#define ObjDir               "/obj"
#define BinDir               "/bin"
#define PublicDir            "/public"
#define PrivateDir           "/private"
#define StateDir             "/state"
#define EtcDir               "/etc"

#define UserLib              LibDir "/user"
#define FileLib              LibDir "/file"
#define ArrayLib             LibDir "/array"
#define StringsLib           LibDir "/strings"
#define ArgsLib              LibDir "/args"
#define GetoptsLib           LibDir "/getopts"

#define CreatureCode         ModulesDir "/creature"
#define RoomCode             ModulesDir "/room"
#define ThingCode            ModulesDir "/thing"
#define CommandCode          ModulesDir "/command"

#define CommandGiverMixin    ModulesDir "/command_giver"
#define IdMixin              ModulesDir "/id"

#define LoginObject          ObjDir "/auth/login"
#define Avatar               ObjDir "/avatar"
#define CommonRoom           PublicDir "/common"
#define HomeDir              PrivateDir "/home"
#define Workroom(u)          HomeDir + "/" + u + "/workroom"

#define THISO                this_object()
#define THISP                this_player()
#define THISI                this_interactive()
#define FINDO(o)             find_object(o)
#define FINDP(p)             find_player(p)
#define PID                  THISP->query_id()
#define CAP(s)               capitalize(s)

#endif  // _AUTO_H
