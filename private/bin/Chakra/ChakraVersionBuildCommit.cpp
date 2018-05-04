#include "ChakraVersionBuildCommit.h"
#include "ChakraDllVersion.h"

#define _PARENTHESIZE2(x) "(" #x  ")"
#define _PARENTHESIZE(x) _PARENTHESIZE2(x)

#if !defined(CHAKRA_VERSION_BUILD_COMMIT)
#define _CHAKRA_BUILD_COMMIT_STRING _PARENTHESIZE("")
#else
#define _CHAKRA_BUILD_COMMIT_STRING _PARENTHESIZE(CHAKRA_VERSION_BUILD_COMMIT)
#endif

const char* chakraVersionBuildCommit = _CHAKRA_BUILD_COMMIT_STRING;
