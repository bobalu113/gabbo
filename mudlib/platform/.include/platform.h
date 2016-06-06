#ifndef _PLATFORM_H
#define _PLATFORM_H

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
#define ProgramLib           PlatformLibDir "/program"
#define RenderLib            PlatformLibDir "/render"
#define RodneyClientLib      PlatformLibDir "/rodney_client"
#define SessionLib           PlatformLibDir "/session"
#define SqlLib               PlatformLibDir "/sql"
#define StringsLib           PlatformLibDir "/strings"
#define StructLib            PlatformLibDir "/struct"
#define UserLib              PlatformLibDir "/user"
#define ValidationLib        PlatformLibDir "/validation"

#define AvatarMixin          PlatformModuleDir "/avatar"
#define CommandCode          PlatformModuleDir "/command"
#define CommandGiverMixin    PlatformModuleDir "/command_giver"
#define PropertyMixin        PlatformModuleDir "/property"
#define SensorMixin          PlatformModuleDir "/sensor"
#define SoulMixin            PlatformModuleDir "/soul"

#define CommandController    PlatformModuleDir "/command_controller"
#define ZoneController       PlatformModuleDir "/zone_controller"

#define AccessService        PlatformObjDir "/access_service"
#define LoggerFactory        PlatformObjDir "/logger/logger_factory"
#define LoginObject          PlatformObjDir "/login"
#define PlatformController   PlatformObjDir "/platform_controller"
#define PostalService        PlatformObjDir "/postal_service"
#define SqlClientFactory     PlatformObjDir "/sql/sql_client_factory"

#define PlatformTrackerDir   PlatformObjDir "/tracker"
#define ConnectionTracker    PlatformTrackerDir "/connection_tracker"
#define DomainTracker        PlatformTrackerDir "/domain_tracker"
#define FileTracker          PlatformTrackerDir "/file_tracker"
#define ObjectTracker        PlatformTrackerDir "/object_tracker"
#define ProgramTracker       PlatformTrackerDir "/program_tracker"
#define SessionTracker       PlatformTrackerDir "/session_tracker"
#define TopicTracker         PlatformTrackerDir "/topic_tracker"
#define TrackerTracker       PlatformTrackerDir "/tracker_tracker"
#define ZoneTracker          PlatformTrackerDir "/zone_tracker"

#define PlatformRenderDir    PlatformObjDir "/render"
#define DefaultRenderer      PlatformRenderDir "/default"
#define LoginRenderer        PlatformRenderDir "/login"

#endif  // _PLATFORM_H
