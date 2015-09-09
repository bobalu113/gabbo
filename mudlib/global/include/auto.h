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

#define LibDir               "/lib"
#define ZoneDir              "/zone"
#define FlavorsDir           "/flavors"
#define GlobalDir            "/global"
#define SecureDir            "/secure"

#define MasterObject         __MASTER_OBJECT__
#define SimulEfunObject      SecureDir "/simul_efun"

#define UserLib              LibDir "/user"
#define FileLib              LibDir "/file"
#define ObjectLib            LibDir "/object"
#define ArrayLib             LibDir "/array"
#define StringsLib           LibDir "/strings"
#define ClosureLib           LibDir "/closure"
#define ArgsLib              LibDir "/args"
#define GetoptsLib           LibDir "/getopts"
#define ObjectExpansionLib   LibDir "/expand_object"
#define FormatStringsLib     LibDir "/format_strings"
#define JSONLib              LibDir "/json"
#define RodneyClientLib      LibDir "/rodney_client"

#define FileTracker          GlobalDir "/file_tracker"
#define DomainTracker        GlobalDir "/domain_tracker"

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

#endif  // _AUTO_H
