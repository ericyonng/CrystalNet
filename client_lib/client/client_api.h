#include "client/client_lib_export.h"

extern "C"
{
    typedef  int (*ClientInitPtr)();
    typedef  void (*ClientSetPtr)(char *ip, int ipLen, int port, char *account, int accountLen, char *pwd, int pwdLen);
    typedef  void (*ClientStartPtr)();
    typedef  void (*ClientClosePtr)();
    typedef  void (*SendLogPtr)(char *buffer, int bufferSize, unsigned long long uid);
    
    extern int CLIENT_LIB_EXPORT ClientInit();
    extern void CLIENT_LIB_EXPORT ClientSet(char *ip, int ipLen, int port, char *account, int accountLen, char *pwd, int pwdLen);

    extern void CLIENT_LIB_EXPORT ClientStart();

    extern void CLIENT_LIB_EXPORT ClientClose();

    extern void CLIENT_LIB_EXPORT SendLog(char *buffer, int bufferSize, unsigned long long uid);
}