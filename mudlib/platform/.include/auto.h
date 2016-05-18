#ifndef _PLATFORM_AUTO_H
#define _PLATFORM_AUTO_H

#pragma combine_strings
#pragma strong_types
#pragma save_types
#pragma verbose_errors
#pragma local_scopes
#pragma sloppy
#pragma no_range_check
#pragma warn_deprecated
#pragma warn_empty_casts
#pragma warn_missing_return
#pragma warn_function_inconsistent

#define FlavorDir            "/flavor"
#define PlatformDir          "/platform"
#define SecureDir            "/secure"
#define UserDir              "/user"

#define _BinDir              "/.bin"
#define _EtcDir              "/.etc"
#define _IncludeDir          "/.include"
#define _LibDir              "/.lib"
#define _ModuleDir           "/.module"
#define _ObjDir              "/.obj"

#define PlatformBinDir       PlatformDir _BinDir
#define PlatformEtcDir       PlatformDir _EtcDir
#define PlatformIncludeDir   PlatformDir _IncludeDir
#define PlatformLibDir       PlatformDir _LibDir
#define PlatformModuleDir    PlatformDir _ModuleDir
#define PlatformObjDir       PlatformDir _ObjDir

#define ArgsLib              PlatformLibDir "/args"
#define ArrayLib             PlatformLibDir "/array"
#define ClosureLib           PlatformLibDir "/closure"
#define CommandLib           PlatformLibDir "/command"
#define CommandSpecLib       PlatformLibDir "/command_spec"
#define ConnectionLib        PlatformLibDir "/connection"
#define DomainLib            PlatformLibDir "/domain"
#define FileLib              PlatformLibDir "/file"
#define FormatStringsLib     PlatformLibDir "/format_strings"
#define GetoptsLib           PlatformLibDir "/getopts"
#define JSONLib              PlatformLibDir "/json"
#define MessageLib           PlatformLibDir "/message"
#define MessageRenderingLib  PlatformLibDir "/render_message"
#define ObjectExpansionLib   PlatformLibDir "/expand_object"
#define ObjectLib            PlatformLibDir "/object"
#define RenderLib            PlatformLibDir "/render"
#define RodneyClientLib      PlatformLibDir "/rodney_client"
#define StringsLib           PlatformLibDir "/strings"
#define StructLib            PlatformLibDir "/struct"
#define UserLib              PlatformLibDir "/user"

#define AvatarMixin          PlatformModuleDir "/avatar"
#define CommandCode          PlatformModuleDir "/command"
#define CommandGiverMixin    PlatformModuleDir "/command_giver"
#define PropertyMixin        PlatformModuleDir "/property"
#define SensorMixin          PlatformModuleDir "/sensor"
#define SoulMixin            PlatformModuleDir "/soul"

#define CommandController    PlatformModuleDir "/command_controller"

#define AccessService        PlatformObjDir "/access_service"
#define ConnectionTracker    PlatformObjDir "/connection_tracker"
#define DomainTracker        PlatformObjDir "/domain_tracker"
#define FileTracker          PlatformObjDir "/file_tracker"
#define LoggerFactory        PlatformObjDir "/logger_factory"
#define LoginObject          PlatformObjDir "/login"
#define PostalService        PlatformObjDir "/postal_service"
#define TopicTracker         PlatformObjDir "/topic_tracker"

#define PlatformRenderDir    PlatformObjDir "/render"
#define LoginRenderer        PlatformRenderDir "/login"

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
