#ifndef _AUTO_H
#define _AUTO_H

#pragma combine_strings
#pragma strong_types
#pragma save_types
#pragma verbose_errors
#pragma range_check
#pragma warn_deprecated

#if __VERSION_MAJOR__ <= 3 && __VERSION_MINOR__ <= 2
#define LEGACY_SUPPORT
#endif

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
#define ObjectExpansionLib   LibDir "/expand_object"

#define RoomCode             ModulesDir "/room"
#define ThingCode            ModulesDir "/thing"
#define LivingCode           ModulesDir "/living"
#define CreatureCode         ModulesDir "/creature"
#define ContainerCode        ModulesDir "/container"
#define CommandCode          ModulesDir "/command"

#define IdMixin              ModulesDir "/id"
#define NameMixin            ModulesDir "/name"
#define DetailMixin          ModulesDir "/detail"
#define PropertyMixin        ModulesDir "/property"
#define ShellMixin           ModulesDir "/shell"
#define CommandGiverMixin    ModulesDir "/command_giver"
#define LockMixin            ModulesDir "/lock"

#define LoggerFactory        ObjDir "/logger_factory"
#define LoginObject          ObjDir "/auth/login"
#define Avatar               ObjDir "/avatar"

#define CommonRoom           SharedDir "/common"
#define HomeDir              PrivateDir "/home"
#define Workroom(u)          (HomeDir + "/" + u + "/workroom")

#define THISO                this_object()
#define THISP                this_player()
#define THISI                this_interactive()
#define FINDO(o)             find_object(o)
#define FINDP(p)             find_player(p)
#define PID                  THISP->query_primary_id()
#define CAP(s)               capitalize(s)
#define ENV(o)               environment(o)
#define UNAME                THISP->query_username()

#define MAXINT               0xf0000000
#define MININT               0xf0000001

#endif  // _AUTO_H
