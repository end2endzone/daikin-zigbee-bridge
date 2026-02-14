#pragma once

#include "logging.h"

class ScopeDebugger {
  private:
    const char * scope_name;
    int line;
  public:
    ScopeDebugger(const char * name, int line) : 
        scope_name(name),
        line(line) {
      logEntry(">>> %s() <ENTER>, line %d", scope_name, line);
    }

    ~ScopeDebugger() {
      logEntry(">>> %s() <LEAVE>", scope_name);
    }
};
#define LOG_SCOPE ScopeDebugger scope_debugger(__FUNCTION__,__LINE__);
#define LOG_LINE logEntry("%s(), line %d", __FUNCTION__, __LINE__);
