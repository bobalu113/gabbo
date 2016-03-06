#ifndef _PLATFORM_AUTO_H
#define _PLATFORM_AUTO_H

#pragma combine_strings
#pragma strong_types
#pragma save_types
#pragma verbose_errors
#pragma range_check
#pragma warn_deprecated

#define FlavorDir            "/flavor"
#define PlatformDir          "/platform"
#define SecureDir            "/secure"
#define UserDir              "/user"

#define _BinDir              "/.bin"
#define _EtcDir              "/.etc"
#define _IncludeDir          "/.include"
#define _LibDir              "/.lib"
#define _ModulesDir          "/.modules"
#define _ObjDir              "/.obj"

#define PlatformEtcDir       PlatformDir _EtcDir
#define PlatformIncludeDir   PlatformDir _IncludeDir
#define PlatformLibDir       PlatformDir _LibDir
#define PlatformObjDir       PlatformDir _ObjDir

#define ArgsLib              PlatformLibDir "/args"
#define ArrayLib             PlatformLibDir "/array"
#define ClosureLib           PlatformLibDir "/closure"
#define FileLib              PlatformLibDir "/file"
#define FormatStringsLib     PlatformLibDir "/format_strings"
#define GetoptsLib           PlatformLibDir "/getopts"
#define JSONLib              PlatformLibDir "/json"
#define MessageRenderingLib  PlatformLibDir "/render_message"
#define ObjectExpansionLib   PlatformLibDir "/expand_object"
#define ObjectLib            PlatformLibDir "/object"
#define RodneyClientLib      PlatformLibDir "/rodney_client"
#define StringsLib           PlatformLibDir "/strings"
#define UserLib              PlatformLibDir "/user"

#define AccessService        PlatformObjDir "/access_service"
#define DomainTracker        PlatformObjDir "/domain_tracker"
#define FileTracker          PlatformObjDir "/file_tracker"
#define LoggerFactory        PlatformObjDir "/logger_factory"
#define LoginObject          PlatformObjDir "/login"
#define TopicService         PlatformObjDir "/topic_service"

#define MasterObject         __MASTER_OBJECT__
#define SimulEfunObject      SecureDir "/simul_efun"
#define Workroom(u)          (UserDir + "/" + u + "/workroom")

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

#endif  // _PLATFORM_AUTO_H
