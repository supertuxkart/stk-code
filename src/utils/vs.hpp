// Visual studio workarounds in one place

#if defined(_MSC_VER)
#  include <WinSock2.h>
#  include <windows.h>
#  include <math.h>

#  define isnan _isnan
//#  define round(x) (floor(x + 0.5f))
#  define roundf(x) (floor(x + 0.5f))
#endif

