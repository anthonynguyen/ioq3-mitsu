// under GPL v2

#include "ioq3-urt.h"
//for uint32_t etc. (overkill compliance ftw):
#if defined(_MSC_VER) && (_MSC_VER <= 1500) //MSVC <= 2008 doesn't have stdint.h
	#include "msvc08_stdint.h"	
#else
	#include <stdint.h> 
#endif
#if !defined(WIN32)
	#include <sys/time.h> //gettimeofday()
	extern time_t initial_tv_sec;
#else
	LONGLONG initial_lpPerformanceCount;
#endif

/*	
	ioq3-urt: Microsecond Granularity Timing - /sysMicroGranularity

	A potentially unimportant experiment. A concrete outcome is that it allows FPS throttling at any (sane) com_maxFPS
	and meters/benchmarks such as cl_drawFPS (that utilize Sys_Microseconds directly) have finer output. 

	Engine FPS is throttled using microGranularity (directly, not just converted to milliseconds) which may have benefits in
	consistency. It could be expanded to other throttling mechanisms such as cl_maxpackets, or 'god forbid', packet timing. 
	Meters/benchmarks other than cl_drawFPS, could make use of it.

	Sys_Milliseconds of sys_win32 uses Sys_Microseconds converted to ms; it also synchronizes them to allow for live switching.
	sys_unix' Sys_Millseconds already utilized gettimeofday(). 

	Sys_Millisecons return value is historically an int and timeGetTime() returned DWORD and UNIX' tv_usec a long; hopefully 
	that's not a problem for musecs->msecs either 

*/
unsigned long long Sys_Microseconds (void) { 
	#ifdef WIN32
		/*	NOTICE: This paraphernalia is unstable on certain systems due to hardware or windows API bugs
		(since frequency may not be updated properly). */

		LARGE_INTEGER lpPerformanceCount, lpFrequency;

		QueryPerformanceCounter(&lpPerformanceCount);
		QueryPerformanceFrequency(&lpFrequency);

		return (uint64_t) ((lpPerformanceCount.QuadPart - initial_lpPerformanceCount) * 1000000) / lpFrequency.QuadPart;

		/*	An alternative less precise floating calc way:
			return ((long double) lpPerformanceCount.QuadPart / lpFrequency.QuadPart) * 1000000; */
			
	#else
		struct timeval time;
		gettimeofday(&time, NULL);
			
		return (uint64_t) (time.tv_sec - initial_tv_sec) * 1000000 + time.tv_usec; 
	#endif
}

/*  This is for announcement of the feature in console, disabling it in case of unavailability,
	and for accommodating 'live' switching. */
void Sys_MicroGranularityCheck (void) {
	
	static qboolean enabled = qfalse;

	if (clu.sys_microGranularity->integer) {
		if (!enabled) {
			qboolean check; //multiplatform
			#ifdef WIN32
				LARGE_INTEGER lpFrequency;
				if (QueryPerformanceFrequency(&lpFrequency)) //this is the only check needed according to the function's documentation.
					check = qtrue; else 	check = qfalse;
			#else
				check = qtrue; // Not likely, and Sys_Milliseconds already uses gettimeofday() anyway.
			#endif
			if (check) { 
				Com_Printf("* Microsecond Definition Enabled\n");
				enabled = qtrue;
			} else {
				clu.sys_microGranularity->integer = 0;
				Com_Printf("High-resolution counter is unavailable; microsecond definition is now disabled.\n");
				enabled = qfalse;
			}
		} 
	} else if (enabled) { // sys_microGranularity 0 on the fly.
		Com_Printf("Microsecond Definition is now disabled\n");
		enabled = qfalse;
	}
}
	

