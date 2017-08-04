
#ifdef SHAREDAEMON_EXPORTS
#define SHAREDAEMON_API __declspec(dllexport)
#else
#define SHAREDAEMON_API __declspec(dllimport)
#endif


SHAREDAEMON_API void SetShareDaemon(BOOL bValue);

SHAREDAEMON_API int GetShareDaemon(void);
