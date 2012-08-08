#include "prnscp.h"

#include "storefmt.hpp"

/*
 * NavigatorFormat
 */

class NavigatorFormat : public StorageSystemFormat
{
public:
  virtual LONG identifyFile(PSZ fileName, HMMIO fileHandle);
};

