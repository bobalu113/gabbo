
#ifdef FlavorDir
#define ParentFlavorDir      FlavorDir
#undef FlavorDir
#endif
#define FlavorDir            FlavorsDir "/" Flavor

#ifdef ModulesDir
#undef ModulesDir
#endif
#define ModulesDir           FlavorDir "/modules"

#ifdef ObjDir
#undef ObjDir
#endif
#define ObjDir               FlavorDir "/obj"

#ifdef BinDir
#undef BinDir
#endif
#define BinDir               FlavorDir "/bin"

#ifdef StateDir
#undef StateDir
#endif
#define StateDir             FlavorDir "/state"

#ifdef EtcDir
#undef EtcDir
#endif
#define EtcDir               FlavorDir "/etc"
