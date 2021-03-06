// This file is automatically generated.
#include "cssysdef.h"
#include "csutil/scf.h"

// Put static linking stuff into own section.
// The idea is that this allows the section to be swapped out but not
// swapped in again b/c something else in it was needed.
#if !defined(CS_DEBUG) && defined(CS_COMPILER_MSVC)
#pragma const_seg(".CSmetai")
#pragma comment(linker, "/section:.CSmetai,r")
#pragma code_seg(".CSmeta")
#pragma comment(linker, "/section:.CSmeta,er")
#pragma comment(linker, "/merge:.CSmetai=.CSmeta")
#endif

namespace csStaticPluginInit
{
static char const metainfo_recast[] =
"<?xml version=\"1.0\"?>"
"<!-- recast.csplugin -->"
"<plugin>"
"  <scf>"
"    <classes>      "
"      <class>"
"        <name>cel.navmeshbuilder</name>"
"        <implementation>celNavMeshBuilder</implementation>"
"        <description>CEL navigation mesh builder</description>"
"      </class>      "
"      <class>"
"        <name>cel.hnavstructbuilder</name>"
"        <implementation>celHNavStructBuilder</implementation>"
"        <description>CEL hierarchical navigation structure builder</description>"
"      </class>"
"    </classes>"
"  </scf>"
"</plugin>"
;
  #ifndef celNavMeshBuilder_FACTORY_REGISTER_DEFINED 
  #define celNavMeshBuilder_FACTORY_REGISTER_DEFINED 
    SCF_DEFINE_FACTORY_FUNC_REGISTRATION(celNavMeshBuilder) 
  #endif
  #ifndef celHNavStructBuilder_FACTORY_REGISTER_DEFINED 
  #define celHNavStructBuilder_FACTORY_REGISTER_DEFINED 
    SCF_DEFINE_FACTORY_FUNC_REGISTRATION(celHNavStructBuilder) 
  #endif

class recast
{
SCF_REGISTER_STATIC_LIBRARY(recast,metainfo_recast)
  #ifndef celNavMeshBuilder_FACTORY_REGISTERED 
  #define celNavMeshBuilder_FACTORY_REGISTERED 
    celNavMeshBuilder_StaticInit celNavMeshBuilder_static_init__; 
  #endif
  #ifndef celHNavStructBuilder_FACTORY_REGISTERED 
  #define celHNavStructBuilder_FACTORY_REGISTERED 
    celHNavStructBuilder_StaticInit celHNavStructBuilder_static_init__; 
  #endif
public:
 recast();
};
recast::recast() {}

}
