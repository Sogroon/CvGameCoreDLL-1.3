#include "CvGameCoreDLL.h"

#include "BetterBTSAI.h"

#ifdef LOG_AI
int gPlayerLogLevel = 0;
int gTeamLogLevel = 0;
int gCityLogLevel = 0;
int gUnitLogLevel = 0;
#endif

// AI decision making logging

void logBBAI(char* format, ... )
{
	static CRITICAL_SECTION	cLogAccessSection;
	static bool bLogAccessCritSecInitialized = false;

	if (GC.isXMLLogging())
	{
		if ( !bLogAccessCritSecInitialized )
		{
			InitializeCriticalSectionAndSpinCount(&cLogAccessSection,4000);
			bLogAccessCritSecInitialized = true;
		}

		EnterCriticalSection(&cLogAccessSection);
		
		static char buf[2048];
		_vsnprintf( buf, 2048-4, format, (char*)(&format+1) );

		if (GC.getTurnActiveTeam() != NO_TEAM)
		{
			TCHAR szOut[1024];
			sprintf(szOut, "BBAI_Team_%d.log", GC.getTurnActiveTeam());
			
			gDLL->logMsg(szOut, buf);
		}
		else
		{
			gDLL->logMsg("BBAI.log", buf);
		}

		//	Echo to debugger
		strcat(buf, "\n");
		OutputDebugString(buf);

		LeaveCriticalSection(&cLogAccessSection);
	}
}

void logBBAIForTeam(TeamTypes eTeam, char* format, ... )
{
	static CRITICAL_SECTION	cLogAccessSection;
	static bool bLogAccessCritSecInitialized = false;

	if (GC.isXMLLogging())
	{
		if ( !bLogAccessCritSecInitialized )
		{
			InitializeCriticalSectionAndSpinCount(&cLogAccessSection,4000);
			bLogAccessCritSecInitialized = true;
		}

		EnterCriticalSection(&cLogAccessSection);
		
		static char buf[2048];
		_vsnprintf( buf, 2048-4, format, (char*)(&format+1) );
		if (eTeam != NO_TEAM)
		{
			TCHAR szOut[1024];
			sprintf(szOut, "BBAI_Team_%d.log", eTeam);
			
			gDLL->logMsg(szOut, buf);
		}
		else
		{
			gDLL->logMsg("BBAI.log", buf);
		}

		//	Echo to debugger
		strcat(buf, "\n");
		OutputDebugString(buf);

		LeaveCriticalSection(&cLogAccessSection);
	}
}
