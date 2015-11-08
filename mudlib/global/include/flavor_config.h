#ifdef FlavorDir
#define ParentFlavorDir      FlavorDir
#undef FlavorDir
#endif
#define FlavorDir            FlavorsDir "/" Flavor

#ifdef _BinDir
#undef _BinDir
#endif
#define BinDir               FlavorDir _BinDir

#ifdef _EtcDir
#undef _EtcDir
#endif
#define EtcDir               FlavorDir _EtcDir

#ifdef _IncludeDir
#undef _IncludeDir
#endif
#define IncludeDir           FlavorDir _IncludeDir

#ifdef _LibDir
#undef _LibDir
#endif
#define LibDir               FlavorDir _LibDir

#ifdef _ModulesDir
#undef _ModulesDir
#endif
#define ModulesDir           FlavorDir _ModulesDir

#ifdef _ObjDir
#undef _ObjDir
#endif
#define ObjDir               FlavorDir _ObjDir

#ifdef _StateDir
#undef _StateDir
#endif
#define StateDir             FlavorDir _StateDir
