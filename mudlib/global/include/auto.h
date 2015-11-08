#ifndef _GLOBAL_AUTO_H
#define _GLOBAL_AUTO_H

#pragma combine_strings
#pragma strong_types
#pragma save_types
#pragma verbose_errors
#pragma range_check
#pragma warn_deprecated

#define FlavorsDir           "/flavors"
#define GlobalDir            "/global"
#define SecureDir            "/secure"
#define ZoneDir              "/zone"

#define _BinDir              "/bin"
#define _EtcDir              "/etc"
#define _IncludeDir          "/include"
#define _LibDir              "/lib"
#define _ModulesDir          "/modules"
#define _ObjDir              "/obj"
#define _StateDir            "/state"

#define GlobalBinDir         GlobalDir _BinDir
#define GlobalIncludeDir     GlobalDir _IncludeDir
#define GlobalLibDir         GlobalDir _LibDir
#define GlobalObjDir         GlobalDir _ObjDir

#define ArgsLib              GlobalLibDir "/args"
#define ArrayLib             GlobalLibDir "/array"
#define ClosureLib           GlobalLibDir "/closure"
#define FileLib              GlobalLibDir "/file"
#define FormatStringsLib     GlobalLibDir "/format_strings"
#define GetoptsLib           GlobalLibDir "/getopts"
#define JSONLib              GlobalLibDir "/json"
#define MessageRenderingLib  GlobalLibDir "/render_message"
#define ObjectExpansionLib   GlobalLibDir "/expand_object"
#define ObjectLib            GlobalLibDir "/object"
#define RodneyClientLib      GlobalLibDir "/rodney_client"
#define StringsLib           GlobalLibDir "/strings"
#define UserLib              GlobalLibDir "/user"

#define AccessService        GlobalObjDir "/access_service"
#define DomainTracker        GlobalObjDir "/domain_tracker"
#define FileTracker          GlobalObjDir "/file_tracker"
#define LoggerFactory        GlobalObjDir "/logger_factory"
#define LoginObject          GlobalObjDir "/login"
#define TopicService         GlobalObjDir "/topic_service"

#define MasterObject         __MASTER_OBJECT__
#define SimulEfunObject      SecureDir "/simul_efun"
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
