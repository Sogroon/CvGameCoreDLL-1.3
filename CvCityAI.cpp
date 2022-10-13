// cityAI.cpp

#include "CvGameCoreDLL.h"
#include "CvGlobals.h"
#include "CvGameCoreUtils.h"
#include "CvCityAI.h"
#include "CvGameAI.h"
#include "CvPlot.h"
#include "CvArea.h"
#include "CvPlayerAI.h"
#include "CvTeamAI.h"
#include "CyCity.h"
#include "CyArgsList.h"
#include "CvInfos.h"
#include "CvReachablePlotSet.h"
#include "FProfiler.h"

#include "CvDLLPythonIFaceBase.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLFAStarIFaceBase.h"

/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  10/02/09								jdog5000	  */
/*																							  */
/* AI logging																				   */
/************************************************************************************************/
#include "BetterBTSAI.h"
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

//	KOSHLING MOD - calculate all possible building focuses at once
//	to avoid multiple looping - need to know how many options there
//	are.  Note that I am also ASSUMING only one flag is passed at a
//	time (and runtime checking that)
#define BUILDINGFOCUSINDEX_ANY					0
#define BUILDINGFOCUSINDEX_FOOD					1
#define BUILDINGFOCUSINDEX_PRODUCTION			2
#define BUILDINGFOCUSINDEX_GOLD					3
#define BUILDINGFOCUSINDEX_RESEARCH				4
#define BUILDINGFOCUSINDEX_CULTURE				5
#define BUILDINGFOCUSINDEX_DEFENSE				6
#define BUILDINGFOCUSINDEX_HAPPY				7
#define BUILDINGFOCUSINDEX_HEALTHY				8
#define BUILDINGFOCUSINDEX_EXPERIENCE			9
#define BUILDINGFOCUSINDEX_MAINTENANCE			10
#define BUILDINGFOCUSINDEX_SPECIALIST			11
#define BUILDINGFOCUSINDEX_ESPIONAGE			12
#define BUILDINGFOCUSINDEX_BIGCULTURE			13
#define BUILDINGFOCUSINDEX_WORLDWONDER			14
#define BUILDINGFOCUSINDEX_DOMAINSEA			15
#define BUILDINGFOCUSINDEX_WONDEROK				16
#define BUILDINGFOCUSINDEX_CAPITAL				17

#define NUM_REAL_BUILDINGFOCUS_FLAGS			18
//	Pseudo flags which represent combinations used in the calculation
//	and are not independent so have to be accumulated separately
#define BUILDINGFOCUSINDEX_GOLDANDRESEARCH		18
#define BUILDINGFOCUSINDEX_GOLDANDMAINTENANCE	19

#define NUM_ALL_BUILDINGFOCUS_FLAGS				20

static CRITICAL_SECTION 	g_cRouteUpdateSection;
static CRITICAL_SECTION 	g_cEvaluateDangerSection;
static bool					g_bStaticsInitialized;

//	Helper function to detrmine if a given bonus is provided by a building
static bool isFreeBonusOfBuilding(CvBuildingInfo& kBuilding, BonusTypes eBonus)
{
	return kBuilding.isFreeBonusOfBuilding(eBonus);
}

// Public Functions...

CvCityAI::CvCityAI()
{
	if ( !g_bStaticsInitialized )
	{
		InitializeCriticalSection(&g_cEvaluateDangerSection);
		InitializeCriticalSection(&g_cRouteUpdateSection);
		g_bStaticsInitialized = true;
	}
	InitializeCriticalSectionAndSpinCount(&m_cBestUnitCacheSection, 4000);

	m_aiEmphasizeYieldCount = new int[NUM_YIELD_TYPES];
	m_aiEmphasizeCommerceCount = new int[NUM_COMMERCE_TYPES];
	m_bForceEmphasizeCulture = false;
	m_aiSpecialYieldMultiplier = new int[NUM_YIELD_TYPES];
	m_aiPlayerCloseness = new int[MAX_PLAYERS];

	m_pbEmphasize = NULL;
	m_pbEmphasizeSpecialist = NULL;
	cachedBuildingValues = NULL;

	AI_reset();
}


CvCityAI::~CvCityAI()
{
	AI_uninit();

	SAFE_DELETE_ARRAY(m_aiEmphasizeYieldCount);
	SAFE_DELETE_ARRAY(m_aiEmphasizeCommerceCount);
	SAFE_DELETE_ARRAY(m_aiSpecialYieldMultiplier);
	SAFE_DELETE_ARRAY(m_aiPlayerCloseness);
}


void CvCityAI::AI_init()
{
	AI_reset();

	//--------------------------------
	// Init other game data
	AI_assignWorkingPlots();

/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  11/14/09								jdog5000	  */
/*																							  */
/* City AI, Worker AI																		   */
/************************************************************************************************/
/* original bts code
	AI_updateWorkersNeededHere();

	AI_updateBestBuild();
*/
	AI_updateBestBuild();

	AI_updateWorkersNeededHere();
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
}


void CvCityAI::AI_uninit()
{
	AI_FlushBuildingValueCache();

	SAFE_DELETE_ARRAY(m_pbEmphasize);
	SAFE_DELETE_ARRAY(m_pbEmphasizeSpecialist);
}


// FUNCTION: AI_reset()
// Initializes data members that are serialized.
void CvCityAI::AI_reset()
{
	int iI;

	AI_uninit();

	m_iEmphasizeAvoidGrowthCount = 0;
	m_iEmphasizeGreatPeopleCount = 0;
	m_bForceEmphasizeCulture = false;
/************************************************************************************************/
/* Afforess					  Start		 02/10/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
	m_iEmphasizeAvoidAngryCitizensCount = 0;
	m_iEmphasizeAvoidUnhealthyCitizensCount = 0;
	m_bMilitaryProductionCity = false;
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
	m_bAssignWorkDirty = false;
	m_bChooseProductionDirty = false;

	m_routeToCity.reset();

	m_bestBuildValuesStale = true;

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		m_aiEmphasizeYieldCount[iI] = 0;
	}

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		m_aiEmphasizeCommerceCount[iI] = 0;
	}

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		m_aiBestBuildValue[iI] = NO_BUILD;
	}

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		m_aeBestBuild[iI] = NO_BUILD;
	}

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		m_aiSpecialYieldMultiplier[iI] = 0;
	}
	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		m_aiPlayerCloseness[iI] = 0;
	}
	m_iCachePlayerClosenessTurn = -1;
	m_iCachePlayerClosenessDistance = -1;

	//m_iNeededFloatingDefenders = -1;
	//m_iNeededFloatingDefendersCacheTurn = -1;
	m_neededDefenders = -1;

	m_iWorkersNeeded = 0;
	m_iWorkersHave = 0;

	m_iBuildPriority = CITY_BUILD_PRIORITY_CEILING;
	m_bRequestedUnit = false;
	m_bRequestedBuilding = false;

	FAssertMsg(m_pbEmphasize == NULL, "m_pbEmphasize not NULL!!!");
	FAssertMsg(GC.getNumEmphasizeInfos() > 0,  "GC.getNumEmphasizeInfos() is not greater than zero but an array is being allocated in CvCityAI::AI_reset");
	m_pbEmphasize = new bool[GC.getNumEmphasizeInfos()];
	for (iI = 0; iI < GC.getNumEmphasizeInfos(); iI++)
	{
		m_pbEmphasize[iI] = false;
	}

	FAssertMsg(m_pbEmphasizeSpecialist == NULL, "m_pbEmphasizeSpecialist not NULL!!!");
	FAssertMsg(GC.getNumSpecialistInfos() > 0,  "GC.getNumSpecialistInfos() is not greater than zero but an array is being allocated in CvCityAI::AI_reset");
	m_pbEmphasizeSpecialist = new bool[GC.getNumSpecialistInfos()];
	for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		m_pbEmphasizeSpecialist[iI] = false;
	}

#ifdef YIELD_VALUE_CACHING
	ClearYieldValueCache();
#endif

	m_iCityThreat = -1;
}

//	KOSHLING - initialisation called on every city prior to performing unit mission allocation logic
//	This allows caches that will remain valid for the processing of the current turn's units to be cleared
void CvCityAI::AI_preUnitTurn(void)
{
	PROFILE_FUNC();

	m_neededDefenders = -1;
	m_requestedEscorts = 0;

	m_iCityThreat = -1;

	m_bestUnits.clear();
}

void CvCityAI::AI_noteUnitEscortNeeded(void)
{
	m_requestedEscorts++;
}

void CvCityAI::AI_trained(UnitTypes eUnitType, UnitAITypes eUnitAIType)
{
}

void CvCityAI::AI_doTurn()
{
	PROFILE_FUNC();

	int iI;

	m_buildValueCache.clear();

	if (!isHuman())
	{
		for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
		{
			setForceSpecialistCount(((SpecialistTypes)iI), 0);
		}
	}

	if (!isHuman())
	{
		AI_stealPlots();
	}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  11/14/09								jdog5000	  */
/*																							  */
/* City AI, Worker AI																		   */
/************************************************************************************************/
/* original bts code
	AI_updateWorkersNeededHere();

	AI_updateBestBuild();
*/
	AI_updateBestBuild();

	AI_updateWorkersNeededHere();
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

	//AI_updateRouteToCity();
	m_routeToCityUpdated = false;	//	Update on demand

	if (isHuman())
	{
		if (isProductionAutomated())
		{
			AI_doHurry();
		}
		return;
	}

	AI_doPanic();

	AI_doDraft();

	AI_doHurry();

	AI_doEmphasize();

	//AI_doContractFloatingDefenders();
}


void CvCityAI::AI_doContractFloatingDefenders()
{
	PROFILE_FUNC();

#ifdef USE_UNIT_TENDERING
	int iLargestThreat = 0;
	int iTotalThreat = std::max(1, GET_PLAYER(getOwnerINLINE()).AI_getTotalAreaCityThreat(area(), &iLargestThreat));
	int iOurThreat = AI_cityThreat();

	if( gCityLogLevel >= 2 )
	{
		logBBAIForTeam(getTeam(), "	  City %S has threat level %d (highest %d, total %d)", getName().GetCString(), iOurThreat, iLargestThreat, iTotalThreat);
	}
	//	If below optimal garrison size request extra troops
	//	to move to us with a priority based on our threat level
	//	The highest priority we assign to this is FLOATING_DEFENDER_REQUEST_PRIORITY_MAX
	//	which is below the lowest building priority, and will thus soak up existing
	//	troops without local work without causing new production
	if (!AI_isDefended(0, false) && !m_bAreaSaturatedOfLandMilitaryUnits) // xUPT (dbkblk, 2015-02)
	{
		//	Assess our priority
		int iGarrisonStrength = getGarrisonStrength(false);
		int iMinStrength = AI_minDefenseStrength();
		int iOurPriority;
		int iRequiredStrength;

		//	If we don't even have minimum strength pull in a few defenders at higher priority
		if ( iGarrisonStrength < iMinStrength )
		{
			iOurPriority = CITY_BUILD_PRIORITY_CEILING;
			iRequiredStrength = iMinStrength - iGarrisonStrength;
		}
		else
		{
			iOurPriority = (FLOATING_DEFENDER_REQUEST_PRIORITY_MAX * iOurThreat)/std::max(1,iLargestThreat);
			iRequiredStrength = AI_neededDefenseStrength() - iGarrisonStrength;
		}

		FAssert(iRequiredStrength > 0);

		if( gCityLogLevel >= 2 )
		{
			logBBAIForTeam(getTeam(), "	  City %S requests %d floating defender strength at priority %d", getName().GetCString(), iRequiredStrength, iOurPriority);
		}

		GET_PLAYER(getOwnerINLINE()).getContractBroker().advertiseWork(iOurPriority,
																	   DEFENSIVE_UNITCAPABILITIES,
																	   getX_INLINE(),
																	   getY_INLINE(),
																	   NULL,
																	   (iGarrisonStrength < iMinStrength ? UNITAI_CITY_DEFENSE : NO_UNITAI),
																	   iRequiredStrength);
	}
	else
	{
		if( gCityLogLevel >= 2 )
		{
			logBBAIForTeam(getTeam(), "	  City %S adequately defended", getName().GetCString());
		}
	}
#endif
}


void CvCityAI::AI_assignWorkingPlots()
{
	PROFILE_FUNC();

	CvPlot* pHomePlot;
	int iI;

	if (0 != GC.getDefineINT("AI_SHOULDNT_MANAGE_PLOT_ASSIGNMENT"))
	{
		return;
	}

	//	Koshling - I can't see the point in scrapping the current worker setup
	//	each time (it's very costly in performance terms), and the eventual
	//	AI_juggleCitizens() in what follows will adjust the allocation if priorities
	//	have changed.  Hence changing it to only stop working plots that are
	//	now invalid as worked plots
	//	Empirically it does make a small difference to the eventual assignments
	//	but it seems to be a neither-here-nor-there difference between two approximations
	//	to optimality - better to choose the low-overhead one!
	// remove all assigned plots if we automated

	// Afforess - re-enabled old BTS evaluation. While it may be time consuming, it
	// is important that the AI can re-evaluate all of it's tile choices. Without this
	// The AI may be locked into mediocre, but not quite terrible tile strategies.
	// Without wiping the working tiles out, the AI only removes particularly bad
	// workers from tiles, leaving mediocre and good workers on the remaining tiles, and
	// re-assigns the bad workers to better tiles. Normally this is ok, but whether a tile
	// is considered 'good' or not varies dramatically on whether the city is producing
	// a unit that is produced with food. Units produced with food radically alter tile values
	// and cause AI's to value hammers over growth. The effect is that after the unit is produced
	// and without reseting all tiles here, the AI may continue to be locked into these
	// mediocre-growth strategies that were previously decent.
//#ifdef OLD_BTS_EVALUATION
	if (!isHuman() || isCitizensAutomated())
	{
		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			setWorkingPlot(iI, false);
		}
	}
//#endif
	//update the special yield multiplier to be current
	AI_updateSpecialYieldMultiplier();

	//	If we're in disorder no need to go through all of this, but we must call updateCommerce()
	//	directly to have the disorder take effect on the commerce of this city
	if ( isDisorder() )
	{
		setCommerceDirty();
	}
	else
	{
		// remove any plots we can no longer work for any reason
		verifyWorkingPlots();

		// if forcing specialists, try to make all future specialists of the same type
		bool bIsSpecialistForced = false;
		int iTotalForcedSpecialists = 0;

		// make sure at least the forced amount of specialists are assigned
		for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
		{
			int iForcedSpecialistCount = getForceSpecialistCount((SpecialistTypes)iI);
			if (iForcedSpecialistCount > 0)
			{
				bIsSpecialistForced = true;
				iTotalForcedSpecialists += iForcedSpecialistCount;
			}

			if (!isHuman() || isCitizensAutomated() || (getSpecialistCount((SpecialistTypes)iI) < iForcedSpecialistCount))
			{
				setSpecialistCount(((SpecialistTypes)iI), iForcedSpecialistCount);
			}
		}

		// if we have more specialists of any type than this city can have, reduce to the max
		for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
		{
			if (!isSpecialistValid((SpecialistTypes)iI))
			{
				if (getSpecialistCount((SpecialistTypes)iI) > getMaxSpecialistCount((SpecialistTypes)iI))
				{
					setSpecialistCount(((SpecialistTypes)iI), getMaxSpecialistCount((SpecialistTypes)iI));
				}
			}
		}

		// always work the home plot (center)
		pHomePlot = getCityIndexPlot(CITY_HOME_PLOT);
		if (pHomePlot != NULL)
		{
			setWorkingPlot(CITY_HOME_PLOT, ((getPopulation() > 0) && canWork(pHomePlot)));
		}

		// keep removing the worst citizen until we are not over the limit
		while (extraPopulation() < 0)
		{
			if (!AI_removeWorstCitizen())
			{
				FAssert(false);
				break;
			}
		}

		// extraSpecialists() is less than extraPopulation()
		FAssertMsg(extraSpecialists() >= 0, "extraSpecialists() is expected to be non-negative (invalid Index)");

		// do we have population unassigned
		while (extraPopulation() > 0)
		{
			// (AI_addBestCitizen now handles forced specialist logic)
			if (!AI_addBestCitizen(/*bWorkers*/ true, /*bSpecialists*/ true))
			{
				break;
			}
		}

		// if forcing specialists, assign any other specialists that we must place based on forced specialists
		int iInitialExtraSpecialists = extraSpecialists();
		int iExtraSpecialists = iInitialExtraSpecialists;
		if (bIsSpecialistForced && iExtraSpecialists > 0)
		{
			FAssertMsg(iTotalForcedSpecialists > 0, "zero or negative total forced specialists");
			for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
			{
				if (isSpecialistValid((SpecialistTypes)iI, 1))
				{
					int iForcedSpecialistCount = getForceSpecialistCount((SpecialistTypes)iI);
					if (iForcedSpecialistCount > 0)
					{
						int iSpecialistCount = getSpecialistCount((SpecialistTypes)iI);
						int iMaxSpecialistCount = getMaxSpecialistCount((SpecialistTypes)iI);

						int iSpecialistsToAdd = ((iInitialExtraSpecialists * iForcedSpecialistCount) + (iTotalForcedSpecialists/2)) / iTotalForcedSpecialists;
						if (iExtraSpecialists < iSpecialistsToAdd)
						{
							iSpecialistsToAdd = iExtraSpecialists;
						}

						iSpecialistCount += iSpecialistsToAdd;
						iExtraSpecialists -= iSpecialistsToAdd;

						// if we cannot fit that many, then add as many as we can
						if (iSpecialistCount > iMaxSpecialistCount && !GET_PLAYER(getOwnerINLINE()).isSpecialistValid((SpecialistTypes)iI))
						{
							iExtraSpecialists += iSpecialistCount - iMaxSpecialistCount;
							iSpecialistCount = iMaxSpecialistCount;
						}

						setSpecialistCount((SpecialistTypes)iI, iSpecialistCount);
					}
				}
			}
		}
		FAssertMsg(iExtraSpecialists >= 0, "added too many specialists");

		// if we still have population to assign, assign specialists
		while (extraSpecialists() > 0)
		{
			if (!AI_addBestCitizen(/*bWorkers*/ false, /*bSpecialists*/ true))
			{
				break;
			}
		}

		// if automated, look for better choices than the current ones
		if (!isHuman() || isCitizensAutomated())
		{
			AI_juggleCitizens();
		}

		// at this point, we should not be over the limit
		FAssert((getWorkingPopulation() + getSpecialistPopulation()) <= (totalFreeSpecialists() + getPopulation()));
	}

	AI_setAssignWorkDirty(false);

	if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && isCitySelected())
	{
		gDLL->getInterfaceIFace()->setDirty(CitizenButtons_DIRTY_BIT, true);
	}
}


void CvCityAI::AI_updateAssignWork()
{
	if (AI_isAssignWorkDirty())
	{
		//	Don't mess with plot assignments while the user is in the city screen as
		//	current assignments might be transitory
		if ( !GC.getGameINLINE().isNetworkMultiPlayer() && isHuman() && gDLL->getInterfaceIFace()->isCityScreenUp() )
		{
			//	Also assume when they leave the city screen they will leave a state
			//	they are happy with
			AI_setAssignWorkDirty(false);
			return;
		}

		//Afforess: when updateAssignWork is called, the happy/health level of the city has probably changed
		//which alters the yield values. The cache must be refreshed.
		ClearYieldValueCache();

		AI_assignWorkingPlots();
	}
}


bool CvCityAI::AI_avoidGrowth()
{
	PROFILE_FUNC();

	if (isFoodProduction())
	{
		return true;
	}

	if ( isHuman() && !isProductionAutomated() )
	{
		return false;
	}

	if (AI_isEmphasizeAvoidGrowth())
	{
		return true;
	}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  05/14/10								jdog5000	  */
/*																							  */
/* City AI																					  */
/************************************************************************************************/
/* original code
	if (!AI_isEmphasizeYield(YIELD_FOOD) && !AI_isEmphasizeGreatPeople())
*/
	// AI should avoid growth when it has angry citizens, even if emphasizing great people
	if( !(isHuman()) || (!AI_isEmphasizeYield(YIELD_FOOD) && !AI_isEmphasizeGreatPeople()) )
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
	{
		int iExtra = (isHuman()) ? 0 : 1;
		int iHappinessLevel = happyLevel() - unhappyLevel(iExtra);

		// ignore military unhappy, since we assume it will be fixed quickly, grow anyway
		if (getMilitaryHappinessUnits() == 0)
		{
			iHappinessLevel += ((GC.getDefineINT("NO_MILITARY_PERCENT_ANGER") * (getPopulation() + 1)) / GC.getPERCENT_ANGER_DIVISOR());
		}

/************************************************************************************************/
/* REVOLUTION_MOD						 09/05/09								jdog5000	  */
/*																							  */
/* Revolution AI																				*/
/************************************************************************************************/
		if(!GC.getGameINLINE().isOption(GAMEOPTION_NO_REVOLUTION))
		{
			iHappinessLevel -= std::min(getRevolutionIndex()/600, 2);
		}
		else
		{
			// if we can pop rush, we want to grow one over the cap
			if (GET_PLAYER(getOwnerINLINE()).canPopRush())
			{
				iHappinessLevel++;
			}
		}
/************************************************************************************************/
/* REVOLUTION_MOD						  END												  */
/************************************************************************************************/

		// if we have angry citizens
		if (iHappinessLevel < 0)
		{
			return true;
		}
	}
/************************************************************************************************/
/* Afforess					  Start		 02/10/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
	if (AI_isEmphasizeAvoidAngryCitizens())
	{
		if (unhappyLevel() > happyLevel())
		{
			return true;
		}

		if (getFoodTurnsLeft() == 1)
		{
			int iHappyFacesLeft = happyLevel() - unhappyLevel(getHappinessTimer() > 0 ? GC.getDefineINT("TEMP_HAPPY") : 0);

			if (iHappyFacesLeft < 1)
			{
				return true;
			}
		}
	}

	if (AI_isEmphasizeAvoidUnhealthyCitizens())
	{
		if (badHealth() > goodHealth())
		{
			return true;
		}

		if (getFoodTurnsLeft() == 1)
		{
			int iHealthyFacesLeft = goodHealth() - badHealth();

			if (iHealthyFacesLeft < 1)
			{
				return true;
			}
		}
	}
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
	return false;
}


bool CvCityAI::AI_ignoreGrowth()
{
	PROFILE_FUNC();

	if (!AI_isEmphasizeYield(YIELD_FOOD) && !AI_isEmphasizeGreatPeople())
	{
		if (!AI_foodAvailable((isHuman()) ? 0 : 1))
		{
			return true;
		}
	}

	return false;
}


int CvCityAI::AI_specialistValue(SpecialistTypes eSpecialist, bool bAvoidGrowth, bool bRemove)
{
	PROFILE_FUNC();

	short aiYields[NUM_YIELD_TYPES];
	int iTempValue;
	int iGreatPeopleRate;
	int iValue;
	int iI, iJ;
	int iNumCities = GET_PLAYER(getOwnerINLINE()).getNumCities();

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aiYields[iI] = GET_PLAYER(getOwnerINLINE()).specialistYield(eSpecialist, ((YieldTypes)iI));
	}

	short int aiCommerceYields[NUM_COMMERCE_TYPES];

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		aiCommerceYields[iI] = GET_PLAYER(getOwnerINLINE()).specialistCommerce(eSpecialist, ((CommerceTypes)iI));
	}

	iValue = AI_yieldValue(aiYields, aiCommerceYields, bAvoidGrowth, bRemove);

	iGreatPeopleRate = GC.getSpecialistInfo(eSpecialist).getGreatPeopleRateChange();

	int iEmphasisCount = 0;
	if (iGreatPeopleRate != 0)
	{
		int iGPPValue = 4;
		if (AI_isEmphasizeGreatPeople())
		{
			iGPPValue = isHuman() ? 30 : 20;
		}
		else
		{
			if (AI_isEmphasizeYield(YIELD_COMMERCE))
			{
				iGPPValue = 2;
				iEmphasisCount++;
			}
			if (AI_isEmphasizeYield(YIELD_PRODUCTION))
			{
				iGPPValue = 1;
				iEmphasisCount++;
			}
			if (AI_isEmphasizeYield(YIELD_FOOD))
			{
				iGPPValue = 1;
				iEmphasisCount++;
			}
		}

		//iGreatPeopleRate = ((iGreatPeopleRate * getTotalGreatPeopleRateModifier()) / 100);
		// UnitTypes iGreatPeopleType = (UnitTypes)GC.getSpecialistInfo(eSpecialist).getGreatPeopleUnitClass();

/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  12/06/09								jdog5000	  */
/*																							  */
/* City AI																					  */
/************************************************************************************************/
		// Scale up value for civs/civics with bonuses
		iGreatPeopleRate *= (100 + GET_PLAYER(getOwnerINLINE()).getGreatPeopleRateModifier());
		iGreatPeopleRate /= 100;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

		iTempValue = (iGreatPeopleRate * iGPPValue);

//		if (isHuman() && (getGreatPeopleUnitRate(iGreatPeopleType) == 0)
//			&& (getForceSpecialistCount(eSpecialist) == 0) && !AI_isEmphasizeGreatPeople())
//		{
//			iTempValue -= (iGreatPeopleRate * 4);
//		}

		if (!isHuman() || AI_isEmphasizeGreatPeople())
		{
			int iProgress = getGreatPeopleProgress();
			if (iProgress > 0)
			{
				int iThreshold = GET_PLAYER(getOwnerINLINE()).greatPeopleThreshold();
				iTempValue += ((iGreatPeopleRate * (isHuman() ? 1 : 4) * iGPPValue * iProgress * iProgress) / (iThreshold * iThreshold));
			}
		}

		int iCurrentEra = GET_PLAYER(getOwnerINLINE()).getCurrentEra();
		int iTotalEras = GC.getNumEraInfos();

/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  03/08/10								jdog5000	  */
/*																							  */
/* Victory Strategy AI																		  */
/************************************************************************************************/
		if (GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2))
		{
			int iUnitClass = GC.getSpecialistInfo(eSpecialist).getGreatPeopleUnitClass();
			//FAssert(iUnitClass != NO_UNITCLASS);
/************************************************************************************************/
/* Afforess					  Start		 01/18/10											   */
/************************************************************************************************/
			if (iUnitClass != NO_UNITCLASS)
			{
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/

			UnitTypes eGreatPeopleUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitClass);
			if (eGreatPeopleUnit != NO_UNIT)
			{
				CvUnitInfo& kUnitInfo = GC.getUnitInfo(eGreatPeopleUnit);
				if (kUnitInfo.getGreatWorkCulture() > 0)
				{
					iTempValue += kUnitInfo.getGreatWorkCulture() / ((GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3)) ? 200 : 350);
				}
			}
/************************************************************************************************/
/* Afforess					  Start		 01/18/10											   */
/************************************************************************************************/
			}
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

		if (!isHuman() && (iCurrentEra <= ((iTotalEras * 2) / 3)))
		{
			// try to spawn a prophet for any shrines we have yet to build
			bool bNeedProphet = false;
			int iBestSpreadValue = 0;


			for (iJ = 0; iJ < GC.getNumReligionInfos(); iJ++)
			{
				ReligionTypes eReligion = (ReligionTypes) iJ;

				if (isHolyCity(eReligion) && !hasShrine(eReligion)
					&& ((iCurrentEra < iTotalEras / 2) || GC.getGameINLINE().countReligionLevels(eReligion) >= 10))
				{
					CvCivilizationInfo* pCivilizationInfo = &GC.getCivilizationInfo(getCivilizationType());

					int iUnitClass = GC.getSpecialistInfo(eSpecialist).getGreatPeopleUnitClass();
					//FAssert(iUnitClass != NO_UNITCLASS);
/************************************************************************************************/
/* Afforess					  Start		 01/18/10											   */
/************************************************************************************************/
					if (iUnitClass != NO_UNITCLASS)
					{
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
					UnitTypes eGreatPeopleUnit = (UnitTypes) pCivilizationInfo->getCivilizationUnits(iUnitClass);
					if (eGreatPeopleUnit != NO_UNIT)
					{
						// note, for normal XML, this count will be one (there is only 1 shrine building for each religion)
						int	shrineBuildingCount = GC.getGameINLINE().getShrineBuildingCount(eReligion);
						for (int iI = 0; iI < shrineBuildingCount; iI++)
						{
							int eBuilding = (int) GC.getGameINLINE().getShrineBuilding(iI, eReligion);

							// if this unit builds or forceBuilds this building
							if (GC.getUnitInfo(eGreatPeopleUnit).getBuildings(eBuilding) || GC.getUnitInfo(eGreatPeopleUnit).getForceBuildings(eBuilding))
							{
								bNeedProphet = true;
								iBestSpreadValue = std::max(iBestSpreadValue, GC.getGameINLINE().countReligionLevels(eReligion));
							}
						}
					}
/************************************************************************************************/
/* Afforess					  Start		 01/18/10											   */
/************************************************************************************************/
					}
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
				}
			}

			if (bNeedProphet)
			{
				iTempValue += ((iGreatPeopleRate * iBestSpreadValue));
			}
		}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  12/06/09								jdog5000	  */
/*																							  */
/* Bugfix, City AI																			  */
/************************************************************************************************/
/* original BTS code
		iTempValue *= 100;
*/
		// Scale up value for civs/civics with bonuses
		iTempValue *= getTotalGreatPeopleRateModifier();
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
		iTempValue /= GET_PLAYER(getOwnerINLINE()).AI_averageGreatPeopleMultiplier();

		iTempValue /= (1 + iEmphasisCount);
		iValue += iTempValue;
	}
	else
	{
		SpecialistTypes eGenericCitizen = (SpecialistTypes) GC.getDefineINT("DEFAULT_SPECIALIST");

		// are we the generic specialist?
		if (eSpecialist == eGenericCitizen)
		{
			iValue *= 60;
			iValue /= 100;
		}
	}

	int iExperience = GC.getSpecialistInfo(eSpecialist).getExperience();
	if (0 != iExperience)
	{
		int iProductionRank = findYieldRateRank(YIELD_PRODUCTION);
		int iHasMetCount = GET_TEAM(getTeam()).getHasMetCivCount(true);

		iValue += (iExperience * ((iHasMetCount > 0) ? 4 : 2));
		if (iProductionRank <= iNumCities/2 + 1)
		{
			iValue += iExperience *  4;
		}
		iValue += ((getMilitaryProductionModifier() * iExperience * 8) / 100);
	}

/************************************************************************************************/
/* Specialists Enhancements, by Supercheese 10/12/09											*/
/*																							  */
/*																							  */
/************************************************************************************************/
	int iSpecialistHealth = GC.getSpecialistInfo(eSpecialist).getHealthPercent() / 100;
	int iSpecialistHappiness = GC.getSpecialistInfo(eSpecialist).getHappinessPercent() / 100;
	int iHappinessLevel = happyLevel() - unhappyLevel(1) + getEspionageHappinessCounter();
	int iHealthLevel = goodHealth() - badHealth(/*bNoAngry*/ false, std::max(0, (iHappinessLevel + 1) / 2)) + getEspionageHealthCounter();
	int iBaseFoodDifference = getYieldRate(YIELD_FOOD) - getPopulation()*GC.getFOOD_CONSUMPTION_PER_POPULATION() - std::max(0,-iHealthLevel);

	if (iSpecialistHealth != 0)
	{
		iValue += healthValue(iSpecialistHealth, iHappinessLevel - (getEspionageHappinessCounter() / 2) + std::max(0, iSpecialistHappiness), iHealthLevel, iBaseFoodDifference);
	}

	if (iSpecialistHappiness != 0)
	{
		iValue += happynessValue(iSpecialistHappiness, iHappinessLevel, iHealthLevel);
	}

	int iMaxPop = AI_maxTheoriticalPopulation();
	if (getPopulation() < iMaxPop)
	{
		iValue *= 75;
		iValue /= 100;
	}
/************************************************************************************************/
/* Specialists Enhancements						  END										*/
/************************************************************************************************/

	//	Koshling - evaluate properties
	int iPropertyValue = 0;

	CvPropertyManipulators* pMani = GC.getSpecialistInfo(eSpecialist).getPropertyManipulators();

	for (int i=0; i < pMani->getNumSources(); i++)
	{
		CvPropertySource* pSource = pMani->getSource(i);

		//	Sources that deliver to the city or the plot are both considered since the city plot diffuses
		//	to the city for most properties anyway
		if (pSource->getType() == PROPERTYSOURCE_CONSTANT &&
			(pSource->getObjectType() == GAMEOBJECT_CITY || pSource->getObjectType() == GAMEOBJECT_PLOT))
		{
			PropertyTypes eProperty = pSource->getProperty();
			int iCurrentSourceSize = getTotalBuildingSourcedProperty(eProperty) + getTotalUnitSourcedProperty(eProperty) + getPropertyNonBuildingSource(eProperty);
			int iNewSourceSize = iCurrentSourceSize + ((CvPropertySourceConstant*)pSource)->getAmountPerTurn(getGameObjectConst());
			int iDecayPercent = getPropertyDecay(eProperty);

			//	Steady state occurs at a level where the decay removes as much per turn as the sources add
			//	Decay can be 0 if the current level is below the threshold at which decay cuts in, so for the
			//	purposes of calculation just treat this as very slow decay
			int	iCurrentSteadyStateLevel = (100*iCurrentSourceSize)/std::max(1,iDecayPercent);
			int	iNewSteadyStateLevel = (100*iNewSourceSize)/std::max(1,iDecayPercent);

			iPropertyValue += getPropertySourceValue(eProperty, iNewSteadyStateLevel - iCurrentSteadyStateLevel)/100;
		}
	}

	iValue += iPropertyValue;

	return AI_isEmphasizeSpecialist(eSpecialist) ? (iValue * 175) : (iValue * 100);
}

bool CvCityAI::AI_changeProduction()
{
	PROFILE_FUNC();

	if (isProduction())
	{
		if (getProduction() > 0)
		{
			CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
			bool bDanger = AI_isDanger();

			if ((getProductionUnitAI() != UNITAI_SETTLE) || !kPlayer.AI_isFinancialTrouble())
			{
				//if we are killing our growth to train this, then finish it.
				if (!bDanger && isFoodProduction())
				{
					if ((area()->getAreaAIType(getTeam()) != AREAAI_DEFENSIVE))
					{
						return false;
					}
				}
			}

			// if nearly done, keep building current item
			int iTurns = 5 * std::max(100, GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent());
			iTurns /= 100;
			if (getProductionTurnsLeft() <= iTurns)
			{
				return false;
			}

			// if building a combat unit, and we have no defenders, keep building it
			UnitTypes eProductionUnit = getProductionUnit();
			if (eProductionUnit != NO_UNIT && !m_bAreaSaturatedOfLandMilitaryUnits)
			{
				if (plot()->getNumDefenders(getOwnerINLINE()) == 0)
				{
					if ((GC.getUnitInfo(eProductionUnit).getCombat() +
					GET_TEAM(getTeam()).getUnitClassStrengthChange((UnitClassTypes)GC.getUnitInfo(eProductionUnit).getUnitClassType())) > 0) // Afforess, 2011-11-06
					{
						return false;
					}
				}
			}

			// if we are building a wonder, do not cancel, keep building it (if no danger)
			BuildingTypes eProductionBuilding = getProductionBuilding();
			if (!bDanger && eProductionBuilding != NO_BUILDING &&
				isLimitedWonderClass((BuildingClassTypes) GC.getBuildingInfo(eProductionBuilding).getBuildingClassType()))
			{
				return false;
			}
		}

		return true;
	}

	return false;
}

void CvCityAI::AI_prepareForTurnProcessing()
{
	m_iNumImprovableBonuses = AI_countNumImprovableBonuses(true, GET_PLAYER(getOwnerINLINE()).getCurrentResearch());
}

void CvCityAI::AI_chooseProduction()
{ 
	// Rework of the production function (dbkblk, Afforess, 2015-03)

	PROFILE_FUNC();

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());

	// Only clear the dirty bit if we actually do a check, multiple items might be queued
	AI_setChooseProductionDirty(false);

	/* Check the situation of the civ and the city before making tests */
	// Bypass if anarchy
	if (kPlayer.isAnarchy())
	{
		return;
	}

	// Is it a city ruled by a governor ?
	bool bInhibitUnits = false;
	if (isHuman() && isProductionAutomated())
	{
		if (!GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_MODDER_3))
		{
			AI_buildGovernorChooseProduction();
			return;
		}
		for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			if (AI_isEmphasizeCommerce((CommerceTypes)iI))
			{
				bInhibitUnits = true;
			}
		}
		for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
		{
			if (AI_isEmphasizeYield((YieldTypes)iI))
			{
				bInhibitUnits = true;
			}
		}
	}

	/* Preparing boolean tests */

	// Geopolitic situation
	CvWString sCityName = this->getName(); // Debug
	CvArea* pArea = area();
	CvArea* pWaterArea = waterArea(true);
	int pWaterAreaSize = 0;
	bool bMaybeWaterArea = false;
	bool bWaterDanger = false;
	if (pWaterArea != NULL)
	{
		pWaterAreaSize = pWaterArea->getNumTiles();
		bMaybeWaterArea = true;
		if (!GET_TEAM(getTeam()).AI_isWaterAreaRelevant(pWaterArea))
		{
			pWaterArea = NULL;
		}

		bWaterDanger = kPlayer.AI_getWaterDanger(plot(), 4) > 0;
	}
	bool bPrimaryArea = kPlayer.AI_isPrimaryArea(pArea);
	int iNumCitiesInArea = pArea->getCitiesPerPlayer(getOwnerINLINE());
	int iNumCitiesPlayer = kPlayer.getNumCities();

	// Economic situation
	bool bImportantCity = false; //be very careful about setting this.
	bool bWasFoodProduction = isFoodProduction();
	bool bFinancialTrouble = kPlayer.AI_isFinancialTrouble();
	bool bBestWorkerIsOneShot = false;
	int iNeededSeaWorkers = (bMaybeWaterArea) ? AI_neededSeaWorkers() : 0;
	int iExistingSeaWorkers = (waterArea(true) != NULL) ? kPlayer.AI_totalWaterAreaUnitAIs(waterArea(true), UNITAI_WORKER_SEA) : 0;

	// War situation
	bool bDanger = AI_isDanger();
	bool bEnemyCloseness = false; // Check if there is a barbarian city close to our territory
	if (GET_PLAYER(BARBARIAN_PLAYER).isAlive())
	{
		int iCloseness = AI_playerCloseness(BARBARIAN_PLAYER, DEFAULT_PLAYER_CLOSENESS);
		if (iCloseness > 0)
		{
			bEnemyCloseness = true;
		}
	}
	bool bMassing = (pArea->getAreaAIType(getTeam()) == AREAAI_ASSAULT_MASSING);
	bool bLandWar = ((pArea->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (pArea->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE) || bMassing);
	bool bAssaultAssist = (pArea->getAreaAIType(getTeam()) == AREAAI_ASSAULT_ASSIST);
	bool bAssault = bAssaultAssist || (pArea->getAreaAIType(getTeam()) == AREAAI_ASSAULT) || bMassing;
	bool bDefenseWar = (pArea->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE);
	int iWarSuccessRatio = GET_TEAM(getTeam()).AI_getWarSuccessCapitulationRatio();
	int iEnemyPowerPerc = GET_TEAM(getTeam()).AI_getEnemyPowerPercent(true);
	int iWarTroubleThreshold = 0;
	bool bMinor = kPlayer.isMinorCiv();
	bool bBarbarian = kPlayer.isBarbarian();
	bool bRebel = kPlayer.isRebel();

	// Units
	int iUnitsScout = kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_EXPLORE);
	int iUnitsSiege = kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_COLLATERAL);
	int iUnitsAttackCity = kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_ATTACK_CITY);
	int iUnitsAttack = kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_ATTACK);
	int iUnitsCounter = kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_COUNTER);
	int iUnitsParadrop = kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_PARADROP);
	int iUnitsCityDefense = plot()->plotCount(PUF_canDefend);
	int iUnitsCityAreaDefense = (int)(kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_CITY_DEFENSE) / iNumCitiesInArea);
	int iUnitsCitySettler = plot()->plotCount(PUF_isUnitAIType, UNITAI_SETTLE, -1, getOwnerINLINE());
	int iUnitsSeaScout = 0;
	int iUnitsSeaAttack = 0;
	int iUnitsSeaCarrier = 0;
	int iUnitsSeaMissileCarrier = 0;
	int iUnitsAircraft = kPlayer.AI_totalUnitAIs(UNITAI_ATTACK_AIR) + kPlayer.AI_totalUnitAIs(UNITAI_DEFENSE_AIR);
	int iUnitsMissiles = kPlayer.AI_totalUnitAIs(UNITAI_MISSILE_AIR);
	int iUnitsNukes = kPlayer.AI_totalUnitAIs(UNITAI_ICBM);
	int iUnitsToTransport = 0;
	iUnitsToTransport += kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_ATTACK);
	iUnitsToTransport += kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_COUNTER);
	int iTransports = 0;
	if (pWaterArea != NULL)
	{
		iUnitsToTransport = iUnitsAttack;
		iUnitsSeaScout = kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_EXPLORE_SEA);
		iTransports = kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_ESCORT_SEA);
		iUnitsSeaAttack = kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_ATTACK_SEA);
		iUnitsSeaCarrier = kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_CARRIER_SEA);
		iUnitsSeaMissileCarrier = kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_MISSILE_CARRIER_SEA);
	}
	int iOptimalOffensiveArmy = (iNumCitiesInArea * 2) + 2; // Define the minimal offensive army the try should try to reach at peace time (4 with 1 city, then 2 by cities)
	int iOptimalSeaArmy = 0;
	if (pWaterArea != NULL)
	{
		iOptimalSeaArmy = iTransports + iExistingSeaWorkers + iUnitsSeaCarrier + iUnitsSeaMissileCarrier + kPlayer.countNumCoastalCitiesByArea(pArea); // A minimal sea army force to make sure the AI is defended
	}
	int iOptimalScout = 1 + (int)(pArea->getNumTiles() / 500); // Determine an optimal scout number
	int iOptimalSeaScout = 0;
	if (pWaterArea != NULL)
	{
		iOptimalSeaScout = 1 + (int)(pWaterAreaSize / 500); // Determine an optimal sea scout number
	}
	
	
	// Culture situation
	int iCulturePressure = AI_calculateCulturePressure();
	int iCultureRateRank = findCommerceRateRank(COMMERCE_CULTURE);
	int iCulturalVictoryNumCultureCities = GC.getGameINLINE().culturalVictoryNumCultureCities();

	// Other
	int iProductionRank;
	m_bRequestedBuilding = false;
	m_bRequestedUnit = false;
	m_iBuildPriority = CITY_BUILD_PRIORITY_CEILING;
	m_iTempBuildPriority = CITY_BUILD_PRIORITY_CEILING;


	// To review
	if (!bLandWar && !bAssault && GET_TEAM(getTeam()).isAVassal())
	{
		bLandWar = GET_TEAM(getTeam()).isMasterPlanningLandWar(area());

		if (!bLandWar)
		{
			bAssault = GET_TEAM(getTeam()).isMasterPlanningSeaWar(area());
		}
	}

	bool bAggressiveAI = GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI);
	bool bAlwaysPeace = GC.getGameINLINE().isOption(GAMEOPTION_ALWAYS_PEACE);

	int iUnitCostPercentage = (kPlayer.calculateUnitCost() * 100) / std::max(1, kPlayer.calculatePreInflatedCosts());
	int iWaterPercent = AI_calculateWaterWorldPercent();

	int iBuildUnitProb = AI_buildUnitProb();

	int iExistingWorkers = kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_WORKER);
	int iNeededWorkers = kPlayer.AI_neededWorkers(pArea);
	int iTargetCulturePerTurn = AI_calculateTargetCulturePerTurn();

	int iAreaBestFoundValue;
	int iNumAreaCitySites = kPlayer.AI_getNumAreaCitySites(getArea(), iAreaBestFoundValue);

	int iWaterAreaBestFoundValue = 0;
	CvArea* pWaterSettlerArea = pWaterArea;
	if (pWaterSettlerArea == NULL)
	{
		pWaterSettlerArea = GC.getMap().findBiggestArea(true);

		if (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterSettlerArea, UNITAI_SETTLER_SEA) == 0)
		{
			pWaterSettlerArea = NULL;
		}
	}
	int iNumWaterAreaCitySites = (pWaterSettlerArea == NULL) ? 0 : kPlayer.AI_getNumAdjacentAreaCitySites(pWaterSettlerArea->getID(), getArea(), iWaterAreaBestFoundValue);
	int iNumSettlers = kPlayer.AI_totalUnitAIs(UNITAI_SETTLE) + GET_PLAYER(getOwnerINLINE()).getContractBroker().numRequestsOutstanding(UNITAI_SETTLE);

	bool bIsCapitalArea = false;
	int iNumCapitalAreaCities = 0;
	if (kPlayer.getCapitalCity() != NULL)
	{
		iNumCapitalAreaCities = kPlayer.getCapitalCity()->area()->getCitiesPerPlayer(getOwnerINLINE());
		if (getArea() == kPlayer.getCapitalCity()->getArea())
		{
			bIsCapitalArea = true;
		}
	}

	int iMaxSettlers = 0;
	if (!bFinancialTrouble && GET_PLAYER(getOwnerINLINE()).bestBuildableUnitForAIType(NO_DOMAIN, UNITAI_SETTLE) != NO_UNIT)
	{
		iMaxSettlers = std::min((kPlayer.getNumCities() + 1) / 2, iNumAreaCitySites + iNumWaterAreaCitySites);
		if (bLandWar || bAssault)
		{
			iMaxSettlers = (iMaxSettlers + 2) / 3;
		}

		if (kPlayer.getCityLimit() > 0)
		{
			if (kPlayer.getCityOverLimitUnhappy() > 0)
			{
				//	Soft limit.  If we already have unhappy cities don't create
				//	settlers that will increase overall unhappiness as they found
				//	new cities
				if (kPlayer.AI_getOverallHappyness() < 0)
				{
					iMaxSettlers = std::min(iMaxSettlers, std::max(0, kPlayer.getCityLimit() - kPlayer.getNumCities()));
				}
			}
			else
			{
				//	Hard limit - don't stockpile more than 2 settlers
				iMaxSettlers = std::min(iMaxSettlers, kPlayer.getCityLimit() - kPlayer.getNumCities() + 2);
			}
		}
	}
	/************************************************************************************************/
	/* Afforess					  Start		 06/29/10											   */
	/*																							  */
	/*																							  */
	/************************************************************************************************/
	bool bDevelopingCity = isDevelopingCity();
	/************************************************************************************************/
	/* Afforess						 END															*/
	/************************************************************************************************/

	int iEconomyFlags = 0;
	int iEconomyFlagBits = 0;

	if (!isHuman() || AI_isEmphasizeYield(YIELD_PRODUCTION))
	{
		iEconomyFlags |= BUILDINGFOCUS_PRODUCTION;
		iEconomyFlagBits++;
	}
	if (!isHuman() || AI_isEmphasizeYield(YIELD_COMMERCE))
	{
		iEconomyFlags |= BUILDINGFOCUS_GOLD;
		iEconomyFlagBits++;
	}
	if (!isHuman() || AI_isEmphasizeCommerce(COMMERCE_RESEARCH))
	{
		iEconomyFlags |= BUILDINGFOCUS_RESEARCH;
		iEconomyFlagBits++;
	}
	if (!bInhibitUnits)	//	This is actually a proxy for the human owner having set no explicit preferences
	{
		iEconomyFlags |= BUILDINGFOCUS_MAINTENANCE;
		iEconomyFlags |= BUILDINGFOCUS_HAPPY;
		iEconomyFlags |= BUILDINGFOCUS_HEALTHY;

		iEconomyFlagBits += 3;
	}
	if (AI_isEmphasizeGreatPeople())
	{
		iEconomyFlags |= BUILDINGFOCUS_SPECIALIST;
		iEconomyFlagBits++;
	}
	if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
	{
		iEconomyFlags |= BUILDINGFOCUS_ESPIONAGE;
		iEconomyFlagBits++;
	}

	//	Normalize threholds using this against the number of bits we are including
	int iEcononmyFlagsThreasholdWeighting;
	iEcononmyFlagsThreasholdWeighting = (100 * iEconomyFlagBits) / 8;

	if (iNumCitiesInArea > 2)
	{
		if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2))
		{
			if (iCultureRateRank <= iCulturalVictoryNumCultureCities + 1)
			{
				// if we do not have enough cities, then the highest culture city will not get special attention
				if (iCultureRateRank > 1 || (kPlayer.getNumCities() > (iCulturalVictoryNumCultureCities + 1)))
				{
					if ((((iNumAreaCitySites + iNumWaterAreaCitySites) > 0) && (kPlayer.getNumCities() < 6)) && (getCitySorenRandNum(2, "AI Less Culture More Expand") == 0))
					{
						bImportantCity = false;
					}
					else
					{
						bImportantCity = true;
					}
				}
			}
		}
	}

	// Free experience for various unit domains
	int iFreeLandExperience = getSpecialistFreeExperience() + getDomainFreeExperience(DOMAIN_LAND);
	// AIAndy: Not used here at the moment
	//int iFreeSeaExperience = getSpecialistFreeExperience() + getDomainFreeExperience(DOMAIN_SEA);
	int iFreeAirExperience = getSpecialistFreeExperience() + getDomainFreeExperience(DOMAIN_AIR);

	clearOrderQueue();

	if (bWasFoodProduction)
	{
		AI_assignWorkingPlots();
	}

	iProductionRank = findYieldRateRank(YIELD_PRODUCTION);

	// K-Mod, military exemption for commerce cities and underdeveloped cities
	bool bInhibitMilitaryUnits = false;
	if (kPlayer.AI_isDoStrategy(AI_STRATEGY_ECONOMY_FOCUS))
	{
		bInhibitMilitaryUnits = true;
	}
	else if (iProductionRank > kPlayer.getNumCities() / 2)
	{
		bool bBelowMedian = true;
		for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			// I'd use the total commerce rank, but there currently isn't a cached value of that.
			int iRank = findCommerceRateRank((CommerceTypes)iI);
			if (iRank < iProductionRank)
			{
				bInhibitMilitaryUnits = true;
				break;
			}
			if (iRank < kPlayer.getNumCities() / 2)
			{
				bBelowMedian = false;
			}
		}

		if (bBelowMedian)
		{
			bInhibitMilitaryUnits = true;
		}
	}
	//Exemptions (Afforess)
	if (isCapital() || kPlayer.getNumCities() < 3)
	{
		bInhibitMilitaryUnits = false;
	}
	if (bBarbarian)
	{
		bInhibitMilitaryUnits = false;
		bInhibitUnits = false;
	}

	// K-Mod end

	if (gCityLogLevel >= 3) logBBAIForTeam(getTeam(), "	  City %S pop %d considering new production: iProdRank %d, iBuildUnitProb %d", getName().GetCString(), getPopulation(), iProductionRank, iBuildUnitProb);

	// AI Strategies
	/* This logic allows to make the City production more in adequation with the leader trait. We consider 60% random and 20% for each trait */
	int iAffinityRandomness = 60;
	int iAffinityThreshold = 50;
	int iAffinityMilitary = getRandNum100() * iAffinityRandomness / 100;
	int iAffinityGrowth = getRandNum100() * iAffinityRandomness / 100;
	int iAffinityHappyness = getRandNum100() * iAffinityRandomness / 100;
	int iAffinityHealth = getRandNum100() * iAffinityRandomness / 100;
	int iAffinityCulture = getRandNum100() * iAffinityRandomness / 100;
	int iAffinityEspionage = getRandNum100() * iAffinityRandomness / 100;
	int iAffinityCommerce = getRandNum100() * iAffinityRandomness / 100;
	int iAffinityWonders = getRandNum100() * iAffinityRandomness / 100;
	int iAffinityScience = getRandNum100() * iAffinityRandomness / 100;
	int iAffinityReligion = getRandNum100() * iAffinityRandomness / 100;

	// Attribute 20% of bonus due to the trait
	int iAffinityTraitValue = 20;
	if (kPlayer.hasTrait((TraitTypes)GC.getInfoTypeForString("TRAIT_AGGRESSIVE")))
	{
		iAffinityMilitary += iAffinityTraitValue;
		iOptimalOffensiveArmy += iNumCitiesInArea; // Add one unit by city to the optimal offensive army
	}
	if (kPlayer.hasTrait((TraitTypes)GC.getInfoTypeForString("TRAIT_AGRICULTURAL")))
	{
		iAffinityGrowth += iAffinityTraitValue;
	}
	if (kPlayer.hasTrait((TraitTypes)GC.getInfoTypeForString("TRAIT_CHARISMATIC")))
	{
		iAffinityHappyness += iAffinityTraitValue;
	}
	if (kPlayer.hasTrait((TraitTypes)GC.getInfoTypeForString("TRAIT_CREATIVE")))
	{
		iAffinityCulture += iAffinityTraitValue;
	}
	if (kPlayer.hasTrait((TraitTypes)GC.getInfoTypeForString("TRAIT_DECEIVER")))
	{
		iAffinityEspionage += iAffinityTraitValue;
	}
	if (kPlayer.hasTrait((TraitTypes)GC.getInfoTypeForString("TRAIT_EXPANSIVE")))
	{
		iAffinityCulture += iAffinityTraitValue;
	}
	if (kPlayer.hasTrait((TraitTypes)GC.getInfoTypeForString("TRAIT_FINANCIAL")))
	{
		iAffinityCommerce += iAffinityTraitValue;
	}
	if (kPlayer.hasTrait((TraitTypes)GC.getInfoTypeForString("TRAIT_HUMANITARIAN")))
	{
		iAffinityHealth += iAffinityTraitValue;
	}
	if (kPlayer.hasTrait((TraitTypes)GC.getInfoTypeForString("TRAIT_IMPERIALIST")))
	{
		iAffinityGrowth += iAffinityTraitValue;
	}
	if (kPlayer.hasTrait((TraitTypes)GC.getInfoTypeForString("TRAIT_INDUSTRIOUS")))
	{
		iAffinityWonders += iAffinityTraitValue;
	}
	if (kPlayer.hasTrait((TraitTypes)GC.getInfoTypeForString("TRAIT_NOMAD")))
	{
		iAffinityEspionage += iAffinityTraitValue;
	}
	if (kPlayer.hasTrait((TraitTypes)GC.getInfoTypeForString("TRAIT_ORGANIZED")))
	{
		iAffinityGrowth += iAffinityTraitValue;
	}
	if (kPlayer.hasTrait((TraitTypes)GC.getInfoTypeForString("TRAIT_PHILOSOPHICAL")))
	{
		iAffinityScience += iAffinityTraitValue;
	}
	if (kPlayer.hasTrait((TraitTypes)GC.getInfoTypeForString("TRAIT_PROTECTIVE")))
	{
		iAffinityMilitary += iAffinityTraitValue;
	}
	if (kPlayer.hasTrait((TraitTypes)GC.getInfoTypeForString("TRAIT_SCIENTIFIC")))
	{
		iAffinityScience += iAffinityTraitValue;
	}
	if (kPlayer.hasTrait((TraitTypes)GC.getInfoTypeForString("TRAIT_SEAFARING")))
	{
		iAffinityCommerce += iAffinityTraitValue;
	}
	if (kPlayer.hasTrait((TraitTypes)GC.getInfoTypeForString("TRAIT_SPIRITUAL")))
	{
		iAffinityReligion += iAffinityTraitValue;
	}
	
	if (bFinancialTrouble)
	{
		iOptimalOffensiveArmy /= 3;
		iOptimalSeaArmy /= 3;
	}
	
	if (gCityLogLevel >= 3) logBBAIForTeam(getTeam(), "	  City %S pop %d has iAffinityMilitary %d, iAffinityGrowth %d, iAffinityHappyness %d, iAffinityHealth %d, iAffinityCulture %d, iAffinityEspionage %d, iAffinityCommerce %d, iAffinityWonders %d, iAffinityScience %d, iAffinityReligion %d", getName().GetCString(), getPopulation()
							, iAffinityMilitary, iAffinityGrowth, iAffinityHappyness, iAffinityHealth, iAffinityCulture, iAffinityEspionage, iAffinityCommerce, iAffinityScience, iAffinityReligion);

	/* Start of the production conditions */
	bool bEarlyGameMilitary = (kPlayer.getCurrentEra() < 2 && iNumCitiesPlayer < 3); // Prevent the early AI to build only military units. The threshold is increased to have a real low chance of creating military units (except for agressive or protective traits)

	/* War time */ // Preparing a war plan or defending is more important than anything else. The units production isn't related to danger because it would defeat the early start of AI. Instead try to slowly reach an optimal number of units and max out when there is a war or a warplan to really prepare!
	// Minimal defense: At any time we should have at least the minimal city defenders (always!), some counter units and some attack units.
	if ((iUnitsCityDefense < 2) && !m_bAreaSaturatedOfLandMilitaryUnits) // WARNING: No protection
	{
		if (AI_chooseUnit("City defense: very low defense (iUnitsCityDefense < 2)", UNITAI_CITY_DEFENSE))
		{
			return;
		}
	}

	// 2nd defender with a lower priority (40%). Avoid this when minor as it cause an endless flow of archers.	//45deg: lowered probability to 20% because of increased units costs
	if ((getRandNum100() > 79 && (getPopulation() > 3 || iNumCitiesInArea < 2 || isCapital())) && !bMinor && !bEarlyGameMilitary)
	{
		if ((iUnitsCityAreaDefense < (AI_minDefenders() * iNumCitiesInArea)) && !m_bAreaSaturatedOfLandMilitaryUnits)
		{
			if (AI_chooseUnit("City defense: defense unit (iUnitsCityAreaDefense < (AI_minDefenders)", UNITAI_CITY_DEFENSE))
			{
				return;
			}
		}
	}
	
		//45deg: moving up Air Units and Nukes priority - START
		/* Extra-military units */
	if (!bInhibitUnits)
	{
		// Nukes
		if (iAffinityMilitary > (iAffinityThreshold/2)) //45deg: cut in half iAffinityThreshold when considering building nukes. There are so many other restrictions on nukes production that too few nukes are being built otherwise; if there's a chance to build nukes, AI should do so
		{
			// ICBM and carriers (Afforess reduced 12 -> 6, since AI rarely reaches this logic, Added exemption for MAD players) //45deg: further reduced 6 -> 3 (and 1) because there are many other limitation for nukes production
			int chance = !GC.getGameINLINE().isOption(GAMEOPTION_RUTHLESS_AI) ? 3 : 1;
			if (!bInhibitMilitaryUnits && !bInhibitUnits && !bAlwaysPeace && (kPlayer.AI_isDoStrategy(AI_STRATEGY_OWABWNW) || kPlayer.isEnabledMAD() || (getCitySorenRandNum(chance, "AI consider Nuke") == 0)))
			{
				if (!bFinancialTrouble)
				{
					int iNukesWanted = 1 + 2 * std::min(kPlayer.getNumCities(), GC.getGame().getNumCities() - kPlayer.getNumCities());
					//Afforess rolling randoms twice makes getting nukes much harder
					if ((iUnitsNukes < iNukesWanted)/* && (getCitySorenRandNum(100, "AI train nuke MWAHAHAH") < (90 - (80 * iTotalNukes) / iNukesWanted))*/)
					{
						//Reordered, because nukes are more valuable than carriers
						if (AI_chooseUnit("ICBM (2)", UNITAI_ICBM))
						{
							return;
						}
					}
				}
			}
		}
		m_iTempBuildPriority--;

		// Aircraft
		int iMaxUnitSpending = (bAggressiveAI ? 6 : 3) + iBuildUnitProb / 3;

		if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST4))
		{
			iMaxUnitSpending += 7;
		}
		else if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST3) || kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_DOMINATION3))
		{
			iMaxUnitSpending += 3;
		}
		else if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST1))
		{
			iMaxUnitSpending += 1;
		}

		if (bAlwaysPeace)
		{
			iMaxUnitSpending = -10;
		}
		else if (kPlayer.AI_isDoStrategy(AI_STRATEGY_FINAL_WAR))
		{
			iMaxUnitSpending = 5 + iMaxUnitSpending + (100 - iMaxUnitSpending) / 2;
		}
		else
		{
			iMaxUnitSpending += bDefenseWar ? 4 : 0;
			switch (pArea->getAreaAIType(getTeam()))
			{
			case AREAAI_OFFENSIVE:
				iMaxUnitSpending += 5;
				break;

			case AREAAI_DEFENSIVE:
				iMaxUnitSpending += 10;
				break;

			case AREAAI_MASSING:
				iMaxUnitSpending += 25;
				break;

			case AREAAI_ASSAULT:
				iMaxUnitSpending += 8;
				break;

			case AREAAI_ASSAULT_MASSING:
				iMaxUnitSpending += 16;
				break;

			case AREAAI_ASSAULT_ASSIST:
				iMaxUnitSpending += 6;
				break;

			case AREAAI_NEUTRAL:
				break;
			default:
				FAssert(false);
			}
		}

		if (!bInhibitMilitaryUnits && !bInhibitUnits
			&& iUnitCostPercentage < iMaxUnitSpending + 15 && !bFinancialTrouble)
		{	// f1rpo: I think reusing iMaxUnitSpending is intended here
			/*int*/ iMaxUnitSpending = (bAggressiveAI ? 6 : 3) + iBuildUnitProb / 3;

			if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST4))
			{
				iMaxUnitSpending += 7;
			}
			else if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST3) || kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_DOMINATION3))
			{
				iMaxUnitSpending += 3;
			}
			else if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST1))
			{
				iMaxUnitSpending += 1;
			}

			//Afforess moved AI Aircraft logic up, Aircraft >> Sea transports
			UnitTypeWeightArray airUnitTypes;

			int iAircraftNeed = 0;
			UnitTypes eBestAttackAircraft = NO_UNIT;
			UnitTypes eBestMissile = NO_UNIT;

			if (!bInhibitMilitaryUnits && !bInhibitUnits
				&& iUnitCostPercentage < iMaxUnitSpending + 15 && (!bImportantCity || bDefenseWar))
			{
				if (bLandWar || bAssault || (iFreeAirExperience > 0) || (getCitySorenRandNum(2, "AI train air") == 0))
				{
					int iBestAirValue = kPlayer.AI_bestCityUnitAIValue(UNITAI_ATTACK_AIR, this, &eBestAttackAircraft);
					int iBestMissileValue = kPlayer.AI_bestCityUnitAIValue(UNITAI_MISSILE_AIR, this, &eBestMissile);
					if ((iBestAirValue + iBestMissileValue) > 0)
					{
						
						if (NO_UNIT != eBestAttackAircraft)
						{
							iAircraftNeed = (15 + kPlayer.getNumCities() * (3 * GC.getUnitInfo(eBestAttackAircraft).getAirCombat())) / (std::max(1, GC.getGame().getBestLandUnitCombat()));
							int iBestDefenseValue = kPlayer.AI_bestCityUnitAIValue(UNITAI_DEFENSE_AIR, this);
							if ((iBestDefenseValue > 0) && (iBestAirValue > iBestDefenseValue))
							{
								iAircraftNeed *= 3;
								iAircraftNeed /= 2;
							}
						}
						if (iBestMissileValue > 0)
						{
							iAircraftNeed = std::max(iAircraftNeed, 1 + kPlayer.getNumCities() / 2);
						}

						bool bAirBlitz = kPlayer.AI_isDoStrategy(AI_STRATEGY_AIR_BLITZ);
						bool bLandBlitz = kPlayer.AI_isDoStrategy(AI_STRATEGY_LAND_BLITZ);
						if (bAirBlitz)
						{
							iAircraftNeed *= 3;
							iAircraftNeed /= 2;
						}
						else if (bLandBlitz)
						{
							iAircraftNeed /= 2;
							iAircraftNeed += 1;
						}

						airUnitTypes.push_back(std::make_pair(UNITAI_ATTACK_AIR, bAirBlitz ? 125 : 80));
						airUnitTypes.push_back(std::make_pair(UNITAI_DEFENSE_AIR, bLandBlitz ? 100 : 100));
						if (iBestMissileValue > 0)
						{
							airUnitTypes.push_back(std::make_pair(UNITAI_MISSILE_AIR, bAssault ? 60 : 40));
						}

						airUnitTypes.push_back(std::make_pair(UNITAI_ICBM, 20));

						if ((iUnitsAircraft + iUnitsMissiles) * 2 < iAircraftNeed)
						{
							if (AI_chooseLeastRepresentedUnit("need airforce", airUnitTypes))
							{
								return;
							}
						}
						// Additional check for air defenses
						int iFightersHave = kPlayer.AI_totalUnitAIs(UNITAI_DEFENSE_AIR);

						if (3 * iFightersHave < iAircraftNeed)
						{
							if (AI_chooseUnit("need air defense", UNITAI_DEFENSE_AIR))
							{
								return;
							}
						}
					}
				}
			}

			m_iTempBuildPriority--;

			if (((iUnitsAircraft + iUnitsMissiles) * 2 >= iAircraftNeed) && ((iUnitsAircraft + iUnitsMissiles) < iAircraftNeed))
			{
				int iOdds = 33;

				if (iFreeAirExperience > 0 || (iProductionRank <= (1 + kPlayer.getNumCities() / 2)))
				{
					iOdds = -1;
				}

				if (AI_chooseLeastRepresentedUnit("air units", airUnitTypes, iOdds))
				{
					return;
				}
			}
		}

		m_iTempBuildPriority--;
	}
	
	//45deg: moving up Air Units and Nukes priority - END
	
	// Resolution of health problems //45deg: moving up health section to avoid starvation
	int iHealth = goodHealth() - badHealth(true, 0);
	int iFoodDiffBase = foodDifference(false, false, true);

	//	If there is negative health address it if we can do so quickly (relative to the amount of ill health)
	//	Koshling - don't waste time with this if we have a food excess that is being trimmed by less than 20%
	//	by the ill health (rounded - so at less than 5 net food we always consider this)
	if (iHealth < 0 && -iHealth > iFoodDiffBase / 5)
	{
		//	Koshling - reduce the time we're prepared to spend on this by the current food excess (if any)
		if (AI_chooseBuilding(BUILDINGFOCUS_HEALTHY, std::max(1, -iHealth - std::max(0, iFoodDiffBase)), 0, -1, true))
		{
			if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S is unhealthy, will build a health focused building (peace)", getName().GetCString());
			return;
		}
	}

	m_iTempBuildPriority--;	

	//45deg - Boosting projects in late game - START
	//		  adding this part for late game when building SS parts might be very important
	if ((kPlayer.getCurrentEra() >= 5) && !bMinor && !bBarbarian && (GC.getGame().getSorenRandNum(std::max(30, (GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getBuildingClassPrereqModifier()) + 100 - iNumCitiesPlayer*10), "Create Project")) == 0)  //chance that AI will choose to create a project
	{
		bool bSpaceVictoryEnabled = false;
		//Find the space race victory	
		ProjectTypes eSpaceship = NO_PROJECT;
		for (int iM = 0; iM < GC.getNumProjectInfos(); iM++)
		{
			if (GC.getProjectInfo((ProjectTypes)iM).getVictoryPrereq() != NO_VICTORY)
			{
				eSpaceship = (ProjectTypes)iM;
				break;
			}
		}	
		if (eSpaceship != NO_PROJECT)
		{
			bSpaceVictoryEnabled = GC.getGameINLINE().isVictoryValid((VictoryTypes)GC.getProjectInfo(eSpaceship).getVictoryPrereq());
		}	
		// if Mastery or space race victories are enabled, check if you can create a project now; 
		for (int iL = 0; iL < GC.getNumVictoryInfos(); iL++)
		{
			if (GC.getGameINLINE().isVictoryValid((VictoryTypes)iL))
			{
				if (GC.getVictoryInfo((VictoryTypes)iL).isTotalVictory() || bSpaceVictoryEnabled)
				{
					if (AI_chooseProject())
					{
						if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S will create a project", getName().GetCString());
						return;
					}
				}
			}
		}
	}
	//45deg - Boosting projects in late game - END
	
	// Try to reach an optimal army but don't make it straight to not harm development. 20% of chance if no enemy neighbour and 50% if barbarians next to our city
	int iRandNumber = getRandNum100();
	if (((!bEnemyCloseness && iRandNumber > 79) || (bEnemyCloseness && iRandNumber > 49)) && ((getPopulation() > 3 && iNumCitiesInArea > 1) || isCapital()) && !bInhibitMilitaryUnits && !bEarlyGameMilitary)
	{
		if ((iUnitsCityAreaDefense < (AI_minDefenders() * iNumCitiesInArea)) && !m_bAreaSaturatedOfLandMilitaryUnits) // Less urgent need. Only defense units don't check the financial trouble.
		{
			if (AI_chooseUnit("Reserve army: City defense (iUnitsCityAreaDefense < (AI_minDefenders).", UNITAI_CITY_DEFENSE))
			{
				return;
			}
		}
		if ((iUnitsCounter < iNumCitiesInArea) && !bFinancialTrouble && !m_bAreaSaturatedOfLandMilitaryUnits) // At least a counter unit by city
		{
			if (AI_chooseUnit("Reserve army: Counter (iUnitsCounter < iNumCitiesInArea)", UNITAI_COUNTER))
			{
				return;
			}
		}
		if (((iUnitsAttack + iUnitsAttackCity) < iOptimalOffensiveArmy - kPlayer.countNumCoastalCitiesByArea(pArea)) && !bFinancialTrouble && !m_bAreaSaturatedOfLandMilitaryUnits) // Prepare a floating army if have some money
		{
			if (getRandNum100() > 50)
			{
				if (AI_chooseUnit("Reserve army: Attack City (iUnitsAttack < iOptimalOffensiveArmy)", UNITAI_ATTACK_CITY))
				{
					return;
				}
			}
			if (AI_chooseUnit("Reserve army: Attack (iUnitsAttack < iOptimalOffensiveArmy)", UNITAI_ATTACK))
			{
				return;
			}
		}
		if ((iUnitsSeaAttack < iOptimalSeaArmy) && !bFinancialTrouble) // Build some assault boat from time to time or if we are below the optimal sea army (+ the number of cities in the continental area)
		{
			if (AI_chooseUnit("Reserve army: Sea attack (iUnitsSeaAttack < iOptimalSeaArmy)", UNITAI_ATTACK_SEA))
			{
				return;
			}
		}
		if (iUnitsParadrop < (iNumCitiesPlayer * 2) && !bFinancialTrouble && !m_bAreaSaturatedOfLandMilitaryUnits) // Build some paradrop when possible
		{
			if (AI_chooseUnit("Reserve army: Paradrop (iUnitsParadrop < (iNumCitiesPlayer * 2)", UNITAI_PARADROP))
			{
				return;
			}
		}

		// Build transport to escort units if we have the opportunity to do so
		if (iTransports < (iUnitsToTransport / 4) && pWaterArea != NULL)
		{
			if (AI_chooseUnit("Reserve army: transport unit (iTransports < (iUnitsToTransport / 4)", UNITAI_ESCORT_SEA))
			{
				return;
			}
		}

		if (iUnitsSeaCarrier < (iUnitsAircraft / 4) && pWaterArea != NULL)
		{
			if (AI_chooseUnit("Reserve army: Aircraft carrier", UNITAI_CARRIER_SEA))
			{
				return;
			}
		}

		if (iUnitsSeaMissileCarrier < (iUnitsMissiles / 4) && pWaterArea != NULL)
		{
			if (AI_chooseUnit("Reserve army: Missile carrier", UNITAI_MISSILE_CARRIER_SEA))
			{
				return;
			}
		}
	}


	// War plans
	bool bAtWar = ((iEnemyPowerPerc > 50) || (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0)); // If there is a warplan or a war which the AI doesn't win (ratio lesser than 2.0).
	if (((bAtWar && !bMinor && !bBarbarian && !isOccupation() && !bEarlyGameMilitary) || (bBarbarian && getRandNum100() > 3) || (bRebel && getRandNum100() > 29) || (bEarlyGameMilitary && getRandNum100() > 59)) && !bInhibitMilitaryUnits) // If minors or early game, consider 40% of chance to build a war unit. If barbarian, 90% of chance. If rebels, 70%.
	{
		// Build transport to escort units if we have the opportunity to do so (50% of chance)
		if (getRandNum100() > 49 && !bLandWar)
		{
			if (iTransports < (iUnitsToTransport / 4))
			{
				if (AI_chooseUnit("War: transport unit (iTransports < (iUnitsToTransport / 4)", UNITAI_ESCORT_SEA))
				{
					return;
				}
			}
			if (iUnitsSeaCarrier < (iUnitsAircraft / 4))
			{
				if (AI_chooseUnit("War: Aircraft carrier", UNITAI_CARRIER_SEA))
				{
					return;
				}
			}

			if (iUnitsSeaMissileCarrier < (iUnitsMissiles / 4))
			{
				if (AI_chooseUnit("War: Missile carrier", UNITAI_MISSILE_CARRIER_SEA))
				{
					return;
				}
			}
		}

		// 5% of chance to train a pirate
		if (getRandNum100() > 94)
		{
			if (AI_chooseUnit("War: pirate unit", UNITAI_PIRATE_SEA))
			{
				return;
			}
		}

		// If the AI is doing a turtle strategy, build defense only
		if (kPlayer.AI_isDoStrategy(AI_STRATEGY_TURTLE) && !m_bAreaSaturatedOfLandMilitaryUnits)
		{
			// 1/3 chance to build a defensive structure (like walls...)
			if (getRandNum100() < 33)
			{
				if (AI_chooseBuilding(BUILDINGFOCUS_DEFENSE))
				{
					if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S will build a defensive building (turtle war)", getName().GetCString());
					return;
				}
			}
			// 25% of chance to build a counter defense (minigun, etc).
			if (getRandNum100() > 74)
			{
				if (AI_chooseUnit("War: city counter unit", UNITAI_CITY_COUNTER))
				{
					return;
				}
			}
			if (AI_chooseUnit("War: defense unit", UNITAI_CITY_DEFENSE))
			{
				return;
			}
		}
		// 1/6 of chance to build a exp oriented building, 1/6 to prod building
		else if (getRandNum100() < 16)
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_EXPERIENCE))
			{
				if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S will build a building focused on experience (war)", getName().GetCString());
				return;
			}
		}
		else if (getRandNum100() < 16)
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_PRODUCTION))
			{
				if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S will build a production oriented building (war)", getName().GetCString());
				return;
			}
		}
		else if (!m_bAreaSaturatedOfLandMilitaryUnits)
		{
			if (getRandNum100() < 51)
			{
				// Build only attack units reduce the number of siege units by 25%
				if (AI_chooseUnit("War: attack unit", UNITAI_ATTACK))
				{
					return;
				}
			}
			// A offensive warplan is coming, build offensive units everywhere
			if (AI_chooseUnit("War: attack city unit", UNITAI_ATTACK_CITY))
			{
				return;
			}
		}
	}

	/* ---------- Peace time ---------- */
// 45deg - restore old code - START	
	bool bChooseWorker = false;
	int iMinFoundValue = kPlayer.AI_getMinFoundValue();
	if (bDanger)
	{
		iMinFoundValue *= 3;
		iMinFoundValue /= 2;
	}
	
	if (bMaybeWaterArea && !bInhibitMilitaryUnits && !bInhibitUnits)
	{
		if( !(bLandWar && iWarSuccessRatio < -30) && !bDanger && !bFinancialTrouble )
		{
			if (kPlayer.AI_getNumTrainAIUnits(UNITAI_ATTACK_SEA) + kPlayer.AI_getNumTrainAIUnits(UNITAI_PIRATE_SEA) + kPlayer.AI_getNumTrainAIUnits(UNITAI_RESERVE_SEA) < std::min(3,kPlayer.getNumCities()))
			{
				if ((bMaybeWaterArea && bWaterDanger)
					|| (pWaterArea != NULL &&
						bPrimaryArea &&
						kPlayer.AI_countNumAreaHostileUnits(pWaterArea, true, false, false, false, plot(), 15) > 0) )
				{
					int iLocalNavy = kPlayer.AI_countNumLocalNavy(plot(),4);
					
					if ( iLocalNavy < 2 )	//	If there are a few local appropriate AI units already assume they will deal
					{
						/*if (AI_chooseUnit("minimal navy", UNITAI_ATTACK_SEA))
							return;
						if (AI_chooseUnit("minimal navy", UNITAI_PIRATE_SEA))
							return;
						if (AI_chooseUnit("minimal navy", UNITAI_RESERVE_SEA))
							return;*/
						// <f1rpo> Don't prioritize those ships quite as much (advc.017)
						int iOdds = 60;
						if(bLandWar && iWarSuccessRatio < 0)
							iOdds += 2 * iWarSuccessRatio;
						// Fallback on other AI types shouldn't be needed (and would increase the effective odds)
						if (AI_chooseUnit("minimal navy", UNITAI_ATTACK_SEA, iOdds))
							return;
						// </f1rpo>
					}
				}
			}
		
			if (NULL != pWaterArea)
			{
				int iOdds = -1;
				if (iAreaBestFoundValue == 0 || iWaterAreaBestFoundValue > iAreaBestFoundValue)
				{
					iOdds = 100;
				}
				else if (iWaterPercent > 60)
				{
					iOdds = 13;
				}

				if( iOdds >= 0 )
				{
					if (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_EXPLORE_SEA) == 0)
					{
						if (AI_chooseUnit("sea explorer", UNITAI_EXPLORE_SEA, iOdds))
						{
							return;
						}
					}

					// BBAI TODO: Really only want to do this if no good area city sites ... 13% chance on water heavy maps
					// of slow start, little benefit
					if (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_SETTLER_SEA) == 0)
					{
						if (AI_chooseUnit("settler sea", UNITAI_SETTLER_SEA, iOdds))
						{
							return;
						}
					}
				}
			}
		}
	}
	
	m_iTempBuildPriority--;
	
	int iPlotSettlerCount = (iNumSettlers == 0) ? 0 : plot()->plotCount(PUF_isUnitAIType, UNITAI_SETTLE, -1, getOwnerINLINE());	
	
	if( !bInhibitUnits && !(bDefenseWar && iWarSuccessRatio < -50) )
	{
		if ((iAreaBestFoundValue > iMinFoundValue) || (iWaterAreaBestFoundValue > iMinFoundValue))
		{
			// BBAI TODO: Needs logic to check for early settler builds, settler builds in small cities, whether settler sea exists for water area sites?
			if (pWaterArea != NULL)
			{
				int iTotalCities = kPlayer.getNumCities();
				int iSettlerSeaNeeded = std::min(iNumWaterAreaCitySites, ((iTotalCities + 4) / 8) + 1);
				if (kPlayer.getCapitalCity() != NULL)
				{
					int iOverSeasColonies = iTotalCities - kPlayer.getCapitalCity()->area()->getCitiesPerPlayer(getOwnerINLINE());
					int iLoop = 2;
					int iExtras = 0;
					while (iOverSeasColonies >= iLoop)
					{
						iExtras++;
						iLoop += iLoop + 2;
					}
					iSettlerSeaNeeded += std::min(kPlayer.AI_totalUnitAIs(UNITAI_WORKER) / 4, iExtras);
				}
				if (bAssault)
				{
					iSettlerSeaNeeded = std::min(1, iSettlerSeaNeeded);
				}
				
				if (kPlayer.AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_SETTLER_SEA) < iSettlerSeaNeeded)
				{
/********************************************************************************/
/* 	Build more workers #1.1										Fuyu			*/
/********************************************************************************/
					/* financial trouble: 2/3; */
					if (!bDanger && bFinancialTrouble && (iExistingWorkers < ((2*iNeededWorkers) + 2)/3))
					{
						if ((AI_getWorkersNeeded() > 0) && (AI_getWorkersHave() == 0))
						{
							if( getPopulation() > 1 || (GC.getGameINLINE().getGameTurn() - getGameTurnAcquired() > (15 * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent())/100) )
							{
								if (!bChooseWorker && AI_chooseUnit("worker needed", UNITAI_WORKER))
								{
									return;
								}
								bChooseWorker = true;
							}
						}
					}
/********************************************************************************/
/* 	Build more workers #1.1										END				*/
/********************************************************************************/
					if (AI_chooseUnit("sea settler needed", UNITAI_SETTLER_SEA))
					{
						return;
					}
				}
			}
			
			if (iPlotSettlerCount == 0)
			{
				if ((iNumSettlers < iMaxSettlers))
				{
					if (getCitySorenRandNum(2, "settler training decision") < (bLandWar ? 1 : 2))
					{
/********************************************************************************/
/* 	Build more workers #1.2										Fuyu			*/
/********************************************************************************/
						/* financial trouble: 2/3; */
						if (!bDanger && bFinancialTrouble && (iExistingWorkers < ((2*iNeededWorkers) + 2)/3))
						{
							if ((AI_getWorkersNeeded() > 0) && (AI_getWorkersHave() == 0))
							{
								if( getPopulation() > 1 || (GC.getGameINLINE().getGameTurn() - getGameTurnAcquired() > (15 * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent())/100) )
								{
									if (!bChooseWorker && AI_chooseUnit("worker needed 2", UNITAI_WORKER))
									{
										return;
									}
									bChooseWorker = true;
								}
							}
						}
/********************************************************************************/
/* 	Build more workers #1.2										END				*/
/********************************************************************************/
/********************************************************************************/
/* 	City Defenders						24.07.2010				Fuyu			*/
/********************************************************************************/
						if ((kPlayer.getNumMilitaryUnits() <= (kPlayer.getNumCities() + iNumSettlers + 1))
							//Fuyu: in the beginning, don't count on other cities to build the escorts
							|| (kPlayer.getNumCities() <= 7 && iNumCitiesInArea <= 3 && (plot()->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isCityAIType) <= 1)))
						{
							if (AI_chooseUnit("extra quick defender", UNITAI_CITY_DEFENSE))
							{
								return;
							}
//Fuyu bIgnoreNotUnitAIs
							if (iNumCitiesInArea <= 3)
							{
								CvUnitSelectionCriteria criteria;

								criteria.m_bIgnoreNotUnitAIs = true;
								if (AI_chooseUnit("extra quick defender (any unit)", UNITAI_CITY_DEFENSE, -1, -1, -1, &criteria))
								{
									return;
								}
							}

							if (AI_chooseUnit("extra quick defender", UNITAI_CITY_COUNTER))
							{
								return;
							}
							if (AI_chooseUnit("extra quick defender", UNITAI_CITY_SPECIAL))
							{
								return;
							}

							if (AI_chooseUnit("extra quick defender", UNITAI_RESERVE))
							{
								return;
							}
						}
/********************************************************************************/
/* 	City Defenders												END 			*/
/********************************************************************************/
						if (AI_chooseUnit("settler needed", UNITAI_SETTLE) )
						{
							return;
						}
					}
				}
			}
		}
	}

	m_iTempBuildPriority--;
	
	if (!bInhibitMilitaryUnits && !bInhibitUnits && (pWaterArea != NULL) && !bDefenseWar && !bAssault)
	{
		if( !bFinancialTrouble )
		{
			// Force civs with foreign colonies to build a few assault transports to defend the colonies
			if( kPlayer.AI_totalUnitAIs(UNITAI_ASSAULT_SEA) < (kPlayer.getNumCities() - iNumCapitalAreaCities)/3 )
			{
				if (AI_chooseUnit("colony defense assault ships", UNITAI_ASSAULT_SEA))
				{
					return;
				}
			}

			if (kPlayer.AI_calculateUnitAIViability(UNITAI_SETTLER_SEA, DOMAIN_SEA) < 61)
			{
				// Force civs to build escorts for settler_sea units
				if( kPlayer.AI_totalUnitAIs(UNITAI_SETTLER_SEA) > kPlayer.AI_getNumAIUnits(UNITAI_RESERVE_SEA) )
				{
					if (AI_chooseUnit("sea settler escorts", UNITAI_RESERVE_SEA))
					{
						return;
					}
				}
			}
		}
	}
	
	m_iTempBuildPriority--;	
// 45deg - restore old code - END

	if (!bInhibitUnits)
	{
		// Need to have at least a scout to explore
		if (iUnitsScout == 0 && kPlayer.getCurrentEra() < 2 && !bInhibitMilitaryUnits)
		{
			if (AI_chooseUnit("Minimal scout", UNITAI_EXPLORE))
			{
				return;
			}
		}
		if (iUnitsSeaScout == 0 && !bInhibitMilitaryUnits)
		{
			if (AI_chooseUnit("Minimal scout", UNITAI_EXPLORE_SEA))
			{
				return;
			}
		}
	}

	// Special rule for the secondaries cities
	if (getPopulation() < 5 && !isCapital())
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_FOOD | BUILDINGFOCUS_CULTURE | BUILDINGFOCUS_PRODUCTION, 20))
		{
			if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S is small and secondary, will build a first building (peace)", getName().GetCString());
			return;
		}
	}
	m_iTempBuildPriority--;

	if (!bInhibitUnits)
	{
		// Having at least one worker is an important thing
		if (iExistingWorkers == 0)
		{
			if (AI_chooseUnit("The first worker", UNITAI_WORKER, -1, -1, CITY_NO_WORKERS_WORKER_PRIORITY))
			{
				return;
			}
		}
		if ((iNeededSeaWorkers > iExistingSeaWorkers) && iExistingSeaWorkers == 0)
		{
			if (AI_chooseUnit("The first sea worker", UNITAI_WORKER_SEA, -1, -1, CITY_NO_WORKERS_WORKER_PRIORITY))
			{
				return;
			}
		}
	}

	// Resolution of happyness problems
	int iHappyness = happyLevel() - unhappyLevel(0);
	if (iHappyness > -3 && iHappyness < 0) // If happyness problems are low, make it trait dependant and quickly build something
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_HAPPY, 10, 0, -1, true))
		{
			if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S is unhappy, will build a happy focused building (peace)", getName().GetCString());
			return;
		}
	}
	if (iHappyness < -2 || iHappyness <= -getPopulation() / 2) // If happyness problems are severe, build something
	{
		//	More than 2 unhappy citizens (or more than half the population, increase the valid building lenght)
		if (AI_chooseBuilding(BUILDINGFOCUS_HAPPY, 30, 0, -1, true))
		{
			if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S is really unhappy, will build a happy focused building (peace)", getName().GetCString());
			return;
		}

		//	To review: Can we build military happyness units?  Only do that up to Mindefenders + 1 over what we might
		//	normally consider // TO IMPROVE
		if (!bInhibitUnits && GET_PLAYER(getOwner()).getHappyPerMilitaryUnit() > 0 &&
			plot()->plotCount(PUF_canDefend) < (AI_minDefenders() + 1))
		{
			if (AI_chooseUnit("emergency military happyness", UNITAI_CITY_DEFENSE))
			{
				return;
			}
		}
	}

	m_iTempBuildPriority--;

	// Resolution of financial problems
	if (bFinancialTrouble)
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_GOLD | BUILDINGFOCUS_MAINTENANCE))
		{
			if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S uses choose financial trouble gold", getName().GetCString());
			return;
		}
	}
	m_iTempBuildPriority--;

	// Increase production output if it's quick to do
	if (AI_chooseBuilding(BUILDINGFOCUS_PRODUCTION, 3))
	{
		if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S will build a quick production improvement (peace)", getName().GetCString());
		return;
	}
	m_iTempBuildPriority--;

	// Do we need to expand ?
	if (!bInhibitUnits && !bBarbarian)
	{
		if (iNumSettlers < iMaxSettlers)
		{
			if (!bFinancialTrouble)
			{
				if (iAreaBestFoundValue > kPlayer.AI_getMinFoundValue())
				{
					if (AI_chooseUnit("optional settlers", UNITAI_SETTLE))
					{
						return;
					}
				}
			}
		}
	}
	m_iTempBuildPriority--;

	// Rush Wonders build
	if ((iAffinityWonders >= (iAffinityThreshold - 15)) && iProductionRank <= ((iNumCitiesPlayer / 5) + 1) && !bBarbarian && !bRebel)
	{
		if (iNumCitiesPlayer <= 3)
		{
			// For small civ at war, don't build wonders unless winning
			if (AI_chooseBuilding(BUILDINGFOCUS_WORLDWONDER, 50))
			{
				if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S will build a wonder (small empire)", getName().GetCString());
				return;
			}
		}
		else
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_WORLDWONDER, 70))
			{
				if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S will build a wonder", getName().GetCString());
				return;
			}
		}

		// Projects
		if (AI_chooseProject())
		{
			if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S will create a project", getName().GetCString());
			return;
		}
	}
	m_iTempBuildPriority--;

	// Non-combat units
	if (!bInhibitUnits && !bBarbarian)
	{
		// Do we need some more workers ?
		if (!(iExistingWorkers == 0))
		{
			if (iExistingWorkers < ((iNeededWorkers + 1) / 2))
			{
				if (getPopulation() > 3 || (iProductionRank < (kPlayer.getNumCities() + 1) / 2))
				{
					if (AI_chooseUnit("no danger large city extra worker", UNITAI_WORKER))
					{
						return;
					}
				}
			}
		}
		m_iTempBuildPriority--;

		if (!bWaterDanger && iNeededSeaWorkers > iExistingSeaWorkers)
		{
			if (AI_chooseUnit("no danger extra sea worker", UNITAI_WORKER_SEA))
			{
				return;
			}
		}
		m_iTempBuildPriority--;
		
		// Do we need more scouts ? No priority, so let's say odds of 10%. Starting to era 2, scouts are no more a priority.
		if (!bFinancialTrouble && !bInhibitUnits && ((getRandNum100() > 89 && kPlayer.getCurrentEra() < 2) || (getRandNum100() > 94 && kPlayer.getCurrentEra() >= 2)))
		{
			if (iUnitsScout < iOptimalScout && !m_bAreaSaturatedOfLandMilitaryUnits)
			{
				if (AI_chooseUnit("Minimal scout", UNITAI_EXPLORE))
				{
					return;
				}
			}
			if (iUnitsSeaScout < iOptimalSeaScout)
			{
				if (AI_chooseUnit("Minimal scout", UNITAI_EXPLORE_SEA))
				{
					return;
				}
			}
		}

		// Do we need transport units ? No priority, so let's say odds of 25%. It should help the AI to move its units
		if (iTransports < (iUnitsToTransport / 4) && getRandNum100() > 74)
		{
			if (AI_chooseUnit("Transport unit", UNITAI_ESCORT_SEA))
			{
				return;
			}
		}

		// Build some caravans to improve the economy
		if (iProductionRank <= ((iNumCitiesPlayer / 5) + 1))
		{
			if (AI_buildCaravan())
			{
				return;
			}
			m_iTempBuildPriority--;
		}
	}

	// Capitals buildings
	/*if (!bDanger && !bIsCapitalArea && area()->getCitiesPerPlayer(getOwnerINLINE()) > iNumCapitalAreaCities)
	{
		// BBAI TODO:  This check should be done by player, not by city and optimize placement
		// If losing badly in war, don't build big things
		if (!bLandWar || (iWarSuccessRatio > -30))
		{
			if (kPlayer.getCapitalCity() == NULL || area()->getPopulationPerPlayer(getOwnerINLINE()) > kPlayer.getCapitalCity()->area()->getPopulationPerPlayer(getOwnerINLINE()))
			{
				if (AI_chooseBuilding(BUILDINGFOCUS_CAPITAL, 15))
				{
					return;
				}
			}
		}
	}

	m_iTempBuildPriority--;*/

	/* Special units */
	if (!bInhibitUnits && !bFinancialTrouble)
	{
		// Missionaries
		int iSpreadUnitThreshold = 1000;
		if (bLandWar)
		{
			iSpreadUnitThreshold += 800 - 10 * iWarSuccessRatio;
		}
		iSpreadUnitThreshold += 300 * plot()->plotCount(PUF_isUnitAIType, UNITAI_MISSIONARY, -1, getOwnerINLINE());

		UnitTypes eBestSpreadUnit = NO_UNIT;
		int iBestSpreadUnitValue = -1;

		if (!(kPlayer.AI_isDoStrategy(AI_STRATEGY_TURTLE)))
		{
			int iSpreadUnitRoll = (100 - iBuildUnitProb) / 3;
			iSpreadUnitRoll += bLandWar ? 0 : 10;

			if (AI_bestSpreadUnit(true, true, iSpreadUnitRoll, &eBestSpreadUnit, &iBestSpreadUnitValue))
			{
				if (iBestSpreadUnitValue > iSpreadUnitThreshold)
				{
					if (AI_chooseUnit(eBestSpreadUnit, UNITAI_MISSIONARY))
					{
						return;
					}
				}
			}
		}
		m_iTempBuildPriority--;

		// Spies
		if (pArea->getNumCities() > 4 && getCitySorenRandNum((iProductionRank + 1) * 10, "offensive spy chance") == 0)
		{
			int iNumSpies = std::max(0, kPlayer.AI_totalAreaUnitAIs(pArea, UNITAI_SPY));
			int iNeededAreaSpies = GET_PLAYER(getOwnerINLINE()).AI_getNumNeededSpies(pArea, false);

			if (!bInhibitUnits && iNumSpies < iNeededAreaSpies)
			{
				if (AI_chooseUnit("offensive spy", UNITAI_SPY, 30 * (iNeededAreaSpies - iNumSpies > 3 ? 2 : 1)))
				{
					return;
				}
			}
		}
		m_iTempBuildPriority--;

		// Inquisitors
		if (AI_trainInquisitor())
		{
			return;
		}
	}

	/* Classic buildings */
	// Production oriented. If there is a production enhancement, build it (30%)
	if (getRandNum100() > 30)
	{
		if (getBaseYieldRate(YIELD_PRODUCTION) >= 8)
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_PRODUCTION, 40))
			{
				if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S uses choose BUILDINGFOCUS_PRODUCTION 3", getName().GetCString());
				return;
			}
		}
	}
	m_iTempBuildPriority--;

	// Gold oriented (Afforess - don't wait until we are in trouble, preventative medicine is best)
	int iFundedPercent = kPlayer.AI_costAsPercentIncome();
	int iSafePercent = kPlayer.AI_safeCostAsPercentIncome();
	if ((iFundedPercent < iSafePercent - 10) || iAffinityCommerce > iAffinityThreshold)
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_GOLD | BUILDINGFOCUS_MAINTENANCE, 10))
		{
			if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S uses financial difficulty resolution", getName().GetCString());
			return;
		}
	}
	m_iTempBuildPriority--;

	// Growth oriented (do not build if next level will be unhappy)
	if (iAffinityGrowth > iAffinityThreshold && iHappyness >= 0)
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_FOOD, isCapital() ? 20 : 40))
		{
			if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S uses choose BUILDINGFOCUS_FOOD 2", getName().GetCString());
			return;
		}
	}
	m_iTempBuildPriority--;

	// Science oriented
	if (iAffinityScience > iAffinityThreshold)
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_RESEARCH, isCapital() ? 20 : 40))
		{
			if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S uses choose BUILDINGFOCUS_RESEARCH", getName().GetCString());
			return;
		}
	}
	m_iTempBuildPriority--;

	// Military oriented
	if (iAffinityMilitary > iAffinityThreshold)
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_DEFENSE | BUILDINGFOCUS_EXPERIENCE, isCapital() ? 20 : 40))
		{
			if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S uses choose BUILDINGFOCUS_DEFENSE", getName().GetCString());
			return;
		}
	}
	m_iTempBuildPriority--;

	// Culture oriented
	if (iAffinityCulture > iAffinityThreshold || (iCulturePressure > 90))
	{
		if (bImportantCity && (iCultureRateRank <= iCulturalVictoryNumCultureCities))
		{
			if (iCultureRateRank == iCulturalVictoryNumCultureCities)
			{
				if (AI_chooseBuilding(BUILDINGFOCUS_BIGCULTURE | BUILDINGFOCUS_CULTURE | BUILDINGFOCUS_WONDEROK, 40))
				{
					if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S uses cultural victory 1", getName().GetCString());
					return;
				}
			}
			else if (getCitySorenRandNum(((iCultureRateRank == 1) ? 4 : 1) + iCulturalVictoryNumCultureCities * 2 + (bLandWar ? 5 : 0), "AI Build up Culture") < iCultureRateRank)
			{
				if (AI_chooseBuilding(BUILDINGFOCUS_BIGCULTURE | BUILDINGFOCUS_CULTURE | BUILDINGFOCUS_WONDEROK, (bLandWar ? 20 : 40)))
				{
					if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S uses cultural victory 2", getName().GetCString());
					return;
				}
			}
		}
		else if (AI_chooseBuilding(BUILDINGFOCUS_CULTURE | BUILDINGFOCUS_BIGCULTURE, 20))
		{
			if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S uses financial difficulty resolution", getName().GetCString());
			return;
		}
	}
	m_iTempBuildPriority--;

	// Happyness oriented
	if (iAffinityHappyness > iAffinityThreshold)
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_HAPPY, isCapital() ? 20 : 40))
		{
			if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S uses choose BUILDINGFOCUS_RESEARCH", getName().GetCString());
			return;
		}
	}
	m_iTempBuildPriority--;

	// Health oriented
	if (iAffinityHealth > iAffinityThreshold)
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_HEALTHY, isCapital() ? 20 : 40))
		{
			if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S uses choose BUILDINGFOCUS_RESEARCH", getName().GetCString());
			return;
		}
	}
	m_iTempBuildPriority--;

	// Espionage oriented
	if (iAffinityEspionage > iAffinityThreshold)
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_ESPIONAGE, isCapital() ? 20 : 40))
		{
			if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S uses choose BUILDINGFOCUS_RESEARCH", getName().GetCString());
			return;
		}
	}
	m_iTempBuildPriority--;

	// Capital special case
	if ((isCapital() || getYieldRate(YIELD_PRODUCTION) > std::min(70, std::max(40, iNumCitiesInArea * 6))))
	{
		if (bFinancialTrouble)
		{
			if (AI_chooseProcess(COMMERCE_GOLD))
			{
				if (gCityLogLevel >= 2) logBBAIForTeam(getTeam(), "	  City %S chooses to build wealth due to financial trouble", getName().GetCString());
				return;
			}
		}
	}

	m_iTempBuildPriority--;

	//	Koshling.  AI shouldn't choose gold as often as it does.  If we have plenty of
	//	gold prefer research most of the time

	//	Set up weights 0-100 for each commerce type to weight the choice (gold weigth can actuially go higher than
	//	100, but only if we already have less gold than our target in which case we should already have unconditonally
	//	gone for gold earlier in this choice)
	int commerceWeights[NUM_COMMERCE_TYPES];

	commerceWeights[COMMERCE_GOLD] = std::max(0, 100 - (GET_PLAYER(getOwnerINLINE()).getGold() - GET_PLAYER(getOwnerINLINE()).AI_goldTarget()) * 100 / std::max(1, GET_PLAYER(getOwnerINLINE()).AI_goldTarget()));
	if (isHuman() && !AI_isEmphasizeCommerce(COMMERCE_GOLD))
	{
		commerceWeights[COMMERCE_GOLD] = 0;
	}

	commerceWeights[COMMERCE_RESEARCH] = 50;	//	Can't think of a good measure for research pressure so just set to a middling value
	if (isHuman() && !AI_isEmphasizeCommerce(COMMERCE_RESEARCH))
	{
		commerceWeights[COMMERCE_RESEARCH] = 0;
	}
	commerceWeights[COMMERCE_CULTURE] = iCulturePressure;
	if (isHuman() && !AI_isEmphasizeCommerce(COMMERCE_CULTURE))
	{
		commerceWeights[COMMERCE_CULTURE] = 0;
	}
	commerceWeights[COMMERCE_ESPIONAGE] = 10;	//	Is this ever really worthwhile?

	AI_chooseProcess(NO_COMMERCE, commerceWeights);

	// If nothing has been chosen, choose a building
	if (AI_chooseBuilding(BUILDINGFOCUS_CONSIDER_ANY))
	{
		return;
	}
}

UnitTypes CvCityAI::AI_bestUnit(int& iBestUnitValue, int iNumSelectableTypes, UnitAITypes* pSelectableTypes, bool bAsync, UnitAITypes* peBestUnitAI, bool bNoRand, bool bNoWeighting, CvUnitSelectionCriteria* criteria)
{
	CvArea* pWaterArea;
	int aiUnitAIVal[NUM_UNITAI_TYPES];
	UnitTypes eUnit;
	UnitTypes eBestUnit;
	bool bWarPlan;
	bool bDefense;
	bool bLandWar;
	bool bAssault;
	bool bPrimaryArea;
	bool bAreaAlone;
	bool bFinancialTrouble;
	bool bWarPossible;
	bool bDanger;
	int iHasMetCount;
	int iMilitaryWeight;
	int iCoastalCities = 0;
	int iBestValue;
	int iI;

	iBestUnitValue = 0;

	if (peBestUnitAI != NULL)
	{
		*peBestUnitAI = NO_UNITAI;
	}

	if ( !bNoWeighting )
	{
	/************************************************************************************************/
	/* BETTER_BTS_AI_MOD					  11/30/08								jdog5000	  */
	/*																							  */
	/* City AI																					  */
	/************************************************************************************************/
	/* original bts code
		pWaterArea = waterArea();
	*/
		pWaterArea = waterArea(true);
	/************************************************************************************************/
	/* BETTER_BTS_AI_MOD					   END												  */
	/************************************************************************************************/

		bWarPlan = (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0);
		bDefense = (area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE);
		bLandWar = (bDefense || (area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING));
		bAssault = (area()->getAreaAIType(getTeam()) == AREAAI_ASSAULT);
		bPrimaryArea = GET_PLAYER(getOwnerINLINE()).AI_isPrimaryArea(area());
		bAreaAlone = GET_PLAYER(getOwnerINLINE()).AI_isAreaAlone(area());
		bFinancialTrouble = GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble();
		bWarPossible = GET_TEAM(getTeam()).AI_isWarPossible();
		bDanger = AI_isDanger();

		iHasMetCount = GET_TEAM(getTeam()).getHasMetCivCount(true);
		iMilitaryWeight = GET_PLAYER(getOwnerINLINE()).AI_militaryWeight(area());
		int iNumCitiesInArea = area()->getCitiesPerPlayer(getOwnerINLINE());

		if (pWaterArea != NULL)
		{
			iCoastalCities = GET_PLAYER(getOwnerINLINE()).countNumCoastalCitiesByArea(pWaterArea);
		}

		for (iI = 0; iI < NUM_UNITAI_TYPES; iI++)
		{
			aiUnitAIVal[iI] = 0;
		}

		if (!bFinancialTrouble && ((bPrimaryArea) ? (GET_PLAYER(getOwnerINLINE()).findBestFoundValue() > 0) : (area()->getBestFoundValue(getOwnerINLINE()) > 0)))
		{
			aiUnitAIVal[UNITAI_SETTLE]++;
		}

		aiUnitAIVal[UNITAI_WORKER] += GET_PLAYER(getOwnerINLINE()).AI_neededWorkers(area());

		aiUnitAIVal[UNITAI_ATTACK] += ((iMilitaryWeight / ((bWarPlan || bLandWar || bAssault) ? 7 : 12)) + ((bPrimaryArea && bWarPossible) ? 2 : 0) + 1);

		aiUnitAIVal[UNITAI_CITY_DEFENSE] += (iNumCitiesInArea + 1);
		aiUnitAIVal[UNITAI_CITY_COUNTER] += ((5 * (iNumCitiesInArea + 1)) / 8);
		aiUnitAIVal[UNITAI_CITY_SPECIAL] += ((iNumCitiesInArea + 1) / 2);

		if (bWarPossible)
		{
			aiUnitAIVal[UNITAI_ATTACK_CITY] += ((iMilitaryWeight / ((bWarPlan || bLandWar || bAssault) ? 10 : 17)) + ((bPrimaryArea) ? 1 : 0));
			aiUnitAIVal[UNITAI_COUNTER] += ((iMilitaryWeight / ((bWarPlan || bLandWar || bAssault) ? 13 : 22)) + ((bPrimaryArea) ? 1 : 0));
			aiUnitAIVal[UNITAI_PARADROP] += ((iMilitaryWeight / ((bWarPlan || bLandWar || bAssault) ? 5 : 8)) + ((bPrimaryArea) ? 1 : 0));

			aiUnitAIVal[UNITAI_DEFENSE_AIR] += (GET_PLAYER(getOwnerINLINE()).getNumCities() + 1);
			aiUnitAIVal[UNITAI_CARRIER_AIR] += GET_PLAYER(getOwnerINLINE()).AI_countCargoSpace(UNITAI_CARRIER_SEA);
			aiUnitAIVal[UNITAI_MISSILE_AIR] += GET_PLAYER(getOwnerINLINE()).AI_countCargoSpace(UNITAI_MISSILE_CARRIER_SEA);

			if (bPrimaryArea)
			{
				aiUnitAIVal[UNITAI_ICBM] += std::max((GET_PLAYER(getOwnerINLINE()).getTotalPopulation() / 25), ((GC.getGameINLINE().countCivPlayersAlive() + GC.getGameINLINE().countTotalNukeUnits()) / (GC.getGameINLINE().countCivPlayersAlive() + 1)));
			}
		}

		if (isBarbarian())
		{
			aiUnitAIVal[UNITAI_ATTACK] *= 4;
		}
		else
		{
			if (!bLandWar)
			{
				aiUnitAIVal[UNITAI_EXPLORE] += GET_PLAYER(getOwnerINLINE()).AI_neededExplorers(area());
			}

			if (pWaterArea != NULL)
			{
				aiUnitAIVal[UNITAI_WORKER_SEA] += AI_neededSeaWorkers();

				if ((GET_PLAYER(getOwnerINLINE()).getNumCities() > 3) || (area()->getNumUnownedTiles() < 10))
				{
					if (bPrimaryArea)
					{
						aiUnitAIVal[UNITAI_EXPLORE_SEA] += GET_PLAYER(getOwnerINLINE()).AI_neededExplorers(pWaterArea);
					}

					if (bPrimaryArea && (GET_PLAYER(getOwnerINLINE()).findBestFoundValue() > 0) && (pWaterArea->getNumTiles() > 300))
					{
						aiUnitAIVal[UNITAI_SETTLER_SEA]++;
					}

					if (bPrimaryArea && (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_MISSIONARY) > 0) && (pWaterArea->getNumTiles() > 400))
					{
						aiUnitAIVal[UNITAI_MISSIONARY_SEA]++;
					}

					if (bPrimaryArea && (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_SPY) > 0) && (pWaterArea->getNumTiles() > 500))
					{
						aiUnitAIVal[UNITAI_SPY_SEA]++;
					}

					aiUnitAIVal[UNITAI_PIRATE_SEA] += pWaterArea->getNumTiles() / 600;

					if (bWarPossible)
					{
						aiUnitAIVal[UNITAI_ATTACK_SEA] += std::min((pWaterArea->getNumTiles() / 150), ((((iCoastalCities * 2) + (iMilitaryWeight / 9)) / ((bAssault) ? 4 : 6)) + ((bPrimaryArea) ? 1 : 0)));
						aiUnitAIVal[UNITAI_RESERVE_SEA] += std::min((pWaterArea->getNumTiles() / 200), ((((iCoastalCities * 2) + (iMilitaryWeight / 7)) / 5) + ((bPrimaryArea) ? 1 : 0)));
						aiUnitAIVal[UNITAI_ESCORT_SEA] += (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_ASSAULT_SEA) + (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_CARRIER_SEA) * 2));
						aiUnitAIVal[UNITAI_ASSAULT_SEA] += std::min((pWaterArea->getNumTiles() / 250), ((((iCoastalCities * 2) + (iMilitaryWeight / 6)) / ((bAssault) ? 5 : 8)) + ((bPrimaryArea) ? 1 : 0)));
						aiUnitAIVal[UNITAI_CARRIER_SEA] += std::min((pWaterArea->getNumTiles() / 350), ((((iCoastalCities * 2) + (iMilitaryWeight / 8)) / 7) + ((bPrimaryArea) ? 1 : 0)));
						aiUnitAIVal[UNITAI_MISSILE_CARRIER_SEA] += std::min((pWaterArea->getNumTiles() / 350), ((((iCoastalCities * 2) + (iMilitaryWeight / 8)) / 7) + ((bPrimaryArea) ? 1 : 0)));
					}
				}
			}

			if ((iHasMetCount > 0) && bWarPossible)
			{
				if (bLandWar || bAssault || !bFinancialTrouble || (GET_PLAYER(getOwnerINLINE()).calculateUnitCost() == 0))
				{
					aiUnitAIVal[UNITAI_ATTACK] += ((iMilitaryWeight / ((bLandWar || bAssault) ? 9 : 16)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));
					aiUnitAIVal[UNITAI_ATTACK_CITY] += ((iMilitaryWeight / ((bLandWar || bAssault) ? 7 : 15)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));
					aiUnitAIVal[UNITAI_COLLATERAL] += ((iMilitaryWeight / ((bDefense) ? 8 : 14)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));
					aiUnitAIVal[UNITAI_PILLAGE] += ((iMilitaryWeight / ((bLandWar || bAssault) ? 10 : 19)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));
					aiUnitAIVal[UNITAI_RESERVE] += ((iMilitaryWeight / ((bLandWar) ? 12 : 17)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));
					aiUnitAIVal[UNITAI_COUNTER] += ((iMilitaryWeight / ((bLandWar || bAssault) ? 9 : 16)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));
					aiUnitAIVal[UNITAI_PARADROP] += ((iMilitaryWeight / ((bLandWar || bAssault) ? 4 : 8)) + ((bPrimaryArea && !bAreaAlone) ? 1 : 0));

					aiUnitAIVal[UNITAI_ATTACK_AIR] += (GET_PLAYER(getOwnerINLINE()).getNumCities() + 1);

					if (pWaterArea != NULL)
					{
						if ((GET_PLAYER(getOwnerINLINE()).getNumCities() > 3) || (area()->getNumUnownedTiles() < 10))
						{
							aiUnitAIVal[UNITAI_ATTACK_SEA] += std::min((pWaterArea->getNumTiles() / 100), ((((iCoastalCities * 2) + (iMilitaryWeight / 10)) / ((bAssault) ? 5 : 7)) + ((bPrimaryArea) ? 1 : 0)));
							aiUnitAIVal[UNITAI_RESERVE_SEA] += std::min((pWaterArea->getNumTiles() / 150), ((((iCoastalCities * 2) + (iMilitaryWeight / 11)) / 8) + ((bPrimaryArea) ? 1 : 0)));
						}
					}
				}
			}
		}

		// XXX this should account for air and heli units too...
		for (iI = 0; iI < NUM_UNITAI_TYPES; iI++)
		{
			if (GET_PLAYER(getOwnerINLINE()).AI_unitAIDomainType((UnitAITypes)iI) == DOMAIN_SEA)
			{
				if (pWaterArea != NULL)
				{
					aiUnitAIVal[iI] -= GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, ((UnitAITypes)iI));
				}
			}
			else if ((GET_PLAYER(getOwnerINLINE()).AI_unitAIDomainType((UnitAITypes)iI) == DOMAIN_AIR) || (iI == UNITAI_ICBM))
			{
				aiUnitAIVal[iI] -= GET_PLAYER(getOwnerINLINE()).AI_totalUnitAIs((UnitAITypes)iI);
			}
			else
			{
				aiUnitAIVal[iI] -= GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), ((UnitAITypes)iI));
			}
		}

		aiUnitAIVal[UNITAI_SETTLE] *= ((bDanger) ? 8 : 20);
		aiUnitAIVal[UNITAI_WORKER] *= ((bDanger) ? 2 : 7);
		aiUnitAIVal[UNITAI_ATTACK] *= 3;
		aiUnitAIVal[UNITAI_ATTACK_CITY] *= 4;
		aiUnitAIVal[UNITAI_COLLATERAL] *= 5;
		aiUnitAIVal[UNITAI_PILLAGE] *= 3;
		aiUnitAIVal[UNITAI_RESERVE] *= 3;
		aiUnitAIVal[UNITAI_COUNTER] *= 3;
		aiUnitAIVal[UNITAI_COUNTER] *= 2;
		aiUnitAIVal[UNITAI_CITY_DEFENSE] *= 2;
		aiUnitAIVal[UNITAI_CITY_COUNTER] *= 2;
		aiUnitAIVal[UNITAI_CITY_SPECIAL] *= 2;
		aiUnitAIVal[UNITAI_EXPLORE] *= ((bDanger) ? 6 : 15);
		aiUnitAIVal[UNITAI_ICBM] *= 18;
		aiUnitAIVal[UNITAI_WORKER_SEA] *= ((bDanger) ? 3 : 10);
		aiUnitAIVal[UNITAI_ATTACK_SEA] *= 5;
		aiUnitAIVal[UNITAI_RESERVE_SEA] *= 4;
		aiUnitAIVal[UNITAI_ESCORT_SEA] *= 20;
		aiUnitAIVal[UNITAI_EXPLORE_SEA] *= 18;
		aiUnitAIVal[UNITAI_ASSAULT_SEA] *= 14;
		aiUnitAIVal[UNITAI_SETTLER_SEA] *= 16;
		aiUnitAIVal[UNITAI_MISSIONARY_SEA] *= 12;
		aiUnitAIVal[UNITAI_SPY_SEA] *= 10;
		aiUnitAIVal[UNITAI_CARRIER_SEA] *= 8;
		aiUnitAIVal[UNITAI_MISSILE_CARRIER_SEA] *= 8;
		aiUnitAIVal[UNITAI_PIRATE_SEA] *= 5;
		aiUnitAIVal[UNITAI_ATTACK_AIR] *= 6;
		aiUnitAIVal[UNITAI_DEFENSE_AIR] *= 3;
		aiUnitAIVal[UNITAI_CARRIER_AIR] *= 15;
		aiUnitAIVal[UNITAI_MISSILE_AIR] *= 15;

		for (iI = 0; iI < NUM_UNITAI_TYPES; iI++)
		{
			aiUnitAIVal[iI] *= std::max(0, (GC.getLeaderHeadInfo(getPersonalityType()).getUnitAIWeightModifier(iI) + 100));
			aiUnitAIVal[iI] /= 100;
		}
	}

	iBestValue = 0;
	eBestUnit = NO_UNIT;

	for (iI = 0; iI < NUM_UNITAI_TYPES; iI++)
	{
		bool bValid = false;

		if ( iNumSelectableTypes != -1 )
		{
			for (int iJ = 0; iJ < iNumSelectableTypes; iJ++)
			{
				if ( pSelectableTypes[iJ] == iI )
				{
					bValid = true;
					break;
				}
			}
		}
		else
		{
			bValid = true;
		}

		if (bValid && (bNoWeighting || aiUnitAIVal[iI] > 0))
		{
			if ( !bNoWeighting )
			{
				if ( !bNoRand )
				{
					if (bAsync)
					{
						aiUnitAIVal[iI] += GC.getASyncRand().get(iMilitaryWeight, "AI Best UnitAI ASYNC");
					}
					else
					{
						aiUnitAIVal[iI] += getCitySorenRandNum(iMilitaryWeight, "AI Best UnitAI");
					}
				}

				if (aiUnitAIVal[iI] < iBestValue)
				{
					bValid = false;
				}
			}

			if ( bValid )
			{
				int iUnitValue;

				eUnit = AI_bestUnitAI(((UnitAITypes)iI), iUnitValue, bAsync, bNoRand, criteria);

				if (eUnit != NO_UNIT && iUnitValue > iBestUnitValue)
				{
					iBestValue = aiUnitAIVal[iI];
					eBestUnit = eUnit;
					if (peBestUnitAI != NULL)
					{
						*peBestUnitAI = ((UnitAITypes)iI);
					}
					iBestUnitValue = iUnitValue;
				}
			}
		}
	}

	return eBestUnit;
}

/********************************************************************************/
/* 	City Defenders						24.07.2010				Fuyu			*/
/********************************************************************************/
//Fuyu bIgnoreNotUnitAIs
UnitTypes CvCityAI::AI_bestUnitAI(UnitAITypes eUnitAI, int& iBestValue, bool bAsync, bool bNoRand, CvUnitSelectionCriteria* criteria)
/********************************************************************************/
/* 	City Defenders												END 			*/
/********************************************************************************/
{
	PROFILE_FUNC();

	UnitTypes eLoopUnit;
	UnitTypes eBestUnit;
	int iValue;
	int iOriginalValue;
	int iBestOriginalValue;
	int iI, iJ, iK;
	CvUnitSelectionCriteria tempCriteria;

	FAssert (eUnitAI != NO_UNITAI);

	bool bGrowMore = false;

	if ( criteria != NULL )
	{
		tempCriteria = *criteria;
	}

	if ( tempCriteria.m_eUnitAI == NO_UNITAI )
	{
		tempCriteria.m_eUnitAI = eUnitAI;
	}
	else
	{
		FAssert(tempCriteria.m_eUnitAI == eUnitAI);
	}

	if (!tempCriteria.m_bIgnoreGrowth && foodDifference(true, true) > 0)
	{
		// BBAI NOTE: This is where small city worker and settler production is blocked
		// TB Interpret Note: bGrowMore must be FALSE for cities to queue settlers!
		// This was basically saying that if the city has more population than improved tiles then it won't build settlers!
		if (GET_PLAYER(getOwnerINLINE()).getNumCities() <= 2 || isBarbarian())
		{
			/* original bts code
			bGrowMore = ((getPopulation() < 3) && (AI_countGoodTiles(true, false, 100) >= getPopulation())); */
			// K-Mod. We need to allow the starting city to build a worker at size 1.
			bGrowMore = (eUnitAI != UNITAI_WORKER || GET_PLAYER(getOwner()).AI_totalAreaUnitAIs(area(), UNITAI_WORKER) > 0)
				&& getPopulation() < 3 && AI_countGoodTiles(true, false, 100) >= getPopulation();
			// K-Mod end
		}
		else
		{
			bGrowMore = ((getPopulation() < 3) || (AI_countGoodTiles(true, false, 100) >= getPopulation()));
		}

		if (!bGrowMore && (getPopulation() < 6) && (AI_countGoodTiles(true, false, 80) >= getPopulation()))
		{
			if ((getFood() - (getFoodKept() / 2)) >= (growthThreshold() / 2))
			{
				if ((angryPopulation(1) == 0) && (healthRate(false, 1) == 0))
				{
					bGrowMore = true;
				}
			}
		}
		//K-Mod
		else if (bGrowMore)
		{
			if (angryPopulation(1) > 0)
				bGrowMore = false;
		}
		// K-Mod end
	}

	//	Check the cache
	EnterCriticalSection(&m_cBestUnitCacheSection);

	bool bHasCachedValue = false;
	int cacheKey = tempCriteria.getHash();
	std::map<int,UnitValueInfo>::const_iterator itr = m_bestUnits.find(cacheKey);

	if ( itr != m_bestUnits.end() )
	{
		//	Have to recheck canTrain as we may hit a unit limit mid-turn
		if ( (itr->second.eUnit == NO_UNIT || !bGrowMore || !isFoodProduction(itr->second.eUnit)) && canTrain(itr->second.eUnit) )
		{
			iBestValue = itr->second.iValue;
			eBestUnit = itr->second.eUnit;
			bHasCachedValue = true;
		}
	}

	LeaveCriticalSection(&m_cBestUnitCacheSection);

	if ( bHasCachedValue )
	{
		if ( UNITAI_CITY_DEFENSE == eUnitAI && eBestUnit == NO_UNIT && (criteria == NULL || criteria->m_eProperty == NO_PROPERTY) )
		{
			OutputDebugString("No buildable defender!!\n");
		}
		return eBestUnit;
	}

	iBestOriginalValue = 0;

	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

		if (eLoopUnit != NO_UNIT)
		{
			if (NO_ADVISOR == tempCriteria.m_eIgnoreAdvisor || (GC.getUnitInfo(eLoopUnit).getAdvisorType() != tempCriteria.m_eIgnoreAdvisor))
			{
				//	Koshling - this causes crashes with new code that asumes you will have some unit capable
				//	of UNITAI_CITY_DEFENDER becasuse early on you have nothing with this as default.  Since I cannot
				//	see why the human player wouldn't want an alternate that CAN do the requested AI, any less than
				//	the AI does I have decided to just remove this check for now
				//if (!isHuman() || (GC.getUnitInfo(eLoopUnit).getDefaultUnitAIType() == eUnitAI))
				{

					if (!(bGrowMore && isFoodProduction(eLoopUnit)))
					{
						if (canTrain(eLoopUnit))
						{
/********************************************************************************/
/* 	City Defenders						24.07.2010				Fuyu			*/
/********************************************************************************/
//Fuyu bIgnoreNotUnitAIs
							iOriginalValue = GET_PLAYER(getOwnerINLINE()).AI_unitValue(eLoopUnit, eUnitAI, area(), &tempCriteria);
/********************************************************************************/
/* 	City Defenders												END 			*/
/********************************************************************************/

							if (iOriginalValue > iBestOriginalValue)
							{
								iBestOriginalValue = iOriginalValue;
							}
						}
					}
				}
			}
		}
	}

	iBestValue = 0;
	eBestUnit = NO_UNIT;

	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

		if (eLoopUnit != NO_UNIT)
		{
			if (tempCriteria.m_eIgnoreAdvisor == NO_ADVISOR || (GC.getUnitInfo(eLoopUnit).getAdvisorType() != tempCriteria.m_eIgnoreAdvisor))
			{
				//	Koshling - this causes crashes with new code that asumes you will have some unit capable
				//	of UNITAI_CITY_DEFENDER becasuse early on you have nothing with this as default.  Since I cannot
				//	see why the human player wouldn't want an alternate that CAN do the requested AI, any less than
				//	the AI does I have decided to just remove this check for now
				//if (!isHuman() || (GC.getUnitInfo(eLoopUnit).getDefaultUnitAIType() == eUnitAI))
				{

					if (!(bGrowMore && isFoodProduction(eLoopUnit)))
					{
						if (canTrain(eLoopUnit))
						{
/********************************************************************************/
/* 	City Defenders						24.07.2010				Fuyu			*/
/********************************************************************************/
//Fuyu bIgnoreNotUnitAIs
							iValue = GET_PLAYER(getOwnerINLINE()).AI_unitValue(eLoopUnit, eUnitAI, area(), &tempCriteria);
/********************************************************************************/
/* 	City Defenders												END 			*/
/********************************************************************************/

							//	Koshling - can't assume anythign about the ratio of value to original value
							//	if we need a healer since the original didn't factor that in at all
							if ( (iValue > 0 && tempCriteria.m_bIsHealer) ||
								 (iValue > ((iBestOriginalValue * 2) / 3) && ((eUnitAI != UNITAI_EXPLORE) || (iValue >= iBestOriginalValue))))
							{
								int iHealAmount = 0;

								iValue *= (getProductionExperience(eLoopUnit) + 10);

								FAssert((MAX_INT / 1000) > iValue);	//	Allow order fo magnitude
								iValue *= 100;	//	Need it multiplying up so that truncation errors don't render
												//	modifiers irrelevant

								//	KOSHLING - this need rework to take actual promotion values.  May need some caching to do so at
								//	appropriate performance levels.  TODO ****


								//free promotions. slow?
								//only 1 promotion per source is counted (ie protective isn't counted twice)
								int iPromotionValue = 0;
								//buildings
								for (iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
								{
									if (isFreePromotion((PromotionTypes)iJ) && !GC.getUnitInfo(eLoopUnit).getFreePromotions((PromotionTypes)iJ))
									{
										if ((GC.getUnitInfo(eLoopUnit).getUnitCombatType() != NO_UNITCOMBAT) && GC.getPromotionInfo((PromotionTypes)iJ).getUnitCombat(GC.getUnitInfo(eLoopUnit).getUnitCombatType()))
										{
											iPromotionValue += 15;

											iHealAmount += 3*GC.getPromotionInfo((PromotionTypes)iJ).getAdjacentTileHealChange() + GC.getPromotionInfo((PromotionTypes)iJ).getSameTileHealChange();
											break;
										}
									}
								}
								//TB SubCombat Mod Begin
								UnitCombatTypes eSubCombatType;

								for (iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
								{
									if (isFreePromotion((PromotionTypes)iJ) && !GC.getUnitInfo(eLoopUnit).getFreePromotions((PromotionTypes)iJ))
									{
										for (iK = 0; iK < GC.getUnitInfo(eLoopUnit).getNumSubCombatTypes(); iK++)
										{
											eSubCombatType = ((UnitCombatTypes)GC.getUnitInfo(eLoopUnit).getSubCombatType(iK));
											if (GC.getPromotionInfo((PromotionTypes)iJ).getUnitCombat(eSubCombatType))
											{
												iPromotionValue += 15;

												iHealAmount += 3*GC.getPromotionInfo((PromotionTypes)iJ).getAdjacentTileHealChange() + GC.getPromotionInfo((PromotionTypes)iJ).getSameTileHealChange();
												break;
											}
										}
									}
								}
								//TB SubCombat Mod End
								//special to the unit
								for (iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
								{
									if (GC.getUnitInfo(eLoopUnit).getFreePromotions(iJ))
									{
										iPromotionValue += 15;

										iHealAmount += 3*GC.getPromotionInfo((PromotionTypes)iJ).getAdjacentTileHealChange() + GC.getPromotionInfo((PromotionTypes)iJ).getSameTileHealChange();
										break;
									}
								}

								//traits
/************************************************************************************************/
/* Afforess					  Start		 08/26/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
/*								for (iJ = 0; iJ < GC.getNumTraitInfos(); iJ++)
								{
									if (hasTrait((TraitTypes)iJ))
									{
										for (iK = 0; iK < GC.getNumPromotionInfos(); iK++)
										{
											if (GC.getTraitInfo((TraitTypes) iJ).isFreePromotion(iK))
											{
												if ((GC.getUnitInfo(eLoopUnit).getUnitCombatType() != NO_UNITCOMBAT) && GC.getTraitInfo((TraitTypes) iJ).isFreePromotionUnitCombat(GC.getUnitInfo(eLoopUnit).getUnitCombatType()))
												{
													iPromotionValue += 15;
													break;
												}
											}
										}
									}
								}
*/
								if (GC.getUnitInfo(eLoopUnit).getUnitCombatType() != NO_UNITCOMBAT)
								{
									for (iJ = 0; iJ < GC.getNumTraitInfos(); iJ++)
									{
										if (hasTrait((TraitTypes)iJ))
										{
											for (iK = 0; iK < GC.getNumPromotionInfos(); iK++)
											{
												if (GC.getTraitInfo((TraitTypes) iJ).isFreePromotionUnitCombats(iK, GC.getUnitInfo(eLoopUnit).getUnitCombatType()))
												{
													iPromotionValue += 15;
												}
											}
										}
									}
								}
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/

								if ( tempCriteria.m_bIsHealer && iHealAmount == 0 )
								{
									continue;
								}

								iValue *= (iPromotionValue + 100);
								iValue /= 100;

								if ( !bNoRand )
								{
									if (bAsync)
									{
										iValue *= (GC.getASyncRand().get(50, "AI Best Unit ASYNC") + 100);
										iValue /= 100;
									}
									else
									{
										iValue *= (getCitySorenRandNum(50, "AI Best Unit") + 100);
										iValue /= 100;
									}
								}

								int iBestHappy = 0;
								for (int iHurry = 0; iHurry < GC.getNumHurryInfos(); ++iHurry)
								{
									if (canHurryUnit((HurryTypes)iHurry, eLoopUnit, true))
									{
										int iHappy = AI_getHappyFromHurry((HurryTypes)iHurry, eLoopUnit, true);
										if (iHappy > iBestHappy)
										{
											iBestHappy = iHappy;
										}
									}
								}

								if (0 == iBestHappy)
								{
									iValue += getUnitProduction(eLoopUnit);
								}

								iValue *= (GET_PLAYER(getOwnerINLINE()).getNumCities() * 2);
								iValue /= std::max(1,GET_PLAYER(getOwnerINLINE()).getUnitClassCountPlusMaking((UnitClassTypes)iI) + GET_PLAYER(getOwnerINLINE()).getNumCities()+1);

								bool bIsSuicide = GC.getUnitInfo(eLoopUnit).isSuicide();

								if (bIsSuicide)
								{
									//much of this is compensated
									iValue /= 3;
								}

								if (0 == iBestHappy)
								{
									iValue /= std::max(1, (getProductionTurnsLeft(eLoopUnit, 0) + (GC.getUnitInfo(eLoopUnit).isSuicide() ? 1 : 4)));
								}
								else
								{
									iValue *= (2 + 3 * iBestHappy);
									iValue /= 100;
								}

								iValue = std::max(1, iValue);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									eBestUnit = eLoopUnit;
								}
							}
						}
					}
				}
			}
		}
	}

	if ( !bGrowMore && !bAsync )
	{
		UnitValueInfo unitValueInfo;

		unitValueInfo.eUnit = eBestUnit;
		unitValueInfo.iValue = iBestValue;

		EnterCriticalSection(&m_cBestUnitCacheSection);

		m_bestUnits.insert(std::make_pair(cacheKey, unitValueInfo));

		LeaveCriticalSection(&m_cBestUnitCacheSection);
	}
	if ( UNITAI_CITY_DEFENSE == eUnitAI && eBestUnit == NO_UNIT && (criteria == NULL || criteria->m_eProperty == NO_PROPERTY) )
	{
		OutputDebugString("No buildable defender!!\n");
	}

	return eBestUnit;
}


BuildingTypes CvCityAI::AI_bestBuilding(int iFocusFlags, int iMaxTurns, bool bAsync, AdvisorTypes eIgnoreAdvisor, bool bMaximizeFlaggedValue)
{
	return AI_bestBuildingThreshold(iFocusFlags, iMaxTurns, /*iMinThreshold*/ 0, bAsync, eIgnoreAdvisor, bMaximizeFlaggedValue);
}

BuildingTypes CvCityAI::AI_bestBuildingThreshold(int iFocusFlags, int iMaxTurns, int iMinThreshold, bool bAsync, AdvisorTypes eIgnoreAdvisor, bool bMaximizeFlaggedValue)
{
	PROFILE_FUNC();

	BuildingTypes eLoopBuilding;
	BuildingTypes eBestBuilding;
	bool bAreaAlone;
	int iProductionRank;
	int iValue;
	int iTempValue;
	int iBestValue;
	int iI, iJ;

	bAreaAlone = GET_PLAYER(getOwnerINLINE()).AI_isAreaAlone(area());

	iProductionRank = findYieldRateRank(YIELD_PRODUCTION);

	iBestValue = 0;
	eBestBuilding = NO_BUILDING;

	if (iFocusFlags & BUILDINGFOCUS_CAPITAL)
	{
		int iBestTurnsLeft = iMaxTurns > 0 ? iMaxTurns : MAX_INT;
		for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
		{
			eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)));

			if (NO_BUILDING != eLoopBuilding)
			{
				CvBuildingInfo& kBuilding = GC.getBuildingInfo(eLoopBuilding);

				if (kBuilding.isCapital())
				{
					if (canConstruct(eLoopBuilding))
					{
						int iTurnsLeft = getProductionTurnsLeft(eLoopBuilding, 0);

						if (iTurnsLeft <= iBestTurnsLeft)
						{
							eBestBuilding = eLoopBuilding;
							iBestTurnsLeft = iTurnsLeft;
						}
					}
				}
			}
		}

		return eBestBuilding;
	}

	for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		if (!(GET_PLAYER(getOwnerINLINE()).isBuildingClassMaxedOut(((BuildingClassTypes)iI), GC.getBuildingClassInfo((BuildingClassTypes)iI).getExtraPlayerInstances())))
		{
			eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)));

			if ((eLoopBuilding != NO_BUILDING) && (getNumBuilding(eLoopBuilding) < GC.getCITY_MAX_NUM_BUILDINGS())
			&&  (!isProductionAutomated() || !(isWorldWonderClass((BuildingClassTypes)iI) || isNationalWonderClass((BuildingClassTypes)iI))))
			{
				//don't build wonders?
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  05/08/09								jdog5000	  */
/*																							  */
/* City AI																					  */
/************************************************************************************************/
				// BBAI TODO: Temp testing, remove once centralized building is working
				//bool bWonderOk = false;
				//if( isHuman() || getOwner()%2 == 1 )
				//{
				bool bWonderOk = ((iFocusFlags == 0) || (iFocusFlags & BUILDINGFOCUS_WONDEROK) || (iFocusFlags & BUILDINGFOCUS_WORLDWONDER));
				//}

				if( bWonderOk ||
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
					!(isWorldWonderClass((BuildingClassTypes)iI) ||
					 isTeamWonderClass((BuildingClassTypes)iI) ||
					  isNationalWonderClass((BuildingClassTypes)iI) ||
					   isLimitedWonderClass((BuildingClassTypes)iI)))
				{
					if ((eIgnoreAdvisor == NO_ADVISOR) || (GC.getBuildingInfo(eLoopBuilding).getAdvisorType() != eIgnoreAdvisor))
					{
						if (canConstruct(eLoopBuilding))
						{
							if (isProductionAutomated() && GC.getBuildingInfo(eLoopBuilding).getPrereqNumOfBuildingClass(NO_BUILDINGCLASS) > 0)
							{
								iValue = 0;
							}
							else
							{
								bool bEnablesOtherBuildings = GC.getBuildingInfo(eLoopBuilding).EnablesOtherBuildings();
								BuildingTypes eFreeAreaBuilding = NO_BUILDING;
								BuildingTypes eFreeTradeRegionBuilding = NO_BUILDING;

								if (GC.getBuildingInfo(eLoopBuilding).getFreeAreaBuildingClass() != NO_BUILDINGCLASS)
								{
									eFreeAreaBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBuildingInfo(eLoopBuilding).getFreeAreaBuildingClass());
								}
								if (GC.getBuildingInfo(eLoopBuilding).getFreeTradeRegionBuildingClass() != NO_BUILDINGCLASS)
								{
									eFreeTradeRegionBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBuildingInfo(eLoopBuilding).getFreeTradeRegionBuildingClass());
								}

								iValue = AI_buildingValueThreshold(eLoopBuilding, iFocusFlags, iMinThreshold, bMaximizeFlaggedValue);

								if (isNationalWonderClass((BuildingClassTypes)iI))
								{								
									iValue += 10;	//45deg: a little boost for National Wonders
									if ((GET_PLAYER(getOwnerINLINE()).getCurrentEra()) > 4)   //45deg: boost for later eras when more NW can be built
									{	
										iValue += 300;
									}
								}								
								
								if (!GC.getGameINLINE().isNoNukes())
								{
									if (GC.getBuildingInfo(eLoopBuilding).isAllowsNukes())
									{								
										iValue += GC.getBuildingInfo(eLoopBuilding).getAIWeight();
									}
								}
								
								if (GC.getBuildingInfo(eLoopBuilding).getFreeBonus() != NO_BONUS)	//45deg: added so that AI considers building National Wonders like Aluminum Factory
								{
									iValue += GC.getBuildingInfo(eLoopBuilding).getAIWeight();
									if (GET_PLAYER(getOwnerINLINE()).getBuildingClassMaking((BuildingClassTypes)(GC.getBuildingInfo(eLoopBuilding).getBuildingClassType())) > 0)  //45deg: added this hopefully to prevent building the same NW at the same time in more cities
									{
										iValue = 0;
									}									
									else
									{
										int iLoop = 0;
										int iCount = 0;
										for (CvCity* pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
										{
											if ( pLoopCity->getNumBuilding(eLoopBuilding) != 0 )
											{
												iCount++;
											}
										}									
										if (iCount > 0)
										{
											iValue -= GC.getBuildingInfo(eLoopBuilding).getAIWeight()/3;	//45deg: if one NW of this kind has already been built, reduce its value
										}
									}
								}
								
								// DarkLunaPhantom - FreeBuilding accepts lists.
								int iNumFreeBuildingClass = GC.getBuildingInfo(eLoopBuilding).getNumFreeBuildingClass();
								
								for (int i = 0; i < iNumFreeBuildingClass; ++i)
								{
									BuildingTypes eFreeBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBuildingInfo(eLoopBuilding).getFreeBuildingClass(i));
									if (NO_BUILDING != eFreeBuilding)
									{
										iValue += (AI_buildingValue(eFreeBuilding, iFocusFlags) * (GET_PLAYER(getOwnerINLINE()).getNumCities() - GET_PLAYER(getOwnerINLINE()).getBuildingClassCountPlusMaking((BuildingClassTypes)GC.getBuildingInfo(eLoopBuilding).getFreeBuildingClass(i))));
									}
								}
								// DarkLunaPhantom - Extra FreeBuilding entries (cf. BuildingSchema xml).
								int iNumExtraFreeBuildingClass = GC.getBuildingInfo(eLoopBuilding).getNumExtraFreeBuildingClass();
								for (int i = 0; i < iNumExtraFreeBuildingClass; ++i)
								{
									BuildingTypes eFreeBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBuildingInfo(eLoopBuilding).getExtraFreeBuildingClass(i));
									if (NO_BUILDING != eFreeBuilding)
									{
										iValue += (AI_buildingValue(eFreeBuilding, iFocusFlags) * (GET_PLAYER(getOwnerINLINE()).getNumCities() - GET_PLAYER(getOwnerINLINE()).getBuildingClassCountPlusMaking((BuildingClassTypes)GC.getBuildingInfo(eLoopBuilding).getExtraFreeBuildingClass(i))));
									}
								}

								if ( gCityLogLevel > 3 )
								{
									logBBAIForTeam(getTeam(), "City %S base value for %S (flags %08lx)=%d",
											 getName().GetCString(),
											 GC.getBuildingInfo(eLoopBuilding).getDescription(),
											 iFocusFlags,
											 iValue);
								}

								//if this new building replaces an old one, subtract the old value.
								if ( iValue > 0 )
								{
									PROFILE("CvCityAI::AI_bestBuildingThreshold.Replacement");

									for (iJ = 0; iJ < GC.getNumBuildingClassInfos(); iJ++)
									{
										BuildingTypes eJLoopBuilding = ((BuildingTypes)(GC.getBuildingClassInfo((BuildingClassTypes)iJ).getDefaultBuildingIndex()));
										if (eJLoopBuilding != NO_BUILDING)
										{
											if (GC.getBuildingInfo(eJLoopBuilding).isReplaceBuildingClass(iI))
											{
												if (getNumBuilding(eJLoopBuilding) > 0)
												{
													PROFILE("AI_bestBuildingThreshold.Replace");

													iValue -= AI_buildingValueThreshold(eJLoopBuilding, iFocusFlags, iMinThreshold, bMaximizeFlaggedValue, true);

													if ( gCityLogLevel > 3 )
													{
														logBBAIForTeam(getTeam(), "	replaces %S - reduce value to %d",
																 GC.getBuildingInfo(eJLoopBuilding).getDescription(),
																 iValue);
													}
												}
											}
										}
									}
								}

								if ( bEnablesOtherBuildings )
								{
									PROFILE("CvCityAI::AI_bestBuildingThreshold.EnablesOthers");

									for (iJ = 0; iJ < GC.getNumBuildingInfos(); iJ++)
									{
										if (GC.getBuildingInfo((BuildingTypes)iJ).isBuildingClassNeededInCity(iI) ||
											GC.getBuildingInfo((BuildingTypes)iJ).isPrereqOrBuildingClass(iI))
										{
											if ( getNumBuilding((BuildingTypes)iJ) == 0 &&
												 canConstructInternal((BuildingTypes)iJ, false, false, false, true, (BuildingClassTypes)iI) )
											{
												PROFILE("AI_bestBuildingThreshold.Enablement");

												iValue += AI_buildingValueThreshold((BuildingTypes)iJ, iFocusFlags, 0, false, true) / 2;

												if ( gCityLogLevel > 3 )
												{
													logBBAIForTeam(getTeam(), "	enables %S - increase value to %d",
															 GC.getBuildingInfo((BuildingTypes)iJ).getDescription(),
															 iValue);
												}
											}
										}
									}
								}

								if (NO_BUILDING != eFreeAreaBuilding)
								{
									int weighting = GET_PLAYER(getOwnerINLINE()).getNumCities() - GET_PLAYER(getOwnerINLINE()).getBuildingClassCountPlusMaking((BuildingClassTypes)GC.getBuildingInfo(eLoopBuilding).getFreeAreaBuildingClass());

									weighting = (100*weighting*area()->getCitiesPerPlayer(getOwnerINLINE()))/GET_PLAYER(getOwnerINLINE()).getNumCities();
									if ( getNumBuilding(eFreeAreaBuilding) == 0 )
									{
										weighting++;
									}
									iValue += (AI_buildingValue(eFreeAreaBuilding, iFocusFlags) * weighting)/100;
								}

								if (NO_BUILDING != eFreeTradeRegionBuilding)
								{
									int weighting = GET_PLAYER(getOwnerINLINE()).getNumCities() - GET_PLAYER(getOwnerINLINE()).getBuildingClassCountPlusMaking((BuildingClassTypes)GC.getBuildingInfo(eLoopBuilding).getFreeTradeRegionBuildingClass());
									int numPlotGroupCities = (plot()->getOwnerPlotGroup() == NULL) ? 1 : plot()->getOwnerPlotGroup()->getNumCities();

									weighting = (100*weighting*numPlotGroupCities)/GET_PLAYER(getOwnerINLINE()).getNumCities();
									if ( getNumBuilding(eFreeTradeRegionBuilding) == 0 )
									{
										weighting++;
									}
									iValue += (AI_buildingValue(eFreeTradeRegionBuilding, iFocusFlags) * weighting)/100;
								}
							}

							if ( gCityLogLevel > 3 )
							{
								logBBAIForTeam(getTeam(), "	final value %d",
										 iValue);
							}

							if (iValue > 0)
							{
								int iTurnsLeft = getProductionTurnsLeft(eLoopBuilding, 0);

								if (isWorldWonderClass((BuildingClassTypes)iI))
								{
									if (iProductionRank <= std::min(3, ((GET_PLAYER(getOwnerINLINE()).getNumCities() + 2) / 3)))
									{
										if (bAsync)
										{
/************************************************************************************************/
/* Afforess					  Start		 06/30/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
/*
											iTempValue = GC.getASyncRand().get(GC.getLeaderHeadInfo(getPersonalityType()).getWonderConstructRand(), "Wonder Construction Rand ASYNC");
*/
											iTempValue = GC.getASyncRand().get(GET_PLAYER(getOwnerINLINE()).getWonderConstructRand(), "Wonder Construction Rand ASYNC");
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/

										}
										else
										{
/************************************************************************************************/
/* Afforess					  Start		 06/30/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
/*
											iTempValue = getCitySorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getWonderConstructRand(), "Wonder Construction Rand");
*/
											iTempValue = getCitySorenRandNum(GET_PLAYER(getOwnerINLINE()).getWonderConstructRand(), "Wonder Construction Rand");
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
										}

										if (bAreaAlone)
										{
											iTempValue *= 2;
										}
										iValue += iTempValue;
									}
								}

								if (bAsync)
								{
									iValue *= (GC.getASyncRand().get(25, "AI Best Building ASYNC") + 100);
									iValue /= 100;
								}
								else
								{
									iValue *= (getCitySorenRandNum(25, "AI Best Building") + 100);
									iValue /= 100;
								}

								iValue += getBuildingProduction(eLoopBuilding);

								bool bValid = ((iMaxTurns <= 0) ? true : false);
								if (!bValid)
								{
									bValid = (iTurnsLeft <= GC.getGameINLINE().AI_turnsPercent(iMaxTurns, GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent()));
								}
								if (!bValid)
								{
									for (int iHurry = 0; iHurry < GC.getNumHurryInfos(); ++iHurry)
									{
										if (canHurryBuilding((HurryTypes)iHurry, eLoopBuilding, true))
										{
											if (AI_getHappyFromHurry((HurryTypes)iHurry, eLoopBuilding, true) > 0)
											{
												bValid = true;
												break;
											}
										}
									}
								}

								if (bValid)
								{
									FAssert((MAX_INT / 1000) > iValue);
									iValue *= 1000;
									iValue /= std::max(1,iTurnsLeft+3);
									iValue = std::max(1, iValue);

									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										eBestBuilding = eLoopBuilding;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return eBestBuilding;
}


int CvCityAI::AI_buildingValue(BuildingTypes eBuilding, int iFocusFlags, bool bForTech)
{
	if (bForTech)
	{
		PROFILE("AI_buildingValue.ForTech");

		return AI_buildingValueThresholdOriginal(eBuilding, iFocusFlags, 0, false, false, bForTech);
	}
	return AI_buildingValueThreshold(eBuilding, iFocusFlags, 0);
}

int CvCityAI::AI_buildingValueThreshold(BuildingTypes eBuilding, int iFocusFlags, int iThreshold, bool bMaximizeFlaggedValue, bool bIgnoreCanConstruct)
{
	PROFILE_FUNC();

	return GetBuildingValue(eBuilding, iFocusFlags, iThreshold, bMaximizeFlaggedValue, bIgnoreCanConstruct);
}

// XXX should some of these count cities, buildings, etc. based on teams (because wonders are shared...)
// XXX in general, this function needs to be more sensitive to what makes this city unique (more likely to build airports if there already is a harbor...)
//
// KOSHLING - this routine is no longer used for most building evaluations durign city production
// determination.  For efficiency reasons this now pre-calculates most of the needed values and then
// assembles the final result fom cached partial results.  For this reason if you are making chnages to this routine
// you must also make equivalent changes to CalculateAllBuildingValues() also in this file (it mirriors the calculations
// in a fairly obvious and direct way)
int CvCityAI::AI_buildingValueThresholdOriginal(BuildingTypes eBuilding, int iFocusFlags, int iThreshold, bool bMaximizeFlaggedValue, bool bIgnoreCanBuildReplacement, bool bForTech)
{
	PROFILE_FUNC();

	// We keep a small cache of results for cases not involved in city production (that therefore still use this routine)
	CvChecksum cacheKey;

	cacheKey.add((int)eBuilding);
	cacheKey.add(iFocusFlags);
	cacheKey.add(iThreshold);
	cacheKey.add(bMaximizeFlaggedValue);
	cacheKey.add(bIgnoreCanBuildReplacement);
	cacheKey.add(bForTech);

	std::map<int,int>::const_iterator itr = m_buildValueCache.find(cacheKey.get());
	if (itr != m_buildValueCache.end())
	{
		return itr->second;
	}
	int iResult = AI_buildingValueThresholdOriginalUncached(eBuilding, iFocusFlags, iThreshold, bMaximizeFlaggedValue, bIgnoreCanBuildReplacement, bForTech);

	m_buildValueCache.insert(std::make_pair(cacheKey.get(), iResult));

	return iResult;
}

int CvCityAI::AI_buildingValueThresholdOriginalUncached(BuildingTypes eBuilding, int iFocusFlags, int iThreshold, bool bMaximizeFlaggedValue, bool bIgnoreCanBuildReplacement, bool bForTech)
{
	PROFILE_FUNC();

	MEMORY_TRACK();

	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);
	BuildingClassTypes eBuildingClass = (BuildingClassTypes) kBuilding.getBuildingClassType();
	int iLimitedWonderLimit = limitedWonderClassLimit(eBuildingClass);
	bool bIsLimitedWonder = (iLimitedWonderLimit >= 0);

	ReligionTypes eStateReligion = kOwner.getStateReligion();

	bool bAreaAlone = kOwner.AI_isAreaAlone(area());

	int iHasMetCount = GET_TEAM(getTeam()).getHasMetCivCount(true);

	int iFoodDifference = foodDifference(false);

	// Reduce reaction to espionage induced happy/health problems
	int iHappinessLevel = happyLevel() - unhappyLevel(1) + getEspionageHappinessCounter()/2;
	int iAngryPopulation = range(-iHappinessLevel, 0, (getPopulation() + 1));
	int iHealthLevel = goodHealth() - badHealth(/*bNoAngry*/ false, std::max(0, (iHappinessLevel + 1) / 2)) + getEspionageHealthCounter()/2;
	int iBadHealth = std::max(0, -iHealthLevel);

	int iHappyModifier = (iHappinessLevel <= iHealthLevel && iHappinessLevel <= 6) ? 6 : 3;
	int iHealthModifier = (iHealthLevel < iHappinessLevel && iHealthLevel <= 4) ? 4 : 2;
	if (iHappinessLevel >= 10)
	{
		iHappyModifier = 1;
	}
	if (iHealthLevel >= 8)
	{
		iHealthModifier = 0;
	}

	int iGoldValueAssessmentModifier = GET_PLAYER(getOwnerINLINE()).AI_goldValueAssessmentModifier();

	bool bProvidesPower = (kBuilding.isPower() || ((kBuilding.getPowerBonus() != NO_BONUS) && hasBonus((BonusTypes)(kBuilding.getPowerBonus()))) || kBuilding.isAreaCleanPower());

	/********************************************************************************/
	/* 	Alternative Building Evaluation				20.02.2010		Fuyu			*/
	/* 	 - requires Actual Building Effects (BULL)									*/
	/* 																				*/
	/* 	TODO:																		*/
	/* 		value riverPlotYieldChanges higher when we are already working on river	*/
	/* 		add getBuildingYieldChange() ?											*/
	/* 		more consistancy (values from yield changes should not differ so much	*/
	/* 			depending on where they come from)									*/
	/********************************************************************************/
	// Don't consider a building if it causes the city to immediately start shrinking from unhealthiness
	// For that purpose ignore bad health and unhappiness from Espionage.
	int iBuildingActualHappiness = getAdditionalHappinessByBuilding(eBuilding);
	int iBuildingActualHealth = getAdditionalHealthByBuilding(eBuilding);
	int iBaseHappinessLevel = happyLevel() - unhappyLevel() + getEspionageHappinessCounter();
	int iBaseHealthLevel = goodHealth() - badHealth() + getEspionageHealthCounter();
	int iBaseFoodDifference = getYieldRate(YIELD_FOOD) - getPopulation()*GC.getFOOD_CONSUMPTION_PER_POPULATION() - std::max(0,-iBaseHealthLevel);

	//Temporary unhappiness that will mostly not be ignored
	int iTemporaryUnhappiness = 0;
	int iAngerPercent = 0;
	iAngerPercent += getHurryPercentAnger();
	iAngerPercent += getConscriptPercentAnger();
	iAngerPercent += getDefyResolutionPercentAnger();
	iAngerPercent += getWarWearinessPercentAnger();
	iTemporaryUnhappiness += ((iAngerPercent * (getPopulation())) / GC.getPERCENT_ANGER_DIVISOR());

	int iBadHealthFromBuilding = std::max(0,(-iBuildingActualHealth));
	int iUnhealthyPopulationFromBuilding = std::min(0,(-iBaseHealthLevel)) + iBadHealthFromBuilding;
//	int iTotalHealth = iBaseHealthLevel + iBuildingActualHealth;
	bool bShrinksWithPower = false;
//Allow a bit of shrinking: Population is expendable if angry, working a bad tile, or running a not-so-good specialist
	int iAllowedShrinkRate = GC.getFOOD_CONSUMPTION_PER_POPULATION() * (0
		+ std::max(0,-iBaseHappinessLevel-iTemporaryUnhappiness)
		+ std::min(1, std::max(0,(getWorkingPopulation() - AI_countGoodTiles(true, false, 50))))
		+ std::max(0,(visiblePopulation() - AI_countGoodSpecialists(false))));

	if (iUnhealthyPopulationFromBuilding > 0 && (iBaseFoodDifference + iAllowedShrinkRate < iUnhealthyPopulationFromBuilding ))
	{
		return 0;
	}
	if (!bProvidesPower && !isPower()) //if the city is still without power after building this
	{
		const int iUnhealthFromBuildingPower = std::min(0,(badHealth() - goodHealth() - getEspionageHealthCounter())) + iBadHealthFromBuilding - GC.getDefineINT("DIRTY_POWER_HEALTH_CHANGE");
		if (iUnhealthFromBuildingPower > 0 && (iBaseFoodDifference + iAllowedShrinkRate < iUnhealthFromBuildingPower ))
		{
			bShrinksWithPower = true;
		}
	}

	int iTotalPopulation = kOwner.getTotalPopulation();
	int iNumCities = kOwner.getNumCities();
	int iNumCitiesInArea = area()->getCitiesPerPlayer(getOwnerINLINE());

	int aiYieldRank[NUM_YIELD_TYPES];
	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aiYieldRank[iI] = MAX_INT;
	}

	int aiCommerceRank[NUM_COMMERCE_TYPES];
	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		aiCommerceRank[iI] = MAX_INT;
	}

	int aiBaseCommerceRate[NUM_COMMERCE_TYPES];
	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		aiBaseCommerceRate[iI] = MAX_INT;
	}

	int aiPlayerCommerceRate[NUM_COMMERCE_TYPES];
	for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		aiPlayerCommerceRate[iI] = kOwner.getTotalCityBaseCommerceRate((CommerceTypes)iI);
	}

	aiYieldRank[YIELD_PRODUCTION] = findBaseYieldRateRank(YIELD_PRODUCTION);
	bool bIsHighProductionCity = (aiYieldRank[YIELD_PRODUCTION] <= std::max(3, (iNumCities / 2)));

	int iCultureRank = findCommerceRateRank(COMMERCE_CULTURE);
	int iCulturalVictoryNumCultureCities = GC.getGameINLINE().culturalVictoryNumCultureCities();

	bool bFinancialTrouble = GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble();

	bool bCulturalVictory1 = GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE1);
	bool bCulturalVictory2 = GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2);
	bool bCulturalVictory3 = GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3);

	bool bSpaceVictory1 = GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_SPACE1);

	int iSpecialistGreatPeopleRate = 0;
	int iSpecialistExtraHealth = 0;
	int iSpecialistExtraHappy = 0;


	int* paiFreeSpecialistCommerce = new int[NUM_COMMERCE_TYPES];
	int* paiFreeSpecialistYield = new int[NUM_YIELD_TYPES];
	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		paiFreeSpecialistYield[iI] = 0;
	}
	for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		paiFreeSpecialistCommerce[iI] = 0;
	}

	if (kBuilding.getNumPopulationEmployed() > 0)
	{
		removeWorstCitizenActualEffects(kBuilding.getNumPopulationEmployed(), iSpecialistGreatPeopleRate, iSpecialistExtraHappy, iSpecialistExtraHealth, paiFreeSpecialistYield, paiFreeSpecialistCommerce);
	}

	int aiFreeSpecialistYield[NUM_YIELD_TYPES];
	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aiFreeSpecialistYield[iI] = paiFreeSpecialistYield[iI];
	}

	int aiFreeSpecialistCommerce[NUM_COMMERCE_TYPES];
	for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		aiFreeSpecialistCommerce[iI] = paiFreeSpecialistCommerce[iI];
	}

	SAFE_DELETE_ARRAY(paiFreeSpecialistCommerce);
	SAFE_DELETE_ARRAY(paiFreeSpecialistYield);

	for (int iI = 1; iI < kBuilding.getFreeSpecialist() + 1; iI++)
	{
		SpecialistTypes eNewSpecialist = getBestSpecialist(iI);
		if (eNewSpecialist == NO_SPECIALIST) break;
		CvSpecialistInfo& kSpecialist = GC.getSpecialistInfo(eNewSpecialist);
		for (int iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
		{
			aiFreeSpecialistYield[iJ] += GET_PLAYER(getOwnerINLINE()).specialistYield(eNewSpecialist, (YieldTypes)iJ);
		}
		for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
		{
			aiFreeSpecialistCommerce[iJ] += GET_PLAYER(getOwnerINLINE()).specialistCommerce(eNewSpecialist, (CommerceTypes)iJ);
		}
		iSpecialistGreatPeopleRate += kSpecialist.getGreatPeopleRateChange();
		iSpecialistExtraHealth += kSpecialist.getHealthPercent();
		iSpecialistExtraHappy += kSpecialist.getHappinessPercent();
	}
	iSpecialistExtraHappy /= 100;
	iSpecialistExtraHealth /= 100;
	iBuildingActualHappiness += iSpecialistExtraHappy;
	iBuildingActualHealth += iSpecialistExtraHealth;

	bool bCanPopRush = GET_PLAYER(getOwnerINLINE()).canPopRush();

	bool bForeignTrade = false;
	int iNumTradeRoutes = getTradeRoutes();
	for (int iI = 0; iI < iNumTradeRoutes; ++iI)
	{
		CvCity* pTradeCity = getTradeCity(iI);
		if (NULL != pTradeCity
		&& (GET_PLAYER(pTradeCity->getOwnerINLINE()).getTeam() != getTeam() || pTradeCity->area() != area()))
		{
			bForeignTrade = true;
			break;
		}
	}

	if (kBuilding.isCapital())
	{
		return 0;
	}

/************************************************************************************************/
/* Afforess					  Start		 01/24/10											   */
/*																							  */
/* If the AI can build the replacement building...											  */
/************************************************************************************************/
	if (!bIgnoreCanBuildReplacement)
	{
		for (int iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
		{
			if (kBuilding.isReplaceBuildingClass((BuildingClassTypes)iI))
			{
				const BuildingTypes eBuildingX = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI);
				if (eBuildingX != NO_BUILDING && canConstruct(eBuildingX))
				{
					return 0;
				}
			}
		}
	}

	for (int iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		if (kBuilding.getReligionChange(iI) > 0 && !GET_TEAM(getTeam()).hasHolyCity((ReligionTypes)iI))
		{
			return 0;
		}
	}
	int iTempValue;

	int iValue = 0;
	int iPass1Value;

	for (int iPass = 0; iPass < 2; iPass++)
	{
		if (iPass == 1)
		{
			iPass1Value = iValue;
		}

		if (iFocusFlags == 0 || iValue > 0 || iPass == 0)
		{
			if (((iFocusFlags & BUILDINGFOCUS_WORLDWONDER) || iPass > 0)
			&& isWorldWonderClass(eBuildingClass) && aiYieldRank[YIELD_PRODUCTION] <= 3)
			{
				iValue++;
			}

			if ((iFocusFlags & BUILDINGFOCUS_DEFENSE) || (iPass > 0))
			{
				if (!bAreaAlone)
				{
					if ((GC.getGameINLINE().getBestLandUnit() == NO_UNIT) || !(GC.getUnitInfo(GC.getGameINLINE().getBestLandUnit()).isIgnoreBuildingDefense()))
					{
						iValue += (std::max(0, std::min(((kBuilding.getDefenseModifier() + getBuildingDefense()) - getNaturalDefense() - 10), kBuilding.getDefenseModifier())) / 4);

						for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
						{
							if (hasBonus((BonusTypes)iI))
							{
								iValue += (kBuilding.getBonusDefenseChanges(iI) / 4);
							}
						}
					}


					iValue += kBuilding.isZoneOfControl() ? 50 : 0;

					iValue += kBuilding.getLineOfSight() * 15;

					for (int iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
					{
						if (kBuilding.getUnitCombatExtraStrength(iI) != 0)
						{
							int iValidUnitCount = 0;
							CLLNode<IDInfo>* pUnitNode;
							CvUnit* pLoopUnit;
							pUnitNode = plot()->headUnitNode();
							while (pUnitNode != NULL)
							{
								pLoopUnit = ::getUnit(pUnitNode->m_data);
								pUnitNode = plot()->nextUnitNode(pUnitNode);
								if (pLoopUnit->getTeam() == getTeam())
								{
									if (pLoopUnit->getUnitCombatType() == iI)
									{
										iValidUnitCount++;
									}
								}
							}
							iValue += iValidUnitCount * kBuilding.getUnitCombatExtraStrength(iI) / 6;
						}
					}

					bool bDefense = (area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE);
					bool bLandWar = (bDefense || (area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING));
					bool bDanger = AI_isDanger();

					if (bDanger || bLandWar)
					{
						iValue += kBuilding.getAdjacentDamagePercent() * 10;
					}

					iValue -= kBuilding.getInvasionChance() * 5;

					iValue += kBuilding.isProtectedCulture() ? 50 : 0;
					iValue += kBuilding.getOccupationTimeModifier() / 20;

					if (kBuilding.getNoEntryDefenseLevel() > 0 && kBuilding.getNoEntryDefenseLevel() < getTotalDefense(false))
					{
						iValue += getTotalDefense(false) - kBuilding.getNoEntryDefenseLevel() / 2;
					}
					if (kBuilding.getNumUnitFullHeal() > 0)
					{
						iValue += kBuilding.getNumUnitFullHeal() * 50;
					}
				}

				iValue += kBuilding.getBombardDefenseModifier() / 8;

				iValue -= kBuilding.getAirModifier() / 4;
				iValue -= kBuilding.getNukeModifier() / 4;

				iValue += kBuilding.getAllCityDefenseModifier() * iNumCities / 5;

				iValue += kBuilding.getAirlift() * 25;

				if (kBuilding.isAreaBorderObstacle() && !GC.getGameINLINE().isOption(GAMEOPTION_NO_BARBARIANS))
				{
					//The great wall is much more valuable with more barbarian activity.
					iValue += iNumCitiesInArea;
					if(GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS))
						iValue += iNumCitiesInArea;
					if (GC.getGameINLINE().isOption(GAMEOPTION_BARBARIAN_WORLD))
						iValue += iNumCitiesInArea;
					if (GC.getGameINLINE().isOption(GAMEOPTION_BARBARIAN_GENERALS))
						iValue += iNumCitiesInArea;
				}
			}

			if ((iFocusFlags & BUILDINGFOCUS_ESPIONAGE) || (iPass > 0))
			{
				iValue += kBuilding.getEspionageDefenseModifier() / 8;
			}

			//	If we're evaluating a builidng we already have (e.g. - for civic enabling/disabling)
			//	and it gives no unhealthy and that's the reason we have it, count it!
			bool bCountHappy = !isNoUnhappiness();

			if ( getNumBuilding(eBuilding) > 0 && kBuilding.isNoUnhappiness() )
			{
				bCountHappy = true;
			}

			if (((iFocusFlags & BUILDINGFOCUS_HAPPY) || (iPass > 0)) && bCountHappy)
			{
				int iBestHappy = 0;
				for (int iI = 0; iI < GC.getNumHurryInfos(); iI++)
				{
					if (canHurryBuilding((HurryTypes)iI, eBuilding, true))
					{
						int iHappyFromHurry = AI_getHappyFromHurry((HurryTypes)iI, eBuilding, true);
						if (iHappyFromHurry > iBestHappy)
						{
							iBestHappy = iHappyFromHurry;
						}
					}
				}
				iValue += iBestHappy * 10;

				//Fuyu ToDo: How to handle Globe Theater national wonder?
				//For now just give massive boost if city is high food yet not one of the main production or commerce cities
				if (kBuilding.isNoUnhappiness() && bIsLimitedWonder)
				{
					iValue += (iAngryPopulation * 10) + getPopulation();
					aiYieldRank[YIELD_FOOD] = findBaseYieldRateRank(YIELD_FOOD);
					aiYieldRank[YIELD_COMMERCE] = findBaseYieldRateRank(YIELD_COMMERCE);
					if (aiYieldRank[YIELD_FOOD] <= (2 + iLimitedWonderLimit))
					{
						if (!(isNationalWonderClass(eBuildingClass)) ||
							((aiYieldRank[YIELD_PRODUCTION] > (2 + iLimitedWonderLimit))
							&& (aiYieldRank[YIELD_COMMERCE] > (2 + iLimitedWonderLimit))))
						{
							iValue += getPopulation() * 5;
						}
					}
				}
				else
				{
					iValue += happynessValue(iBuildingActualHappiness, iBaseHappinessLevel, iBaseHealthLevel);
				}

				iValue += (-kBuilding.getHurryAngerModifier() * getHurryPercentAnger()) / 100;

				for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
				{
					iValue += (kBuilding.getCommerceHappiness(iI) * iHappyModifier) / 4;
				}

				int iWarWearinessModifer = kBuilding.getWarWearinessModifier();
				if (iWarWearinessModifer != 0)
				{
					if ( GET_TEAM(getTeam()).getAtWarCount(true) == 0 )
					{
						iWarWearinessModifer /= 2;
					}

					iValue += (-iWarWearinessModifer * iHappyModifier) / 16;
				}

				iValue += (kBuilding.getAreaHappiness() * (iNumCitiesInArea - 1) * 8);
				iValue += (kBuilding.getGlobalHappiness() * (iNumCities - 1) * 8);

				int iWarWearinessPercentAnger = kOwner.getWarWearinessPercentAnger();
				int iGlobalWarWearinessModifer = kBuilding.getGlobalWarWearinessModifier();
				if (iGlobalWarWearinessModifer != 0)
				{
					iValue += (-(((iGlobalWarWearinessModifer * iWarWearinessPercentAnger / 100) / GC.getPERCENT_ANGER_DIVISOR())) * (iNumCities - 1));
					iValue += (-iGlobalWarWearinessModifer * iHappyModifier) / 16;
				}

				CvCivilizationInfo& kCivilization = GC.getCivilizationInfo(getCivilizationType());
				for (int iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
				{
					int iBuildingHappinessChanges = kBuilding.getBuildingHappinessChanges(iI);
					if (iBuildingHappinessChanges != 0)
					{
						BuildingTypes eLoopBuilding = (BuildingTypes)kCivilization.getCivilizationBuildings(iI);
						if (eLoopBuilding != NO_BUILDING)
						{
							iValue += (iBuildingHappinessChanges * (kOwner.getBuildingClassCount((BuildingClassTypes)iI) - getNumBuilding(eLoopBuilding)) * 8);
						}
						else
						{
							iValue += (iBuildingHappinessChanges * kOwner.getBuildingClassCount((BuildingClassTypes)iI) * 8);
						}
					}
				}

				if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_REVOLUTION))
				{
					if (kBuilding.getRevIdxLocal() != 0)
					{
						int localRevIdx = getLocalRevIndex();

						//	Use the more serious of the before and after values if this building were to be built
						if ( kBuilding.getRevIdxLocal() > 0 )
						{
							localRevIdx += kBuilding.getRevIdxLocal();
						}

						//	Treat instability seriously as it goes up - not just linear
						const int localRevScaling = (localRevIdx < 0 ? 0 : std::min(localRevIdx*localRevIdx/50 + localRevIdx/2, 100));

						iValue -= kBuilding.getRevIdxLocal() * localRevScaling / 4;
					}
					if (kBuilding.getRevIdxNational() != 0)
					{
						iValue -= (8 * GET_PLAYER(getOwnerINLINE()).getNumCities()) * kBuilding.getRevIdxNational();
					}
					if (kBuilding.getRevIdxDistanceModifier() != 0 && !isCapital())
					{
						CvCity* pCapital = GET_PLAYER(getOwnerINLINE()).getCapitalCity();
						if (pCapital != NULL)
						{
							int iCapitalDistance = ::plotDistance(getX_INLINE(), getY_INLINE(), pCapital->getX_INLINE(), pCapital->getY_INLINE());
							int iOldCapitalDistance = iCapitalDistance;
							iCapitalDistance *= 100 + kBuilding.getRevIdxDistanceModifier();
							iCapitalDistance /= 100;

							iValue += ((iOldCapitalDistance - iCapitalDistance) * (10+std::max(0,getLocalRevIndex())));
						}
					}
				}
			}

			if (((iFocusFlags & BUILDINGFOCUS_HEALTHY) || iPass > 0)
			// If we're evaluating a building we already have (e.g. - for civic enabling/disabling)
			//	and it gives no unhealthy and that's the reason we have it, count it!
			&& (!isNoUnhealthyPopulation() || getNumBuilding(eBuilding) > 0 && kBuilding.isNoUnhealthyPopulation()))
			{
				iValue += (
					healthValue(
						iBuildingActualHealth,
						iBaseHappinessLevel - getEspionageHappinessCounter() / 2 + std::max(0, iBuildingActualHappiness),
						iBaseHealthLevel,
						iBaseFoodDifference
					)
					+ kBuilding.getAreaHealth() * (iNumCitiesInArea - 1 ) * 4
					+ kBuilding.getGlobalHealth() * (iNumCities - 1) * 4
				);
			}

			if ((iFocusFlags & BUILDINGFOCUS_EXPERIENCE) || (iPass > 0))
			{
				iValue += (kBuilding.getFreeExperience() * ((iHasMetCount > 0) ? 12 : 6));

				for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
				{
					if (canTrain((UnitCombatTypes)iI))
					{
						iValue += (kBuilding.getUnitCombatFreeExperience(iI) * ((iHasMetCount > 0) ? 6 : 3));
					}
				}

				for (int iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
				{
					int iDomainExpValue = 0;
					if (iI == DOMAIN_SEA)
					{
						iDomainExpValue = 7;
					}
					else if (iI == DOMAIN_LAND)
					{
						iDomainExpValue = 12;
					}
					else
					{
						iDomainExpValue = 6;
					}
					iValue += (kBuilding.getDomainFreeExperience(iI) * ((iHasMetCount > 0) ? iDomainExpValue : iDomainExpValue / 2));
				}
				iTempValue = 0;
				bool bWarPlan = (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0);
				if (kBuilding.getFreePromotion() != NO_PROMOTION)
				{
					iTempValue += AI_getPromotionValue((PromotionTypes)kBuilding.getFreePromotion());
				}
				if (kBuilding.getFreePromotion_2() != NO_PROMOTION)
				{
					iTempValue += AI_getPromotionValue((PromotionTypes)kBuilding.getFreePromotion());
				}
				if (kBuilding.getFreePromotion_3() != NO_PROMOTION)
				{
					iTempValue += AI_getPromotionValue((PromotionTypes)kBuilding.getFreePromotion());
				}
				if (kBuilding.isApplyFreePromotionOnMove())
				{
					iTempValue *= 2;
				}
				iValue += iTempValue;
				bool bDevelopingCity = isDevelopingCity();
				if ((!bDevelopingCity || isCapital()) && kBuilding.EnablesUnits())
				{
					for (int iI = 0; iI < GC.getNumUnitInfos(); iI++)
					{
						CvUnitInfo& kUnit = GC.getUnitInfo((UnitTypes)iI);
						bool bUnitIsEnabler = (kUnit.getPrereqBuilding() == eBuilding);

						if (
							GET_TEAM(getTeam()).isHasTech((TechTypes)kUnit.getPrereqAndTech())
						&&	(
								bUnitIsEnabler
								||
								kUnit.getPrereqBuilding() == NO_BUILDING
								||
								getNumBuilding((BuildingTypes)kUnit.getPrereqBuilding()) > 0
							)
						)
						{
							bool bUnitIsBonusEnabled = true;
							if (kUnit.getPrereqAndBonus() != NO_BONUS && !hasBonus((BonusTypes)kUnit.getPrereqAndBonus()))
							{
								if (isFreeBonusOfBuilding(kBuilding, (BonusTypes)kUnit.getPrereqAndBonus()))
								{
									bUnitIsEnabler = true;
								}
								else bUnitIsBonusEnabled = false;
							}
							bool bHasORBonusAlready = false;
							bool bFreeBonusIsORBonus = false;
							int iFreeExtraBonusCount = 0;

							for (int iK = 0; iK < GC.getNUM_UNIT_PREREQ_OR_BONUSES(); iK++)
							{
								BonusTypes eXtraFreeBonus = (BonusTypes)kUnit.getPrereqOrBonuses(iK);

								if (eXtraFreeBonus != NO_BONUS)
								{
									iFreeExtraBonusCount++;

									if (hasBonus(eXtraFreeBonus))
									{
										bHasORBonusAlready = true;
									}
									else if ( isFreeBonusOfBuilding(kBuilding, eXtraFreeBonus) )
									{
										bFreeBonusIsORBonus = true;
									}
								}
							}

							if (iFreeExtraBonusCount > 0 && !bHasORBonusAlready)
							{
								if (bFreeBonusIsORBonus)
								{
									bUnitIsEnabler = true;
								}
								else bUnitIsBonusEnabled = false;
							}

							if (bUnitIsEnabler)
							{
								PROFILE("CalculateAllBuildingValues.IsUnitPrereq");

								if (kUnit.getDefaultUnitAIType() != NO_UNITAI && kOwner.AI_totalAreaUnitAIs(area(), ((UnitAITypes)(kUnit.getDefaultUnitAIType()))) == 0)
								{
									iValue += iNumCitiesInArea;
								}
								//This forces the AI to build neccessary buildings for units.

								UnitAITypes eUnitAI = (UnitAITypes)(kUnit.getDefaultUnitAIType());

								iTempValue = 0;

								switch (eUnitAI)
								{
									case UNITAI_UNKNOWN:
									break;
									case UNITAI_ANIMAL:
										iTempValue += kUnit.getCombat() / 5;
										break;
									case UNITAI_SETTLE:
									case UNITAI_SETTLER_SEA:
										iTempValue += 25;
										break;
									case UNITAI_WORKER:
									case UNITAI_WORKER_SEA:
										iTempValue += std::max(0, AI_getWorkersNeeded()) * 10;
										break;
									case UNITAI_ATTACK:
									case UNITAI_ATTACK_CITY:
									case UNITAI_COLLATERAL:
									case UNITAI_PILLAGE:
									case UNITAI_RESERVE:
									case UNITAI_COUNTER:
									case UNITAI_CITY_DEFENSE:
									case UNITAI_CITY_COUNTER:
									case UNITAI_CITY_SPECIAL:
									case UNITAI_ATTACK_SEA:
									case UNITAI_RESERVE_SEA:
									case UNITAI_ESCORT_SEA:
									case UNITAI_ASSAULT_SEA:
									case UNITAI_CARRIER_SEA:
									case UNITAI_MISSILE_CARRIER_SEA:
									case UNITAI_PIRATE_SEA:
									case UNITAI_ATTACK_AIR:
									case UNITAI_DEFENSE_AIR:
									case UNITAI_CARRIER_AIR:
									case UNITAI_MISSILE_AIR:
									case UNITAI_PARADROP:
									case UNITAI_ATTACK_CITY_LEMMING:
										iTempValue += std::max(6, (kUnit.getCombat() * kUnit.getCombat())) / 5 + (kUnit.getAirCombat() * kUnit.getAirCombat()) / 5;
										break;
									case UNITAI_EXPLORE:
									case UNITAI_MISSIONARY:
									case UNITAI_PROPHET:
									case UNITAI_ARTIST:
									case UNITAI_SCIENTIST:
									case UNITAI_GENERAL:
									case UNITAI_MERCHANT:
									case UNITAI_ENGINEER:
									case UNITAI_SPY:
									case UNITAI_SPY_SEA:
									case UNITAI_MISSIONARY_SEA:
									case UNITAI_EXPLORE_SEA:
										iTempValue += 10;
										break;
									case UNITAI_ICBM:
										iTempValue += kUnit.getNukeRange() * 50;
										break;
								}
								if (bWarPlan)
									iTempValue *= 2;

								if (iTempValue > 0)
								{
									//	Just because we can build new units here doesn't add much if we can already
									//	build them a ton of other places
									iTempValue = (iTempValue*3)/(3+std::min(10,kOwner.countNumBuildings(eBuilding)));
								}

								iValue += bUnitIsBonusEnabled ? iTempValue : iTempValue/5;
							}
						}
					}
				}
			}

			// since this duplicates BUILDINGFOCUS_EXPERIENCE checks, do not repeat on pass 1
			if ((iFocusFlags & BUILDINGFOCUS_DOMAINSEA))
			{
				iValue += (kBuilding.getFreeExperience() * ((iHasMetCount > 0) ? 16 : 8));

				for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
				{
					UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);

					if (NO_UNIT != eUnit)
					{
						CvUnitInfo& kUnitInfo = GC.getUnitInfo(eUnit);
						int iCombatType = kUnitInfo.getUnitCombatType();
						if (kUnitInfo.getDomainType() == DOMAIN_SEA && canTrain(eUnit) && iCombatType != NO_UNITCOMBAT)
						{
							iValue += (kBuilding.getUnitCombatFreeExperience(iCombatType) * ((iHasMetCount > 0) ? 6 : 3));
						}
					}
				}
				iValue += (kBuilding.getDomainFreeExperience(DOMAIN_SEA) * ((iHasMetCount > 0) ? 16 : 8));
				iValue += (kBuilding.getDomainProductionModifier(DOMAIN_SEA) / 4);
			}

			if ((iFocusFlags & BUILDINGFOCUS_MAINTENANCE) || (iFocusFlags & BUILDINGFOCUS_GOLD) || (iPass > 0))
			{
				int iExtraMaintenance = 0;
				if (kBuilding.getCommerceChange(COMMERCE_GOLD) < 0 && GC.getDefineINT("TREAT_NEGATIVE_GOLD_AS_MAINTENANCE"))
				{
					iExtraMaintenance -= kBuilding.getCommerceChange(COMMERCE_GOLD) * GC.getHandicapInfo(getHandicapType()).getCorporationMaintenancePercent();
				}
				const int iBaseMaintenance = getMaintenanceTimes100();
				const int iExistingUpkeep = (iBaseMaintenance * std::max(0, 100 + getEffectiveMaintenanceModifier())) / 100;
				const int iNewUpkeep = ((iBaseMaintenance + iExtraMaintenance)* std::max(0, 100 + getEffectiveMaintenanceModifier() + kBuilding.getMaintenanceModifier())) / 100;

				iTempValue = (iExistingUpkeep - iNewUpkeep) / 16;

				if (bFinancialTrouble)
				{
					iTempValue *= 2;
				}
				else
				{
					iTempValue = (iGoldValueAssessmentModifier*iTempValue)/100;
				}

				iValue += iTempValue;
			}

			if ((iFocusFlags & BUILDINGFOCUS_SPECIALIST) || (iPass > 0))
			{
				const int iDefaultSpecialist = GC.getDefineINT("DEFAULT_SPECIALIST");
				int iCurrentSpecialistsRunnable = 0;
				int iSpecialistsValue = 0;
				for (int iI = GC.getNumSpecialistInfos() - 1; iI > -1; iI--)
				{
					if (iI != iDefaultSpecialist)
					{
						const SpecialistTypes eTypeX = static_cast<SpecialistTypes>(iI);

						if (!GET_PLAYER(getOwnerINLINE()).isSpecialistValid(eTypeX))
						{
							const int iRunnable = std::min(5, getMaxSpecialistCount(eTypeX));
							if (iRunnable > 0)
							{
								iCurrentSpecialistsRunnable += iRunnable;
							}
							const int iBuildingSpecialists = kBuilding.getSpecialistCount(iI) + GET_TEAM(getTeam()).getBuildingSpecialistChange(eBuilding, eTypeX);

							if (iBuildingSpecialists > 0 && iRunnable + iBuildingSpecialists > 0)
							{
								iTempValue = AI_specialistValue(eTypeX, false, false);

								iTempValue *= 20 + 40 * iBuildingSpecialists;

								iTempValue /= 100;

								if (iFoodDifference < 2)
								{
									iTempValue /= 4;
								}
								if (iRunnable > 0)
								{
									iTempValue /= 1 + iRunnable;
								}
								iSpecialistsValue += std::max(12, iTempValue / 100);
							}
						}
						else iCurrentSpecialistsRunnable += 6;
					}
				}

				if (iSpecialistsValue > 0)
				{
					iValue += iSpecialistsValue / std::max(2, iCurrentSpecialistsRunnable);
				}
			}

			if ((iFocusFlags & BUILDINGFOCUS_GOLD) || iPass > 0)
			{
				// trade routes
				iValue += (kOwner.getCommercePercent(COMMERCE_GOLD)*tradeRouteValue(kBuilding, YIELD_COMMERCE, bForeignTrade)*(bFinancialTrouble ? 200 : iGoldValueAssessmentModifier))/10000;
			}

			if ((iFocusFlags & BUILDINGFOCUS_RESEARCH) || iPass > 0)
			{
				// trade routes
				iValue += (kOwner.getCommercePercent(COMMERCE_RESEARCH)*tradeRouteValue(kBuilding, YIELD_COMMERCE, bForeignTrade))/100;
			}

			if ((iFocusFlags & BUILDINGFOCUS_CULTURE) || iPass > 0)
			{
				// trade routes
				iValue += (kOwner.getCommercePercent(COMMERCE_CULTURE)*tradeRouteValue(kBuilding, YIELD_COMMERCE, bForeignTrade))/100;
			}

			if ((iFocusFlags & BUILDINGFOCUS_ESPIONAGE) || iPass > 0)
			{
				// trade routes
				iValue += (kOwner.getCommercePercent(COMMERCE_ESPIONAGE)*tradeRouteValue(kBuilding, YIELD_COMMERCE, bForeignTrade))/100;
			}

			if (iPass > 0)
			{
				if (kBuilding.isAreaCleanPower() && !(area()->isCleanPower(getTeam())))
				{
					int iLoop = 0;
					for (CvCity* pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop) )
					{
						if (pLoopCity->area() == area())
						{
							if (pLoopCity->isDirtyPower())
							{
								//Fuyu: less value for cities that don't already have power
								iValue += 8;
							}
							else if (!pLoopCity->isPower())
							{
								iValue += 12;
							}
						}
					}
				}

				// Toffer - simple crime evaluation
				{
					const int iCrime = kBuilding.getCrime() + kBuilding.getCrimePerPop() * getPopulation();
					if (iCrime != 0)
					{
						iValue -= iCrime * getCrimeRate();
					}
				}

				if (kBuilding.getDomesticGreatGeneralRateModifier() != 0)
				{
					iValue += (kBuilding.getDomesticGreatGeneralRateModifier() / 10);
				}

				if (kBuilding.isAreaBorderObstacle() && !area()->isBorderObstacle(getTeam())
				&& !GC.getGameINLINE().isOption(GAMEOPTION_NO_BARBARIANS))
				{
					iValue += iNumCitiesInArea;

					if (GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS))
					{
						iValue += iNumCitiesInArea;
					}
				}

				if (kBuilding.isGovernmentCenter())
				{
					FAssert(!kBuilding.isCapital());
					iValue += (calculateDistanceMaintenance() - 3) * iNumCitiesInArea;
				}

				if (kBuilding.isMapCentering())
				{
					iValue++;
				}

				if (kBuilding.getFreeBonus() != NO_BONUS)
				{
					//	If we have spares adding another doesn't do anything that scales by city count - only
					//	the first one does that.  Furthermore as spares rack up even their trade value decreases
					iValue += (kOwner.AI_bonusVal((BonusTypes)(kBuilding.getFreeBonus()), 1) *
								 ((kOwner.getNumTradeableBonuses((BonusTypes)(kBuilding.getFreeBonus())) == 0) ? iNumCities : 1) *
								 kBuilding.getNumFreeBonuses() *
								 100)/(100*std::max(1,kOwner.getNumTradeableBonuses((BonusTypes)(kBuilding.getFreeBonus()))));
				}

				for (int iI = 0; iI < kBuilding.getNumExtraFreeBonuses(); iI++)
				{
					//	If we have spares adding another doesn't do anything that scales by city count - only
					//	the first one does that.  Furthermore as spares rack up even their trade value decreases
					iValue += (kOwner.AI_bonusVal((BonusTypes)(kBuilding.getExtraFreeBonus(iI)), 1) *
								 ((kOwner.getNumTradeableBonuses((BonusTypes)(kBuilding.getExtraFreeBonus(iI))) == 0) ? iNumCities : 1) *
								 kBuilding.getExtraFreeBonusNum(iI) *
								 100)/(100*std::max(1,kOwner.getNumTradeableBonuses((BonusTypes)(kBuilding.getExtraFreeBonus(iI)))));
				}

				if (kBuilding.getNoBonus() != NO_BONUS)
				{
					iValue -= kOwner.AI_bonusVal((BonusTypes)kBuilding.getNoBonus());
				}

				if (kBuilding.getCivicOption() != NO_CIVICOPTION)
				{
					for (int iI = 0; iI < GC.getNumCivicInfos(); iI++)
					{
						if (GC.getCivicInfo((CivicTypes)iI).getCivicOptionType() == kBuilding.getCivicOption() && !kOwner.canDoCivics((CivicTypes)iI))
						{
							iValue += kOwner.AI_civicValue((CivicTypes)iI) / 10;
						}
					}
				}

				{
					const int iGreatPeopleRateModifier = kBuilding.getGreatPeopleRateModifier();
					if (iGreatPeopleRateModifier > 0)
					{
						const int iGreatPeopleRate = getBaseGreatPeopleRate();
						const int kTargetGPRate = 10;

						// either not a wonder, or a wonder and our GP rate is at least the target rate
						if (!bIsLimitedWonder || iGreatPeopleRate >= kTargetGPRate)
						{
							iValue += ((iGreatPeopleRateModifier * iGreatPeopleRate) / 16);
						}
						// otherwise, this is a limited wonder (aka National Epic), we _really_ do not want to build this here
						// subtract from the value (if this wonder has a lot of other stuff, we still might build it)
						else
						{
							iValue -= ((iGreatPeopleRateModifier * (kTargetGPRate - iGreatPeopleRate)) / 12);
						}
					}
				}
				iValue += kBuilding.getGlobalGreatPeopleRateModifier() * iNumCities / 8;

				iValue -= kBuilding.getAnarchyModifier() / 4;
				iValue -= kBuilding.getGlobalHurryModifier() * 2;

				iValue += kBuilding.getGlobalFreeExperience() * iNumCities * ((iHasMetCount > 0) ? 6 : 3);

				if (bCanPopRush)
				{
					iValue += kBuilding.getFoodKept() / 2;
				}

				iValue += kBuilding.getAirlift() * (getPopulation() * 3 + 10);

				if (
					kBuilding.getAirModifier() < 0
				&& (
						kOwner.AI_totalUnitAIs(UNITAI_DEFENSE_AIR) > 0
					&&	kOwner.AI_totalUnitAIs(UNITAI_ATTACK_AIR) > 0
					||	kOwner.AI_totalUnitAIs(UNITAI_MISSILE_AIR) > 0
					)
				) iValue -= kBuilding.getAirModifier() / ((iHasMetCount > 0) ? 2 : 4);

				iValue += kBuilding.getAirUnitCapacity() * (getPopulation() * 2 + 10);

				iValue -= kBuilding.getNukeModifier() / ((iHasMetCount > 0) ? 10 : 20);

				iValue += kBuilding.getFreeSpecialist() * 16;
				iValue += kBuilding.getAreaFreeSpecialist() * iNumCitiesInArea * 12;
				iValue += kBuilding.getGlobalFreeSpecialist() * iNumCities * 12;

				iValue += ((kBuilding.getWorkerSpeedModifier() * kOwner.AI_getNumAIUnits(UNITAI_WORKER)) / 10);

				int iMilitaryProductionModifier = kBuilding.getMilitaryProductionModifier();
				if (iHasMetCount > 0 && iMilitaryProductionModifier > 0)
				{
					// either not a wonder, or a wonder and we are a high production city
					if (!bIsLimitedWonder || bIsHighProductionCity)
					{
						iValue += (iMilitaryProductionModifier / 4);

						// if a wonder, then pick one of the best cities
						if (bIsLimitedWonder)
						{
							// if one of the top 3 production cities, give a big boost
							if (aiYieldRank[YIELD_PRODUCTION] <= (2 + iLimitedWonderLimit))
							{
								iValue += (2 * iMilitaryProductionModifier) / (2 + aiYieldRank[YIELD_PRODUCTION]);
							}
						}
						// otherwise, any of the top half of cities will do
						else if (bIsHighProductionCity)
						{
							iValue += iMilitaryProductionModifier / 4;
						}
						iValue += ((iMilitaryProductionModifier * (getFreeExperience() + getSpecialistFreeExperience())) / 10);
					}
					// otherwise, this is a limited wonder (aka Heroic Epic), we _really_ do not want to build this here
					// subtract from the value (if this wonder has a lot of other stuff, we still might build it)
					else
					{
						iValue -= (iMilitaryProductionModifier * aiYieldRank[YIELD_PRODUCTION]) / 5;
					}
				}
				iValue += (kBuilding.getSpaceProductionModifier() / 5);
				iValue += ((kBuilding.getGlobalSpaceProductionModifier() * iNumCities) / 20);


				if (kBuilding.getGreatPeopleUnitClass() != NO_UNITCLASS)
				{
					iValue++; // XXX improve this for diversity...
				}

				// prefer to build great people buildings in places that already have some GP points
				iValue += ((kBuilding.getGreatPeopleRateChange() + iSpecialistGreatPeopleRate ) * 10) * (1 + (getBaseGreatPeopleRate() / 2));

				if (!bAreaAlone)
				{
					iValue += (kBuilding.getHealRateChange() / 2);
				}

				if (kBuilding.getGlobalPopulationChange() != 0)
				{
					const int iGrowthTurns = growthThreshold() / std::max(1, foodDifference(false));

					iValue += kBuilding.getGlobalPopulationChange() * 20 * iNumCities * iGrowthTurns / GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent();
				}

				iValue += (kBuilding.getFreeTechs() * 80);

				iValue += kBuilding.getEnemyWarWearinessModifier() / 2;

				for (int iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
				{
					if (kBuilding.getFreeSpecialistCount(iI) > 0)
					{
						iValue += ((AI_specialistValue(((SpecialistTypes)iI), false, false) * kBuilding.getFreeSpecialistCount(iI)) / 50);
					}
				}

				for (int iI = 0; iI < GC.getNumImprovementInfos(); iI++)
				{
					if (kBuilding.getImprovementFreeSpecialist(iI) > 0)
					{
						iValue += kBuilding.getImprovementFreeSpecialist(iI) * countNumImprovedPlots((ImprovementTypes)iI, true) * 50;
					}
				}

				for (int iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
				{
					iValue += (kBuilding.getDomainProductionModifier(iI) / 5);

					if (bIsHighProductionCity)
					{
						iValue += (kBuilding.getDomainProductionModifier(iI) / 5);
					}
				}

				if (kBuilding.isForceAllTradeRoutes())
				{
					for (int iI = 0; iI < MAX_TEAMS; iI++)
					{
						if (GET_TEAM((TeamTypes)iI).isAlive()
						&& !GET_TEAM(GET_PLAYER(getOwnerINLINE()).getTeam()).isVassal((TeamTypes)iI)
						&& !GET_TEAM(GET_PLAYER(getOwnerINLINE()).getTeam()).isOpenBorders((TeamTypes)iI))
						{
							iValue += GET_TEAM((TeamTypes)iI).getNumCities() * 2;
						}
					}
				}

				iTempValue = 0;
				for (int iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
				{
					const int iModifier = kBuilding.getUnitClassProductionModifier(iI);
					if (iModifier != 0)
					{
						UnitTypes eLoopUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI);

						if (eLoopUnit != NO_UNIT && canTrain(eLoopUnit))
						{
							int iBuildCost;
							UnitAITypes eUnitAI = (UnitAITypes) GC.getUnitInfo(eLoopUnit).getDefaultUnitAIType();

							UnitTypes eBestUnit = kOwner.bestBuildableUnitForAIType((DomainTypes)GC.getUnitInfo(eLoopUnit).getDomainType(), eUnitAI);

							if (eBestUnit != NO_UNIT)
							{
								const int iBestUnitAIValue = kOwner.AI_unitValue(eBestUnit, eUnitAI, area());
								const int iThisUnitAIValue = kOwner.AI_unitValue(eLoopUnit, eUnitAI, area());

								// Value as cost of production of the unit we can build scaled by their relative AI value (non-linear - we're squaring the ratio)
								const int iComparisonToBestFactor = 10 * iThisUnitAIValue / std::max(1, iBestUnitAIValue);

								iBuildCost = (iComparisonToBestFactor * iComparisonToBestFactor * GC.getUnitInfo(eBestUnit).getProductionCost())/100;
							}
							else iBuildCost = GC.getUnitInfo(eLoopUnit).getProductionCost();

							iTempValue += iBuildCost * iModifier / 50;
						}
					}
				}
				if (bIsHighProductionCity)
				{
					iTempValue *= 3; // adding 10% and 20% is same as adding 30% or 3 * 10%
				}
				iValue += iTempValue;

				if (kBuilding.getPopulationgrowthratepercentage() != 0)
				{
					const int iFoodPerTurn = foodDifference();

					if (iFoodPerTurn > 0)
					{
						const int iCityHappy = 1 + happyLevel() - unhappyLevel();
						if (iCityHappy > 0)
						{
							iValue += kBuilding.getPopulationgrowthratepercentage() * (-std::min(3, iCityHappy) * growthThreshold() / iFoodPerTurn) / 100;
						}
					}

				}

				if (kBuilding.getGlobalPopulationgrowthratepercentage() != 0)
				{
					int iFactor = 0;
					int iCount = 0;
					int iLoop = 0;

					for (CvCity* pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
					{
						iCount += 100;
						const int iCityHappy = 1 + pLoopCity->happyLevel() - pLoopCity->unhappyLevel();

						if (iCityHappy > 0)
						{
							const int iFoodPerTurn = pLoopCity->foodDifference();
							if (iFoodPerTurn > 0)
							{
								iFactor -= std::min(3, iCityHappy) * pLoopCity->growthThreshold() / iFoodPerTurn;
							}
						}
					}
					if (iCount > 0)
					{
						iValue += kBuilding.getGlobalPopulationgrowthratepercentage() * iFactor / iCount;
					}
				}

				for (int iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
				{
					if (kBuilding.getBuildingClassProductionModifier(iI) != 0)
					{
						const BuildingTypes eBuildingX = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)));
						if (eBuildingX != NO_BUILDING && canConstruct(eBuildingX))
						{
							const int iModifier = kBuilding.getBuildingClassProductionModifier(iI);
							if (iModifier > -100)
							{
								const int iOriginalCost = getHurryCost(true, eBuildingX, false);
								const int iNewCost = iOriginalCost * 100 / (100 + iModifier);
								iValue += (iOriginalCost - iNewCost) / 10;
							}
							else // If the modifier is less than -99, avoid it like the plague
							{
								iValue -= 1000;
							}
						}
					}
					if (kBuilding.getGlobalBuildingClassProductionModifier(iI) != 0)
					{
						const BuildingTypes eBuildingX = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)));
						if (eBuildingX != NO_BUILDING && canConstruct(eBuildingX))
						{
							const int iModifier = kBuilding.getGlobalBuildingClassProductionModifier(iI);

							if (iModifier > -100)
							{
								int iLoop = 0;
								int iCount = 0;

								for (CvCity* pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
								{
									if (pLoopCity->getNumBuilding(eBuildingX) == 0)
									{
										iCount++;
									}
								}
								if (iCount > 0)
								{
									const int iOriginalCost = getHurryCost(true, eBuildingX, false);
									const int iNewCost = iOriginalCost * 100 / (100 + iModifier);
									iValue += (iOriginalCost - iNewCost) * iCount / 10;
								}
							}
							else // If the modifier is less than -99, avoid it like the plague
							{
								iValue -= 1000;
							}
						}
					}
				}

				if (kBuilding.isProvidesFreshWater() && !plot()->isFreshWater())
				{
					CvPlot* pLoopPlot;
					for (int iI = 0; iI < NUM_CITY_PLOTS_1; iI++)
					{
						if (iI != CITY_HOME_PLOT)
						{
							pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);
							if (pLoopPlot != NULL &&
								!pLoopPlot->isWater() &&
								!pLoopPlot->isFreshWater() &&
								!pLoopPlot->isHills() &&
								!pLoopPlot->isImpassable() )
							{
								if(pLoopPlot->getImprovementType() == NO_IMPROVEMENT)
								{//if there are no improvements, farms become much more likely
									iValue += 20;
								}
								else
								{//we are not likely to tear down existing improvements for a farm
									iValue += 5;
								}
							}
						}
					}
				}

				iTempValue = 0;
				for (int iI = 0; iI < GC.getNumUnitInfos(); iI++)
				{
					if (GC.getUnitInfo((UnitTypes)iI).getPrereqBuilding() == eBuilding)
					{
						// BBAI TODO: Smarter monastary construction, better support for mods

						if (kOwner.AI_totalAreaUnitAIs(area(), ((UnitAITypes)(GC.getUnitInfo((UnitTypes)iI).getDefaultUnitAIType()))) == 0)
						{
							iTempValue += iNumCitiesInArea;
						}

						iTempValue++;

						ReligionTypes eReligion = (ReligionTypes)(GC.getUnitInfo((UnitTypes)iI).getPrereqReligion());
						if (eReligion != NO_RELIGION)
						{
							//encouragement to get some minimal ability to train special units
							if (bCulturalVictory1 || isHolyCity(eReligion) || isCapital())
							{
								iTempValue += (2 + iNumCitiesInArea);
							}

							if (bCulturalVictory2 && GC.getUnitInfo((UnitTypes)iI).getReligionSpreads(eReligion))
							{
								//this gives a very large extra value if the religion is (nearly) unique
								//to no extra value for a fully spread religion.
								//I'm torn between huge boost and enough to bias towards the best monastery type.
								int iReligionCount = GET_PLAYER(getOwnerINLINE()).getHasReligionCount(eReligion);
								iTempValue += (100 * (iNumCities - iReligionCount)) / (iNumCities * (iReligionCount + 1));
							}
						}
					}
				}

				if (iTempValue > 0)
				{
					//	Just because we can build new units here doesn't add much if we can already
					//	build them a ton of other places
					iTempValue = (iTempValue*3)/(3+std::min(10,kOwner.countNumBuildings(eBuilding)));
				}

				iValue += iTempValue;

				// is this building needed to build other buildings?
				for (int iI = 0; iI < GC.getNumBuildingInfos(); iI++)
				{
					int iPrereqBuildings = kOwner.getBuildingClassPrereqBuilding(((BuildingTypes) iI), eBuildingClass);

					// if we need some of us to build iI building, and we dont need more than we have cities
					if (iPrereqBuildings > 0 && iPrereqBuildings <= iNumCities)
					{
						// do we need more than what we are currently building?
						if (iPrereqBuildings > kOwner.getBuildingClassCountPlusMaking(eBuildingClass))
						{
							iValue += (iNumCities * 3);

							if (bCulturalVictory1)
							{
								BuildingTypes eLoopBuilding = (BuildingTypes) iI;
								CvBuildingInfo& kLoopBuilding = GC.getBuildingInfo(eLoopBuilding);
								int iLoopBuildingCultureModifier = kLoopBuilding.getCommerceModifier(COMMERCE_CULTURE);
								if (iLoopBuildingCultureModifier > 0)
								{
									int iLoopBuildingsBuilt = kOwner.getBuildingClassCount((BuildingClassTypes) kLoopBuilding.getBuildingClassType());

									// if we have less than the number needed in culture cities
									//		OR we are one of the top cities and we do not have the building
									if (iLoopBuildingsBuilt < iCulturalVictoryNumCultureCities ||
										(iCultureRank <= iCulturalVictoryNumCultureCities && 0 == getNumBuilding(eLoopBuilding)))
									{
										iValue += iLoopBuildingCultureModifier;

										if (bCulturalVictory3)
										{
											iValue += iLoopBuildingCultureModifier * 2;
										}
									}
								}
							}
						}
					}
				}

				for (int iI = 0; iI < GC.getNumVoteSourceInfos(); ++iI)
				{
					if (kBuilding.getVoteSourceType() == iI)
					{
						iTempValue = 0;
						if (kBuilding.isStateReligion())
						{
							int iShareReligionCount = 0;
							int iPlayerCount = 0;
							for (int iPlayer = 0; iPlayer < MAX_PLAYERS; iPlayer++)
							{
								CvPlayerAI& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
								if (iPlayer != getOwner() && kLoopPlayer.isAlive())
								{
									iPlayerCount++;
									if (GET_PLAYER(getOwnerINLINE()).getStateReligion() == kLoopPlayer.getStateReligion())
									{
										iShareReligionCount++;
									}
								}
							}
							iTempValue += (200 * (1 + iShareReligionCount)) / (1 + iPlayerCount);
						}
						else
						{
							iTempValue += 100;
						}
						iValue += (iTempValue * (GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_DIPLOMACY1) ? 5 : 1));
					}
				}

				for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
				{
					iTempValue = 0;

					if (iFoodDifference > 0)
					{
						iValue += kBuilding.getFoodKept() / 2;
					}

					if (kBuilding.getSeaPlotYieldChange(iI) > 0)
					{
						iTempValue += kBuilding.getSeaPlotYieldChange(iI) * AI_buildingSpecialYieldChangeValue(eBuilding, (YieldTypes)iI);
					}
					if (kBuilding.getRiverPlotYieldChange(iI) > 0)
					{
						iTempValue += kBuilding.getRiverPlotYieldChange(iI) * countNumRiverPlots() * 4;
					}
					iTempValue += kBuilding.getGlobalSeaPlotYieldChange(iI) * kOwner.countNumCoastalCities() * 8;

					iTempValue += (kBuilding.getYieldChange(iI) + GET_TEAM(getTeam()).getBuildingYieldChange(eBuilding, (YieldTypes)iI) + aiFreeSpecialistYield[iI]) * 8;
					iTempValue += (kBuilding.getYieldModifier(iI) + GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, (YieldTypes)iI)) * getBaseYieldRate((YieldTypes)iI) / 12;

					if (!bShrinksWithPower)
					{
						iTempValue += ((kBuilding.getPowerYieldModifier(iI) * getBaseYieldRate((YieldTypes)iI)) / ((bProvidesPower || isPower()) ? 10 : 17));
					}

					{
						const int iGlobalModifier = kBuilding.getGlobalYieldModifier(iI);
						const int iAreaModifier = kBuilding.getAreaYieldModifier(iI);
						int iYieldModiferValue = 0;

						if (iGlobalModifier > 0 || iAreaModifier > 0)
						{
							int iLoop = 0;
							for (CvCity* pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
							{
								iYieldModiferValue += pLoopCity->getBaseYieldRate((YieldTypes)iI) * (iGlobalModifier + (pLoopCity->area() == area() ? iAreaModifier : 0));
							}
						}
						iTempValue += iYieldModiferValue / 12;
					}

					if (bProvidesPower && !isPower())
					{
						iTempValue += getPowerYieldRateModifier((YieldTypes)iI) * getBaseYieldRate((YieldTypes)iI) / 12;
					}

					for (int iJ = 0; iJ < GC.getNumSpecialistInfos(); iJ++)
					{
						iTempValue += kBuilding.getSpecialistYieldChange(iJ, iI) * kOwner.getTotalPopulation() / 5;
					}

					for (int iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
					{
						if (hasBonus((BonusTypes)iJ))
						{
							iTempValue += ((kBuilding.getBonusYieldModifier(iJ, iI) * getBaseYieldRate((YieldTypes)iI)) / 12);
							iTempValue += (kBuilding.getBonusYieldChanges(iJ, iI) * 8);
							if (kBuilding.getVicinityBonusYieldChanges(iJ, iI) != 0 && hasVicinityBonus((BonusTypes)iJ))
							{
								iTempValue += (kBuilding.getVicinityBonusYieldChanges(iJ, iI) * 8);
							}
						}
					}

					if (iTempValue != 0)
					{
						if (bFinancialTrouble && iI == YIELD_COMMERCE)
						{
							iTempValue *= 2;
						}

						iTempValue *= kOwner.AI_yieldWeight((YieldTypes)iI);
						iTempValue /= 100;

						if (aiYieldRank[iI] == MAX_INT)
						{
							aiYieldRank[iI] = findBaseYieldRateRank((YieldTypes) iI);
						}

						// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
						// we do _not_ want to build this here (unless the value was small anyway)
						// Exempt unit build from this test however
						if (bIsLimitedWonder && canConstruct(eBuilding) && (aiYieldRank[iI] > (3 + iLimitedWonderLimit)))
						{
							iTempValue *= -1;
						}
						iValue += iTempValue;
					}
				}

				// Deal with properties
				iValue += buildingPropertiesValue(kBuilding);

				for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
				{
					iTempValue = getBuildingCommerceValue(eBuilding, iI, aiFreeSpecialistYield, aiFreeSpecialistCommerce, aiBaseCommerceRate, aiPlayerCommerceRate);

					if (iTempValue != 0)
					{
						//	Make sure we don't reduce 1 to 0!
						if ( iTempValue >= 2 )
						{
							iTempValue /= 2;
						}

						if (bFinancialTrouble && iI == COMMERCE_GOLD)
						{
							iTempValue *= 2;
						}

						iTempValue *= kOwner.AI_commerceWeight(((CommerceTypes)iI), this);
						iTempValue = (iTempValue + 99) / 100;

						// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
						// we do _not_ want to build this here (unless the value was small anyway)
						if (MAX_INT == aiCommerceRank[iI])
						{
							aiCommerceRank[iI] = findCommerceRateRank((CommerceTypes) iI);
						}
						if (bIsLimitedWonder && ((aiCommerceRank[iI] > (3 + iLimitedWonderLimit)))
							|| (bCulturalVictory1 && (iI == COMMERCE_CULTURE) && (aiCommerceRank[iI] == 1)))
						{
							iTempValue *= -1;

							// for culture, just set it to zero, not negative, just about every wonder gives culture
							if (iI == COMMERCE_CULTURE)
							{
								iTempValue = 0;
							}
						}
						iValue += iTempValue;
					}
				}

				for (int iI = 0; iI < GC.getNumReligionInfos(); iI++)
				{
					if (kBuilding.getReligionChange(iI) > 0 && GET_TEAM(getTeam()).hasHolyCity((ReligionTypes)iI))
					{
						iValue += (kBuilding.getReligionChange(iI) * ((eStateReligion == iI) ? 10 : 1));
					}
				}
				if (NO_VOTESOURCE != kBuilding.getVoteSourceType())
				{
					iValue += 100;
				}
			}
			else
			{
				if (iFocusFlags & BUILDINGFOCUS_FOOD)
				{
					// trade routes
					iValue += tradeRouteValue(kBuilding, YIELD_FOOD, bForeignTrade);

					iValue += kBuilding.getFoodKept();

					if (kBuilding.getSeaPlotYieldChange(YIELD_FOOD) > 0)
					{

						iTempValue = kBuilding.getSeaPlotYieldChange(YIELD_FOOD) * AI_buildingSpecialYieldChangeValue(eBuilding, YIELD_FOOD);

						if (iTempValue > 7 || getPopulation() < 4)
						{
							iValue += iTempValue * 4 / std::max(2, iFoodDifference);
						}
						//else don't bother
					}

					iTempValue = 0;
					iTempValue += ((GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, YIELD_FOOD) * getBaseYieldRate(YIELD_FOOD)) / 8);
					iTempValue += ((kBuilding.getYieldChange(YIELD_FOOD) + GET_TEAM(getTeam()).getBuildingYieldChange(eBuilding, YIELD_FOOD) + aiFreeSpecialistYield[YIELD_FOOD]) * 8);
					iTempValue += ((kBuilding.getYieldModifier(YIELD_FOOD) + GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, YIELD_FOOD)) * getBaseYieldRate(YIELD_FOOD) / 12);
					for (int iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
					{
						if (hasBonus((BonusTypes)iJ))
						{
							iTempValue += (kBuilding.getBonusYieldModifier(iJ, YIELD_FOOD) * getBaseYieldRate(YIELD_FOOD) / 12);
							if (kBuilding.getVicinityBonusYieldChanges(iJ, YIELD_FOOD) != 0 && hasVicinityBonus((BonusTypes)iJ))
							{
								iTempValue += kBuilding.getVicinityBonusYieldChanges(iJ, YIELD_FOOD) * 8;
							}

						}
					}
					iValue += iTempValue;

					if (kBuilding.getRiverPlotYieldChange(YIELD_FOOD) > 0)
					{
						iValue += kBuilding.getRiverPlotYieldChange(YIELD_FOOD) * countNumRiverPlots() * 4;
					}
				}

				if (iFocusFlags & BUILDINGFOCUS_PRODUCTION)
				{
					// trade routes
					iValue += tradeRouteValue(kBuilding, YIELD_PRODUCTION, bForeignTrade);

					iTempValue = (kBuilding.getYieldModifier(YIELD_PRODUCTION) + GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, YIELD_PRODUCTION)) * getBaseYieldRate(YIELD_PRODUCTION) / 16;

					//Fuyu Shrink consideration; and "? 24 : 30" is not enough / too much -> "? 20 : 35" (full value if there is already power, less if you still have to build a power plant. No value if you can't build any powerplant without shrinking the city)
					if (!bShrinksWithPower)
					{
						iTempValue += kBuilding.getPowerYieldModifier(YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION) / ((bProvidesPower || isPower()) ? 20 : 35);
					}

					if (kBuilding.getSeaPlotYieldChange(YIELD_PRODUCTION) > 0)
					{
						int iNumWaterPlots = countNumWaterPlots();

						if (!bIsLimitedWonder || (iNumWaterPlots > getNumCityPlots() / 2))
						{
							iTempValue += kBuilding.getSeaPlotYieldChange(YIELD_PRODUCTION) * iNumWaterPlots;
						}
					}
					if (kBuilding.getRiverPlotYieldChange(YIELD_PRODUCTION) > 0)
					{
						iTempValue += (kBuilding.getRiverPlotYieldChange(YIELD_PRODUCTION) * countNumRiverPlots() * 4);
					}
					if (bProvidesPower && !isPower())
					{
						//Fuyu "/ 12" is too much. -> "/ 20"
						iTempValue += ((getPowerYieldRateModifier(YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION)) / 20);
					}

					// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
					// we do _not_ want to build this here (unless the value was small anyway)
					if (bIsLimitedWonder && (aiYieldRank[YIELD_PRODUCTION] > (3 + iLimitedWonderLimit)))
					{
						iTempValue *= -1;
					}

					iValue += iTempValue;

					iTempValue = 0;
					iTempValue += ((GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION) + aiFreeSpecialistYield[YIELD_PRODUCTION]) / 8);
					iTempValue += ((kBuilding.getYieldChange(YIELD_PRODUCTION) + GET_TEAM(getTeam()).getBuildingYieldChange(eBuilding, YIELD_PRODUCTION)) * 8);
					iTempValue += ((kBuilding.getYieldModifier(YIELD_PRODUCTION) + GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, YIELD_PRODUCTION)) * getBaseYieldRate(YIELD_PRODUCTION) / 12);
					for (int iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
					{
						if (hasBonus((BonusTypes)iJ))
						{
							iTempValue += (kBuilding.getBonusYieldModifier(iJ, YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION) / 12);
							iTempValue += (kBuilding.getBonusYieldChanges(iJ, YIELD_PRODUCTION) * 8);
							if (kBuilding.getVicinityBonusYieldChanges(iJ, YIELD_PRODUCTION) != 0 && hasVicinityBonus((BonusTypes)iJ))
							{
								iTempValue += (kBuilding.getVicinityBonusYieldChanges(iJ, YIELD_PRODUCTION) * 8);
							}
						}
					}

					// Koshling - modify direct production value if the base yield rate for this city (for production) is low
					// in relation to the amount this would add
					// i.e. - boost the value of extra production especially when we donit already have much
					if (iTempValue > getBaseYieldRate(YIELD_PRODUCTION) / 3)
					{
						//	iTempValue is roughly 8*<direct production gain>, so if that implies a net gain
						//	of 4% (ish) or more boost the value
						const int iBoost = (iTempValue*12 - getBaseYieldRate(YIELD_PRODUCTION)*4);

						//	Examples -
						//	1) City poduction is 1, building adds 1 (iTempValue 8), so iBoost is 92 - value almost trebbled
						//	2) Same building when prodcution is already 10, iBoost is 56 - value roughly doubled
						//	3) Same building when production >= 24 - no boost
						iTempValue += (iTempValue*iBoost)/50;
					}

					iValue += iTempValue;
				}

				if (iFocusFlags & BUILDINGFOCUS_GOLD)
				{
					iValue += (getBuildingCommerceValue(eBuilding, COMMERCE_GOLD, aiFreeSpecialistYield, aiFreeSpecialistCommerce, aiBaseCommerceRate, aiPlayerCommerceRate)*iGoldValueAssessmentModifier)/100;
				}

				if (iFocusFlags & BUILDINGFOCUS_RESEARCH)
				{
					iValue += getBuildingCommerceValue(eBuilding, COMMERCE_RESEARCH, aiFreeSpecialistYield, aiFreeSpecialistCommerce, aiBaseCommerceRate, aiPlayerCommerceRate);
				}

				if (iFocusFlags & BUILDINGFOCUS_CULTURE)
				{
					iValue += getBuildingCommerceValue(eBuilding, COMMERCE_CULTURE, aiFreeSpecialistYield, aiFreeSpecialistCommerce, aiBaseCommerceRate, aiPlayerCommerceRate);
				}

				if (iFocusFlags & BUILDINGFOCUS_BIGCULTURE)
				{
					iValue += getBuildingCommerceValue(eBuilding, COMMERCE_CULTURE, aiFreeSpecialistYield, aiFreeSpecialistCommerce, aiBaseCommerceRate, aiPlayerCommerceRate)/5;
				}

				if (iFocusFlags & BUILDINGFOCUS_ESPIONAGE || (GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE) && (iFocusFlags & BUILDINGFOCUS_CULTURE)))
				{
					iValue += getBuildingCommerceValue(eBuilding, COMMERCE_ESPIONAGE, aiFreeSpecialistYield, aiFreeSpecialistCommerce, aiBaseCommerceRate, aiPlayerCommerceRate);
				}
			}

			if (iThreshold > 0 && iPass == 0 && iValue < iThreshold)
			{
				iValue = 0;
			}

			if (iPass > 0 && !isHuman() && !bForTech)
			{
				iValue += kBuilding.getAIWeight();
				if (iValue > 0)
				{
					for (int iI = 0; iI < GC.getNumFlavorTypes(); iI++)
					{
						iValue += kOwner.AI_getFlavorValue((FlavorTypes)iI) * kBuilding.getFlavorValue(iI);
					}
				}
			}
		}
	}

	//	If we are really seeking an answer specifically on the values from the specific flags
	//	then the rest is just a tie-breaker so dramatically boost the value of the flag-specific
	//	stuff from pass1
	if (bMaximizeFlaggedValue)
	{
		iValue += iPass1Value*19;
	}
	else
	{
		//	Even when not maximizing with all the extra effects on buildings pass1 was getting a
		//	bit dominated by pass2 so provide a little extra pass1 weight anyway
		iValue += iPass1Value;
	}

	if (bIsLimitedWonder && !bForTech && getPopulation() < 6 && !canConstruct(eBuilding))
	{
		// This building is being constructed in some special way, reduce the value for small cities if it's a wonder.
		iValue /= 8 - getPopulation();
	}
	return std::max(0, iValue);
}


ProjectTypes CvCityAI::AI_bestProject()
{
	ProjectTypes eBestProject;
	int iProductionRank;
	int iTurnsLeft;
	int iValue;
	int iBestValue;
	int iI;

	iProductionRank = findYieldRateRank(YIELD_PRODUCTION);

	iBestValue = 0;
	eBestProject = NO_PROJECT;

	for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
	{
		if (canCreate((ProjectTypes)iI))
		{
			iValue = AI_projectValue((ProjectTypes)iI);

			if ((GC.getProjectInfo((ProjectTypes)iI).getEveryoneSpecialUnit() != NO_SPECIALUNIT) ||
				  (GC.getProjectInfo((ProjectTypes)iI).getEveryoneSpecialBuilding() != NO_SPECIALBUILDING) ||
				  GC.getProjectInfo((ProjectTypes)iI).isAllowsNukes())
			{
				if (getCitySorenRandNum(100, "Project Everyone") == 0)
				{
					iValue++;
				}
			}

			if (iValue > 0)
			{
				iValue += getProjectProduction((ProjectTypes)iI);

				iTurnsLeft = getProductionTurnsLeft(((ProjectTypes)iI), 0);

				if ((iTurnsLeft <= GC.getGameINLINE().AI_turnsPercent(10, GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getCreatePercent())) || !(GET_TEAM(getTeam()).isHuman()))
				{
					if ((iTurnsLeft <= GC.getGameINLINE().AI_turnsPercent(20, GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getCreatePercent())) || (iProductionRank <= std::max(3, (GET_PLAYER(getOwnerINLINE()).getNumCities() / 2))))
					{
						if (iProductionRank == 1)
						{
							iValue += iTurnsLeft;
						}
						else
						{
							FAssert((MAX_INT / 1000) > iValue);
							iValue *= 1000;
							iValue /= std::max(1, (iTurnsLeft + 10));
						}

						iValue = std::max(1, iValue);

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							eBestProject = ((ProjectTypes)iI);
						}
					}
				}
			}
		}
	}

	return eBestProject;
}


int CvCityAI::AI_projectValue(ProjectTypes eProject)
{
	int iValue;
	int iI;

	iValue = 0;
	
	if (GC.getProjectInfo(eProject).getEveryoneSpecialBuilding() != NO_SPECIALBUILDING)
	{
		iValue += 10;
	}
	
	for (int iL = 0; iL < GC.getNumVictoryInfos(); iL++)
	{
		if (GC.getGameINLINE().isVictoryValid((VictoryTypes)iL))
		{
			if (GC.getVictoryInfo((VictoryTypes)iL).isTotalVictory())	//45deg: boost for SS production when playing with Mastery
			{
				if (GC.getProjectInfo(eProject).isSpaceprogram())
				{
					iValue += 1000;
				}
				if (GC.getProjectInfo(eProject).isSpaceship())
				{
					iValue += 1000;
				}					
			}
		}	
	}	
	
	if (GC.getProjectInfo(eProject).getNukeInterception() > 0)
	{
		if (GC.getGameINLINE().canTrainNukes())
		{
			iValue += (GC.getProjectInfo(eProject).getNukeInterception() *2);
			if ((GC.getGameINLINE().countTotalNukeUnits()) > (GET_PLAYER(getOwnerINLINE()).getNumCities() *4))
			{
				iValue *= 3;
			}
		}
	}

	if (GC.getProjectInfo(eProject).getTechShare() > 0)
	{
		if (GC.getProjectInfo(eProject).getTechShare() < GET_TEAM(getTeam()).getHasMetCivCount(true))
		{
			iValue += (20 / GC.getProjectInfo(eProject).getTechShare());
		}
	}

	for (iI = 0; iI < GC.getNumVictoryInfos(); iI++)
	{
		if (GC.getGameINLINE().isVictoryValid((VictoryTypes)iI))
		{
			iValue += (std::max(0, (GC.getProjectInfo(eProject).getVictoryThreshold(iI) - GET_TEAM(getTeam()).getProjectCount(eProject))) * 200);
		}
	}

	for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
	{
		iValue += (std::max(0, (GC.getProjectInfo((ProjectTypes)iI).getProjectsNeeded(eProject) - GET_TEAM(getTeam()).getProjectCount(eProject))) * 10);
	}

/************************************************************************************************/
/* Afforess					  Start		 01/02/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
	CvProjectInfo& kProject = GC.getProjectInfo(eProject);	//New Project Valuations Begin
	int iOurNumCities = GET_PLAYER(getOwnerINLINE()).getNumCities();
	int iOurHappy = kProject.getGlobalHappiness();
	int iOurHealth = kProject.getGlobalHealth();
	int iWorldHappy = kProject.getWorldHappiness();
	int iWorldHealth = kProject.getWorldHealth();
	int iWorldTradeRoutes = kProject.getWorldTradeRoutes();

	if (iOurHappy != 0)
	{
		iValue += 4 * iOurNumCities * iOurHappy;
	}	
	if (iOurHealth != 0)
	{
		iValue += 4 * iOurNumCities * iOurHealth;
	}
	if (iWorldTradeRoutes != 0)
	{
		iValue += ((iWorldTradeRoutes * 8 * (getPopulation() / 5 + 1)));
	}
	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		iValue += ((kProject.getCommerceModifier(iI) * iOurNumCities) / 4);
	}

	int iTempValue = 0;
	if ((iWorldHappy != 0) && (iWorldHealth != 0))
	{
		iTempValue += 4 * iOurNumCities * iWorldHappy;
		iTempValue += 4 * iOurNumCities * iWorldHealth;
		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive() && !GET_PLAYER((PlayerTypes)iI).isMinorCiv() && GET_PLAYER((PlayerTypes)iI).getID() != GET_PLAYER(getOwnerINLINE()).getID())
			{
				int iNumCities = GET_PLAYER((PlayerTypes)iI).getNumCities();
				int iAttitude = GET_PLAYER(getOwnerINLINE()).AI_getAttitude((PlayerTypes)iI);
				switch(iAttitude)
				{
					case ATTITUDE_FURIOUS:
						iTempValue -= 2 * iNumCities * iWorldHealth;
						iTempValue -= 2 * iNumCities * iWorldHappy;
						iTempValue -= iWorldTradeRoutes * 8;
						break;

					case ATTITUDE_ANNOYED:
						iTempValue -= iNumCities * iWorldHealth;
						iTempValue -= iNumCities * iWorldHappy;
						iTempValue -= iWorldTradeRoutes * 4;
						break;

					case ATTITUDE_CAUTIOUS:
						iTempValue -= (iNumCities  * iWorldHealth / 3);
						iTempValue -= (iNumCities  * iWorldHappy/ 3);
						iTempValue -= iWorldTradeRoutes;
						break;

					case ATTITUDE_PLEASED:
						iTempValue += iNumCities * iWorldHealth;
						iTempValue += iNumCities * iWorldHappy;
						iTempValue += iWorldTradeRoutes * 4;
						break;

					case ATTITUDE_FRIENDLY:
						iTempValue += 2 * iNumCities * iWorldHealth;
						iTempValue += 2 * iNumCities * iWorldHappy;
						iTempValue += iWorldTradeRoutes * 8;
						break;

					default:
						FAssert(false);
						break;
				}
			}
		}
	}
	iValue += iTempValue;
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/


	return iValue;
}

ProcessTypes CvCityAI::AI_bestProcess(CommerceTypes eCommerceType, int* commerceWeights)
{
	ProcessTypes eBestProcess;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	eBestProcess = NO_PROCESS;

	for (iI = 0; iI < GC.getNumProcessInfos(); iI++)
	{
		if (canMaintain((ProcessTypes)iI))
		{
			iValue = AI_processValue((ProcessTypes)iI, eCommerceType, commerceWeights);

			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				eBestProcess = ((ProcessTypes)iI);
			}
		}
	}

	return eBestProcess;
}

int CvCityAI::AI_processValue(ProcessTypes eProcess, CommerceTypes eCommerceType, int* commerceWeights)
{
	int iValue;
	int iTempValue;
	int iI;
	bool bValid = (eCommerceType == NO_COMMERCE);

	iValue = 0;

	if (GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble())
	{
		iValue += GC.getProcessInfo(eProcess).getProductionToCommerceModifier(COMMERCE_GOLD);
	}

	// if we own less than 50%, or we need to pop borders
	if ((plot()->calculateCulturePercent(getOwnerINLINE()) < 50) || (getCultureLevel() <= (CultureLevelTypes) 1))
	{
		iValue += GC.getProcessInfo(eProcess).getProductionToCommerceModifier(COMMERCE_CULTURE);
	}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  04/30/09								jdog5000	  */
/*																							  */
/* Cultural Victory AI																		  */
/************************************************************************************************/
	if ( GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3) )
	{
		// Final city for cultural victory will build culture to speed up victory
		if( findCommerceRateRank(COMMERCE_CULTURE) == GC.getGameINLINE().culturalVictoryNumCultureCities() )
		{
			iValue += 2*GC.getProcessInfo(eProcess).getProductionToCommerceModifier(COMMERCE_CULTURE);
		}
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		iTempValue = GC.getProcessInfo(eProcess).getProductionToCommerceModifier(iI);
		if (!bValid && ((CommerceTypes)iI == eCommerceType) && (iTempValue > 0))
		{
			bValid = true;
			iTempValue *= 2;
		}

		iTempValue *= GET_PLAYER(getOwnerINLINE()).AI_commerceWeight(((CommerceTypes)iI), this);

		iTempValue /= 100;

/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  07/08/09								jdog5000	  */
/*																							  */
/* Gold AI																					  */
/************************************************************************************************/
		iTempValue *= GET_PLAYER(getOwnerINLINE()).AI_averageCommerceExchange((CommerceTypes)iI);

		iTempValue /= 60;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

		//	Koshling - take into account external factors determining which commerce types
		//	we WANT most right now
		if ( commerceWeights != NULL )
		{
			iTempValue *= commerceWeights[iI];
		}

		iValue += iTempValue;
	}

	return (bValid ? iValue : 0);
}


int CvCityAI::AI_neededSeaWorkers()
{
	CvArea* pWaterArea;
	int iNeededSeaWorkers = 0;

/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  01/01/09								jdog5000	  */
/*																							  */
/* Worker AI																					*/
/************************************************************************************************/
	pWaterArea = waterArea(true);

	if (pWaterArea == NULL)
	{
		return 0;
	}

	iNeededSeaWorkers += GET_PLAYER(getOwnerINLINE()).countUnimprovedBonuses(pWaterArea);

	// Check if second water area city can reach was any unimproved bonuses
	pWaterArea = secondWaterArea();
	if (pWaterArea != NULL)
	{
		iNeededSeaWorkers += GET_PLAYER(getOwnerINLINE()).countUnimprovedBonuses(pWaterArea);
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

	return iNeededSeaWorkers;
}


bool CvCityAI::AI_isDefended(int iExtra, bool bAllowAnyDefenders)
{
	PROFILE_FUNC();

	return getGarrisonStrength(bAllowAnyDefenders) + iExtra >= AI_neededDefenseStrength();
	//	Note - the isDefended range check is slightly flawed since its possible for units 2 away to not actually
	//	be able to get to the city (e.g. - over water) so this probably needs tweaking slightly
	//return ((plot()->plotCount(PUF_canDefend /*PUF_canDefendGroupHead*/, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isCityAIType, -1, -1, iRange) + iExtra) >= AI_neededDefenders()); // XXX check for other team's units?
}

bool CvCityAI::AI_isAdequateHappinessMilitary(int iExtra)
{
	PROFILE_FUNC();

	return ((plot()->plotCount(PUF_isMilitaryHappiness, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isCityAIType) + iExtra) >= AI_neededHappinessDefenders()); // XXX check for other team's units?
}

/********************************************************************************/
/* 	BETTER_BTS_AI_MOD						10/17/08		jdog5000		*/
/* 																			*/
/* 	Air AI																	*/
/********************************************************************************/
/* original BTS code
bool CvCityAI::AI_isAirDefended(int iExtra)
{
	PROFILE_FUNC();

	return ((plot()->plotCount(PUF_canAirDefend, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isDomainType, DOMAIN_AIR) + iExtra) >= AI_neededAirDefenders()); // XXX check for other team's units?
}
*/
// Function now answers question of whether city has enough ready air defense, no longer just counts fighters
bool CvCityAI::AI_isAirDefended(bool bCountLand, int iExtra)
{
	PROFILE_FUNC();

	int iAirDefenders = iExtra;
	int iAirIntercept = 0;
	int iLandIntercept = 0;

	CvUnit* pLoopUnit;
	CLLNode<IDInfo>* pUnitNode = plot()->headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = plot()->nextUnitNode(pUnitNode);

		if ((pLoopUnit->getOwnerINLINE() == getOwnerINLINE()))
		{
			if ( pLoopUnit->canAirDefend() )
			{
				if( pLoopUnit->getDomainType() == DOMAIN_AIR )
				{
					// can find units which are already air patrolling using group activity
					if( pLoopUnit->getGroup()->getActivityType() == ACTIVITY_INTERCEPT )
					{
						iAirIntercept += pLoopUnit->currInterceptionProbability();
					}
					else
					{
						// Count air units which can air patrol
						if( pLoopUnit->getDamage() == 0 && !pLoopUnit->hasMoved() )
						{
							if( pLoopUnit->AI_getUnitAIType() == UNITAI_DEFENSE_AIR )
							{
								iAirIntercept += pLoopUnit->currInterceptionProbability();
							}
							else
							{
								iAirIntercept += pLoopUnit->currInterceptionProbability()/3;
							}
						}

					}
				}
				else if( pLoopUnit->getDomainType() == DOMAIN_LAND )
				{
					iLandIntercept += pLoopUnit->currInterceptionProbability();
				}
			}
		}
	}

	iAirDefenders += (iAirIntercept/100);

	if( bCountLand )
	{
		iAirDefenders += (iLandIntercept/100);
	}

	int iNeededAirDefenders = AI_neededAirDefenders();
	bool bHaveEnough = (iAirDefenders >= iNeededAirDefenders);

	return bHaveEnough;
}
/********************************************************************************/
/* 	BETTER_BTS_AI_MOD						END								*/
/********************************************************************************/


/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  04/25/10								jdog5000	  */
/*																							  */
/* War strategy AI, Barbarian AI																*/
/************************************************************************************************/
int CvCityAI::AI_neededDefenders()
{
	PROFILE_FUNC();

	if ( m_neededDefenders != -1 )
	{
		return m_neededDefenders;
	}

	int iDefenders;
	bool bOffenseWar = ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING));
	bool bDefenseWar = ((area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE));

	if (!(GET_TEAM(getTeam()).AI_isWarPossible()))
	{
		return 1;
	}

	if (isBarbarian())
	{  // Raging barbarians update. dbkblk, 2015-09
		iDefenders = GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getBarbarianInitialDefenders();
		if (GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS))
		{
			iDefenders = 3;
			if (getPopulation() - 7 > 0) {
				iDefenders += getPopulation() - 7;
			}
		}		
		return iDefenders;
	}

	iDefenders = 0;

	if (!GC.getGame().isEarlyGame() && (hasActiveWorldWonder() || isCapital() || isHolyCity()))
	{
		iDefenders++;

		if( GET_PLAYER(getOwner()).AI_isDoStrategy(AI_STRATEGY_ALERT1) || GET_PLAYER(getOwner()).AI_isDoStrategy(AI_STRATEGY_TURTLE) )
		{
			iDefenders++;
		}
	}

	int iFloatingDefenders = 0;
	if (!GET_PLAYER(getOwner()).AI_isDoStrategy(AI_STRATEGY_CRUSH))
	{
		iFloatingDefenders += AI_neededFloatingDefenders();
	}
	else
	{
		iFloatingDefenders += (AI_neededFloatingDefenders() + 2) / 4;
	}
	//iDefenders += iFloatingDefenders;
	logBBAIForTeam(getTeam(), "	  City %S floating defenders needed: %d", getName().GetCString(), iFloatingDefenders);

	if (bDefenseWar || GET_PLAYER(getOwner()).AI_isDoStrategy(AI_STRATEGY_ALERT2))
	{
		if (!(plot()->isHills()))
		{
			iDefenders++;
		}
	}

	if (!GC.getGame().isEarlyGame() && (GC.getGame().getGameTurn() - getGameTurnAcquired()) < 10)
	{
/* original code
		if (bOffenseWar)
		{
			if (!hasActiveWorldWonder() && !isHolyCity())
			{
				iDefenders /= 2;
				iDefenders = std::max(1, iDefenders);
			}
		}
	}

	if (GC.getGame().getGameTurn() - getGameTurnAcquired() < 10)
	{
		iDefenders = std::max(2, iDefenders);
		if (AI_isDanger())
		{
			iDefenders ++;
		}
		if (bDefenseWar)
		{
			iDefenders ++;
		}
*/
		iDefenders = std::max(2, iDefenders);

		if (bOffenseWar && getTotalDefense(true) > 0)
		{
			if (!hasActiveWorldWonder() && !isHolyCity())
			{
				iDefenders /= 2;
			}
		}

		if (AI_isDanger())
		{
			iDefenders++;
		}
		if (bDefenseWar)
		{
			iDefenders++;
		}
	}

	if (GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_LAST_STAND))
	{
		iDefenders += 10;
	}

	if( GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3) )
	{
		if( findCommerceRateRank(COMMERCE_CULTURE) <= GC.getGameINLINE().culturalVictoryNumCultureCities() )
		{
			iDefenders += 4;

			if( bDefenseWar )
			{
				iDefenders += 2;
			}
		}
	}

	if( GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_SPACE3) )
	{
		if( isCapital() || isProductionProject())
		{
			iDefenders += 2; // f1rpo (advc.107): was +=4

			if( bDefenseWar )
			{
				iDefenders += 3;
			}
		}

		if( isCapital() && GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_SPACE4) )
		{
			iDefenders += 6;
		}
	}
/************************************************************************************************/
/* RevolutionDCM Revolution AI		  02/03/10							   Afforess		 */
/*																							  */
/*																							  */
/************************************************************************************************/
	if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_REVOLUTION))
	{
		int iRevIndex = getRevolutionIndex();
		int iExtraRevDefenders = std::max(0, ((int)(std::pow((float)iRevIndex, 0.35f) - 6.5f)));
		iDefenders += iExtraRevDefenders;
		logBBAIForTeam(getTeam(), "	  City %S extra revolution defenders needed: %d", getName().GetCString(), iExtraRevDefenders);
	}
/************************************************************************************************/
/* RevolutionDCM						 END													*/
/************************************************************************************************/

/************************************************************************************************/
/* Afforess					  Start		 07/18/10											   */
/*																							  */
/*	City Defense Against Barbarians														   */
/************************************************************************************************/
	if (GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS) || GC.getGameINLINE().isOption(GAMEOPTION_BARBARIAN_WORLD))
	{
		if (GC.getGameINLINE().getElapsedGameTurns() < 250)
		{
			bool bAnimals = false;

			if (GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).isNoBarbUnits())
			{
				bAnimals = true;
			}

			if (GC.getGameINLINE().getNumCivCities() < ((GC.getGameINLINE().countCivPlayersAlive() * 3) / 2) && !GC.getGameINLINE().isOption(GAMEOPTION_ONE_CITY_CHALLENGE))
			{
				bAnimals = true;
			}

			if (GC.getGameINLINE().getElapsedGameTurns() < ((GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getBarbarianCreationTurnsElapsed() * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getBarbPercent()) / 100))
			{
				bAnimals = true;
			}

			if (!bAnimals)
			{
				iDefenders = std::max(iDefenders, GC.getGameINLINE().getElapsedGameTurns() > 150 ? 3 : 2);
			}
		}
	}

	int iTempDefenders = 0;
	int iHappiness = happyLevel() - unhappyLevel(0);
	if (GET_PLAYER(getOwnerINLINE()).getHappyPerMilitaryUnit() > 0)
	{
		if (iHappiness < 0)
		{
			iTempDefenders -= GET_PLAYER(getOwnerINLINE()).getHappyPerMilitaryUnit() * iHappiness;
		}
	}
	else if (GET_PLAYER(getOwnerINLINE()).getHappyPerMilitaryUnit() < 0)
	{
		if (iHappiness < 0)
		{
			iTempDefenders -= GET_PLAYER(getOwnerINLINE()).getHappyPerMilitaryUnit() * iHappiness;
		}
	}
	iTempDefenders /= (1 + GET_TEAM(getTeam()).getAtWarCount(true));
	iDefenders += iTempDefenders;

/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/


	iDefenders = std::max(iDefenders, AI_minDefenders());

	m_neededDefenders = iDefenders;

	return iDefenders;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

int CvCityAI::AI_neededHappinessDefenders()
{
	int iResult = 0;
	int iHappiness = happyLevel() - unhappyLevel(0);
	if (GET_PLAYER(getOwnerINLINE()).getHappyPerMilitaryUnit() > 0)
	{
		int iCurrent = plot()->plotCount(PUF_isMilitaryHappiness, -1, -1, getOwnerINLINE(), NO_TEAM);
		int iBaseHappiness = iHappiness - GET_PLAYER(getOwnerINLINE()).getHappyPerMilitaryUnit() * iCurrent;

		if (iBaseHappiness < 0)
		{
			iResult = -iBaseHappiness/GET_PLAYER(getOwnerINLINE()).getHappyPerMilitaryUnit();
		}
	}

	return iResult;
}

int CvCityAI::AI_minDefenders()
{
	int iDefenders = 1;
	int iEra = GET_PLAYER(getOwnerINLINE()).getCurrentEra();
	if (iEra >= 3) // f1rpo (advc.107): was >0
	{
		iDefenders++;
		//iDefenders += std::min(3, (iEra / 2));
		/*	<f1rpo> 4 defenders in any backwater by the Industrial era?
			From AdvCiv (advc.107): */
		if (iEra > GC.getGameINLINE().getStartEra() && iEra >= 3 &&
			isCoastal(2 * GC.getMIN_WATER_SIZE_FOR_OCEAN()))
		{
			iDefenders++;
		} // </f1rpo>
	}
	// Cap the iDefenders to the number a city tile can accept
	int iMaxUnitsPerTile = GC.getGameINLINE().getModderGameOption(MODDERGAMEOPTION_MAX_UNITS_PER_TILES);
	if (iMaxUnitsPerTile > 0 && iDefenders > ((iMaxUnitsPerTile * GC.getUNITS_PER_TILES_CITY_FACTOR()) - 1)) {
		iDefenders = ((iMaxUnitsPerTile * GC.getUNITS_PER_TILES_CITY_FACTOR()) - 1);
	}

	return std::max(1, iDefenders);
}

int CvCityAI::getGarrisonStrength(bool bAllowAnyDefenders) const
{
	PROFILE_FUNC();

	//	Note - the range check is slightly flawed since its possible for units 2 away to not actually
	//	be able to get to the city (e.g. - over water) so this probably needs tweaking slightly
	return plot()->plotStrength(UNITVALUE_FLAGS_DEFENSIVE, PUF_canDefend, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isCityGarrison, getID(), (int)bAllowAnyDefenders, 2);
}

int CvCityAI::AI_neededDefenseStrength()
{
	//	For now we just use the old calculation for needed defenders (a count)
	//	and multiply it by the base strength of the best defender we can currently build
	return AI_neededDefenders() * GET_PLAYER(getOwnerINLINE()).strengthOfBestUnitAI(DOMAIN_LAND, UNITAI_CITY_DEFENSE);
}

int CvCityAI::AI_minDefenseStrength()
{
	//	For now we just use the old calculation for needed defenders (a count)
	//	and multiply it by the base strength of the best defender we can currently build
	return AI_minDefenders() * GET_PLAYER(getOwnerINLINE()).strengthOfBestUnitAI(DOMAIN_LAND, UNITAI_CITY_DEFENSE);
}

int CvCityAI::AI_neededFloatingDefenders()
{
	int iFloatingDefenders = GET_PLAYER(getOwnerINLINE()).AI_getTotalFloatingDefendersNeeded(area());

	int iTotalThreat = std::max(1, GET_PLAYER(getOwnerINLINE()).AI_getTotalAreaCityThreat(area()));

	int iThreatModifier = 0;
	int iCityThreat = AI_cityThreat(NO_TEAM, &iThreatModifier);
	logBBAIForTeam(getTeam(), "	  City %S total area floating defenders needed: %d (Total Area Threat: %d, City Threat: %d, Threat Modifier: %d)", getName().GetCString(), iFloatingDefenders, iTotalThreat, iCityThreat, iThreatModifier);

	//	Use the threat modifier (which is based on danger from higher-difficulty setting players
	//	which usually means humans!) to increase local floating defender needs
	iFloatingDefenders *= iCityThreat;
	iFloatingDefenders = (iFloatingDefenders*(100+iThreatModifier))/100;
	iFloatingDefenders += (iTotalThreat / 2);
	iFloatingDefenders /= iTotalThreat;

	return iFloatingDefenders;
}

int CvCityAI::AI_neededAirDefenders()
{
	int iDefenders;

	if (!(GET_TEAM(getTeam()).AI_isWarPossible()))
	{
		return 0;
	}

	iDefenders = 0;

	int iRange = 5;

	int iOtherTeam = 0;
	int iEnemyTeam = 0;
	for (int iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (int iDY = -(iRange); iDY <= iRange; iDY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if ((pLoopPlot != NULL) && pLoopPlot->isOwned() && (pLoopPlot->getTeam() != getTeam()))
			{
				iOtherTeam++;
				if (GET_TEAM(getTeam()).AI_getWarPlan(pLoopPlot->getTeam()) != NO_WARPLAN)
				{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  01/01/09								jdog5000	  */
/*																							  */
/* Air AI																					   */
/************************************************************************************************/
					// If enemy has no bombers, don't need to defend as much
					if( GET_PLAYER(pLoopPlot->getOwner()).AI_totalUnitAIs(UNITAI_ATTACK_AIR) == 0 )
					{
						continue;
					}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
					iEnemyTeam += 2;
					if (pLoopPlot->isCity())
					{
						iEnemyTeam += 6;
					}
				}
			}
		}
	}

	iDefenders += (iOtherTeam + iEnemyTeam + 2) / 8;

	iDefenders = std::min((iEnemyTeam > 0) ? 4 : 2, iDefenders);

	if (iDefenders == 0)
	{
		if (isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()))
		{
			iDefenders++;
		}
	}
	return iDefenders;
}

#define	MAX_DANGER_EVALUATION_RANGE		6
#define	DISTANCE_ATTENUATION_FACTOR(x)	((2*(x))/5)
#define	NEUTRAL_BORDER_PLOT_DANGER		5
#define	OWNED_PLOT_NON_WAR_BASE_DANGER	10
#define	OWNED_PLOT_WAR_DANGER			30

//	Koshling - provide a numerical evaluation of city dnager from 0 (very safe) to 100 (about to be lost)
int CvCityAI::evaluateDanger()
{
	PROFILE_FUNC();

	CvUnitSelectionCriteria	noGrowthCriteria;

	noGrowthCriteria.m_bIgnoreGrowth = true;

	UnitTypes				eDummyUnit = GET_PLAYER(getOwnerINLINE()).bestBuildableUnitForAIType(DOMAIN_LAND, UNITAI_ATTACK, &noGrowthCriteria);

	if ( eDummyUnit == NO_UNIT )
	{
		eDummyUnit = GET_PLAYER(getOwnerINLINE()).bestBuildableUnitForAIType(DOMAIN_LAND, UNITAI_CITY_DEFENSE, &noGrowthCriteria);

		FAssert(eDummyUnit != NO_UNIT);

		if ( eDummyUnit == NO_UNIT )
		{
			OutputDebugString("Cannot find defender to use for strength test\n");
		}
	}

	if ( eDummyUnit != NO_UNIT )
	{
		EnterCriticalSection(&g_cEvaluateDangerSection);

		CvUnit*				pTempUnit = GET_PLAYER(getOwnerINLINE()).getTempUnit(eDummyUnit, getX_INLINE(), getY_INLINE());
		CvReachablePlotSet	plotSet(pTempUnit->getGroup(), 0, MAX_DANGER_EVALUATION_RANGE, false, 1);
		int					iBorderDanger = 0;
		int					iGeneralDanger = 0;
		int					iDistanceModifier[MAX_DANGER_EVALUATION_RANGE];

		iDistanceModifier[0] = 1000;
		for(int iI = 1; iI < MAX_DANGER_EVALUATION_RANGE; iI++)
		{
			iDistanceModifier[iI] = DISTANCE_ATTENUATION_FACTOR(iDistanceModifier[iI-1]);
		}

		for(CvReachablePlotSet::const_iterator itr = plotSet.begin(); itr != plotSet.end(); ++itr)
		{
			CvPlot*		pLoopPlot = itr.plot();
			TeamTypes	ePlotTeam = pLoopPlot->getTeam();

			// AIAndy: This sums up iGeneralDanger but never uses it. Looks incorrect for city danger evaluation.
			iGeneralDanger += pLoopPlot->getDangerCount(getOwnerINLINE());

			if ( ePlotTeam != getTeam() )
			{
				if ( ePlotTeam == NO_TEAM || !GET_TEAM(ePlotTeam).isAlive())
				{
					//	Neutral territory border
					iBorderDanger += NEUTRAL_BORDER_PLOT_DANGER*iDistanceModifier[itr.stepDistance()-1];
				}
				else
				{
					if ( GET_TEAM(getTeam()).isAtWar(ePlotTeam) )
					{
						//	Border with someone we are at war with
						iBorderDanger += OWNED_PLOT_WAR_DANGER*iDistanceModifier[itr.stepDistance()-1];
					}
					else
					{
						int iModifier;
						int iTempBorderDanger;

						//	Border with a player we are not at war with - weight by attitude
						iTempBorderDanger = (OWNED_PLOT_NON_WAR_BASE_DANGER*getPlayerDangerPercentage(pLoopPlot->getOwnerINLINE(), iModifier)*iDistanceModifier[itr.stepDistance()-1])/100;
						iTempBorderDanger = (iTempBorderDanger*(100+iModifier))/100;

						iBorderDanger += iTempBorderDanger;
					}
				}
			}
		}

		GET_PLAYER(getOwnerINLINE()).releaseTempUnit();

		LeaveCriticalSection(&g_cEvaluateDangerSection);

		return iBorderDanger/100;
	}
	else
	{
		//	Should never happen but empirically it does (very rarely) - needs future investigation
		return 100;
	}
}

bool CvCityAI::AI_isDanger()
{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  08/20/09								jdog5000	  */
/*																							  */
/* City AI, Efficiency																		  */
/************************************************************************************************/
	//return GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2, false);
	return GET_PLAYER(getOwnerINLINE()).AI_getAnyPlotDanger(plot(), 2, false);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
}


int CvCityAI::AI_getEmphasizeAvoidGrowthCount()
{
	return m_iEmphasizeAvoidGrowthCount;
}


bool CvCityAI::AI_isEmphasizeAvoidGrowth()
{
	return (AI_getEmphasizeAvoidGrowthCount() > 0);
}


int CvCityAI::AI_getEmphasizeGreatPeopleCount()
{
	return m_iEmphasizeGreatPeopleCount;
}


bool CvCityAI::AI_isEmphasizeGreatPeople()
{
	return (AI_getEmphasizeGreatPeopleCount() > 0);
}


bool CvCityAI::AI_isAssignWorkDirty()
{
	return m_bAssignWorkDirty;
}


void CvCityAI::AI_setAssignWorkDirty(bool bNewValue)
{
	m_bAssignWorkDirty = bNewValue;
}


bool CvCityAI::AI_isChooseProductionDirty()
{
	return m_bChooseProductionDirty;
}


void CvCityAI::AI_setChooseProductionDirty(bool bNewValue)
{
	m_bChooseProductionDirty = bNewValue;
}


CvCity* CvCityAI::AI_getRouteToCity() const
{
	if ( !m_routeToCityUpdated )
	{
		AI_updateRouteToCity();

		m_routeToCityUpdated = true;
	}

	return getCity(m_routeToCity);
}


void CvCityAI::AI_updateRouteToCity() const
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvCity* pBestCity = NULL;
	int iValue;
	int iBestValue;
	int iLoop = 0;
	int iI;
	static PlayerTypes eCachedPlayer = NO_PLAYER;
	static stdext::hash_map<int,bool>* allAreaCitiesConnected = NULL;
	stdext::hash_map<int,bool>::const_iterator itr;

	EnterCriticalSection(&g_cRouteUpdateSection);

	if ( allAreaCitiesConnected == NULL )
	{
		allAreaCitiesConnected = new stdext::hash_map<int,bool>();
	}

	if ( eCachedPlayer != getOwnerINLINE() )
	{
		eCachedPlayer = getOwnerINLINE();
		allAreaCitiesConnected->clear();
	}

	if ( (itr = allAreaCitiesConnected->find(area()->getID())) == allAreaCitiesConnected->end() ||
		 !(itr->second) )
	{
		gDLL->getFAStarIFace()->ForceReset(&GC.getRouteFinder());

		iBestValue = MAX_INT;
		pBestCity = NULL;

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getTeam())
			{
				for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
				{
					if (pLoopCity != this)
					{
						if (pLoopCity->area() == area())
						{
							if (!(gDLL->getFAStarIFace()->GeneratePath(&GC.getRouteFinder(), getX_INLINE(), getY_INLINE(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), false, getOwnerINLINE(), true)))
							{
								iValue = plotDistance(getX_INLINE(), getY_INLINE(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE());

								if (iValue < iBestValue)
								{
									iBestValue = iValue;
									pBestCity = pLoopCity;
								}
							}
						}
					}
				}
			}
		}

		if ( itr == allAreaCitiesConnected->end() )
		{
			(*allAreaCitiesConnected)[area()->getID()] = (pBestCity == NULL);
		}
	}

	if (pBestCity != NULL)
	{
		m_routeToCity = pBestCity->getIDInfo();
	}
	else
	{
		m_routeToCity.reset();
	}

	EnterCriticalSection(&g_cRouteUpdateSection);
}


int CvCityAI::AI_getEmphasizeYieldCount(YieldTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiEmphasizeYieldCount[eIndex];
}


bool CvCityAI::AI_isEmphasizeYield(YieldTypes eIndex)
{
	return (AI_getEmphasizeYieldCount(eIndex) > 0);
}


int CvCityAI::AI_getEmphasizeCommerceCount(CommerceTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_COMMERCE_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return (m_aiEmphasizeCommerceCount[eIndex] > 0);
}


bool CvCityAI::AI_isEmphasizeCommerce(CommerceTypes eIndex)
{
	return (AI_getEmphasizeCommerceCount(eIndex) > 0);
}


bool CvCityAI::AI_isEmphasize(EmphasizeTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumEmphasizeInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(m_pbEmphasize != NULL, "m_pbEmphasize is not expected to be equal with NULL");
	return m_pbEmphasize[eIndex];
}

bool CvCityAI::AI_isEmphasizeSpecialist(SpecialistTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumSpecialistInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(m_pbEmphasizeSpecialist != NULL, "m_pbEmphasize is not expected to be equal with NULL");
	return m_pbEmphasizeSpecialist[eIndex];
}


void CvCityAI::AI_setEmphasize(EmphasizeTypes eIndex, bool bNewValue)
{
	int iI;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumEmphasizeInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (AI_isEmphasize(eIndex) != bNewValue)
	{
		m_pbEmphasize[eIndex] = bNewValue;

		if (GC.getEmphasizeInfo(eIndex).isAvoidGrowth())
		{
			m_iEmphasizeAvoidGrowthCount += ((AI_isEmphasize(eIndex)) ? 1 : -1);
			FAssert(AI_getEmphasizeAvoidGrowthCount() >= 0);
		}

		if (GC.getEmphasizeInfo(eIndex).isGreatPeople())
		{
			m_iEmphasizeGreatPeopleCount += ((AI_isEmphasize(eIndex)) ? 1 : -1);
			FAssert(AI_getEmphasizeGreatPeopleCount() >= 0);
		}
/************************************************************************************************/
/* Afforess					  Start		 02/10/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
		if (GC.getEmphasizeInfo(eIndex).isAvoidAngryCitizens())
		{
			m_iEmphasizeAvoidAngryCitizensCount += ((AI_isEmphasize(eIndex)) ? 1 : -1);
			FAssert(AI_getEmphasizeAvoidAngryCitizensCount() >= 0);
		}

		if (GC.getEmphasizeInfo(eIndex).isAvoidUnhealthyCitizens())
		{
			m_iEmphasizeAvoidUnhealthyCitizensCount += ((AI_isEmphasize(eIndex)) ? 1 : -1);
			FAssert(AI_getEmphasizeAvoidUnhealthyCitizensCount() >= 0);
		}
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
		for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
		{
			if (GC.getEmphasizeInfo(eIndex).getYieldChange(iI))
			{
				m_aiEmphasizeYieldCount[iI] += ((AI_isEmphasize(eIndex)) ? 1 : -1);
				FAssert(AI_getEmphasizeYieldCount((YieldTypes)iI) >= 0);
			}
		}

		for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			if (GC.getEmphasizeInfo(eIndex).getCommerceChange(iI))
			{
				m_aiEmphasizeCommerceCount[iI] += ((AI_isEmphasize(eIndex)) ? 1 : -1);
				FAssert(AI_getEmphasizeCommerceCount((CommerceTypes)iI) >= 0);
			}
		}

		AI_assignWorkingPlots();

		if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && isCitySelected())
		{
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		}

		//	If we're using AI govenors and not part way through a build reflect
		//	the changes in a new production choice immediately
		if (isHuman() && (!isProduction() || getProduction() == 0) && isProductionAutomated() &&	GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_MODDER_3))
		{
			AI_chooseProduction();
		}
	}
}

void CvCityAI::AI_setEmphasizeSpecialist(SpecialistTypes eIndex, bool bNewValue)
{

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumSpecialistInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (AI_isEmphasizeSpecialist(eIndex) != bNewValue)
	{
		m_pbEmphasizeSpecialist[eIndex] = bNewValue;

		AI_assignWorkingPlots();

		if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && isCitySelected())
		{
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		}
	}
}

void CvCityAI::AI_forceEmphasizeCulture(bool bNewValue)
{
	if (m_bForceEmphasizeCulture != bNewValue)
	{
		m_bForceEmphasizeCulture = bNewValue;

		m_aiEmphasizeCommerceCount[COMMERCE_CULTURE] += (bNewValue ? 1 : -1);
		FAssert(m_aiEmphasizeCommerceCount[COMMERCE_CULTURE] >= 0);
	}
}


int CvCityAI::AI_getBestBuildValue(int iIndex)
{
	FAssertMsg(iIndex >= 0, "iIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(iIndex < NUM_CITY_PLOTS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiBestBuildValue[iIndex];
}


int CvCityAI::AI_totalBestBuildValue(CvArea* pArea)
{
	CvPlot* pLoopPlot;
	int iTotalValue;
	int iI;

	iTotalValue = 0;

/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 06/17/09														*/
/*																							  */
/*																							  */
/************************************************************************************************/
	for (iI = 0; iI < getNumCityPlots(); iI++)
/************************************************************************************************/
/* JOOYO_ADDON						  END													 */
/************************************************************************************************/
	{
		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->area() == pArea)
				{
/************************************************************************************************/
/* Afforess					  Start		 01/19/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
					if ((pLoopPlot->getImprovementType() == NO_IMPROVEMENT) || !(GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_SAFE_AUTOMATION) && !(pLoopPlot->getImprovementType() == (GC.getDefineINT("RUINS_IMPROVEMENT"))) && !(GC.getImprovementInfo((ImprovementTypes)pLoopPlot->getImprovementType()).isDepletedMine())))
					{
						iTotalValue += AI_getBestBuildValue(iI);
					}
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
				}
			}
		}
	}

	return iTotalValue;
}

int CvCityAI::AI_clearFeatureValue(int iIndex)
{
	CvPlot* pPlot = plotCity(getX_INLINE(), getY_INLINE(), iIndex);
	FAssert(pPlot != NULL);

	FeatureTypes eFeature = pPlot->getFeatureType();
/************************************************************************************************/
/* Afforess					  Start		 12/7/09												*/
/*																							  */
/*	   CTD Fix																				*/
/************************************************************************************************/
	if (eFeature == NO_FEATURE)
		return 0;
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
	FAssert(eFeature != NO_FEATURE);

	CvFeatureInfo& kFeatureInfo = GC.getFeatureInfo(eFeature);

	int iValue = 0;
	iValue += kFeatureInfo.getYieldChange(YIELD_FOOD) * 100;
	iValue += kFeatureInfo.getYieldChange(YIELD_PRODUCTION) * 60;
	iValue += kFeatureInfo.getYieldChange(YIELD_COMMERCE) * 40;

	if (iValue > 0 && pPlot->isBeingWorked())
	{
		iValue *= 3;
		iValue /= 2;
	}
	if (iValue != 0)
	{
		BonusTypes eBonus = pPlot->getBonusType(getTeam());
		if (eBonus != NO_BONUS)
		{
			iValue *= 3;
			if (pPlot->getImprovementType() != NO_IMPROVEMENT)
			{
				if (GC.getImprovementInfo(pPlot->getImprovementType()).isImprovementBonusTrade(eBonus))
				{
					iValue *= 4;
				}
			}
		}
	}

	int iHealthValue = 0;
	if (kFeatureInfo.getHealthPercent() != 0)
	{
		int iHealth = goodHealth() - badHealth();

/************************************************************************************************/
/* Afforess					  Start		 06/17/10											   */
/************************************************************************************************/
/*
		iHealthValue += (6 * kFeatureInfo.getHealthPercent()) / std::max(3, 1 + iHealth);
*/
	//speed up Jungle Clearing
		int iMultiplier = kFeatureInfo.getHealthPercent() > 0 ? 6 : 10;
		iHealthValue += (iMultiplier * kFeatureInfo.getHealthPercent()) / std::max(3, 1 + iHealth);
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
		if (iHealthValue > 0 && !pPlot->isBeingWorked())
		{
			iHealthValue *= 3;
			iHealthValue /= 2;
		}
	}
	iValue += iHealthValue;

	if (iValue > 0)
	{
		if (pPlot->getImprovementType() != NO_IMPROVEMENT)
		{
			if (GC.getImprovementInfo(pPlot->getImprovementType()).isRequiresFeature())
			{
				iValue += 500;
			}
		}

		if (GET_PLAYER(getOwnerINLINE()).getAdvancedStartPoints() >= 0)
		{
			iValue += 400;
		}
	}

	return -iValue;
}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  07/25/09								jdog5000	  */
/*																							  */
/* Debug																						*/
/************************************************************************************************/
int CvCityAI::AI_getGoodTileCount()
{
	CvPlot* pLoopPlot;
	int iGoodTileCount = 0;
	int iJ;
	int aiFinalYields[NUM_YIELD_TYPES];

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());

	for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = getCityIndexPlot(iI);

			if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this)
			{
				int iCount = (kPlayer.AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD));

				bool bUseBaseValue = true;
				//If the tile has a BestBuild and is being improved, then use the BestBuild
				//determine if the tile is being improved.

				if (iCount > 0)
				{
					BuildTypes eBuild = NO_BUILD;
					if (m_aeBestBuild[iI] != NO_BUILD && m_aiBestBuildValue[iI] > 0)
					{
						eBuild = m_aeBestBuild[iI];
					}
					else
					{
						// This check is necessary to stop oscillation which can result
						// when best build changes food situation for city, changing the best build.
						CvUnit* pLoopUnit;
						CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();

						while (pUnitNode != NULL)
						{
							pLoopUnit = ::getUnit(pUnitNode->m_data);
							pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

							if (pLoopUnit->getBuildType() != NO_BUILD)
							{
								if( eBuild == NO_BUILD || pLoopPlot->getBuildTurnsLeft(eBuild,0,0) > pLoopPlot->getBuildTurnsLeft(pLoopUnit->getBuildType(),0,0) )
								{
									eBuild = pLoopUnit->getBuildType();
								}
							}
						}
					}

					if( eBuild != NO_BUILD)
					{
						ImprovementTypes eImprovement = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();
						if (eImprovement != NO_IMPROVEMENT)
						{
							bool bIgnoreFeature = false;
							if (pLoopPlot->getFeatureType() != NO_FEATURE)
							{
								if (GC.getBuildInfo(eBuild).isFeatureRemove(pLoopPlot->getFeatureType()))
								{
									bIgnoreFeature = true;
								}
							}

							bUseBaseValue = false;
							for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
							{
								aiFinalYields[iJ] = (pLoopPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature) + pLoopPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
							}
						}
					}
				}

				//Otherwise use the base value.
				if (bUseBaseValue)
				{
					for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
						//by default we'll use the current value
						aiFinalYields[iJ] = pLoopPlot->getYield((YieldTypes)iJ);
						if (pLoopPlot->getFeatureType() != NO_FEATURE)
						{
							aiFinalYields[iJ] += std::max(0, -GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange((YieldTypes)iJ));
						}
					}
				}

				if ( ((aiFinalYields[YIELD_FOOD]*10) + (aiFinalYields[YIELD_PRODUCTION]*6) + (aiFinalYields[YIELD_COMMERCE]*4)) > 21 )
				{
					iGoodTileCount++;
				}
			}
		}
	}

	return iGoodTileCount;
}

int CvCityAI::AI_countWorkedPoorTiles()
{
	CvPlot* pLoopPlot;
	int iWorkedPoorTileCount = 0;
	int iJ;
	int aiFinalYields[NUM_YIELD_TYPES];

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());

	for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = getCityIndexPlot(iI);

			if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this && pLoopPlot->isBeingWorked())
			{
				int iCount = (kPlayer.AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD));

				bool bUseBaseValue = true;
				//If the tile has a BestBuild and is being improved, then use the BestBuild
				//determine if the tile is being improved.

				if (iCount > 0)
				{
					BuildTypes eBuild = NO_BUILD;
					if (m_aeBestBuild[iI] != NO_BUILD && m_aiBestBuildValue[iI] > 0)
					{
						eBuild = m_aeBestBuild[iI];
					}
					else
					{
						// This check is necessary to stop oscillation which can result
						// when best build changes food situation for city, changing the best build.
						CvUnit* pLoopUnit;
						CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();

						while (pUnitNode != NULL)
						{
							pLoopUnit = ::getUnit(pUnitNode->m_data);
							pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

							if (pLoopUnit->getBuildType() != NO_BUILD)
							{
								if( eBuild == NO_BUILD || pLoopPlot->getBuildTurnsLeft(eBuild,0,0) > pLoopPlot->getBuildTurnsLeft(pLoopUnit->getBuildType(),0,0) )
								{
									eBuild = pLoopUnit->getBuildType();
								}
							}
						}
					}

					if( eBuild != NO_BUILD )
					{
						ImprovementTypes eImprovement = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();
						if (eImprovement != NO_IMPROVEMENT)
						{
							bool bIgnoreFeature = false;
							if (pLoopPlot->getFeatureType() != NO_FEATURE)
							{
								if (GC.getBuildInfo(eBuild).isFeatureRemove(pLoopPlot->getFeatureType()))
								{
									bIgnoreFeature = true;
								}
							}

							bUseBaseValue = false;
							for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
							{
								aiFinalYields[iJ] = (pLoopPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature) + pLoopPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
							}
						}
					}
				}

				//Otherwise use the base value.
				if (bUseBaseValue)
				{
					for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
						//by default we'll use the current value
						aiFinalYields[iJ] = pLoopPlot->getYield((YieldTypes)iJ);
						if (pLoopPlot->getFeatureType() != NO_FEATURE)
						{
							aiFinalYields[iJ] += std::max(0, -GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange((YieldTypes)iJ));
						}
					}
				}

				if ( ((aiFinalYields[YIELD_FOOD]*10) + (aiFinalYields[YIELD_PRODUCTION]*6) + (aiFinalYields[YIELD_COMMERCE]*4)) <= 21 )
				{
					iWorkedPoorTileCount++;
				}
			}
		}
	}

	return iWorkedPoorTileCount;
}

int CvCityAI::AI_getTargetSize()
{
	PROFILE_FUNC();

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	int iTargetSize = AI_getGoodTileCount();

/********************************************************************************/
/*	Better Evaluation							09.03.2010		Fuyu			*/
/********************************************************************************/
	int iHappyAdjust = 0;
	int iHealthAdjust = 0;
	if (getProductionBuilding() != NO_BUILDING)
	{
		iHappyAdjust += getAdditionalHappinessByBuilding(getProductionBuilding());
		iHealthAdjust += getAdditionalHealthByBuilding(getProductionBuilding());
	}
/********************************************************************************/
/*	BE	END																		*/
/********************************************************************************/
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  09/02/10						 jdog5000 & Fuyu	  */
/*																							  */
/* City AI																					  */
/************************************************************************************************/
	//iTargetSize = std::min(iTargetSize, 2 + getPopulation() + goodHealth() - badHealth() + getEspionageHealthCounter() + std::max(0, iHealthAdjust));
	iTargetSize -= std::max(0, (iTargetSize - (1 + getPopulation() + goodHealth() - badHealth() + getEspionageHealthCounter() + std::max(0, iHealthAdjust))) / 2);

	if( iTargetSize < getPopulation() )
	{
		iTargetSize = std::max(iTargetSize, getPopulation() - (AI_countWorkedPoorTiles()/2));
	}

	// Target city size should not be perturbed by espionage, other short term effects
	iTargetSize = std::min(iTargetSize, getPopulation() + (happyLevel() - unhappyLevel() + getEspionageHappinessCounter() + std::max(0, iHappyAdjust)));
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

	if (kPlayer.getAdvancedStartPoints() >= 0)
	{
		iTargetSize += 2 + kPlayer.getCurrentEra();
	}

	return iTargetSize;
}


void CvCityAI::AI_getYieldMultipliers( int &iFoodMultiplier, int &iProductionMultiplier, int &iCommerceMultiplier, int &iDesiredFoodChange )
{
	iFoodMultiplier = 100;
	iCommerceMultiplier = 100;
	iProductionMultiplier = 100;
	iDesiredFoodChange = 0;

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());

	CvPlot* pLoopPlot;
	int iI, iJ;
	int aiFinalYields[NUM_YIELD_TYPES];

	int iBonusFoodSurplus = 0;
	int iBonusFoodDeficit = 0;
	int iFeatureFoodSurplus = 0;
	int iHillFoodDeficit = 0;
	int iFoodTotal = GC.getYieldInfo(YIELD_FOOD).getMinCity();
	int iProductionTotal = GC.getYieldInfo(YIELD_PRODUCTION).getMinCity();

	int iWorkerCount = 0;
	int iWorkedFood = 0;
	int iWorkableFood = 0;
	int iWorkableFoodPlotCount = 0;

	int iGoodTileCount = 0;

	int iSpecialistCount = getSpecialistPopulation() - totalFreeSpecialists();

/************************************************************************************************/
/* Afforess					  Start		 03/18/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
	for (iI = 0; iI < getNumCityPlots(); iI++)
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
	{
		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = getCityIndexPlot(iI);

			if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this)
			{
				int iCount = (kPlayer.AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD));

				bool bUseBaseValue = true;
				//If the tile has a BestBuild or is being improved, then use the BestBuild
				//determine if the tile is being improved.
				if (iCount > 0)
				{
					BuildTypes eBuild = NO_BUILD;
					if (m_aeBestBuild[iI] != NO_BUILD && m_aiBestBuildValue[iI] > 0)
					{
						eBuild = m_aeBestBuild[iI];
					}
					else
					{
						// This check is necessary to stop oscillation which can result
						// when best build changes food situation for city.
						CvUnit* pLoopUnit;
						CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();

						while (pUnitNode != NULL)
						{
							pLoopUnit = ::getUnit(pUnitNode->m_data);
							pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

							if (pLoopUnit->getBuildType() != NO_BUILD)
							{
								if( eBuild == NO_BUILD || pLoopPlot->getBuildTurnsLeft(eBuild,0,0) > pLoopPlot->getBuildTurnsLeft(pLoopUnit->getBuildType(),0,0) )
								{
									eBuild = pLoopUnit->getBuildType();
								}
							}
						}
					}

					if( eBuild != NO_BUILD )
					{
						ImprovementTypes eImprovement = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();
						if (eImprovement != NO_IMPROVEMENT)
						{
							bool bIgnoreFeature = false;
							if (pLoopPlot->getFeatureType() != NO_FEATURE)
							{
								if (GC.getBuildInfo(eBuild).isFeatureRemove(pLoopPlot->getFeatureType()))
								{
									bIgnoreFeature = true;
								}
							}

							bUseBaseValue = false;
							for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
							{
								aiFinalYields[iJ] = (pLoopPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature) + pLoopPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
							}
						}
					}
				}

				//Otherwise use the base value.
				if (bUseBaseValue)
				{
					for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
						//by default we'll use the current value
						aiFinalYields[iJ] = pLoopPlot->getYield((YieldTypes)iJ);
						if (pLoopPlot->getFeatureType() != NO_FEATURE)
						{
							aiFinalYields[iJ] += std::max(0, -GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange((YieldTypes)iJ));
						}
					}
				}

				if (pLoopPlot->isBeingWorked())
				{
					iWorkedFood += aiFinalYields[YIELD_FOOD];
				}
				else
				{
					if (aiFinalYields[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION())
					{
						iWorkableFood += aiFinalYields[YIELD_FOOD];
						iWorkableFoodPlotCount++;
					}
				}

				if (pLoopPlot->isBeingWorked() || (((aiFinalYields[YIELD_FOOD]*10) + (aiFinalYields[YIELD_PRODUCTION]*6) + (aiFinalYields[YIELD_COMMERCE]*4)) > 21))
				{
					iGoodTileCount++;
					if (pLoopPlot->isBeingWorked())
					{
						iFoodTotal += aiFinalYields[YIELD_FOOD];
					}
					else
					{
						iFoodTotal += aiFinalYields[YIELD_FOOD] / 2;
					}
					if (aiFinalYields[YIELD_PRODUCTION] > 1)
					{
						iProductionTotal += aiFinalYields[YIELD_PRODUCTION];
					}
				}

				if (pLoopPlot->getBonusType(getTeam()) != NO_BONUS)
				{
					int iNetFood = (aiFinalYields[YIELD_FOOD] - GC.getFOOD_CONSUMPTION_PER_POPULATION());
					iBonusFoodSurplus += std::max(0, iNetFood);
					iBonusFoodDeficit += std::max(0, -iNetFood);
				}

				if ((pLoopPlot->getFeatureType()) != NO_FEATURE)
				{
					iFeatureFoodSurplus += std::max(0, pLoopPlot->calculateNatureYield(YIELD_FOOD, getTeam()) - GC.getFOOD_CONSUMPTION_PER_POPULATION());
				}

				if ((pLoopPlot->isHills()))
				{
					iHillFoodDeficit += std::max(0, GC.getFOOD_CONSUMPTION_PER_POPULATION() - pLoopPlot->calculateNatureYield(YIELD_FOOD, getTeam()));
				}
			}
		}
	}


	int iBonusFoodDiff = ((iBonusFoodSurplus + iFeatureFoodSurplus) - (iBonusFoodDeficit + iHillFoodDeficit / 2));
	if (iBonusFoodDiff < 2)
	{
		iFoodMultiplier += 10 * (2 - iBonusFoodDiff);
	}

	int iHealth = goodHealth() - badHealth();
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  09/02/10						 jdog5000 & Fuyu	  */
/*																							  */
/* City AI																					  */
/************************************************************************************************/
	int iTargetSize = AI_getTargetSize();
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

	//int iExtraFoodForGrowth = (std::max(0, iTargetSize - getPopulation()) + 3) / 4;
	int iExtraFoodForGrowth = (std::max(0, iTargetSize - getPopulation() + 3)) / 4; // K-Mod!!
	if (getPopulation() < iTargetSize)
	{
		iExtraFoodForGrowth ++;
	}

	int iFoodDifference = iFoodTotal - ((iTargetSize * GC.getFOOD_CONSUMPTION_PER_POPULATION()) + iExtraFoodForGrowth);

	iDesiredFoodChange = -iFoodDifference + std::max(0, -iHealth);
	if (iTargetSize > getPopulation())
	{
		if (iDesiredFoodChange > 3)
		{
			iDesiredFoodChange = (iDesiredFoodChange + 3) / 2;
		}
	}

	if (iFoodDifference > 4)
	{
		iFoodMultiplier -= 8 + 4 * iFoodDifference;
	}

	if (iFoodDifference < 0)
	{
		iFoodMultiplier +=  -iFoodDifference * 4;
	}

	if (iProductionTotal < 10)
	{
		iProductionMultiplier += (80 - 8 * iProductionTotal);
	}
	int iProductionTarget = 1 + (std::min(getPopulation(), (iTargetSize * 3) / 5));

	if (iProductionTotal < iProductionTarget)
	{
		iProductionMultiplier += 8 * (iProductionTarget - iProductionTotal);
	}

	if ((iBonusFoodSurplus + iFeatureFoodSurplus > 5) && ((iBonusFoodDeficit + iHillFoodDeficit) > 5))
	{
		if ((iBonusFoodDeficit + iHillFoodDeficit) > 8)
		{
			//probably a good candidate for a wonder pump
			iProductionMultiplier += 40;
			iCommerceMultiplier += (kPlayer.AI_isFinancialTrouble()) ? 0 : -40;
		}
	}

	int iNetCommerce = 1 + kPlayer.getCommerceRate(COMMERCE_GOLD) + kPlayer.getCommerceRate(COMMERCE_RESEARCH) + std::max(0, kPlayer.getGoldPerTurn());
	int iNetExpenses = kPlayer.calculateInflatedCosts() + std::max(0, -kPlayer.getGoldPerTurn());
	int iRatio = (100 * iNetExpenses) / std::max(1, iNetCommerce);

	if (iRatio > 40)
	{
		iCommerceMultiplier += (33 * (iRatio - 40)) / 60;
	}

	// AI no longer uses emphasis except for short term boosts.
	if( isHuman() )
	{
		if (AI_isEmphasizeYield(YIELD_FOOD))
		{
			iFoodMultiplier *= 130;
			iFoodMultiplier /= 100;
		}
		if (AI_isEmphasizeYield(YIELD_PRODUCTION))
		{
			iProductionMultiplier *= 140;
			iProductionMultiplier /= 100;
		}
		if (AI_isEmphasizeYield(YIELD_COMMERCE))
		{
			iCommerceMultiplier *= 140;
			iCommerceMultiplier /= 100;
		}
	}

	int iProductionAdvantage = 100 * AI_yieldMultiplier(YIELD_PRODUCTION);
	iProductionAdvantage /= kPlayer.AI_averageYieldMultiplier(YIELD_PRODUCTION);
	iProductionAdvantage *= kPlayer.AI_averageYieldMultiplier(YIELD_COMMERCE);
	iProductionAdvantage /= AI_yieldMultiplier(YIELD_COMMERCE);

	//now we normalize the effect by # of cities

	int iNumCities = kPlayer.getNumCities();
	FAssert(iNumCities > 0);//superstisious?

	//in short in an OCC the relative multipliers should *never* make a difference
	//so this indeed equals "100" for the iNumCities == 0 case.
	iProductionAdvantage = ((iProductionAdvantage * (iNumCities - 1) + 200) / (iNumCities + 1));

	iProductionMultiplier *= iProductionAdvantage;
	iProductionMultiplier /= 100;

	iCommerceMultiplier *= 100;
	iCommerceMultiplier /= iProductionAdvantage;

	int iGreatPeopleAdvantage = 100 * getTotalGreatPeopleRateModifier();
	iGreatPeopleAdvantage /= kPlayer.AI_averageGreatPeopleMultiplier();
	iGreatPeopleAdvantage = ((iGreatPeopleAdvantage * (iNumCities - 1) + 200) / (iNumCities + 1));
	iGreatPeopleAdvantage += 200; //gpp multipliers are larger than others so lets not go overboard
	iGreatPeopleAdvantage /= 3;

	//With great people we want to slightly increase food priority at the expense of commerce
	//this gracefully handles both wonder and specialist based GPP...
	iCommerceMultiplier *= 100;
	iCommerceMultiplier /= iGreatPeopleAdvantage;
	iFoodMultiplier *= iGreatPeopleAdvantage;
	iFoodMultiplier /= 100;

	// if leader flavor likes production, increase production, reduce commerce
	if (kPlayer.AI_isDoStrategy(AI_STRATEGY_PRODUCTION))
	{
		iProductionMultiplier += 10;
		iCommerceMultiplier -= 10;
	}

	if (iFoodMultiplier < 100)
	{
		iFoodMultiplier = 10000 / (200 - iFoodMultiplier);
	}
	if (iProductionMultiplier < 100)
	{
		iProductionMultiplier = 10000 / (200 - iProductionMultiplier);
	}
	if (iCommerceMultiplier < 100)
	{
		iCommerceMultiplier = 10000 / (200 - iCommerceMultiplier);
	}

	if (angryPopulation(1) > 0)
	{
		iFoodMultiplier /= 2;
	}
}


int CvCityAI::AI_getImprovementValue( CvPlot* pPlot, ImprovementTypes eImprovement, int iFoodPriority, int iProductionPriority, int iCommercePriority, int iFoodChange, bool bOriginal )
{
	int iValue = 0;

	int aiFinalYields[NUM_YIELD_TYPES];
	int aiDiffYields[NUM_YIELD_TYPES];

	int iBestTempBuildValue = 0;
	BuildTypes eBestTempBuild = NO_BUILD;

	BonusTypes eBonus = pPlot->getBonusType(getTeam());
	BonusTypes eNonObsoleteBonus = pPlot->getNonObsoleteBonusType(getTeam());

	bool bHasBonusImprovement = false;

	if (eNonObsoleteBonus != NO_BONUS)
	{
		if (pPlot->getImprovementType() != NO_IMPROVEMENT)
		{
			if (GC.getImprovementInfo(pPlot->getImprovementType()).isImprovementBonusTrade(eNonObsoleteBonus))
			{
				bHasBonusImprovement = true;
			}
		}
	}

	BuildTypes eForcedBuild = NO_BUILD;

	{	//If a worker is already building a build, force that Build.
		CLLNode<IDInfo>* pUnitNode;
		CvUnit* pLoopUnit;

		pUnitNode = pPlot->headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = pPlot->nextUnitNode(pUnitNode);

			if (pLoopUnit->getBuildType() != NO_BUILD)
			{
				if (GC.getBuildInfo(pLoopUnit->getBuildType()).getImprovement() != NO_IMPROVEMENT)
				{
					eForcedBuild = pLoopUnit->getBuildType();
					break;
				}
			}
		}
	}


	bool bIgnoreFeature = false;
	bool bValid = false;

	if (eImprovement == pPlot->getImprovementType())
	{
		bValid = true;
	}
	else
	{
		if (eForcedBuild != NO_BUILD)
		{
			if (GC.getBuildInfo(eForcedBuild).getImprovement() == eImprovement)
			{
				eBestTempBuild = eForcedBuild;
			}
		}
		else
		{
			for (int iJ = 0; iJ < GC.getNumBuildInfos(); iJ++)
			{
				BuildTypes eBuild = ((BuildTypes)iJ);

				if (GC.getBuildInfo(eBuild).getImprovement() == eImprovement)
				{
					if (GET_PLAYER(getOwnerINLINE()).canBuild(pPlot, eBuild, false))
					{
						iValue = 10000;

						iValue /= (GC.getBuildInfo(eBuild).getTime() + 1);

						// XXX feature production???

						if (iValue > iBestTempBuildValue)
						{
							iBestTempBuildValue = iValue;
							eBestTempBuild = eBuild;
						}
					}
				}
			}
		}

		if (eBestTempBuild != NO_BUILD)
		{
			bValid = true;

			if (pPlot->getFeatureType() != NO_FEATURE)
			{
				if (GC.getBuildInfo(eBestTempBuild).isFeatureRemove(pPlot->getFeatureType()))
				{
					bIgnoreFeature = true;

					if (GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange(YIELD_PRODUCTION) > 0)
					{
						if (eNonObsoleteBonus == NO_BONUS)
						{
							if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_LEAVE_FORESTS))
							{
								bValid = false;
							}
							else if (healthRate() < 0 && GC.getFeatureInfo(pPlot->getFeatureType()).getHealthPercent() > 0)
							{
								bValid = false;
							}
							else if (GET_PLAYER(getOwnerINLINE()).getFeatureHappiness(pPlot->getFeatureType()) > 0)
							{
								bValid = false;
							}
						}
					}
				}
			}
		}
	}

	if (bValid)
	{
		ImprovementTypes eFinalImprovement = finalImprovementUpgrade(eImprovement);

		if (eFinalImprovement == NO_IMPROVEMENT)
		{
			eFinalImprovement = eImprovement;
		}

		iValue = 0;

		if (eBonus != NO_BONUS)
		{
			if (eNonObsoleteBonus != NO_BONUS)
			{
				if (GC.getImprovementInfo(eFinalImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
				{
					iValue += (GET_PLAYER(getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus) * 10);
					iValue += 200;
					/*if (eBestBuild != NO_BUILD)
					{
						if ((GC.getBuildInfo(eBestBuild).getImprovement() == NO_IMPROVEMENT) || (!GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(eBestBuild).getImprovement()).isImprovementBonusTrade(eNonObsoleteBonus)))
						{
							//Always prefer improvements which connect bonuses.
							eBestBuild = NO_BUILD;
							iBestValue = 0;
						}
					}*/
				}
				else
				{
					/*if (eBestBuild != NO_BUILD)
					{
						if ((GC.getBuildInfo(eBestBuild).getImprovement() != NO_IMPROVEMENT) && (GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(eBestBuild).getImprovement()).isImprovementBonusTrade(eNonObsoleteBonus)))
						{
							iValue -= 1000;
						}
					}*/
				}
			}
		}
		else
		{
			for (int iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
			{
				if (GC.getImprovementInfo(eFinalImprovement).getImprovementBonusDiscoverRand(iJ) > 0)
				{
					iValue++;
				}
			}
		}

		if (iValue >= 0)
		{
			int iJ;

			iValue *= 2;
			for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
			{
				if( false && bOriginal )
				{
					aiFinalYields[iJ] = 2*(pPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature));
					aiFinalYields[iJ] += (pPlot->calculateImprovementYieldChange(eFinalImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
					aiFinalYields[iJ] += (pPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
					if (bIgnoreFeature && pPlot->getFeatureType() != NO_FEATURE)
					{
						aiFinalYields[iJ] -= 2 * GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange((YieldTypes)iJ);
					}
					aiDiffYields[iJ] = (aiFinalYields[iJ] - (2 * pPlot->getYield(((YieldTypes)iJ))));
				}
				else
				{
					aiFinalYields[iJ] = 2*(pPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature));
					aiFinalYields[iJ] += (pPlot->calculateImprovementYieldChange(eFinalImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false, true));
					aiFinalYields[iJ] += (pPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false, true));
					if (bIgnoreFeature && pPlot->getFeatureType() != NO_FEATURE)
					{
						aiFinalYields[iJ] -= 2 * GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange((YieldTypes)iJ);
					}

					int iCurYield = 2*(pPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), false));

					ImprovementTypes eCurImprovement = pPlot->getImprovementType();
					if( eCurImprovement != NO_IMPROVEMENT )
					{
						ImprovementTypes eCurFinalImprovement = finalImprovementUpgrade(eCurImprovement);
						if (eCurFinalImprovement == NO_IMPROVEMENT)
						{
							eCurFinalImprovement = eCurImprovement;
						}
						iCurYield += (pPlot->calculateImprovementYieldChange(eCurFinalImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false, true));
						iCurYield += (pPlot->calculateImprovementYieldChange(eCurImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false, true));
					}

					aiDiffYields[iJ] = (aiFinalYields[iJ] - iCurYield);
				}
			}
			iValue += (aiDiffYields[YIELD_FOOD] * ((100 * iFoodPriority) / 100));
			iValue += (aiDiffYields[YIELD_PRODUCTION] * ((60 * iProductionPriority) / 100));
			iValue += (aiDiffYields[YIELD_COMMERCE] * ((40 * iCommercePriority) / 100));

			iValue /= 2;

			for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
			{
				aiFinalYields[iJ] /= 2;
				aiDiffYields[iJ] /= 2;
			}

			if (iValue > 0)
			{
				// this is mainly to make it improve better tiles first
				//flood plain > grassland > plain > tundra
				iValue += (aiFinalYields[YIELD_FOOD] * 10);
				iValue += (aiFinalYields[YIELD_PRODUCTION] * 6);
				iValue += (aiFinalYields[YIELD_COMMERCE] * 4);

				if (aiFinalYields[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION())
				{
					//this is a food yielding tile
					if (iFoodPriority > 100)
					{
						iValue *= 100 + iFoodPriority;
						iValue /= 200;
					}
					if (iFoodChange > 0)
					{
						iValue += (10 * (1 + aiDiffYields[YIELD_FOOD]) * (1 + aiFinalYields[YIELD_FOOD] - GC.getFOOD_CONSUMPTION_PER_POPULATION()) * iFoodChange * iFoodPriority) / 100;
					}
					if (iCommercePriority > 100)
					{
						iValue *= 100 + (((iCommercePriority - 100) * aiDiffYields[YIELD_COMMERCE]) / 2);
						iValue /= 100;
					}
				}
				else if (aiFinalYields[YIELD_FOOD] < GC.getFOOD_CONSUMPTION_PER_POPULATION())
				{
					if ((aiDiffYields[YIELD_PRODUCTION] > 0) && (aiFinalYields[YIELD_FOOD]+aiFinalYields[YIELD_PRODUCTION] > 3))
					{
						if (iFoodPriority < 100 || GET_PLAYER(getOwnerINLINE()).getCurrentEra() < 2)
						{
							//value booster for mines on hills
							iValue *= (100 + 25 * aiDiffYields[YIELD_PRODUCTION]);
							iValue /= 100;
						}
					}
					if (iFoodChange < 0)
					{
						iValue *= 4 - iFoodChange;
						iValue /= 3 + aiFinalYields[YIELD_FOOD];
					}
				}

				if ((iFoodPriority < 100) && (iProductionPriority > 100))
				{
					if( bOriginal )
					{
						iValue *= (200 + iProductionPriority);
					}
					else
					{
						iValue *= (200 + ((iProductionPriority - 100)*aiFinalYields[YIELD_PRODUCTION]));
					}
					iValue /= 200;
				}
				if (eBonus == NO_BONUS)
				{
					if (iFoodChange > 0)
					{
						//We want more food.
						iValue *= 2 + std::max(0, aiDiffYields[YIELD_FOOD]);
						iValue /= 2 * (1 + std::max(0, -aiDiffYields[YIELD_FOOD]));
					}
//							else if (iFoodChange < 0)
//							{
//								//We want to soak up food.
//								iValue *= 8;
//								iValue /= 8 + std::max(0, aiDiffYields[YIELD_FOOD]);
//							}
				}
			}


			/*if (bEmphasizeIrrigation && GC.getImprovementInfo(eFinalImprovement).isCarriesIrrigation())
			{
				iValue += 500;
			}*/

			if (getImprovementFreeSpecialists(eFinalImprovement) > 0)
			{
				iValue += 2000;
			}

			int iHappiness = GC.getImprovementInfo(eFinalImprovement).getHappiness();
			if ((iHappiness != 0) && !(GET_PLAYER(getOwnerINLINE()).getAdvancedStartPoints() >= 0))
			{
				//int iHappyLevel = iHappyAdjust + (happyLevel() - unhappyLevel(0));
				int iHappyLevel = 0 + (happyLevel() - unhappyLevel(0));
				if (eImprovement == pPlot->getImprovementType())
				{
					iHappyLevel -= iHappiness;
				}
				int iHealthLevel = (goodHealth() - badHealth(false, 0));

				int iHappyValue = 0;
				if (iHappyLevel <= 0)
				{
					iHappyValue += 400;
				}
				bool bCanGrow = true;// (getYieldRate(YIELD_FOOD) > foodConsumption());

				if (iHappyLevel <= iHealthLevel)
				{
					iHappyValue += 200 * std::max(0, (bCanGrow ? std::min(6, 2 + iHealthLevel - iHappyLevel) : 0) - iHappyLevel);
				}
				else
				{
					iHappyValue += 200 * std::max(0, (bCanGrow ? 1 : 0) - iHappyLevel);
				}
				if (!pPlot->isBeingWorked())
				{
					iHappyValue *= 4;
					iHappyValue /= 3;
				}
				iHappyValue += std::max(0, (pPlot->getCityRadiusCount() - 1)) * ((iHappyValue > 0) ? iHappyLevel / 2 : 200);
				iValue += iHappyValue * iHappiness;
			}
/************************************************************************************************/
/* Afforess					  Start		 08/22/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
			int iHealthPercent = GC.getImprovementInfo(eFinalImprovement).getHealthPercent();
			int iBadHealth = abs(getImprovementBadHealth());
			int iGoodHealth = getImprovementGoodHealth();
			//Bad Health AND will push over the edge and give more unhealthiness
			if ((iHealthPercent < 0 && (iBadHealth - iHealthPercent % 100 < iBadHealth % 100)) ||
			//OR Good Health AND will give us more healthiness
			(iHealthPercent > 0 && (iGoodHealth + iHealthPercent % 100 > iGoodHealth % 100)))
			{
				int iHealth = iHealthPercent > 0 ? std::max(1, iHealthPercent % 100) : std::min(-1, iHealthPercent % 100);
				int iHealthLevel = iHealth + (goodHealth() - badHealth(false, 0));
				if (eImprovement == pPlot->getImprovementType())
				{
					iHealthLevel -= iHealth;
				}

				int iHealthValue = 0;
				bool bCanGrow = (getYieldRate(YIELD_FOOD) > foodConsumption());
				if (iHealthLevel < 0)
				{
					iHealthValue -= 12 * iHealth;
				}
				if (!bCanGrow && iHealthLevel < 0)
				{
					iHealthValue -= 40 * iHealth;
				}
				if (iHealthLevel > 0)
				{
					iHealthValue += 10 * iHealth;
				}

				if (!pPlot->isBeingWorked())
				{
					iHealthValue *= 2;
				}
				iValue += iHealthValue * 3;
			}
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/

			if (!isHuman())
			{
				iValue *= std::max(0, (GC.getLeaderHeadInfo(getPersonalityType()).getImprovementWeightModifier(eFinalImprovement) + 200));
				iValue /= 200;
			}

			if (pPlot->getImprovementType() == NO_IMPROVEMENT)
			{
				if (pPlot->isBeingWorked())
				{
					iValue *= 5;
					iValue /= 4;
				}

				if (eBestTempBuild != NO_BUILD)
				{
					if (pPlot->getFeatureType() != NO_FEATURE)
					{
						if (GC.getBuildInfo(eBestTempBuild).isFeatureRemove(pPlot->getFeatureType()))
						{
							CvCity* pCity = NULL;
							iValue += pPlot->getFeatureProduction(eBestTempBuild, getTeam(), &pCity) * 2;
							FAssert(pCity == this);

							//iValue += iClearFeatureValue;
						}
					}
				}
			}
			else
			{
				// cottage/villages (don't want to chop them up if turns have been invested)
				ImprovementTypes eImprovementDowngrade = (ImprovementTypes)GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementPillage();
				while (eImprovementDowngrade != NO_IMPROVEMENT)
				{
					CvImprovementInfo& kImprovementDowngrade = GC.getImprovementInfo(eImprovementDowngrade);
					iValue -= kImprovementDowngrade.getUpgradeTime() * 8;
					eImprovementDowngrade = (ImprovementTypes)kImprovementDowngrade.getImprovementPillage();
				}
/************************************************************************************************/
/* Afforess					  Start		 05/23/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
/*
				if (GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementUpgrade() != NO_IMPROVEMENT)
*/
				if (GET_TEAM(getTeam()).getImprovementUpgrade((ImprovementTypes)pPlot->getImprovementType()) != NO_IMPROVEMENT)
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
				{
					iValue -= (GC.getImprovementInfo(pPlot->getImprovementType()).getUpgradeTime() * 8 * (pPlot->getUpgradeProgressHundredths())) / std::max(1, 100*GC.getGameINLINE().getImprovementUpgradeTime(pPlot->getImprovementType()));
				}

				if (eNonObsoleteBonus == NO_BONUS)
				{
					if (isWorkingPlot(pPlot))
					{
						if (((iFoodPriority < 100) && (aiFinalYields[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION())) || (GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementPillage() != NO_IMPROVEMENT))
						{
							iValue -= 70;
							iValue *= 2;
							iValue /= 3;
						}
					}
				}

				if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_SAFE_AUTOMATION))
				{
					iValue /= 4;	//Greatly prefer builds which are legal.
				}
			}
		}
	}

	return iValue;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

BuildTypes CvCityAI::AI_getBestBuild(int iIndex)
{
	FAssertMsg(iIndex >= 0, "iIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(iIndex < NUM_CITY_PLOTS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aeBestBuild[iIndex];
}


int CvCityAI::AI_countBestBuilds(CvArea* pArea)
{
	CvPlot* pLoopPlot;
	int iCount;
	int iI;

	iCount = 0;

/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 06/17/09														*/
/*																							  */
/*																							  */
/************************************************************************************************/
	for (iI = 0; iI < getNumCityPlots(); iI++)
/************************************************************************************************/
/* JOOYO_ADDON						  END													 */
/************************************************************************************************/
	{
		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->area() == pArea)
				{
					if (AI_getBestBuild(iI) != NO_BUILD)
					{
						iCount++;
					}
				}
			}
		}
	}

	return iCount;
}

void CvCityAI::AI_markBestBuildValuesStale(void)
{
	m_bestBuildValuesStale = true;
}

// Improved worker AI provided by Blake - thank you!
void CvCityAI::AI_updateBestBuild()
{
	PROFILE_FUNC();

	//	This is VERY expensive to update, and is pretty accurate from
	//	turn to turn except when suignificant things change (when we will mark it stale)
	if ( !m_bestBuildValuesStale )
	{
		OutputDebugString(CvString::format("City %S skips re-evaluation of build values\n", getName().GetCString()).c_str());
		return;
	}

	OutputDebugString(CvString::format("City %S re-evaluates build values\n", getName().GetCString()).c_str());
	m_bestBuildValuesStale = false;

	CvPlot* pLoopPlot;
	int iI, iJ;
	int aiFinalYields[NUM_YIELD_TYPES];

	int iBonusFoodSurplus = 0;
	int iBonusFoodDeficit = 0;
	int iFeatureFoodSurplus = 0;
	int iHillFoodDeficit = 0;
	int iFoodTotal = GC.getYieldInfo(YIELD_FOOD).getMinCity();
	int iProductionTotal = GC.getYieldInfo(YIELD_PRODUCTION).getMinCity();
	int iFoodMultiplier = 100;
	int iCommerceMultiplier = 100;
	int iProductionMultiplier = 100;
	int iWorkerCount = 0;

	int iWorkedFood = 0;
	int iWorkableFood = 0;
	int iWorkableFoodPlotCount = 0;

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());

	bool bChop = false;

	int iGoodTileCount = 0;

	int iSpecialistCount = getSpecialistPopulation() - totalFreeSpecialists();

	int iHappyAdjust = 0;
	int iHealthAdjust = 0;

/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 06/17/09														*/
/*																							  */
/*																							  */
/************************************************************************************************/
	for (iI = 0; iI < getNumCityPlots(); iI++)
/************************************************************************************************/
/* JOOYO_ADDON						  END													 */
/************************************************************************************************/
	{
		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = getCityIndexPlot(iI);

			if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this)
			{
				int iCount = (kPlayer.AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD));
				iWorkerCount += iCount;

				bool bUseBaseValue = true;
				//If the tile has a BestBuild and is being improved, then use the BestBuild
				//determine if the tile is being improved.
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  06/25/09								jdog5000	  */
/*																							  */
/* Worker AI, City AI																		   */
/************************************************************************************************/
				if (iCount > 0)
				{
					BuildTypes eBuild = NO_BUILD;
					if (m_aeBestBuild[iI] != NO_BUILD && m_aiBestBuildValue[iI] > 0)
					{
						eBuild = m_aeBestBuild[iI];
					}
					else
					{
						// This check is necessary to stop oscillation which can result
						// when best build changes food situation for city, changing the best build.
						CvUnit* pLoopUnit;
						CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();

						while (pUnitNode != NULL)
						{
							pLoopUnit = ::getUnit(pUnitNode->m_data);
							pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

							if (pLoopUnit->getBuildType() != NO_BUILD)
							{
								if( eBuild == NO_BUILD || pLoopPlot->getBuildTurnsLeft(eBuild,0,0) > pLoopPlot->getBuildTurnsLeft(pLoopUnit->getBuildType(),0,0) )
								{
									eBuild = pLoopUnit->getBuildType();
								}
							}
						}
					}

					if( eBuild != NO_BUILD )
					{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
						ImprovementTypes eImprovement = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();
						if (eImprovement != NO_IMPROVEMENT)
						{
							bool bIgnoreFeature = false;
							if (pLoopPlot->getFeatureType() != NO_FEATURE)
							{
								if (GC.getBuildInfo(eBuild).isFeatureRemove(pLoopPlot->getFeatureType()))
								{
									bIgnoreFeature = true;
								}
							}

							iHappyAdjust += GC.getImprovementInfo(eImprovement).getHappiness();
							if (pLoopPlot->getImprovementType() != NO_IMPROVEMENT)
							{
								iHappyAdjust -= GC.getImprovementInfo(pLoopPlot->getImprovementType()).getHappiness();
							}


							bUseBaseValue = false;
							for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
							{
								aiFinalYields[iJ] = (pLoopPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature) + pLoopPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
							}
						}
					}
				}

				//Otherwise use the base value.
				if (bUseBaseValue)
				{
					for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
						//by default we'll use the current value
						aiFinalYields[iJ] = pLoopPlot->getYield((YieldTypes)iJ);
						if (pLoopPlot->getFeatureType() != NO_FEATURE)
						{
							aiFinalYields[iJ] += std::max(0, -GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange((YieldTypes)iJ));
						}
					}
				}

				if (pLoopPlot->isBeingWorked())
				{
					iWorkedFood += aiFinalYields[YIELD_FOOD];
				}
				else
				{
					if (aiFinalYields[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION())
					{
						iWorkableFood += aiFinalYields[YIELD_FOOD];
						iWorkableFoodPlotCount++;
					}
				}

				if (pLoopPlot->isBeingWorked() || (((aiFinalYields[YIELD_FOOD]*10) + (aiFinalYields[YIELD_PRODUCTION]*6) + (aiFinalYields[YIELD_COMMERCE]*4)) > 21))
				{
					iGoodTileCount++;
					if (pLoopPlot->isBeingWorked())
					{
						iFoodTotal += aiFinalYields[YIELD_FOOD];
					}
					else
					{
						iFoodTotal += aiFinalYields[YIELD_FOOD] / 2;
					}
					if (aiFinalYields[YIELD_PRODUCTION] > 1)
					{
						iProductionTotal += aiFinalYields[YIELD_PRODUCTION];
					}
				}

				if (pLoopPlot->getBonusType(getTeam()) != NO_BONUS)
				{
					int iNetFood = (aiFinalYields[YIELD_FOOD] - GC.getFOOD_CONSUMPTION_PER_POPULATION());
					iBonusFoodSurplus += std::max(0, iNetFood);
					iBonusFoodDeficit += std::max(0, -iNetFood);
				}

				if ((pLoopPlot->getFeatureType()) != NO_FEATURE)
				{
					iFeatureFoodSurplus += std::max(0, pLoopPlot->calculateNatureYield(YIELD_FOOD, getTeam()) - GC.getFOOD_CONSUMPTION_PER_POPULATION());
				}

				if ((pLoopPlot->isHills()))
				{
					iHillFoodDeficit += std::max(0, GC.getFOOD_CONSUMPTION_PER_POPULATION() - pLoopPlot->calculateNatureYield(YIELD_FOOD, getTeam()));
				}
			}
		}
	}

	if (getProductionBuilding() != NO_BUILDING)
	{
/********************************************************************************/
/*	Better Evaluation							09.03.2010		Fuyu			*/
/********************************************************************************/
/* original code
		iHappyAdjust += getBuildingHappiness(getProductionBuilding());
		iHealthAdjust += getBuildingHealth(getProductionBuilding());
*/
		iHappyAdjust += getAdditionalHappinessByBuilding(getProductionBuilding());
		iHealthAdjust += getAdditionalHealthByBuilding(getProductionBuilding());
/********************************************************************************/
/*	BE	END																		*/
/********************************************************************************/
	}

	//XXX rewrite this to fix too many farms issue
	int iSpecialistAdjustment = 0;
	if (iWorkableFoodPlotCount > 0)
	{
		//Calculate approximately how much food the assigned specialists could work
		iSpecialistAdjustment = (std::min(iSpecialistCount, iWorkableFoodPlotCount) * iWorkableFood) / iWorkableFoodPlotCount;
	}
	iFoodTotal += iSpecialistAdjustment;

	int iBonusFoodDiff = ((iBonusFoodSurplus + iFeatureFoodSurplus) - (iBonusFoodDeficit + iHillFoodDeficit / 2));

	int iHealth = goodHealth() - badHealth();
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  09/02/10						 jdog5000 & Fuyu	  */
/*																							  */
/* City AI																					  */
/************************************************************************************************/
	int iTargetSize = iGoodTileCount;

	if( getEspionageHealthCounter() > 0 )
	{
		iTargetSize = std::min(iTargetSize, 2 + getPopulation());
	}
	else
	{
		iTargetSize = std::min(iTargetSize, 2 + getPopulation() + (iHealth)/2);
	}

	if( iTargetSize < getPopulation() )
	{
		iTargetSize = std::max(iTargetSize, getPopulation() - (AI_countWorkedPoorTiles()/2));
	}

	// Target city size should not be perturbed by espionage, other short term effects
	if( getEspionageHappinessCounter() > 0 )
	{
		iTargetSize = std::min(iTargetSize, getPopulation());
	}
	else
	{
		iTargetSize = std::min(iTargetSize, getPopulation()+(happyLevel()-unhappyLevel()));
	}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  05/02/09								jdog5000	  */
/*																							  */
/* Worker AI																					*/
/************************************************************************************************/
/* original bts code
	if (GET_PLAYER(getOwnerINLINE()).getAdvancedStartPoints() >= 0)
	{
		iTargetSize += 2 + GET_PLAYER(getOwnerINLINE()).getCurrentEra() / 2;
	}

	if (kPlayer.getAdvancedStartPoints() >= 0)
	{
		iTargetSize += kPlayer.getCurrentEra() / 2;
	}
*/
	// WTF code duplication
	if (kPlayer.getAdvancedStartPoints() >= 0)
	{
		iTargetSize += 2 + kPlayer.getCurrentEra();
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

	if (iBonusFoodDiff < 2)
	{
		iFoodMultiplier += 10 * (2 - iBonusFoodDiff);
	}

	int iExtraFoodForGrowth = (std::max(0, iTargetSize - getPopulation()) + 3) / 4;
	if (getPopulation() < iTargetSize)
	{
		iExtraFoodForGrowth ++;
	}

	int iFoodDifference = iFoodTotal - ((iTargetSize * GC.getFOOD_CONSUMPTION_PER_POPULATION()) + iExtraFoodForGrowth);

	int iDesiredFoodChange = -iFoodDifference + std::max(0, -iHealth);
	if (iTargetSize > getPopulation())
	{
		if (iDesiredFoodChange > 3)
		{
			iDesiredFoodChange = (iDesiredFoodChange + 3) / 2;
		}
	}

	if (iFoodDifference < 0)
	{
		iFoodMultiplier +=  -iFoodDifference * 4;
	}

	if (iFoodDifference > 4)
	{
		iFoodMultiplier -= 8 + 4 * iFoodDifference;
	}

	if (iProductionTotal < 10)
	{
		iProductionMultiplier += (80 - 8 * iProductionTotal);
	}
	int iProductionTarget = 1 + (std::min(getPopulation(), (iTargetSize * 3) / 5));

	if (iProductionTotal < iProductionTarget)
	{
		iProductionMultiplier += 8 * (iProductionTarget - iProductionTotal);
	}

	if ((iBonusFoodSurplus + iFeatureFoodSurplus > 5) && ((iBonusFoodDeficit + iHillFoodDeficit) > 5))
	{
		if ((iBonusFoodDeficit + iHillFoodDeficit) > 8)
		{
			//probably a good candidate for a wonder pump
			iProductionMultiplier += 40;
			iCommerceMultiplier += (kPlayer.AI_isFinancialTrouble()) ? 0 : -40;
		}
	}


	int iNetCommerce = 1 + kPlayer.getCommerceRate(COMMERCE_GOLD) + kPlayer.getCommerceRate(COMMERCE_RESEARCH) + std::max(0, kPlayer.getGoldPerTurn());
/************************************************************************************************/
/* UNOFFICIAL_PATCH					   06/11/09					   jdog5000 & DanF5771	*/
/*																							  */
/* Bugfix																					   */
/************************************************************************************************/
/* original BTS code
	int iNetExpenses = kPlayer.calculateInflatedCosts() + std::min(0, kPlayer.getGoldPerTurn());
*/
	int iNetExpenses = kPlayer.calculateInflatedCosts() + std::max(0, -kPlayer.getGoldPerTurn());
/************************************************************************************************/
/* UNOFFICIAL_PATCH						END												  */
/************************************************************************************************/
	int iRatio = (100 * iNetExpenses) / std::max(1, iNetCommerce);

	if (iRatio > 40)
	{
		iCommerceMultiplier += (33 * (iRatio - 40)) / 60;
	}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  05/06/09								jdog5000	  */
/*																							  */
/* Worker AI																					*/
/************************************************************************************************/
	// AI no longer uses emphasis except for short term boosts.
	if( isHuman() )
	{
		if (AI_isEmphasizeYield(YIELD_FOOD))
		{
			iFoodMultiplier *= 130;
			iFoodMultiplier /= 100;
		}
		if (AI_isEmphasizeYield(YIELD_PRODUCTION))
		{
			iProductionMultiplier *= 140;
			iProductionMultiplier /= 100;
		}
		if (AI_isEmphasizeYield(YIELD_COMMERCE))
		{
			iCommerceMultiplier *= 140;
			iCommerceMultiplier /= 100;
		}
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

	int iProductionAdvantage = 100 * AI_yieldMultiplier(YIELD_PRODUCTION);
	iProductionAdvantage /= kPlayer.AI_averageYieldMultiplier(YIELD_PRODUCTION);
	iProductionAdvantage *= kPlayer.AI_averageYieldMultiplier(YIELD_COMMERCE);
	iProductionAdvantage /= AI_yieldMultiplier(YIELD_COMMERCE);

	//now we normalize the effect by # of cities

	int iNumCities = kPlayer.getNumCities();
	FAssert(iNumCities > 0);//superstisious?

	//in short in an OCC the relative multipliers should *never* make a difference
	//so this indeed equals "100" for the iNumCities == 0 case.
	iProductionAdvantage = ((iProductionAdvantage * (iNumCities - 1) + 200) / (iNumCities + 1));

	iProductionMultiplier *= iProductionAdvantage;
	iProductionMultiplier /= 100;

	iCommerceMultiplier *= 100;
	iCommerceMultiplier /= iProductionAdvantage;

	int iGreatPeopleAdvantage = 100 * getTotalGreatPeopleRateModifier();
	iGreatPeopleAdvantage /= kPlayer.AI_averageGreatPeopleMultiplier();
	iGreatPeopleAdvantage = ((iGreatPeopleAdvantage * (iNumCities - 1) + 200) / (iNumCities + 1));
	iGreatPeopleAdvantage += 200; //gpp multipliers are larger than others so lets not go overboard
	iGreatPeopleAdvantage /= 3;

	//With great people we want to slightly increase food priority at the expense of commerce
	//this gracefully handles both wonder and specialist based GPP...
	iCommerceMultiplier *= 100;
	iCommerceMultiplier /= iGreatPeopleAdvantage;
	iFoodMultiplier *= iGreatPeopleAdvantage;
	iFoodMultiplier /= 100;

	// if leader flavor likes production, increase production, reduce commerce
	if (kPlayer.AI_isDoStrategy(AI_STRATEGY_PRODUCTION))
	{
		iProductionMultiplier += 10;
		iCommerceMultiplier -= 10;
	}

	if (iFoodMultiplier < 100)
	{
		iFoodMultiplier = 10000 / (200 - iFoodMultiplier);
	}
	if (iProductionMultiplier < 100)
	{
		iProductionMultiplier = 10000 / (200 - iProductionMultiplier);
	}
	if (iCommerceMultiplier < 100)
	{
		iCommerceMultiplier = 10000 / (200 - iCommerceMultiplier);
	}

	if (angryPopulation(1) > 0)
	{
		iFoodMultiplier /= 2;
	}

	if (!bChop)
	{
		ProjectTypes eProductionProject = getProductionProject();
		bChop = (eProductionProject != NO_PROJECT && AI_projectValue(eProductionProject) > 0);
	}
	if (!bChop)
	{
		BuildingTypes eProductionBuilding = getProductionBuilding();
		bChop = (eProductionBuilding != NO_BUILDING && isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eProductionBuilding).getBuildingClassType())));
	}
	if (!bChop)
	{
		UnitTypes eProductionUnit = getProductionUnit();
		bChop = (eProductionUnit != NO_UNIT && GC.getUnitInfo(eProductionUnit).isFoodProduction());
	}
	if (!bChop)
	{
		bChop = ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING));
	}


/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 06/17/09														*/
/*																							  */
/*																							  */
/************************************************************************************************/
	for (iI = 0; iI < getNumCityPlots(); iI++)
/************************************************************************************************/
/* JOOYO_ADDON						  END													 */
/************************************************************************************************/
	{
		m_aiBestBuildValue[iI] = 0;
		m_aeBestBuild[iI] = NO_BUILD;

		if (iI != CITY_HOME_PLOT)
		{
			pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

			if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this)
			{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  03/16/10								jdog5000	  */
/*																							  */
/* City AI, Worker AI																		   */
/************************************************************************************************/
				int iLastBestBuildValue = m_aiBestBuildValue[iI];
				BuildTypes eLastBestBuildType = m_aeBestBuild[iI];

				AI_bestPlotBuild(pLoopPlot, &(m_aiBestBuildValue[iI]), &(m_aeBestBuild[iI]), iFoodMultiplier, iProductionMultiplier, iCommerceMultiplier, bChop, iHappyAdjust, iHealthAdjust, iDesiredFoodChange);
				m_aiBestBuildValue[iI] *= 4;
				m_aiBestBuildValue[iI] += 3 + iWorkerCount;  // to round up
				m_aiBestBuildValue[iI] /= (4 + iWorkerCount);

				if (m_aiBestBuildValue[iI] > 0)
				{
					FAssert(m_aeBestBuild[iI] != NO_BUILD);
				}
				if (m_aeBestBuild[iI] != NO_BUILD)
				{
					FAssert(m_aiBestBuildValue[iI] > 0);
				}

				if( eLastBestBuildType != NO_BUILD )
				{
					if( eLastBestBuildType != m_aeBestBuild[iI] )
					{
						if( iWorkerCount > 0 )
						{
							// BBAI TODO: Check workers are building the new thing

							if( gCityLogLevel >= 2 )
							{
								logBBAIForTeam(getTeam(),  "	  City %S switches best build on plot %d, %d from %S (%d) to %S (%d) with worker count %d",getName().GetCString(),pLoopPlot->getX(),pLoopPlot->getY(),GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(eLastBestBuildType).getImprovement()).getDescription(),iLastBestBuildValue,GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(m_aeBestBuild[iI]).getImprovement()).getDescription(),m_aiBestBuildValue[iI],iWorkerCount);
							}
						}
					}
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
			}
		}
	}


	{	//new experimental yieldValue calcuation
		short aiYields[NUM_YIELD_TYPES];
		int iBestPlot = -1;
		int iBestPlotValue = -1;
		int iValue;

		int iBestUnworkedPlotValue = 0;

		int aiValues[NUM_CITY_PLOTS];

/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 06/17/09														*/
/*																							  */
/*																							  */
/************************************************************************************************/
		for (iI = 0; iI < getNumCityPlots(); iI++)
/************************************************************************************************/
/* JOOYO_ADDON						  END													 */
/************************************************************************************************/
		{
			if (iI != CITY_HOME_PLOT)
			{
				pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

				if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this)
				{
					if (m_aeBestBuild[iI] != NO_BUILD)
					{
						for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
						{
							aiYields[iJ] = pLoopPlot->getYieldWithBuild(m_aeBestBuild[iI], (YieldTypes)iJ, true);
						}

						iValue = AI_yieldValue(aiYields, NULL, false, false, false, false, true, true);
						aiValues[iI] = iValue;
						if ((iValue > 0) && (pLoopPlot->getRouteType() != NO_ROUTE))
						{
							iValue++;
						}
						//FAssert(iValue > 0);

						iValue = std::max(0, iValue);

						m_aiBestBuildValue[iI] *= iValue + 100;
						m_aiBestBuildValue[iI] /= 100;

						if (iValue > iBestPlotValue)
						{
							iBestPlot = iI;
							iBestPlotValue = iValue;
						}
					}
					if (!pLoopPlot->isBeingWorked())
					{
						for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
						{
							aiYields[iJ] = pLoopPlot->getYield((YieldTypes)iJ);
						}

						iValue = AI_yieldValue(aiYields, NULL, false, false, false, false, true, true);

						iBestUnworkedPlotValue = std::max(iBestUnworkedPlotValue, iValue);
					}
				}
			}
		}
		if (iBestPlot != -1)
		{
			m_aiBestBuildValue[iBestPlot] *= 2;
		}

		//Prune plots which are sub-par.
		if (iBestUnworkedPlotValue > 0)
		{
/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 06/17/09														*/
/*																							  */
/*																							  */
/************************************************************************************************/
			for (iI = 0; iI < getNumCityPlots(); iI++)
/************************************************************************************************/
/* JOOYO_ADDON						  END													 */
/************************************************************************************************/
			{
				if (iI != CITY_HOME_PLOT)
				{
					pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

					if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this)
					{
						if (m_aeBestBuild[iI] != NO_BUILD)
						{
							if (!pLoopPlot->isBeingWorked() && (pLoopPlot->getImprovementType() == NO_IMPROVEMENT))
							{
								if (GC.getBuildInfo(m_aeBestBuild[iI]).getImprovement() != NO_IMPROVEMENT)
								{
									if ((aiValues[iI] <= iBestUnworkedPlotValue) && (aiValues[iI] < 500))
									{
										m_aiBestBuildValue[iI] = 1;
									}
								}
							}
							else if ((pLoopPlot->getImprovementType() != NO_IMPROVEMENT) && (GC.getBuildInfo(m_aeBestBuild[iI]).getImprovement() != NO_IMPROVEMENT))
							{
								for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
								{
									aiYields[iJ] = pLoopPlot->getYield((YieldTypes)iJ);
								}

								iValue = AI_yieldValue(aiYields, NULL, false, false, false, false, true, true);
								if (iValue > aiValues[iI])
								{
									m_aiBestBuildValue[iI] = 1;
								}
							}
						}
					}
				}
			}
		}
	}
}

// Protected Functions...

// Better drafting strategy by Blake - thank you!
void CvCityAI::AI_doDraft(bool bForce)
{
	PROFILE_FUNC();

	FAssert(!isHuman());
	if (isBarbarian())
		return;

	// xUPT: Do not conscript if area is full (dbkblk, 2015-02)
	if (m_bAreaSaturatedOfLandMilitaryUnits)
		return; // f1rpo: Moved up (no functional change)

	if (canConscript())
	{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  07/12/09								jdog5000	  */
/*																							  */
/* City AI, War Strategy AI																	 */
/************************************************************************************************/
		if (GC.getGameINLINE().AI_combatValue(getConscriptUnit()) > 33)
		{
			if (bForce)
			{
				conscript();
				return;
			}
			bool bLandWar = ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING));
			bool bDanger = (!AI_isDefended() && AI_isDanger());

			// Don't go broke from drafting
			if( !bDanger && GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble() )
			{
				return;
			}

			// Don't shrink cities too much
			int iConscriptPop = getConscriptPopulation();
			if ( !bDanger && (3 * (getPopulation() - iConscriptPop) < getHighestPopulation() * 2) )
			{
				return;
			}

			// Large cities want a little spare happiness
			int iHappyDiff = GC.getDefineINT("CONSCRIPT_POP_ANGER") - iConscriptPop + getPopulation()/10;

			if (bLandWar && (0 == angryPopulation(iHappyDiff))
				&& (bDanger || GET_PLAYER(getOwnerINLINE()).AI_isFocusWar())) // f1rpo (advc.017)
			{
				bool bWait = true;

				if( bWait && GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_TURTLE) )
				{
					// Full out defensive
					if( bDanger || (getPopulation() >= std::max(5, getHighestPopulation() - 1)) )
					{
						bWait = false;
					}
					else if( (AI_countWorkedPoorTiles() + std::max(0,(visiblePopulation() - AI_countGoodSpecialists(true)))) >= 1 )
					{
						bWait = false;
					}
				}

				if( bWait && bDanger )
				{
					// If city might be captured, don't hold back
					int iOurDefense = GET_TEAM(getTeam()).AI_getOurPlotStrength(plot(),0,true,false,true);
					int iEnemyOffense = GET_PLAYER(getOwnerINLINE()).AI_getEnemyPlotStrength(plot(),2,false,false);

					if( (iOurDefense == 0) || (3*iEnemyOffense > 2*iOurDefense) )
					{
						bWait = false;
					}
				}

				if( bWait )
				{
					// Non-critical, only burn population if population is not worth much
					if ((getConscriptAngerTimer() == 0) && ((AI_countWorkedPoorTiles() + std::max(0,(visiblePopulation() - AI_countGoodSpecialists(true)))) >= iConscriptPop))
					{
						if( (getPopulation() >= std::max(5, getHighestPopulation() - 2)) ) // f1rpo: was ...-1
						{	// <f1rpo> Tie it to AI_buildUnitProb
							if (per100(AI_buildUnitProb(true)).bernoulliSuccess(
								GC.getGame().getSorenRand(), "advc.017")) // </f1rpo>
							{
								bWait = false;
							}
						}
					}
				}

				if( !bWait && gCityLogLevel >= 2 )
				{
					logBBAIForTeam(getTeam(), "	  City %S (size %d, highest %d) chooses to conscript with danger: %d, land war: %d, poor tiles: %d, bad specialists: %d", getName().GetCString(), getPopulation(), getHighestPopulation(), bDanger, bLandWar, AI_countWorkedPoorTiles(), std::max(0,(visiblePopulation() - AI_countGoodSpecialists(true))) );
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
				if (!bWait)
				{
					conscript();
				}
			}
		}
	}
}

// Better pop-rushing strategy by Blake - thank you!
void CvCityAI::AI_doHurry(bool bForce)
{
	PROFILE_FUNC();

	CvArea* pWaterArea;
	UnitTypes eProductionUnit;
	UnitAITypes eProductionUnitAI;
	BuildingTypes eProductionBuilding;
	int iHurryAngerLength;
	int iHurryPopulation;
	int iMinTurns;
	bool bEssential;
	bool bGrowth;
	int iI, iJ;

	FAssert(!isHuman() || isProductionAutomated());

	if (isBarbarian())
	{
		return;
	}

	if ((getProduction() == 0) && !bForce)
	{
		return;
	}

	pWaterArea = waterArea();

	eProductionUnit = getProductionUnit();
	eProductionUnitAI = getProductionUnitAI();
	eProductionBuilding = getProductionBuilding();

	if (eProductionUnit != NO_UNIT){ //	xUPT (dbkblk, 2015-02)
		if (GC.getUnitInfo(eProductionUnit).isMilitaryProduction() && (GC.getUnitInfo(eProductionUnit).getDomainType() == DOMAIN_LAND) && m_bAreaSaturatedOfLandMilitaryUnits){
			return;
		}
		if (!GC.getUnitInfo(eProductionUnit).isMilitaryProduction() && (GC.getUnitInfo(eProductionUnit).getDomainType() != DOMAIN_SEA) && m_bAreaSaturatedOfCivilianUnits){
			return;
		}
	}

	bool const bDanger = AI_isDanger();

	for (iI = 0; iI < GC.getNumHurryInfos(); iI++)
	{
		if (canHurry((HurryTypes)iI))
		{
			if (bForce)
			{
				hurry((HurryTypes)iI);
				break;
			}
			iHurryAngerLength = hurryAngerLength((HurryTypes)iI);
			iHurryPopulation = hurryPopulation((HurryTypes)iI);

			iMinTurns = MAX_INT;
			bEssential = false;
			bGrowth = false;

			// Whip to eliminate unhappiness - thank you Blake!
			if (getProduction() > 0)
			{
				if (AI_getHappyFromHurry((HurryTypes)iI) > 0)
				{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  10/02/09								jdog5000	  */
/*																							  */
/* AI logging																				   */
/************************************************************************************************/
					if( gCityLogLevel >= 2 )
					{
						logBBAIForTeam(getTeam(), "	  City %S hurry to remove unhappiness", getName().GetCString() );
					}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
					hurry((HurryTypes)iI);
					break;
				}
			}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  12/07/09								jdog5000	  */
/*																							  */
/* City AI																					  */
/************************************************************************************************/
			// Rush defenses when in big trouble
			if ( (area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE) && GET_TEAM(getTeam()).AI_getEnemyPowerPercent(true) > 150 )
			{
				if( eProductionUnit != NO_UNIT && GC.getGameINLINE().AI_combatValue(eProductionUnit) > 33 && getProduction() > 0 )
				{
					if( (iHurryPopulation > 0) && (iHurryAngerLength == 0 || getHurryAngerTimer() < 2) && (iHurryPopulation < 3 && iHurryPopulation < getPopulation()/3))
					{
						bool bWait = true;

						if( GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_TURTLE) )
						{
							bWait = false;
						}
						else if( (3*(getPopulation() - iHurryPopulation)) < getHighestPopulation()*2 )
						{
							bWait = true;
						}
						else if( GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble() )
						{
							bWait = true;
						}
						else
						{
							for( int iJ = 0; iJ < MAX_CIV_TEAMS; iJ++ )
							{
								if( GET_TEAM((TeamTypes)iJ).isAlive() && !GET_TEAM((TeamTypes)iJ).isMinorCiv() )
								{
									if( GET_TEAM(getTeam()).isAtWar((TeamTypes)iJ) && GET_TEAM(getTeam()).AI_getAtWarCounter((TeamTypes)iJ) < 10 )
									{
										bWait = false;
										break;
									}
								}
							}
						}

						if( !bWait )
						{
							if( gCityLogLevel >= 2 )
							{
								logBBAIForTeam(getTeam(), "	  City %S hurry pop at %d to rush defenses for recent attack", getName().GetCString(), iHurryPopulation );
							}
							hurry((HurryTypes)iI);
							break;
						}
					}
					else
					{
						if( !(GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble()) )
						{
							int iHurryGold = hurryGold((HurryTypes)iI);
							if( iHurryGold > 0 && iHurryAngerLength == 0 )
							{
								//bool bDanger = AI_isDanger(); // f1rpo: redundant
								bool bWait = true;

								if( GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_TURTLE) )
								{
									if( (bDanger ? 5 : 8)*iHurryGold < GET_PLAYER(getOwnerINLINE()).getGold() )
									{
										bWait = false;
									}
								}
								else
								{
									if( (bDanger ? 8 : 12)*iHurryGold < GET_PLAYER(getOwnerINLINE()).getGold() )
									{
										bWait = false;
									}
								}

								if( !bWait )
								{
									if( gCityLogLevel >= 2 )
									{
										logBBAIForTeam(getTeam(), "	  City %S hurry gold at %d to rush defenses for recent attack", getName().GetCString(), iHurryGold );
									}
									hurry((HurryTypes)iI);
									break;
								}
							}
						}
					}
				}
			}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

			if ((iHurryAngerLength == 0) && (iHurryPopulation == 0))
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_avoidScience())
				{
					if (GET_PLAYER(getOwnerINLINE()).getGold() > GET_PLAYER(getOwnerINLINE()).AI_goldTarget())
					{
						iMinTurns = std::min(iMinTurns, 10);
					}
				}
				if (eProductionBuilding != NO_BUILDING)
				{
					int iValuePerTurn = AI_buildingValueThreshold(eProductionBuilding, BUILDINGFOCUS_GOLD | BUILDINGFOCUS_MAINTENANCE | BUILDINGFOCUS_PRODUCTION);

					iValuePerTurn /= 3;

					if (iValuePerTurn > 0)
					{
						int iHurryGold = hurryGold((HurryTypes)iI);
/************************************************************************************************/
/* UNOFFICIAL_PATCH					   08/06/09								jdog5000	  */
/*																							  */
/* Bugfix																					   */
/************************************************************************************************/
/* original bts code
						if ((iHurryGold / iValuePerTurn) < getProductionTurnsLeft(eProductionBuilding, 1))
*/
						if ( (iHurryGold > 0) && ((iHurryGold / iValuePerTurn) < getProductionTurnsLeft(eProductionBuilding, 1)) )
/************************************************************************************************/
/* UNOFFICIAL_PATCH						END												  */
/************************************************************************************************/
						{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  08/06/09								jdog5000	  */
/*																							  */
/* Gold AI																					  */
/************************************************************************************************/
/* original bts code
							if (iHurryGold < (GET_PLAYER(getOwnerINLINE()).getGold() / 3))
*/
							int iGoldThreshold = GET_PLAYER(getOwnerINLINE()).getGold();
							iGoldThreshold -= (GET_PLAYER(getOwnerINLINE()).AI_goldToUpgradeAllUnits() / ((GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0) ? 1 : 3));
							iGoldThreshold /= 3;
							if (iHurryGold < iGoldThreshold)
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
							{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  10/02/09								jdog5000	  */
/*																							  */
/* AI logging																				   */
/************************************************************************************************/
								if( gCityLogLevel >= 2 )
								{
									logBBAIForTeam(getTeam(), "	  City %S hurry gold at %d < threshold %d", getName().GetCString(), iHurryGold, iGoldThreshold );
								}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
								hurry((HurryTypes)iI);
								return;
							}
						}
					}
				}
			}

			if (eProductionBuilding != NO_BUILDING)
			{
				if (isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eProductionBuilding).getBuildingClassType())))
				{
					iMinTurns = std::min(iMinTurns, 10);
				}

				if (GC.getBuildingInfo(eProductionBuilding).getDefenseModifier() > 0)
				{
					if (bDanger)
					{
						iMinTurns = std::min(iMinTurns, 3);
						bEssential = true;
					}
				}

				if (GC.getBuildingInfo(eProductionBuilding).getBombardDefenseModifier() > 0)
				{
					if (bDanger)
					{
						iMinTurns = std::min(iMinTurns, 3);
						bEssential = true;
					}
				}

				if (GC.getBuildingInfo(eProductionBuilding).getYieldModifier(YIELD_PRODUCTION) > 0)
				{
					if (getBaseYieldRate(YIELD_PRODUCTION) >= 6)
					{
						iMinTurns = std::min(iMinTurns, 10);
						bGrowth = true;
					}
				}

				if ((GC.getBuildingInfo(eProductionBuilding).getCommerceChange(COMMERCE_CULTURE) > 0) ||
						(GC.getBuildingInfo(eProductionBuilding).getObsoleteSafeCommerceChange(COMMERCE_CULTURE) > 0))
				{
					if ((getCommerceRateTimes100(COMMERCE_CULTURE) == 0) || (plot()->calculateCulturePercent(getOwnerINLINE()) < 40))
					{
						iMinTurns = std::min(iMinTurns, 10);
						if (getCommerceRateTimes100(COMMERCE_CULTURE) == 0)
						{
							bEssential = true;
							iMinTurns = std::min(iMinTurns, 5);
							if (AI_countNumBonuses(NO_BONUS, false, true, 2, true, true) > 0)
							{
								bGrowth = true;
							}
						}
					}
				}

				if (GC.getBuildingInfo(eProductionBuilding).getHappiness() > 0)
				{
					if (angryPopulation() > 0)
					{
						iMinTurns = std::min(iMinTurns, 10);
					}
				}

				if (GC.getBuildingInfo(eProductionBuilding).getHealth() > 0)
				{
					if (healthRate() < 0)
					{
						iMinTurns = std::min(iMinTurns, 10);
					}
				}

				if (GC.getBuildingInfo(eProductionBuilding).getSeaPlotYieldChange(YIELD_FOOD) > 0 || GC.getBuildingInfo(eProductionBuilding).getRiverPlotYieldChange(YIELD_FOOD) > 0)
				{

					iMinTurns = std::min(iMinTurns, 10);

					if (AI_buildingSpecialYieldChangeValue(eProductionBuilding, YIELD_FOOD) > (getPopulation() * 2))
					{
						bEssential = true;
						bGrowth = true;
					}
				}

				if (GC.getBuildingInfo(eProductionBuilding).getFreeExperience() > 0)
				{
					if (bDanger)
					{
						iMinTurns = std::min(iMinTurns, 3);
						bEssential = true;
					}
				}

				if (GC.getBuildingInfo(eProductionBuilding).getMaintenanceModifier() < 0)
				{
					if (getMaintenance() >= 10)
					{
						iMinTurns = std::min(iMinTurns, 10);
						bEssential = true;
					}
				}

				if (GC.getDefineINT("DEFAULT_SPECIALIST") != NO_SPECIALIST)
				{
					if (getSpecialistCount((SpecialistTypes)(GC.getDefineINT("DEFAULT_SPECIALIST"))) > 0)
					{
						for (iJ = 0; iJ < GC.getNumSpecialistInfos(); iJ++)
						{
							if (GC.getBuildingInfo(eProductionBuilding).getSpecialistCount(iJ) > 0)
							{
								iMinTurns = std::min(iMinTurns, 10);
								break;
							}
						}
					}
				}

				if (GC.getBuildingInfo(eProductionBuilding).getCommerceModifier(COMMERCE_GOLD) > 0)
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble())
					{
						if (getBaseCommerceRate(COMMERCE_GOLD) >= 16)
						{
							iMinTurns = std::min(iMinTurns, 10);
						}
					}
				}

				if (GC.getBuildingInfo(eProductionBuilding).getCommerceModifier(COMMERCE_RESEARCH) > 0)
				{
					if (!(GET_PLAYER(getOwnerINLINE()).AI_avoidScience()))
					{
						if (getBaseCommerceRate(COMMERCE_RESEARCH) >= 16)
						{
							iMinTurns = std::min(iMinTurns, 10);
						}
					}
				}

				if (GC.getBuildingInfo(eProductionBuilding).getFoodKept() > 0)
				{
					iMinTurns = std::min(iMinTurns, 5);
					bEssential = true;
					bGrowth = true;
				}
			}

			if (eProductionUnit != NO_UNIT && !m_bAreaSaturatedOfLandMilitaryUnits)
			{
				if (GC.getUnitInfo(eProductionUnit).getDomainType() == DOMAIN_LAND)
				{
/************************************************************************************************/
/* Afforess					  Start		 6/11/11												*/
/*																							  */
/*																							  */
/************************************************************************************************/
					if ((GC.getUnitInfo(eProductionUnit).getCombat() +
					GET_TEAM(getTeam()).getUnitClassStrengthChange((UnitClassTypes)GC.getUnitInfo(eProductionUnit).getUnitClassType())) > 0)
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
					{
						if (bDanger)
						{
							iMinTurns = std::min(iMinTurns, 3);
							bEssential = true;
						}
					}
				}
			}

			if (eProductionUnitAI == UNITAI_CITY_DEFENSE && !m_bAreaSaturatedOfLandMilitaryUnits)
			{
				if (plot()->plotCheck(PUF_isUnitAIType, UNITAI_SETTLE, -1, getOwnerINLINE()) != NULL)
				{
					if (!AI_isDefended(-2*GET_PLAYER(getOwnerINLINE()).strengthOfBestUnitAI(DOMAIN_LAND, UNITAI_CITY_DEFENSE))) // XXX check for other team's units?
					{
						iMinTurns = std::min(iMinTurns, 5);
					}
				}
			}

			if (eProductionUnitAI == UNITAI_SETTLE && !m_bAreaSaturatedOfCivilianUnits)
			{
				if (area()->getNumAIUnits(getOwnerINLINE(), UNITAI_SETTLE) == 0)
				{
					if (!(GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble()))
					{
						if (area()->getBestFoundValue(getOwnerINLINE()) > 0)
						{
							iMinTurns = std::min(iMinTurns, 5);
							bEssential = true;
							bGrowth = true;
						}
					}
				}
			}

			if (eProductionUnitAI == UNITAI_SETTLER_SEA)
			{
				if (pWaterArea != NULL)
				{
					if (pWaterArea->getNumAIUnits(getOwnerINLINE(), UNITAI_SETTLER_SEA) == 0)
					{
						if (area()->getNumAIUnits(getOwnerINLINE(), UNITAI_SETTLE) > 0)
						{
							iMinTurns = std::min(iMinTurns, 5);
						}
					}
				}
			}

			if (eProductionUnitAI == UNITAI_WORKER && !m_bAreaSaturatedOfCivilianUnits)
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_neededWorkers(area()) > (area()->getNumAIUnits(getOwnerINLINE(), UNITAI_WORKER) * 2))
				{
					iMinTurns = std::min(iMinTurns, 5);
					bEssential = true;
					bGrowth = true;
				}
			}

			if (eProductionUnitAI == UNITAI_WORKER_SEA)
			{
				if (AI_neededSeaWorkers() > 0)
				{
					iMinTurns = std::min(iMinTurns, 5);
					bEssential = true;
					bGrowth = true;
				}
			}

			// adjust for game speed
			if (NO_UNIT != getProductionUnit())
			{
				iMinTurns *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent();
			}
			else if (NO_BUILDING != getProductionBuilding())
			{
				iMinTurns *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent();
			}
			else if (NO_PROJECT != getProductionProject())
			{
				iMinTurns *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getCreatePercent();
			}
			else
			{
				iMinTurns *= 100;
			}

			iMinTurns /= 100;

			//this overrides everything.
			if (bGrowth)
			{
				int iHurryGold = hurryGold((HurryTypes)iI);
				if ((iHurryGold > 0) && ((iHurryGold * 16) < GET_PLAYER(getOwnerINLINE()).getGold()))
				{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  10/02/09								jdog5000	  */
/*																							  */
/* AI logging																				   */
/************************************************************************************************/
					if( gCityLogLevel >= 2 )
					{
						logBBAIForTeam(getTeam(), "	  City %S hurry gold at %d for growth when rich at %d", getName().GetCString(), iHurryGold, GET_PLAYER(getOwnerINLINE()).getGold() );
					}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
					hurry((HurryTypes)iI);
					break;
				}
/************************************************************************************************/
/* UNOFFICIAL_PATCH					   08/06/09								jdog5000	  */
/*																							  */
/* Bugfix																					   */
/************************************************************************************************/
/* original bts code
				if (AI_countGoodTiles((healthRate(0) == 0), false, 100) <= (getPopulation() - iHurryPopulation))
				{
					hurry((HurryTypes)iI);
					break;
				}
			}
			if (AI_countGoodTiles((healthRate(0) == 0), false, 100) <= (getPopulation() - iHurryPopulation))
			{
*/
				// Only consider population hurry if that's actually what the city can do!!!
				if( (iHurryPopulation > 0) && (getPopulation() > iHurryPopulation) )
				{
					//BBAI TODO: could be (bEssential ? 100 : 80) instead
					if (AI_countGoodTiles((healthRate(0) == 0), false, 100) <= (getPopulation() - iHurryPopulation))
					{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  10/02/09								jdog5000	  */
/*																							  */
/* AI logging																				   */
/************************************************************************************************/
						if( gCityLogLevel >= 2 )
						{
							logBBAIForTeam(getTeam(), "	  City %S hurry pop at %d for growth with bad tiles with pop %d", getName().GetCString(), iHurryPopulation, getPopulation() );
						}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
						hurry((HurryTypes)iI);
						break;
					}
				}
			}
			if ((iHurryPopulation > 0) && (AI_countGoodTiles((healthRate(0) == 0), false, 100) <= (getPopulation() - iHurryPopulation)))
			{
/************************************************************************************************/
/* UNOFFICIAL_PATCH						END												  */
/************************************************************************************************/
				if (getProductionTurnsLeft() > iMinTurns)
				{
					bool bWait = isHuman();

					if ((iHurryPopulation * 3) > (getProductionTurnsLeft() * 2))
					{
						bWait = true;
					}

					if (!bWait)
					{
						if (iHurryAngerLength > 0)
						{
							//is the whip just too small or the population just too reduced to bother?
							if (!bEssential && ((iHurryPopulation < (1 + GC.getDefineINT("HURRY_POP_ANGER"))) || ((getPopulation() - iHurryPopulation) <= std::max(3, (getHighestPopulation() / 2)))))
							{
								bWait = true;
							}
							else
							{
								//sometimes it's worth whipping even with existing anger
								if (getHurryAngerTimer() > 1)
								{
									if (!bEssential)
									{
										bWait = true;
									}
									else if (GC.getDefineINT("HURRY_POP_ANGER") == iHurryPopulation && angryPopulation() > 0)
									{
										//ideally we'll whip something more expensive
										bWait = true;
									}
								}
							}

							//if the city is just lame then don't whip the poor thing
							//(it'll still get whipped when unhappy/unhealthy)
							if (!bWait && !bEssential)
							{
								int iFoodSurplus = 0;
								CvPlot * pLoopPlot;

/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 06/17/09														*/
/*																							  */
/*																							  */
/************************************************************************************************/
								for (iJ = 0; iJ < getNumCityPlots(); iJ++)
/************************************************************************************************/
/* JOOYO_ADDON						  END													 */
/************************************************************************************************/
								{
									if (iJ != CITY_HOME_PLOT)
									{
										pLoopPlot = getCityIndexPlot(iJ);

										if (pLoopPlot != NULL)
										{
											if (pLoopPlot->getWorkingCity() == this)
											{
												iFoodSurplus += std::max(0, pLoopPlot->getYield(YIELD_FOOD) - GC.getFOOD_CONSUMPTION_PER_POPULATION());
											}
										}
									}
								}

								if (iFoodSurplus < 3)
								{
									bWait = true;
								}
							}
						}
					}

					if (!bWait)
					{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  10/02/09								jdog5000	  */
/*																							  */
/* AI logging																				   */
/************************************************************************************************/
						if( gCityLogLevel >= 2 )
						{
							logBBAIForTeam(getTeam(), "	  City %S hurry pop at %d with bad tiles and no reason to wait with pop %d", getName().GetCString(), iHurryPopulation, getPopulation() );
						}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
						hurry((HurryTypes)iI);
						break;
					}
				}
			}
		}
	}
}


// Improved use of emphasize by Blake, to go with his whipping strategy - thank you!
void CvCityAI::AI_doEmphasize()
{
	PROFILE_FUNC();

	FAssert(!isHuman());

	bool bFirstTech;
	bool bEmphasize;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  03/08/10								jdog5000	  */
/*																							  */
/* Victory Strategy AI																		  */
/************************************************************************************************/
	bool bCultureVictory = GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

	//Note from Blake:
	//Emphasis proved to be too clumsy to manage AI economies,
	//as such it's been nearly completely phased out by
	//the AI_specialYieldMultiplier system which allows arbitary
	//value-boosts and works very well.
	//Ideally the AI should never use emphasis.
	int iI;

	if (GET_PLAYER(getOwnerINLINE()).getCurrentResearch() != NO_TECH)
	{
		bFirstTech = GET_PLAYER(getOwnerINLINE()).AI_isFirstTech(GET_PLAYER(getOwnerINLINE()).getCurrentResearch());
	}
	else
	{
		bFirstTech = false;
	}

	int iPopulationRank = findPopulationRank();

	for (iI = 0; iI < GC.getNumEmphasizeInfos(); iI++)
	{
		bEmphasize = false;

		if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getYieldChange(YIELD_FOOD) > 0)
		{

		}

		if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getYieldChange(YIELD_PRODUCTION) > 0)
		{

		}

		if (AI_specialYieldMultiplier(YIELD_PRODUCTION) < 50)
		{
			if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getYieldChange(YIELD_COMMERCE) > 0)
			{
				if (bFirstTech)
				{
					bEmphasize = true;
				}
			}

			if (GC.getEmphasizeInfo((EmphasizeTypes)iI).getCommerceChange(COMMERCE_RESEARCH) > 0)
			{
				if (bFirstTech && !bCultureVictory)
				{
					if (iPopulationRank < ((GET_PLAYER(getOwnerINLINE()).getNumCities() / 4) + 1))
					{
						bEmphasize = true;
					}
				}
			}

			if (GC.getEmphasizeInfo((EmphasizeTypes)iI).isGreatPeople())
			{
				int iHighFoodTotal = 0;
				int iHighFoodPlotCount = 0;
				int iHighHammerPlotCount = 0;
				int iHighHammerTotal = 0;
				int iGoodFoodSink = 0;
				int iFoodPerPop = GC.getFOOD_CONSUMPTION_PER_POPULATION();
/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 06/17/09														*/
/*																							  */
/*																							  */
/************************************************************************************************/
				for (int iPlot = 0; iPlot < getNumCityPlots(); iPlot++)
/************************************************************************************************/
/* JOOYO_ADDON						  END													 */
/************************************************************************************************/
				{
					CvPlot* pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iPlot);
					if (pLoopPlot != NULL && pLoopPlot->getWorkingCity() == this)
					{
						int iFood = pLoopPlot->getYield(YIELD_FOOD);
						if (iFood > iFoodPerPop)
						{
							iHighFoodTotal += iFood;
							iHighFoodPlotCount++;
						}
						int iHammers = pLoopPlot->getYield(YIELD_PRODUCTION);
						if ((iHammers >= 3) && ((iHammers + iFood) >= 4))
						{
							iHighHammerPlotCount++;
							iHighHammerTotal += iHammers;
						}
						int iCommerce = pLoopPlot->getYield(YIELD_COMMERCE);
						if ((iCommerce * 2 + iHammers * 3) > 9)
						{
							iGoodFoodSink += std::max(0, iFoodPerPop - iFood);
						}
					}
				}

				if ((iHighFoodTotal + iHighFoodPlotCount - iGoodFoodSink) >= foodConsumption(true))
				{
					if ((iHighHammerPlotCount < 2) && (iHighHammerTotal < (getPopulation())))
					{
						if (AI_countGoodTiles(true, false, 100, true) < getPopulation())
						{
							bEmphasize = true;
						}
					}
				}
			}
		}

		AI_setEmphasize(((EmphasizeTypes)iI), bEmphasize);
	}
}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  01/09/10								jdog5000	  */
/*																							  */
/* City AI																					  */
/************************************************************************************************/
/********************************************************************************/
/* 	City Defenders						24.07.2010				Fuyu			*/
/********************************************************************************/
//Fuyu bIgnoreNotUnitAIs
bool CvCityAI::AI_chooseUnit(const char* reason, UnitAITypes eUnitAI, int iOdds, int iUnitStrength, int iPriorityOverride, CvUnitSelectionCriteria* criteria)
{
	CvString	unitAIType;

#ifdef USE_UNIT_TENDERING
	//	Have we already contracted for a unit?
	if ( m_bRequestedUnit )
	{
		return false;
	}
	UnitTypes eProductionUnit = getProductionUnit();
	if (eProductionUnit != NO_UNIT)
	{ // xUPT (dbkblk, 2015-02)
		if (GC.getUnitInfo(eProductionUnit).isMilitaryProduction() && (GC.getUnitInfo(eProductionUnit).getDomainType() == DOMAIN_LAND) && m_bAreaSaturatedOfLandMilitaryUnits)
		{
			return false;
		}
		if (!GC.getUnitInfo(eProductionUnit).isMilitaryProduction() && (GC.getUnitInfo(eProductionUnit).getDomainType() != DOMAIN_SEA) && m_bAreaSaturatedOfCivilianUnits)
		{
			return false;
		}
	}

#endif
	if( iOdds < 0 || getCitySorenRandNum(100, "City AI choose unit") < iOdds )
	{
		if ( iPriorityOverride == -1 )
		{
			iPriorityOverride = m_iTempBuildPriority;
		}

		if (eUnitAI != NO_UNITAI)
		{
			CvInfoBase& AIType = GC.getUnitAIInfo(eUnitAI);
			unitAIType = AIType.getType();
		}
		else
		{
			unitAIType = "NO_UNITAI";
		}

#ifdef USE_UNIT_TENDERING
		if ( !isBarbarian() )
		{
			//	Check someone can build a suitable unit before putting out a tender for one.
			if ( eUnitAI == NO_UNITAI || GET_PLAYER(getOwnerINLINE()).bestBuildableUnitForAIType(NO_DOMAIN, eUnitAI, criteria) != NO_UNIT )
			{
				if( gCityLogLevel >= 2 )
				{
					logBBAIForTeam(getTeam(), "	  City %S pop %d puts out tenders for %d unit strength of AIType: %s at priority %d (reason: %s)", getName().GetCString(), getPopulation(), iUnitStrength, unitAIType.c_str(), iPriorityOverride, reason);
				}
				//	Put out a tender for this unit
				GET_PLAYER(getOwnerINLINE()).getContractBroker().advertiseWork(iPriorityOverride, NO_UNITCAPABILITIES, getX_INLINE(), getY_INLINE(), NULL, eUnitAI, iUnitStrength, criteria);

				m_bRequestedUnit = true;

				return m_bRequestedBuilding;
			}
			//	Looks like we probably can't build it, but fallback check if we can ourselves, and if so just
			//	queue it here (handles edge cases where building requirements mean it can be built locally but
			//	not in capital or best coastal).  Do this by falling through to local handling
		}
#endif

		return AI_chooseUnitImmediate(reason, eUnitAI, criteria);
	}

	return false;
}

bool CvCityAI::AI_chooseUnitImmediate(const char* reason, UnitAITypes eUnitAI, CvUnitSelectionCriteria* criteria)
{
	int			iDummyValue;
	UnitTypes	eBestUnit;

	if (eUnitAI != NO_UNITAI)
	{
		eBestUnit = AI_bestUnitAI(eUnitAI, iDummyValue, false, false, criteria);
	}
	else
	{
		eBestUnit = AI_bestUnit(iDummyValue, -1, NULL, false, &eUnitAI, false, false, criteria);
	}

	if (eBestUnit != NO_UNIT)
	{ // xUPT (dbkblk, 2015-02)
		if (GC.getUnitInfo(eBestUnit).isMilitaryProduction() && (GC.getUnitInfo(eBestUnit).getDomainType() == DOMAIN_LAND) && m_bAreaSaturatedOfLandMilitaryUnits)
		{
			return false;
		}
		if (!GC.getUnitInfo(eBestUnit).isMilitaryProduction() && (GC.getUnitInfo(eBestUnit).getDomainType() != DOMAIN_SEA) && m_bAreaSaturatedOfCivilianUnits)
		{
			return false;
		}
		if( gCityLogLevel >= 2 )
		{
			CvString	unitAIType;

			if (eUnitAI != NO_UNITAI)
			{
				CvInfoBase& AIType = GC.getUnitAIInfo(eUnitAI);
				unitAIType = AIType.getType();
			}
			else
			{
				unitAIType = "NO_UNITAI";
			}

			logBBAIForTeam(getTeam(), "	  City %S pop %d builds unit of AIType: %s at priority %d (reason: %s)", getName().GetCString(), getPopulation(), unitAIType.c_str(), m_iTempBuildPriority, reason);
		}

		setProposedOrder(ORDER_TRAIN, eBestUnit, eUnitAI);
		return true;
	}

	return false;
}

/********************************************************************************/
/* 	City Defenders												END 			*/
/********************************************************************************/
bool CvCityAI::AI_chooseUnit(UnitTypes eUnit, UnitAITypes eUnitAI)
{
	if (eUnit != NO_UNIT)
	{ // xUPT (dbkblk, 2015-02)
		if (GC.getUnitInfo(eUnit).isMilitaryProduction() && (GC.getUnitInfo(eUnit).getDomainType() == DOMAIN_LAND) && m_bAreaSaturatedOfLandMilitaryUnits)
		{
			return false;
		}
		if (!GC.getUnitInfo(eUnit).isMilitaryProduction() && (GC.getUnitInfo(eUnit).getDomainType() != DOMAIN_SEA) && m_bAreaSaturatedOfCivilianUnits)
		{
			return false;
		}
		setProposedOrder(ORDER_TRAIN, eUnit, eUnitAI);
		return true;
	}
	return false;
}

bool CvCityAI::AI_chooseDefender(const char* reason)
{
	if (plot()->plotCheck(PUF_isUnitAIType, UNITAI_CITY_SPECIAL, -1, getOwnerINLINE()) == NULL)
	{
		if (AI_chooseUnit(reason, UNITAI_CITY_SPECIAL))
		{
			return true;
		}
	}

	if (plot()->plotCheck(PUF_isUnitAIType, UNITAI_CITY_COUNTER, -1, getOwnerINLINE()) == NULL)
	{
		if (AI_chooseUnit(reason, UNITAI_CITY_COUNTER))
		{
			return true;
		}
	}

	if (AI_chooseUnit(reason, UNITAI_CITY_DEFENSE))
	{
		return true;
	}

	return false;
}

bool CvCityAI::AI_chooseLeastRepresentedUnit(const char* reason, UnitTypeWeightArray &allowedTypes, int iOdds)
{
	if ( iOdds < 0 || iOdds > getCitySorenRandNum(100, "AI choose least represented unit overall odds") )
	{
		int iValue;

		UnitTypeWeightArray::iterator it;

 		std::multimap<int, UnitAITypes, std::greater<int> > bestTypes;
 		std::multimap<int, UnitAITypes, std::greater<int> >::iterator best_it;
 		int iTotalWeight = 0;

		for (it = allowedTypes.begin(); it != allowedTypes.end(); ++it)
		{
			iValue = it->second;
			iValue *= 750 + getCitySorenRandNum(250, "AI choose least represented unit");
			iValue /= std::max(1,GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), it->first));
			bestTypes.insert(std::make_pair(iValue, it->first));

			iTotalWeight += iValue/100;
		}

		int iChoiceWeight = getCitySorenRandNum(iTotalWeight, "AI choose least represented unit");

 		for (best_it = bestTypes.begin(); best_it != bestTypes.end(); ++best_it)
 		{
			iChoiceWeight -= best_it->first/100;

			CvString reasonString;

			reasonString.Format("least represented (%s)", reason);

			if ( iChoiceWeight <= 0 && AI_chooseUnit(reasonString.c_str(), best_it->second) )
			{
				return true;
			}
 		}
	}

	return false;
}

bool CvCityAI::AI_bestSpreadUnit(bool bMissionary, bool bExecutive, int iBaseChance, UnitTypes* eBestSpreadUnit, int* iBestSpreadUnitValue)
{
	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	CvTeamAI& kTeam = GET_TEAM(getTeam());
	CvGame& kGame = GC.getGame();

	FAssert(eBestSpreadUnit != NULL && iBestSpreadUnitValue != NULL);

	int iBestValue = 0;

	if (bMissionary)
	{
		for (int iReligion = 0; iReligion < GC.getNumReligionInfos(); iReligion++)
		{
			ReligionTypes eReligion = (ReligionTypes)iReligion;
/************************************************************************************************/
/* Afforess					  Start		 09/15/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
			int iNeededMissionaries = kPlayer.AI_neededMissionaries(area(), eReligion);
/*
			if (isHasReligion(eReligion))
*/
			if (isHasReligion(eReligion) && iNeededMissionaries > 0)
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
			{
				int iHasCount = kPlayer.getHasReligionCount(eReligion);
				FAssert(iHasCount > 0);
				int iRoll = (iHasCount > 4) ? iBaseChance : (((100 - iBaseChance) / iHasCount) + iBaseChance);
				if (kPlayer.AI_isDoStrategy(AI_STRATEGY_MISSIONARY))
				{
					iRoll *= (kPlayer.getStateReligion() == eReligion) ? 170 : 65;
					iRoll /= 100;
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  03/08/10								jdog5000	  */
/*																							  */
/* Victory Strategy AI																		  */
/************************************************************************************************/
				if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2))
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
				{
					iRoll += 25;
				}
				else if (!kTeam.hasHolyCity(eReligion) && !(kPlayer.getStateReligion() == eReligion))
				{
					iRoll /= 2;
					if (kPlayer.isNoNonStateReligionSpread())
					{
						iRoll /= 2;
					}
				}

/************************************************************************************************/
/* RevDCM					  Start		 5/1/09												 */
/*																							  */
/* Inquisitions																				 */
/************************************************************************************************/
				if (kPlayer.isPushReligiousVictory() && (kPlayer.getStateReligion() == eReligion))
				{
					iRoll += 25;
				} else if(kPlayer.isConsiderReligiousVictory() && (kPlayer.getStateReligion() == eReligion))
				{
					iRoll += 7;
				} else if( (kPlayer.isPushReligiousVictory() || kPlayer.isConsiderReligiousVictory())
				&& (kPlayer.getStateReligion() != eReligion) )
				{
					iRoll = 0;
				}
/************************************************************************************************/
/* Inquisitions						 END														*/
/************************************************************************************************/

				if (iRoll > kGame.getSorenRandNum(100, "AI choose missionary"))
				{
					int iReligionValue = kPlayer.AI_missionaryValue(area(), eReligion);
					if (iReligionValue > 0)
					{
						for (int iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
						{
							UnitTypes eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

							if (eLoopUnit != NO_UNIT)
							{
								CvUnitInfo& kUnitInfo = GC.getUnitInfo(eLoopUnit);
								if (kUnitInfo.getReligionSpreads(eReligion) > 0)
								{
									if (canTrain(eLoopUnit))
									{
										int iValue = iReligionValue;
										iValue /= kUnitInfo.getProductionCost();

										if (iValue > iBestValue)
										{
											iBestValue = iValue;
											*eBestSpreadUnit = eLoopUnit;
											*iBestSpreadUnitValue = iReligionValue;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (bExecutive)
	{
		for (int iCorporation = 0; iCorporation < GC.getNumCorporationInfos(); iCorporation++)
		{
			CorporationTypes eCorporation = (CorporationTypes)iCorporation;
			if (isActiveCorporation(eCorporation) && kPlayer.getHasCorporationCount(eCorporation) > 0)
			{
				int iHasCount = kPlayer.getHasCorporationCount(eCorporation);
				FAssert(iHasCount > 0);
				int iRoll = (iHasCount > 4) ? iBaseChance : (((100 - iBaseChance) / iHasCount) + iBaseChance);
				if (!kTeam.hasHeadquarters(eCorporation))
				{
					iRoll /= 8;
				}

				if (iRoll > kGame.getSorenRandNum(100, "AI choose executive"))
				{
					int iCorporationValue = kPlayer.AI_executiveValue(area(), eCorporation);
					if (iCorporationValue > 0)
					{
						for (int iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
						{
							UnitTypes eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

							if (eLoopUnit != NO_UNIT)
							{
								CvUnitInfo& kUnitInfo = GC.getUnitInfo(eLoopUnit);
								if (kUnitInfo.getCorporationSpreads(eCorporation) > 0)
								{
									if (canTrain(eLoopUnit))
									{
										int iValue = iCorporationValue;
										iValue /= kUnitInfo.getProductionCost();

										int iLoop = 0;
										int iTotalCount = 0;
										int iPlotCount = 0;
										for (CvUnit* pLoopUnit = kPlayer.firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = kPlayer.nextUnit(&iLoop))
										{
											if ((pLoopUnit->AI_getUnitAIType() == UNITAI_MISSIONARY) && (pLoopUnit->getUnitInfo().getCorporationSpreads(eCorporation) > 0))
											{
												iTotalCount++;
												if (pLoopUnit->plot() == plot())
												{
													iPlotCount++;
												}
											}
										}
										iCorporationValue /= std::max(1, (iTotalCount / 4) + iPlotCount);

										int iCost = std::max(0, GC.getCorporationInfo(eCorporation).getSpreadCost() * (100 + GET_PLAYER(getOwnerINLINE()).calculateInflationRate()));
										iCost /= 100;

										if (kPlayer.getGold() >= iCost)
										{
											iCost *= GC.getDefineINT("CORPORATION_FOREIGN_SPREAD_COST_PERCENT");
											iCost /= 100;
											if (kPlayer.getGold() < iCost && iTotalCount > 1)
											{
												iCorporationValue /= 2;
											}
										}
										else if (iTotalCount > 1)
										{
											iCorporationValue /= 5;
										}
										if (iValue > iBestValue)
										{
											iBestValue = iValue;
											*eBestSpreadUnit = eLoopUnit;
											*iBestSpreadUnitValue = iCorporationValue;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return (*eBestSpreadUnit != NULL);
}

bool CvCityAI::AI_chooseBuilding(int iFocusFlags, int iMaxTurns, int iMinThreshold, int iOdds, bool bMaximizeFlaggedValue)
{
	BuildingTypes eBestBuilding;

#ifdef USE_UNIT_TENDERING
	//	Have we already selected a building?
	if ( m_bRequestedBuilding )
	{
		return false;
	}

	m_iBuildPriority = m_iTempBuildPriority;
#endif

	eBestBuilding = AI_bestBuildingThreshold(iFocusFlags, iMaxTurns, iMinThreshold, false, NO_ADVISOR, bMaximizeFlaggedValue);

	if (eBestBuilding != NO_BUILDING)
	{
		if( iOdds < 0 ||
			getBuildingProduction(eBestBuilding) > 0 ||
			getCitySorenRandNum(100,"City AI choose building") < iOdds )
		{
			setProposedOrder(ORDER_CONSTRUCT, eBestBuilding);

#ifdef USE_UNIT_TENDERING
			m_bRequestedBuilding = true;

			return (isBarbarian() || m_bRequestedUnit);
#else
			return true;
#endif
		}
	}

	return false;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/


bool CvCityAI::AI_chooseProject()
{
	ProjectTypes eBestProject;

#ifdef USE_UNIT_TENDERING
	if ( m_bRequestedBuilding )
	{
		return false;
	}

	m_iBuildPriority = m_iTempBuildPriority;
#endif

	eBestProject = AI_bestProject();

	if (eBestProject != NO_PROJECT)
	{
		setProposedOrder(ORDER_CREATE, eBestProject);

		return true;
	}

	return false;
}


bool CvCityAI::AI_chooseProcess(CommerceTypes eCommerceType, int* commerceWeights)
{
	ProcessTypes eBestProcess;

#ifdef USE_UNIT_TENDERING
	if ( m_bRequestedBuilding )
	{
		return false;
	}

	m_iBuildPriority = m_iTempBuildPriority;
#endif

	eBestProcess = AI_bestProcess(eCommerceType, commerceWeights);

	if (eBestProcess != NO_PROCESS)
	{
		setProposedOrder(ORDER_MAINTAIN, eBestProcess);

		return true;
	}

	return false;
}


// Returns true if a worker was added to a plot...
bool CvCityAI::AI_addBestCitizen(bool bWorkers, bool bSpecialists, int* piBestPlot, SpecialistTypes* peBestSpecialist)
{
	PROFILE_FUNC();

	bool bAvoidGrowth = AI_avoidGrowth();
	bool bIgnoreGrowth = AI_ignoreGrowth();
	bool bIsSpecialistForced = false;

	int iBestSpecialistValue = 0;
	SpecialistTypes eBestSpecialist = NO_SPECIALIST;
	SpecialistTypes eBestForcedSpecialist = NO_SPECIALIST;

	if (bSpecialists)
	{
		// count the total forced specialists
		int iTotalForcedSpecialists = 0;
		for (int iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
		{
			int iForcedSpecialistCount = getForceSpecialistCount((SpecialistTypes)iI);
			if (iForcedSpecialistCount > 0)
			{
				bIsSpecialistForced = true;
				iTotalForcedSpecialists += iForcedSpecialistCount;
			}
		}

		// if forcing any specialists, find the best one that we can still assign
		if (bIsSpecialistForced)
		{
			int iBestForcedValue = MIN_INT;

			int iTotalSpecialists = 1 + getSpecialistPopulation();
			for (int iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
			{
				if (isSpecialistValid((SpecialistTypes)iI, 1))
				{
					int iForcedSpecialistCount = getForceSpecialistCount((SpecialistTypes)iI);
					if (iForcedSpecialistCount > 0)
					{
						int iSpecialistCount = getSpecialistCount((SpecialistTypes)iI);

						// the value is based on how close we are to our goal ratio forced/total
						int iForcedValue = ((iForcedSpecialistCount * 128) / iTotalForcedSpecialists) -  ((iSpecialistCount * 128) / iTotalSpecialists);
						if (iForcedValue >= iBestForcedValue)
						{
							int iSpecialistValue = AI_specialistValue((SpecialistTypes)iI, bAvoidGrowth, false);

							// if forced value larger, or if equal, does this specialist have a higher value
							if (iForcedValue > iBestForcedValue || iSpecialistValue > iBestSpecialistValue)
							{
								iBestForcedValue = iForcedValue;
								iBestSpecialistValue = iSpecialistValue;
								eBestForcedSpecialist = ((SpecialistTypes)iI);
								eBestSpecialist = eBestForcedSpecialist;
							}
						}
					}
				}
			}
		}

		// if we do not have a best specialist yet, then just find the one with the best value
		if (eBestSpecialist == NO_SPECIALIST)
		{
			for (int iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
			{
				if (isSpecialistValid((SpecialistTypes)iI, 1))
				{
					int iValue = AI_specialistValue(((SpecialistTypes)iI), bAvoidGrowth, false);
					if (iValue >= iBestSpecialistValue)
					{
						iBestSpecialistValue = iValue;
						eBestSpecialist = ((SpecialistTypes)iI);
					}
				}
			}
		}
	}

	int iBestPlotValue = 0;
	int iBestPlot = -1;
	if (bWorkers)
	{
		for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			if (iI != CITY_HOME_PLOT)
			{
				if (!isWorkingPlot(iI))
				{
					CvPlot* pLoopPlot = getCityIndexPlot(iI);

					if (pLoopPlot != NULL)
					{
						if (canWork(pLoopPlot))
						{
							int iValue = AI_plotValue(pLoopPlot, bAvoidGrowth, /*bRemove*/ false, /*bIgnoreFood*/ false, bIgnoreGrowth);

							if (iValue > iBestPlotValue)
							{
								iBestPlotValue = iValue;
								iBestPlot = iI;
							}
						}
					}
				}
			}
		}
	}

	// if we found a plot to work
	if (iBestPlot != -1)
	{
		// if the best plot value is better than the best specialist, or if we forcing and we could not assign a forced specialst
		if (iBestPlotValue > iBestSpecialistValue || (bIsSpecialistForced && eBestForcedSpecialist == NO_SPECIALIST))
		{
			// do not work the specialist
			eBestSpecialist = NO_SPECIALIST;
		}
	}

	if (eBestSpecialist != NO_SPECIALIST)
	{
		changeSpecialistCount(eBestSpecialist, 1);
		if (piBestPlot != NULL)
		{
			FAssert(peBestSpecialist != NULL);
			*peBestSpecialist = eBestSpecialist;
			*piBestPlot = -1;
		}
		return true;
	}
	else if (iBestPlot != -1)
	{
		setWorkingPlot(iBestPlot, true);
		if (piBestPlot != NULL)
		{
			FAssert(peBestSpecialist != NULL);
			*peBestSpecialist = NO_SPECIALIST;
			*piBestPlot = iBestPlot;

		}
		return true;
	}

	return false;
}


// Returns true if a worker was removed from a plot...
bool CvCityAI::AI_removeWorstCitizen(SpecialistTypes eIgnoreSpecialist)
{
	CvPlot* pLoopPlot;
	SpecialistTypes eWorstSpecialist;
	bool bAvoidGrowth;
	bool bIgnoreGrowth;
	int iWorstPlot;
	int iValue;
	int iWorstValue;
	int iI;

	// if we are using more specialists than the free ones we get
	if (extraFreeSpecialists() < 0)
	{
		// does generic 'citizen' specialist exist?
		if (GC.getDefineINT("DEFAULT_SPECIALIST") != NO_SPECIALIST)
		{
			// is ignore something other than generic citizen?
			if (eIgnoreSpecialist != GC.getDefineINT("DEFAULT_SPECIALIST"))
			{
				// do we have at least one more generic citizen than we are forcing?
				if (getSpecialistCount((SpecialistTypes)(GC.getDefineINT("DEFAULT_SPECIALIST"))) > getForceSpecialistCount((SpecialistTypes)(GC.getDefineINT("DEFAULT_SPECIALIST"))))
				{
					// remove the extra generic citzen
					changeSpecialistCount(((SpecialistTypes)(GC.getDefineINT("DEFAULT_SPECIALIST"))), -1);
					return true;
				}
			}
		}
	}

	bAvoidGrowth = AI_avoidGrowth();
	bIgnoreGrowth = AI_ignoreGrowth();

	iWorstValue = MAX_INT;
	eWorstSpecialist = NO_SPECIALIST;
	iWorstPlot = -1;

	// if we are using more specialists than the free ones we get
	if (extraFreeSpecialists() < 0)
	{
		for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
		{
			if (eIgnoreSpecialist != iI)
			{
				if (getSpecialistCount((SpecialistTypes)iI) > getForceSpecialistCount((SpecialistTypes)iI))
				{
					iValue = AI_specialistValue(((SpecialistTypes)iI), bAvoidGrowth, /*bRemove*/ true);

					if (iValue < iWorstValue)
					{
						iWorstValue = iValue;
						eWorstSpecialist = ((SpecialistTypes)iI);
						iWorstPlot = -1;
					}
				}
			}
		}
	}

	// check all the plots we working
	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		if (iI != CITY_HOME_PLOT)
		{
			if (isWorkingPlot(iI))
			{
				pLoopPlot = getCityIndexPlot(iI);

				if (pLoopPlot != NULL)
				{
					iValue = AI_plotValue(pLoopPlot, bAvoidGrowth, /*bRemove*/ true, /*bIgnoreFood*/ false, bIgnoreGrowth);

					if (iValue < iWorstValue)
					{
						iWorstValue = iValue;
						eWorstSpecialist = NO_SPECIALIST;
						iWorstPlot = iI;
					}
				}
			}
		}
	}

	if (eWorstSpecialist != NO_SPECIALIST)
	{
		changeSpecialistCount(eWorstSpecialist, -1);
		return true;
	}
	else if (iWorstPlot != -1)
	{
		setWorkingPlot(iWorstPlot, false);
		return true;
	}

	// if we still have not removed one, then try again, but do not ignore the one we were told to ignore
	if (extraFreeSpecialists() < 0)
	{
		for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
		{
			if (getSpecialistCount((SpecialistTypes)iI) > 0)
			{
				iValue = AI_specialistValue(((SpecialistTypes)iI), bAvoidGrowth, /*bRemove*/ true);

				if (iValue < iWorstValue)
				{
					iWorstValue = iValue;
					eWorstSpecialist = ((SpecialistTypes)iI);
					iWorstPlot = -1;
				}
			}
		}
	}

	if (eWorstSpecialist != NO_SPECIALIST)
	{
		changeSpecialistCount(eWorstSpecialist, -1);
		return true;
	}

	return false;
}


void CvCityAI::AI_juggleCitizens()
{
	MEMORY_TRACK()

	bool bAvoidGrowth = AI_avoidGrowth();
	bool bIgnoreGrowth = AI_ignoreGrowth();

	//Afforess: remove citizens from worked buildings if starving
	if (!isHuman() && foodDifference(false) <= 0 && getNumPopulationEmployed() > 0)
	{
		for (int iI = 0; iI < GC.getNumBuildingInfos(); iI++)
		{
			if (getNumRealBuilding((BuildingTypes)iI) > 0 && !isDisabledBuilding((BuildingTypes)iI))
			{
				if (GC.getBuildingInfo((BuildingTypes)iI).getNumPopulationEmployed() > 0)
				{
					int iPrevFoodDiff = foodDifference(false);
					setDisabledBuilding((BuildingTypes)iI, true);
					//Re-enable building if it makes the situation worse
					if (iPrevFoodDiff > foodDifference(false))
					{
						setDisabledBuilding((BuildingTypes)iI, false);
					}
				}
			}
		}
	}

	// one at a time, remove the worst citizen, then add the best citizen
	// until we add back the same one we removed
	for (int iPass = 0; iPass < 2; iPass++)
	{
		bool bCompletedChecks = false;
		int iCount = 0;

		//std::vector<int> aWorstPlots;
		bool worstPlots[NUM_CITY_PLOTS];
		memset(worstPlots, 0, sizeof(worstPlots));

		while (!bCompletedChecks)
		{
			int iLowestValue = MAX_INT;
			int iWorstPlot = -1;
			int iValue;

			for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
			{
				if (iI != CITY_HOME_PLOT)
				{
					if (isWorkingPlot(iI))
					{
						CvPlot* pLoopPlot = getCityIndexPlot(iI);

						if (pLoopPlot != NULL)
						{
								iValue = AI_plotValue(pLoopPlot, bAvoidGrowth, /*bRemove*/ true, /*bIgnoreFood*/ false, bIgnoreGrowth, (iPass == 0));

								// use <= so that we pick the last one that is lowest, to avoid infinite loop with AI_addBestCitizen
								if (iValue <= iLowestValue)
								{
									iLowestValue = iValue;
									iWorstPlot = iI;
								}
							}
						}
					}
				}

			// if no worst plot, or we looped back around and are trying to remove the first plot we removed, stop
			//if (iWorstPlot == -1 || std::find(aWorstPlots.begin(), aWorstPlots.end(), iWorstPlot) != aWorstPlots.end())
			if (iWorstPlot == -1 || worstPlots[iWorstPlot])
			{
				bCompletedChecks = true;
			}
			else
			{
				// if this the first worst plot, remember it
				//aWorstPlots.push_back(iWorstPlot);
				worstPlots[iWorstPlot] = true;

				setWorkingPlot(iWorstPlot, false);

				if (AI_addBestCitizen(true, true))
				{
					if (isWorkingPlot(iWorstPlot))
					{
						bCompletedChecks = true;
					}
				}
			}

			iCount++;
			if (iCount > (NUM_CITY_PLOTS + 1))
			{
				FAssertMsg(false, "infinite loop");
				break; // XXX
			}
		}

		if ((iPass == 0) && (foodDifference(false) >= 0))
		{
			//good enough, the starvation code
			break;
		}
	}
}


bool CvCityAI::AI_potentialPlot(short* piYields)
{
	int iNetFood = piYields[YIELD_FOOD] - GC.getFOOD_CONSUMPTION_PER_POPULATION();

	if (iNetFood < 0)
	{
 		if (piYields[YIELD_FOOD] == 0)
		{
			if (piYields[YIELD_PRODUCTION] + piYields[YIELD_COMMERCE] < 2)
			{
				return false;
			}
		}
		else
		{
			if (piYields[YIELD_PRODUCTION] + piYields[YIELD_COMMERCE] == 0)
			{
				return false;
			}
		}
	}

	return true;
}


bool CvCityAI::AI_foodAvailable(int iExtra)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	bool abPlotAvailable[NUM_CITY_PLOTS];
	int iFoodCount;
	int iPopulation;
	int iBestPlot;
	int iValue;
	int iBestValue;
	int iI;

	iFoodCount = 0;

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		abPlotAvailable[iI] = false;
	}

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		pLoopPlot = getCityIndexPlot(iI);

		if (pLoopPlot != NULL)
		{
			if (iI == CITY_HOME_PLOT)
			{
				iFoodCount += pLoopPlot->getYield(YIELD_FOOD);
			}
			else if ((pLoopPlot->getWorkingCity() == this) && AI_potentialPlot(pLoopPlot->getYield()))
			{
				abPlotAvailable[iI] = true;
			}
		}
	}

	iPopulation = (getPopulation() + iExtra);

	while (iPopulation > 0)
	{
		iBestValue = 0;
		iBestPlot = CITY_HOME_PLOT;

/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 06/17/09														*/
/*																							  */
/*																							  */
/************************************************************************************************/
		for (iI = 0; iI < getNumCityPlots(); iI++)
/************************************************************************************************/
/* JOOYO_ADDON						  END													 */
/************************************************************************************************/
		{
			if (abPlotAvailable[iI])
			{
				iValue = getCityIndexPlot(iI)->getYield(YIELD_FOOD);

				if (iValue > iBestValue)
				{
					iBestValue = iValue;
					iBestPlot = iI;
				}
			}
		}

		if (iBestPlot != CITY_HOME_PLOT)
		{
			iFoodCount += iBestValue;
			abPlotAvailable[iBestPlot] = false;
		}
		else
		{
			break;
		}

		iPopulation--;
	}

	for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		iFoodCount += (GC.getSpecialistInfo((SpecialistTypes)iI).getYieldChange(YIELD_FOOD) * getFreeSpecialistCount((SpecialistTypes)iI));
	}

	if (iFoodCount < foodConsumption(false, iExtra))
	{
		return false;
	}

	return true;
}

#ifdef YIELD_VALUE_CACHING
void CvCityAI::AI_NoteSpecialistChange(void)
{
	ClearYieldValueCache();
}

void CvCityAI::AI_NoteWorkerChange(void)
{
	ClearYieldValueCache();
}

void CvCityAI::ClearYieldValueCache()
{
	if ( yieldValueCache.currentUseCounter > 0 )
	{
		yieldValueCache.currentUseCounter = 0;

		for(int i = 0; i < YIELD_VALUE_CACHE_SIZE; i++)
		{
			yieldValueCache.entries[i].iLastUseCount = 0;
		}
	}
}

void CvCityAI::CheckYieldValueCache(char* label)
{
	if ( yieldValueCache.currentUseCounter > 0 )
	{
		for(int i = 0; i < YIELD_VALUE_CACHE_SIZE; i++)
		{
			struct yieldValueCacheEntry* entry = &yieldValueCache.entries[i];

			if ( entry->iLastUseCount != 0 )
			{
				int realValue = AI_yieldValueInternal(entry->aiYields, entry->aiCommerceYields, entry->bAvoidGrowth, entry->bRemove, entry->bIgnoreFood, entry->bIgnoreGrowth, entry->bIgnoreStarvation, entry->bWorkerOptimization);

				FAssertMsg( realValue == entry->iResult, CvString::format("CheckYieldValueCache %s failed\n", label).c_str());
			}
		}
	}
}

int CvCityAI::yieldValueCacheHits = 0;
int CvCityAI::yieldValueCacheReads = 0;
#endif

int CvCityAI::AI_yieldValue(short* piYields, short* piCommerceYields, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood, bool bIgnoreGrowth, bool bIgnoreStarvation, bool bWorkerOptimization)
{
#ifdef YIELD_VALUE_CACHING
#ifdef _DEBUG
//	Uncomment this to perform functional verification
//#define VERIFY_YIELD_CACHE_RESULTS
#endif

	//	Check cache first
	int worstLRU = 0x7FFFFFFF;

	struct yieldValueCacheEntry* worstLRUEntry = NULL;
	yieldValueCacheReads++;

	//OutputDebugString(CvString::format("AI_yieldValue (%d,%d,%d) at seq %d\n", piYields[0], piYields[1], piYields[2], yieldValueCacheReads).c_str());
	//PROFILE_STACK_DUMP

	for(int i = 0; i < YIELD_VALUE_CACHE_SIZE; i++)
	{
		struct yieldValueCacheEntry* entry = &yieldValueCache.entries[i];
		if ( entry->iLastUseCount == 0 )
		{
			worstLRUEntry = entry;
			break;
		}

		bool bMatch = true;

		for( int j = 0; j < NUM_YIELD_TYPES; j++ )
		{
			if ( piYields[j] != entry->aiYields[j] )
			{
				bMatch = false;
				break;
			}
		}
		if ( bMatch )
		{
			if ( piCommerceYields != NULL )
			{
				for( int j = 0; j < NUM_COMMERCE_TYPES; j++ )
				{
					if ( piCommerceYields[j] != entry->aiCommerceYields[j] )
					{
						bMatch = false;
						break;
					}
				}
			}
			else
			{
				for( int j = 0; j < NUM_COMMERCE_TYPES; j++ )
				{
					if ( entry->aiCommerceYields[j] != 0 )
					{
						bMatch = false;
						break;
					}
				}
			}
		}

		if ( bMatch &&
			 entry->bAvoidGrowth == bAvoidGrowth &&
			 entry->bRemove == bRemove &&
			 entry->bIgnoreFood == bIgnoreFood &&
			 entry->bIgnoreGrowth == bIgnoreGrowth &&
			 entry->bIgnoreStarvation == bIgnoreStarvation)
		{
			entry->iLastUseCount = ++yieldValueCache.currentUseCounter;
			yieldValueCacheHits++;
#ifdef VERIFY_YIELD_CACHE_RESULTS
			int realValue = AI_yieldValueInternal(piYields, piCommerceYields, bAvoidGrowth, bRemove, bIgnoreFood, bIgnoreGrowth, bIgnoreStarvation, bWorkerOptimization);

			if ( realValue != entry->iResult )
			{
				OutputDebugString(CvString::format("Cache entry %08lx verification failed, turn is %d\n", entry, GC.getGameINLINE().getGameTurn()).c_str());
				FAssertMsg(false, "Yield value cache verification failure");
				CHECK_YIELD_VALUE_CACHE("Validation");
			}
#endif
			return entry->iResult;
		}
		else if ( entry->iLastUseCount < worstLRU )
		{
			worstLRU = entry->iLastUseCount;
			worstLRUEntry = entry;
		}
	}

	int iResult = AI_yieldValueInternal(piYields, piCommerceYields, bAvoidGrowth, bRemove, bIgnoreFood, bIgnoreGrowth, bIgnoreStarvation, bWorkerOptimization);

	FAssertMsg(worstLRUEntry != NULL, "No plot value cache entry found to replace");
	if ( worstLRUEntry != NULL )
	{
		for( int j = 0; j < NUM_YIELD_TYPES; j++ )
		{
			worstLRUEntry->aiYields[j] = piYields[j];
		}

		if ( piCommerceYields != NULL )
		{
			for( int j = 0; j < NUM_COMMERCE_TYPES; j++ )
			{
				worstLRUEntry->aiCommerceYields[j] = piCommerceYields[j];
			}
		}
		else
		{
			for( int j = 0; j < NUM_COMMERCE_TYPES; j++ )
			{
				worstLRUEntry->aiCommerceYields[j] = 0;
			}
		}

		worstLRUEntry->bAvoidGrowth = bAvoidGrowth;
		worstLRUEntry->bRemove = bRemove;
		worstLRUEntry->bIgnoreFood = bIgnoreFood;
		worstLRUEntry->bIgnoreGrowth = bIgnoreGrowth;
		worstLRUEntry->bIgnoreStarvation = bIgnoreStarvation;
		worstLRUEntry->bWorkerOptimization = bWorkerOptimization;
		worstLRUEntry->iResult = iResult;
		worstLRUEntry->iLastUseCount = ++yieldValueCache.currentUseCounter;
	}

	return iResult;
#else
	return AI_yieldValueInternal(piYields, piCommerceYields, bAvoidGrowth, bRemove, bIgnoreFood, bIgnoreGrowth, bIgnoreStarvation, bWorkerOptimization);
#endif
}


int CvCityAI::AI_yieldValueInternal(short* piYields, short* piCommerceYields, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood, bool bIgnoreGrowth, bool bIgnoreStarvation, bool bWorkerOptimization)
{
	PROFILE_FUNC()

	const int iBaseProductionValue = 15;
	const int iBaseCommerceValue[NUM_COMMERCE_TYPES] = {7,10,7,7};	//	Koshling - boost science a bit

	const int iMaxFoodValue = (5 * iBaseProductionValue) - 1;

	int aiYields[NUM_YIELD_TYPES];
	int aiCommerceYieldsTimes100[NUM_COMMERCE_TYPES];

	int iExtraProductionModifier = 0;
	int iBaseProductionModifier = 100;

	bool bEmphasizeFood = AI_isEmphasizeYield(YIELD_FOOD);
	bool bFoodIsProduction = isFoodProduction();
	bool bCanPopRush = GET_PLAYER(getOwnerINLINE()).canPopRush();

	for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
	{
		aiCommerceYieldsTimes100[iJ] = 0;
	}

	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		if (piYields[iI] == 0)
		{
			aiYields[iI] = 0;
		}
		else
		{
			// Get yield for city after adding/removing the citizen in question
			int iOldCityYield = getBaseYieldRate((YieldTypes)iI);
			int iNewCityYield = (bRemove ? (iOldCityYield - piYields[iI]) : (iOldCityYield + piYields[iI]));
			int iModifier = getBaseYieldRateModifier((YieldTypes)iI);
			if (iI == YIELD_PRODUCTION)
			{
				iBaseProductionModifier = iModifier;
				iExtraProductionModifier = getProductionModifier();
				iModifier += iExtraProductionModifier;
			}

			iNewCityYield = (iNewCityYield * iModifier) / 100;
			iOldCityYield = (iOldCityYield * iModifier) / 100;

			// The yield of the citizen in question is the difference of total yields
			// to account for rounding of modifiers
			aiYields[iI] = (bRemove ? (iOldCityYield - iNewCityYield) : (iNewCityYield - iOldCityYield));
		}
	}

	for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
	{
		int iModifier = getTotalCommerceRateModifier((CommerceTypes)iJ);

		int iCommerceTimes100 = aiYields[YIELD_COMMERCE] * GET_PLAYER(getOwnerINLINE()).getCommercePercent((CommerceTypes)iJ);
		if (piCommerceYields != NULL)
		{
			iCommerceTimes100 += piCommerceYields[iJ] * 100;
		}
		aiCommerceYieldsTimes100[iJ] += (iCommerceTimes100 * iModifier) / 100;
	}

/************************************************************************************************/
/* UNOFFICIAL_PATCH					   07/09/09								jdog5000	  */
/*																							  */
/* General AI																				   */
/************************************************************************************************/
/* original BTS code
	if (isProductionProcess() && !bWorkerOptimization)
	{
		for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
		{
			aiCommerceYieldsTimes100[iJ] += GC.getProcessInfo(getProductionProcess()).getProductionToCommerceModifier(iJ) * aiYields[YIELD_PRODUCTION];
		}

		aiYields[YIELD_PRODUCTION] = 0;
	}
*/
	// Above code causes governor and AI to heavily weight food when building any form of commerce,
	// which is not expected by human and does not seem to produce better results for AI either.
/************************************************************************************************/
/* UNOFFICIAL_PATCH						END												  */
/************************************************************************************************/

	// should not really use this much, but making it accurate
	aiYields[YIELD_COMMERCE] = 0;
	for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
	{
		aiYields[YIELD_COMMERCE] += aiCommerceYieldsTimes100[iJ] / 100;
	}

	int iValue = 0;
	int iSlaveryValue = 0;

	int iFoodGrowthValue = 0;
	int iFoodGPPValue = 0;

	if (!bIgnoreFood && aiYields[YIELD_FOOD] != 0)
	{
		// tiny food factor, to ensure that even when we don't want to grow,
		// we still prefer more food if everything else is equal
		iValue += (aiYields[YIELD_FOOD] * 1);

		int iFoodPerTurn = (foodDifference(false) - ((bRemove) ? aiYields[YIELD_FOOD] : 0));
		int iFoodLevel = getFood();
		int iFoodToGrow = growthThreshold();
		int iHealthLevel = goodHealth() - badHealth(/*bNoAngry*/ false, 0);
		int iHappinessLevel = (isNoUnhappiness() ? std::max(3, iHealthLevel + 5) : happyLevel() - unhappyLevel(0));
		int iPopulation = getPopulation();
		int	iExtraPopulationThatCanWork = std::min(iPopulation - range(-iHappinessLevel, 0, iPopulation) + std::min(0, extraFreeSpecialists()) , NUM_CITY_PLOTS) - getWorkingPopulation() + ((bRemove) ? 1 : 0);
		int iConsumtionPerPop = GC.getFOOD_CONSUMPTION_PER_POPULATION();

		int iAdjustedFoodDifference = (getYieldRate(YIELD_FOOD) + std::min(0, iHealthLevel)) - ((iPopulation + std::min(0, iHappinessLevel) - ((bRemove) ? 1 : 0)) * iConsumtionPerPop);

		// if we not human, allow us to starve to half full if avoiding growth
		if (!bIgnoreStarvation)
		{
			int iStarvingAllowance = 0;
			if (bAvoidGrowth && !isHuman())
			{
				iStarvingAllowance = std::max(0, (iFoodLevel - std::max(1, ((9 * iFoodToGrow) / 10))));
			}

			if ((iStarvingAllowance < 1) && (iFoodLevel > ((iFoodToGrow * 75) / 100)))
			{
				iStarvingAllowance = 1;
			}

			// if still starving
			if ((iFoodPerTurn + iStarvingAllowance) < 0)
			{
				// if working plots all like this one will save us from starving
				if (std::max(0, iExtraPopulationThatCanWork * aiYields[YIELD_FOOD]) >= -iFoodPerTurn)
				{
					// if this is high food, then we want to pick it first, this will allow us to pick some great non-food later
					int iHighFoodThreshold = std::min(getBestYieldAvailable(YIELD_FOOD), iConsumtionPerPop + 1);
					if (iFoodPerTurn <= (AI_isEmphasizeGreatPeople() ? 0 : -iHighFoodThreshold) && aiYields[YIELD_FOOD] >= iHighFoodThreshold)
					{
						// value all the food that will contribute to not starving
						iValue += 2048 * std::min(aiYields[YIELD_FOOD], -iFoodPerTurn);
					}
					else
					{
						// give a huge boost to this plot, but not based on how much food it has
						// ie, if working a bunch of 1f 7h plots will stop us from starving, then do not force working unimproved 2f plot
						iValue += 2048;
					}
				}
				else
				{
					// value food high(32), but not forced
					iValue += 32 * std::min(aiYields[YIELD_FOOD], -iFoodPerTurn);
				}
			}
		}

		// if food isn't production, then adjust for growth
		if (bWorkerOptimization || !bFoodIsProduction)
		{
			int iPopToGrow = 0;
			if (!bAvoidGrowth)
			{
				// only do relative checks on food if we want to grow AND we not emph food
				// the emp food case will just give a big boost to all food under all circumstances
				//if (bWorkerOptimization || (!bIgnoreGrowth))// && !bEmphasizeFood))
				{
					// also avail: iFoodLevel, iFoodToGrow

					// adjust iFoodPerTurn assuming that we work plots all equal to iConsumtionPerPop
					// that way it is our guesstimate of how much excess food we will have
					iFoodPerTurn += (iExtraPopulationThatCanWork * iConsumtionPerPop);

					// we have less than 10 extra happy, do some checks to see if we can increase it
					if (iHappinessLevel < 10)
					{
						// if we have anger becase no military, do not count it, on the assumption that it will
						// be remedied soon, and that we still want to grow
						if (getMilitaryHappinessUnits() == 0)
						{
							if (GET_PLAYER(getOwnerINLINE()).getNumCities() > 2)
							{
								iHappinessLevel += ((GC.getDefineINT("NO_MILITARY_PERCENT_ANGER") * (iPopulation + 1)) / GC.getPERCENT_ANGER_DIVISOR());
							}
						}

						// currently we can at most increase happy by 2 in the following checks
						const int kMaxHappyIncrease = 2;

						// if happy is large enough so that it will be over zero after we do the checks
						int iNewFoodPerTurn = iFoodPerTurn + aiYields[YIELD_FOOD] - iConsumtionPerPop;
						if ((iHappinessLevel + kMaxHappyIncrease) > 0 && iNewFoodPerTurn > 0)
						{
							int iApproxTurnsToGrow = (iNewFoodPerTurn > 0) ? ((iFoodToGrow - iFoodLevel) / iNewFoodPerTurn) : MAX_INT;

							// do we have hurry anger?
							int iHurryAngerTimer = getHurryAngerTimer();
							if (iHurryAngerTimer > 0)
							{
								int iTurnsUntilAngerIsReduced = iHurryAngerTimer % flatHurryAngerLength();

								// angry population is bad but if we'll recover by the time we grow...
								if (iTurnsUntilAngerIsReduced <= iApproxTurnsToGrow)
								{
									iHappinessLevel++;
								}
							}

							// do we have conscript anger?
							int iConscriptAngerTimer = getConscriptAngerTimer();
							if (iConscriptAngerTimer > 0)
							{
								int iTurnsUntilAngerIsReduced = iConscriptAngerTimer % flatConscriptAngerLength();

								// angry population is bad but if we'll recover by the time we grow...
								if (iTurnsUntilAngerIsReduced <= iApproxTurnsToGrow)
								{
									iHappinessLevel++;
								}
							}

							// do we have defy resolution anger?
							int iDefyResolutionAngerTimer = getDefyResolutionAngerTimer();
							if (iDefyResolutionAngerTimer > 0)
							{
								int iTurnsUntilAngerIsReduced = iDefyResolutionAngerTimer % flatDefyResolutionAngerLength();

								// angry population is bad but if we'll recover by the time we grow...
								if (iTurnsUntilAngerIsReduced <= iApproxTurnsToGrow)
								{
									iHappinessLevel++;
								}
							}
						}
					}

					if (bEmphasizeFood)
					{
						//If we are emphasize food, pay less heed to caps.
						iHealthLevel += 5;
						iHappinessLevel += 2;
					}
					else
					{
						//Afforess: Assume a little unhealthiness is not a bad thing
						iHealthLevel += 2;
					}

					bool bBarFull = (iFoodLevel + iFoodPerTurn /*+ aiYields[YIELD_FOOD]*/ > ((90 * iFoodToGrow) / 100));

					iPopToGrow = std::max(0, iHappinessLevel);
					//Afforess: prevent runaway growth with low health
					//Note: due to the above healthLevel +=2, <=2 is actually at equal health to unhealth or unhealthy...
					if (iPopToGrow > 0 && iHealthLevel <= 2)
					{
						iPopToGrow = 1;
					}
					int iGoodTiles = AI_countGoodTiles(iHealthLevel < 0, true, 50, true);
					iGoodTiles += AI_countGoodSpecialists(iHealthLevel > 0);
					iGoodTiles += bBarFull ? 0 : 1;

					if (!bEmphasizeFood)
					{
						iPopToGrow = std::min(iPopToGrow, iGoodTiles + ((bRemove) ? 1 : 0));
					}

					// if we have growth pontential, fill food bar to 85%
					bool bFillingBar = false;
					if (iPopToGrow == 0 && iHappinessLevel >= 0 && iGoodTiles >= 0 && iHealthLevel >= 0)
					{
						if (!bBarFull)
						{
							if (AI_specialYieldMultiplier(YIELD_PRODUCTION) < 50)
							{
								bFillingBar = true;
							}
						}
					}

					if (getPopulation() < 3)
					{
						iPopToGrow = std::max(iPopToGrow, 3 - getPopulation());
						iPopToGrow += 2;
					}

					// if we want to grow
					if (iPopToGrow > 0 || bFillingBar)
					{

						// will multiply this by factors
						iFoodGrowthValue = aiYields[YIELD_FOOD];
						if (iHealthLevel < (bFillingBar ? 0 : 1))
						{
							iFoodGrowthValue--;
						}

						// (range 1-25) - we want to grow more if we have a lot of growth to do
						// factor goes up like this: 0:1, 1:8, 2:9, 3:10, 4:11, 5:13, 6:14, 7:15, 8:16, 9:17, ... 17:25
						int iFactorPopToGrow;

						if (iPopToGrow < 1 || bFillingBar)
							iFactorPopToGrow = 20 - (10 * (iFoodLevel + iFoodPerTurn + aiYields[YIELD_FOOD])) / iFoodToGrow;
						else if (iPopToGrow < 2)
							iFactorPopToGrow = 13;
						else if (iPopToGrow < 7)
							iFactorPopToGrow = 17 + 3 * iPopToGrow;
						else
							iFactorPopToGrow = 41;

						iFoodGrowthValue *= iFactorPopToGrow;

						//If we already grow somewhat fast, devalue further food
						//Remember growth acceleration is not dependent on food eaten per
						//pop, 4f twice as fast as 2f twice as fast as 1f...
						int iHighGrowthThreshold = 2 + std::max(std::max(0, 5 - getPopulation()), (iPopToGrow + 1) / 2);
						if (bEmphasizeFood)
						{
							iHighGrowthThreshold *= 2;
						}

						if (iFoodPerTurn > iHighGrowthThreshold)
						{
							iFoodGrowthValue *= 25 + ((75 * iHighGrowthThreshold) / iFoodPerTurn);
							iFoodGrowthValue /= 100;
						}
					}
				}

				//very high food override
				if ((isHuman()) && ((iPopToGrow > 0) || bCanPopRush))
				{
					//very high food override
					int iTempValue = std::max(0, 30 * aiYields[YIELD_FOOD] - 15 * iConsumtionPerPop);
					iTempValue *= std::max(0, 3 * iConsumtionPerPop - iAdjustedFoodDifference);
					iTempValue /= 3 * iConsumtionPerPop;
					if (iHappinessLevel < 0)
					{
						iTempValue *= 2;
						iTempValue /= 1 + 2 * -iHappinessLevel;
					}
					iFoodGrowthValue += iTempValue;
				}
				//Slavery Override
				if (bCanPopRush && (iHappinessLevel > 0))
				{
					iSlaveryValue = 30 * 14 * std::max(0, aiYields[YIELD_FOOD] - ((iHealthLevel < 0) ? 1 : 0));
					iSlaveryValue /= std::max(10, (growthThreshold() * (100 - getMaxFoodKeptPercent())));

					iSlaveryValue *= 100;
					iSlaveryValue /= getHurryCostModifier(true);

					iSlaveryValue *= iConsumtionPerPop * 2;
					iSlaveryValue /= iConsumtionPerPop * 2 + std::max(0, iAdjustedFoodDifference);
				}

				//Great People Override
				if ((iExtraPopulationThatCanWork > 1) && AI_isEmphasizeGreatPeople())
				{
					int iAdjust = iConsumtionPerPop;
					if (iFoodPerTurn == 0)
					{
						iAdjust -= 1;
					}
					iFoodGPPValue += std::max(0, aiYields[YIELD_FOOD] - iAdjust) * std::max(0, (12 + 5 * std::min(0, iHappinessLevel)));
				}
			}
		}
	}


	int iProductionValue = 0;
	int iCommerceValue = 0;
	int iFoodValue = std::min(iFoodGrowthValue, iMaxFoodValue * aiYields[YIELD_FOOD]);
	// if food is production, the count it
	int adjustedYIELD_PRODUCTION = (((bFoodIsProduction) ? aiYields[YIELD_FOOD] : 0) + aiYields[YIELD_PRODUCTION]);

	// value production medium(15)
	iProductionValue += (adjustedYIELD_PRODUCTION * iBaseProductionValue);
	if (!isProduction() && !isHuman())
	{
		iProductionValue /= 2;
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  05/18/09								jdog5000	  */
/*																							  */
/* City AI																						 */
/************************************************************************************************/
	// If city has more than enough food, but very little production, add large value to production
	// Particularly helps coastal cities with plains forests
	if( aiYields[YIELD_PRODUCTION] > 0 )
	{
		if( !bFoodIsProduction && isProduction() )
		{
			if( foodDifference(false) >= GC.getFOOD_CONSUMPTION_PER_POPULATION() )
			{
				if( getYieldRate(YIELD_PRODUCTION) < (1 + getPopulation()/3) )
				{
					iValue += 128 + 8 * aiYields[YIELD_PRODUCTION];
				}
			}
		}
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

	// value commerce low(6)

	for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		if (aiCommerceYieldsTimes100[iI] != 0)
		{
			int iCommerceWeight = GET_PLAYER(getOwnerINLINE()).AI_commerceWeight((CommerceTypes)iI);
			if (AI_isEmphasizeCommerce((CommerceTypes)iI))
			{
				iCommerceWeight *= 200;
				iCommerceWeight /= 100;
			}
			if (iI == COMMERCE_CULTURE)
			{
				if (getCultureLevel() <= (CultureLevelTypes) 1)
				{
					iCommerceValue += (15 * aiCommerceYieldsTimes100[iI]) / 100;
				}
			}
			//	Koshling - Removing the 'normalization' by AI_averageCommerceExchange() from this calculation.  Basically what this did was
			//	to say that commerce types we have a civilization-wide good multiplier for should be devalued relative to those we don't.
			//	However, within the context of city yield evaluation it's not being compared just with other commerce types, but also with other
			//	yield types which have no similar normalization.  The result was that as average science multipliers increased (via buildings and
			//	so on) production was valued relatively more and more over science
			//iCommerceValue += (iCommerceWeight * (aiCommerceYieldsTimes100[iI] * iBaseCommerceValue) * GET_PLAYER(getOwnerINLINE()).AI_averageCommerceExchange((CommerceTypes)iI)) / 1000000;
			iCommerceValue += (iCommerceWeight * (aiCommerceYieldsTimes100[iI] * iBaseCommerceValue[iI])) / 10000;
		}
	}
/*
	if (!bWorkerOptimization && bEmphasizeFood)
	{
		if (!bFoodIsProduction)
		{
			// value food extremely high(180)
			iFoodValue *= 125;
			iFoodValue /= 100;
		}
	}

	if (!bWorkerOptimization && AI_isEmphasizeYield(YIELD_PRODUCTION))
	{
		// value production high(80)
		iProductionValue += (adjustedYIELD_PRODUCTION * 80);
	}
*/
	//Slavery translation
	if ((iSlaveryValue > 0) && (iSlaveryValue > iFoodValue))
	{
		//treat the food component as production
		iFoodValue = 0;
	}
	else
	{
		//treat it as just food
		iSlaveryValue = 0;
	}

	iFoodValue += iFoodGPPValue;
/*
	if (!bWorkerOptimization && AI_isEmphasizeYield(YIELD_COMMERCE))
	{
		for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			iCommerceValue += ((iCommerceYields[iI] * 40) * GET_PLAYER(getOwnerINLINE()).AI_averageCommerceExchange((CommerceTypes)iI)) / 100;
		}
	}

	for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
	{
		if (!bWorkerOptimization && AI_isEmphasizeCommerce((CommerceTypes) iJ))
		{
			// value the part of our commerce that goes to our emphasis medium (40)
			iCommerceValue += (iCommerceYields[iJ] * 40);
		}
	}
*/
	//Lets have some fun with the multipliers, this basically bluntens the impact of
	//massive bonuses.....

	//normalize the production... this allows the system to account for rounding
	//and such while preventing an "out to lunch smoking weed" scenario with
	//unusually high transient production modifiers.
	//Other yields don't have transient bonuses in quite the same way.

/************************************************************************************************/
/* UNOFFICIAL_PATCH					   05/16/10								jdog5000	  */
/*																							  */
/* City AI																					  */
/************************************************************************************************/
	// Rounding can be a problem, particularly for small commerce amounts.  Added safe guards to make
	// sure commerce is counted, even if just a tiny amount.
	if (AI_isEmphasizeYield(YIELD_PRODUCTION))
	{
		iProductionValue *= 130;
		iProductionValue /= 100;

		if (isFoodProduction())
		{
			iFoodValue *= 130;
			iFoodValue /= 100;
		}

		if (!AI_isEmphasizeYield(YIELD_COMMERCE) && iCommerceValue > 0)
		{
			iCommerceValue *= 60;
			iCommerceValue /= 100;
			iCommerceValue = std::max(1, iCommerceValue);
		}
		if (!AI_isEmphasizeYield(YIELD_FOOD) && iFoodValue > 0)
		{
			iFoodValue *= 75;
			iFoodValue /= 100;
			iFoodValue = std::max(1, iFoodValue);
		}
	}
	if (AI_isEmphasizeYield(YIELD_FOOD))
	{
		if (!isFoodProduction())
		{
			iFoodValue *= 130;
			iFoodValue /= 100;
			iSlaveryValue *= 130;
			iSlaveryValue /= 100;
		}
	}

	if (AI_isEmphasizeYield(YIELD_COMMERCE))
	{
		iCommerceValue *= 130;
		iCommerceValue /= 100;
		if (!AI_isEmphasizeYield(YIELD_PRODUCTION) && iProductionValue > 0)
		{
			iProductionValue *= 75;
			iProductionValue /= 100;
			iProductionValue = std::max(1,iProductionValue);
		}
		if (!AI_isEmphasizeYield(YIELD_FOOD) && iFoodValue > 0)
		{
			//Don't supress twice.
			if (!AI_isEmphasizeYield(YIELD_PRODUCTION))
			{
				iFoodValue *= 80;
				iFoodValue /= 100;
				iFoodValue = std::max(1, iFoodValue);
			}
		}
	}

	if( iProductionValue > 0 )
	{
		if (isFoodProduction())
		{
			iProductionValue *= 100 + (bWorkerOptimization ? 0 : AI_specialYieldMultiplier(YIELD_PRODUCTION));
			iProductionValue /= 100;
		}
		else
		{
			iProductionValue *= iBaseProductionModifier;
			iProductionValue /= (iBaseProductionModifier + iExtraProductionModifier);

			iProductionValue += iSlaveryValue;
			iProductionValue *= (100 + (bWorkerOptimization ? 0 : AI_specialYieldMultiplier(YIELD_PRODUCTION)));

			iProductionValue /= GET_PLAYER(getOwnerINLINE()).AI_averageYieldMultiplier(YIELD_PRODUCTION);
		}

		iValue += std::max(1,iProductionValue);
	}

	if( iCommerceValue > 0 )
	{
		iCommerceValue *= (100 + (bWorkerOptimization ? 0 : AI_specialYieldMultiplier(YIELD_COMMERCE)));
		iCommerceValue /= GET_PLAYER(getOwnerINLINE()).AI_averageYieldMultiplier(YIELD_COMMERCE);
		iValue += std::max(1, iCommerceValue);
	}
//
	if( iFoodValue > 0 )
	{
		iFoodValue *= 100;
		iFoodValue /= GET_PLAYER(getOwnerINLINE()).AI_averageYieldMultiplier(YIELD_FOOD);
		iValue += std::max(1, iFoodValue);
	}
/************************************************************************************************/
/* UNOFFICIAL_PATCH						END												  */
/************************************************************************************************/

	return iValue;
}


int CvCityAI::AI_plotValue(CvPlot* pPlot, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood, bool bIgnoreGrowth, bool bIgnoreStarvation)
{
	PROFILE_FUNC();

	short aiYields[NUM_YIELD_TYPES];
	ImprovementTypes eCurrentImprovement;
	ImprovementTypes eFinalImprovement;
	int iYieldDiff;
	int iValue;
	int iI;
	//int iTotalDiff;

	iValue = 0;
	//iTotalDiff = 0;

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aiYields[iI] = pPlot->getYield((YieldTypes)iI);
	}

	eCurrentImprovement = pPlot->getImprovementType();
	eFinalImprovement = NO_IMPROVEMENT;

	if (eCurrentImprovement != NO_IMPROVEMENT)
	{
/************************************************************************************************/
/* Afforess					  Start		 05/23/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
/*
		eFinalImprovement = finalImprovementUpgrade(eCurrentImprovement);
*/
		eFinalImprovement = GET_TEAM(getTeam()).finalImprovementUpgrade(eCurrentImprovement);
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
	}


	int iYieldValue = (AI_yieldValue(aiYields, NULL, bAvoidGrowth, bRemove, bIgnoreFood, bIgnoreGrowth, bIgnoreStarvation) * 100);

	if (eFinalImprovement != NO_IMPROVEMENT)
	{
		for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
		{
			iYieldDiff = (pPlot->calculateImprovementYieldChange(eFinalImprovement, ((YieldTypes)iI), getOwnerINLINE()) - pPlot->calculateImprovementYieldChange(eCurrentImprovement, ((YieldTypes)iI), getOwnerINLINE()));
			aiYields[iI] += iYieldDiff;
		}
		int iFinalYieldValue = (AI_yieldValue(aiYields, NULL, bAvoidGrowth, bRemove, bIgnoreFood, bIgnoreGrowth, bIgnoreStarvation) * 100);

		if (iFinalYieldValue > iYieldValue)
		{
			iYieldValue = (40 * iYieldValue + 60 * iFinalYieldValue) / 100;
		}
		else
		{
			iYieldValue = (60 * iYieldValue + 40 * iFinalYieldValue) / 100;
		}
	}
	// unless we are emph food (and also food not production)
	if (!(AI_isEmphasizeYield(YIELD_FOOD) && !isFoodProduction()))
		// if this plot is super bad (less than 2 food and less than combined 2 prod/commerce
		if (!AI_potentialPlot(aiYields))
			// undervalue it even more!
			iYieldValue /= 16;
	iValue += iYieldValue;

	if (eCurrentImprovement != NO_IMPROVEMENT)
	{
		if (pPlot->getBonusType(getTeam()) == NO_BONUS) // XXX double-check CvGame::doFeature that the checks are the same...
		{
			for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
			{
				if (GET_TEAM(getTeam()).isHasTech((TechTypes)(GC.getBonusInfo((BonusTypes) iI).getTechReveal())))
				{
					if (GC.getImprovementInfo(eCurrentImprovement).getImprovementBonusDiscoverRand(iI) > 0)
					{
						iValue += 35;
					}
				}
			}
		}
	}

	if ((eCurrentImprovement != NO_IMPROVEMENT) && (GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementUpgrade() != NO_IMPROVEMENT))
	{
		iValue += 200;
		iValue -= pPlot->getUpgradeTimeLeft(eCurrentImprovement, NO_PLAYER);
	}

	return iValue;
}


int CvCityAI::AI_experienceWeight()
{
	//return ((getProductionExperience() + getDomainFreeExperience(DOMAIN_SEA)) * 2);
	// <f1rpo> From K-Mod; the -2 is from AdvCiv (advc.017). Replacing the above.
	return 2 * getProductionExperience() + getDomainFreeExperience(DOMAIN_LAND)
			+ getDomainFreeExperience(DOMAIN_SEA) - 2; // </f1rpo>
}


int CvCityAI::AI_buildUnitProb(/* f1rpo: */ bool bDraft)
{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  05/29/10								jdog5000	  */
/*																							  */
/* City AI, Barbarian AI																		*/
/************************************************************************************************/
	scaled r = per100( // f1rpo: Was int iProb. Use ScaledNum to reduce rounding artifacts.
			GC.getLeaderHeadInfo(getPersonalityType()).getBuildUnitProb()/* + AI_experienceWeight()*/);
	// <f1rpo> Count XP weight only half when drafting (advc.017)
	{
		int iXPWeight = AI_experienceWeight();
		if (bDraft)
			iXPWeight /= 2;
		r += per100(iXPWeight);
	}
	bool bGreatlyReduced = false;
	CvPlayerAI const& kOwner = GET_PLAYER(getOwner());
	// </f1rpo>
	if (!isBarbarian() && kOwner.AI_isFinancialTrouble())
	{
		r /= 2;
		bGreatlyReduced = true; // f1rpo
	}
	/*else if( GET_TEAM(getTeam()).getHasMetCivCount(false) == 0 )
		r /= 2;*/
	// <f1rpo> Replacing the above. The ECONOMY_FOCUS check is from K-Mod.
	else if (kOwner.AI_isDoStrategy(AI_STRATEGY_ECONOMY_FOCUS)
		|| kOwner.getNumCities() <= 1)
	{
		r /= 2;
		bGreatlyReduced = true;
	}
	//else // </f1rpo>
	// more units from cities with military production bonuses
	r += scaled::min(fixp(0.15), per100(getMilitaryProductionModifier()) / 4);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
	// <f1rpo> From K-Mod; GET_BETTER_UNITS guard from AdvCiv (advc.017).
	CvGame const& kGame = GC.getGameINLINE();
	if (kOwner.AI_isDoStrategy(AI_STRATEGY_GET_BETTER_UNITS))
	{
		int const iEraDiff = kGame.getCurrentEra() - kOwner.getCurrentEra();
		if (iEraDiff > 0)
			r *= per100(std::max(40, 100 - 20 * iEraDiff));
	} // <f1rpo>
/************************************************************************************************/
/* REVOLUTION_MOD						 11/08/08								jdog5000	  */
/*																							  */
/*																							  */
/************************************************************************************************/
	if(kOwner.isRebel())
		r *= fixp(4/3.);
/************************************************************************************************/
/* REVOLUTION_MOD						  END												  */
/************************************************************************************************/
/************************************************************************************************/
/* Afforess					  Start		 10/29/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
	if (AI_isMilitaryProductionCity())
		r *= fixp(4/3.);
	else if (kOwner.AI_getMilitaryProductionCityCount() > 0)
		r *= fixp(3/4.);
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
	if (isBarbarian() && kGame.isOption(GAMEOPTION_RAGING_BARBARIANS))
	{
		//iProb = 95; // Increased unit building for barbarians when raging is on. dbkblk, 2015-09
		r *= fixp(1.6); // f1rpo: ^That seems very crude. NB: Barbarian BuildUnitProb is 50 in XML.
	}
	/*	<f1rpo> Throttle unit production when already far more powerful
		than potential enemies (advc.017) */
	if (!isBarbarian())
	{
		int const iCities = kOwner.getNumCities();
		if (iCities > 1)
		{
			CvTeamAI const& kOurTeam = GET_TEAM(getTeam());
			int iHighestRivalPow = 1;
			for (int i = 0; i < MAX_CIV_TEAMS; i++)
			{
				if (i == kOurTeam.getID())
					continue;
				CvTeamAI const& kRival = GET_TEAM((TeamTypes)i);
				if (!kRival.isAlive() || kRival.isAVassal() || !kOurTeam.isHasMet(kRival.getID()))
					continue;
				if (kOurTeam.AI_getWarPlan(kRival.getID()) != NO_WARPLAN
					|| !kOurTeam.AI_isAvoidWar(kRival.getID(), true)
					|| !kRival.AI_isAvoidWar(kOurTeam.getID()))
				{
					iHighestRivalPow = std::max(kRival.getPower(true), iHighestRivalPow);
				}
			}
			scaled rPowRatio(kOurTeam.getPower(false), iHighestRivalPow);
			if (rPowRatio > 1)
			{
				scaled rAdvantage = rPowRatio - 1;
				if(rAdvantage >= fixp(1.5))
				{
					//r /= 4;
					// Decrease to less than one quarter (as requested by Inthegarev)
					r /= scaled(16,9) * rAdvantage * rAdvantage;
					bGreatlyReduced = true;
				}
				else r *= 1 - rAdvantage / 2;
			}
		}
		/*  Can't afford to specialize one city entirely on military production
			until we've expanded a bit */
		r.decreaseTo(fixp(0.2) * (1 + iCities));
		/*  Don't get too careless in the early game when most cities have
			negative AI_experienceWeight */
		if (!bGreatlyReduced)
			r.increaseTo(per100(25 - 2 * iCities));
	} // </f1rpo>

	r.decreaseTo(1); // f1rpo (from K-Mod)
	return r.getPercent(); // f1rpo: Caller expects percent scale
}



// Improved worker AI provided by Blake - thank you!
void CvCityAI::AI_bestPlotBuild(CvPlot* pPlot, int* piBestValue, BuildTypes* peBestBuild, int iFoodPriority, int iProductionPriority, int iCommercePriority, bool bChop, int iHappyAdjust, int iHealthAdjust, int iFoodChange)
{
	PROFILE_FUNC();

	CvCity* pCity = NULL;
	int aiFinalYields[NUM_YIELD_TYPES];
	int aiDiffYields[NUM_YIELD_TYPES];
	int aiBestDiffYields[NUM_YIELD_TYPES];
	ImprovementTypes eImprovement;
	ImprovementTypes eFinalImprovement;
	BuildTypes eBuild;
	BuildTypes eBestBuild;
	BuildTypes eBestTempBuild;
	BonusTypes eBonus;
	BonusTypes eNonObsoleteBonus;

	bool bEmphasizeIrrigation = false;
	bool bIgnoreFeature;
	bool bHasBonusImprovement;
	bool bValid;
	int iValue;
	int iBestValue;
	int iBestTempBuildValue;
	int iI, iJ;

	if (piBestValue != NULL)
	{
		*piBestValue = 0;
	}
	if (peBestBuild != NULL)
	{
		*peBestBuild = NO_BUILD;
	}

	if (pPlot->getWorkingCity() != this)
	{
		return;
	}

	//When improving new plots only, count emphasis twice
	//helps to avoid too much tearing up of old improvements.
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  05/06/09								jdog5000	  */
/*																							  */
/* Worker AI																					*/
/************************************************************************************************/
	// AI no longer uses emphasis really, except for short term boosts to commerce.
	// Inappropriate to base improvements on short term goals.
	if( isHuman() )
	{
		if (pPlot->getImprovementType() == NO_IMPROVEMENT)
		{
			if (AI_isEmphasizeYield(YIELD_FOOD))
			{
				iFoodPriority *= 130;
				iFoodPriority /= 100;
			}
			if (AI_isEmphasizeYield(YIELD_PRODUCTION))
			{
				iProductionPriority *= 180;
				iProductionPriority /= 100;
			}
			if (AI_isEmphasizeYield(YIELD_COMMERCE))
			{
				iCommercePriority *= 180;
				iCommercePriority /= 100;
			}
		}
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

	FAssertMsg(pPlot->getOwnerINLINE() == getOwnerINLINE(), "pPlot must be owned by this city's owner");

	eBonus = pPlot->getBonusType(getTeam());
	eNonObsoleteBonus = pPlot->getNonObsoleteBonusType(getTeam());

	bHasBonusImprovement = false;

	if (eNonObsoleteBonus != NO_BONUS)
	{
		if (pPlot->getImprovementType() != NO_IMPROVEMENT)
		{
/*************************************************************************************************/
/**	Forts Connect Resources									02/12/10				Xienwolf	**/
/*************************************************************************************************/
/* original code
			if (GC.getImprovementInfo(pPlot->getImprovementType()).isImprovementBonusTrade(eNonObsoleteBonus))
*/
			if (GC.getImprovementInfo(pPlot->getImprovementType()).isImprovementBonusTrade(eNonObsoleteBonus) || GC.getImprovementInfo(pPlot->getImprovementType()).isActsAsCity())
/*************************************************************************************************/
/**	Forts Connect Resources					END													**/
/*************************************************************************************************/
			{
				bHasBonusImprovement = true;
			}
/********************************************************************************/
/* 	Bonus Improvement: Wait for Upgrade							Fuyu			*/
/********************************************************************************/
			//Fuyu: patience. We can wait 10 turns for upgrade
			else if (GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementUpgrade() != NO_IMPROVEMENT)
			{
				if (GC.getImprovementInfo((ImprovementTypes)(GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementUpgrade())).isImprovementBonusTrade(eNonObsoleteBonus)
					|| GC.getImprovementInfo((ImprovementTypes)(GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementUpgrade())).isActsAsCity())
				{
					if (pPlot->getUpgradeTimeLeft(pPlot->getImprovementType(), getOwner()) <= 1 + ((9 * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getImprovementPercent() * GC.getEraInfo(GC.getGameINLINE().getStartEra()).getImprovementPercent())/10000))
					{
						bHasBonusImprovement = true;
					}
				}
			}
/********************************************************************************/
/* 	Bonus Improvement: Wait for Upgrade							END				*/
/********************************************************************************/
		}
	}

	for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
	{
		aiBestDiffYields[iJ] = 0;
	}

	BuildTypes eForcedBuild = NO_BUILD;

	{	//If a worker is already building a build, force that Build.
		CLLNode<IDInfo>* pUnitNode;
		CvUnit* pLoopUnit;

		pUnitNode = pPlot->headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = pPlot->nextUnitNode(pUnitNode);

			if (pLoopUnit->getBuildType() != NO_BUILD)
			{
				if (GC.getBuildInfo(pLoopUnit->getBuildType()).getImprovement() != NO_IMPROVEMENT)
				{
					eForcedBuild = pLoopUnit->getBuildType();
					break;
				}
			}
		}
	}

	iBestValue = 0;
	eBestBuild = NO_BUILD;

	int iClearFeatureValue = 0;

	if (pPlot->getFeatureType() != NO_FEATURE)
	{
		iClearFeatureValue = AI_clearFeatureValue(getCityPlotIndex(pPlot));
	}

	if (!bHasBonusImprovement)
	{
		bEmphasizeIrrigation = false;

		CvPlot* pAdjacentPlot;
		CvPlot* pAdjacentPlot2;
		BonusTypes eTempBonus;

		//It looks unwieldly but the code has to be rigid to avoid "worker ADD"
		//where they keep connecting then disconnecting a crops resource or building
		//multiple farms to connect a single crop resource.
		//isFreshWater is used to ensure invalid plots are pruned early, the inner loop
		//really doesn't run that often.

		//using logic along the lines of "Will irrigating me make crops wet"
		//wont really work... it does have to "am i the tile the crops want to be irrigated"

		//I optimized through the use of "isIrrigated" which is just checking a bool...
		//once everything is nicely irrigated, this code should be really fast...
		if ((pPlot->isIrrigated()) || (pPlot->isFreshWater() && pPlot->canHavePotentialIrrigation()))
		{
			for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
			{
				pAdjacentPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

				if ((pAdjacentPlot != NULL) && (pAdjacentPlot->getOwner() == getOwner()) && (pAdjacentPlot->isCityRadius()))
				{
					if (!pAdjacentPlot->isFreshWater())
					{
						//check for a city? cities can conduct irrigation and that effect is quite
						//useful... so I think irrigate cities.
						if (pAdjacentPlot->isPotentialIrrigation())
						{
							CvPlot* eBestIrrigationPlot = NULL;


							for (iJ = 0; iJ < NUM_DIRECTION_TYPES; iJ++)
							{
								pAdjacentPlot2 = plotDirection(pAdjacentPlot->getX_INLINE(), pAdjacentPlot->getY_INLINE(), ((DirectionTypes)iJ));
								if ((pAdjacentPlot2 != NULL) && (pAdjacentPlot2->getOwner() == getOwner()))
								{
									eTempBonus = pAdjacentPlot2->getNonObsoleteBonusType(getTeam());
									if (pAdjacentPlot->isIrrigated())
									{
										//the irrigation has to be coming from somewhere
										if (pAdjacentPlot2->isIrrigated())
										{
											//if we find a tile which is already carrying irrigation
											//then lets not replace that one...
											eBestIrrigationPlot = pAdjacentPlot2;

											if ((pAdjacentPlot2->isCity()) || (eTempBonus != NO_BONUS) || (!pAdjacentPlot2->isCityRadius()))
											{
												if (pAdjacentPlot2->isFreshWater())
												{
													//these are all ideal for irrigation chains so stop looking.
													break;
												}
											}
										}

									}
									else
									{
										if (pAdjacentPlot2->getNonObsoleteBonusType(getTeam()) == NO_BONUS)
										{
											if (pAdjacentPlot2->canHavePotentialIrrigation() && pAdjacentPlot2->isIrrigationAvailable())
											{
												//could use more sophisticated logic
												//however this would rely on things like smart irrigation chaining
												//of out-of-city plots
												eBestIrrigationPlot = pAdjacentPlot2;
												break;
											}
										}
									}
								}
							}

							if (pPlot == eBestIrrigationPlot)
							{
								bEmphasizeIrrigation = true;
								break;
							}
						}
					}
				}
			}
		}
	}

		for (iI = 0; iI < GC.getNumImprovementInfos(); iI++)
		{
			eImprovement = ((ImprovementTypes)iI);

			iBestTempBuildValue = 0;
			eBestTempBuild = NO_BUILD;

			bIgnoreFeature = false;
			bValid = false;

			if (eImprovement == pPlot->getImprovementType())
			{
				bValid = true;
			}
			//	Don't count forts - they have their own separate decision criteria
			else if (!GC.getImprovementInfo(eImprovement).isActsAsCity())
			{
				if (eForcedBuild != NO_BUILD)
				{
					if (GC.getBuildInfo(eForcedBuild).getImprovement() == eImprovement)
					{
						eBestTempBuild = eForcedBuild;
					}
				}
				else
				{
					for (iJ = 0; iJ < GC.getNumBuildInfos(); iJ++)
					{
						eBuild = ((BuildTypes)iJ);

						if (GC.getBuildInfo(eBuild).getImprovement() == eImprovement)
						{
							//Afforess check automation rules for humans
							bool bCanUnitBuild = true;
							if (isHuman())
							{
								if (!GET_PLAYER(getOwnerINLINE()).isAutomatedCanBuild(eBuild))
								{
									bCanUnitBuild = false;
								}
								else if (!isAutomatedCanBuild(eBuild))
								{
									bCanUnitBuild = false;
								}
							}

							if (bCanUnitBuild && GET_PLAYER(getOwnerINLINE()).canBuild(pPlot, eBuild, false, false, false))
							{
								iValue = 10000;
								iValue /= (GC.getBuildInfo(eBuild).getTime() + 1);

								// XXX feature production???

								if (iValue > iBestTempBuildValue)
								{
									iBestTempBuildValue = iValue;
									eBestTempBuild = eBuild;
								}
							}
						}
					}
				}

				if (eBestTempBuild != NO_BUILD)
				{
					bValid = true;

					if (pPlot->getFeatureType() != NO_FEATURE)
					{
						if (GC.getBuildInfo(eBestTempBuild).isFeatureRemove(pPlot->getFeatureType()))
						{
							bIgnoreFeature = true;

							if (GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange(YIELD_PRODUCTION) > 0)
							{
								if (eNonObsoleteBonus == NO_BONUS)
								{
									if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_LEAVE_FORESTS))
									{
										bValid = false;
									}
									else if (healthRate() < 0 && GC.getFeatureInfo(pPlot->getFeatureType()).getHealthPercent() > 0)
									{
										bValid = false;
									}
									else if (GET_PLAYER(getOwnerINLINE()).getFeatureHappiness(pPlot->getFeatureType()) > 0)
									{
										bValid = false;
									}
								}
							}
						}
					}
				}
			}

			if (bValid)
			{
/************************************************************************************************/
/* Afforess					  Start		 05/29/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
/*
				eFinalImprovement = finalImprovementUpgrade(eImprovement);
*/
				eFinalImprovement = GET_TEAM(getTeam()).finalImprovementUpgrade(eImprovement);
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
				if (eFinalImprovement == NO_IMPROVEMENT)
				{
					eFinalImprovement = eImprovement;
				}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD																			*/
/*																								*/
/* City AI, FinalImprovement part moved here from inside yield loop (Fuyu)						*/
/************************************************************************************************/
				ImprovementTypes eCurImprovement = pPlot->getImprovementType();
				ImprovementTypes eCurFinalImprovement = NO_IMPROVEMENT;
				if( eCurImprovement != NO_IMPROVEMENT )
				{
/************************************************************************************************/
/* Afforess					  Start		 05/29/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
/*
					eCurFinalImprovement = finalImprovementUpgrade(eCurImprovement);
*/
					eCurFinalImprovement = GET_TEAM(getTeam()).finalImprovementUpgrade(eCurImprovement);
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/

					if (eCurFinalImprovement == NO_IMPROVEMENT)
					{
						eCurFinalImprovement = eCurImprovement;
					}
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

				iValue = 0;

				if (eBonus != NO_BONUS)
				{
/*************************************************************************************************/
/**	Forts Connect Resources but should never be built					30.07.2010		Fuyu	**/
/*************************************************************************************************/
					if (eNonObsoleteBonus != NO_BONUS)
					{
						if (!bHasBonusImprovement)
						{
							if (GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
							{
								if (!(GC.getImprovementInfo(eFinalImprovement).isImprovementBonusTrade(eNonObsoleteBonus)))
								{
									//reduced value for temporary solutions
									if (!(GET_TEAM(getTeam()).isHasTech((TechTypes)(GC.getBonusInfo(eNonObsoleteBonus).getTechCityTrade()))))
									{
										iValue++;
									}
									else
									{
										iValue += (GET_PLAYER(getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus) * 10);
										iValue += 150;
									}
								}
								else
								{
									//full value for permanent solutions
									iValue += (GET_PLAYER(getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus) * 10);
									iValue += 200;
								}

								if (eBestBuild != NO_BUILD)
								{
									if ((GC.getBuildInfo(eBestBuild).getImprovement() == NO_IMPROVEMENT) || !(GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(eBestBuild).getImprovement()).isImprovementBonusTrade(eNonObsoleteBonus)))
									{
										//Always prefer improvements which connect bonuses.
										eBestBuild = NO_BUILD;
										iBestValue = 0;
									}
								}
							}
							else
							{
								if (eBestBuild != NO_BUILD)
								{
									if ((GC.getBuildInfo(eBestBuild).getImprovement() != NO_IMPROVEMENT) && (GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(eBestBuild).getImprovement()).isImprovementBonusTrade(eNonObsoleteBonus)))
									{
										iValue -= 1000;
									}
								}
								if (GC.getImprovementInfo(eFinalImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
								{
									//very small incentive to build improvements that could later connect the bonus resource
									iValue++;
								}
							}
						}
						else if (!(GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus)))
						{
							iValue -= 1000;
						}
/*************************************************************************************************/
/**	Forts Connect Resources but should never be built					END						**/
/*************************************************************************************************/
					}
				}
				else
				{
					for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
					{
						if (GC.getImprovementInfo(eFinalImprovement).getImprovementBonusDiscoverRand(iJ) > 0)
						{
							iValue++;
						}
					}
				}

				if (iValue >= 0)
				{

					iValue *= 2;
					for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  10/06/09								jdog5000	  */
/*																							  */
/* City AI																					  */
/************************************************************************************************/
/* original BTS code
						aiFinalYields[iJ] = 2*(pPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature));
						aiFinalYields[iJ] += (pPlot->calculateImprovementYieldChange(eFinalImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
						aiFinalYields[iJ] += (pPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false));
						if (bIgnoreFeature && pPlot->getFeatureType() != NO_FEATURE)
						{
							aiFinalYields[iJ] -= 2 * GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange((YieldTypes)iJ);
						}
						aiDiffYields[iJ] = (aiFinalYields[iJ] - (2 * pPlot->getYield(((YieldTypes)iJ))));
*/
						//
						aiFinalYields[iJ] = 2*(pPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), bIgnoreFeature));
						aiFinalYields[iJ] += (pPlot->calculateImprovementYieldChange(eFinalImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false, true));
						aiFinalYields[iJ] += (pPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false, true));

						//	Koshling - removed the following original code because it is double-counting since the
						//	call to calculateNatureYield() above ALREADY factors out a removed improvement
						//if (bIgnoreFeature && pPlot->getFeatureType() != NO_FEATURE)
						//{
						//	aiFinalYields[iJ] -= 2 * GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange((YieldTypes)iJ);
						//}

						int iCurYield = 2*(pPlot->calculateNatureYield(((YieldTypes)iJ), getTeam(), false));

/********************************************************************************/
/*	Better Evaluation							19.03.2010		Fuyu			*/
/********************************************************************************/
/* Fuyu: moved FinalImprovement code up to before the loop
						ImprovementTypes eCurImprovement = pPlot->getImprovementType();
*/
						if( eCurImprovement != NO_IMPROVEMENT )
						{
/* moved
							ImprovementTypes eCurFinalImprovement = finalImprovementUpgrade(eCurImprovement);
							if (eCurFinalImprovement == NO_IMPROVEMENT)
							{
								eCurFinalImprovement = eCurImprovement;
							}
*/

							iCurYield += (pPlot->calculateImprovementYieldChange(eCurFinalImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false, true));
							iCurYield += (pPlot->calculateImprovementYieldChange(eCurImprovement, ((YieldTypes)iJ), getOwnerINLINE(), false, true));
						}
/********************************************************************************/
/*	BE	END																		*/
/********************************************************************************/

						aiDiffYields[iJ] = (aiFinalYields[iJ] - iCurYield);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
					}
/************************************************************************************************/
/* Afforess					  Start		 08/29/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
					int iHealth = AI_calculateActualImprovementHealth(eFinalImprovement) - AI_calculateActualImprovementHealth(eCurFinalImprovement);
					int iBaseHealthLevel = goodHealth() - badHealth() + getEspionageHealthCounter();
					//int iBaseFoodDifference = getYieldRate(YIELD_FOOD) - getPopulation()*GC.getFOOD_CONSUMPTION_PER_POPULATION() - std::max(0,-iBaseHealthLevel);
					int iBadHealthFromImprovement = std::max(0,(-iHealth));
					int iUnhealthyPopulationFromImprovement = std::min(0,(-iBaseHealthLevel)) + iBadHealthFromImprovement;
					//int iGoodHealthFromImprovement = std::max(0, iHealth);
					int iHealthyPopulationFromImprovement = 0;
					if (iBaseHealthLevel < 0 && iHealth > 0)
					{
						iHealthyPopulationFromImprovement = std::min(iHealth, -iBaseHealthLevel);
					}
					aiDiffYields[YIELD_FOOD] -= iUnhealthyPopulationFromImprovement;
					aiDiffYields[YIELD_FOOD] += iHealthyPopulationFromImprovement;
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/

					iValue += (aiDiffYields[YIELD_FOOD] * ((100 * iFoodPriority) / 100));
					iValue += (aiDiffYields[YIELD_PRODUCTION] * ((60 * iProductionPriority) / 100));
					iValue += (aiDiffYields[YIELD_COMMERCE] * ((40 * iCommercePriority) / 100));

					iValue /= 2;

					for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
						aiFinalYields[iJ] /= 2;
						aiDiffYields[iJ] /= 2;
					}

					if (iValue > 0)
					{
						// this is mainly to make it improve better tiles first
						//flood plain > grassland > plain > tundra
						iValue += (aiFinalYields[YIELD_FOOD] * 10);
						iValue += (aiFinalYields[YIELD_PRODUCTION] * 6);
						iValue += (aiFinalYields[YIELD_COMMERCE] * 4);

						if (aiFinalYields[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION())
						{
							//this is a food yielding tile
							if (iFoodPriority > 100)
							{
								iValue *= 100 + iFoodPriority;
								iValue /= 200;
							}
							if (iFoodChange > 0)
							{
								iValue += (10 * (1 + aiDiffYields[YIELD_FOOD]) * (1 + aiFinalYields[YIELD_FOOD] - GC.getFOOD_CONSUMPTION_PER_POPULATION()) * iFoodChange * iFoodPriority) / 100;
							}
							if (iCommercePriority > 100)
							{
								iValue *= 100 + (((iCommercePriority - 100) * aiDiffYields[YIELD_COMMERCE]) / 2);
								iValue /= 100;
							}
						}
						else if (aiFinalYields[YIELD_FOOD] < GC.getFOOD_CONSUMPTION_PER_POPULATION())
						{
							if ((aiDiffYields[YIELD_PRODUCTION] > 0) && (aiFinalYields[YIELD_FOOD]+aiFinalYields[YIELD_PRODUCTION] > 3))
							{
								if (iFoodPriority < 100 || GET_PLAYER(getOwnerINLINE()).getCurrentEra() < 2)
								{
									//value booster for mines on hills
									iValue *= (100 + 25 * aiDiffYields[YIELD_PRODUCTION]);
									iValue /= 100;
								}
							}
							if (iFoodChange < 0)
							{
								iValue *= 4 - iFoodChange;
								iValue /= 3 + aiFinalYields[YIELD_FOOD];
							}
						}

						if ((iFoodPriority < iProductionPriority) && (iProductionPriority > 100))
						{
							iValue *= (200 + ((iProductionPriority - 100)*aiFinalYields[YIELD_PRODUCTION]));
							iValue /= 200;
						}
						if (eBonus == NO_BONUS)
						{
							if (iFoodChange > 0)
							{
								//We want more food.
								iValue *= 2 + std::max(0, aiDiffYields[YIELD_FOOD]);
								iValue /= 2 * (1 + std::max(0, -aiDiffYields[YIELD_FOOD]));
							}
//							else if (iFoodChange < 0)
//							{
//								//We want to soak up food.
//								iValue *= 8;
//								iValue /= 8 + std::max(0, aiDiffYields[YIELD_FOOD]);
//							}
						}
					}


					if (bEmphasizeIrrigation && GC.getImprovementInfo(eFinalImprovement).isCarriesIrrigation())
					{
						iValue += 500;
					}

					if (getImprovementFreeSpecialists(eFinalImprovement) > 0)
					{
						iValue += 2000;
					}
/********************************************************************************/
/*	Better Evaluation							19.03.2010		Fuyu			*/
/********************************************************************************/
					//int iNewHappiness = GC.getImprovementInfo(eFinalImprovement).getHappiness();
					//int iCurHappiness = GC.getImprovementInfo(eCurFinalImprovement).getHappiness();
					//int iHappinessDiff = iNewHappiness - iCurHappiness;

					int iHappinessDiff = GC.getImprovementInfo(eFinalImprovement).getHappiness() - ((eCurFinalImprovement != NO_IMPROVEMENT)? GC.getImprovementInfo(eCurFinalImprovement).getHappiness() : 0);
					if ((iHappinessDiff != 0) && !(GET_PLAYER(getOwnerINLINE()).getAdvancedStartPoints() >= 0))
					{
						bool bIsNegative = false;
						if (iHappinessDiff < 0)
						{
							bIsNegative = true;
							iHappinessDiff = -iHappinessDiff;
						}

						int iHappyLevel = iHappyAdjust + (happyLevel() - unhappyLevel(0)) + getEspionageHappinessCounter();
						int iHealthLevel = std::max(0, iHealthAdjust) + (goodHealth() - badHealth(false, 0)) + getEspionageHealthCounter();

						for (iJ = 0; iJ < iHappinessDiff; iJ++)
						{
							int iHappyValue = 0;

							if (bIsNegative)
							{
								iHappyLevel--;
							}

							if (iHappyLevel <= 0)
							{
								iHappyValue += 400;
							}
							bool bCanGrow = true;// (getYieldRate(YIELD_FOOD) > foodConsumption());

							if (iHappyLevel <= iHealthLevel)
							{
								iHappyValue += 200 * std::max(0, (bCanGrow ? std::min(6, 2 + iHealthLevel - iHappyLevel) : 0) - iHappyLevel);
							}
							else
							{
								iHappyValue += 50 * std::max(0, (bCanGrow ? 4 : 0) - iHappyLevel);

							}
							if (!pPlot->isBeingWorked() || bIsNegative)
							{
								iHappyValue *= 4;
								iHappyValue /= 3;
							}

							if (iHappyValue == 0 && bIsNegative && iHappyLevel < 8)
							{
								//an unhappy improvement always has negative value, even if the city is really happy right now
								iHappyValue = 10;
							}

							//iHappyValue += std::max(0, (pPlot->getCityRadiusCount() - 1)) * ((iHappyValue > 0) ? iHappyLevel / 2 : 200);
							iHappyValue += (pPlot->getPlayerCityRadiusCount(getOwnerINLINE()) - 1) * ((iHappyValue > 0) ? 200 : 50);

							if (!bIsNegative)
							{
								iHappyLevel++;
								iValue += iHappyValue;
							}
							else
							{
								iValue -= iHappyValue;
							}
						}
					}
/********************************************************************************/
/*	BE	END																		*/
/********************************************************************************/

/************************************************************************************************/
/* Afforess					  Start		 05/29/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
					int iHealthPercent = GC.getImprovementInfo(eFinalImprovement).getHealthPercent();
					int iBadHealth = abs(getImprovementBadHealth());
					int iGoodHealth = getImprovementGoodHealth();
					//Bad Health AND will push over the edge and give more unhealthiness
					if ((iHealthPercent < 0 && (iBadHealth - iHealthPercent % 100 < iBadHealth % 100)) ||
					//OR Good Health AND will give us more healthiness
					(iHealthPercent > 0 && (iGoodHealth + iHealthPercent % 100 > iGoodHealth % 100)))
					{
						int iHealth = iHealthPercent > 0 ? std::max(1, iHealthPercent % 100) : std::min(-1, iHealthPercent % 100);
						int iHealthLevel = iHealthAdjust + (goodHealth() - badHealth(false, 0));
						if (eImprovement == pPlot->getImprovementType())
						{
							iHealthLevel -= iHealth;
						}

						int iHealthValue = 0;
						bool bCanGrow = (getYieldRate(YIELD_FOOD) > foodConsumption());
						if (iHealthLevel < 0)
						{
							iHealthValue -= 10;
						}
						if (!bCanGrow && iHealthLevel < 0)
						{
							iHealthValue -= 40;
						}
						if (iHealthLevel > 0)
						{
							iHealthValue += 10;
						}

						if (!pPlot->isBeingWorked())
						{
							iHealthValue *= 4;
							iHealthValue /= 3;
						}
						iValue += iHealthValue * 3;
					}
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
					if (!isHuman())
					{
						iValue *= std::max(0, (GC.getLeaderHeadInfo(getPersonalityType()).getImprovementWeightModifier(eFinalImprovement) + 200));
						iValue /= 200;
					}

					if (pPlot->getImprovementType() == NO_IMPROVEMENT)
					{
						if (pPlot->isBeingWorked())
						{
							iValue *= 5;
							iValue /= 4;
						}

						if (eBestTempBuild != NO_BUILD)
						{
							if (pPlot->getFeatureType() != NO_FEATURE)
							{
								if (GC.getBuildInfo(eBestTempBuild).isFeatureRemove(pPlot->getFeatureType()))
								{
									// BBAI TODO: extra boost in early game to chop rush
									iValue += pPlot->getFeatureProduction(eBestTempBuild, getTeam(), &pCity) * 2;
									FAssert(pCity == this);

									iValue += iClearFeatureValue;
								}
							}
						}
					}
					else
					{
						// cottage/villages (don't want to chop them up if turns have been invested)
						ImprovementTypes eImprovementDowngrade = (ImprovementTypes)GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementPillage();
						while (eImprovementDowngrade != NO_IMPROVEMENT)
						{
							CvImprovementInfo& kImprovementDowngrade = GC.getImprovementInfo(eImprovementDowngrade);
/********************************************************************************/
/*	Better Evaluation							19.03.2010		Fuyu			*/
/********************************************************************************/
//An attempt to make replacing hamlet+ less interesting
/* original code
							iValue -= kImprovementDowngrade.getUpgradeTime() * 8;
*/
							iValue -= kImprovementDowngrade.getUpgradeTime() * ((pPlot->isBeingWorked())? 20 : 10);
/********************************************************************************/
/*	BE	END																		*/
/********************************************************************************/
							eImprovementDowngrade = (ImprovementTypes)kImprovementDowngrade.getImprovementPillage();
						}
/************************************************************************************************/
/* Afforess					  Start		 05/23/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
/*
						if (GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementUpgrade() != NO_IMPROVEMENT)
*/
						if (GET_TEAM(getTeam()).getImprovementUpgrade(pPlot->getImprovementType()) != NO_IMPROVEMENT)
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
						{
							iValue -= (GC.getImprovementInfo(pPlot->getImprovementType()).getUpgradeTime() * 8 * (pPlot->getUpgradeProgressHundredths())) / std::max(1, 100*GC.getGameINLINE().getImprovementUpgradeTime(pPlot->getImprovementType()));
						}

						if (eNonObsoleteBonus == NO_BONUS)
						{
							if (isWorkingPlot(pPlot))
							{
								if (((iFoodPriority < 100) && (aiFinalYields[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION())) || (GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementPillage() != NO_IMPROVEMENT))
								{
									iValue -= 70;
									iValue *= 2;
									iValue /= 3;
								}
							}
						}

						if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_SAFE_AUTOMATION))
						{
							iValue /= 4;	//Greatly prefer builds which are legal.
						}
/************************************************************************************************/
/* Afforess					  Start		 07/27/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
						if (eFinalImprovement != NO_IMPROVEMENT && (GC.getImprovementInfo(eFinalImprovement).isActsAsCity()))
						{
							iValue /= 2;
						}
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
					}

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						eBestBuild = eBestTempBuild;

						for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
						{
							aiBestDiffYields[iJ] = aiDiffYields[iJ];
						}
					}
				}
			}
		}

	if (iClearFeatureValue > 0)
	{
		FAssert(pPlot->getFeatureType() != NO_FEATURE);

		{
			if ((GC.getFeatureInfo(pPlot->getFeatureType()).getHealthPercent() < 0) ||
				((GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange(YIELD_FOOD) + GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange(YIELD_PRODUCTION) + GC.getFeatureInfo(pPlot->getFeatureType()).getYieldChange(YIELD_COMMERCE)) < 0))
			{
				for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
				{
					eBuild = ((BuildTypes)iI);

					if (GC.getBuildInfo(eBuild).getImprovement() == NO_IMPROVEMENT)
					{
						if (GC.getBuildInfo(eBuild).isFeatureRemove(pPlot->getFeatureType()))
						{
							if (GET_PLAYER(getOwnerINLINE()).canBuild(pPlot, eBuild))
							{
								iValue = iClearFeatureValue;
								iValue += (pPlot->getFeatureProduction(eBuild, getTeam(), &pCity) * 10);

								iValue *= 400;
								iValue /= std::max(1, (GC.getBuildInfo(eBuild).getFeatureTime(pPlot->getFeatureType()) + 100));

								if ((iValue > iBestValue) || ((iValue > 0) && (eBestBuild == NO_BUILD)))
								{
									iBestValue = iValue;
									eBestBuild = eBuild;
								}
							}
						}
					}
				}
			}
		}
	}

	//Chop - maybe integrate this better with the other feature-clear code tho the logic
	//is kinda different
	if (bChop && (eBonus == NO_BONUS) && (pPlot->getFeatureType() != NO_FEATURE) &&
		(pPlot->getImprovementType() == NO_IMPROVEMENT) && !(GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_LEAVE_FORESTS)))
	{
		for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
		{
			eBuild = ((BuildTypes)iI);
			if (GC.getBuildInfo(eBuild).getImprovement() == NO_IMPROVEMENT)
			{
				if (GC.getBuildInfo(eBuild).isFeatureRemove(pPlot->getFeatureType()))
				{
					if (GET_PLAYER(getOwnerINLINE()).canBuild(pPlot, eBuild))
					{
						iValue = (pPlot->getFeatureProduction(eBuild, getTeam(), &pCity)) * 10;
						FAssert(pCity == this);

						if (iValue > 0)
						{
							iValue += iClearFeatureValue;

							if (iValue > 0)
							{
								if (GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_DAGGER))
								{
									iValue += 20;
									iValue *= 2;
								}
								iValue *= 500;
								iValue /= std::max(1, (GC.getBuildInfo(eBuild).getFeatureTime(pPlot->getFeatureType()) + 100));

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									eBestBuild = eBuild;
								}
							}
						}
					}
				}
			}
		}
	}


	for (iI = 0; iI < GC.getNumRouteInfos(); iI++)
	{
		RouteTypes eRoute = (RouteTypes)iI;
		RouteTypes eOldRoute = pPlot->getRouteType();

		if (eRoute != eOldRoute)
		{
			int iTempValue = 0;
			if (pPlot->getImprovementType() != NO_IMPROVEMENT)
			{
				if ((eOldRoute == NO_ROUTE) || (GC.getRouteInfo(eRoute).getValue() > GC.getRouteInfo(eOldRoute).getValue()))
				{
					iTempValue += ((GC.getImprovementInfo(pPlot->getImprovementType()).getRouteYieldChanges(eRoute, YIELD_FOOD)) * 100);
					iTempValue += ((GC.getImprovementInfo(pPlot->getImprovementType()).getRouteYieldChanges(eRoute, YIELD_PRODUCTION)) * 60);
					iTempValue += ((GC.getImprovementInfo(pPlot->getImprovementType()).getRouteYieldChanges(eRoute, YIELD_COMMERCE)) * 40);
				}

				if (pPlot->isBeingWorked())
				{
					iTempValue *= 2;
				}
				//road up bonuses if sort of bored.
				if ((eOldRoute == NO_ROUTE) && (eBonus != NO_BONUS))
				{
					iTempValue += (pPlot->isConnectedToCapital() ? 10 : 30);
				}
			}

			if (iTempValue > 0)
			{
				for (iJ = 0; iJ < GC.getNumBuildInfos(); iJ++)
				{
					eBuild = static_cast<BuildTypes>(iJ);

					if (GC.getBuildInfo(eBuild).getRoute() == eRoute && GET_PLAYER(getOwnerINLINE()).canBuild(pPlot, eBuild, false))
					{
						//the value multiplier is based on the default time...
						iValue = iTempValue * 5 * 300;
						iValue /= std::max(1, GC.getBuildInfo(eBuild).getTime());

						if (iValue > iBestValue || iValue > 0 && eBestBuild == NO_BUILD)
						{
							iBestValue = iValue;
							eBestBuild = eBuild;
						}
					}
				}
			}
		}
	}



	if (eBestBuild != NO_BUILD)
	{
		FAssertMsg(iBestValue > 0, "iBestValue is expected to be greater than 0");

		//Now modify the priority for this build.
		if (GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble())
		{
			if (GC.getBuildInfo(eBestBuild).getImprovement() != NO_IMPROVEMENT)
			{
				iBestValue += (iBestValue * std::max(0, aiBestDiffYields[YIELD_COMMERCE])) / 4;
				iBestValue = std::max(1, iBestValue);
			}

		}

		if (piBestValue != NULL)
		{
			*piBestValue = iBestValue;
		}
		if (peBestBuild != NULL)
		{
			if ( *peBestBuild != NO_BUILD )
			{
				if( gCityLogLevel >= 2 )
				{
					logBBAIForTeam(getTeam(), "	City %S, tile (%d,%d) best build changed from %S to %S", getName().GetCString(), pPlot->getX_INLINE(), pPlot->getY_INLINE(), GC.getBuildInfo(*peBestBuild).getDescription(), GC.getBuildInfo(eBestBuild).getDescription());
				}
			}

			*peBestBuild = eBestBuild;
		}
	}
}

int CvCityAI::AI_getHappyFromHurry(HurryTypes eHurry)
{
	return AI_getHappyFromHurry(hurryPopulation(eHurry));
}

int CvCityAI::AI_getHappyFromHurry(int iHurryPopulation)
{
	int iHappyDiff = iHurryPopulation - GC.getDefineINT("HURRY_POP_ANGER");
	if (iHappyDiff > 0)
	{
		if (getHurryAngerTimer() <= 1)
		{
			if (2 * angryPopulation(1) - healthRate(false, 1) > 1)
			{
				return iHappyDiff;
			}
		}
	}

	return 0;
}

int CvCityAI::AI_getHappyFromHurry(HurryTypes eHurry, UnitTypes eUnit, bool bIgnoreNew)
{
	return AI_getHappyFromHurry(getHurryPopulation(eHurry, getHurryCost(true, eUnit, bIgnoreNew)));
}

int CvCityAI::AI_getHappyFromHurry(HurryTypes eHurry, BuildingTypes eBuilding, bool bIgnoreNew)
{
	return AI_getHappyFromHurry(getHurryPopulation(eHurry, getHurryCost(true, eBuilding, bIgnoreNew)));
}


int CvCityAI::AI_cityValue() const
{

	AreaAITypes eAreaAI = area()->getAreaAIType(getTeam());
	if ((eAreaAI == AREAAI_OFFENSIVE) || (eAreaAI == AREAAI_MASSING) || (eAreaAI == AREAAI_DEFENSIVE))
	{
		return 0;
	}

	int iValue = 0;

	iValue += getCommerceRateTimes100(COMMERCE_GOLD);
	iValue += getCommerceRateTimes100(COMMERCE_RESEARCH);
	iValue += 100 * getYieldRate(YIELD_PRODUCTION);

	iValue -= 3 * calculateColonyMaintenanceTimes100();

	return iValue;
}

bool CvCityAI::AI_doPanic()
{

	bool bLandWar = ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING));

	if (bLandWar)
	{
		int iOurDefense = GET_PLAYER(getOwnerINLINE()).AI_getOurPlotStrength(plot(), 0, true, false);
		int iEnemyOffense = GET_PLAYER(getOwnerINLINE()).AI_getEnemyPlotStrength(plot(), 2, false, false);
		int iRatio = (100 * iEnemyOffense) / (std::max(1, iOurDefense));

		if (iRatio > 100)
		{
			UnitTypes eProductionUnit = getProductionUnit();

			if (eProductionUnit != NO_UNIT)
			{ // xUPT (dbkblk, 2015-02)
				if (GC.getUnitInfo(eProductionUnit).isMilitaryProduction() && (GC.getUnitInfo(eProductionUnit).getDomainType() == DOMAIN_LAND) && m_bAreaSaturatedOfLandMilitaryUnits)
				{
					return false;
				}
				if (!GC.getUnitInfo(eProductionUnit).isMilitaryProduction() && (GC.getUnitInfo(eProductionUnit).getDomainType() != DOMAIN_SEA) && m_bAreaSaturatedOfCivilianUnits)
				{
					return false;
				} // xUPT: end
				if (getProduction() > 0)
				{
/************************************************************************************************/
/* Afforess					  Start		 6/11/11												*/
/*																							  */
/*																							  */
/************************************************************************************************/
					if ((GC.getUnitInfo(eProductionUnit).getCombat() +
					GET_TEAM(getTeam()).getUnitClassStrengthChange((UnitClassTypes)GC.getUnitInfo(eProductionUnit).getUnitClassType())) > 0)
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
					{
						AI_doHurry(true);
						return true;
					}
				}
			}
			else
			{
				if ((getCitySorenRandNum(2, "AI choose panic unit") == 0) && AI_chooseUnitImmediate("panic defense", UNITAI_CITY_COUNTER))
				{
					AI_doHurry((iRatio > 140));
				}
				else if (AI_chooseUnitImmediate("panic defense", UNITAI_CITY_DEFENSE))
				{
					AI_doHurry((iRatio > 140));
				}
				else if (AI_chooseUnitImmediate("panic defense", UNITAI_ATTACK))
				{
					AI_doHurry((iRatio > 140));
				}
			}
		}
	}
	return false;
}

int CvCityAI::AI_calculateCulturePressure(bool bGreatWork)
{
	CvPlot* pLoopPlot;
	BonusTypes eNonObsoleteBonus;
	int iValue;
	int iTempValue;
	int iI;

	iValue = 0;
	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);
		if (pLoopPlot != NULL)
		{
			if (pLoopPlot->getOwnerINLINE() == NO_PLAYER)
			{
				iValue++;
			}
			else
			{
				iTempValue = pLoopPlot->calculateCulturePercent(getOwnerINLINE());
				if (iTempValue == 100)
				{
					//do nothing
				}
				else if ((iTempValue == 0) || (iTempValue > 75))
				{
					iValue++;
				}
				else
				{
					iTempValue = (100 - iTempValue);
					FAssert(iTempValue > 0);
					FAssert(iTempValue <= 100);

					if (iI != CITY_HOME_PLOT)
					{
						iTempValue *= 4;
						iTempValue /= NUM_CITY_PLOTS;
					}

					eNonObsoleteBonus = pLoopPlot->getNonObsoleteBonusType(getTeam());

					if (eNonObsoleteBonus != NO_BONUS)
					{
						iTempValue += (GET_PLAYER(getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus) * ((GET_PLAYER(getOwnerINLINE()).getNumTradeableBonuses(eNonObsoleteBonus) == 0) ? 4 : 2));
					}
/************************************************************************************************/
/* UNOFFICIAL_PATCH					   03/20/10						  denev & jdog5000	*/
/*																							  */
/* Bugfix																					   */
/************************************************************************************************/
/* original bts code
					if ((iTempValue > 80) && (pLoopPlot->getOwnerINLINE() == getID()))
*/
					if ((iTempValue > 80) && (pLoopPlot->getOwnerINLINE() == getOwnerINLINE()))
/************************************************************************************************/
/* UNOFFICIAL_PATCH						END												  */
/************************************************************************************************/
					{
						//captured territory special case
						iTempValue *= (100 - iTempValue);
						iTempValue /= 100;
					}

					if (pLoopPlot->getTeam() == getTeam())
					{
						iTempValue /= (bGreatWork ? 10 : 2);
					}
					else
					{
						iTempValue *= 2;
						if (bGreatWork)
						{
							if (GET_PLAYER(getOwnerINLINE()).AI_getAttitude(pLoopPlot->getOwnerINLINE()) == ATTITUDE_FRIENDLY)
							{
								iValue /= 10;
							}
						}
					}

					iValue += iTempValue;
				}
			}
		}
	}


	return iValue;
}


void CvCityAI::AI_buildGovernorChooseProduction()
{
	PROFILE_FUNC();

	CvArea* pWaterArea;
	bool bWasFoodProduction;
	bool bDanger;
	int iCulturePressure;

	bDanger = AI_isDanger();


	// only clear the dirty bit if we actually do a check, multiple items might be queued
	AI_setChooseProductionDirty(false);

	pWaterArea = waterArea();

	bWasFoodProduction = isFoodProduction();
	iCulturePressure = AI_calculateCulturePressure();
	int iMinValueDivisor = 1;
	if (getPopulation() < 3)
	{
		iMinValueDivisor = 3;
	}
	else if (getPopulation() < 7)
	{
		iMinValueDivisor = 2;
	}


	clearOrderQueue();

	if (bWasFoodProduction)
	{
		AI_assignWorkingPlots();
	}

//TB Build Mod (Bad Strategy)
#ifndef C2C_BUILD
	// if we need to pop borders, then do that immediately if we have drama and can do it
	// if we need to pop borders, then do that immediately if we have drama and can do it
	if ((getCultureLevel() <= (CultureLevelTypes)1) && ((getCommerceRate(COMMERCE_CULTURE) < 2) || (iCulturePressure > 0)))
	{
		if (AI_chooseProcess(COMMERCE_CULTURE))
		{
			return;
		}
	}
#endif

// BUG - Governor Builds Workboats - start
#ifdef _MOD_GOVWORKERS
	if (!isHuman() || GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_MODDER_1))
	{
#endif
	//workboat
	if (pWaterArea != NULL)
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_WORKER_SEA) == 0)
		{
			if (AI_neededSeaWorkers() > 0)
			{
				if (AI_chooseUnit("no sea workers for area", UNITAI_WORKER_SEA))
				{
					return;
				}
			}
		}
	}
#ifdef _MOD_GOVWORKERS
	}
#endif
// BUG - Governor Builds Workboats - end

	if ((AI_countNumBonuses(NO_BONUS, false, true, 10, true, true) > 0)
		&& (getPopulation() > AI_countNumBonuses(NO_BONUS, true, false, -1, true, true)))
	{
		if (getCommerceRate(COMMERCE_CULTURE) == 0)
		{
			AI_chooseBuilding(BUILDINGFOCUS_CULTURE);
			return;
		}
	}

	// pick granary or lighthouse, any duration
	if (AI_chooseBuilding(BUILDINGFOCUS_FOOD))
	{
		return;
	}

	if (angryPopulation(1) > 1)
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_HAPPY, 40))
		{
			return;
		}
	}

//TB Build Mod (Move up in priority)
#ifndef C2C_BUILD
	if (AI_chooseBuilding(BUILDINGFOCUS_PRODUCTION, 30, 10 / iMinValueDivisor))
	{
		return;
	}
#endif

	if (AI_chooseBuilding(BUILDINGFOCUS_EXPERIENCE, 8, 33))
	{
		return;
	}


	if (((getCommerceRateTimes100(COMMERCE_CULTURE) == 0) && (iCulturePressure != 0))
		|| (iCulturePressure > 100))
	{
		if (AI_chooseBuilding(BUILDINGFOCUS_CULTURE, 30))
		{
			return;
		}
	}


	int iEconomyFlags = 0;
	iEconomyFlags |= BUILDINGFOCUS_GOLD;
	iEconomyFlags |= BUILDINGFOCUS_RESEARCH;
	iEconomyFlags |= BUILDINGFOCUS_MAINTENANCE;
	iEconomyFlags |= BUILDINGFOCUS_HAPPY;
	iEconomyFlags |= BUILDINGFOCUS_HEALTHY;
	iEconomyFlags |= BUILDINGFOCUS_SPECIALIST;
	if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
	{
		iEconomyFlags |= BUILDINGFOCUS_ESPIONAGE;
	}

	//20 means 5g or ~2 happiness...
	if (AI_chooseBuilding(iEconomyFlags, 20, 20 / iMinValueDivisor))
	{
		return;
	}

// BUG - Governor Builds Workers - start
#ifdef _MOD_GOVWORKERS
	if (!isHuman() || GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_MODDER_1))
	{
#endif
	int iExistingWorkers = GET_PLAYER(getOwner()).AI_totalAreaUnitAIs(area(), UNITAI_WORKER);
	int iNeededWorkers = GET_PLAYER(getOwner()).AI_neededWorkers(area());

	if (!bDanger && (iExistingWorkers < ((iNeededWorkers + 1) / 2)))
	{
		if (AI_chooseUnit("no danger optional worker", UNITAI_WORKER))
		{
			return;
		}
	}
#ifdef _MOD_GOVWORKERS
	}
#endif
// BUG - Governor Builds Workers - end

	if (GC.getDefineINT("DEFAULT_SPECIALIST") != NO_SPECIALIST)
	{
		if (getSpecialistCount((SpecialistTypes)(GC.getDefineINT("DEFAULT_SPECIALIST"))) > 0)
		{
			if (AI_chooseBuilding(BUILDINGFOCUS_SPECIALIST, 60))
			{
				return;
			}
		}
	}

	if (AI_chooseBuilding(iEconomyFlags, 40, 15 / iMinValueDivisor))
	{
		return;
	}

	if (AI_chooseBuilding(iEconomyFlags | BUILDINGFOCUS_CULTURE, 10, 10 / iMinValueDivisor))
	{
		return;
	}


	if (AI_chooseProcess())
	{
		return;
	}

	if (AI_chooseBuilding())
	{
		return;
	}

	if (AI_chooseUnit("crappy governor"))
	{
		return;
	}
}

int CvCityAI::AI_calculateWaterWorldPercent()
{
	int iI;
	int iWaterPercent = 0;
	int iTeamCityCount = 0;
	int iOtherCityCount = 0;
	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (iI == getTeam() || GET_TEAM((TeamTypes)iI).isVassal(getTeam())
				|| GET_TEAM(getTeam()).isVassal((TeamTypes)iI))
			{
				iTeamCityCount += GET_TEAM((TeamTypes)iI).countNumCitiesByArea(area());
			}
			else
			{
				iOtherCityCount += GET_TEAM((TeamTypes)iI).countNumCitiesByArea(area());
			}
		}
	}

	if (iOtherCityCount == 0)
	{
		iWaterPercent = 100;
	}
	else
	{
		iWaterPercent = 100 - ((iTeamCityCount + iOtherCityCount) * 100) / std::max(1, (GC.getGame().getNumCities()));
	}

	iWaterPercent *= 50;
	iWaterPercent /= 100;

	iWaterPercent += (50 * (2 + iTeamCityCount)) / (2 + iTeamCityCount + iOtherCityCount);

	iWaterPercent = std::max(1, iWaterPercent);


	return iWaterPercent;
}

//Please note, takes the yield multiplied by 100
int CvCityAI::AI_getYieldMagicValue(const int* piYieldsTimes100, bool bHealthy)
{
	FAssert(piYieldsTimes100 != NULL);

	int iPopEats = GC.getFOOD_CONSUMPTION_PER_POPULATION();
	iPopEats += (bHealthy ? 0 : 1);
	iPopEats *= 100;

	int iValue = ((piYieldsTimes100[YIELD_FOOD] * 100 + piYieldsTimes100[YIELD_PRODUCTION]*55 + piYieldsTimes100[YIELD_COMMERCE]*40) - iPopEats * 102);
	iValue /= 100;
	return iValue;
}

//The magic value is basically "Look at this plot, is it worth working"
//-50 or lower means the plot is worthless in a "workers kill yourself" kind of way.
//-50 to -1 means the plot isn't worth growing to work - might be okay with emphasize though.
//Between 0 and 50 means it is marginal.
//50-100 means it's okay.
//Above 100 means it's definitely decent - seriously question ever not working it.
//This function deliberately doesn't use emphasize settings.
int CvCityAI::AI_getPlotMagicValue(CvPlot* pPlot, bool bHealthy, bool bWorkerOptimization)
{
	int aiYields[NUM_YIELD_TYPES];
	ImprovementTypes eCurrentImprovement;
	ImprovementTypes eFinalImprovement;
	int iI;
	int iYieldDiff;

	FAssert(pPlot != NULL);

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		if ((bWorkerOptimization) && (pPlot->getWorkingCity() == this) && (AI_getBestBuild(getCityPlotIndex(pPlot)) != NO_BUILD))
		{
			aiYields[iI] = pPlot->getYieldWithBuild(AI_getBestBuild(getCityPlotIndex(pPlot)), (YieldTypes)iI, true);
		}
		else
		{
			aiYields[iI] = pPlot->getYield((YieldTypes)iI) * 100;
		}
	}

	eCurrentImprovement = pPlot->getImprovementType();

	if (eCurrentImprovement != NO_IMPROVEMENT)
	{
		eFinalImprovement = finalImprovementUpgrade(eCurrentImprovement);

		if ((eFinalImprovement != NO_IMPROVEMENT) && (eFinalImprovement != eCurrentImprovement))
		{
			for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
			{
				iYieldDiff = 100 * pPlot->calculateImprovementYieldChange(eFinalImprovement, ((YieldTypes)iI), getOwnerINLINE());
				iYieldDiff -= 100 * pPlot->calculateImprovementYieldChange(eCurrentImprovement, ((YieldTypes)iI), getOwnerINLINE());
				aiYields[iI] += iYieldDiff / 2;
			}
		}
	}

	return AI_getYieldMagicValue(aiYields, bHealthy);
}

//useful for deciding whether or not to grow... or whether the city needs terrain
//improvement.
//if healthy is false it assumes bad health conditions.
int CvCityAI::AI_countGoodTiles(bool bHealthy, bool bUnworkedOnly, int iThreshold, bool bWorkerOptimization)
{
	CvPlot* pLoopPlot;
	int iI;
	int iCount;

	iCount = 0;
/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 06/17/09														*/
/*																							  */
/*																							  */
/************************************************************************************************/
	for (iI = 0; iI < getNumCityPlots(); iI++)
/************************************************************************************************/
/* JOOYO_ADDON						  END													 */
/************************************************************************************************/
	{
		pLoopPlot = plotCity(getX_INLINE(),getY_INLINE(), iI);
		if ((iI != CITY_HOME_PLOT) && (pLoopPlot != NULL))
		{
			if (pLoopPlot->getWorkingCity() == this)
			{
				if (!bUnworkedOnly || !(pLoopPlot->isBeingWorked()))
				{
					if (AI_getPlotMagicValue(pLoopPlot, bHealthy) > iThreshold)
					{
						iCount++;
					}
				}
			}
		}
	}
	return iCount;
}

int CvCityAI::AI_calculateTargetCulturePerTurn()
{
	/*
	int iTarget = 0;

	bool bAnyGoodPlotUnowned = false;
	bool bAnyGoodPlotHighPressure = false;

	for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		CvPlot* pLoopPlot = plotCity(getX_INLINE(),getY_INLINE(),iI);

		if (pLoopPlot != NULL)
		{
			if ((pLoopPlot->getBonusType(getTeam()) != NO_BONUS)
				|| (pLoopPlot->getYield(YIELD_FOOD) > GC.getFOOD_CONSUMPTION_PER_POPULATION()))
			{
				if (!pLoopPlot->isOwned())
				{
					bAnyGoodPlotUnowned = true;
				}
				else if (pLoopPlot->getOwnerINLINE() != getOwnerINLINE())
				{
					bAnyGoodPlotHighPressure = true;
				}
			}
		}
	}
	if (bAnyGoodPlotUnowned)
	{
		iTarget = 1;
	}
	if (bAnyGoodPlotHighPressure)
	{
		iTarget += getCommerceRate(COMMERCE_CULTURE) + 1;
	}
	return iTarget;
	*/
	return 1;
}

int CvCityAI::AI_countGoodSpecialists(bool bHealthy)
{
	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	int iCount = 0;
	for (int iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		SpecialistTypes eSpecialist = (SpecialistTypes)iI;

		int iValue = 0;

		iValue += 100 * kPlayer.specialistYield(eSpecialist, YIELD_FOOD);
		iValue += 65 * kPlayer.specialistYield(eSpecialist, YIELD_PRODUCTION);
		iValue += 40 * kPlayer.specialistYield(eSpecialist, YIELD_COMMERCE);

		iValue += 40 * kPlayer.specialistCommerce(eSpecialist, COMMERCE_RESEARCH);
		iValue += 40 * kPlayer.specialistCommerce(eSpecialist, COMMERCE_GOLD);
		iValue += 20 * kPlayer.specialistCommerce(eSpecialist, COMMERCE_ESPIONAGE);
		iValue += 15 * kPlayer.specialistCommerce(eSpecialist, COMMERCE_CULTURE);
		iValue += 25 * GC.getSpecialistInfo(eSpecialist).getGreatPeopleRateChange();
/************************************************************************************************/
/* Specialists Enhancements, by Supercheese 10/12/09											*/
/*																							  */
/*																							  */
/************************************************************************************************/
		iValue += GC.getSpecialistInfo(eSpecialist).getHealthPercent();
		iValue += GC.getSpecialistInfo(eSpecialist).getHappinessPercent();
/************************************************************************************************/
/* Specialists Enhancements						  END										*/
/************************************************************************************************/

		if (iValue >= (bHealthy ? 200 : 300))
		{
			iCount += getMaxSpecialistCount(eSpecialist);
		}
	}
	iCount -= getFreeSpecialist();

	return iCount;
}
//0 is normal
//higher than zero means special.
int CvCityAI::AI_getCityImportance(bool bEconomy, bool bMilitary)
{
	int iValue = 0;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  03/08/10								jdog5000	  */
/*																							  */
/* Victory Strategy AI																		  */
/************************************************************************************************/
	if (GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2))
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
	{
		int iCultureRateRank = findCommerceRateRank(COMMERCE_CULTURE);
		int iCulturalVictoryNumCultureCities = GC.getGameINLINE().culturalVictoryNumCultureCities();

		if (iCultureRateRank <= iCulturalVictoryNumCultureCities)
		{
			iValue += 100;

			if ((getCultureLevel() < (GC.getNumCultureLevelInfos() - 1)))
			{
				iValue += !bMilitary ? 100 : 0;
			}
			else
			{
				iValue += bMilitary ? 100 : 0;
			}
		}
	}

	return iValue;
}

void CvCityAI::AI_stealPlots()
{
	PROFILE_FUNC();
	CvPlot* pLoopPlot;
	CvCityAI* pWorkingCity;
	int iI;
	int iOtherImportance;

	int iImportance = AI_getCityImportance(true, false);

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		pLoopPlot = plotCity(getX_INLINE(),getY_INLINE(),iI);

		if (pLoopPlot != NULL)
		{
			if (iImportance > 0)
			{
				if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE())
				{
					pWorkingCity = static_cast<CvCityAI*>(pLoopPlot->getWorkingCity());
					if ((pWorkingCity != this) && (pWorkingCity != NULL))
					{
						FAssert(pWorkingCity->getOwnerINLINE() == getOwnerINLINE());
						iOtherImportance = pWorkingCity->AI_getCityImportance(true, false);
						if (iImportance > iOtherImportance)
						{
							pLoopPlot->setWorkingCityOverride(this);
						}
					}
				}
			}

			if (pLoopPlot->getWorkingCityOverride() == this)
			{
				if (pLoopPlot->getOwnerINLINE() != getOwnerINLINE())
				{
					pLoopPlot->setWorkingCityOverride(NULL);
				}
			}
		}
	}
}




// +1/+3/+5 plot based on base food yield (1/2/3)
// +4 if being worked.
// +4 if a bonus.
// Unworked ocean ranks very lowly. Unworked lake ranks at 3. Worked lake at 7.
// Worked bonus in ocean ranks at like 11
int CvCityAI::AI_buildingSpecialYieldChangeValue(BuildingTypes eBuilding, YieldTypes eYield)
{
	int iI;
	CvPlot* pLoopPlot;
	int iValue = 0;
	CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);
	int iWorkedCount = 0;

	int iYieldChange = kBuilding.getSeaPlotYieldChange(eYield);
	if (iYieldChange > 0)
	{
		int iWaterCount = 0;
/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 06/17/09														*/
/*																							  */
/*																							  */
/************************************************************************************************/
		for (iI = 0; iI < getNumCityPlots(); iI++)
/************************************************************************************************/
/* JOOYO_ADDON						  END													 */
/************************************************************************************************/
		{
			if (iI != CITY_HOME_PLOT)
			{
				pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);
				if ((pLoopPlot != NULL) && (pLoopPlot->getWorkingCity() == this))
				{
					if (pLoopPlot->isWater())
					{
						iWaterCount++;
						int iFood = pLoopPlot->getYield(YIELD_FOOD);
						iFood += (eYield == YIELD_FOOD) ? iYieldChange : 0;

						iValue += std::max(0, iFood * 2 - 1);
						if (pLoopPlot->isBeingWorked())
						{
							iValue += 4;
							iWorkedCount++;
						}
						iValue += ((pLoopPlot->getBonusType(getTeam()) != NO_BONUS) ? 8 : 0);
					}
				}
			}
		}
	}
	if (iWorkedCount == 0)
	{
		SpecialistTypes eDefaultSpecialist = (SpecialistTypes)GC.getDefineINT("DEFAULT_SPECIALIST");
		if ((getPopulation() > 2) && ((eDefaultSpecialist == NO_SPECIALIST) || (getSpecialistCount(eDefaultSpecialist) == 0)))
		{
			iValue /= 2;
		}
	}

	return iValue;
}


int CvCityAI::AI_yieldMultiplier(YieldTypes eYield)
{
	PROFILE_FUNC();

	int iMultiplier = getBaseYieldRateModifier(eYield);

	if (eYield == YIELD_PRODUCTION)
	{
		iMultiplier += (getMilitaryProductionModifier() / 2);
	}

	if (eYield == YIELD_COMMERCE)
	{
		iMultiplier += (getCommerceRateModifier(COMMERCE_RESEARCH) * 60) / 100;
		iMultiplier += (getCommerceRateModifier(COMMERCE_GOLD) * 35) / 100;
		iMultiplier += (getCommerceRateModifier(COMMERCE_CULTURE) * 15) / 100;
	}

	return iMultiplier;
}
//this should be called before doing governor stuff.
//This is the function which replaces emphasis
//Could stand for a Commerce Variety to be added
//especially now that there is Espionage
void CvCityAI::AI_updateSpecialYieldMultiplier()
{
	PROFILE_FUNC();

	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		m_aiSpecialYieldMultiplier[iI] = 0;
	}

	UnitTypes eProductionUnit = getProductionUnit();
	if (eProductionUnit != NO_UNIT)
	{
		if (GC.getUnitInfo(eProductionUnit).getDefaultUnitAIType() == UNITAI_WORKER_SEA)
		{
			m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] += 50;
			m_aiSpecialYieldMultiplier[YIELD_COMMERCE] -= 50;
		}
		if ((GC.getUnitInfo(eProductionUnit).getDefaultUnitAIType() == UNITAI_WORKER) ||
			(GC.getUnitInfo(eProductionUnit).getDefaultUnitAIType() == UNITAI_SETTLE))

		{
			m_aiSpecialYieldMultiplier[YIELD_COMMERCE] -= 50;
		}
	}

	BuildingTypes eProductionBuilding = getProductionBuilding();
	if (eProductionBuilding != NO_BUILDING)
	{
		if (isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eProductionBuilding).getBuildingClassType()))
			|| isProductionProject())
		{
			m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] += 50;
			m_aiSpecialYieldMultiplier[YIELD_COMMERCE] -= 25;
		}
		m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] += std::max(-25, GC.getBuildingInfo(eProductionBuilding).getFoodKept());

		if ((GC.getBuildingInfo(eProductionBuilding).getCommerceChange(COMMERCE_CULTURE) > 0)
			|| (GC.getBuildingInfo(eProductionBuilding).getObsoleteSafeCommerceChange(COMMERCE_CULTURE) > 0))
		{
			int iTargetCultureRate = AI_calculateTargetCulturePerTurn();
			if (iTargetCultureRate > 0)
			{
				if (getCommerceRate(COMMERCE_CULTURE) == 0)
				{
					m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] += 50;
				}
				else if (getCommerceRate(COMMERCE_CULTURE) < iTargetCultureRate)
				{
					m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] += 20;
				}
			}
		}
	}

	// non-human production value increase
	if (!isHuman())
	{
		CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
		AreaAITypes eAreaAIType = area()->getAreaAIType(getTeam());

		if ((kPlayer.AI_isDoStrategy(AI_STRATEGY_DAGGER) && getPopulation() >= 4)
			|| (eAreaAIType == AREAAI_OFFENSIVE) || (eAreaAIType == AREAAI_DEFENSIVE)
			|| (eAreaAIType == AREAAI_MASSING) || (eAreaAIType == AREAAI_ASSAULT))
		{
			m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] += 10;
			if (!kPlayer.AI_isFinancialTrouble())
			{
				m_aiSpecialYieldMultiplier[YIELD_COMMERCE] -= 40;
			}
		}

		int iIncome = 1 + kPlayer.getCommerceRate(COMMERCE_GOLD) + kPlayer.getCommerceRate(COMMERCE_RESEARCH) + std::max(0, kPlayer.getGoldPerTurn());
		int iExpenses = 1 + kPlayer.calculateInflatedCosts() - std::min(0, kPlayer.getGoldPerTurn());
		FAssert(iIncome > 0);

/************************************************************************************************/
/* Afforess					  Start		 12/7/09												*/
/*																							  */
/*	   Divide By Zero Fix																	 */
/************************************************************************************************/
		int iRatio = (100 * iExpenses) / std::max(1, iIncome);
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
		//Gold -> Production Reduced To
		// 40- -> 100%
		// 60 -> 83%
		// 100 -> 28%
		// 110+ -> 14%
		m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] += 100;
		if (iRatio > 60)
		{
			//Greatly decrease production weight
			m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] *= std::max(10, 120 - iRatio);
			m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] /= 72;
		}
		else if (iRatio > 40)
		{
			//Slightly decrease production weight.
			m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] *= 160 - iRatio;
			m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] /= 120;
		}
		m_aiSpecialYieldMultiplier[YIELD_PRODUCTION] -= 100;
	}
}

int CvCityAI::AI_specialYieldMultiplier(YieldTypes eYield)
{
	return m_aiSpecialYieldMultiplier[eYield];
}


int CvCityAI::AI_countNumBonuses(BonusTypes eBonus, bool bIncludeOurs, bool bIncludeNeutral, int iOtherCultureThreshold, bool bLand, bool bWater)
{
	CvPlot* pLoopPlot;
	BonusTypes eLoopBonus;
	int iI;
	int iCount = 0;
/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 06/17/09														*/
/*																							  */
/*																							  */
/************************************************************************************************/
	for (iI = 0; iI < getNumCityPlots(); iI++)
/************************************************************************************************/
/* JOOYO_ADDON						  END													 */
/************************************************************************************************/
	{
		pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

		if (pLoopPlot != NULL)
		{
			if ((pLoopPlot->area() == area()) || (bWater && pLoopPlot->isWater()))
			{
				eLoopBonus = pLoopPlot->getBonusType(getTeam());
				if (eLoopBonus != NO_BONUS)
				{
					if ((eBonus == NO_BONUS) || (eBonus == eLoopBonus))
					{
						if (bIncludeOurs && (pLoopPlot->getOwnerINLINE() == getOwnerINLINE()) && (pLoopPlot->getWorkingCity() == this))
						{
							iCount++;
						}
						else if (bIncludeNeutral && (!pLoopPlot->isOwned()))
						{
							iCount++;
						}
						else if ((iOtherCultureThreshold > 0) && (pLoopPlot->isOwned() && (pLoopPlot->getOwnerINLINE() != getOwnerINLINE())))
						{
							if ((pLoopPlot->getCulture(pLoopPlot->getOwnerINLINE()) - pLoopPlot->getCulture(getOwnerINLINE())) < iOtherCultureThreshold)
							{
								iCount++;
							}
						}
					}
				}
			}
		}
	}


	return iCount;

}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  11/14/09								jdog5000	  */
/*																							  */
/* City AI																					  */
/************************************************************************************************/
int CvCityAI::AI_countNumImprovableBonuses( bool bIncludeNeutral, TechTypes eExtraTech, bool bLand, bool bWater )
{
	CvPlot* pLoopPlot;
	BonusTypes eLoopBonus;
	int iI;
	int iCount = 0;
	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

		if (pLoopPlot != NULL)
		{
			if ((bLand && pLoopPlot->area() == area()) || (bWater && pLoopPlot->isWater()))
			{
				eLoopBonus = pLoopPlot->getBonusType(getTeam());
				if (eLoopBonus != NO_BONUS)
				{
					if ( ((pLoopPlot->getOwnerINLINE() == getOwnerINLINE()) && (pLoopPlot->getWorkingCity() == this)) || (bIncludeNeutral && (!pLoopPlot->isOwned())))
					{
						for (int iJ = 0; iJ < GC.getNumBuildInfos(); iJ++)
						{
							BuildTypes eBuild = ((BuildTypes)iJ);

							if( eBuild != NO_BUILD && pLoopPlot->canBuild(eBuild, getOwnerINLINE()) )
							{
								ImprovementTypes eImp = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();

								if( eImp != NO_IMPROVEMENT && GC.getImprovementInfo(eImp).isImprovementBonusTrade(eLoopBonus) )
								{
									if( GET_PLAYER(getOwnerINLINE()).canBuild(pLoopPlot, eBuild) )
									{
										iCount++;
										break;
									}
									else if( (eExtraTech != NO_TECH) )
									{
										if (GC.getBuildInfo(eBuild).getTechPrereq() == eExtraTech)
										{
											iCount++;
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}


	return iCount;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

int CvCityAI::AI_playerCloseness(PlayerTypes eIndex, int iMaxDistance)
{
	FAssert(GET_PLAYER(eIndex).isAlive());
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  05/16/10							  jdog5000		*/
/*																							  */
/* War tactics AI																			   */
/************************************************************************************************/
/* original bts code
	FAssert(eIndex != getID());
*/
	// No point checking player type against city ID ... Firaxis copy and paste error from
	// CvPlayerAI version of this function
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/

	if ((m_iCachePlayerClosenessTurn != GC.getGame().getGameTurn())
		|| (m_iCachePlayerClosenessDistance != iMaxDistance))
	{
		AI_cachePlayerCloseness(iMaxDistance);
	}

	return m_aiPlayerCloseness[eIndex];
}

void CvCityAI::AI_cachePlayerCloseness(int iMaxDistance)
{
	PROFILE_FUNC();
	CvCity* pLoopCity;
	int iI;
	int iLoop = 0;
	int iValue;
	int iTempValue;
	int iBestValue;

/********************************************************************************/
/* 	BETTER_BTS_AI_MOD						5/16/10				jdog5000		*/
/* 																				*/
/* 	General AI, closeness changes												*/
/********************************************************************************/
	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive() &&
			((GET_TEAM(getTeam()).isHasMet(GET_PLAYER((PlayerTypes)iI).getTeam()))))
		{
			iValue = 0;
			iBestValue = 0;
			for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
			{
				if( pLoopCity == this )
				{
					continue;
				}

				int iDistance = stepDistance(getX_INLINE(), getY_INLINE(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE());
				/*  <f1rpo> No functional change here. It's OK to use a higher search range
					for cities on other continents; but will have to decrease the distance
					later on when computing the closeness value. (advc.107) */
				bool const bSameArea = (getArea() == pLoopCity->getArea());
				if (!bSameArea) // </f1rpo>
				{
					iDistance += 1;
					iDistance /= 2;
				}
				if (iDistance <= iMaxDistance)
				{
					if (bSameArea)
					{
						int iPathDistance = GC.getMap().calculatePathDistance(plot(), pLoopCity->plot());
						if (iPathDistance > 0)
						{
							iDistance = iPathDistance;
						}
					}
					if (iDistance <= iMaxDistance)
					{
						// Weight by population of both cities, not just pop of other city
						//iTempValue = 20 + 2*pLoopCity->getPopulation();
						iTempValue = 20 + pLoopCity->getPopulation() + getPopulation();

						iTempValue *= (1 + (iMaxDistance - iDistance));
						iTempValue /= (1 + iMaxDistance);

						//reduce for small islands.
						int iAreaCityCount = pLoopCity->area()->getNumCities();
						iTempValue *= std::min(iAreaCityCount, 5);
						iTempValue /= 5;
						if (iAreaCityCount < 3)
						{
							iTempValue /= 2;
						}

						if (pLoopCity->isBarbarian())
						{
							iTempValue /= 4;
						}
						// <f1rpo>
						if(!bSameArea)
							iTempValue /= (pLoopCity->isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()) ? 2 : 3);
						// </f1rpo>
						iValue += iTempValue;
						iBestValue = std::max(iBestValue, iTempValue);
					}
				}
			}
			m_aiPlayerCloseness[iI] = (iBestValue + iValue / 4);
		}
	}
/********************************************************************************/
/* 	BETTER_BTS_AI_MOD						END									*/
/********************************************************************************/

	m_iCachePlayerClosenessTurn = GC.getGame().getGameTurn();
	m_iCachePlayerClosenessDistance = iMaxDistance;
}

int	CvCityAI::getPlayerDangerPercentage(PlayerTypes ePlayer, int& iModifier) const
{
	int			iResult;
	CvPlayer&	kPlayer = GET_PLAYER(ePlayer);
	bool		bCrushStrategy = GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_CRUSH);

	iModifier = 0;

	if ((bCrushStrategy) && (GET_TEAM(getTeam()).AI_getWarPlan(kPlayer.getTeam()) != NO_WARPLAN))
	{
		iResult = 400;
	}
	else if (atWar(getTeam(), kPlayer.getTeam()))
	{
		iResult = 300;
	}
	// Beef up border security before starting war, but not too much
	else if ( GET_TEAM(getTeam()).AI_getWarPlan(kPlayer.getTeam()) != NO_WARPLAN )
	{
		iResult = 180;
	}
	// Extra trust of/for Vassals, regardless of relations
	else if ( GET_TEAM(kPlayer.getTeam()).isVassal(getTeam()) ||
				GET_TEAM(getTeam()).isVassal(kPlayer.getTeam()))
	{
		iResult = 30;
	}
	else
	{
		switch (GET_PLAYER(getOwnerINLINE()).AI_getAttitude(ePlayer))
		{
		case ATTITUDE_FURIOUS:
			iResult = 180;
			break;

		case ATTITUDE_ANNOYED:
			iResult = 130;
			break;

		case ATTITUDE_CAUTIOUS:
			iResult = 100;
			break;

		case ATTITUDE_PLEASED:
			iResult = 50;
			break;

		case ATTITUDE_FRIENDLY:
			iResult = 20;
			break;

		default:
			FAssert(false);
			break;
		}

/********************************************************************************/
/*	RevDCM Better BUG AI changes	28.10.2010							Fuyu	*/
/********************************************************************************/
		// Beef up border security next to powerful rival, (Fuyu) just not too much if our units are weaker on average
		if( kPlayer.getPower() > GET_PLAYER(getOwnerINLINE()).getPower() )
		{
			int iTempMultiplier = std::min( 400, (100 * kPlayer.getPower())/std::max(1, GET_PLAYER(getOwnerINLINE()).getPower()) );
			iTempMultiplier += range(( (100 * kPlayer.getNumMilitaryUnits())/std::max(1, GET_PLAYER(getOwnerINLINE()).getNumMilitaryUnits()) ), 100, iTempMultiplier);
			iTempMultiplier /= 2;
			iResult *= iTempMultiplier;
			iResult /= 100;
		}
/********************************************************************************/
/*	RevDCM Better BUG AI changes	28.10.2010							END		*/
/********************************************************************************/


/************************************************************************************************/
/* UNOFFICIAL_PATCH					   01/04/09								jdog5000	  */
/*																							  */
/* Bugfix																					   */
/************************************************************************************************/
/* orginal bts code
		if (bCrushStrategy)
		{
			iValue /= 2;
		}
*/
		if (bCrushStrategy)
		{
			iResult /= 2;
		}
/************************************************************************************************/
/* UNOFFICIAL_PATCH						END												  */
/************************************************************************************************/
	}

	//	Koshling - modify by the difference in difficulty levels between us and the player
	//	concerned (upwards only). This is to encourage the AI to be more prepared against humnas
	//	playing on higher difficulties
	int	iDifficultyDifference = (GET_PLAYER(getOwnerINLINE()).getHandicapType() - GET_PLAYER(ePlayer).getHandicapType());
	if ( iDifficultyDifference < 0 )
	{
		iModifier = -10*iDifficultyDifference;
		iResult = (iResult*(100 + iModifier))/100;
	}

	return iResult;
}

int CvCityAI::AI_cityThreat(TeamTypes eTargetTeam, int* piThreatModifier)
{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					  03/04/10								jdog5000	  */
/*																							  */
/* War tactics AI																			   */
/************************************************************************************************/
	PROFILE_FUNC();

	if ( m_iCityThreat != -1 && eTargetTeam == NO_TEAM )
	{
		if ( piThreatModifier != NULL )
		{
			*piThreatModifier = m_iCityThreatModifier;
		}
		return m_iCityThreat;
	}

	int iValue = (eTargetTeam == NO_TEAM ? evaluateDanger() : 0);
	int iModifier = 0;

	//Afforess: changed to MAX_CIV_PLAYERS to ignore BARBARIAN_PLAYER, we will evaluate that separately
	for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if ((iI != getOwnerINLINE()) && GET_PLAYER((PlayerTypes)iI).isAlive() && (eTargetTeam == NO_TEAM || GET_PLAYER((PlayerTypes)iI).getTeam() == eTargetTeam))
		{
			int iCloseness = AI_playerCloseness((PlayerTypes)iI, DEFAULT_PLAYER_CLOSENESS);
			if (iCloseness > 0)
			{
				int iTempModifier;

				int iPlayerDangerPercentage = getPlayerDangerPercentage((PlayerTypes)iI, iTempModifier);
				int iTempValue = iCloseness * iPlayerDangerPercentage;
				iTempValue /= 100;
				iValue += iTempValue;

				iModifier += iTempModifier;

				if (gCityLogLevel >= 3) logBBAIForTeam(getTeam(), "		  AI_cityThreat for city: %S, player danger for %S (modifier: %d, value: %d, iCloseness %d, iPlayerDangerPercentage: %d)", getName().GetCString(), GET_PLAYER((PlayerTypes)iI).getCivilizationDescription(0), iTempModifier, iTempValue, iCloseness, iPlayerDangerPercentage);
			}
		}
	}

	if (GET_PLAYER(BARBARIAN_PLAYER).isAlive())
	{
		int iCloseness = AI_playerCloseness(BARBARIAN_PLAYER, DEFAULT_PLAYER_CLOSENESS);
		if (iCloseness > 0)
		{
			iValue += 70 / iCloseness;
		}
	}

	if (isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()))
	{
		int iCurrentEra = GET_PLAYER(getOwnerINLINE()).getCurrentEra();
		int iTempValue = std::max(0, ((10 * iCurrentEra) / 3) - 6); //there are better ways to do this
		if (gCityLogLevel >= 3) logBBAIForTeam(getTeam(), "		  AI_cityThreat for city: %S, coastal extra danger: %d", getName().GetCString(), iTempValue);

		iValue += iTempValue;
	}

	iValue += getNumActiveWorldWonders() * 5;

	if (gCityLogLevel >= 3 && getNumActiveWorldWonders() > 0) logBBAIForTeam(getTeam(), "		  AI_cityThreat for city: %S, extra wonder danger: %d", getName().GetCString(), getNumActiveWorldWonders() * 5);

	if (GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3))
	{
		int iTempValue = 5;
		iTempValue += getCommerceRateModifier(COMMERCE_CULTURE) / 20;
		if (getCultureLevel() >= (GC.getNumCultureLevelInfos() - 2))
		{
			iTempValue += 20;
			if (getCultureLevel() >= (GC.getNumCultureLevelInfos() - 1))
			{
				iTempValue += 30;
			}
		}
		iValue += iTempValue;
		if (gCityLogLevel >= 3) logBBAIForTeam(getTeam(), "		  AI_cityThreat for city: %S, AI_VICTORY_CULTURE3: %d", getName().GetCString(), iTempValue);
	}

	iValue += 2 * GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 3, false);
/************************************************************************************************/
/* REVOLUTION_MOD						 05/22/08								jdog5000	  */
/*																							  */
/* Revolution AI																				*/
/************************************************************************************************/
	if( getRevolutionIndex() > 750 )
	{
		iValue += getRevolutionIndex() / 25;
		if (gCityLogLevel >= 3) logBBAIForTeam(getTeam(), "		  AI_cityThreat for city: %S, revolution index: %d", getName().GetCString(), getRevolutionIndex() / 25);
	}
	else
	{
		iValue += getRevolutionIndex() / 100;
	}
/************************************************************************************************/
/* REVOLUTION_MOD						  END												  */
/************************************************************************************************/

	if (gCityLogLevel >= 3) logBBAIForTeam(getTeam(), "		  AI_cityThreat total for city: %S, %d", getName().GetCString(), iValue);
	m_iCityThreat = iValue;
	m_iCityThreatModifier = iModifier;
	if ( piThreatModifier != NULL )
	{
		*piThreatModifier = m_iCityThreatModifier;
	}

	return iValue;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD					   END												  */
/************************************************************************************************/
}

//Workers have/needed is not intended to be a strict
//target but rather an indication.
//if needed is at least 1 that means a worker
//will be doing something useful
int CvCityAI::AI_getWorkersHave()
{
	return m_iWorkersHave;
}

int CvCityAI::AI_getWorkersNeeded()
{
	return m_iWorkersNeeded;
}

void CvCityAI::AI_changeWorkersHave(int iChange)
{
	m_iWorkersHave += iChange;
	//FAssert(m_iWorkersHave >= 0);
	m_iWorkersHave = std::max(0, m_iWorkersHave);
}

//This needs to be serialized for human workers.
void CvCityAI::AI_updateWorkersNeededHere()
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;

	short aiYields[NUM_YIELD_TYPES];

	int iWorkersNeeded = 0;
	int iWorkersHave = 0;
	int iUnimprovedWorkedPlotCount = 0;
	int iUnimprovedUnworkedPlotCount = 0;
	int iWorkedUnimprovableCount = 0;
	int iImprovedUnworkedPlotCount = 0;

	int iSpecialCount = 0;

	int iWorstWorkedPlotValue = MAX_INT;
	int iBestUnworkedPlotValue = 0;

	iWorkersHave = 0;

	if (getProductionUnit() != NO_UNIT)
	{
		if (getProductionUnitAI() == UNITAI_WORKER)
		{
			if (getProductionTurnsLeft() <= 2)
			{
				iWorkersHave++;
			}
		}
	}

/********************************************************************************/
/* 	Worker Counting						03.08.2010				Fuyu			*/
/********************************************************************************/
	//iWorkersHave += iWorkersHaveByPlotTargetMissionAI;
	int iWorkersHaveByPlotTargetMissionAI = AI_workingCityPlotTargetMissionAIs(getOwnerINLINE(), MISSIONAI_BUILD, UNITAI_WORKER, true);
	int iWorkersHaveNewlyBuilt = 0;
/********************************************************************************/
/* 	Worker Counting 											END 			*/
/********************************************************************************/

	for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		pLoopPlot = getCityIndexPlot(iI);
		if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this)
		{
			if (pLoopPlot->getArea() == getArea())
			{
				// BBAI TODO: Check late game slowness

				//How slow is this? It could be almost NUM_CITY_PLOT times faster
				//by iterating groups and seeing if the plot target lands in this city
				//but since this is only called once/turn i'm not sure it matters.

/********************************************************************************/
/* 	Worker Counting							03.08.2010			Fuyu			*/
/********************************************************************************/
//Fuyu: it might matter, so if we can make it NUM_CITY_PLOT times faster
//then lets do it. Moved to before the loop
/* original code
				iWorkersHave += (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD));
*/

//was counting workers twice because missionais are never reset
/* original code
				iWorkersHave += pLoopPlot->plotCount(PUF_isUnitAIType, UNITAI_WORKER, -1, getOwner(), getTeam(), PUF_isNoMission, -1, -1);
*/
				//iWorkersHave += pLoopPlot->plotCount(PUF_isUnitAIType, UNITAI_WORKER, -1, getOwner(), getTeam(), PUF_isNoMissionAI, -1, -1);
				iWorkersHaveNewlyBuilt += pLoopPlot->plotCount(PUF_isUnitAIType, UNITAI_WORKER, -1, getOwner(), getTeam(), PUF_isNoMissionAI, -1, -1);
/********************************************************************************/
/* 	Worker Counting 											END 			*/
/********************************************************************************/

				if (iI != CITY_HOME_PLOT)
				{
					if (pLoopPlot->getImprovementType() == NO_IMPROVEMENT)
					{
						if (pLoopPlot->isBeingWorked())
						{
							if (AI_getBestBuild(iI) != NO_BUILD)
							{
								iUnimprovedWorkedPlotCount++;
							}
							else
							{
								iWorkedUnimprovableCount++;
							}
						}
						else
						{
							if (AI_getBestBuild(iI) != NO_BUILD)
							{
								iUnimprovedUnworkedPlotCount++;
							}
						}
					}
					else
					{
						if (!pLoopPlot->isBeingWorked())
						{
							iImprovedUnworkedPlotCount++;
						}
					}

					for (int iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
						aiYields[iJ] = pLoopPlot->getYield((YieldTypes)iJ);
					}

					if (pLoopPlot->isBeingWorked())
					{
						int iPlotValue = AI_yieldValue(aiYields, NULL, false, false, false, false, true, true);
						iWorstWorkedPlotValue = std::min(iWorstWorkedPlotValue, iPlotValue);
					}
					else
					{
						int iPlotValue = AI_yieldValue(aiYields, NULL, false, false, false, false, true, true);
						iBestUnworkedPlotValue = std::max(iBestUnworkedPlotValue, iPlotValue);
					}
				}
			}
		}
	}
	//specialists?

	iUnimprovedWorkedPlotCount += std::min(iUnimprovedUnworkedPlotCount, iWorkedUnimprovableCount) / 2;

	iWorkersNeeded += 2 * iUnimprovedWorkedPlotCount;

	int iBestPotentialPlotValue = -1;
	if (iWorstWorkedPlotValue != MAX_INT)
	{
		//Add an additional citizen to account for future growth.
		int iBestPlot = -1;
		SpecialistTypes eBestSpecialist = NO_SPECIALIST;

		if (angryPopulation() == 0)
		{
			AI_addBestCitizen(true, true, &iBestPlot, &eBestSpecialist);
		}

		for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
		{
			if (iI != CITY_HOME_PLOT)
			{
				pLoopPlot = getCityIndexPlot(iI);

				if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this && pLoopPlot->getArea() == getArea())
				{
					if (AI_getBestBuild(iI) != NO_BUILD)
					{
						for (int iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
						{
							aiYields[iJ] = pLoopPlot->getYieldWithBuild(m_aeBestBuild[iI], (YieldTypes)iJ, true);
						}

						int iPlotValue = AI_yieldValue(aiYields, NULL, false, false, false, false, true, true);
						ImprovementTypes eImprovement = (ImprovementTypes)GC.getBuildInfo(AI_getBestBuild(iI)).getImprovement();
						if (eImprovement != NO_IMPROVEMENT)
						{
							if ((getImprovementFreeSpecialists(eImprovement) > 0) || (GC.getImprovementInfo(eImprovement).getHappiness() > 0))
							{
								iSpecialCount ++;
							}
						}
						iBestPotentialPlotValue = std::max(iBestPotentialPlotValue, iPlotValue);
					}
				}
			}
		}

		if (iBestPlot != -1)
		{
			setWorkingPlot(iBestPlot, false);
		}
		if (eBestSpecialist != NO_SPECIALIST)
		{
			changeSpecialistCount(eBestSpecialist, -1);
		}

		if (iBestPotentialPlotValue > iWorstWorkedPlotValue)
		{
			iWorkersNeeded += 2;
		}
	}

	iWorkersNeeded += (std::max(0, iUnimprovedWorkedPlotCount - 1) * (GET_PLAYER(getOwnerINLINE()).getCurrentEra())) / 3;

	if (GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble())
	{
		iWorkersNeeded *= 3;
		iWorkersNeeded /= 2;
	}

	if (iWorkersNeeded > 0)
	{
		iWorkersNeeded++;
		iWorkersNeeded = std::max(1, iWorkersNeeded / 3);
	}

	int iSpecialistExtra = std::min((getSpecialistPopulation() - totalFreeSpecialists()), iUnimprovedUnworkedPlotCount);
	iSpecialistExtra -= iImprovedUnworkedPlotCount;

	iWorkersNeeded += std::max(0, 1 + iSpecialistExtra) / 2;

	if (iWorstWorkedPlotValue <= iBestUnworkedPlotValue && iBestUnworkedPlotValue >= iBestPotentialPlotValue)
	{
		iWorkersNeeded /= 2;
	}
	if (angryPopulation(1) > 0)
	{
		iWorkersNeeded++;
		iWorkersNeeded /= 2;
	}

	iWorkersNeeded += (iSpecialCount + 1) / 2;

	iWorkersNeeded = std::max((iUnimprovedWorkedPlotCount + 1) / 2, iWorkersNeeded);
/********************************************************************************/
/* 	Worker Counting						03.08.2010				Fuyu			*/
/********************************************************************************/
	//if( gCityLogLevel >= 3 ) logBBAIForTeam(getTeam(), "	  City %S has %d workers: %d from plotTarget, %d newly built, %d finished soon", getName().GetCString(), (iWorkersHave + iWorkersHaveNewlyBuilt + iWorkersHaveByPlotTargetMissionAI), iWorkersHaveByPlotTargetMissionAI, iWorkersHaveNewlyBuilt, iWorkersHave);
	iWorkersHave += iWorkersHaveNewlyBuilt;
	iWorkersHave += iWorkersHaveByPlotTargetMissionAI;
/********************************************************************************/
/* 	Worker Counting 											END 			*/
/********************************************************************************/

	m_iWorkersNeeded = iWorkersNeeded;
	m_iWorkersHave = iWorkersHave;
}
/********************************************************************************/
/* 	Worker Counting						03.08.2010				Fuyu			*/
/********************************************************************************/
int CvCityAI::AI_workingCityPlotTargetMissionAIs(PlayerTypes ePlayer, MissionAITypes eMissionAI, UnitAITypes eUnitAI, bool bSameAreaOnly) const
{
	PROFILE_FUNC();

	if (this == NULL)
	{
		return 0;
	}

	CvPlayer& kPlayer = GET_PLAYER(ePlayer);
	bool bCanMoveAllTerrain = bSameAreaOnly; //only check if bSameAreaOnly
	int iCorrectUnitAICount;
	int iCount = 0;

	int iLoop = 0;
	for(CvSelectionGroup* pLoopSelectionGroup = kPlayer.firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = kPlayer.nextSelectionGroup(&iLoop))
	{
		CvPlot* pMissionPlot = pLoopSelectionGroup->AI_getMissionAIPlot();

		if (pMissionPlot != NULL)
		{
			if (eMissionAI == NO_MISSIONAI || pLoopSelectionGroup->AI_getMissionAIType() == eMissionAI)
			{
				if (pMissionPlot->getWorkingCity() == this)
				{
					if (eUnitAI == NO_UNITAI && !bSameAreaOnly)
					{
						iCount += pLoopSelectionGroup->getNumUnits();
					}
					else
					{
						CLLNode<IDInfo>* pUnitNode;
						CvUnit* pLoopUnit;

						pUnitNode = pLoopSelectionGroup->headUnitNode();
						if (pUnitNode != NULL)
						{
							CvUnit* pHeadUnit = ::getUnit(pUnitNode->m_data);
							iCorrectUnitAICount = 0;

							if (pHeadUnit != NULL)
							{
								while (pUnitNode != NULL)
								{
									pLoopUnit = ::getUnit(pUnitNode->m_data);
									pUnitNode = pLoopSelectionGroup->nextUnitNode(pUnitNode);

									if (bCanMoveAllTerrain && !(pLoopUnit->canMoveAllTerrain()))
									{
										bCanMoveAllTerrain = false;
									}
									if (eUnitAI == NO_UNITAI || pLoopUnit->AI_getUnitAIType() == eUnitAI)
									{
										iCorrectUnitAICount++;
									}
								}

								if (!bSameAreaOnly || bCanMoveAllTerrain || pHeadUnit->getArea() == pMissionPlot->getArea())
								{
									iCount += iCorrectUnitAICount;
								}
							}
						}
					}
				}
			}
		}
	}

	return iCount;
}
/********************************************************************************/
/* 	Worker Counting 											END 			*/
/********************************************************************************/

BuildingTypes CvCityAI::AI_bestAdvancedStartBuilding(int iPass)
{
	int iFocusFlags = 0;
/************************************************************************************************/
/* Afforess					  Start		 06/07/10											   */
/************************************************************************************************/
	if (iPass >= 0)
	{
		iFocusFlags |= BUILDINGFOCUS_CULTURE;
	}
	if (iPass >= 1)
	{
		iFocusFlags |= BUILDINGFOCUS_FOOD;
	}
	if (iPass >= 2)
	{
		iFocusFlags |= BUILDINGFOCUS_PRODUCTION;
	}
	if (iPass >= 3)
	{
		iFocusFlags |= BUILDINGFOCUS_EXPERIENCE;
	}
	if (iPass >= 4)
	{
		iFocusFlags |= (BUILDINGFOCUS_HAPPY | BUILDINGFOCUS_HEALTHY);
	}
	if (iPass >= 5)
	{
		iFocusFlags |= (BUILDINGFOCUS_GOLD | BUILDINGFOCUS_RESEARCH | BUILDINGFOCUS_MAINTENANCE);
		if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
		{
			iFocusFlags |= BUILDINGFOCUS_ESPIONAGE;
		}
	}

	return AI_bestBuildingThreshold(iFocusFlags, MAX_INT, std::max(0, 25 - iPass * 5));
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
}

void CvCityAI::resync(bool bWrite, ByteBuffer* pBuffer)
{

	CvCity::resync(bWrite, pBuffer);

	RESYNC_INT(bWrite, pBuffer, m_iEmphasizeAvoidGrowthCount);
	RESYNC_INT(bWrite, pBuffer, m_iEmphasizeGreatPeopleCount);
	RESYNC_BOOL(bWrite, pBuffer, m_bAssignWorkDirty);
	RESYNC_BOOL(bWrite, pBuffer, m_bChooseProductionDirty);
	RESYNC_INT(bWrite, pBuffer, m_iEmphasizeAvoidAngryCitizensCount);
	RESYNC_INT(bWrite, pBuffer, m_iEmphasizeAvoidUnhealthyCitizensCount);
	RESYNC_BOOL(bWrite, pBuffer, m_bMilitaryProductionCity);
	RESYNC_INT_WITH_CAST(bWrite, pBuffer, m_routeToCity.eOwner, PlayerTypes);
	RESYNC_INT(bWrite, pBuffer, m_routeToCity.iID);
	RESYNC_INT_ARRAY(bWrite, pBuffer, NUM_YIELD_TYPES, m_aiEmphasizeYieldCount);
	RESYNC_INT_ARRAY(bWrite, pBuffer, NUM_COMMERCE_TYPES, m_aiEmphasizeCommerceCount);
	RESYNC_INT(bWrite, pBuffer, m_bForceEmphasizeCulture);
	RESYNC_INT_ARRAY(bWrite, pBuffer, NUM_CITY_PLOTS, m_aiBestBuildValue);
	RESYNC_INT_ARRAY_WITH_CAST(bWrite, pBuffer, NUM_CITY_PLOTS, m_aeBestBuild, BuildTypes);
	RESYNC_BOOL_ARRAY(bWrite, pBuffer, GC.getNumEmphasizeInfos(), m_pbEmphasize);
	RESYNC_BOOL_ARRAY(bWrite, pBuffer, GC.getNumSpecialistInfos(), m_pbEmphasizeSpecialist);
	RESYNC_INT_ARRAY(bWrite, pBuffer, NUM_YIELD_TYPES, m_aiSpecialYieldMultiplier);
	RESYNC_INT(bWrite, pBuffer, m_iCachePlayerClosenessTurn);
	RESYNC_INT(bWrite, pBuffer, m_iCachePlayerClosenessDistance);
	RESYNC_INT_ARRAY(bWrite, pBuffer, MAX_PLAYERS, m_aiPlayerCloseness);
	RESYNC_INT(bWrite, pBuffer, m_iWorkersNeeded);
	RESYNC_INT(bWrite, pBuffer, m_iWorkersHave);
	RESYNC_INT(bWrite, pBuffer, m_iTempBuildPriority);
	RESYNC_INT(bWrite, pBuffer, m_iBuildPriority);
}

//
//
//
void CvCityAI::read(FDataStreamBase* pStream)
{
	CvTaggedSaveFormatWrapper&	wrapper = CvTaggedSaveFormatWrapper::getSaveFormatWrapper();

	wrapper.AttachToStream(pStream);

	WRAPPER_READ_OBJECT_START(wrapper);

	CvCity::read(pStream);

	uint uiFlag=0;
	WRAPPER_READ(wrapper, "CvCityAI", &uiFlag);	// flags for expansion

	WRAPPER_READ(wrapper, "CvCityAI", &m_iEmphasizeAvoidGrowthCount);
	WRAPPER_READ(wrapper, "CvCityAI", &m_iEmphasizeGreatPeopleCount);
	WRAPPER_READ(wrapper, "CvCityAI", &m_bAssignWorkDirty);
	WRAPPER_READ(wrapper, "CvCityAI", &m_bChooseProductionDirty);
/************************************************************************************************/
/* Afforess					  Start		 02/10/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
	WRAPPER_READ(wrapper, "CvCityAI", &m_iEmphasizeAvoidAngryCitizensCount);
	WRAPPER_READ(wrapper, "CvCityAI", &m_iEmphasizeAvoidUnhealthyCitizensCount);
	WRAPPER_READ(wrapper, "CvCityAI", &m_bMilitaryProductionCity);
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
	WRAPPER_READ(wrapper, "CvCityAI", (int*)&m_routeToCity.eOwner);
	WRAPPER_READ(wrapper, "CvCityAI", &m_routeToCity.iID);

	WRAPPER_READ_ARRAY(wrapper, "CvCityAI", NUM_YIELD_TYPES, m_aiEmphasizeYieldCount);
	WRAPPER_READ_ARRAY(wrapper, "CvCityAI", NUM_COMMERCE_TYPES, m_aiEmphasizeCommerceCount);
	WRAPPER_READ(wrapper, "CvCityAI", &m_bForceEmphasizeCulture);
	WRAPPER_READ_ARRAY(wrapper, "CvCityAI", NUM_CITY_PLOTS, m_aiBestBuildValue);
	WRAPPER_READ_CLASS_ENUM_ARRAY(wrapper, "CvCityAI", REMAPPED_CLASS_TYPE_BUILDS, NUM_CITY_PLOTS, (int*)m_aeBestBuild);
	WRAPPER_READ_ARRAY(wrapper, "CvCityAI", GC.getNumEmphasizeInfos(), m_pbEmphasize);

	//	Old saves incorrectly had this as a plain array - allow either on load - whichever is not present will be skipped
	WRAPPER_READ_ARRAY(wrapper, "CvCityAI", GC.getNumSpecialistInfos(), m_pbEmphasizeSpecialist);
	WRAPPER_READ_CLASS_ARRAY_DECORATED(wrapper, "CvCityAI", REMAPPED_CLASS_TYPE_SPECIALISTS, GC.getNumSpecialistInfos(), m_pbEmphasizeSpecialist, "EmphasizeSpecialist");

	WRAPPER_READ_ARRAY(wrapper, "CvCityAI", NUM_YIELD_TYPES, m_aiSpecialYieldMultiplier);
	WRAPPER_READ(wrapper, "CvCityAI", &m_iCachePlayerClosenessTurn);
	WRAPPER_READ(wrapper, "CvCityAI", &m_iCachePlayerClosenessDistance);
	WRAPPER_READ_ARRAY(wrapper, "CvCityAI", MAX_PLAYERS, m_aiPlayerCloseness);
	WRAPPER_READ(wrapper, "CvCityAI", &m_iWorkersNeeded);
	WRAPPER_READ(wrapper, "CvCityAI", &m_iWorkersHave);
	WRAPPER_READ(wrapper, "CvCityAI", &m_iTempBuildPriority);
	WRAPPER_READ(wrapper, "CvCityAI", &m_iBuildPriority);

	WRAPPER_READ_OBJECT_END(wrapper);
}

//
//
//
void CvCityAI::write(FDataStreamBase* pStream)
{
	CvTaggedSaveFormatWrapper&	wrapper = CvTaggedSaveFormatWrapper::getSaveFormatWrapper();

	wrapper.AttachToStream(pStream);

	WRAPPER_WRITE_OBJECT_START(wrapper);

	CvCity::write(pStream);

	uint uiFlag=0;
	WRAPPER_WRITE(wrapper, "CvCityAI", uiFlag);		// flag for expansion

	WRAPPER_WRITE(wrapper, "CvCityAI", m_iEmphasizeAvoidGrowthCount);
	WRAPPER_WRITE(wrapper, "CvCityAI", m_iEmphasizeGreatPeopleCount);
	WRAPPER_WRITE(wrapper, "CvCityAI", m_bAssignWorkDirty);
	WRAPPER_WRITE(wrapper, "CvCityAI", m_bChooseProductionDirty);
/************************************************************************************************/
/* Afforess					  Start		 02/10/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
	WRAPPER_WRITE(wrapper, "CvCityAI", m_iEmphasizeAvoidAngryCitizensCount);
	WRAPPER_WRITE(wrapper, "CvCityAI", m_iEmphasizeAvoidUnhealthyCitizensCount);
	WRAPPER_WRITE(wrapper, "CvCityAI", m_bMilitaryProductionCity);
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
	WRAPPER_WRITE(wrapper, "CvCityAI", m_routeToCity.eOwner);
	WRAPPER_WRITE(wrapper, "CvCityAI", m_routeToCity.iID);

	WRAPPER_WRITE_ARRAY(wrapper, "CvCityAI", NUM_YIELD_TYPES, m_aiEmphasizeYieldCount);
	WRAPPER_WRITE_ARRAY(wrapper, "CvCityAI", NUM_COMMERCE_TYPES, m_aiEmphasizeCommerceCount);
	WRAPPER_WRITE(wrapper, "CvCityAI", m_bForceEmphasizeCulture);
	WRAPPER_WRITE_ARRAY(wrapper, "CvCityAI", NUM_CITY_PLOTS, m_aiBestBuildValue);
	WRAPPER_WRITE_CLASS_ENUM_ARRAY(wrapper, "CvCityAI", REMAPPED_CLASS_TYPE_BUILDS, NUM_CITY_PLOTS, (int*)m_aeBestBuild);
	WRAPPER_WRITE_ARRAY(wrapper, "CvCityAI", GC.getNumEmphasizeInfos(), m_pbEmphasize);
	WRAPPER_WRITE_CLASS_ARRAY_DECORATED(wrapper, "CvCityAI", REMAPPED_CLASS_TYPE_SPECIALISTS, GC.getNumSpecialistInfos(), m_pbEmphasizeSpecialist, "EmphasizeSpecialist");
	WRAPPER_WRITE_ARRAY(wrapper, "CvCityAI", NUM_YIELD_TYPES, m_aiSpecialYieldMultiplier);
	WRAPPER_WRITE(wrapper, "CvCityAI", m_iCachePlayerClosenessTurn);
	WRAPPER_WRITE(wrapper, "CvCityAI", m_iCachePlayerClosenessDistance);
	WRAPPER_WRITE_ARRAY(wrapper, "CvCityAI", MAX_PLAYERS, m_aiPlayerCloseness);
	WRAPPER_WRITE(wrapper, "CvCityAI", m_iWorkersNeeded);
	WRAPPER_WRITE(wrapper, "CvCityAI", m_iWorkersHave);
	WRAPPER_WRITE(wrapper, "CvCityAI", m_iTempBuildPriority);
	WRAPPER_WRITE(wrapper, "CvCityAI", m_iBuildPriority);

	WRAPPER_WRITE_OBJECT_END(wrapper);
}


/************************************************************************************************/
/* RevDCM				  Start		 5/2/09												 */
/*																							  */
/* Inquisitions																				 */
/************************************************************************************************/
bool CvCityAI::AI_trainInquisitor()
{
	CvPlayerAI& kPlayerAI = GET_PLAYER(getOwnerINLINE());

	if( GC.getGameINLINE().isOption(GAMEOPTION_NO_INQUISITIONS)
	|| !kPlayerAI.isInquisitionConditions() )
	{
		return false;
	}
	if(!kPlayerAI.hasInquisitionTarget())
	{
		return false;
	}
	if(getPopulation() < GC.getDefineINT("MIN_POP_CONSIDER_INQUISITORS"))
	{
		return false;
	}

	int iUnitCost = MAX_INT;
	int iBuildOdds = (kPlayerAI.hasStateReligionHolyCity() ? 10 : 0);
	UnitTypes eLoopUnit;
	UnitTypes eBestUnit = NO_UNIT;
	CvCivilizationInfo& kCivilization = GC.getCivilizationInfo(kPlayerAI.getCivilizationType());
	int iI;

	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		eLoopUnit = (UnitTypes)kCivilization.getCivilizationUnits((UnitClassTypes)iI);
		if (eLoopUnit == NO_UNIT || !GC.getUnitInfo(eLoopUnit).isInquisitor())
		{
			continue;
		}
		if (canTrain(eLoopUnit, false, false))
		{
			if (GC.getUnitInfo(eLoopUnit).getProductionCost() < iUnitCost)
			{
				eBestUnit = eLoopUnit;
				iUnitCost = GC.getUnitInfo(eBestUnit).getProductionCost();
			}
		}
	}
	if (eBestUnit == NO_UNIT)
	{
		return false;
	}

	if(kPlayerAI.isPushReligiousVictory())
	{
		iBuildOdds += 30;
	} else if(kPlayerAI.isConsiderReligiousVictory())
	{
		iBuildOdds += 7;
	}
	if(kPlayerAI.countCityReligionRevolts() > 3)
	{
		iBuildOdds += kPlayerAI.countCityReligionRevolts() * 20;
	}else
	{
		iBuildOdds += kPlayerAI.countCityReligionRevolts() * 10;
	}
	if(kPlayerAI.isNonStateReligionCommerce())
	{
		iBuildOdds /= 2;
	}

	if (iBuildOdds > 0)
	{
		if (getCitySorenRandNum(100, "AI choose Inquisitor") < iBuildOdds)
		{
			setProposedOrder(ORDER_TRAIN, eBestUnit);
			return true;
		}
	}

	return false;
}
/************************************************************************************************/
/* Inquisitions						 END														*/
/************************************************************************************************/
/************************************************************************************************/
/* Afforess					  Start		 05/28/10											   */
/*																							  */
/*																							  */
/************************************************************************************************/
int CvCityAI::AI_getEmphasizeAvoidAngryCitizensCount()
{
	return m_iEmphasizeAvoidAngryCitizensCount;
}

bool CvCityAI::AI_isEmphasizeAvoidAngryCitizens()
{
	return (AI_getEmphasizeAvoidAngryCitizensCount() > 0);
}

int CvCityAI::AI_getEmphasizeAvoidUnhealthyCitizensCount()
{
	return m_iEmphasizeAvoidUnhealthyCitizensCount;
}

bool CvCityAI::AI_isEmphasizeAvoidUnhealthyCitizens()
{
	return (AI_getEmphasizeAvoidUnhealthyCitizensCount() > 0);
}

bool CvCityAI::AI_buildCaravan()
{
	CvCity* pLoopCity;
	int iI;
	int iNumCities = GET_PLAYER(getOwnerINLINE()).getNumCities();
	int iAveProduction = 0;
	int iBestHurry = 0;
	UnitTypes eBestUnit = NO_UNIT;
	int iLoop = 0;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		iAveProduction += pLoopCity->getYieldRate(YIELD_PRODUCTION);
	}
	iAveProduction /= iNumCities;
	if (((2 * getYieldRate(YIELD_PRODUCTION)) / 3) > iAveProduction)
	{
		for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
		{
			if (GC.getUnitInfo((UnitTypes)iI).getBaseHurry() > iBestHurry)
			{
				if (canTrain((UnitTypes)iI, false, true, false))
				{
					iBestHurry = GC.getUnitInfo((UnitTypes)iI).getBaseHurry();
					eBestUnit = (UnitTypes)iI;
				}
			}
		}
		if (eBestUnit != NO_UNIT && GET_PLAYER(getOwnerINLINE()).getUnitClassCountPlusMaking((UnitClassTypes)GC.getUnitInfo(eBestUnit).getUnitClassType()) < 4)
		{
			int iOdds = 100;
			if (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI))
				iOdds *= 2;
			iOdds /= iNumCities;
			iOdds *= iAveProduction;
			iOdds /= std::max(1, getYieldRate(YIELD_PRODUCTION));
			iOdds = std::max(1, iOdds);
			if (getCitySorenRandNum(iOdds, "Caravan Production") == 0)
			{
				pushOrder(ORDER_TRAIN, eBestUnit, -1, false, false, false);
				//GC.getGameINLINE().logMsg("City %S built a caravan", getName().GetCString());
				return true;
			}
		}
	}
	return false;
}

int CvCityAI::AI_getPromotionValue(PromotionTypes ePromotion) const
{
	int iI;
	//Increased based on the number of units that can recieve this promotion
	int iMultiplier = 100;
	int iValue = 0;
	UnitTypes eLoopUnit;
	int iPromotionCount = 0;
	int iCanTrainCount = 0;
	//TB SubCombat Mod Begin
	int iJ;
	UnitCombatTypes eSubCombatType;
	//TB SubCombat Mod End
	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		eLoopUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI);
		if (eLoopUnit != NO_UNIT)
		{
/************************************************************************************************/
/* Afforess					  Start		 6/11/11												*/
/*																							  */
/*																							  */
/************************************************************************************************/
		if ((GC.getUnitInfo(eLoopUnit).getCombat() +
		GET_TEAM(getTeam()).getUnitClassStrengthChange((UnitClassTypes)GC.getUnitInfo(eLoopUnit).getUnitClassType())) > 0)
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
			{
				if (GC.getUnitInfo(eLoopUnit).getUnitCombatType() != NO_UNITCOMBAT)
				{
					if (canTrain(eLoopUnit))
					{
						iCanTrainCount++;
						if (GC.getPromotionInfo(ePromotion).getUnitCombat(GC.getUnitInfo(eLoopUnit).getUnitCombatType()))
						{
							iPromotionCount++;
						}
						//TB SubCombat Mod Begin
						for (iJ = 0; iJ < GC.getUnitInfo(eLoopUnit).getNumSubCombatTypes(); iJ++)
						{
							eSubCombatType = ((UnitCombatTypes)GC.getUnitInfo(eLoopUnit).getSubCombatType(iJ));
							if (GC.getPromotionInfo(ePromotion).getUnitCombat(eSubCombatType))
							{
								iPromotionCount++;
							}
						}

						//TB SubCombat Mod End
					}
				}
			}
		}
	}
	if (iCanTrainCount == 0 || iPromotionCount == 0)
	{
		return 0; //Avoid division by 0, and if the city can't train units or the promotion never applies, it's useless
	}
	iMultiplier *= iPromotionCount;
	iMultiplier /= iCanTrainCount;
	iMultiplier *= (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0) ? 3 : 2;

	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		eLoopUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI);
		if (eLoopUnit != NO_UNIT)
		{
			if (GC.getUnitInfo(eLoopUnit).getCombat() > 0)
			{
				if (GC.getUnitInfo(eLoopUnit).getUnitCombatType() != NO_UNITCOMBAT)
				{
					if (GC.getPromotionInfo(ePromotion).getUnitCombat(GC.getUnitInfo(eLoopUnit).getUnitCombatType()))
					{
						if (canTrain(eLoopUnit))
						{
							iValue += GET_PLAYER(getOwnerINLINE()).AI_promotionValue(ePromotion, eLoopUnit);
						}
					}
				}
			}
		}
	}
	//iValue becomes the average value of the promotion
	iValue /= iPromotionCount;

	iValue *= iMultiplier;
	iValue /= 100;

	return iValue;

}

SpecialistTypes CvCity::getBestSpecialist(int iExtra)
{
	int iI;
	int iValue;
	int iBestValue = 0;
	SpecialistTypes eBestSpecialist = NO_SPECIALIST;
	for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		if (isSpecialistValid((SpecialistTypes)iI, iExtra))
		{
			iValue = AI_specialistValue((SpecialistTypes)iI, false, false);
			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				eBestSpecialist = (SpecialistTypes)iI;
			}
		}
	}

	return eBestSpecialist;
}

int CvCityAI::AI_calculateActualImprovementHealth(ImprovementTypes eImprovement) const
{
	if (eImprovement == NO_IMPROVEMENT) return 0;
	int iHealthPercent = GC.getImprovementInfo(eImprovement).getHealthPercent();
	int iBadHealthPercent = abs(getImprovementBadHealth());
	int iGoodHealthPercent = getImprovementGoodHealth();
	int iHealthChange;
	if (iHealthPercent < 0)
	{
		iHealthChange = iBadHealthPercent - iHealthPercent;
		iHealthChange /= 100;
		return -(iHealthChange - (iBadHealthPercent / 100));
	}
	else
	{
		iHealthChange = iGoodHealthPercent + iHealthPercent;
		iHealthChange /= 100;
		return iHealthChange - (iGoodHealthPercent / 100);
	}
}

bool CvCityAI::AI_isMilitaryProductionCity() const
{
	return m_bMilitaryProductionCity;
}

void CvCityAI::AI_setMilitaryProductionCity(bool bNewVal)
{
	if (bNewVal != AI_isMilitaryProductionCity())
	{
		m_bMilitaryProductionCity = bNewVal;
		AI_setChooseProductionDirty(true);
	}
}

int CvCityAI::AI_getMilitaryProductionRateRank() const
{
	PROFILE_FUNC();

	int iRate = AI_getMilitaryProductionRate(); // f1rpo: Moved into new function
	int iRank = 1;

	int iLoop = 0;
	for (CvCity const* pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		/*	f1rpo: Replacing code that was repeating the calculation from above except
			for the evaluation of specialists - which I suppose was omitted by accident. */
		int iLoopRate = pLoopCity->AI().AI_getMilitaryProductionRate();
		if ((iLoopRate > iRate) ||
			((iLoopRate == iRate) && (pLoopCity->getID() < getID())))
		{
			iRank++;
		}
	}
	return iRank;
}
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
// f1rpo: Cut from AI_getMilitaryProductionRateRank
int CvCityAI::AI_getMilitaryProductionRate() const
{
	int iRate = getPopulation() - getYieldRate(YIELD_COMMERCE);
	int iProductionRate = getYieldRate(YIELD_PRODUCTION);
	/*	<f1rpo> Want this taken into account for the evaluation of nuke targets -
		but seems like a good idea in general. */
	iProductionRate *= 100 + getMilitaryProductionModifier();
	iProductionRate /= 100; // </f1rpo>
	iRate += iProductionRate;
	FOR_EACH_ENUM(Specialist)
	{
		iRate += getSpecialistCount(eLoopSpecialist)
				* GC.getInfo(eLoopSpecialist).getExperience();
	}
	return iRate;
}

//	KOSHLING Mod - pre-calculate and cache building values for all focuses
//
//	Simple class to hold the calculated values for one building type and city
#ifdef _DEBUG
//#define VALIDATE_BUILDING_CACHE_CONSISTENCY
#endif

class FocusValueSet
{
public:
	FocusValueSet()
	{
		for(int i = 0; i < NUM_ALL_BUILDINGFOCUS_FLAGS; i++)
		{
			m_focusValues[i] = 0;
		}
	}

	void AccumulateTo(int iFocusIndex, int value)
	{
		FAssertMsg( iFocusIndex < NUM_ALL_BUILDINGFOCUS_FLAGS, "iFocusIndex out of range" );
		m_focusValues[iFocusIndex] += value;
	}

	void AccumulateToAny(int value)
	{
		AccumulateTo(BUILDINGFOCUSINDEX_ANY, value);
	}

	void Accumulate(int iFocusIndex, int value, bool toAny)
	{
		if ( toAny )
		{
			AccumulateToAny(value);
		}
		else
		{
			AccumulateTo(iFocusIndex, value);
		}
	}

	void SetValue(int iFocusIndex, int value)
	{
		m_focusValues[iFocusIndex] = value;
	}

	void SetValueAll(int value)
	{
		for( int i = 0; i < NUM_ALL_BUILDINGFOCUS_FLAGS; i++ )
		{
			m_focusValues[i] = value;
		}
	}

	int	m_focusValues[NUM_ALL_BUILDINGFOCUS_FLAGS];
};

class OneBuildingValueCache
{
public:
	OneBuildingValueCache()
	{
	}

	void AccumulateTo(int iFocusIndex, int value, bool isThresholdSet)
	{
		MEMORY_TRACK_EXEMPT();

		if ( isThresholdSet )
		{
			m_thresholdSet.AccumulateTo(iFocusIndex, value);
		}
		else
		{
			m_noThresholdSet.AccumulateTo(iFocusIndex, value);
		}
	}

	void AccumulateToAny(int value, bool isThresholdSet)
	{
		AccumulateTo(BUILDINGFOCUSINDEX_ANY, value, isThresholdSet);
	}

	void Accumulate(int iFocusIndex, int value)
	{
		AccumulateToAny(value, false);
		AccumulateTo(iFocusIndex, value, true);
	}

	void SetValue(int iFocusIndex, int value, bool isThresholdSet)
	{
		if ( isThresholdSet )
		{
			m_thresholdSet.SetValue(iFocusIndex, value);
		}
		else
		{
			m_noThresholdSet.SetValue(iFocusIndex, value);
		}
	}

	void SetValueAll(int value, bool isThresholdSet)
	{
		for( int i = 0; i < NUM_ALL_BUILDINGFOCUS_FLAGS; i++ )
		{
			SetValue(i, value, isThresholdSet);
		}
	}

	FocusValueSet	m_thresholdSet;
	FocusValueSet	m_noThresholdSet;
};

class BuildingValueCache
{
public:
	BuildingValueCache(CvCityAI* cachedCity)
	{
		m_iCachedFlags	= 0;
		m_bIncomplete	= false;
	}

	virtual ~BuildingValueCache()
	{
		std::map<int,OneBuildingValueCache*>::const_iterator itr;

		for(itr = m_buildingValues.begin(); itr != m_buildingValues.end(); ++itr)
		{
			delete itr->second;
		}
	}

	bool HasValues(BuildingTypes eType)
	{
		return (m_buildingValues.find(eType) != m_buildingValues.end());
	}

	int GetValue(BuildingTypes eType, int iFocusFlags, int threshold, bool bMaximizeFlaggedValue)
	{
		int iValueThreshold = 0;
		int iValueNoThreshold = 0;

		std::map<int,OneBuildingValueCache*>::const_iterator itr = m_buildingValues.find(eType);
		if ( itr == m_buildingValues.end() )
		{
			//	No cached value
			return -1;
		}
		else
		{
			OneBuildingValueCache* valueCache = itr->second;

			for(int i = 0; i < NUM_REAL_BUILDINGFOCUS_FLAGS; i++)
			{
				if ( (iFocusFlags & 1 << i) != 0 )
				{
					iValueThreshold += valueCache->m_thresholdSet.m_focusValues[i];
					iValueNoThreshold += valueCache->m_noThresholdSet.m_focusValues[i];
				}
			}

			//	Add in pseudo-flag contributions
			if ( (iFocusFlags & (BUILDINGFOCUS_GOLD | BUILDINGFOCUS_RESEARCH)) != 0 )
			{
				iValueThreshold += valueCache->m_thresholdSet.m_focusValues[BUILDINGFOCUSINDEX_GOLDANDRESEARCH];
				iValueNoThreshold += valueCache->m_noThresholdSet.m_focusValues[BUILDINGFOCUSINDEX_GOLDANDRESEARCH];
			}
			if ( (iFocusFlags & (BUILDINGFOCUS_GOLD | BUILDINGFOCUS_MAINTENANCE)) != 0 )
			{
				iValueThreshold += valueCache->m_thresholdSet.m_focusValues[BUILDINGFOCUSINDEX_GOLDANDMAINTENANCE];
				iValueNoThreshold += valueCache->m_noThresholdSet.m_focusValues[BUILDINGFOCUSINDEX_GOLDANDMAINTENANCE];
			}

			int iPass1Value = iValueThreshold + valueCache->m_thresholdSet.m_focusValues[BUILDINGFOCUSINDEX_ANY];
			if ( iValueThreshold < threshold )
			{
				iPass1Value = 0;
			}

			int iPass2Value;
			if ( (iFocusFlags & BUILDINGFOCUS_CONSIDER_ANY) != 0 )
			{
				iPass2Value = valueCache->m_noThresholdSet.m_focusValues[BUILDINGFOCUSINDEX_ANY];
			}
			else if ( iPass1Value > 0 )
			{
				iPass2Value = iValueNoThreshold + valueCache->m_noThresholdSet.m_focusValues[BUILDINGFOCUSINDEX_ANY];
			}
			else
			{
				iPass2Value = 0;
			}

			//	If we are really seeking an answer specifically on the values from the specific flags
			//	then the rest is just a tie-breaker so dramatically boost the value of the flag-specific
			//	stuff from pass1
			if ( bMaximizeFlaggedValue )
			{
				iPass1Value *= 20;
			}
			else
			{
				//	Even when not maximizing with all the extra effects on buildings pass1 was getting a
				//	bit dominated by pass2 so provide a little extra pas1 weight anyway
				iPass1Value *= 2;
			}

			return std::max(0, iPass1Value + iPass2Value);
		}
	}

	OneBuildingValueCache* GetOneBuildingCache(int eBuilding)
	{
		OneBuildingValueCache* result;

		std::map<int,OneBuildingValueCache*>::const_iterator itr = m_buildingValues.find(eBuilding);
		if ( itr == m_buildingValues.end() )
		{
			MEMORY_TRACK_EXEMPT();

			//	New entry
			result = new OneBuildingValueCache();

			m_buildingValues[eBuilding] = result;
		}
		else
		{
			result = itr->second;
		}

		return result;
	}

	//	One city is cached at a time
	int			m_iCachedFlags;
	bool		m_bIncomplete;
	std::map<int,OneBuildingValueCache*>	m_buildingValues;
};

static bool buildingHasTradeRouteValue(BuildingTypes eBuilding)
{
	CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);

	return (kBuilding.getTradeRoutes() > 0 ||
			kBuilding.getCoastalTradeRoutes() > 0 ||
			kBuilding.getGlobalTradeRoutes() > 0 ||
			kBuilding.getTradeRouteModifier() > 0 ||
			kBuilding.getForeignTradeRouteModifier() > 0);
}

bool CvCityAI::buildingMayHaveAnyValue(BuildingTypes eBuilding, int iFocusFlags)
{
	PROFILE_FUNC();

	CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);

	if ( kBuilding.isAutoBuild() )
	{
		return false;
	}

	if ( (iFocusFlags & BUILDINGFOCUS_CONSIDER_ANY) != 0 )
	{
		return true;
	}

	bool bIsWonder = isWorldWonderClass((BuildingClassTypes)kBuilding.getBuildingClassType()) ||
					 isTeamWonderClass((BuildingClassTypes)kBuilding.getBuildingClassType()) ||
					  isNationalWonderClass((BuildingClassTypes)kBuilding.getBuildingClassType()) ||
					   isLimitedWonderClass((BuildingClassTypes)kBuilding.getBuildingClassType());

	if ( (iFocusFlags & ~BUILDINGFOCUS_WONDEROK) == BUILDINGFOCUS_WORLDWONDER )
	{
		return bIsWonder;
	}

	if ( (iFocusFlags & BUILDINGFOCUS_WONDEROK) != 0 )
	{
		if ( bIsWonder )
		{
			return true;
		}
	}

	if ( (iFocusFlags & BUILDINGFOCUS_BIGCULTURE) != 0 )
	{
		iFocusFlags |= BUILDINGFOCUS_CULTURE;
	}

	//	Some things cause us to consider the building in any role
	if ( kBuilding.getFreeSpecialist() > 0 ||
		 kBuilding.getAreaFreeSpecialist() > 0 ||
		 kBuilding.getGlobalFreeSpecialist() > 0 ||
		 (kBuilding.getPropertyManipulators() != NULL && kBuilding.getPropertyManipulators()->getNumSources() > 0) ||
		 kBuilding.getProperties()->getNumProperties() > 0 )
	{
		return true;
	}
	if (kBuilding.isPower() || ((kBuilding.getPowerBonus() != NO_BONUS) && hasBonus((BonusTypes)(kBuilding.getPowerBonus()))) || kBuilding.isAreaCleanPower())
	{
		return true;
	}
	if ( kBuilding.getFoundsCorporation() != NO_CORPORATION || kBuilding.getGlobalReligionCommerce() > 0 || kBuilding.getGlobalCorporationCommerce() > 0 )
	{
		return true;
	}

	bool buildingModifiesGenericYields =
			 (kBuilding.getTechYieldChange(NO_TECH, NO_COMMERCE) > 0 ||
			  kBuilding.getBonusYieldModifier(NO_BONUS, NO_COMMERCE) > 0 ||
			  kBuilding.getBonusYieldChanges(NO_BONUS, NO_COMMERCE) > 0 ||
			  kBuilding.getVicinityBonusYieldChanges(NO_BONUS, NO_COMMERCE) > 0);

	bool buildingModifiesCommerceYields =
			 (buildingModifiesGenericYields ||
			  kBuilding.getYieldModifier(YIELD_COMMERCE) > 0 ||
			  GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, YIELD_COMMERCE) > 0 ||
			  kBuilding.getPowerYieldModifier(YIELD_COMMERCE) > 0 ||
			  kBuilding.getSeaPlotYieldChange(YIELD_COMMERCE) > 0 ||
			  kBuilding.getRiverPlotYieldChange(YIELD_COMMERCE) > 0 ||
			  kBuilding.getYieldChange(YIELD_COMMERCE) > 0 ||
			  GET_TEAM(getTeam()).getBuildingYieldChange(eBuilding, YIELD_COMMERCE) > 0);

	if ( (iFocusFlags & BUILDINGFOCUS_FOOD) != 0 )
	{
		if ( buildingModifiesGenericYields ||
			 buildingHasTradeRouteValue(eBuilding) ||
			 kBuilding.getFoodKept() > 0 ||
			 kBuilding.getSeaPlotYieldChange(YIELD_FOOD) > 0 ||
			 GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, YIELD_FOOD) > 0 ||
			 kBuilding.getYieldChange(YIELD_FOOD) > 0 ||
			 GET_TEAM(getTeam()).getBuildingYieldChange(eBuilding, YIELD_FOOD) > 0 ||
			 kBuilding.getYieldModifier(YIELD_FOOD) > 0 ||
			 kBuilding.getRiverPlotYieldChange(YIELD_FOOD) > 0 ||
			 kBuilding.getHurryCostModifier() < 0)
		{
			return true;
		}
	}
	if ( (iFocusFlags & BUILDINGFOCUS_PRODUCTION) != 0 )
	{
		if ( buildingModifiesGenericYields ||
			 buildingHasTradeRouteValue(eBuilding) ||
			 kBuilding.getYieldModifier(YIELD_PRODUCTION) > 0 ||
			 GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, YIELD_PRODUCTION) > 0 ||
			 kBuilding.getPowerYieldModifier(YIELD_PRODUCTION) > 0 ||
			 kBuilding.getSeaPlotYieldChange(YIELD_PRODUCTION) > 0 ||
			 kBuilding.getRiverPlotYieldChange(YIELD_PRODUCTION) > 0 ||
			 kBuilding.getYieldChange(YIELD_PRODUCTION) > 0 ||
			 GET_TEAM(getTeam()).getBuildingYieldChange(eBuilding, YIELD_PRODUCTION) > 0 ||
			 kBuilding.getHurryCostModifier() < 0 ||
			 kBuilding.getMilitaryProductionModifier() > 0 ||
			 kBuilding.getSpaceProductionModifier() > 0 ||
			 kBuilding.getGlobalSpaceProductionModifier() > 0 ||
			 kBuilding.getDomainProductionModifier(NO_DOMAIN) > 0)
		{
			return true;
		}
	}
	if ( (iFocusFlags & BUILDINGFOCUS_GOLD) != 0 )
	{
		iFocusFlags |= BUILDINGFOCUS_MAINTENANCE;

		if ( buildingModifiesCommerceYields ||
			 buildingHasTradeRouteValue(eBuilding) ||
			 kBuilding.getCommerceChange(COMMERCE_GOLD) > 0 ||
			 kBuilding.getCommerceModifier(COMMERCE_GOLD) > 0 ||
			 kBuilding.getGlobalCommerceModifier(COMMERCE_GOLD) > 0 ||
			 kBuilding.getSpecialistExtraCommerce(COMMERCE_GOLD) > 0 ||
			 kBuilding.getStateReligionCommerce(COMMERCE_GOLD) > 0 ||
			 kBuilding.getObsoleteSafeCommerceChange(COMMERCE_GOLD) > 0 ||
			 kBuilding.getCommerceAttacks(COMMERCE_GOLD) > 0 ||
			 kBuilding.getTechCommerceChange(NO_TECH, COMMERCE_GOLD) > 0 ||
			 kBuilding.getTechCommerceModifier(NO_TECH, COMMERCE_GOLD) > 0 )
		{
			return true;
		}
	}
	if ( (iFocusFlags & BUILDINGFOCUS_RESEARCH) != 0 )
	{
		if ( buildingModifiesCommerceYields ||
			 buildingHasTradeRouteValue(eBuilding) ||
			 kBuilding.getCommerceChange(COMMERCE_RESEARCH) > 0 ||
			 kBuilding.getCommerceModifier(COMMERCE_RESEARCH) > 0 ||
			 kBuilding.getGlobalCommerceModifier(COMMERCE_RESEARCH) > 0 ||
			 kBuilding.getSpecialistExtraCommerce(COMMERCE_RESEARCH) > 0 ||
			 kBuilding.getStateReligionCommerce(COMMERCE_RESEARCH) > 0 ||
			 kBuilding.getObsoleteSafeCommerceChange(COMMERCE_RESEARCH) > 0 ||
			 kBuilding.getCommerceAttacks(COMMERCE_RESEARCH) > 0 ||
			 kBuilding.getTechCommerceChange(NO_TECH, COMMERCE_RESEARCH) > 0 ||
			 kBuilding.getTechCommerceModifier(NO_TECH, COMMERCE_RESEARCH) > 0)
		{
			return true;
		}
	}
	if ( (iFocusFlags & BUILDINGFOCUS_CULTURE) != 0 )
	{
		if ( buildingModifiesCommerceYields ||
			 buildingHasTradeRouteValue(eBuilding) ||
			 kBuilding.getCommerceChange(COMMERCE_CULTURE) > 0 ||
			 kBuilding.getCommerceModifier(COMMERCE_CULTURE) > 0 ||
			 kBuilding.getGlobalCommerceModifier(COMMERCE_CULTURE) > 0 ||
			 kBuilding.getSpecialistExtraCommerce(COMMERCE_CULTURE) > 0 ||
			 kBuilding.getStateReligionCommerce(COMMERCE_CULTURE) > 0 ||
			 kBuilding.getObsoleteSafeCommerceChange(COMMERCE_CULTURE) > 0 ||
			 kBuilding.getCommerceAttacks(COMMERCE_CULTURE) > 0 ||
			 kBuilding.getTechCommerceChange(NO_TECH, COMMERCE_CULTURE) > 0 ||
			 kBuilding.getTechCommerceModifier(NO_TECH, COMMERCE_CULTURE) > 0)
		{
			return true;
		}
	}
	if ( (iFocusFlags & BUILDINGFOCUS_DEFENSE) != 0 )
	{
		if ( kBuilding.getDefenseModifier() > 0 ||
			 kBuilding.getBombardDefenseModifier() > 0 ||
			 kBuilding.getAllCityDefenseModifier() >  0 ||
			 kBuilding.isNeverCapture() ||
			 kBuilding.isNukeImmune() ||
			 kBuilding.isZoneOfControl() ||
			 kBuilding.getLineOfSight() > 0 ||
			 kBuilding.getUnitCombatExtraStrength(NO_UNITCOMBAT) > 0 ||
			 kBuilding.getAdjacentDamagePercent() > 0 ||
			 kBuilding.isProtectedCulture() ||
			 kBuilding.getOccupationTimeModifier() > 0 ||
			 kBuilding.getNoEntryDefenseLevel() > 0 ||
			 kBuilding.getNumUnitFullHeal() > 0 ||
			 kBuilding.isAreaBorderObstacle() )
		{
			return true;
		}
	}
	if ( (iFocusFlags & BUILDINGFOCUS_HAPPY) != 0 )
	{
		if ( kBuilding.getHappiness() > 0 ||
			 kBuilding.getAreaHappiness() > 0 ||
			 kBuilding.getGlobalHappiness() > 0 ||
			 kBuilding.getStateReligionHappiness() > 0 ||
			 kBuilding.isNoUnhappiness() ||
			 kBuilding.getWarWearinessModifier() < 0 ||
			 kBuilding.getGlobalWarWearinessModifier() < 0 ||
			 kBuilding.getCommerceHappiness(NO_COMMERCE) > 0 ||
			 kBuilding.getBonusHappinessChanges(NO_BONUS) > 0 ||
			 kBuilding.getBuildingHappinessChanges(NO_BUILDING) > 0 ||
			 kBuilding.getTechHappinessChanges(NO_TECH) > 0 ||
			 GET_PLAYER(getOwnerINLINE()).getExtraBuildingHappiness(eBuilding) > 0)
		{
			return true;
		}
	}
	if ( (iFocusFlags & BUILDINGFOCUS_HEALTHY) != 0 )
	{
		if ( kBuilding.getHealth() > 0 ||
			 kBuilding.getAreaHealth() > 0 ||
			 kBuilding.getGlobalHealth() > 0 ||
			 kBuilding.getStateReligionHealth() > 0 ||
			 kBuilding.isNoUnhealthyPopulation() ||
			 kBuilding.isBuildingOnlyHealthy() ||
			 kBuilding.getBonusHealthChanges(NO_BONUS) > 0 ||
			 kBuilding.getTechHealthChanges(NO_TECH) > 0 ||
			 kBuilding.getHealthPercentPerPopulation() > 0 ||
			 GET_PLAYER(getOwnerINLINE()).getExtraBuildingHealth(eBuilding) > 0)
		{
			return true;
		}
	}
	if ( (iFocusFlags & BUILDINGFOCUS_DOMAINSEA) != 0 )
	{
		iFocusFlags |= BUILDINGFOCUS_EXPERIENCE;

		if ( kBuilding.getDomainProductionModifier(DOMAIN_SEA) > 0 ||
			 kBuilding.getDomainFreeExperience(DOMAIN_SEA) > 0)
		{
			return true;
		}
	}
	if ( (iFocusFlags & BUILDINGFOCUS_EXPERIENCE) != 0 )
	{
		if ( kBuilding.getFreeExperience() > 0 ||
			 kBuilding.getGlobalFreeExperience() > 0 ||
			 kBuilding.getFreePromotion() != NO_PROMOTION ||
			 kBuilding.getUnitCombatFreeExperience(NO_UNITCOMBAT) > 0 ||
			 kBuilding.getDomainFreeExperience(NO_DOMAIN) > 0 ||
			 kBuilding.getFreePromotion_2() != NO_PROMOTION ||
			 kBuilding.getFreePromotion_3() != NO_PROMOTION ||
			 kBuilding.isApplyFreePromotionOnMove() ||
			 kBuilding.EnablesUnits() )
		{
			return true;
		}
	}
	if ( (iFocusFlags & BUILDINGFOCUS_MAINTENANCE) != 0 )
	{
		if ( kBuilding.getMaintenanceModifier() < 0 ||
			 kBuilding.getGlobalMaintenanceModifier() < 0 ||
			 kBuilding.getAreaMaintenanceModifier() < 0 ||
			 kBuilding.getOtherAreaMaintenanceModifier() < 0 ||
			 kBuilding.getDistanceMaintenanceModifier() < 0 ||
			 kBuilding.getNumCitiesMaintenanceModifier() < 0 ||
			 kBuilding.getCoastalDistanceMaintenanceModifier() < 0 ||
			 kBuilding.getConnectedCityMaintenanceModifier() )
		{
			return true;
		}
	}
	if ( (iFocusFlags & BUILDINGFOCUS_SPECIALIST) != 0 )
	{
		if ( kBuilding.getSpecialistCount(NO_SPECIALIST) > 0 ||
			 kBuilding.getFreeSpecialistCount(NO_SPECIALIST) > 0 ||
			 kBuilding.getImprovementFreeSpecialist(NO_IMPROVEMENT) > 0 ||
			 kBuilding.getTechSpecialistChange(NO_TECH, NO_SPECIALIST) > 0)
		{
			return true;
		}
	}
	if ( (iFocusFlags & BUILDINGFOCUS_ESPIONAGE) != 0 )
	{
		if ( kBuilding.getEspionageDefenseModifier() > 0 )
		{
			return true;
		}
		if ( buildingModifiesCommerceYields ||
			 buildingHasTradeRouteValue(eBuilding) ||
			 kBuilding.getCommerceChange(COMMERCE_ESPIONAGE) > 0 ||
			 kBuilding.getCommerceModifier(COMMERCE_ESPIONAGE) > 0 ||
			 kBuilding.getGlobalCommerceModifier(COMMERCE_ESPIONAGE) > 0 ||
			 kBuilding.getSpecialistExtraCommerce(COMMERCE_ESPIONAGE) > 0 ||
			 kBuilding.getStateReligionCommerce(COMMERCE_ESPIONAGE) > 0 ||
			 kBuilding.getObsoleteSafeCommerceChange(COMMERCE_ESPIONAGE) > 0 ||
			 kBuilding.getCommerceAttacks(COMMERCE_ESPIONAGE) > 0 ||
			 kBuilding.getTechCommerceChange(NO_TECH, COMMERCE_ESPIONAGE) > 0 ||
			 kBuilding.getTechCommerceModifier(NO_TECH, COMMERCE_ESPIONAGE) > 0)
		{
			return true;
		}
	}
	if ( (iFocusFlags & BUILDINGFOCUS_CAPITAL) != 0 )
	{
		if ( kBuilding.isCapital() )
		{
			return true;
		}
	}

	return false;
}

int	CvCityAI::GetBuildingValue(BuildingTypes eBuilding, int iFocusFlags, int iThreshold, bool bMaximizeFlaggedValue, bool bIgnoreCanConstruct)
{
	int iResult = -1;

	//	We only pre-calculate and cache values for buildings this city can build
	//	Only trigger a change in cached city from iFocusFlags != 0 which happens at the start
	//	of building search to choose construction.  Other requests (with 0 flags) can occur
	//	occassionally from other areas (such as civic evaluation) without implying a lot of locality
	//	of reference that makes the caching worthwhile
	if ( (bIgnoreCanConstruct || canConstruct(eBuilding)) && (cachedBuildingValues != NULL || iFocusFlags != 0) )
	{
		if ( cachedBuildingValues == NULL )
		{
			{
				MEMORY_TRACK_EXEMPT();

				OutputDebugString(CvString::format("Rebuilding building value cache for City %S\n", getName().GetCString()).c_str());
				cachedBuildingValues = new BuildingValueCache(this);
			}
		}

		if ( iFocusFlags == 0 )
		{
			iFocusFlags = BUILDINGFOCUS_CONSIDER_ANY;
		}
		if ( (cachedBuildingValues->m_iCachedFlags & iFocusFlags) != iFocusFlags )
		{
			CalculateAllBuildingValues( ~cachedBuildingValues->m_iCachedFlags & iFocusFlags );
		}

retry:
		iResult = cachedBuildingValues->GetValue(eBuilding, iFocusFlags, iThreshold, bMaximizeFlaggedValue);

		if ( iResult == -1 )
		{
			if ( cachedBuildingValues->m_bIncomplete )
			{
				int iCachedFlags = cachedBuildingValues->m_iCachedFlags;
				cachedBuildingValues->m_iCachedFlags = 0;

				CalculateAllBuildingValues( iCachedFlags );

				cachedBuildingValues->m_bIncomplete = false;
				goto retry;
			}
			else if ( bIgnoreCanConstruct )
			{
				iResult = AI_buildingValueThresholdOriginal(eBuilding, iFocusFlags, iThreshold, bMaximizeFlaggedValue);
			}
			else
			{
				iResult = 0;
				FAssert(AI_buildingValueThresholdOriginal(eBuilding, iFocusFlags, iThreshold, bMaximizeFlaggedValue) == 0);
			}
		}
		else
		{
			if (!isHuman())
			{
				if (iResult > 0)
				{
					CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
					CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);

					for (int iI = 0; iI < GC.getNumFlavorTypes(); iI++)
					{
						iResult += (kOwner.AI_getFlavorValue((FlavorTypes)iI) * kBuilding.getFlavorValue(iI));
					}
				}
			}

#ifdef VALIDATE_BUILDING_CACHE_CONSISTENCY
			int realResult = AI_buildingValueThresholdOriginal(eBuilding, iFocusFlags, iThreshold);
			if ( realResult != iResult )
			{
				FAssertMsg(false,"Cached building value result mismatch!!\n");
			}
#endif
		}
	}
	else
	{
		PROFILE("GetBuildingValue.CacheMiss");

		iResult = AI_buildingValueThresholdOriginal(eBuilding, iFocusFlags, iThreshold);
	}

	return iResult;
}

void CvCityAI::AI_FlushBuildingValueCache(bool bRetainValues)
{
	PROFILE_FUNC();

	if ( !bRetainValues )
	{
		OutputDebugString("Flush building cache\n");

		SAFE_DELETE(cachedBuildingValues);
	}
	else if ( cachedBuildingValues != NULL )
	{
		cachedBuildingValues->m_bIncomplete = true;
	}
}

void CvCityAI::CalculateAllBuildingValues(int iFocusFlags)
{
	PROFILE_FUNC()

	//	KOSHLING optimisation - moved what we could outside of the building loop
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	ReligionTypes eStateReligion = kOwner.getStateReligion();
	bool bAreaAlone = kOwner.AI_isAreaAlone(area());
	int iHasMetCount = GET_TEAM(getTeam()).getHasMetCivCount(true);

	int iFoodDifference = foodDifference(false);

	// Reduce reaction to espionage induced happy/health problems
	int iHappinessLevel = happyLevel() - unhappyLevel(1) + getEspionageHappinessCounter()/2;
	int iAngryPopulation = range(-iHappinessLevel, 0, (getPopulation() + 1));
	int iHealthLevel = goodHealth() - badHealth(/*bNoAngry*/ false, std::max(0, (iHappinessLevel + 1) / 2)) + getEspionageHealthCounter()/2;
	int iBadHealth = std::max(0, -iHealthLevel);

	int iHappyModifier = (iHappinessLevel <= iHealthLevel && iHappinessLevel <= 6) ? 6 : 3;
	int iHealthModifier = (iHealthLevel < iHappinessLevel && iHealthLevel <= 4) ? 4 : 2;
	if (iHappinessLevel >= 10)
	{
		iHappyModifier = 1;
	}
	if (iHealthLevel >= 8)
	{
		iHealthModifier = 0;
	}

	int iBaseHappinessLevel = happyLevel() - unhappyLevel() + getEspionageHappinessCounter();
	int iBaseHealthLevel = goodHealth() - badHealth() + getEspionageHealthCounter();
	int iBaseFoodDifference = getYieldRate(YIELD_FOOD) - getPopulation()*GC.getFOOD_CONSUMPTION_PER_POPULATION() - std::max(0,-iBaseHealthLevel);

	int iGoldValueAssessmentModifier = GET_PLAYER(getOwnerINLINE()).AI_goldValueAssessmentModifier();

	//Temporary unhappiness that will mostly not be ignored
	int iTemporaryUnhappiness = 0;
	int iAngerPercent = 0;
	iAngerPercent += getHurryPercentAnger();
	iAngerPercent += getConscriptPercentAnger();
	iAngerPercent += getDefyResolutionPercentAnger();
	iAngerPercent += getWarWearinessPercentAnger();
	iTemporaryUnhappiness += ((iAngerPercent * (getPopulation())) / GC.getPERCENT_ANGER_DIVISOR());

	int iTotalPopulation = kOwner.getTotalPopulation();
	int iNumCities = kOwner.getNumCities();
	int iNumCitiesInArea = area()->getCitiesPerPlayer(getOwnerINLINE());

	int aiYieldRank[NUM_YIELD_TYPES];
	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aiYieldRank[iI] = MAX_INT;
	}

	int aiCommerceRank[NUM_COMMERCE_TYPES];
	for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		aiCommerceRank[iI] = MAX_INT;
	}

	int aiBaseCommerceRate[NUM_COMMERCE_TYPES];
	for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		aiBaseCommerceRate[iI] = MAX_INT;
	}

	int aiPlayerCommerceRate[NUM_COMMERCE_TYPES];
	for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		aiPlayerCommerceRate[iI] = kOwner.getTotalCityBaseCommerceRate((CommerceTypes)iI);
	}

	aiYieldRank[YIELD_PRODUCTION] = findBaseYieldRateRank(YIELD_PRODUCTION);
	bool bIsHighProductionCity = (aiYieldRank[YIELD_PRODUCTION] <= std::max(3, (iNumCities / 2)));

	int iCultureRank = findCommerceRateRank(COMMERCE_CULTURE);
	aiCommerceRank[COMMERCE_CULTURE] = iCultureRank;

	int iCulturalVictoryNumCultureCities = GC.getGameINLINE().culturalVictoryNumCultureCities();

	bool bFinancialTrouble = GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble();

	int iAllowedShrinkRate = (
		GC.getFOOD_CONSUMPTION_PER_POPULATION()
		* (
			std::max(0, -iBaseHappinessLevel - iTemporaryUnhappiness)
			+
			std::min(1, std::max(0, getWorkingPopulation() - AI_countGoodTiles(true, false, 50)))
			+
			std::max(0, visiblePopulation() - AI_countGoodSpecialists(false))
		)
	);
	bool bCulturalVictory1 = GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE1);
	bool bCulturalVictory2 = GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2);
	bool bCulturalVictory3 = GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3);

	bool bSpaceVictory1 = GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_SPACE1);

	std::vector<bool> cityHasVicinityBonus;

	for (int iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
	{
		cityHasVicinityBonus.push_back( hasBonus((BonusTypes)iJ) && hasVicinityBonus((BonusTypes)iJ));
	}

	logBBAIForTeam(getTeam(), "	  City %S CalculateAllBuildingValues for flags %08lx (already has %08lx)", getName().GetCString(), iFocusFlags, cachedBuildingValues->m_iCachedFlags);

	//	Either wonder flag forces all wonders to be included in what gets calculated so we
	//	can mark them both off as done
	if ((iFocusFlags & (BUILDINGFOCUS_WORLDWONDER | BUILDINGFOCUS_WONDEROK)) != 0)
	{
		iFocusFlags |= (BUILDINGFOCUS_WORLDWONDER | BUILDINGFOCUS_WONDEROK);
	}

	cachedBuildingValues->m_iCachedFlags |= iFocusFlags;

	std::set<BuildingTypes> buildingsToCalculate;

	{
		PROFILE("CvCityAI::CalculateAllBuildingValues.PreLoop");
		for (int iI = GC.getNumBuildingClassInfos() - 1; iI > -1; iI--)
		{
			BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI);

			if (NO_BUILDING == eBuilding || !canConstruct(eBuilding))
			{
				continue;
			}
			buildingsToCalculate.insert(eBuilding);

			for (int iJ = GC.getNumBuildingClassInfos() - 1; iJ > -1; iJ--)
			{
				BuildingTypes eJLoopBuilding = ((BuildingTypes)(GC.getBuildingClassInfo((BuildingClassTypes)iJ).getDefaultBuildingIndex()));
				if (eJLoopBuilding != NO_BUILDING
				&& buildingsToCalculate.find(eJLoopBuilding) == buildingsToCalculate.end()
				&& GC.getBuildingInfo(eJLoopBuilding).isReplaceBuildingClass(iI)
				&& getNumBuilding(eJLoopBuilding) > 0)
				{
					buildingsToCalculate.insert(eJLoopBuilding);
				}
			}

			if (GC.getBuildingInfo(eBuilding).EnablesOtherBuildings())
			{
				for (int iJ = GC.getNumBuildingInfos() - 1; iJ > -1; iJ--)
				{
					if (
						buildingsToCalculate.find((BuildingTypes)iJ) == buildingsToCalculate.end()
					&&
						(
							GC.getBuildingInfo((BuildingTypes)iJ).isBuildingClassNeededInCity(iI)
							||
							GC.getBuildingInfo((BuildingTypes)iJ).isPrereqOrBuildingClass(iI)
						)
					&&	getNumBuilding((BuildingTypes)iJ) == 0
					&&	canConstructInternal((BuildingTypes)iJ, false, false, false, true, (BuildingClassTypes)iI)
					) {
						buildingsToCalculate.insert((BuildingTypes)iJ);
					}
				}
			}
		}
	}

	{
		PROFILE("CvCityAI::CalculateAllBuildingValues.Loop");
		//	Calculate all possible (by focus) values for each building type
		for (std::set<BuildingTypes>::const_iterator itr = buildingsToCalculate.begin(); itr != buildingsToCalculate.end(); ++itr)
		{
			BuildingTypes eBuilding = *itr;

			if (NO_BUILDING != eBuilding)
			{
				if (cachedBuildingValues->HasValues(eBuilding) || !buildingMayHaveAnyValue(eBuilding, iFocusFlags))
				{
					continue;
				}

				OneBuildingValueCache* valuesCache = cachedBuildingValues->GetOneBuildingCache(eBuilding);

				//	What follows is essentailyl a copy of the code in AI_buildingValueThreshold with the
				//	modification that it accumulates all focus values to the cache's focus value array
				int iTempValue;

				PROFILE("CvCityAI::CalculateAllBuildingValues.building");

				CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);
				BuildingClassTypes eBuildingClass = (BuildingClassTypes) kBuilding.getBuildingClassType();
				int iLimitedWonderLimit = limitedWonderClassLimit(eBuildingClass);
				bool bIsLimitedWonder = (iLimitedWonderLimit >= 0);

				logBBAIForTeam(getTeam(), "		  Calc value for %S", kBuilding.getDescription());

				bool bProvidesPower = kBuilding.isPower() || kBuilding.getPowerBonus() != NO_BONUS && hasBonus((BonusTypes)kBuilding.getPowerBonus()) || kBuilding.isAreaCleanPower();

			/********************************************************************************/
			/* 	Alternative Building Evaluation				20.02.2010		Fuyu			*/
			/* 	 - requires Actual Building Effects (BULL)									*/
			/* 																				*/
			/* 	TODO:																		*/
			/* 		value riverPlotYieldChanges higher when we are already working on river	*/
			/* 		add getBuildingYieldChange() ?											*/
			/* 		more consistancy (values from yield changes should not differ so much	*/
			/* 			depending on where they come from)									*/
			/********************************************************************************/
				//Don't consider a building if it causes the city to immediately start shrinking from unhealthiness
				//For that purpose ignore bad health and unhappiness from Espionage.
				int iBuildingActualHappiness = getAdditionalHappinessByBuilding(eBuilding);
				int iBuildingActualHealth = getAdditionalHealthByBuilding(eBuilding);

				int iBadHealthFromBuilding = std::max(0,(-iBuildingActualHealth));
				int iUnhealthyPopulationFromBuilding = std::min(0,(-iBaseHealthLevel)) + iBadHealthFromBuilding;
				//	int iTotalHealth = iBaseHealthLevel + iBuildingActualHealth;
				bool bShrinksWithPower = false;
				//Allow a bit of shrinking: Population is expendable if angry, working a bad tile, or running a not-so-good specialist
				if (iUnhealthyPopulationFromBuilding > 0 && (iBaseFoodDifference + iAllowedShrinkRate < iUnhealthyPopulationFromBuilding ))
				{
					continue;
				}
				if (!bProvidesPower && !isPower()) //if the city is still without power after building this
				{
					const int iUnhealthFromBuildingPower = std::min(0,(badHealth() - goodHealth() - getEspionageHealthCounter())) + iBadHealthFromBuilding - GC.getDefineINT("DIRTY_POWER_HEALTH_CHANGE");
					if (iUnhealthFromBuildingPower > 0 && iBaseFoodDifference + iAllowedShrinkRate < iUnhealthFromBuildingPower)
					{
						bShrinksWithPower = true;
					}
				}
				int iSpecialistGreatPeopleRate = 0;
				int iSpecialistExtraHealth = 0;
				int iSpecialistExtraHappy = 0;

				int* paiFreeSpecialistCommerce = new int[NUM_COMMERCE_TYPES];
				int* paiFreeSpecialistYield = new int[NUM_YIELD_TYPES];
				for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
				{
					paiFreeSpecialistYield[iI] = 0;
				}
				for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
				{
					paiFreeSpecialistCommerce[iI] = 0;
				}

				if (kBuilding.getNumPopulationEmployed() > 0)
				{
					removeWorstCitizenActualEffects(kBuilding.getNumPopulationEmployed(), iSpecialistGreatPeopleRate, iSpecialistExtraHappy, iSpecialistExtraHealth, paiFreeSpecialistYield, paiFreeSpecialistCommerce);
				}

				int aiFreeSpecialistYield[NUM_YIELD_TYPES];
				for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
				{
					aiFreeSpecialistYield[iI] = paiFreeSpecialistYield[iI];
				}

				int aiFreeSpecialistCommerce[NUM_COMMERCE_TYPES];
				for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
				{
					aiFreeSpecialistCommerce[iI] = paiFreeSpecialistCommerce[iI];
				}

				SAFE_DELETE_ARRAY(paiFreeSpecialistCommerce);
				SAFE_DELETE_ARRAY(paiFreeSpecialistYield);

				for (int iI = 1; iI < kBuilding.getFreeSpecialist() + 1; iI++)
				{
					SpecialistTypes eNewSpecialist = getBestSpecialist(iI);
					if (eNewSpecialist == NO_SPECIALIST) break;
					CvSpecialistInfo& kSpecialist = GC.getSpecialistInfo(eNewSpecialist);
					for (int iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
						aiFreeSpecialistYield[iJ] += GET_PLAYER(getOwnerINLINE()).specialistYield(eNewSpecialist, (YieldTypes)iJ);
					}
					for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
					{
						aiFreeSpecialistCommerce[iJ] += GET_PLAYER(getOwnerINLINE()).specialistCommerce(eNewSpecialist, (CommerceTypes)iJ);
					}
					iSpecialistGreatPeopleRate += kSpecialist.getGreatPeopleRateChange();
					iSpecialistExtraHealth += kSpecialist.getHealthPercent();
					iSpecialistExtraHappy += kSpecialist.getHappinessPercent();
				}
				iSpecialistExtraHappy /= 100;
				iSpecialistExtraHealth /= 100;
				iBuildingActualHappiness += iSpecialistExtraHappy;
				iBuildingActualHealth += iSpecialistExtraHealth;

				bool bCanPopRush = GET_PLAYER(getOwnerINLINE()).canPopRush();

				bool bForeignTrade = false;
				int iNumTradeRoutes = getTradeRoutes();
				for (int iI = 0; iI < iNumTradeRoutes; ++iI)
				{
					CvCity* pTradeCity = getTradeCity(iI);
					if (NULL != pTradeCity
					&& (GET_PLAYER(pTradeCity->getOwnerINLINE()).getTeam() != getTeam() || pTradeCity->area() != area()))
					{
						bForeignTrade = true;
						break;
					}
				}

				if (kBuilding.isCapital())
				{
					continue;
				}

				bool bSkipThisBuilding = false;

				for (int iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
				{
					BuildingTypes eLoopBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI);

					if (kBuilding.isReplaceBuildingClass((BuildingClassTypes)iI) && eLoopBuilding != NO_BUILDING && canConstruct(eLoopBuilding))
					{
						bSkipThisBuilding = true;
						break;
					}
				}
				if (bSkipThisBuilding)
				{
					continue;
				}

				for (int iI = 0; iI < GC.getNumReligionInfos(); iI++)
				{
					if (kBuilding.getReligionChange(iI) > 0 && !GET_TEAM(getTeam()).hasHolyCity((ReligionTypes)iI))
					{
						bSkipThisBuilding = true;
					}
				}

				if (bSkipThisBuilding)
				{
					continue;
				}

				if (isWorldWonderClass(eBuildingClass) && aiYieldRank[YIELD_PRODUCTION] <= 3)
				{
					valuesCache->Accumulate(BUILDINGFOCUSINDEX_WORLDWONDER,1);
				}

				{
					PROFILE("CalculateAllBuildingValues.Defense");
					int iValue = 0;

					if (!bAreaAlone)
					{
						if ((GC.getGameINLINE().getBestLandUnit() == NO_UNIT) || !(GC.getUnitInfo(GC.getGameINLINE().getBestLandUnit()).isIgnoreBuildingDefense()))
						{
							iValue += (std::max(0, std::min(((kBuilding.getDefenseModifier() + getBuildingDefense()) - getNaturalDefense() - 10), kBuilding.getDefenseModifier())) / 4);

							for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
							{
								if (hasBonus((BonusTypes)iI))
								{
									iValue += (kBuilding.getBonusDefenseChanges(iI) / 4);
								}
							}
						}


						iValue += kBuilding.isZoneOfControl() ? 50 : 0;

						iValue += kBuilding.getLineOfSight() * 15;

						for (int iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
						{
							if (kBuilding.getUnitCombatExtraStrength(iI) != 0)
							{
								int iValidUnitCount = 0;
								CLLNode<IDInfo>* pUnitNode;
								CvUnit* pLoopUnit;
								pUnitNode = plot()->headUnitNode();
								while (pUnitNode != NULL)
								{
									pLoopUnit = ::getUnit(pUnitNode->m_data);
									pUnitNode = plot()->nextUnitNode(pUnitNode);
									if (pLoopUnit->getTeam() == getTeam() && pLoopUnit->getUnitCombatType() == iI)
									{
										iValidUnitCount++;
									}
								}
								iValue += iValidUnitCount * kBuilding.getUnitCombatExtraStrength(iI) / 6;
							}
						}

						if (AI_isDanger()
						|| area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE
						|| area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE
						|| area()->getAreaAIType(getTeam()) == AREAAI_MASSING)
						{
							iValue += kBuilding.getAdjacentDamagePercent() * 10;
						}
						iValue -= kBuilding.getInvasionChance() * 5;

						iValue += kBuilding.isProtectedCulture() ? 50 : 0;
						iValue += kBuilding.getOccupationTimeModifier() / 20;

						if (kBuilding.getNoEntryDefenseLevel() > 0 && kBuilding.getNoEntryDefenseLevel() < getTotalDefense(false))
						{
							iValue += getTotalDefense(false) - kBuilding.getNoEntryDefenseLevel() / 2;
						}
						if (kBuilding.getNumUnitFullHeal() > 0)
						{
							iValue += kBuilding.getNumUnitFullHeal() * 50;
						}
					}

					iValue += kBuilding.getBombardDefenseModifier() / 8;

					iValue -= kBuilding.getAirModifier() / 4;
					iValue -= kBuilding.getNukeModifier() / 4;

					iValue += kBuilding.getAllCityDefenseModifier() * iNumCities / 5;

					iValue += kBuilding.getAirlift() * 25;

					if (kBuilding.isAreaBorderObstacle() && !GC.getGameINLINE().isOption(GAMEOPTION_NO_BARBARIANS))
					{
						//The great wall is much more valuable with more barbarian activity.
						iValue += iNumCitiesInArea;
						if(GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS))
							iValue += iNumCitiesInArea;
						if (GC.getGameINLINE().isOption(GAMEOPTION_BARBARIAN_WORLD))
							iValue += iNumCitiesInArea;
						if (GC.getGameINLINE().isOption(GAMEOPTION_BARBARIAN_GENERALS))
							iValue += iNumCitiesInArea;
					}
					valuesCache->Accumulate(BUILDINGFOCUSINDEX_DEFENSE, iValue);
				}

				valuesCache->Accumulate(BUILDINGFOCUSINDEX_ESPIONAGE, kBuilding.getEspionageDefenseModifier() / 8);

				if (!isNoUnhappiness())
				{
					PROFILE("CalculateAllBuildingValues.Happy");
					int iBestHappy = 0;
					int iValue = 0;

					for (int iI = 0; iI < GC.getNumHurryInfos(); iI++)
					{
						if (canHurryBuilding((HurryTypes)iI, eBuilding, true))
						{
							const int iHappyFromHurry = AI_getHappyFromHurry((HurryTypes)iI, eBuilding, true);
							if (iHappyFromHurry > iBestHappy)
							{
								iBestHappy = iHappyFromHurry;
							}
						}
					}
					iValue += iBestHappy * 10;

					//Fuyu ToDo: How to handle Globe Theater national wonder?
					//For now just give massive boost if city is high food yet not one of the main production or commerce cities
					if (kBuilding.isNoUnhappiness() && bIsLimitedWonder)
					{
						iValue += (iAngryPopulation * 10) + getPopulation();
						aiYieldRank[YIELD_FOOD] = findBaseYieldRateRank(YIELD_FOOD);
						aiYieldRank[YIELD_COMMERCE] = findBaseYieldRateRank(YIELD_COMMERCE);
						if (aiYieldRank[YIELD_FOOD] <= (2 + iLimitedWonderLimit))
						{
							if (!(isNationalWonderClass(eBuildingClass)) ||
								((aiYieldRank[YIELD_PRODUCTION] > (2 + iLimitedWonderLimit))
								&& (aiYieldRank[YIELD_COMMERCE] > (2 + iLimitedWonderLimit))))
							{
								iValue += getPopulation() * 5;
							}
						}
					}
					else
					{
						iValue += happynessValue(iBuildingActualHappiness, iBaseHappinessLevel, iBaseHealthLevel);
					}

					iValue += (-kBuilding.getHurryAngerModifier() * getHurryPercentAnger()) / 100;

					for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
					{
						iValue += (kBuilding.getCommerceHappiness(iI) * iHappyModifier) / 4;
					}

					int iWarWearinessModifer = kBuilding.getWarWearinessModifier();
					if (iWarWearinessModifer != 0)
					{
						if ( GET_TEAM(getTeam()).getAtWarCount(true) == 0 )
						{
							iWarWearinessModifer /= 2;
						}

						iValue += (-iWarWearinessModifer * iHappyModifier) / 16;
					}

					iValue += (kBuilding.getAreaHappiness() * (iNumCitiesInArea - 1) * 8);
					iValue += (kBuilding.getGlobalHappiness() * (iNumCities - 1) * 8);

					int iWarWearinessPercentAnger = kOwner.getWarWearinessPercentAnger();
					int iGlobalWarWearinessModifer = kBuilding.getGlobalWarWearinessModifier();
					if (iGlobalWarWearinessModifer != 0)
					{
						iValue += (-(((iGlobalWarWearinessModifer * iWarWearinessPercentAnger / 100) / GC.getPERCENT_ANGER_DIVISOR())) * (iNumCities - 1));
						iValue += (-iGlobalWarWearinessModifer * iHappyModifier) / 16;
					}

					CvCivilizationInfo& kCivilization = GC.getCivilizationInfo(getCivilizationType());
					for (int iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
					{
						int iBuildingHappinessChanges = kBuilding.getBuildingHappinessChanges(iI);
						if (iBuildingHappinessChanges != 0)
						{
							BuildingTypes eLoopBuilding = (BuildingTypes)kCivilization.getCivilizationBuildings(iI);
							if (eLoopBuilding != NO_BUILDING)
							{
								iValue += (iBuildingHappinessChanges * (kOwner.getBuildingClassCount((BuildingClassTypes)iI) - getNumBuilding(eLoopBuilding)) * 8);
							}
							else
							{
								iValue += (iBuildingHappinessChanges * kOwner.getBuildingClassCount((BuildingClassTypes)iI) * 8);
							}
						}
					}

					if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_REVOLUTION))
					{
						if (kBuilding.getRevIdxLocal() != 0)
						{
							int localRevIdx = getLocalRevIndex();

							//	Use the more serious of the before and after values if this building were to be built
							if ( kBuilding.getRevIdxLocal() > 0 )
							{
								localRevIdx += kBuilding.getRevIdxLocal();
							}

							//	Treat instability seriously as it goes up - not just linear
							int localRevScaling = (localRevIdx < 0 ? 0 : std::min(localRevIdx*localRevIdx/50 + localRevIdx/2, 100));

							iValue -= (kBuilding.getRevIdxLocal() * localRevScaling)/4;
						}
						if (kBuilding.getRevIdxNational() != 0)
						{
							iValue -= (8 * GET_PLAYER(getOwnerINLINE()).getNumCities()) * kBuilding.getRevIdxNational();
						}
						if (kBuilding.getRevIdxDistanceModifier() != 0)
						{
							if (!isCapital())
							{
								CvCity* pCapital = GET_PLAYER(getOwnerINLINE()).getCapitalCity();
								if (pCapital != NULL)
								{
									int iCapitalDistance = ::plotDistance(getX_INLINE(), getY_INLINE(), pCapital->getX_INLINE(), pCapital->getY_INLINE());
									int iOldCapitalDistance = iCapitalDistance;
									iCapitalDistance *= 100 + kBuilding.getRevIdxDistanceModifier();
									iCapitalDistance /= 100;

									iValue += ((iOldCapitalDistance - iCapitalDistance) * (10+std::max(0,getLocalRevIndex())));
								}
							}
						}
					}
					valuesCache->Accumulate(BUILDINGFOCUSINDEX_HAPPY, iValue);
				}

				if (!isNoUnhealthyPopulation())
				{
					PROFILE("CalculateAllBuildingValues.Health");
					valuesCache->Accumulate(
						BUILDINGFOCUSINDEX_HEALTHY,
						healthValue(
							iBuildingActualHealth,
							iBaseHappinessLevel - getEspionageHappinessCounter()/2 + std::max(0, iBuildingActualHappiness),
							iBaseHealthLevel,
							iBaseFoodDifference
						)
						+ kBuilding.getAreaHealth() * (iNumCitiesInArea - 1 ) * 4
						+ kBuilding.getGlobalHealth() * (iNumCities - 1) * 4
					);
				}

				{
					PROFILE("CalculateAllBuildingValues.Experience");
					int iValue = kBuilding.getFreeExperience() * ((iHasMetCount > 0) ? 12 : 6);

					for (int iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
					{
						if (canTrain((UnitCombatTypes)iI))
						{
							iValue += (kBuilding.getUnitCombatFreeExperience(iI) * ((iHasMetCount > 0) ? 6 : 3));
						}
					}

					for (int iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
					{
						int iDomainExpValue = 0;
						if (iI == DOMAIN_SEA)
						{
							iDomainExpValue = 7;
						}
						else if (iI == DOMAIN_LAND)
						{
							iDomainExpValue = 12;
						}
						else
						{
							iDomainExpValue = 6;
						}
						iValue += (kBuilding.getDomainFreeExperience(iI) * ((iHasMetCount > 0) ? iDomainExpValue : iDomainExpValue / 2));
					}

					iTempValue = 0;
					bool bWarPlan = (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0);
					if (kBuilding.getFreePromotion() != NO_PROMOTION)
					{
						iTempValue += AI_getPromotionValue((PromotionTypes)kBuilding.getFreePromotion());
					}
					if (kBuilding.getFreePromotion_2() != NO_PROMOTION)
					{
						iTempValue += AI_getPromotionValue((PromotionTypes)kBuilding.getFreePromotion());
					}
					if (kBuilding.getFreePromotion_3() != NO_PROMOTION)
					{
						iTempValue += AI_getPromotionValue((PromotionTypes)kBuilding.getFreePromotion());
					}
					if (kBuilding.isApplyFreePromotionOnMove())
					{
						iTempValue *= 2;
					}
					iValue += iTempValue;
					bool bDevelopingCity = isDevelopingCity();
					if ((!bDevelopingCity || isCapital()) && kBuilding.EnablesUnits())
					{
						PROFILE("CalculateAllBuildingValues.NotDeveloping");

						for (int iI = 0; iI < GC.getNumUnitInfos(); iI++)
						{
							CvUnitInfo& kUnit = GC.getUnitInfo((UnitTypes)iI);
							bool bUnitIsEnabler = kUnit.getPrereqBuilding() == eBuilding;

							if (
								GET_TEAM(getTeam()).isHasTech((TechTypes)kUnit.getPrereqAndTech())
							&&	(
									bUnitIsEnabler
									||
									kUnit.getPrereqBuilding() == NO_BUILDING
									||
									getNumBuilding((BuildingTypes)kUnit.getPrereqBuilding()) > 0
								)
							)
							{
								bool bUnitIsBonusEnabled = true;
								if (kUnit.getPrereqAndBonus() != NO_BONUS && !hasBonus((BonusTypes)kUnit.getPrereqAndBonus()))
								{
									if (isFreeBonusOfBuilding(kBuilding, (BonusTypes)kUnit.getPrereqAndBonus()))
									{
										bUnitIsEnabler = true;
									}
									else bUnitIsBonusEnabled = false;
								}
								bool bHasORBonusAlready = false;
								bool bFreeBonusIsORBonus = false;
								int iFreeExtraBonusCount = 0;

								for (int iK = 0; iK < GC.getNUM_UNIT_PREREQ_OR_BONUSES(); iK++)
								{
									BonusTypes eXtraFreeBonus = (BonusTypes)kUnit.getPrereqOrBonuses(iK);

									if ( eXtraFreeBonus != NO_BONUS )
									{
										iFreeExtraBonusCount++;

										if ( hasBonus(eXtraFreeBonus) )
										{
											bHasORBonusAlready = true;
										}
										else if ( isFreeBonusOfBuilding(kBuilding, eXtraFreeBonus) )
										{
											bFreeBonusIsORBonus = true;
										}
									}
								}

								if (iFreeExtraBonusCount > 0 && !bHasORBonusAlready)
								{
									if (bFreeBonusIsORBonus)
									{
										bUnitIsEnabler = true;
									}
									else bUnitIsBonusEnabled = false;
								}

								if (bUnitIsEnabler)
								{
									PROFILE("CalculateAllBuildingValues.IsUnitPrereq");

									if (kUnit.getDefaultUnitAIType() != NO_UNITAI && kOwner.AI_totalAreaUnitAIs(area(), ((UnitAITypes)(kUnit.getDefaultUnitAIType()))) == 0)
									{
										iValue += iNumCitiesInArea;
									}
									//This forces the AI to build neccessary buildings for units.

									UnitAITypes eUnitAI = (UnitAITypes)(kUnit.getDefaultUnitAIType());

									iTempValue = 0;

									switch (eUnitAI)
									{
										case UNITAI_UNKNOWN:
										break;
										case UNITAI_ANIMAL:
											iTempValue += kUnit.getCombat() / 5;
											break;
										case UNITAI_SETTLE:
										case UNITAI_SETTLER_SEA:
											iTempValue += 25;
											break;
										case UNITAI_WORKER:
										case UNITAI_WORKER_SEA:
											iTempValue += std::max(0, AI_getWorkersNeeded()) * 10;
											break;
										case UNITAI_ATTACK:
										case UNITAI_ATTACK_CITY:
										case UNITAI_COLLATERAL:
										case UNITAI_PILLAGE:
										case UNITAI_RESERVE:
										case UNITAI_COUNTER:
										case UNITAI_CITY_DEFENSE:
										case UNITAI_CITY_COUNTER:
										case UNITAI_CITY_SPECIAL:
										case UNITAI_ATTACK_SEA:
										case UNITAI_RESERVE_SEA:
										case UNITAI_ESCORT_SEA:
										case UNITAI_ASSAULT_SEA:
										case UNITAI_CARRIER_SEA:
										case UNITAI_MISSILE_CARRIER_SEA:
										case UNITAI_PIRATE_SEA:
										case UNITAI_ATTACK_AIR:
										case UNITAI_DEFENSE_AIR:
										case UNITAI_CARRIER_AIR:
										case UNITAI_MISSILE_AIR:
										case UNITAI_PARADROP:
										case UNITAI_ATTACK_CITY_LEMMING:
											iTempValue += std::max(6, (kUnit.getCombat() * kUnit.getCombat())) / 6;
											break;
										case UNITAI_EXPLORE:
										case UNITAI_MISSIONARY:
										case UNITAI_PROPHET:
										case UNITAI_ARTIST:
										case UNITAI_SCIENTIST:
										case UNITAI_GENERAL:
										case UNITAI_MERCHANT:
										case UNITAI_ENGINEER:
										case UNITAI_SPY:
										case UNITAI_SPY_SEA:
										case UNITAI_MISSIONARY_SEA:
										case UNITAI_EXPLORE_SEA:
											iTempValue += 10;
											break;
										case UNITAI_ICBM:
											iTempValue += kUnit.getNukeRange() * 50;
											break;
									}
									if (bWarPlan)
										iTempValue *= 2;

									if (iTempValue > 0)
									{
										//	Just because we can build new units here doesn't add much if we can already
										//	build them a ton of other places
										iTempValue = (iTempValue*3)/(3+std::min(10,kOwner.countNumBuildings(eBuilding)));
									}

									iValue += bUnitIsBonusEnabled ? iTempValue : iTempValue/5;
								}
							}
						}
						valuesCache->Accumulate(BUILDINGFOCUSINDEX_EXPERIENCE, iValue);
					}
				}

				{
					PROFILE("CalculateAllBuildingValues.Sea");
					int iValue = kBuilding.getFreeExperience() * ((iHasMetCount > 0) ? 16 : 8);

					for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
					{
						UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);

						if (NO_UNIT != eUnit)
						{
							CvUnitInfo& kUnitInfo = GC.getUnitInfo(eUnit);

							if (kUnitInfo.getDomainType() == DOMAIN_SEA && canTrain(eUnit) && kUnitInfo.getUnitCombatType() != NO_UNITCOMBAT)
							{
								iValue += kBuilding.getUnitCombatFreeExperience(kUnitInfo.getUnitCombatType()) * ((iHasMetCount > 0) ? 6 : 3);
							}
						}
					}

					iValue += kBuilding.getDomainFreeExperience(DOMAIN_SEA) * ((iHasMetCount > 0) ? 16 : 8);

					iValue += kBuilding.getDomainProductionModifier(DOMAIN_SEA) / 4;
					valuesCache->AccumulateTo(BUILDINGFOCUSINDEX_DOMAINSEA,iValue, false);
					valuesCache->AccumulateTo(BUILDINGFOCUSINDEX_DOMAINSEA,iValue, true);
				}

				{
					PROFILE("CalculateAllBuildingValues.Maintenance");
					int iValue = 0;
					int iExtraMaintenance = 0;
					if (kBuilding.getCommerceChange(COMMERCE_GOLD) < 0 && GC.getDefineINT("TREAT_NEGATIVE_GOLD_AS_MAINTENANCE"))
					{
						iExtraMaintenance = -kBuilding.getCommerceChange(COMMERCE_GOLD) * GC.getHandicapInfo(getHandicapType()).getCorporationMaintenancePercent();
					}
					const int iBaseMaintenance = getMaintenanceTimes100();
					const int iExistingUpkeep = iBaseMaintenance * std::max(0, 100 + getEffectiveMaintenanceModifier()) / 100;
					const int iNewUpkeep = (iBaseMaintenance + iExtraMaintenance) * std::max(0, 100 + getEffectiveMaintenanceModifier() + kBuilding.getMaintenanceModifier()) / 100;

					iTempValue = (iExistingUpkeep - iNewUpkeep) / 16;

					if (bFinancialTrouble)
					{
						iTempValue *= 2;
					}
					else iTempValue = iGoldValueAssessmentModifier * iTempValue / 100;

					iValue += iTempValue;

					valuesCache->Accumulate(BUILDINGFOCUSINDEX_GOLDANDMAINTENANCE, iValue);
				}

				{
					PROFILE("CalculateAllBuildingValues.Specialist");
					const int iDefaultSpecialist = GC.getDefineINT("DEFAULT_SPECIALIST");
					int iValue = 0;
					int iSpecialistsValue = 0;
					int iCurrentSpecialistsRunnable = 0;
					for (int iI = GC.getNumSpecialistInfos() - 1; iI > -1; iI--)
					{
						if (iI != iDefaultSpecialist)
						{
							const SpecialistTypes eTypeX = static_cast<SpecialistTypes>(iI);

							if (!GET_PLAYER(getOwnerINLINE()).isSpecialistValid(eTypeX))
							{
								const int iRunnable = std::min(5, getMaxSpecialistCount(eTypeX));
								if (iRunnable > 0)
								{
									iCurrentSpecialistsRunnable += iRunnable;
								}
								const int iBuildingSpecialists = kBuilding.getSpecialistCount(iI) + GET_TEAM(getTeam()).getBuildingSpecialistChange(eBuilding, eTypeX);

								if (iBuildingSpecialists > 0 && iRunnable + iBuildingSpecialists > 0)
								{
									iTempValue = AI_specialistValue(eTypeX, false, false);

									iTempValue *= 20 + 40 * iBuildingSpecialists;

									iTempValue /= 100;

									if (iFoodDifference < 2)
									{
										iTempValue /= 4;
									}
									if (iRunnable > 0)
									{
										iTempValue /= 1 + iRunnable;
									}
									iSpecialistsValue += std::max(12, iTempValue / 100);
								}
							}
							else iCurrentSpecialistsRunnable += 6;
						}
					}

					if (iSpecialistsValue > 0)
					{
						iValue += iSpecialistsValue / std::max(2, iCurrentSpecialistsRunnable);
					}
					valuesCache->Accumulate(BUILDINGFOCUSINDEX_SPECIALIST, iValue);
				}

				if (GC.getMAX_TRADE_ROUTES() > 0)
				{
					PROFILE("CalculateAllBuildingValues.Pass1TradeRoutes");

					// trade routes
					const int iTradeRouteValue = tradeRouteValue(kBuilding, YIELD_COMMERCE, bForeignTrade);

					valuesCache->Accumulate(
						BUILDINGFOCUSINDEX_GOLD,
						kOwner.getCommercePercent(COMMERCE_GOLD) * iTradeRouteValue * (bFinancialTrouble ? 200 : iGoldValueAssessmentModifier) / 10000
					);
					valuesCache->Accumulate(BUILDINGFOCUSINDEX_RESEARCH, kOwner.getCommercePercent(COMMERCE_RESEARCH) * iTradeRouteValue / 100);

					valuesCache->Accumulate(BUILDINGFOCUSINDEX_CULTURE, kOwner.getCommercePercent(COMMERCE_CULTURE) * iTradeRouteValue / 100);

					valuesCache->Accumulate(BUILDINGFOCUSINDEX_ESPIONAGE, kOwner.getCommercePercent(COMMERCE_ESPIONAGE) * iTradeRouteValue / 100);
				}

				{
					int iValue = 0;

					if (kBuilding.isAreaCleanPower() && !(area()->isCleanPower(getTeam())))
					{
						int iLoop = 0;
						for (CvCity* pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
						{
							if (pLoopCity->area() == area())
							{
								if (pLoopCity->isDirtyPower())
								{
									//Fuyu: less value for cities that don't already have power
									iValue += 8;
								}
								else if (!pLoopCity->isPower())
								{
									iValue += 12;
								}
							}
						}
					}

					// Toffer - simple crime evaluation
					{
						const int iCrime = kBuilding.getCrime() + kBuilding.getCrimePerPop() * getPopulation();
						if (iCrime != 0)
						{
							iValue -= iCrime * getCrimeRate();
						}
					}

					if (kBuilding.getDomesticGreatGeneralRateModifier() != 0)
					{
						iValue += (kBuilding.getDomesticGreatGeneralRateModifier() / 10);
					}

					if (kBuilding.isAreaBorderObstacle() && !area()->isBorderObstacle(getTeam())
					&& !GC.getGameINLINE().isOption(GAMEOPTION_NO_BARBARIANS))
					{
						iValue += iNumCitiesInArea;

						if (GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS))
						{
							iValue += iNumCitiesInArea;
						}
					}

					if (kBuilding.isGovernmentCenter())
					{
						FAssert(!kBuilding.isCapital());
						iValue += ((calculateDistanceMaintenance() - 3) * iNumCitiesInArea);
					}

					if (kBuilding.isMapCentering())
					{
						iValue++;
					}

					if (kBuilding.getFreeBonus() != NO_BONUS)
					{
						//	If we have spares adding another doesn't do anything that scales by city count - only
						//	the first one does that.  Furthermore as spares rack up even their trade value decreases
						iValue += (kOwner.AI_bonusVal((BonusTypes)(kBuilding.getFreeBonus()), 1) *
									 ((kOwner.getNumTradeableBonuses((BonusTypes)(kBuilding.getFreeBonus())) == 0) ? iNumCities : 1) *
									 kBuilding.getNumFreeBonuses() *
									 100)/(100*std::max(1,kOwner.getNumTradeableBonuses((BonusTypes)(kBuilding.getFreeBonus()))));
					}

					for (int iI = 0; iI < kBuilding.getNumExtraFreeBonuses(); iI++)
					{
						//	If we have spares adding another doesn't do anything that scales by city count - only
						//	the first one does that.  Furthermore as spares rack up even their trade value decreases
						iValue += (kOwner.AI_bonusVal((BonusTypes)(kBuilding.getExtraFreeBonus(iI)), 1) *
									 ((kOwner.getNumTradeableBonuses((BonusTypes)(kBuilding.getExtraFreeBonus(iI))) == 0) ? iNumCities : 1) *
									 kBuilding.getExtraFreeBonusNum(iI) *
									 100)/(100*std::max(1,kOwner.getNumTradeableBonuses((BonusTypes)(kBuilding.getExtraFreeBonus(iI)))));
					}

					if (kBuilding.getNoBonus() != NO_BONUS)
					{
						iValue -= kOwner.AI_bonusVal((BonusTypes)kBuilding.getNoBonus());
					}



					if (kBuilding.getCivicOption() != NO_CIVICOPTION)
					{
						for (int iI = GC.getNumCivicInfos() - 1; iI > -1; iI--)
						{
							if (GC.getCivicInfo((CivicTypes)iI).getCivicOptionType() == kBuilding.getCivicOption()
							&& !kOwner.canDoCivics((CivicTypes)iI))
							{
								iValue += (kOwner.AI_civicValue((CivicTypes)iI) / 10);
							}
						}
					}

					if (kBuilding.getGreatPeopleRateModifier() > 0)
					{
						const int iGreatPeopleRate = getBaseGreatPeopleRate();
						const int kTargetGPRate = 10;

						// either not a wonder, or a wonder and our GP rate is at least the target rate
						if (!bIsLimitedWonder || iGreatPeopleRate >= kTargetGPRate)
						{
							iValue += kBuilding.getGreatPeopleRateModifier() * iGreatPeopleRate / 16;
						}
						// otherwise, this is a limited wonder (aka National Epic), we _really_ do not want to build this here
						// subtract from the value (if this wonder has a lot of other stuff, we still might build it)
						else
						{
							iValue -= kBuilding.getGreatPeopleRateModifier() * (kTargetGPRate - iGreatPeopleRate) / 12;
						}
					}

					iValue += kBuilding.getGlobalGreatPeopleRateModifier() * iNumCities / 8;

					iValue -= kBuilding.getAnarchyModifier() / 4;

					iValue -= kBuilding.getGlobalHurryModifier() * 2;

					iValue += kBuilding.getGlobalFreeExperience() * iNumCities * ((iHasMetCount > 0) ? 6 : 3);

					if (bCanPopRush)
					{
						iValue += kBuilding.getFoodKept() / 2;
					}

					iValue += kBuilding.getAirlift() * (getPopulation() * 3 + 10);

					if (kBuilding.getAirModifier() < 0
					&& (kOwner.AI_totalUnitAIs(UNITAI_DEFENSE_AIR) > 0 && kOwner.AI_totalUnitAIs(UNITAI_ATTACK_AIR) > 0 || kOwner.AI_totalUnitAIs(UNITAI_MISSILE_AIR) > 0))
					{
						iValue -= kBuilding.getAirModifier() / ((iHasMetCount > 0) ? 2 : 4);
					}
					iValue += kBuilding.getAirUnitCapacity() * (getPopulation() * 2 + 10);

					iValue += (-(kBuilding.getNukeModifier()) / ((iHasMetCount > 0) ? 10 : 20));

					//iValue += (kBuilding.getFreeSpecialist() * 16);
					iValue += (kBuilding.getAreaFreeSpecialist() * iNumCitiesInArea * 12);
					iValue += (kBuilding.getGlobalFreeSpecialist() * iNumCities * 12);

					iValue += ((kBuilding.getWorkerSpeedModifier() * kOwner.AI_getNumAIUnits(UNITAI_WORKER)) / 10);

					const int iMilitaryProductionModifier = kBuilding.getMilitaryProductionModifier();
					if (iHasMetCount > 0 && iMilitaryProductionModifier > 0)
					{
						// either not a wonder, or a wonder and we are a high production city
						if (!bIsLimitedWonder || bIsHighProductionCity)
						{
							iValue += (iMilitaryProductionModifier / 4);

							// if a wonder, then pick one of the best cities
							if (bIsLimitedWonder)
							{
								// if one of the top 3 production cities, give a big boost
								if (aiYieldRank[YIELD_PRODUCTION] <= (2 + iLimitedWonderLimit))
								{
									iValue += (2 * iMilitaryProductionModifier) / (2 + aiYieldRank[YIELD_PRODUCTION]);
								}
							}
							// otherwise, any of the top half of cities will do
							else if (bIsHighProductionCity)
							{
								iValue += iMilitaryProductionModifier / 4;
							}
							iValue += ((iMilitaryProductionModifier * (getFreeExperience() + getSpecialistFreeExperience())) / 10);
						}
						// otherwise, this is a limited wonder (aka Heroic Epic), we _really_ do not want to build this here
						// subtract from the value (if this wonder has a lot of other stuff, we still might build it)
						else
						{
							iValue -= (iMilitaryProductionModifier * aiYieldRank[YIELD_PRODUCTION]) / 5;
						}
					}
					iValue += (kBuilding.getSpaceProductionModifier() / 5);
					iValue += ((kBuilding.getGlobalSpaceProductionModifier() * iNumCities) / 20);

					if (kBuilding.getGreatPeopleUnitClass() != NO_UNITCLASS)
					{
						iValue++; // XXX improve this for diversity...
					}

					// prefer to build great people buildings in places that already have some GP points
					iValue += ((kBuilding.getGreatPeopleRateChange() + iSpecialistGreatPeopleRate ) * 10) * (1 + (getBaseGreatPeopleRate() / 2));

					if (!bAreaAlone)
					{
						iValue += (kBuilding.getHealRateChange() / 2);
					}

					iValue += (kBuilding.getGlobalPopulationChange() * iNumCities * 4);

					iValue += (kBuilding.getFreeTechs() * 80);

					iValue += kBuilding.getEnemyWarWearinessModifier() / 2;

					for (int iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
					{
						if (kBuilding.getFreeSpecialistCount(iI) > 0)
						{
							iValue += ((AI_specialistValue(((SpecialistTypes)iI), false, false) * kBuilding.getFreeSpecialistCount(iI)) / 50);
						}
					}

					for (int iI = 0; iI < GC.getNumImprovementInfos(); iI++)
					{
						if (kBuilding.getImprovementFreeSpecialist(iI) > 0)
						{
							iValue += kBuilding.getImprovementFreeSpecialist(iI) * countNumImprovedPlots((ImprovementTypes)iI, true) * 50;
						}
					}

					for (int iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
					{
						iValue += (kBuilding.getDomainProductionModifier(iI) / 5);

						if (bIsHighProductionCity)
						{
							iValue += (kBuilding.getDomainProductionModifier(iI) / 5);
						}
					}

					if (kBuilding.isForceAllTradeRoutes())
					{
						for (int iI = 0; iI < MAX_TEAMS; iI++)
						{
							if (GET_TEAM((TeamTypes)iI).isAlive()
							&& !GET_TEAM(GET_PLAYER(getOwnerINLINE()).getTeam()).isVassal((TeamTypes)iI)
							&& !GET_TEAM(GET_PLAYER(getOwnerINLINE()).getTeam()).isOpenBorders((TeamTypes)iI))
							{
								iValue += GET_TEAM((TeamTypes)iI).getNumCities() * 2;
							}
						}
					}
					iTempValue = 0;
					for (int iI = GC.getNumUnitClassInfos() - 1; iI > -1; iI--)
					{
						const int iModifier = kBuilding.getUnitClassProductionModifier(iI);

						if (iModifier != 0)
						{
							UnitTypes eLoopUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI);

							if (canTrain(eLoopUnit))
							{
								UnitAITypes eUnitAI = (UnitAITypes) GC.getUnitInfo(eLoopUnit).getDefaultUnitAIType();

								UnitTypes eBestUnit = kOwner.bestBuildableUnitForAIType((DomainTypes)GC.getUnitInfo(eLoopUnit).getDomainType(), eUnitAI);
								int iBuildCost;

								if (eBestUnit != NO_UNIT)
								{
									const int iBestUnitAIValue = kOwner.AI_unitValue(eBestUnit, eUnitAI, area());
									const int iThisUnitAIValue = kOwner.AI_unitValue(eLoopUnit, eUnitAI, area());

									//	Value as cost of production of the unit we can build scaled by their relative AI value (non-linear - we're squaring the ratio)
									const int iComparisonToBestFactor = 10 * iThisUnitAIValue / std::max(1, iBestUnitAIValue);

									iBuildCost = iComparisonToBestFactor * iComparisonToBestFactor * GC.getUnitInfo(eBestUnit).getProductionCost() / 100;
								}
								else iBuildCost = GC.getUnitInfo(eLoopUnit).getProductionCost();

								iTempValue += iBuildCost * 2 * iModifier / 100;
							}
						}
					}
					if (bIsHighProductionCity)
					{
						iTempValue *= 3; // adding 10% and 20% is same as adding 30% or 3 * 10%
					}
					iValue += iTempValue;

					if (kBuilding.getPopulationgrowthratepercentage() != 0)
					{
						const int iCityHappy = 1 + happyLevel() - unhappyLevel();

						if (iCityHappy > 0)
						{
							const int iFoodPerTurn = foodDifference();
							if (iFoodPerTurn > 0)
							{
								iValue += kBuilding.getPopulationgrowthratepercentage() * (-std::min(3, iCityHappy) * growthThreshold() / iFoodPerTurn) / 100;
							}
						}
					}

					if (kBuilding.getGlobalPopulationgrowthratepercentage() != 0)
					{
						int iFactor = 0;
						int iCount = 0;
						int iLoop = 0;

						for (CvCity* pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
						{
							iCount += 100;
							const int iCityHappy = 1 + pLoopCity->happyLevel() - pLoopCity->unhappyLevel();
							if (iCityHappy > 0)
							{
								const int iFoodPerTurn = pLoopCity->foodDifference();
								if (iFoodPerTurn > 0)
								{
									iFactor -= std::min(3, iCityHappy) * pLoopCity->growthThreshold() / iFoodPerTurn;
								}
							}
						}
						if (iCount > 0)
						{
							iValue += kBuilding.getGlobalPopulationgrowthratepercentage() * iFactor / iCount;
						}
					}

					for (int iI = GC.getNumBuildingClassInfos() - 1; iI > -1; iI--)
					{
						if (kBuilding.getBuildingClassProductionModifier(iI) != 0)
						{
							const BuildingTypes eBuildingX = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI);

							if (eBuildingX != NO_BUILDING && canConstruct(eBuildingX))
							{
								const int iModifier = kBuilding.getBuildingClassProductionModifier(iI);

								if (iModifier > -100)
								{
									const int iOriginalCost = getHurryCost(true, eBuildingX, false);
									const int iNewCost = iOriginalCost * 100 / (100 + iModifier);

									iValue += (iOriginalCost - iNewCost) / 10;
								}
								else //If the modifier is less than -99, avoid it like the plague
								{
									iValue -= 1000;
								}
							}
						}
						if (kBuilding.getGlobalBuildingClassProductionModifier(iI) != 0)
						{
							const BuildingTypes eBuildingX = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI);

							if (eBuildingX != NO_BUILDING && canConstruct(eBuildingX))
							{
								const int iModifier = kBuilding.getGlobalBuildingClassProductionModifier(iI);

								if (iModifier > -100)
								{
									int iCount = 0;
									int iLoop = 0;
									for (CvCity* pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
									{
										if (pLoopCity->getNumBuilding(eBuildingX) == 0)
										{
											iCount++;
										}
									}
									if (iCount > 0)
									{
										const int iOriginalCost = getHurryCost(true, eBuildingX, false);
										const int iNewCost = iOriginalCost * 100 / (100 + iModifier);
										iValue += (iOriginalCost - iNewCost) * iCount / 10;
									}
								}
								else //If the modifier is less than -99, avoid it like the plague
								{
									iValue -= 1000;
								}
							}
						}
					}

					if (kBuilding.isProvidesFreshWater() && !plot()->isFreshWater())
					{
						CvPlot* pLoopPlot;
						for (int iI = 0; iI < NUM_CITY_PLOTS_1; iI++)
						{
							if (iI != CITY_HOME_PLOT)
							{
								pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);
								if (pLoopPlot != NULL &&
									!pLoopPlot->isWater() &&
									!pLoopPlot->isFreshWater() &&
									!pLoopPlot->isHills() &&
									!pLoopPlot->isImpassable() )
								{
									if(pLoopPlot->getImprovementType() == NO_IMPROVEMENT)
									{//if there are no improvements, farms become much more likely
										iValue += 20;
									}
									else
									{//we are not likely to tear down existing improvements for a farm
										iValue += 5;
									}
								}
							}
						}
					}

					iTempValue = 0;
					for (int iI = GC.getNumUnitInfos() - 1; iI > -1; iI--)
					{
						if (GC.getUnitInfo((UnitTypes)iI).getPrereqBuilding() == eBuilding)
						{
							// BBAI TODO: Smarter monastary construction, better support for mods

							if (kOwner.AI_totalAreaUnitAIs(area(), ((UnitAITypes)(GC.getUnitInfo((UnitTypes)iI).getDefaultUnitAIType()))) == 0)
							{
								iTempValue += iNumCitiesInArea;
							}

							iTempValue++;

							ReligionTypes eReligion = (ReligionTypes)(GC.getUnitInfo((UnitTypes)iI).getPrereqReligion());
							if (eReligion != NO_RELIGION)
							{
								//encouragement to get some minimal ability to train special units
								if (bCulturalVictory1 || isHolyCity(eReligion) || isCapital())
								{
									iTempValue += (2 + iNumCitiesInArea);
								}

								if (bCulturalVictory2 && GC.getUnitInfo((UnitTypes)iI).getReligionSpreads(eReligion))
								{
									//this gives a very large extra value if the religion is (nearly) unique
									//to no extra value for a fully spread religion.
									//I'm torn between huge boost and enough to bias towards the best monastery type.
									int iReligionCount = GET_PLAYER(getOwnerINLINE()).getHasReligionCount(eReligion);
									iTempValue += (100 * (iNumCities - iReligionCount)) / (iNumCities * (iReligionCount + 1));
								}
							}
						}
					}
					if (iTempValue > 0)
					{
						// Just because we can build new units here doesn't add much if we can already build them a ton of other places
						iTempValue = iTempValue * 3 / (3 + std::min(10, kOwner.countNumBuildings(eBuilding)));
					}
					iValue += iTempValue;

					// is this building needed to build other buildings?
					for (int iI = GC.getNumBuildingInfos() - 1; iI > -1; iI--)
					{
						const BuildingTypes eBuildingX = static_cast<BuildingTypes>(iI);
						const int iPrereqBuildings = kOwner.getBuildingClassPrereqBuilding(eBuildingX, eBuildingClass);

						// if we need some of us to build iI building, and we dont need more than we have cities
						if (iPrereqBuildings > 0 && iPrereqBuildings <= iNumCities
						// do we need more than what we are currently building?
						&& iPrereqBuildings > kOwner.getBuildingClassCountPlusMaking(eBuildingClass))
						{
							iValue += iNumCities * 3;

							if (bCulturalVictory1)
							{
								const CvBuildingInfo& buildingX = GC.getBuildingInfo(eBuildingX);
								const int iCultureModifier = buildingX.getCommerceModifier(COMMERCE_CULTURE);
								if (
									iCultureModifier > 0
								&&
									(
										// if we have less than the number needed in culture cities
										kOwner.getBuildingClassCount((BuildingClassTypes) buildingX.getBuildingClassType()) < iCulturalVictoryNumCultureCities
									|| // OR we are one of the top cities and we do not have the building
										iCultureRank <= iCulturalVictoryNumCultureCities && 0 == getNumBuilding(eBuildingX)
									)
								)
								{
									iValue += iCultureModifier;

									if (bCulturalVictory3)
									{
										iValue += iCultureModifier * 2;
									}
								}
							}
						}
					}

					for (int iI = 0; iI < GC.getNumVoteSourceInfos(); ++iI)
					{
						if (kBuilding.getVoteSourceType() == iI)
						{
							iTempValue = 0;
							if (kBuilding.isStateReligion())
							{
								int iShareReligionCount = 0;
								int iPlayerCount = 0;
								for (int iJ = 0; iJ < MAX_PLAYERS; iJ++)
								{
									CvPlayerAI& kLoopPlayer = GET_PLAYER((PlayerTypes)iJ);
									if (iJ != getOwner() && kLoopPlayer.isAlive())
									{
										iPlayerCount++;
										if (GET_PLAYER(getOwnerINLINE()).getStateReligion() == kLoopPlayer.getStateReligion())
										{
											iShareReligionCount++;
										}
									}
								}
								iTempValue += (200 * (1 + iShareReligionCount)) / (1 + iPlayerCount);
							}
							else
							{
								iTempValue += 100;
							}
							iValue += iTempValue * (GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_DIPLOMACY1) ? 5 : 1);
						}
					}
					valuesCache->AccumulateToAny(iValue, false);
				}
				{
					int iValue = 0;

					for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
					{
						iTempValue = 0;

						if (iFoodDifference > 0)
						{
							iValue += kBuilding.getFoodKept() / 2;
						}

						if (kBuilding.getSeaPlotYieldChange(iI) > 0)
						{
							iTempValue += kBuilding.getSeaPlotYieldChange(iI) * AI_buildingSpecialYieldChangeValue(eBuilding, (YieldTypes)iI);
						}
						if (kBuilding.getRiverPlotYieldChange(iI) > 0)
						{
							iTempValue += (kBuilding.getRiverPlotYieldChange(iI) * countNumRiverPlots() * 4);
						}
						iTempValue += kBuilding.getGlobalSeaPlotYieldChange(iI) * kOwner.countNumCoastalCities() * 8;
						iTempValue += (kBuilding.getYieldChange(iI) + GET_TEAM(getTeam()).getBuildingYieldChange(eBuilding, (YieldTypes)iI) + aiFreeSpecialistYield[iI]) * 8;
						iTempValue += ((kBuilding.getYieldModifier(iI) + GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, (YieldTypes)iI)) * getBaseYieldRate((YieldTypes)iI)) / 12;

						if (!bShrinksWithPower)
						{
							iTempValue += ((kBuilding.getPowerYieldModifier(iI) * getBaseYieldRate((YieldTypes)iI)) / ((bProvidesPower || isPower()) ? 10 : 17));
						}

						//Afforess extra connectedness modifiers
						if (iI == YIELD_COMMERCE)
						{
							iTempValue += getConnectednessYield(eBuilding, YIELD_COMMERCE, true) * 8;
						}
						//Afforess end

						int iGlobalModifier = kBuilding.getGlobalYieldModifier(iI);
						int iAreaModifier = kBuilding.getAreaYieldModifier(iI);
						int iYieldModifierValue = 0;

						if ( iGlobalModifier > 0 || iAreaModifier > 0 )
						{
							int iLoop = 0;
							for (CvCity* pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
							{
								int iCityValue = pLoopCity->getBaseYieldRate((YieldTypes)iI);
								iYieldModifierValue += iCityValue*(iGlobalModifier + (pLoopCity->area() == area() ? iAreaModifier : 0));
							}
						}
						iTempValue += iYieldModifierValue/12;

						if (bProvidesPower && !isPower())
						{
							iTempValue += ((getPowerYieldRateModifier((YieldTypes)iI) * getBaseYieldRate((YieldTypes)iI)) / 12);
						}

						for (int iJ = 0; iJ < GC.getNumSpecialistInfos(); iJ++)
						{
							iTempValue += ((kBuilding.getSpecialistYieldChange(iJ, iI) * kOwner.getTotalPopulation()) / 5);
						}

						for (int iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
						{
							if (hasBonus((BonusTypes)iJ))
							{
								iTempValue += kBuilding.getBonusYieldModifier(iJ, iI) * getBaseYieldRate((YieldTypes)iI) / 12;
								iTempValue += kBuilding.getBonusYieldChanges(iJ, iI) * 8;

								if (kBuilding.getVicinityBonusYieldChanges(iJ, iI) != 0 && cityHasVicinityBonus[iJ])
								{
									iTempValue += (kBuilding.getVicinityBonusYieldChanges(iJ, iI) * 8);
								}
							}
						}

						if (iTempValue != 0)
						{
							if (bFinancialTrouble && iI == YIELD_COMMERCE)
							{
								iTempValue *= 2;
							}

							iTempValue *= kOwner.AI_yieldWeight((YieldTypes)iI);
							iTempValue /= 100;

							if (aiYieldRank[iI] == MAX_INT)
							{
								aiYieldRank[iI] = findBaseYieldRateRank((YieldTypes) iI);
							}

							// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
							// we do _not_ want to build this here (unless the value was small anyway)
							if (bIsLimitedWonder && (aiYieldRank[iI] > (3 + iLimitedWonderLimit)))
							{
								iTempValue *= -1;
							}

							iValue += iTempValue;
						}
					}
					valuesCache->AccumulateToAny(iValue,false);
				}
				{
					PROFILE("CalculateAllBuildingValues.Food");
					int iValue = 0;

					iValue += tradeRouteValue(kBuilding, YIELD_FOOD, bForeignTrade);

					iValue += kBuilding.getFoodKept();

					if (kBuilding.getSeaPlotYieldChange(YIELD_FOOD) > 0)
					{

						iTempValue = kBuilding.getSeaPlotYieldChange(YIELD_FOOD) * AI_buildingSpecialYieldChangeValue(eBuilding, YIELD_FOOD);

						if (iTempValue >= 8 || getPopulation() <= 3)
						{
							iValue += ((iTempValue * 4) / std::max(2, iFoodDifference));
						}
						// else don't bother
					}
					iTempValue = GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, YIELD_FOOD) * getBaseYieldRate(YIELD_FOOD) / 8;
					iTempValue += (kBuilding.getYieldChange(YIELD_FOOD) + GET_TEAM(getTeam()).getBuildingYieldChange(eBuilding, YIELD_FOOD) + aiFreeSpecialistYield[YIELD_FOOD]) * 8;
					iTempValue += (kBuilding.getYieldModifier(YIELD_FOOD) + GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, YIELD_FOOD)) * getBaseYieldRate(YIELD_FOOD) / 12;
					for (int iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
					{
						if (hasBonus((BonusTypes)iJ))
						{
							iTempValue += (kBuilding.getBonusYieldModifier(iJ, YIELD_FOOD) * getBaseYieldRate(YIELD_FOOD) / 12);
							if (kBuilding.getVicinityBonusYieldChanges(iJ, YIELD_FOOD) != 0 && cityHasVicinityBonus[iJ])
							{
								iTempValue += (kBuilding.getVicinityBonusYieldChanges(iJ, YIELD_FOOD) * 8);
							}

						}
					}
					iValue += iTempValue;

					if (kBuilding.getRiverPlotYieldChange(YIELD_FOOD) > 0)
					{
						iValue += (kBuilding.getRiverPlotYieldChange(YIELD_FOOD) * countNumRiverPlots() * 4);
					}

					valuesCache->AccumulateTo(BUILDINGFOCUSINDEX_FOOD,iValue, true);
				}
				{
					PROFILE("CalculateAllBuildingValues.Production");
					int iValue = 0;

					iValue += tradeRouteValue(kBuilding, YIELD_PRODUCTION, bForeignTrade)	;

					iTempValue = (((kBuilding.getYieldModifier(YIELD_PRODUCTION) + GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, YIELD_PRODUCTION)) * getBaseYieldRate(YIELD_PRODUCTION)) / 16);

					//Fuyu - Shrink consideration; and "? 24 : 30" is not enough / too much -> "? 20 : 35" (full value if there is already power, less if you still have to build a power plant. No value if you can't build any powerplant without shrinking the city)
					if (!bShrinksWithPower)
					{
						iTempValue += ((kBuilding.getPowerYieldModifier(YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION)) / ((bProvidesPower || isPower()) ? 20 : 35));
					}

					if (kBuilding.getSeaPlotYieldChange(YIELD_PRODUCTION) > 0)
					{
						int iNumWaterPlots = countNumWaterPlots();

						if (!bIsLimitedWonder || (iNumWaterPlots > getNumCityPlots() / 2))
						{
							iTempValue += kBuilding.getSeaPlotYieldChange(YIELD_PRODUCTION) * iNumWaterPlots;
						}
					}
					if (kBuilding.getRiverPlotYieldChange(YIELD_PRODUCTION) > 0)
					{
						iTempValue += (kBuilding.getRiverPlotYieldChange(YIELD_PRODUCTION) * countNumRiverPlots() * 4);
					}
					if (bProvidesPower && !isPower())
					{
						iTempValue += ((getPowerYieldRateModifier(YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION)) / 20);
					}

					// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
					// we do _not_ want to build this here (unless the value was small anyway)
					if (bIsLimitedWonder && (aiYieldRank[YIELD_PRODUCTION] > (3 + iLimitedWonderLimit)))
					{
						iTempValue *= -1;
					}

					iValue += iTempValue;

					iTempValue = 0;
					iTempValue += ((GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION) + aiFreeSpecialistYield[YIELD_PRODUCTION]) / 8);
					iTempValue += ((kBuilding.getYieldChange(YIELD_PRODUCTION) + GET_TEAM(getTeam()).getBuildingYieldChange(eBuilding, YIELD_PRODUCTION)) * 8);
					iTempValue += ((kBuilding.getYieldModifier(YIELD_PRODUCTION) + GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, YIELD_PRODUCTION)) * getBaseYieldRate(YIELD_PRODUCTION) / 12);
					for (int iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
					{
						if (hasBonus((BonusTypes)iJ))
						{
							iTempValue += (kBuilding.getBonusYieldModifier(iJ, YIELD_PRODUCTION) * getBaseYieldRate(YIELD_PRODUCTION) / 12);
							iTempValue += (kBuilding.getBonusYieldChanges(iJ, YIELD_PRODUCTION) * 8);
							if (kBuilding.getVicinityBonusYieldChanges(iJ, YIELD_PRODUCTION) != 0 && cityHasVicinityBonus[iJ])
							{
								iTempValue += (kBuilding.getVicinityBonusYieldChanges(iJ, YIELD_PRODUCTION) * 8);
							}
						}
					}
					iValue += iTempValue;

					valuesCache->AccumulateTo(BUILDINGFOCUSINDEX_PRODUCTION,iValue, true);
				}
				{
					PROFILE("CalculateAllBuildingValues.Gold");
					int iValue = 0;

					iTempValue = (kBuilding.getYieldModifier(YIELD_COMMERCE) + GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, YIELD_COMMERCE)) * getBaseYieldRate(YIELD_COMMERCE);

					iTempValue *= kOwner.getCommercePercent(COMMERCE_GOLD);

					if (bFinancialTrouble)
					{
						iTempValue *= 2;
					}
					else
					{
						iTempValue = (iGoldValueAssessmentModifier*iTempValue)/100;
					}

					iTempValue /= 3000;

					if (MAX_INT == aiCommerceRank[COMMERCE_GOLD])
					{
						aiCommerceRank[COMMERCE_GOLD] = findCommerceRateRank(COMMERCE_GOLD);
					}

					// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
					// we do _not_ want to build this here (unless the value was small anyway)
					if (bIsLimitedWonder && (aiCommerceRank[COMMERCE_GOLD] > (3 + iLimitedWonderLimit)))
					{
						iTempValue *= -1;
					}
					iValue += iTempValue;

					iTempValue = 0;
					iTempValue += ((kBuilding.getYieldModifier(YIELD_COMMERCE) + GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, YIELD_COMMERCE)) * getBaseYieldRate(YIELD_COMMERCE) + aiFreeSpecialistYield[YIELD_COMMERCE]) / 8;
					iTempValue += (kBuilding.getYieldChange(YIELD_COMMERCE) + GET_TEAM(getTeam()).getBuildingYieldChange(eBuilding, YIELD_COMMERCE)) * 8;
					for (int iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
					{
						if (hasBonus((BonusTypes)iJ))
						{
							iTempValue += (kBuilding.getBonusYieldModifier(iJ, YIELD_COMMERCE) * getBaseYieldRate(YIELD_COMMERCE) / 12);
							iTempValue += (kBuilding.getBonusYieldChanges(iJ, YIELD_COMMERCE) * 8);
							if (kBuilding.getVicinityBonusYieldChanges(iJ, YIELD_COMMERCE) != 0 && cityHasVicinityBonus[iJ])
							{
								iTempValue += (kBuilding.getVicinityBonusYieldChanges(iJ, YIELD_COMMERCE) * 8);
							}
						}
					}
					iTempValue *= GET_PLAYER(getOwnerINLINE()).getCommercePercent(COMMERCE_GOLD)*iGoldValueAssessmentModifier;
					iTempValue /= 10000;
					iValue += iTempValue;

					valuesCache->AccumulateTo(BUILDINGFOCUSINDEX_GOLD,iValue, true);
				}
				{
					int iValue = 0;

					//	Deal with properties
					iValue += buildingPropertiesValue(kBuilding);

					for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
					{
						iTempValue = getBuildingCommerceValue(eBuilding, iI, aiFreeSpecialistYield, aiFreeSpecialistCommerce, aiBaseCommerceRate, aiPlayerCommerceRate);

						if (iTempValue != 0)
						{
							//	Make sure we don't reduce 1 to 0!
							if ( iTempValue >= 2 )
							{
								iTempValue /= 2;
							}

							if (bFinancialTrouble && iI == COMMERCE_GOLD)
							{
								iTempValue *= 2;
							}

							iTempValue *= kOwner.AI_commerceWeight(((CommerceTypes)iI), this);
							iTempValue = (iTempValue + 99) / 100;

							// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
							// we do _not_ want to build this here (unless the value was small anyway)
							if (MAX_INT == aiCommerceRank[iI])
							{
								aiCommerceRank[iI] = findCommerceRateRank((CommerceTypes) iI);
							}
							if (bIsLimitedWonder && ((aiCommerceRank[iI] > (3 + iLimitedWonderLimit)))
								|| (bCulturalVictory1 && (iI == COMMERCE_CULTURE) && (aiCommerceRank[iI] == 1)))
							{
								iTempValue *= -1;

								// for culture, just set it to zero, not negative, just about every wonder gives culture
								if (iI == COMMERCE_CULTURE)
								{
									iTempValue = 0;
								}
							}
							iValue += iTempValue;
						}
					}

					for (int iI = 0; iI < GC.getNumReligionInfos(); iI++)
					{
						if (kBuilding.getReligionChange(iI) > 0 && GET_TEAM(getTeam()).hasHolyCity((ReligionTypes)iI))
						{
							iValue += (kBuilding.getReligionChange(iI) * ((eStateReligion == iI) ? 10 : 1));
						}
					}

					if (NO_VOTESOURCE != kBuilding.getVoteSourceType())
					{
						iValue += 100;
					}

					valuesCache->AccumulateToAny(iValue,false);
				}
				valuesCache->AccumulateTo(BUILDINGFOCUSINDEX_GOLD, getBuildingCommerceValue(eBuilding, COMMERCE_GOLD, aiFreeSpecialistYield, aiFreeSpecialistCommerce, aiBaseCommerceRate, aiPlayerCommerceRate), true);
				valuesCache->AccumulateTo(BUILDINGFOCUSINDEX_RESEARCH, getBuildingCommerceValue(eBuilding, COMMERCE_RESEARCH, aiFreeSpecialistYield, aiFreeSpecialistCommerce, aiBaseCommerceRate, aiPlayerCommerceRate), true);
				valuesCache->AccumulateTo(BUILDINGFOCUSINDEX_CULTURE, getBuildingCommerceValue(eBuilding, COMMERCE_CULTURE, aiFreeSpecialistYield, aiFreeSpecialistCommerce, aiBaseCommerceRate,aiPlayerCommerceRate), true);
				valuesCache->AccumulateTo(BUILDINGFOCUSINDEX_BIGCULTURE, getBuildingCommerceValue(eBuilding, COMMERCE_CULTURE, aiFreeSpecialistYield, aiFreeSpecialistCommerce, aiBaseCommerceRate, aiPlayerCommerceRate) / 5, true);

				if (GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
				{
					valuesCache->AccumulateTo(BUILDINGFOCUSINDEX_CULTURE, getBuildingCommerceValue(eBuilding, COMMERCE_ESPIONAGE, aiFreeSpecialistYield, aiFreeSpecialistCommerce, aiBaseCommerceRate, aiPlayerCommerceRate),true);
				}
				else
				{
					valuesCache->AccumulateTo(BUILDINGFOCUSINDEX_ESPIONAGE, getBuildingCommerceValue(eBuilding, COMMERCE_ESPIONAGE, aiFreeSpecialistYield, aiFreeSpecialistCommerce, aiBaseCommerceRate, aiPlayerCommerceRate),true);
				}

				if (!isHuman())
				{
					valuesCache->AccumulateToAny(kBuilding.getAIWeight(),false);
				}
				// Flavor calculation is non-linear and cannot be calculated in the caching - it is calculated post-cache retrieval
			}
		}
	}
}

int CvCityAI::getBuildingCommerceValue(BuildingTypes eBuilding, int iI, int* aiFreeSpecialistYield, int* aiFreeSpecialistCommerce, int* aiBaseCommerceRate, int* aiPlayerCommerceRate)
{
	int iResult = 0;
	int iJ;
	CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);
	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	int iLimitedWonderLimit = limitedWonderClassLimit((BuildingClassTypes)kBuilding.getBuildingClassType());
	bool bCulturalVictory1 = kOwner.AI_isDoVictoryStrategy(AI_VICTORY_CULTURE1);
	bool bCulturalVictory2 = kOwner.AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2);
	bool bCulturalVictory3 = kOwner.AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3);
	ReligionTypes eStateReligion = kOwner.getStateReligion();

	int iBaseCommerceChange = GC.getBuildingInfo(eBuilding).getCommerceChange((CommerceTypes)iI);

	if ( iBaseCommerceChange < 0 && iI == COMMERCE_GOLD && GC.getDefineINT("TREAT_NEGATIVE_GOLD_AS_MAINTENANCE") )
	{
		iBaseCommerceChange = 0;
	}
	iResult += ((iBaseCommerceChange + GET_TEAM(getTeam()).getBuildingCommerceChange(eBuilding, (CommerceTypes)iI) + aiFreeSpecialistCommerce[iI]) * 4);

	iResult += (kBuilding.getObsoleteSafeCommerceChange(iI) * 4);
	iResult += (kBuilding.getCommerceAttacks(iI) * 2);


	if ((CommerceTypes)iI == COMMERCE_CULTURE)
	{
		// K-Mod. + 2 * foreign culture percent.
		iResult *= 300 - 2 * calculateCulturePercent(getOwner());
		iResult /= 100;
		// K-Mod end
		if (bCulturalVictory1)
		{
			iResult *= 2;
		}
	}

	if (kBuilding.getCommerceChangeDoubleTime(iI) > 0)
	{
		iResult += (iResult*500)/(500+kBuilding.getCommerceChangeDoubleTime(iI));
	}

	//	Multiply up by the city's current commerce rate modifier
	iResult = (iResult*getTotalCommerceRateModifier((CommerceTypes)iI))/100;

	//	Factor in yield changes

/************************************************************************************************/
/* Afforess					  Start		 12/7/09												*/
/*																							  */
/*																							  */
/************************************************************************************************/
	int iTempValue = (((kBuilding.getYieldModifier(YIELD_COMMERCE) + GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, YIELD_COMMERCE)) * getBaseYieldRate(YIELD_COMMERCE)));
	/* Was:
	iTempValue = ((kBuilding.getYieldModifier(YIELD_COMMERCE) * getBaseYieldRate(YIELD_COMMERCE)));
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
	iTempValue *= kOwner.getCommercePercent((CommerceTypes)iI);

	iTempValue /= 3000;

	// if this is a limited wonder, and we are not one of the top 4 in this category, subtract the value
	// we do _not_ want to build this here (unless the value was small anyway)
	if (iLimitedWonderLimit >= 0 && (findCommerceRateRank((CommerceTypes)iI) > (3 + iLimitedWonderLimit)))
	{
		iTempValue *= -1;
	}

	iResult += iTempValue;
/************************************************************************************************/
/* Afforess					  Start		 4/20/10												*/
/*																							  */
/*																							  */
/************************************************************************************************/
	iTempValue = 0;
	iTempValue += ((kBuilding.getYieldModifier(YIELD_COMMERCE) + GET_TEAM(getTeam()).getBuildingYieldModifier(eBuilding, YIELD_COMMERCE)) * getBaseYieldRate(YIELD_COMMERCE) + aiFreeSpecialistYield[YIELD_COMMERCE]) / 8;
	iTempValue += (kBuilding.getYieldChange(YIELD_COMMERCE) + GET_TEAM(getTeam()).getBuildingYieldChange(eBuilding, YIELD_COMMERCE)) * 8;
	for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
	{
		if (hasBonus((BonusTypes)iJ))
		{
			iTempValue += (kBuilding.getBonusYieldModifier(iJ, YIELD_COMMERCE) * getBaseYieldRate(YIELD_COMMERCE) / 12);
			iTempValue += (kBuilding.getBonusYieldChanges(iJ, YIELD_COMMERCE) * 8);
			if (kBuilding.getVicinityBonusYieldChanges(iJ, YIELD_COMMERCE) != 0 && hasVicinityBonus((BonusTypes)iJ))
			{
				iTempValue += (kBuilding.getVicinityBonusYieldChanges(iJ, YIELD_COMMERCE) * 8);
			}
		}
	}
	iTempValue *= GET_PLAYER(getOwnerINLINE()).getCommercePercent((CommerceTypes)iI);
	iTempValue /= 100;
	iResult += iTempValue;

	// add value for a commerce modifier
/************************************************************************************************/
/* Afforess					  Start		 12/7/09												*/
/*																							  */
/*																							  */
/************************************************************************************************/
	int iCommerceModifier = kBuilding.getCommerceModifier(iI) + GET_TEAM(getTeam()).getBuildingCommerceModifier(eBuilding, (CommerceTypes)iI);
	/* Was:
	int iCommerceModifier = kBuilding.getCommerceModifier(iI);
	*/
	for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
	{
		iCommerceModifier += kBuilding.getBonusCommerceModifier(iJ,iI);
	}
/************************************************************************************************/
/* Afforess						 END															*/
/************************************************************************************************/
	if ( aiBaseCommerceRate[iI] == MAX_INT )
	{
		aiBaseCommerceRate[iI] = getBaseCommerceRate((CommerceTypes) iI);
	}
	int iBaseCommerceRate = aiBaseCommerceRate[iI];
	int iCommerceMultiplierValue = iCommerceModifier * iBaseCommerceRate;

	//	Koshling - % modifiers are undervalued by the AI for two reasons:
	//	1)	It only calculates its efect at the time of construction, but that discounts future increases as
	//		base reserach goes up
	//	2)	In the early game because this calculation only takes account of the absolute increase, not its proportion of the
	//		civilization whole, which will be substantial early on (indeed 100% with 1 city).
	//	We therefore apply a small boost to reflect futures, and compare to the whole
	//	and make an upward adjustment proportional to the ratio with the civilization whole.
	//
	//	We don't apply this fo culture, since it isn't really a civ-wide quantity for most purposes
	if ( iCommerceMultiplierValue > 0 && iI != COMMERCE_CULTURE)
	{
		iCommerceMultiplierValue += 120;	//	Discounted futures
		iCommerceMultiplierValue += 2*iCommerceMultiplierValue*std::max(1,aiBaseCommerceRate[iI])/std::max(1,aiPlayerCommerceRate[iI]);	//	Proportion of whole
	}

	if (((CommerceTypes) iI) == COMMERCE_CULTURE && iCommerceModifier != 0)
	{
		int iCultureRateRank = findCommerceRateRank(COMMERCE_CULTURE);
		int iCulturalVictoryNumCultureCities = GC.getGameINLINE().culturalVictoryNumCultureCities();

		if (bCulturalVictory1)
		{
			// if this is one of our top culture cities, then we want to build this here first!
			if (iCultureRateRank <= iCulturalVictoryNumCultureCities)
			{
				iCommerceMultiplierValue /= 8;

				// if we at culture level 3, then these need to get built asap
				if (bCulturalVictory3)
				{
					// its most important to build in the lowest rate city, but important everywhere
					iCommerceMultiplierValue += std::max(100, 500 - iBaseCommerceRate) * iCommerceModifier;
				}
			}
			else
			{
				int iCountBuilt = kOwner.getBuildingClassCountPlusMaking((BuildingClassTypes)kBuilding.getBuildingClassType());

				// do we have enough buildings to build extras?
				bool bHaveEnough = true;

				// if its limited and the limit is less than the number we need in culture cities, do not build here
				if (iLimitedWonderLimit >= 0 && (iLimitedWonderLimit <= iCulturalVictoryNumCultureCities))
				{
					bHaveEnough = false;
				}

				for (iJ = 0; bHaveEnough && iJ < GC.getNumBuildingClassInfos(); iJ++)
				{
					// count excess the number of prereq buildings which do not have this building built for yet
					int iPrereqBuildings = kOwner.getBuildingClassPrereqBuilding(eBuilding, (BuildingClassTypes) iJ, -iCountBuilt);

					// do we not have enough built (do not count ones in progress)
					if (iPrereqBuildings > 0 && kOwner.getBuildingClassCount((BuildingClassTypes) iJ) <  iPrereqBuildings)
					{
						bHaveEnough = false;
					}
				}

				// if we have enough and our rank is close to the top, then possibly build here too
				if (bHaveEnough && (iCultureRateRank - iCulturalVictoryNumCultureCities) <= 3)
				{
					iCommerceMultiplierValue /= 12;
				}
				// otherwise, we really do not want to build this here
				else
				{
					iCommerceMultiplierValue /= 30;
				}
			}
		}
		else
		{
			iCommerceMultiplierValue /= 15;

			// increase priority if we need culture oppressed city
			iCommerceMultiplierValue *= (100 - calculateCulturePercent(getOwnerINLINE()));
		}
	}
	else
	{
		iCommerceMultiplierValue /= 15;
	}
	iResult += iCommerceMultiplierValue;

	iResult += (kBuilding.getGlobalCommerceModifier(iI) * GET_PLAYER(getOwnerINLINE()).getCommerceRate((CommerceTypes) iI))/8;
	//iResult += ((kBuilding.getGlobalCommerceModifier(iI) * kOwner.getNumCities()) / 4);
	iResult += ((kBuilding.getSpecialistExtraCommerce(iI) * kOwner.getTotalPopulation()) / 3);

	if (kBuilding.getSpecialistCommercePercentChanges(-1, -1))
	{
		for (int iSpecialist = 0; iSpecialist < GC.getNumSpecialistInfos(); iSpecialist++)
		{
			iResult += (kBuilding.getSpecialistCommercePercentChanges(iSpecialist, iI) * kOwner.getTotalPopulation()) / 3;
		}
	}

	if (eStateReligion != NO_RELIGION)
	{
		iResult += (kBuilding.getStateReligionCommerce(iI) * kOwner.getHasReligionCount(eStateReligion) * 3);
	}

	if (kBuilding.getGlobalReligionCommerce() != NO_RELIGION)
	{
		iResult += (GC.getReligionInfo((ReligionTypes)(kBuilding.getGlobalReligionCommerce())).getGlobalReligionCommerce(iI) * GC.getGameINLINE().countReligionLevels((ReligionTypes)(kBuilding.getGlobalReligionCommerce())) * 2);
		if (eStateReligion == (ReligionTypes)(kBuilding.getGlobalReligionCommerce()))
		{
			iResult += 10;
		}
	}

	CorporationTypes eCorporation = (CorporationTypes)kBuilding.getFoundsCorporation();
	int iCorpValue = 0;
	if (NO_CORPORATION != eCorporation)
	{
		iCorpValue = kOwner.AI_corporationValue(eCorporation, this);

		for (int iCorp = 0; iCorp < GC.getNumCorporationInfos(); iCorp++)
		{
			if (iCorp != eCorporation)
			{
				if (kOwner.hasHeadquarters((CorporationTypes)iCorp))
				{
					if (GC.getGame().isCompetingCorporation(eCorporation, (CorporationTypes)iCorp))
					{
						if (kOwner.AI_corporationValue((CorporationTypes)iCorp, this) > iCorpValue)
						{
							iCorpValue = -1;
							break;
						}
						else
						{
							if (!isHasCorporation((CorporationTypes)iCorp))
							{
								iCorpValue = -1;
							}
						}
					}
				}
			}
		}

		iResult += iCorpValue / 25;
	}

	if (iCorpValue >= 0)//Don't build if it'll hurt us.
	{
		if (kBuilding.getGlobalCorporationCommerce() != NO_CORPORATION)
		{
			int iGoldValue = (GC.getCorporationInfo((CorporationTypes)(kBuilding.getGlobalCorporationCommerce())).getHeadquarterCommerce(iI) * GC.getGameINLINE().countCorporationLevels((CorporationTypes)(kBuilding.getGlobalCorporationCommerce())) * 2);

			iGoldValue += GC.getCorporationInfo((CorporationTypes)(kBuilding.getGlobalCorporationCommerce())).getHeadquarterCommerce(iI);
			if (iGoldValue > 0)
			{
				iGoldValue += 2 + (kOwner.getNumCities() / 4);
				iGoldValue += std::min(iGoldValue, getBuildingCommerce((CommerceTypes)iI) / 2) / 2;
			}
			iGoldValue *= 2;
			iGoldValue *= getTotalCommerceRateModifier((CommerceTypes)iI);
			iGoldValue *= std::max(50, getTotalCommerceRateModifier((CommerceTypes)iI) - 150);
			iGoldValue /= 5000;
			iCorpValue += iGoldValue;
		}
	}

	if (iCorpValue > 0)
	{
		if (kOwner.isNoCorporations())
		{
			iCorpValue /= 2;
		}
		iResult += iCorpValue;
	}

	if (kBuilding.isCommerceFlexible(iI))
	{
		if (!(kOwner.isCommerceFlexible((CommerceTypes)iI)))
		{
			iResult += 40;
		}
	}

	if (kBuilding.isCommerceChangeOriginalOwner(iI))
	{
		if ((kBuilding.getCommerceChange(iI) > 0) || (kBuilding.getObsoleteSafeCommerceChange(iI) > 0))
		{
			iResult++;
		}
	}

	if ( iI == COMMERCE_GOLD && kOwner.AI_isFinancialTrouble())
	{
		iResult *= 2;
	}

	if (iI == COMMERCE_RESEARCH && kOwner.AI_isDoVictoryStrategy(AI_VICTORY_SPACE1))
	{
		iResult *= 3;
		iResult /= 2;
	}

	//	If we are desperate for SOME culture boost non-trivial producers
	if (iI == COMMERCE_CULTURE && getCommerceRate(COMMERCE_CULTURE) == 0 && AI_calculateTargetCulturePerTurn() == 1)
	{
		if (iResult >= 3)
		{
			iResult += 7;
		}
	}

	//	For research if this is a significant boost to our total (civ wide) then give it significant
	//	extra weight
	if ( iI == COMMERCE_RESEARCH )
	{
		int iPlayerTotal = GET_PLAYER(getOwnerINLINE()).getCommerceRate((CommerceTypes) iI);

		if ( iPlayerTotal <= 0 )
		{
			iPlayerTotal = 1;
		}

		//	iResult should be approx 8*<research gain> so
		//	we are adding a multiplier based on the proportion of the total
		//	doubling our total will multiply by 9 (+= 8*)
		iResult += (iResult*iResult)/iPlayerTotal;
	}

	return iResult;
}

int CvCityAI::tradeRouteValue(CvBuildingInfo& kBuilding, YieldTypes eYield, bool bForeignTrade) const
{
	PROFILE_FUNC();

	int	iExtraTradeRoutes = kBuilding.getTradeRoutes();
	int iCurrentTradeRoutes = getTradeRoutes();
	int	iTradeRouteValue;
	CvPlayerAI&	kOwner = GET_PLAYER(getOwnerINLINE());
	int iValue;

	if ( iCurrentTradeRoutes <= -iExtraTradeRoutes )
	{
		iTradeRouteValue = 0;
	}
	else if ( iCurrentTradeRoutes > 0 )
	{
		iTradeRouteValue = getTradeYield(eYield)/iCurrentTradeRoutes;
	}
	else
	{
		CvCity* pCity = kOwner.getCapitalCity();
		if (!pCity)
		{
			int iLoop = 0;
			pCity = kOwner.firstCity(&iLoop);
		}

		iTradeRouteValue = calculateTradeYield(eYield, calculateTradeProfit(pCity));

		if (kOwner.isNoForeignTrade())
		{
			iTradeRouteValue /= 3;
		}
	}

	iValue = std::min(iExtraTradeRoutes, iCurrentTradeRoutes+iExtraTradeRoutes)*4*iTradeRouteValue;
	iValue += (kBuilding.getCoastalTradeRoutes() * kOwner.countNumCoastalCities()*4*iTradeRouteValue);
	iValue += (kBuilding.getGlobalTradeRoutes() * kOwner.getNumCities()*4*iTradeRouteValue);

	int iUnmodifiedCurrentValue = (100*getTradeYield(eYield))/(100+getTradeRouteModifier());

	iValue += ((kBuilding.getTradeRouteModifier() * iUnmodifiedCurrentValue) / (bForeignTrade ? 12 : 25));
	if (bForeignTrade)
	{
		iValue += ((kBuilding.getForeignTradeRouteModifier() * iUnmodifiedCurrentValue) / 12);
	}

	return iValue;
#ifdef OLD_TRADE_ROUTE_CALCULATION
	iTempValue = ((kBuilding.getTradeRoutes() * ((8 * std::max(0, (totalTradeModifier() + 100))) / 100))
					* (getPopulation() / 5 + 1));
	int iGlobalTradeValue = (((6 * iTotalPopulation) / 5) / iNumCities);
	iTempValue += (kBuilding.getCoastalTradeRoutes() * kOwner.countNumCoastalCities() * iGlobalTradeValue);
	iTempValue += (kBuilding.getGlobalTradeRoutes() * iNumCities * iGlobalTradeValue);

	iTempValue += ((kBuilding.getTradeRouteModifier() * getTradeYield(YIELD_COMMERCE)) / (bForeignTrade ? 12 : 25));
	if (bForeignTrade)
	{
		iTempValue += ((kBuilding.getForeignTradeRouteModifier() * getTradeYield(YIELD_COMMERCE)) / 12);
	}

	if (bFinancialTrouble)
	{
		iTempValue *= 2;
	}

	if (kOwner.isNoForeignTrade())
	{
		iTempValue /= 3;
	}
#endif
}

//	Evaluate a building we are considering building here in terms of its
//	effect on properties
int CvCityAI::buildingPropertiesValue(CvBuildingInfo& kBuilding) const
{
	//	Evaluate building properties
	int iValue = 0;
	int iI;
	std::map<int,int> effectivePropertyChanges;

	CvPropertyManipulators* pBuildingPropertyManipulators = kBuilding.getPropertyManipulators();
	int num = pBuildingPropertyManipulators->getNumSources();
	for (iI = 0; iI < num; iI++)
	{
		CvPropertySource* pSource = pBuildingPropertyManipulators->getSource(iI);

		//	For now we're only interested in constant sources
		//	TODO - expand this as buildings add other types
		if ( pSource->getType() == PROPERTYSOURCE_CONSTANT )
		{
			//	Convert to an effective absolute amount by looking at the steady state value
			//	given current
			PropertyTypes eProperty = pSource->getProperty();
			//	Only count half the unit source as we want to encourage building sources over unit ones
			int iCurrentSourceSize = getTotalBuildingSourcedProperty(eProperty) + getTotalUnitSourcedProperty(eProperty)/2 + getPropertyNonBuildingSource(eProperty);
			int iNewSourceSize = iCurrentSourceSize + ((CvPropertySourceConstant*)pSource)->getAmountPerTurn(getGameObjectConst());
			int iDecayPercent = getPropertyDecay(eProperty);

			//	Steady state occurs at a level where the decay removes as much per turn as the sources add
			//	Decay can be 0 if the current level is below the threshold at which decay cuts in, so for the
			//	purposes of calculation just treat this as very slow decay
			int	iCurrentSteadyStateLevel = (100*iCurrentSourceSize)/std::max(1,iDecayPercent);
			int	iNewSteadyStateLevel = (100*iNewSourceSize)/std::max(1,iDecayPercent);

			std::map<int,int>::iterator itr = effectivePropertyChanges.find(eProperty);
			if ( itr == effectivePropertyChanges.end() )
			{
				effectivePropertyChanges[eProperty] = (iNewSteadyStateLevel - iCurrentSteadyStateLevel);
			}
			else
			{
				itr->second += (iNewSteadyStateLevel - iCurrentSteadyStateLevel);
			}
		}
	}

	const CvProperties* buildingProperties = kBuilding.getProperties();
	num = buildingProperties->getNumProperties();
	for (iI = 0; iI < num; iI++)
	{
		PropertyTypes eProperty = (PropertyTypes)buildingProperties->getProperty(iI);
		int	iBuildingValue = buildingProperties->getValue(iI);

		std::map<int,int>::iterator itr = effectivePropertyChanges.find(eProperty);
		if ( itr == effectivePropertyChanges.end() )
		{
			effectivePropertyChanges[eProperty] = iBuildingValue;
		}
		else
		{
			itr->second += iBuildingValue;
		}
	}

	for(std::map<int,int>::const_iterator itr = effectivePropertyChanges.begin(); itr != effectivePropertyChanges.end(); ++itr)
	{
		iValue += getPropertySourceValue((PropertyTypes)itr->first, itr->second)/100;
	}

	return iValue;
}

int CvCityAI::getPropertySourceValue(PropertyTypes eProperty, int iSourceValue) const
{
	const CvProperties* cityProperties = getPropertiesConst();
	CvPropertyInfo& kProperty = GC.getPropertyInfo(eProperty);
	int	iOperationalLow = kProperty.getOperationalRangeMin();
	int	iOperationalHigh = kProperty.getOperationalRangeMax();
	int iCurrentValue = cityProperties->getValueByProperty(eProperty);
	int iWouldBeValue = iCurrentValue + iSourceValue;
	int iValue;

	//	Normalize to operation range
	int iCurrentNormalizedValue = ((range(iCurrentValue, iOperationalLow, iOperationalHigh) - iOperationalLow)*100)/(iOperationalHigh - iOperationalLow);
	int iWouldBeNormalizedValue = ((range(iWouldBeValue, iOperationalLow, iOperationalHigh) - iOperationalLow)*100)/(iOperationalHigh - iOperationalLow);

	iValue = kProperty.getAIWeight()*(iWouldBeNormalizedValue - iCurrentNormalizedValue);

	//	Increase value as we move towards the nasty end of the range (cautious approach)
	int nastiness;

	if ( kProperty.getAIWeight() > 0 )
	{
		nastiness = 100 - std::min(iCurrentNormalizedValue, iWouldBeNormalizedValue);
	}
	else
	{
		nastiness = std::max(iCurrentNormalizedValue, iWouldBeNormalizedValue);
	}

	iValue *= (nastiness+100);
	iValue /= 100;

	switch(kProperty.getAIScaleType())
	{
		case AISCALE_CITY:
			break;
		case AISCALE_AREA:
			iValue *= area()->getCitiesPerPlayer(getOwnerINLINE());
			break;
		case AISCALE_PLAYER:
			iValue *= GET_PLAYER(getOwnerINLINE()).getNumCities();
			break;
		case AISCALE_TEAM:
			iValue *= GET_TEAM(getTeam()).getNumCities();
			break;
	}

	return iValue;
}

int CvCityAI::getPropertyDecay(PropertyTypes eProperty) const
{
	CvPropertyInfo& kProperty = GC.getPropertyInfo(eProperty);
	CvPropertyManipulators* pManipulators = kProperty.getPropertyManipulators();
	int	iTotalDecay = 0;
	int current = getPropertiesConst()->getValueByProperty(eProperty);
	bool bNoDecayCounted = true;
	int	iLowestThreshold = MAX_INT;
	int iLowestThresholdDecay = 0;

	for(int iI = 0; iI < pManipulators->getNumSources(); iI++)
	{
		CvPropertySource* pSource = pManipulators->getSource(iI);

		if ( pSource->getType() == PROPERTYSOURCE_DECAY )
		{
			CvPropertySourceDecay* pDecaySource = (CvPropertySourceDecay*)pSource;

			//	For AI evaluation purposes the no-decay threshold is largely
			//	irrelevant since we are only interested in EVENTUAL equilibrium
			//	points.  If there are multiple decay sources we should count
			//	only those in current force OR the single lowest is none are
			int iNoDecayThreshold = pDecaySource->getNoDecayAmount();
			if ( current > iNoDecayThreshold )
			{
				iTotalDecay += pDecaySource->getPercent();
			}
			else if ( iTotalDecay == 0 )
			{
				iLowestThreshold = iNoDecayThreshold;
				iLowestThresholdDecay = pDecaySource->getPercent();
			}
		}
	}

	if ( iTotalDecay == 0 )
	{
		iTotalDecay = iLowestThresholdDecay;
	}

	return iTotalDecay;
}

int CvCityAI::getPropertyNonBuildingSource(PropertyTypes eProperty) const
{
	CvPropertyInfo& kProperty = GC.getPropertyInfo(eProperty);
	CvPropertyManipulators* pManipulators = kProperty.getPropertyManipulators();
	int	iTotal = 0;
	int current = getPropertiesConst()->getValueByProperty(eProperty);

	for(int iI = 0; iI < pManipulators->getNumSources(); iI++)
	{
		CvPropertySource* pSource = pManipulators->getSource(iI);

		if ( pSource->getType() == PROPERTYSOURCE_ATTRIBUTE_CONSTANT &&
			 pSource->getObjectType() == GAMEOBJECT_CITY &&
			 pSource->getProperty() == eProperty )
		{
			CvPropertySourceAttributeConstant* pCitySource = (CvPropertySourceAttributeConstant*)pSource;

			iTotal += pCitySource->getSourcePredict(getGameObjectConst(), current);
		}
	}

	//	Add in contribution from specialists
	for(int iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		int iNum = getSpecialistCount((SpecialistTypes)iI) + getFreeSpecialistCount((SpecialistTypes)iI);

		if ( iNum > 0 )
		{
			CvPropertyManipulators* pMani = GC.getSpecialistInfo((SpecialistTypes)iI).getPropertyManipulators();
			int iContribution = 0;

			for (int i=0; i < pMani->getNumSources(); i++)
			{
				CvPropertySource* pSource = pMani->getSource(i);

				//	Sources that deliver to the city or the plot are both considered since the city plot diffuses
				//	to the city for most properties anyway
				if (pSource->getType() == PROPERTYSOURCE_CONSTANT &&
					(pSource->getObjectType() == GAMEOBJECT_CITY || pSource->getObjectType() == GAMEOBJECT_PLOT) &&
					pSource->getProperty() == eProperty)
				{
					iContribution += ((CvPropertySourceConstant*)pSource)->getAmountPerTurn(getGameObjectConst());
				}
			}

			iTotal += iNum*iContribution;
		}
	}

	//	Add oplayer-based/global sources (population, traits, handicap, ...)
	iTotal += getGlobalSourcedProperty(eProperty);

	return iTotal;
}

int CvCityAI::AI_getBuildPriority(void) const
{
	return m_iBuildPriority;
}

bool CvCityAI::AI_choosePropertyControlUnit(int iTriggerPercentOfPropertyOpRange)
{
	CvProperties* pProperties = getGameObject()->getProperties();

	UnitTypes eProductionUnit = getProductionUnit();
	if (eProductionUnit != NO_UNIT)
	{ // xUPT (dbkblk, 2015-02)
		if (GC.getUnitInfo(eProductionUnit).isMilitaryProduction() && (GC.getUnitInfo(eProductionUnit).getDomainType() == DOMAIN_LAND) && m_bAreaSaturatedOfLandMilitaryUnits)
		{
			return false;
		}
		if (!GC.getUnitInfo(eProductionUnit).isMilitaryProduction() && (GC.getUnitInfo(eProductionUnit).getDomainType() != DOMAIN_SEA) && m_bAreaSaturatedOfCivilianUnits)
		{
			return false;
		}
	}

	for( int iI= 0; iI < pProperties->getNumProperties(); iI++)
	{
		PropertyTypes eProperty = (PropertyTypes)pProperties->getProperty(iI);

		//	Is this a bad property? (right now we only try to control bad props with unit orders)
		if ( GC.getPropertyInfo(eProperty).getAIWeight() < 0 )
		{
			int iCurrentValue = pProperties->getValue(iI);
			int iOperationRangeMin = GC.getPropertyInfo(eProperty).getOperationalRangeMin();
			int iOperationRangeMax = GC.getPropertyInfo(eProperty).getOperationalRangeMax();
			int iCurrentPercent = ((iCurrentValue - iOperationRangeMin)*100)/(iOperationRangeMax - iOperationRangeMin);

			//	Don't bother trying to build units for hopelessly out-of-control properties
			//	or else we'll spam units endlessly in cases we cannot really control
			if ( iCurrentPercent > iTriggerPercentOfPropertyOpRange && iCurrentPercent < 110 )
			{
				//	Order a suitable unit if possible
				CvUnitSelectionCriteria criteria;
				CvString	str;

				criteria.m_eProperty = eProperty;
				criteria.m_bPropertyBeneficial = true;

				str.Format("Property control for %S", GC.getPropertyInfo(eProperty).getDescription());
				return AI_chooseUnit(str.c_str(), UNITAI_CITY_DEFENSE, -1, -1, -1, &criteria);
			}
		}
	}

	return false;
}

int CvCityAI::happynessValue(int iAddedHappyness, int iBaseHappinessLevel, int iBaseHealthLevel)
{
	int iTempHealthLevel = iBaseHealthLevel - (getEspionageHealthCounter() / 2) + std::max(0, iAddedHappyness);
	int iTempHappinessLevel = iBaseHappinessLevel - (getEspionageHappinessCounter() / 2);
	int iTempCurrentHappinessLevel = iBaseHappinessLevel;
	int iTempHappyChangeValue = 0;
	bool bIsNegative = false;
	int iValue = 0;

	if (iAddedHappyness < 0)
	{
		bIsNegative = true;
		iAddedHappyness = -iAddedHappyness;
	}
	if (iAddedHappyness != 0)
	{
		for (int iI = 0; iI < iAddedHappyness; iI++)
		{
			if (bIsNegative) { iTempHappinessLevel--; iTempCurrentHappinessLevel--; iTempHappyChangeValue += 1;}

			//The Value of Happiness
			if (iTempHappinessLevel < 0)
			{
				iTempHappyChangeValue += 10;
				if (bIsNegative) {
					iTempHappyChangeValue += 9;
					if (iTempCurrentHappinessLevel < 0)
					{
						iTempHappyChangeValue += 10;
					}
				}
			}
			else if (iTempHappinessLevel <= iTempHealthLevel && iTempHappinessLevel <= 6) {
				iTempHappyChangeValue += 6;
				if (bIsNegative) { iTempHappyChangeValue += 3;}
			}
			else if (iTempHappinessLevel < 10 ) { iTempHappyChangeValue += 3; }
			else { iTempHappyChangeValue += 1; }

			if (!bIsNegative) { iTempHappinessLevel++; iTempCurrentHappinessLevel++; }
		}

		if (bIsNegative)
		{
			iValue -= iTempHappyChangeValue;

			//	If we're already unhappy then exach extar unhappy will force
			//	use to remove a worked plot as well
			if ( iBaseHappinessLevel <= 0 )
			{
				iValue -= (iAddedHappyness*worstWorkedPlotValue())/100;
			}
		}
		else
		{
			iValue += iTempHappyChangeValue;
		}
	}

	return iValue;
}

int CvCityAI::healthValue(int iAddedHealth, int iUseHappinessLevel, int iBaseHealthLevel, int iBaseFoodDifference)
{
	int iTempHappinessLevel = iUseHappinessLevel;
	int iTempHealthLevel = (goodHealth() - badHealth(/*bNoAngry*/ false, std::max(0, (iTempHappinessLevel + 1) / 2))) + ((getEspionageHealthCounter() + 1) / 2 );
	int iTempCurrentHealthLevel = iBaseHealthLevel;
	int iTempHealthChangeValue = 0;
	bool bIsNegative = false;
	int iValue = 0;

	if (iAddedHealth < 0)
	{
		bIsNegative = true;
		iAddedHealth = -iAddedHealth;
	}
	if (iAddedHealth != 0)
	{
		for (int iI = 0; iI < iAddedHealth; iI++)
		{
			if (bIsNegative) { iTempHealthLevel--; iTempCurrentHealthLevel--; iTempHealthChangeValue += 1;}

			//Health Values
			if (iTempCurrentHealthLevel < 0) {
				iTempHealthChangeValue += 10;
				if (bIsNegative) {
					iTempHealthChangeValue += 9;
					if(std::max(0,iBaseFoodDifference-1) < -iTempCurrentHealthLevel)
					{
						iTempHealthChangeValue += 10;
					}
				}
			}
			else if (iTempHealthLevel < 0) { iTempHealthChangeValue += 8; }
			else if (iTempHealthLevel < iTempHappinessLevel && iTempHealthLevel <= 4) {
				iTempHealthChangeValue += 4;
				if (bIsNegative) { iTempHealthChangeValue += 1;}
			}
			else if (iTempHealthLevel < 8 ) { iTempHealthChangeValue += 2; }
			else if (!bIsNegative) { iTempHealthChangeValue += 1; }

			if (!bIsNegative) { iTempHealthLevel++; iTempCurrentHealthLevel++; }
		}
		if (bIsNegative)
		{
			iValue -= iTempHealthChangeValue;
		}
		else
		{
			iValue += iTempHealthChangeValue;
		}
	}

	return iValue;
}

int CvCityAI::worstWorkedPlotValue()
{
	int iWorstPlotValue = MAX_INT;
	int iWorstPlot = -1;

	for (int iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		if (iI != CITY_HOME_PLOT)
		{
			if (isWorkingPlot(iI))
			{
				CvPlot* pLoopPlot = getCityIndexPlot(iI);

				if (pLoopPlot != NULL)
				{
					int iValue = AI_plotValue(pLoopPlot, false, /*bRemove*/ false, /*bIgnoreFood*/ false, false);

					if (iValue < iWorstPlotValue)
					{
						iWorstPlotValue = iValue;
						iWorstPlot = iI;
					}
				}
			}
		}
	}

	return (iWorstPlotValue == MAX_INT ? 0 : iWorstPlotValue);
}

//Values between 0 (no defensive spy needed) to 1000 (absolute top priority)
int CvCityAI::AI_getDefensiveSpyValue() const
{
	if (isCapital())
	{
		return 1000;
	}

	//We are building a space component
	if (GET_PLAYER(getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_SPACE4))
	{
		if (isProductionProject())
		{
			return 1000;
		}
	}

	int iValue = getPopulation() * 10;

	iValue += getNumWorldWonders() * 25;

	iValue += (getNumTeamWonders() + getNumNationalWonders()) * 10;

	iValue += getNumBuildings();

	iValue += getYieldRate(YIELD_PRODUCTION) * 2;

	if (GC.isSS_ASSASSINATE())
	{
		iValue += getNumGreatPeople() * 25;
	}
	else
	{
		iValue += getNumGreatPeople() * 5;
	}

	return std::max(0, std::min(1000, iValue));
}

int CvCityAI::AI_maxTheoriticalPopulation() const
{
	int iFood = getYieldRate(YIELD_FOOD);

	for (int iI = 0; iI < getNumCityPlots(); iI++)
	{
		CvPlot* pLoopPlot = getCityIndexPlot(iI);
		if (pLoopPlot != NULL && !pLoopPlot->isBeingWorked() && canWork(pLoopPlot))
		{
			iFood += pLoopPlot->calculateYield(YIELD_FOOD, false);
		}
	}

	return iFood / GC.getFOOD_CONSUMPTION_PER_POPULATION();
}
