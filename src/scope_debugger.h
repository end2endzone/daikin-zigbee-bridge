#pragma once

#include "logging.h"

class ScopeDebugger {
  private:
    const char * file_name;
    const char * function_name;
    int line;
  public:
    ScopeDebugger(const char * file_name, const char * function_name, int line) : 
        file_name(file_name),
        function_name(function_name),
        line(line) {
      log_d(">>> %s::%s() <ENTER>, line %d", file_name, function_name, line);
    }

    ~ScopeDebugger() {
      log_d(">>> %s::%s() <LEAVE> (matching line %d)", file_name, function_name, line);
    }
};
#define LOG_SCOPE ScopeDebugger scope_debugger(CURRENT_SOURCE_FILE_NAME, __FUNCTION__, __LINE__);
