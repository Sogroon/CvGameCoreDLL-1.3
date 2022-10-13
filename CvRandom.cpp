// random.cpp

#include "CvGameCoreDLL.h"
#include "CvRandom.h"
#include "cvGlobals.h"
#include "CyArgsList.h"
#include "CvGameAI.h"
#include "CvTaggedSaveFormatWrapper.h"

#define RANDOM_A      (1103515245)
#define RANDOM_C      (12345)
#define RANDOM_SHIFT  (16)

CvRandom::CvRandom()
{ 
	reset();
}


CvRandom::~CvRandom()
{ 
	uninit();
}


void CvRandom::init(unsigned long ulSeed)
{
	//--------------------------------
	// Init saved data
	reset(ulSeed);
	//--------------------------------
	// Init non-saved data
}


void CvRandom::uninit()
{
}


// FUNCTION: reset()
// Initializes data members that are serialized.
void CvRandom::reset(unsigned long ulSeed)
{
	//--------------------------------
	// Uninit class
	uninit();

	m_ulRandomSeed = ulSeed;
}


unsigned short CvRandom::get(unsigned short usNum, const TCHAR* pszLog)
{

//Afforess: MP Rand Logging (only in DEBUG DLLs)
//#ifdef _DEBUG
	unsigned long ulPrevRand = m_ulRandomSeed;
//#endif
	m_ulRandomSeed = ((RANDOM_A * m_ulRandomSeed) + RANDOM_C);

	unsigned short us = ((unsigned short)((((m_ulRandomSeed >> RANDOM_SHIFT) & MAX_UNSIGNED_SHORT) * ((unsigned long)usNum)) / (MAX_UNSIGNED_SHORT + 1)));

//#ifdef _DEBUG
	if (GC.getGameINLINE().isPitboss() || GC.getGameINLINE().isNetworkMultiPlayer())
	{
		if (pszLog != NULL)
		{
			TCHAR szOut[1024];
			if (GC.getGameINLINE().getActivePlayer() != -1)
				sprintf(szOut, "Player %d - Multiplayer RNG Log.log", GC.getGameINLINE().getActivePlayer());
			else	// f1rpo: Format string doesn't have an int param
				sprintf(szOut, "Pitboss Multiplayer RNG Log.log"/*, GC.getGameINLINE().getActivePlayer()*/);
			GC.getGameINLINE().logMsgTo(szOut, "Prev Rand Seed: %d, New Rand Seed: %d, RNG Result: %d -- usNum: (%d) on Turn Slice: %d (%s)\n", ulPrevRand, m_ulRandomSeed, us, usNum, GC.getGameINLINE().getTurnSlice(), pszLog);
		}
		else
		{
		//	TCHAR szOut[1024];
		//	if (GC.getGameINLINE().getActivePlayer() != -1)
		//		sprintf(szOut, "Player %d - Multiplayer RNG Log.log", GC.getGameINLINE().getActivePlayer());
		//	else	// f1rpo: Format string doesn't have an int param
		//		sprintf(szOut, "Pitboss Multiplayer RNG Log.log"/*, GC.getGameINLINE().getActivePlayer()*/);
		//	GC.getGameINLINE().logMsgTo(szOut, "Prev Rand Seed: %d, New Rand Seed: %d, RNG Result: %d -- usNum: (%d) on Turn Slice: %d (%s)\n", ulPrevRand, m_ulRandomSeed, us, usNum, GC.getGameINLINE().getTurnSlice(), pszLog);
		}
	}
//#endif

	return us;
}

unsigned int CvRandom::getInt()
{
	return get(MAX_UNSIGNED_SHORT,NULL) + (get(MAX_UNSIGNED_SHORT,NULL) << 16);
}


float CvRandom::getFloat()
{
	return (((float)(get(MAX_UNSIGNED_SHORT))) / ((float)MAX_UNSIGNED_SHORT));
}


void CvRandom::reseed(unsigned long ulNewValue)
{
	m_ulRandomSeed = ulNewValue;
}


unsigned long CvRandom::getSeed()
{
	return m_ulRandomSeed;
}


void CvRandom::read(FDataStreamBase* pStream)
{
	CvTaggedSaveFormatWrapper&	wrapper = CvTaggedSaveFormatWrapper::getSaveFormatWrapper();

	wrapper.AttachToStream(pStream);

	WRAPPER_READ_OBJECT_START(wrapper);

	reset();

	WRAPPER_READ(wrapper, "CvRandom", &m_ulRandomSeed);

//#ifdef _DEBUG
	if (GC.getLogging() && GC.getRandLogging() && GC.getGameINLINE().isNetworkMultiPlayer())
	{
		TCHAR szOut[1024];
		if (GC.getGameINLINE().getActivePlayer() != -1)
			sprintf(szOut, "Player %d - Multiplayer RNG Log.log", GC.getGameINLINE().getActivePlayer());
		else	// f1rpo: Format string doesn't have an int param
			sprintf(szOut, "Pitboss Multiplayer RNG Log.log"/*, GC.getGameINLINE().getActivePlayer()*/);
		GC.getGameINLINE().logMsgTo(szOut, "!!! - Reading random seed from stream, value is %d - !!!\n", m_ulRandomSeed);
	}
//#endif

	WRAPPER_READ_OBJECT_END(wrapper);
}


void CvRandom::write(FDataStreamBase* pStream)
{
	CvTaggedSaveFormatWrapper&	wrapper = CvTaggedSaveFormatWrapper::getSaveFormatWrapper();

	wrapper.AttachToStream(pStream);

	WRAPPER_WRITE_OBJECT_START(wrapper);

	WRAPPER_WRITE(wrapper, "CvRandom", m_ulRandomSeed);

//#ifdef _DEBUG
	if (GC.getLogging() && GC.getRandLogging() && GC.getGameINLINE().isNetworkMultiPlayer())
	{
		TCHAR szOut[1024];
		if (GC.getGameINLINE().getActivePlayer() != -1)
			sprintf(szOut, "Player %d - Multiplayer RNG Log.log", GC.getGameINLINE().getActivePlayer());
		else	// f1rpo: Format string doesn't have an int param
			sprintf(szOut, "Pitboss Multiplayer RNG Log.log"/*, GC.getGameINLINE().getActivePlayer()*/);
		GC.getGameINLINE().logMsgTo(szOut, "!!! - Writing random seed to stream, value is %d - !!!\n", m_ulRandomSeed);
	}
//#endif

	WRAPPER_WRITE_OBJECT_END(wrapper);
}
