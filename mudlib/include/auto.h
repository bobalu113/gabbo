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
#define HomeDir              "/home"
#define DomainDir            "/domain"
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
#define CommonRoom           DomainDir "/common"

#define THISO                this_object()
#define THISP                this_player()
#define THISI                this_interactive()
#define FINDO(x)             find_object(x)
#define FINDP(x)             find_player(x)
#define PID                  THISP->query_id()
#define CAP(x)               capitalize(x)

#endif  // _AUTO_H
