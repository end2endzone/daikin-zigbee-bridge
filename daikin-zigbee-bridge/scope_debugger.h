#pragma once

class ScopeDebugger {
  private:
    const char * scope_name;
    int line;
  public:
    ScopeDebugger(const char * name, int line) : 
        scope_name(name),
        line(line) {
      print("<ENTER-%s>, line %d", scope_name, line);
    }

    ~ScopeDebugger() {
      print("<LEAVE-%s>", scope_name);
    }
  private:
    void logEntry(const char* fmt, ...) {
      char buffer[2048];
      va_list args;
      
      // Convert arguments to string
      va_start(args, fmt);
      vsnprintf(buffer, sizeof(buffer), fmt, args);
      va_end(args);
      
      // Output on the serial port. 
      Serial.println(buffer);
    }
};
#define LOG_SCOPE ScopeDebugger scope_debugger(__FUNCTION__,__LINE__);
#define LOG_LINE logEntry("%s(), line %d", __FUNCTION__, __LINE__);
