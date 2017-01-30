// Platform specific (macOS)
#include <mach/mach.h>
#include <mach/mach_time.h>

double native_now() {
    static double timeConvert = 0.0;
	if ( timeConvert == 0.0 )
	{
		mach_timebase_info_data_t timeBase;
		(void)mach_timebase_info( &timeBase );
		timeConvert = (double)timeBase.numer /
			(double)timeBase.denom /
			1000000000.0;
	}
	double time_now =  (double)mach_absolute_time( ) * timeConvert;
	return time_now;
}
