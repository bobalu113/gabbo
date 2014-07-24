#ifndef _AUTO_H
#define _AUTO_H

#pragma combine_strings
#pragma strong_types
#pragma save_types
#pragma verbose_errors
#pragma range_check
#pragma warn_deprecated

#define MasterObject         __MASTER_OBJECT__
#define SimulEfunObject      "/secure/simul_efun"

#define LibDir               "/lib"
#define ModulesDir           "/modules"
#define ObjDir               "/obj"
#define BinDir               "/bin"
#define PublicDir            "/public"
#define PrivateDir           "/private"
#define SharedDir            "/shared"
#define StateDir             "/state"
#define EtcDir               "/etc"

#define UserLib              LibDir "/user"
#define FileLib              LibDir "/file"
#define ArrayLib             LibDir "/array"
#define StringsLib           LibDir "/strings"
#define ArgsLib              LibDir "/args"
#define GetoptsLib           LibDir "/getopts"

#define RoomCode             ModulesDir "/room"
#define ThingCode            ModulesDir "/thing"
#define LivingCode           ModulesDir "/living"
#define CreatureCode         ModulesDir "/creature"
#define ContainerCode        ModulesDir "/container"
#define CommandCode          ModulesDir "/command"

#define IdMixin              ModulesDir "/id"
#define NameMixin            ModulesDir "/name"
#define ShellMixin           ModulesDir "/shell"
#define CommandGiverMixin    ModulesDir "/command_giver"
#define LockMixin            ModulesDir "/lock"

#define LoginObject          ObjDir "/auth/login"
#define Avatar               ObjDir "/avatar"
#define CommonRoom           SharedDir "/common"
#define HomeDir              PrivateDir "/home"
#define Workroom(u)          HomeDir + "/" + u + "/workroom"

#define THISO                this_object()
#define THISP                this_player()
#define THISI                this_interactive()
#define FINDO(o)             find_object(o)
#define FINDP(p)             find_player(p)
#define PID                  THISP->query_primary_id()
#define CAP(s)               capitalize(s)

#endif  // _AUTO_H
