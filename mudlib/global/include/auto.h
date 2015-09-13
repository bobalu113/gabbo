#ifndef _GLOBAL_AUTO_H
#define _GLOBAL_AUTO_H

#pragma combine_strings
#pragma strong_types
#pragma save_types
#pragma verbose_errors
#pragma range_check
#pragma warn_deprecated

#define SecureDir            "/secure"
#define ZoneDir              "/zone"
#define FlavorsDir           "/flavors"
#define GlobalDir            "/global"

#define _BinDir              "/bin"
#define _LibDir              "/lib"
#define _IncludeDir          "/include"
#define _ObjDir              "/obj"

#define GlobalBinDir         GlobalDir _BinDir
#define GlobalLibDir         GlobalDir _LibDir
#define GlobalIncludeDir     GlobalDir _IncludeDir
#define GlobalObjDir         GlobalDir _ObjDir

#define MasterObject         __MASTER_OBJECT__
#define SimulEfunObject      SecureDir "/simul_efun"

#define UserLib              GlobalLibDir "/user"
#define FileLib              GlobalLibDir "/file"
#define ObjectLib            GlobalLibDir "/object"
#define ArrayLib             GlobalLibDir "/array"
#define StringsLib           GlobalLibDir "/strings"
#define ClosureLib           GlobalLibDir "/closure"
#define ArgsLib              GlobalLibDir "/args"
#define GetoptsLib           GlobalLibDir "/getopts"
#define ObjectExpansionLib   GlobalLibDir "/expand_object"
#define FormatStringsLib     GlobalLibDir "/format_strings"
#define JSONLib              GlobalLibDir "/json"
#define RodneyClientLib      GlobalLibDir "/rodney_client"

#define LoggerFactory        GlobalObjDir "/logger_factory"
#define LoginObject          GlobalObjDir "/auth/login"
#define FileTracker          GlobalObjDir "/file_tracker"
#define DomainTracker        GlobalObjDir "/domain_tracker"

#define CommonRoom           ZoneDir "/common"
#define HomeDir              ZoneDir "/home"
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
#define PNAME                THISP->query_name()

#define MAXINT               0xf0000000
#define MININT               0xf0000001

#endif  // _GLOBAL_AUTO_H
