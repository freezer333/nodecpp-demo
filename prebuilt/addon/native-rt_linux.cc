// Platform specific (linux)
#include <unistd.h>	
#include <time.h>	
#include <sys/time.h>

double native_now() {
    struct timespec ts;
#if defined(CLOCK_MONOTONIC_RAW)
    const clockid_t id = CLOCK_MONOTONIC_RAW;
#elif defined(CLOCK_REALTIME)
    const clockid_t id = CLOCK_REALTIME;
#else
    const clockid_t id = (clockid_t)-1;
#endif
   if ( id != (clockid_t)-1 && clock_gettime( id, &ts ) != -1 ){
		double time_now = (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
		return time_now;
    }
    return 0;
}
      
