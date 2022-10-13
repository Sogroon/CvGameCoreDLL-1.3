// game.cpp


#include "CvGameCoreDLL.h"
#include "CvGameCoreUtils.h"
#include "CvGame.h"
#include "CvGameAI.h"
#include "CvMap.h"
#include "CvViewport.h"
#include "CvPlot.h"
#include "CvPlayerAI.h"
#include "CvRandom.h"
#include "CvTeamAI.h"
#include "CvGlobals.h"
#include "CvInitCore.h"
#include "CvMapGenerator.h"
#include "CvArtFileMgr.h"
#include "CvDiploParameters.h"
#include "CvReplayMessage.h"
#include "CyArgsList.h"
#include "CvInfos.h"
#include "CvPopupInfo.h"
#include "FProfiler.h"
#include "CvReplayInfo.h"
#include "CvGameTextMgr.h"
#include "CvBugOptions.h"
#include <set>
#include "CvEventReporter.h"
#include "CvMessageControl.h"
#include "CvTaggedSaveFormatWrapper.h"
#include "FDataStreamBuffer.h"

#define BOOST_THREAD_NO_LIB
#define BOOST_THREAD_USE_LIB
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

// interface uses
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLEngineIFaceBase.h"
#include "CvDLLPythonIFaceBase.h"

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/02/09                                jdog5000      */
/*                                                                                              */
/* AI logging                                                                                   */
/************************************************************************************************/
#include "BetterBTSAI.h"
#include "CvMessageData.h"

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
// BUG - start
// RevolutionDCM - BugMod included in CvGlobals.h
//#include "BugMod.h"
// BUG - end

//	Koshling - save game compataibility between (most) builds
//	UI flag values in game serialization.  These are bitwise combinable
#define	GAME_SAVE_UI_FLAG_VALUE_AND_BASE		0x00000001

// Public Functions...

#pragma warning( disable : 4355 )
CvGame::CvGame() : m_GameObject(),
m_Properties(this)
, m_iLastLookatX(-1)
, m_iLastLookatY(-1)
, m_bWasGraphicsPagingEnabled(false)
, m_eCurrentMap(INITIAL_MAP)

{
	InitializeCriticalSectionAndSpinCount(&m_reportableEventsSection, 5000);

	m_aiRankPlayer = new int[MAX_PLAYERS];        // Ordered by rank...
	m_aiPlayerRank = new int[MAX_PLAYERS];        // Ordered by player ID...
	m_aiPlayerScore = new int[MAX_PLAYERS];       // Ordered by player ID...
	m_aiRankTeam = new int[MAX_TEAMS];						// Ordered by rank...
	m_aiTeamRank = new int[MAX_TEAMS];						// Ordered by team ID...
	m_aiTeamScore = new int[MAX_TEAMS];						// Ordered by team ID...

	m_paiUnitCreatedCount = NULL;
	m_paiUnitClassCreatedCount = NULL;
	m_paiBuildingClassCreatedCount = NULL;
	m_paiProjectCreatedCount = NULL;
	m_paiForceCivicCount = NULL;
	m_paiVoteOutcome = NULL;
	m_paiReligionGameTurnFounded = NULL;
	m_paiCorporationGameTurnFounded = NULL;
	m_aiSecretaryGeneralTimer = NULL;
	m_aiVoteTimer = NULL;
	m_aiDiploVote = NULL;

	m_pabSpecialUnitValid = NULL;
	m_pabSpecialBuildingValid = NULL;
	m_abReligionSlotTaken = NULL;
/************************************************************************************************/
/* RevDCM	                  Start		 4/29/10                                                */
/*                                                                                              */
/* OC_LIMITED_RELIGIONS                                                                         */
/************************************************************************************************/
	m_abTechCanFoundReligion = NULL;
/************************************************************************************************/
/* LIMITED_RELIGIONS               END                                                          */
/************************************************************************************************/

	m_paHolyCity = NULL;
	m_paHeadquarters = NULL;
/************************************************************************************************/
/* Afforess                                     12/7/09                                         */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	for (int iI = 0; iI < MAX_TEAMS; iI++)
	{
	  starshipLaunched[iI] = false;
	  diplomaticVictoryAchieved[iI] = false;		
	  AscensionGateBuilt[iI] = false; // code added for Ascension Gate & Mastery Victory (by 45°)
	}
	
	m_abPreviousRequest = new bool[MAX_PLAYERS];
	m_aiModderGameOption = new int[NUM_MODDERGAMEOPTION_TYPES];
	m_aiFlexibleDifficultyTimer = new int[MAX_PLAYERS];
	m_resyncPackets.clear();
	m_resyncReceiveBuffer.clear();
	m_pDarkAgeState = NULL;
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	

	m_pReplayInfo = NULL;

	m_aiShrineBuilding = NULL;
	m_aiShrineReligion = NULL;

	//m_bDeferVoteSourceProcessing = false;
	//m_paiDeferredVoteSources = NULL;

	m_bRecalculatingModifiers = false;

	reset(NO_HANDICAP, true);
}


CvGame::~CvGame()
{
	uninit();

	SAFE_DELETE_ARRAY(m_aiRankPlayer);
	SAFE_DELETE_ARRAY(m_aiPlayerRank);
	SAFE_DELETE_ARRAY(m_aiPlayerScore);
	SAFE_DELETE_ARRAY(m_aiRankTeam);
	SAFE_DELETE_ARRAY(m_aiTeamRank);
	SAFE_DELETE_ARRAY(m_aiTeamScore);
/************************************************************************************************/
/* Afforess	                  Start		 05/19/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	SAFE_DELETE_ARRAY(m_abPreviousRequest);
	SAFE_DELETE_ARRAY(m_aiModderGameOption);
	SAFE_DELETE_ARRAY(m_aiFlexibleDifficultyTimer);
	SAFE_DELETE(m_pDarkAgeState);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	DeleteCriticalSection(&m_reportableEventsSection);
}

void CvGame::init(HandicapTypes eHandicap)
{
	bool bValid;
	int iStartTurn;
	int iEstimateEndTurn;
	int iI;

	//--------------------------------
	// Init saved data
	reset(eHandicap);

	//--------------------------------
	// Init containers
	m_deals.init();
	m_voteSelections.init();
	m_votesTriggered.init();

	m_mapRand.init(GC.getInitCore().getMapRandSeed() % 73637381);
	//Afforess: disable map rand logging
	//m_mapRand.m_bLoggingEnabled = false;
	//GC.getASyncRand().m_bLoggingEnabled = false;
	m_sorenRand.init(GC.getInitCore().getSyncRandSeed() % 52319761);
	//m_sorenRand.m_bLoggingEnabled = true;

	//--------------------------------
	// Init non-saved data

	//--------------------------------
	// Init other game data

	// Turn off all MP options if it's a single player game
	if (GC.getInitCore().getType() == GAME_SP_NEW ||
		GC.getInitCore().getType() == GAME_SP_SCENARIO)
	{
		for (iI = 0; iI < NUM_MPOPTION_TYPES; ++iI)
		{
			setMPOption((MultiplayerOptionTypes)iI, false);
		}
	}

	// If this is a hot seat game, simultaneous turns is always off
	if (isHotSeat() || isPbem())
	{
		setMPOption(MPOPTION_SIMULTANEOUS_TURNS, false);
	}
	// If we didn't set a time in the Pitboss, turn timer off
	if (isPitboss() && getPitbossTurnTime() == 0)
	{
		setMPOption(MPOPTION_TURN_TIMER, false);
	}

	if (isMPOption(MPOPTION_SHUFFLE_TEAMS))
	{
		int aiTeams[MAX_CIV_PLAYERS];

		int iNumPlayers = 0;
		for (int i = 0; i < MAX_CIV_PLAYERS; i++)
		{
			if (GC.getInitCore().getSlotStatus((PlayerTypes)i) == SS_TAKEN)
			{
				aiTeams[iNumPlayers] = GC.getInitCore().getTeam((PlayerTypes)i);
				++iNumPlayers;
			}
		}

		for (int i = 0; i < iNumPlayers; i++)
		{
			int j = (getSorenRand().get(iNumPlayers - i, NULL) + i);

			if (i != j)
			{
				int iTemp = aiTeams[i];
				aiTeams[i] = aiTeams[j];
				aiTeams[j] = iTemp;
			}
		}

		iNumPlayers = 0;
		for (int i = 0; i < MAX_CIV_PLAYERS; i++)
		{
			if (GC.getInitCore().getSlotStatus((PlayerTypes)i) == SS_TAKEN)
			{
				GC.getInitCore().setTeam((PlayerTypes)i, (TeamTypes)aiTeams[iNumPlayers]);
				++iNumPlayers;
			}
		}
	}

	//TB GameOption compatability enforcement project
	for (iI = 0; iI < NUM_GAMEOPTION_TYPES; iI++)
	{
		GameOptionTypes eGameOption = ((GameOptionTypes)iI);
		if (isOption(eGameOption))
		{
			if (GC.getGameOptionInfo(eGameOption).getNumEnforcesGameOptionOnTypes() > 0)
			{
				for (int iJ = 0; iJ < GC.getGameOptionInfo(eGameOption).getNumEnforcesGameOptionOnTypes(); iJ++)
				{
					GameOptionTypes eGameOptionMustOn = ((GameOptionTypes)GC.getGameOptionInfo(eGameOption).isEnforcesGameOptionOnType(iJ).eGameOption);
					if (GC.getGameOptionInfo(eGameOption).isEnforcesGameOptionOnType(iJ).bBool && !isOption(eGameOptionMustOn))
					{
						setOption(eGameOptionMustOn, true);
					}
				}
			}
			if (GC.getGameOptionInfo(eGameOption).getNumEnforcesGameOptionOffTypes() > 0)
			{
				for (int iJ = 0; iJ < GC.getGameOptionInfo(eGameOption).getNumEnforcesGameOptionOffTypes(); iJ++)
				{
					GameOptionTypes eGameOptionMustOff = ((GameOptionTypes)GC.getGameOptionInfo(eGameOption).isEnforcesGameOptionOffType(iJ).eGameOption);
					if (GC.getGameOptionInfo(eGameOption).isEnforcesGameOptionOffType(iJ).bBool && isOption(eGameOptionMustOff))
					{
						setOption(eGameOptionMustOff, false);
					}
				}
			}
		}
	}

	if (isOption(GAMEOPTION_LOCK_MODS))
	{
		if (isGameMultiPlayer())
		{
			setOption(GAMEOPTION_LOCK_MODS, false);
		}
		else
		{
			static const int iPasswordSize = 8;
			char szRandomPassword[iPasswordSize];
			for (int i = 0; i < iPasswordSize-1; i++)
			{
				szRandomPassword[i] = getSorenRandNum(128, NULL);
			}
			szRandomPassword[iPasswordSize-1] = 0;

			GC.getInitCore().setAdminPassword(szRandomPassword);
		}
	}

	if (getGameTurn() == 0)
	{
		iStartTurn = 0;

		for (iI = 0; iI < GC.getGameSpeedInfo(getGameSpeedType()).getNumTurnIncrements(); iI++)
		{
			iStartTurn += GC.getGameSpeedInfo(getGameSpeedType()).getGameTurnInfo(iI).iNumGameTurnsPerIncrement;
		}

		iStartTurn *= GC.getEraInfo(getStartEra()).getStartPercent();
		iStartTurn /= 100;

		setGameTurn(iStartTurn);
	}

	setStartTurn(getGameTurn());

	if (getMaxTurns() == 0)
	{
		iEstimateEndTurn = 0;

		for (iI = 0; iI < GC.getGameSpeedInfo(getGameSpeedType()).getNumTurnIncrements(); iI++)
		{
			iEstimateEndTurn += GC.getGameSpeedInfo(getGameSpeedType()).getGameTurnInfo(iI).iNumGameTurnsPerIncrement;
		}

		setEstimateEndTurn(iEstimateEndTurn);
/************************************************************************************************/
/* Afforess                                     12/7/09                                         */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
#if defined QC_MASTERY_VICTORY
//Sevo Begin--VCM
	// This looks like a reasonable place for me to place my own interrupts for the victory conditions
	// I need to ensure that the AI understands it has to go after everything, so ALL the victory options
	// need to be "enabled" so starships can be built, etc. etc.
	//Only exception is diplo victory, but whatever.

	
		if (getEstimateEndTurn() > getGameTurn())
		{
			bValid = false;

			for (iI = 0; iI < GC.getNumVictoryInfos(); iI++)
			{
				if (isVictoryValid((VictoryTypes)iI))
				{
					if ((GC.getVictoryInfo((VictoryTypes)iI).isEndScore()) || (GC.getVictoryInfo((VictoryTypes)iI).isTotalVictory()))
					{
						bValid = true;
						break;
					}
				}
			}

			if (bValid)
			{
				setMaxTurns(getEstimateEndTurn() - getGameTurn());
			}
		}
	}
	else
	{
		setEstimateEndTurn(getGameTurn() + getMaxTurns());
	}

	// Okay, now we'll actually set up the other VC's to agree with the total victory
	// If Total Victory is enabled, TURN ON ALL OTHERS AS WELL
	// This allows starship construction, gets the AI thinking correctly, etc.
	// The TV condition overrides the others but requries they be "on"
	bValid = false;
	for (iI = 0; iI < GC.getNumVictoryInfos(); iI++)
	{
		if ((GC.getVictoryInfo((VictoryTypes)iI).isTotalVictory()) && (isVictoryValid((VictoryTypes)iI)))
		{
			bValid = true;
			break;
		}
	}

	if (bValid)
	{
		for (iI = 0; iI < GC.getNumVictoryInfos(); iI++)
		{
			setVictoryValid((VictoryTypes)iI, true);
		}
	}

//Sevo End VCM
#else
  }
#endif

	m_plotGroupHashesInitialized = false;
	
	//resetOptionEdits();
	//setOptionEdits();

	//Ruthless AI means Aggressive AI is on too.
	if (isOption(GAMEOPTION_RUTHLESS_AI) && !isOption(GAMEOPTION_AGGRESSIVE_AI))
	{
		setOption(GAMEOPTION_AGGRESSIVE_AI, true);
	}
		
	if (!isOption(GAMEOPTION_GREAT_COMMANDERS))
	{
		for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
		{
			if (GC.getUnitInfo((UnitTypes)iI).isGreatGeneral())
			{
				GC.getUnitInfo((UnitTypes)iI).setPowerValue(GC.getUnitInfo((UnitTypes)iI).getPowerValue() / 10);
			}
		}
	}

	logMsg("Handicap Era Tech Research Cost Modifier: %d", GC.getHandicapInfo(eHandicap).getResearchPercent());
	logMsg("World Size Tech Research Cost Modifier: %d", GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getResearchPercent());
	logMsg("Game Speed Tech Research Cost Modifier: %d", GC.getGameSpeedInfo(getGameSpeedType()).getResearchPercent());
	logMsg("Starting Era Tech Research Cost Modifier: %d", GC.getEraInfo(GC.getGameINLINE().getStartEra()).getResearchPercent());
	logMsg("Global Defines Tech Research Cost Modifier: %d", GC.getTECH_COST_MODIFIER());
	for (iI = 0; iI < GC.getNumEraInfos(); iI++)
	{
		logMsg("Era [%S] Tech Research Cost Modifier: %d", GC.getEraInfo((EraTypes)iI).getDescription(), GC.getEraInfo((EraTypes)iI).getTechCostModifier());
	}
	logMsg(" -------------------------- ");
	for (int iEra = 0; iEra < GC.getNumEraInfos(); iEra++)
	{
		int iTotalBaseCost = 0;
		int iTotalFinalCost = 0;
		int iTechs = 0;
		for (iI = 0; iI < GC.getNumTechInfos(); iI++)
		{
			if (GC.getTechInfo((TechTypes)iI).getEra() == iEra)
			{
				int iBaseCost = GC.getTechInfo((TechTypes)iI).getBaseResearchCost();
				int iFinalCost = GC.getTechInfo((TechTypes)iI).getResearchCost();

				iFinalCost *= GC.getHandicapInfo(eHandicap).getResearchPercent();
				iFinalCost /= 100;

				iFinalCost *= GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getResearchPercent();
				iFinalCost /= 100;

				iFinalCost *= GC.getGameSpeedInfo(getGameSpeedType()).getResearchPercent();
				iFinalCost /= 100;

				iFinalCost *= GC.getEraInfo(getStartEra()).getResearchPercent();
				iFinalCost /= 100;

				logMsg("\tTech [%S] Base Cost: %d, \t\t\t\tFinal Cost: %d", GC.getTechInfo((TechTypes)iI).getDescription(), iBaseCost, iFinalCost);

				iTotalBaseCost += iBaseCost;
				iTotalFinalCost += iFinalCost;
				iTechs++;
			}
		}
		logMsg("Era [%S] Total Base Research Cost: %d, Total Research Cost: %d, Era Techs: %d, Average Tech Cost: %d", GC.getEraInfo((EraTypes)iEra).getDescription(), iTotalBaseCost, iTotalFinalCost, iTechs, iTotalFinalCost / iTechs);
	}
	
	setFutureEras();
	
	if (isOption(GAMEOPTION_UNITED_NATIONS))
	{
		//Find the diplomatic victory
		BuildingTypes eUnitedNations = NO_BUILDING;
		for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
		{
			if (GC.getBuildingInfo((BuildingTypes)iI).getVictoryPrereq() != NO_VICTORY)
			{
				if (GC.getBuildingInfo((BuildingTypes)iI).getVoteSourceType() != NO_VOTESOURCE)
				{
					eUnitedNations = (BuildingTypes)iI;
					break;
				}
			}
		}
		if (eUnitedNations != NO_BUILDING)
		{
			m_bDiploVictoryEnabled = isVictoryValid((VictoryTypes)GC.getBuildingInfo(eUnitedNations).getVictoryPrereq());
			setVictoryValid((VictoryTypes)GC.getBuildingInfo(eUnitedNations).getVictoryPrereq(), true);
		}
	}

	m_resyncPackets.clear();
	m_resyncReceiveBuffer.clear();
	m_bOutOfSync = false;
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	
	setStartYear(GC.getDefineINT("START_YEAR"));
	m_iDateTurn = -500;

	for (iI = 0; iI < GC.getNumSpecialUnitInfos(); iI++)
	{
		if (GC.getSpecialUnitInfo((SpecialUnitTypes)iI).isValid())
		{
			makeSpecialUnitValid((SpecialUnitTypes)iI);
		}
	}

	for (iI = 0; iI < GC.getNumSpecialBuildingInfos(); iI++)
	{
		if (GC.getSpecialBuildingInfo((SpecialBuildingTypes)iI).isValid())
		{
			makeSpecialBuildingValid((SpecialBuildingTypes)iI);
		}
	}

	AI_init();

	doUpdateCacheOnTurn();
}

//
// Set initial items (units, techs, etc...)
//
void CvGame::setInitialItems()
{
	PROFILE_FUNC();

	initFreeState();
	assignStartingPlots();
/************************************************************************************************/
/* Afforess	                  Start		 03/30/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
#define PYCultureLinkModule		"CvCultureLinkInterface"
	if (isOption(GAMEOPTION_CULTURALLY_LINKED_STARTS))
	{
		PYTHON_CALL_FUNCTION2(__FUNCTION__, PYCultureLinkModule, "assignCulturallyLinkedStarts"); 
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/	
	normalizeStartingPlots();
	
// RevolutionDCM - BarbarianWorld - thanks Kael
	// RevDCM TODO: What's right here?
    if (isOption(GAMEOPTION_BARBARIAN_WORLD))
    {
        for (int iI = 0; iI < MAX_PLAYERS; iI++)
        {
            if (GET_PLAYER((PlayerTypes)iI).isAlive() && iI != BARBARIAN_PLAYER)
            {
                foundBarbarianCity();
            }
        }
    }
// RevolutionDCM End

	initFreeUnits();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)i);
		if (kPlayer.isAlive())
		{
			kPlayer.AI_updateFoundValues(true);
		}
	}
/************************************************************************************************/
/* Afforess	                  Start		 03/18/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (isOption(GAMEOPTION_PERSONALIZED_MAP))
	{
		clearLandmarks();
		findMountainRanges();
		findLonePeaks();
		findBays();
		findForests();
		findJungles();
		findDeserts();
		findLakes();
		pruneLandmarks();
		addLandmarkSigns();
		updateInitialSigns();
	}

	logMsg("Handicap Era Tech Research Cost Modifier: %d", GC.getHandicapInfo(m_eHandicap).getResearchPercent());
	logMsg("World Size Tech Research Cost Modifier: %d", GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getResearchPercent());
	logMsg("Game Speed Tech Research Cost Modifier: %d", GC.getGameSpeedInfo(getGameSpeedType()).getResearchPercent());
	logMsg("Starting Era Tech Research Cost Modifier: %d", GC.getEraInfo(GC.getGameINLINE().getStartEra()).getResearchPercent());
	logMsg("Global Defines Tech Research Cost Modifier: %d", GC.getTECH_COST_MODIFIER());
	for (int iI = 0; iI < GC.getNumEraInfos(); iI++)
	{
		logMsg("Era [%S] Tech Research Cost Modifier: %d", GC.getEraInfo((EraTypes)iI).getDescription(), GC.getEraInfo((EraTypes)iI).getTechCostModifier());
	}

	checkInitialResourceAllocations();

/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
}


// BUG - MapFinder - start
// from HOF Mod - Dianthus
bool CvGame::canRegenerateMap() const
{
	if (GC.getGameINLINE().getElapsedGameTurns() != 0) return false;
	if (GC.getGameINLINE().isGameMultiPlayer()) return false;
	if (GC.getInitCore().getWBMapScript()) return false;

	// EF: TODO clear contact at start of regenerateMap()?
	for (int iI = 1; iI < MAX_CIV_TEAMS; iI++)
	{
		CvTeam& team=GET_TEAM((TeamTypes)iI);
		for (int iJ = 0; iJ < iI; iJ++)
		{
			if (team.isHasMet((TeamTypes)iJ)) return false;
		}
	}
	return true;
}
// BUG - MapFinder - end

void CvGame::regenerateMap()
{
	int iI;

	if (GC.getInitCore().getWBMapScript())
	{
		return;
	}

	setFinalInitialized(false);

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		GET_PLAYER((PlayerTypes)iI).killUnits();
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		GET_PLAYER((PlayerTypes)iI).killCities();
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		GET_PLAYER((PlayerTypes)iI).killAllDeals();
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		GET_PLAYER((PlayerTypes)iI).setFoundedFirstCity(false);
		GET_PLAYER((PlayerTypes)iI).setStartingPlot(NULL, false);
	}

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		GC.getMapINLINE().setRevealedPlots(((TeamTypes)iI), false);
	}

	gDLL->getEngineIFace()->clearSigns();

	GC.getMapINLINE().erasePlots();

	CvMapGenerator::GetInstance().generateRandomMap();
	CvMapGenerator::GetInstance().addGameElements();

	gDLL->getEngineIFace()->RebuildAllPlots();

	CvEventReporter::getInstance().resetStatistics();

	setInitialItems();

	initScoreCalculation();
	setFinalInitialized(true);

	GC.getMapINLINE().setupGraphical();
	gDLL->getEngineIFace()->SetDirty(GlobeTexture_DIRTY_BIT, true);
	gDLL->getEngineIFace()->SetDirty(MinimapTexture_DIRTY_BIT, true);
	gDLL->getInterfaceIFace()->setDirty(ColoredPlots_DIRTY_BIT, true);

	gDLL->getInterfaceIFace()->setCycleSelectionCounter(1);
/*************************************************************************************************/
/**	ADDON (regenerate Map) Sephi                                             					**/
/*************************************************************************************************/
    CvEventReporter::getInstance().gameStart();
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	gDLL->getEngineIFace()->AutoSave(true);

// BUG - AutoSave - start
	PYTHON_CALL_FUNCTION2(__FUNCTION__, PYBugModule, "gameStartSave");
// BUG - AutoSave - end

	// EF - This doesn't work until after the game has had time to update.
	//      Centering on the starting location is now done by MapFinder using BugUtil.delayCall().
	//      Must leave this here for non-BUG
	if (NO_PLAYER != getActivePlayer())
	{
		CvPlot* pPlot = GET_PLAYER(getActivePlayer()).getStartingPlot();

		if (NULL != pPlot)
		{
			gDLL->getInterfaceIFace()->lookAt(pPlot->getPoint(), CAMERALOOKAT_NORMAL);
		}
	}
}

void CvGame::uninit()
{
	SAFE_DELETE_ARRAY(m_aiShrineBuilding);
	SAFE_DELETE_ARRAY(m_aiShrineReligion);
	SAFE_DELETE_ARRAY(m_paiUnitCreatedCount);
	SAFE_DELETE_ARRAY(m_paiUnitClassCreatedCount);
	SAFE_DELETE_ARRAY(m_paiBuildingClassCreatedCount);
	SAFE_DELETE_ARRAY(m_paiProjectCreatedCount);
	SAFE_DELETE_ARRAY(m_paiForceCivicCount);
	SAFE_DELETE_ARRAY(m_paiVoteOutcome);
	SAFE_DELETE_ARRAY(m_paiReligionGameTurnFounded);
	SAFE_DELETE_ARRAY(m_paiCorporationGameTurnFounded);
	SAFE_DELETE_ARRAY(m_aiSecretaryGeneralTimer);
	SAFE_DELETE_ARRAY(m_aiVoteTimer);
	SAFE_DELETE_ARRAY(m_aiDiploVote);

	SAFE_DELETE_ARRAY(m_pabSpecialUnitValid);
	SAFE_DELETE_ARRAY(m_pabSpecialBuildingValid);
	SAFE_DELETE_ARRAY(m_abReligionSlotTaken);
/************************************************************************************************/
/* RevDCM	                  Start		 4/29/10                                                */
/*                                                                                              */
/* OC_LIMITED_RELIGIONS                                                                         */
/************************************************************************************************/
	SAFE_DELETE_ARRAY(m_abTechCanFoundReligion);
/************************************************************************************************/
/* LIMITED_RELIGIONS               END                                                          */
/************************************************************************************************/

	SAFE_DELETE_ARRAY(m_paHolyCity);
	SAFE_DELETE_ARRAY(m_paHeadquarters);

	m_aszDestroyedCities.clear();
	m_aszGreatPeopleBorn.clear();
	m_resyncPackets.clear();

	m_deals.uninit();
	m_voteSelections.uninit();
	m_votesTriggered.uninit();

	m_mapRand.uninit();
	m_sorenRand.uninit();

	clearReplayMessageMap();
	SAFE_DELETE(m_pReplayInfo);

	m_aPlotExtraYields.clear();
	m_aPlotExtraCosts.clear();
	m_mapVoteSourceReligions.clear();
	m_aeInactiveTriggers.clear();

#ifdef CAN_BUILD_VALUE_CACHING
	CvPlot::ClearCanBuildCache();
#endif
	resetOptionEdits();
	if (CvPlayerAI::areStaticsInitialized())
	{
		for (int i = 0; i < GC.getNumMapInfos(); i++)
		{
			if ( GC.mapInitialized((MapTypes)i) )
			{
				// Cast to the internal class
				CvMap&	map = GC.getMapByIndex((MapTypes)i);

				CvMapInitData defaultMapData;

				map.reset(&defaultMapData);
			}
		}

		for(int i = 0; i < MAX_PLAYERS; i++)
		{
			GET_PLAYER((PlayerTypes)i).uninit();
		}
	}
	m_allocator.clearAll();
}


// FUNCTION: reset()
// Initializes data members that are serialized.
void CvGame::reset(HandicapTypes eHandicap, bool bConstructorCall)
{
	int iI;

	//--------------------------------
	// Uninit class
	uninit();

	m_iElapsedGameTurns = 0;
	m_iStartTurn = 0;
	m_iStartYear = GC.getDefineINT("START_YEAR");
	m_iEstimateEndTurn = 0;
	m_iDateTurn = -500;
	m_iTurnSlice = 0;
	m_iCutoffSlice = 0;
	m_iNumGameTurnActive = 0;
	m_iNumCities = 0;
	m_iTotalPopulation = 0;
	m_iTradeRoutes = 0;
	m_iFreeTradeCount = 0;
	m_iNoNukesCount = 0;
	m_iNukesExploded = 0;
	m_iMaxPopulation = 0;
	m_iMaxLand = 0;
	m_iMaxTech = 0;
	m_iMaxWonders = 0;
	m_iInitPopulation = 0;
	m_iInitLand = 0;
	m_iInitTech = 0;
	m_iInitWonders = 0;
/************************************************************************************************/
/* Afforess                                     12/7/09                                         */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	m_iCurrentVoteID = 0;
	m_iWaterAnimalSpawnChance = 0;
	m_iXResolution = 1024;
	m_iYResolution = 720;
	m_iMercyRuleCounter = 0;
	m_iCutLosersCounter = 0;
	m_iHighToLowCounter = 0;
	m_iIncreasingDifficultyCounter = 0;
	m_iRiverBuildings = 0;
	m_iNumWonders = 0;
	m_bDiploVictoryEnabled = false;
	m_bAnyoneHasUnitZoneOfControl = false;
	m_iLastResyncTurnSlice = 0; //field is intentionally not saved
	m_iActiveTurnSlice = 0; //field is intentionally not saved
	m_iDarkAgeCount = 0;
	calculateRiverBuildings();
	calculateCoastalBuildings();
	calculateNumWonders();
#if defined QC_MASTERY_VICTORY
	for (int loopI = 0; loopI < MAX_TEAMS; loopI++)
	{
	  starshipLaunched[loopI] = false;
	  diplomaticVictoryAchieved[loopI] = false;		
	  AscensionGateBuilt[loopI] = false; // code added for Ascension Gate & Mastery Victory (by 45°)
	}	
#endif
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	

	m_uiInitialTime = 0;

	m_bScoreDirty = false;
	m_circumnavigatingTeam = NO_TEAM;
	m_bDebugMode = false;
	m_bDebugModeCache = false;
	m_bFinalInitialized = false;
	m_bPbemTurnSent = false;
	m_bHotPbemBetweenTurns = false;
	m_bPlayerOptionsSent = false;
	//TB Nukefix
	//m_bNukesValid = false;

	m_eHandicap = eHandicap;
	// <f1rpo.autoplay>
	m_eAIHandicap = (bConstructorCall ? NO_HANDICAP
			// (XML not loaded when constructor called)
			: (HandicapTypes)GC.getDefineINT("STANDARD_HANDICAP")); // </f1rpo.autoplay>
	// <f1rpo> (era options)
	m_ePenultimateEra = (EraTypes)std::max(0, GC.getNumEraInfos() - 2);
	if (!bConstructorCall)
		updatePenultimateEra(); // </f1rpo>
	m_ePausePlayer = NO_PLAYER;
	m_eBestLandUnit = NO_UNIT;
	m_eWinner = NO_TEAM;
	m_eVictory = NO_VICTORY;
	m_eGameState = GAMESTATE_ON;

	m_szScriptData = "";

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		m_aiRankPlayer[iI] = 0;
		m_aiPlayerRank[iI] = 0;
		m_aiPlayerScore[iI] = 0;
/************************************************************************************************/
/* REVOLUTION_MOD                                                                lemmy101       */
/*                                                                               jdog5000       */
/*                                                                                              */
/************************************************************************************************/
		m_iAIAutoPlay[iI] = 0;
		m_iForcedAIAutoPlay[iI] = 0;
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/
	}

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		m_aiRankTeam[iI] = 0;
		m_aiTeamRank[iI] = 0;
		m_aiTeamScore[iI] = 0;
	}
/************************************************************************************************/
/* Afforess	                  Start		 06/07/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	for (iI = 0; iI < NUM_MODDERGAMEOPTION_TYPES; iI++)
	{
		m_aiModderGameOption[iI] = 0;
	}
	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		m_abPreviousRequest[iI] = false;
		m_aiFlexibleDifficultyTimer[iI] = 0;
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	if (!bConstructorCall)
	{
		FAssertMsg(m_paiUnitCreatedCount==NULL, "about to leak memory, CvGame::m_paiUnitCreatedCount");
		m_paiUnitCreatedCount = new int[GC.getNumUnitInfos()];
		for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
		{
			m_paiUnitCreatedCount[iI] = 0;
		}
		FAssertMsg(m_paiUnitClassCreatedCount==NULL, "about to leak memory, CvGame::m_paiUnitClassCreatedCount");
		m_paiUnitClassCreatedCount = new int[GC.getNumUnitClassInfos()];
		for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
		{
			m_paiUnitClassCreatedCount[iI] = 0;
		}
		FAssertMsg(m_paiBuildingClassCreatedCount==NULL, "about to leak memory, CvGame::m_paiBuildingClassCreatedCount");
		m_paiBuildingClassCreatedCount = new int[GC.getNumBuildingClassInfos()];
		for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
		{
			m_paiBuildingClassCreatedCount[iI] = 0;
		}

		FAssertMsg(m_paiProjectCreatedCount==NULL, "about to leak memory, CvGame::m_paiProjectCreatedCount");
		m_paiProjectCreatedCount = new int[GC.getNumProjectInfos()];
		for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
		{
			m_paiProjectCreatedCount[iI] = 0;
		}

		FAssertMsg(m_paiForceCivicCount==NULL, "about to leak memory, CvGame::m_paiForceCivicCount");
		m_paiForceCivicCount = new int[GC.getNumCivicInfos()];
		for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
		{
			m_paiForceCivicCount[iI] = 0;
		}

		FAssertMsg(0 < GC.getNumVoteInfos(), "GC.getNumVoteInfos() is not greater than zero in CvGame::reset");
		FAssertMsg(m_paiVoteOutcome==NULL, "about to leak memory, CvGame::m_paiVoteOutcome");
		m_paiVoteOutcome = new PlayerVoteTypes[GC.getNumVoteInfos()];
		for (iI = 0; iI < GC.getNumVoteInfos(); iI++)
		{
			m_paiVoteOutcome[iI] = NO_PLAYER_VOTE;
		}

		FAssertMsg(0 < GC.getNumVoteSourceInfos(), "GC.getNumVoteSourceInfos() is not greater than zero in CvGame::reset");
		FAssertMsg(m_aiDiploVote==NULL, "about to leak memory, CvGame::m_aiDiploVote");
		m_aiDiploVote = new int[GC.getNumVoteSourceInfos()];
		for (iI = 0; iI < GC.getNumVoteSourceInfos(); iI++)
		{
			m_aiDiploVote[iI] = 0;
		}

		FAssertMsg(m_pabSpecialUnitValid==NULL, "about to leak memory, CvGame::m_pabSpecialUnitValid");
		m_pabSpecialUnitValid = new bool[GC.getNumSpecialUnitInfos()];
		for (iI = 0; iI < GC.getNumSpecialUnitInfos(); iI++)
		{
			m_pabSpecialUnitValid[iI] = false;
		}

		FAssertMsg(m_pabSpecialBuildingValid==NULL, "about to leak memory, CvGame::m_pabSpecialBuildingValid");
		m_pabSpecialBuildingValid = new bool[GC.getNumSpecialBuildingInfos()];
		for (iI = 0; iI < GC.getNumSpecialBuildingInfos(); iI++)
		{
			m_pabSpecialBuildingValid[iI] = false;
		}

		FAssertMsg(m_paiReligionGameTurnFounded==NULL, "about to leak memory, CvGame::m_paiReligionGameTurnFounded");
		m_paiReligionGameTurnFounded = new int[GC.getNumReligionInfos()];
		FAssertMsg(m_abReligionSlotTaken==NULL, "about to leak memory, CvGame::m_abReligionSlotTaken");
		m_abReligionSlotTaken = new bool[GC.getNumReligionInfos()];
		FAssertMsg(m_paHolyCity==NULL, "about to leak memory, CvGame::m_paHolyCity");
		m_paHolyCity = new IDInfo[GC.getNumReligionInfos()];
		for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
		{
			m_paiReligionGameTurnFounded[iI] = -1;
			m_paHolyCity[iI].reset();
			m_abReligionSlotTaken[iI] = false;
		}
/************************************************************************************************/
/* RevDCM	                  Start		 4/29/10                                                */
/*                                                                                              */
/* OC_LIMITED_RELIGIONS                                                                         */
/************************************************************************************************/
		FAssertMsg(m_abTechCanFoundReligion==NULL, "about to leak memory, CvGame::m_abTechCanFoundReligion");
		m_abTechCanFoundReligion = new bool[GC.getNumTechInfos()];
		for (iI = 0; iI < GC.getNumTechInfos(); iI++)
		{
			m_abTechCanFoundReligion[TechTypes(iI)] = false;
		}
		for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
		{
			TechTypes eIndex = TechTypes(GC.getReligionInfo((ReligionTypes)iI).getTechPrereq());
			m_abTechCanFoundReligion[eIndex] = true;
		}
/************************************************************************************************/
/* LIMITED_RELIGIONS               END                                                          */
/************************************************************************************************/

		FAssertMsg(m_paiCorporationGameTurnFounded==NULL, "about to leak memory, CvGame::m_paiCorporationGameTurnFounded");
		m_paiCorporationGameTurnFounded = new int[GC.getNumCorporationInfos()];
		m_paHeadquarters = new IDInfo[GC.getNumCorporationInfos()];
		for (iI = 0; iI < GC.getNumCorporationInfos(); iI++)
		{
			m_paiCorporationGameTurnFounded[iI] = -1;
			m_paHeadquarters[iI].reset();
		}

		FAssertMsg(m_aiShrineBuilding==NULL, "about to leak memory, CvGame::m_aiShrineBuilding");
		FAssertMsg(m_aiShrineReligion==NULL, "about to leak memory, CvGame::m_aiShrineReligion");
		m_aiShrineBuilding = new int[GC.getNumBuildingInfos()];
		m_aiShrineReligion = new int[GC.getNumBuildingInfos()];
		for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
		{
			m_aiShrineBuilding[iI] = (int) NO_BUILDING;
			m_aiShrineReligion[iI] = (int) NO_RELIGION;
		}

		FAssertMsg(m_aiSecretaryGeneralTimer==NULL, "about to leak memory, CvGame::m_aiSecretaryGeneralTimer");
		FAssertMsg(m_aiVoteTimer==NULL, "about to leak memory, CvGame::m_aiVoteTimer");
		m_aiSecretaryGeneralTimer = new int[GC.getNumVoteSourceInfos()];
		m_aiVoteTimer = new int[GC.getNumVoteSourceInfos()];
		for (iI = 0; iI < GC.getNumVoteSourceInfos(); iI++)
		{
			m_aiSecretaryGeneralTimer[iI] = 0;
			m_aiVoteTimer[iI] = 0;
		}
	}

	m_deals.removeAll();
	m_voteSelections.removeAll();
	m_votesTriggered.removeAll();

	m_mapRand.reset();
	m_sorenRand.reset();

	m_iNumSessions = 1;

	m_iShrineBuildingCount = 0;
	m_iNumCultureVictoryCities = 0;
	m_eCultureVictoryCultureLevel = NO_CULTURELEVEL;

	if (!bConstructorCall)
	{
		AI_reset();
	}
/************************************************************************************************/
/* Afforess/RevDCM                                     12/7/09                                  */
/*                                                                                              */
/* Advanced Automations                                                                         */
/************************************************************************************************/
		// Sanguo Mod Performance start, added by poyuzhe 07.27.09
	UnitTypes eUnit;
	std::vector<UnitTypes> aUpgradeUnits;

	for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
	{
		eUnit = (UnitTypes)iI;
		aUpgradeUnits.clear();
		do
		{
			for (int iJ = 0; iJ < GC.getNumUnitClassInfos(); iJ++)
			{
				if (eUnit != NO_UNIT)
				{
					if (GC.getUnitInfo(eUnit).getUpgradeUnitClass(iJ))
					{
						GC.getUnitInfo((UnitTypes)iI).addUpgradeUnitClassTypes(iJ);
						aUpgradeUnits.push_back((UnitTypes)GC.getUnitClassInfo((UnitClassTypes)iJ).getDefaultUnitIndex());
					}
				}
			}
			if (!aUpgradeUnits.empty())
			{
				if (eUnit != NO_UNIT)
				{
					eUnit = aUpgradeUnits.front();
					aUpgradeUnits.erase(aUpgradeUnits.begin());
				}
			}
			else
			{
				break;
			}
		}while(true);
	}
	// Sanguo Mod Performance, end
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	
	m_Properties.clear();

	m_lastGraphicUpdateRequestTickCount = -1;

	m_plotGroupHashesInitialized = false;
	m_bWasGraphicsPagingEnabled = false;
	m_iLastLookatX = -1;
	m_iLastLookatY = -1;
}


void CvGame::initDiplomacy()
{
	PROFILE_FUNC();

	int iI, iJ;

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		GET_TEAM((TeamTypes)iI).meet(((TeamTypes)iI), false);

		if (GET_TEAM((TeamTypes)iI).isBarbarian() || GET_TEAM((TeamTypes)iI).isMinorCiv())
		{
			for (iJ = 0; iJ < MAX_CIV_TEAMS; iJ++)
			{
				if (iI != iJ)
				{
					GET_TEAM((TeamTypes)iI).declareWar(((TeamTypes)iJ), false, NO_WARPLAN);
				}
			}
		}
	}

	// Forced peace at the beginning of Advanced starts
	if (isOption(GAMEOPTION_ADVANCED_START))
	{
		CLinkList<TradeData> player1List;
		CLinkList<TradeData> player2List;
		TradeData kTradeData;
		setTradeItem(&kTradeData, TRADE_PEACE_TREATY);
		player1List.insertAtEnd(kTradeData);
		player2List.insertAtEnd(kTradeData);

		for (int iPlayer1 = 0; iPlayer1 < MAX_CIV_PLAYERS; ++iPlayer1)
		{
			CvPlayer& kLoopPlayer1 = GET_PLAYER((PlayerTypes)iPlayer1);

			if (kLoopPlayer1.isAlive())
			{
				for (int iPlayer2 = iPlayer1 + 1; iPlayer2 < MAX_CIV_PLAYERS; ++iPlayer2)
				{
					CvPlayer& kLoopPlayer2 = GET_PLAYER((PlayerTypes)iPlayer2);

					if (kLoopPlayer2.isAlive())
					{
						if (GET_TEAM(kLoopPlayer1.getTeam()).canChangeWarPeace(kLoopPlayer2.getTeam()))
						{
							implementDeal((PlayerTypes)iPlayer1, (PlayerTypes)iPlayer2, &player1List, &player2List);
						}
					}
				}
			}
		}
	}
}


void CvGame::initFreeState()
{
	bool bValid;
	int iI, iJ, iK;

	for (iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		for (iJ = 0; iJ < MAX_TEAMS; iJ++)
		{
			if (GET_TEAM((TeamTypes)iJ).isAlive())
			{
				bValid = false;

				if (!bValid)
				{
					if ((GC.getHandicapInfo(getHandicapType()).isFreeTechs(iI)) ||
						  (!(GET_TEAM((TeamTypes)iJ).isHuman())&& GC.getHandicapInfo(getHandicapType()).isAIFreeTechs(iI)) ||
						  (GC.getTechInfo((TechTypes)iI).getEra() < getStartEra()))
					{
						bValid = true;
					}
				}

				if (!bValid)
				{
					for (iK = 0; iK < MAX_PLAYERS; iK++)
					{
						if (GET_PLAYER((PlayerTypes)iK).isAlive())
						{
							if (GET_PLAYER((PlayerTypes)iK).getTeam() == iJ)
							{
								if (GC.getCivilizationInfo(GET_PLAYER((PlayerTypes)iK).getCivilizationType()).isCivilizationFreeTechs(iI))
								{
									bValid = true;
									break;
								}
							}
						}
					}
				}

				GET_TEAM((TeamTypes)iJ).setHasTech(((TechTypes)iI), bValid, NO_PLAYER, false, false);
				if (bValid && GC.getTechInfo((TechTypes)iI).isMapVisible()) 
				{ 
					GC.getMapINLINE().setRevealedPlots((TeamTypes)iJ, true, true); 
				} 
			}
		}
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			GET_PLAYER((PlayerTypes)iI).initFreeState();
		}
	}
}


void CvGame::initFreeUnits()
{
	/*	f1rpo.autoplay: initFreeUnits is one of the few init functions that
		doesn't get skipped in scenarios */
	initAIHandicap();

	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if ((GET_PLAYER((PlayerTypes)iI).getNumUnits() == 0) && (GET_PLAYER((PlayerTypes)iI).getNumCities() == 0))
			{
				GET_PLAYER((PlayerTypes)iI).initFreeUnits();
			}
		}
	}
}


// f1rpo.autoplay:
void CvGame::initAIHandicap()
{
	// Set m_eAIHandicap to the average of AI handicaps
	int iHandicapSum = 0;
	int iDiv = 0;
	for (int i = 0; i < MAX_CIV_PLAYERS; i++)
	{
		CvPlayer const& kAIPlayer = GET_PLAYER((PlayerTypes)i);
		if(kAIPlayer.isAlive() && !kAIPlayer.isHuman() && !kAIPlayer.isMinorCiv())
		{
			iHandicapSum += kAIPlayer.getHandicapType();
			iDiv++;
		}
	}
	if(iDiv > 0) // Leaves it at STANDARD_HANDICAP in all-human games
		m_eAIHandicap = (HandicapTypes)intdiv::uround(iHandicapSum, iDiv);
}

// f1rpo (era options):
void CvGame::updatePenultimateEra()
{
	int iOption = GAMEOPTION_NO_CLASSICAL_ERA;
	for (; iOption >= GAMEOPTION_NO_FUTURE; iOption--)
	{
		if (isOption((GameOptionTypes)iOption))
			break;
	}
	int iEra = 5 - (iOption - GAMEOPTION_NO_FUTURE);
	FAssertBounds(0, GC.getNumEraInfos(), iEra);
	iEra = std::max<int>(iEra, getStartEra());
	if (iEra >= GC.getNumEraInfos() - 1)
	{
		FAssertMsg(false, "Penultimate not before ultimate era");
		return;
	}
	m_ePenultimateEra = (EraTypes)iEra;
}


void CvGame::assignStartingPlots()
{
	PROFILE_FUNC();

	CvPlot* pPlot;
	CvPlot* pBestPlot;
	bool bStartFound;
	bool bValid;
	int iRandOffset;
	int iLoopTeam;
	int iLoopPlayer;
	int iHumanSlot;
	int iValue;
	int iBestValue;
	int iI, iJ, iK;
	
	std::vector<int> playerOrder;
	std::vector<int>::iterator playerOrderIter;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getStartingPlot() == NULL)
			{
				iBestValue = 0;
				pBestPlot = NULL;

				for (iJ = 0; iJ < GC.getMapINLINE().numPlotsINLINE(); iJ++)
				{
					gDLL->callUpdater();	// allow window updates during launch

					pPlot = GC.getMapINLINE().plotByIndexINLINE(iJ);

					if (pPlot->isStartingPlot())
					{
						bValid = true;

						for (iK = 0; iK < MAX_CIV_PLAYERS; iK++)
						{
							if (GET_PLAYER((PlayerTypes)iK).isAlive())
							{
								if (GET_PLAYER((PlayerTypes)iK).getStartingPlot() == pPlot)
								{
									bValid = false;
									break;
								}
							}
						}

						if (bValid)
						{
							iValue = (1 + getSorenRandNum(1000, "Starting Plot"));

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = pPlot;
							}
						}
					}
				}

				if (pBestPlot != NULL)
				{
					GET_PLAYER((PlayerTypes)iI).setStartingPlot(pBestPlot, true);
				}
			}
		}
	}

	if (PYTHON_CALL_FUNCTION2(__FUNCTION__, gDLL->getPythonIFace()->getMapScriptModule(), "assignStartingPlots"))
	{ 
		if (!gDLL->getPythonIFace()->pythonUsingDefaultImpl())
		{
			// Python override
			return;
		}
	}

	if (isTeamGame())
	{
		for (int iPass = 0; iPass < 2 * MAX_PLAYERS; ++iPass)
		{
			bStartFound = false;

			iRandOffset = getSorenRandNum(countCivTeamsAlive(), "Team Starting Plot");

			for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
			{
				iLoopTeam = ((iI + iRandOffset) % MAX_CIV_TEAMS);

				if (GET_TEAM((TeamTypes)iLoopTeam).isAlive())
				{
					for (iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
					{
						if (GET_PLAYER((PlayerTypes)iJ).isAlive())
						{
							if (GET_PLAYER((PlayerTypes)iJ).getTeam() == iLoopTeam)
							{
								if (GET_PLAYER((PlayerTypes)iJ).getStartingPlot() == NULL)
								{
									CvPlot* pStartingPlot = GET_PLAYER((PlayerTypes)iJ).findStartingPlot();

									if (NULL != pStartingPlot)
									{
										GET_PLAYER((PlayerTypes)iJ).setStartingPlot(pStartingPlot, true);
										playerOrder.push_back(iJ);
									}
									bStartFound = true;
									break;
								}
							}
						}
					}
				}
			}

			if (!bStartFound)
			{
				break;
			}
		}

		//check all players have starting plots
		for (iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
		{
			FAssertMsg(!GET_PLAYER((PlayerTypes)iJ).isAlive() || GET_PLAYER((PlayerTypes)iJ).getStartingPlot() != NULL, "Player has no starting plot");
		}
	}
	else if (isGameMultiPlayer())
	{
		iRandOffset = getSorenRandNum(countCivPlayersAlive(), "Player Starting Plot");

		for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			iLoopPlayer = ((iI + iRandOffset) % MAX_CIV_PLAYERS);

			if (GET_PLAYER((PlayerTypes)iLoopPlayer).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)iLoopPlayer).isHuman())
				{
					if (GET_PLAYER((PlayerTypes)iLoopPlayer).getStartingPlot() == NULL)
					{
						GET_PLAYER((PlayerTypes)iLoopPlayer).setStartingPlot(GET_PLAYER((PlayerTypes)iLoopPlayer).findStartingPlot(), true);
						playerOrder.push_back(iLoopPlayer);
					}
				}
			}
		}

		for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (!(GET_PLAYER((PlayerTypes)iI).isHuman()))
				{
					if (GET_PLAYER((PlayerTypes)iI).getStartingPlot() == NULL)
					{
						GET_PLAYER((PlayerTypes)iI).setStartingPlot(GET_PLAYER((PlayerTypes)iI).findStartingPlot(), true);
						playerOrder.push_back(iI);
					}
				}
			}
		}
	}
	else
	{
		iHumanSlot = range((((countCivPlayersAlive() - 1) * GC.getHandicapInfo(getHandicapType()).getStartingLocationPercent()) / 100), 0, (countCivPlayersAlive() - 1));

		for (iI = 0; iI < iHumanSlot; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (!(GET_PLAYER((PlayerTypes)iI).isHuman()))
				{
					if (GET_PLAYER((PlayerTypes)iI).getStartingPlot() == NULL)
					{
						GET_PLAYER((PlayerTypes)iI).setStartingPlot(GET_PLAYER((PlayerTypes)iI).findStartingPlot(), true);
						playerOrder.push_back(iI);
					}
				}
			}
		}

		for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)iI).isHuman())
				{
					if (GET_PLAYER((PlayerTypes)iI).getStartingPlot() == NULL)
					{
						GET_PLAYER((PlayerTypes)iI).setStartingPlot(GET_PLAYER((PlayerTypes)iI).findStartingPlot(), true);
						playerOrder.push_back(iI);
					}
				}
			}
		}

		for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)iI).getStartingPlot() == NULL)
				{
					GET_PLAYER((PlayerTypes)iI).setStartingPlot(GET_PLAYER((PlayerTypes)iI).findStartingPlot(), true);
					playerOrder.push_back(iI);
				}
			}
		}
	}
	
	//Now iterate over the player starts in the original order and re-place them.
	for (playerOrderIter = playerOrder.begin(); playerOrderIter != playerOrder.end(); ++playerOrderIter)
	{
		GET_PLAYER((PlayerTypes)(*playerOrderIter)).setStartingPlot(GET_PLAYER((PlayerTypes)(*playerOrderIter)).findStartingPlot(), true);
	}
}

// Swaps starting locations until we have reached the optimal closeness between teams
// (caveat: this isn't quite "optimal" because we could get stuck in local minima, but it's pretty good)

void CvGame::normalizeStartingPlotLocations()
{
	CvPlot* apNewStartPlots[MAX_CIV_PLAYERS];
	int* aaiDistances[MAX_CIV_PLAYERS];
	int aiStartingLocs[MAX_CIV_PLAYERS];
	int iI, iJ;

	// Precalculate distances between all starting positions:
	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			gDLL->callUpdater();	// allow window to update during launch
			aaiDistances[iI] = new int[iI];
			for (iJ = 0; iJ < iI; iJ++)
			{
				aaiDistances[iI][iJ] = 0;
			}
			CvPlot *pPlotI = GET_PLAYER((PlayerTypes)iI).getStartingPlot();	
			if (pPlotI != NULL)
			{
				for (iJ = 0; iJ < iI; iJ++)
				{
					if (GET_PLAYER((PlayerTypes)iJ).isAlive())
					{
						CvPlot *pPlotJ = GET_PLAYER((PlayerTypes)iJ).getStartingPlot();
						if (pPlotJ != NULL)
						{
							int iDist = GC.getMapINLINE().calculatePathDistance(pPlotI, pPlotJ);
							if (iDist == -1)
							{
								// 5x penalty for not being on the same area, or having no passable route
								iDist = 5*plotDistance(pPlotI->getX_INLINE(), pPlotI->getY_INLINE(), pPlotJ->getX_INLINE(), pPlotJ->getY_INLINE());
							}
							aaiDistances[iI][iJ] = iDist;
						}
					}
				}
			}
		}
		else
		{
			aaiDistances[iI] = NULL;
		}
	}

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		aiStartingLocs[iI] = iI; // each player starting in own location
	}

	int iBestScore = getTeamClosenessScore(aaiDistances, aiStartingLocs);
	bool bFoundSwap = true;
	while (bFoundSwap)
	{
		bFoundSwap = false;
		for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				for (iJ = 0; iJ < iI; iJ++)
				{
					if (GET_PLAYER((PlayerTypes)iJ).isAlive())
					{
						int iTemp = aiStartingLocs[iI];
						aiStartingLocs[iI] = aiStartingLocs[iJ];
						aiStartingLocs[iJ] = iTemp;
						int iScore = getTeamClosenessScore(aaiDistances, aiStartingLocs);
						if (iScore < iBestScore)
						{
							iBestScore = iScore;
							bFoundSwap = true;
						}
						else
						{
							// Swap them back:
							iTemp = aiStartingLocs[iI];
							aiStartingLocs[iI] = aiStartingLocs[iJ];
							aiStartingLocs[iJ] = iTemp;
						}
					}
				}
			}
		}
	} 

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		apNewStartPlots[iI] = NULL;
	} 

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (aiStartingLocs[iI] != iI)
			{
				apNewStartPlots[iI] = GET_PLAYER((PlayerTypes)aiStartingLocs[iI]).getStartingPlot();
			}
		}
	}

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (apNewStartPlots[iI] != NULL)
			{
				GET_PLAYER((PlayerTypes)iI).setStartingPlot(apNewStartPlots[iI], false);
			}
		}
	}

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		SAFE_DELETE_ARRAY(aaiDistances[iI]);
	}
}


void CvGame::normalizeAddRiver()
{
	for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			CvPlot* pStartingPlot = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

			if (pStartingPlot != NULL)
			{
				if (!pStartingPlot->isFreshWater())
				{
					// if we will be able to add a lake, then use old river code
					if (normalizeFindLakePlot((PlayerTypes)iI) != NULL)
					{
						CvMapGenerator::GetInstance().doRiver(pStartingPlot);
					}
					// otherwise, use new river code which is much more likely to succeed
					else
					{
						CvMapGenerator::GetInstance().addRiver(pStartingPlot);
					}

					// add floodplains to any desert tiles the new river passes through
					for (int iK = 0; iK < GC.getMapINLINE().numPlotsINLINE(); iK++)
					{
						CvPlot* pPlot = GC.getMapINLINE().plotByIndexINLINE(iK);
						FAssert(pPlot != NULL);

						for (int iJ = 0; iJ < GC.getNumFeatureInfos(); iJ++)
						{
							if (GC.getFeatureInfo((FeatureTypes)iJ).isRequiresRiver())
							{
								if (pPlot->canHaveFeature((FeatureTypes)iJ))
								{
									if (GC.getFeatureInfo((FeatureTypes)iJ).getAppearanceProbability() == 10000)
									{
										if (pPlot->getBonusType() != NO_BONUS)
										{
											pPlot->setBonusType(NO_BONUS);
										}
										pPlot->setFeatureType((FeatureTypes)iJ);
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


void CvGame::normalizeRemovePeaks()
{
	CvPlot* pStartingPlot;
	CvPlot* pLoopPlot;
	int iRange;
	int iDX, iDY;
	int iI;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			pStartingPlot = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

			if (pStartingPlot != NULL)
			{
				iRange = 3;

				for (iDX = -(iRange); iDX <= iRange; iDX++)
				{
					for (iDY = -(iRange); iDY <= iRange; iDY++)
					{
						pLoopPlot = plotXY(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iDX, iDY);

						if (pLoopPlot != NULL)
						{
							if (pLoopPlot->isPeak())
							{
								pLoopPlot->setPlotType(PLOT_HILLS);
							}
						}
					}
				}
			}
		}
	}
}

void CvGame::normalizeAddLakes()
{
	for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			CvPlot* pLakePlot = normalizeFindLakePlot((PlayerTypes)iI);
			if (pLakePlot != NULL)
			{
				pLakePlot->setPlotType(PLOT_OCEAN);
			}
		}
	}
}

CvPlot* CvGame::normalizeFindLakePlot(PlayerTypes ePlayer)
{
	if (!GET_PLAYER(ePlayer).isAlive())
	{
		return NULL;
	}

	CvPlot* pStartingPlot = GET_PLAYER(ePlayer).getStartingPlot();
	if (pStartingPlot != NULL)
	{
		if (!(pStartingPlot->isFreshWater()))
		{
			for (int iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
			{
				CvPlot* pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iJ);

				if (pLoopPlot != NULL)
				{
					if (!(pLoopPlot->isWater()))
					{
						if (!(pLoopPlot->isCoastalLand()))
						{
							if (!(pLoopPlot->isRiver()))
							{
								if (pLoopPlot->getBonusType() == NO_BONUS)
								{
									bool bStartingPlot = false;

									for (int iK = 0; iK < MAX_CIV_PLAYERS; iK++)
									{
										if (GET_PLAYER((PlayerTypes)iK).isAlive())
										{
											if (GET_PLAYER((PlayerTypes)iK).getStartingPlot() == pLoopPlot)
											{
												bStartingPlot = true;
												break;
											}
										}
									}

									if (!bStartingPlot)
									{
										return pLoopPlot;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return NULL;
}


void CvGame::normalizeRemoveBadFeatures()
{
	CvPlot* pStartingPlot;
	CvPlot* pLoopPlot;
	int iI, iJ;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			pStartingPlot = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

			if (pStartingPlot != NULL)
			{
				for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
					pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iJ);

					if (pLoopPlot != NULL)
					{
						if (pLoopPlot->getFeatureType() != NO_FEATURE)
						{
							if ((GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange(YIELD_FOOD) <= 0) &&
								(GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange(YIELD_PRODUCTION) <= 0))
							{
								pLoopPlot->setFeatureType(NO_FEATURE);
							}
						}
					}
				}
			
				int iX, iY;
				int iCityRange = CITY_PLOTS_RADIUS;
				int iExtraRange = 2;
				int iMaxRange = iCityRange + iExtraRange;
				
				for (iX = -iMaxRange; iX <= iMaxRange; iX++)
				{
					for (iY = -iMaxRange; iY <= iMaxRange; iY++)
					{
						pLoopPlot = plotXY(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iX, iY);
						if (pLoopPlot != NULL)
						{
							int iDistance = plotDistance(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());			
							if (iDistance <= iMaxRange)
							{
								if (pLoopPlot->getFeatureType() != NO_FEATURE)
								{
									if ((GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange(YIELD_FOOD) <= 0) &&
										(GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange(YIELD_PRODUCTION) <= 0))
									{
										if (pLoopPlot->isWater())
										{
											if (pLoopPlot->isAdjacentToLand() || (!(iDistance == iMaxRange) && (getSorenRandNum(2, "Remove Bad Feature") == 0)))
											{
												pLoopPlot->setFeatureType(NO_FEATURE);
											}
										}
										else
										{
											if (!(iDistance == iMaxRange) && (getSorenRandNum((2 + ((pLoopPlot->getBonusType() == NO_BONUS) ? 0 : 2)), "Remove Bad Feature") == 0))
											{
												pLoopPlot->setFeatureType(NO_FEATURE);                                            
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
	}
}


void CvGame::normalizeRemoveBadTerrain()
{
	CvPlot* pStartingPlot;
	CvPlot* pLoopPlot;
	int iI, iK;
	int iX, iY;
	
	int iTargetFood;
	int iTargetTotal;
	int iPlotFood;
	int iPlotProduction;
	
	
	int iCityRange = CITY_PLOTS_RADIUS;
	int iExtraRange = 1;
	int iMaxRange = iCityRange + iExtraRange;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			pStartingPlot = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

			if (pStartingPlot != NULL)
			{
			    for (iX = -iMaxRange; iX <= iMaxRange; iX++)
			    {
			        for (iY = -iMaxRange; iY <= iMaxRange; iY++)
			        {
			            pLoopPlot = plotXY(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iX, iY);
                        if (pLoopPlot != NULL)
                        {
                            int iDistance = plotDistance(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());
                            if (iDistance <= iMaxRange)
                            {
                                if (!(pLoopPlot->isWater()) && ((iDistance <= iCityRange) || (pLoopPlot->isCoastalLand()) || (0 == getSorenRandNum(1 + iDistance - iCityRange, "Map Upgrade Terrain Food"))))
                                {
                                    iPlotFood = GC.getTerrainInfo(pLoopPlot->getTerrainType()).getYield(YIELD_FOOD);
                                    iPlotProduction = GC.getTerrainInfo(pLoopPlot->getTerrainType()).getYield(YIELD_PRODUCTION);
                                    if ((iPlotFood + iPlotProduction) <= 1)
                                    {
                                        iTargetFood = 1;
                                        iTargetTotal = 1;
                                        if (pLoopPlot->getBonusType(GET_PLAYER((PlayerTypes)iI).getTeam()) != NO_BONUS)
                                        {
                                            iTargetFood = 1;
                                            iTargetTotal = 2;
                                        }
                                        else if ((iPlotFood == 1) || (iDistance <= iCityRange))
                                        {
                                            iTargetFood = 1 + getSorenRandNum(2, "Map Upgrade Terrain Food");
                                            iTargetTotal = 2;
                                        }
                                        else
                                        {
                                            iTargetFood = pLoopPlot->isCoastalLand() ? 2 : 1;
                                            iTargetTotal = 2;
                                        }
                                        
                                        for (iK = 0; iK < GC.getNumTerrainInfos(); iK++)
                                        {
                                            if (!(GC.getTerrainInfo((TerrainTypes)iK).isWater()))
                                            {
                                                if ((GC.getTerrainInfo((TerrainTypes)iK).getYield(YIELD_FOOD) >= iTargetFood) && 
                                                    (GC.getTerrainInfo((TerrainTypes)iK).getYield(YIELD_FOOD) + GC.getTerrainInfo((TerrainTypes)iK).getYield(YIELD_PRODUCTION)) == iTargetTotal)
                                                {
                                                    if ((pLoopPlot->getFeatureType() == NO_FEATURE) || GC.getFeatureInfo(pLoopPlot->getFeatureType()).isTerrain(iK))
                                                    {
                                                        pLoopPlot->setTerrainType((TerrainTypes)iK);
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
			}
		}
	}
}

void CvGame::normalizeAddFoodBonuses()
{
	bool bIgnoreLatitude = pythonIsBonusIgnoreLatitudes();

	for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			CvPlot* pStartingPlot = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

			if (pStartingPlot != NULL)
			{
				int iFoodBonus = 0;
				int iGoodNatureTileCount = 0;

				for (int iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
					CvPlot* pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iJ);

					if (pLoopPlot != NULL)
					{
						BonusTypes eBonus = pLoopPlot->getBonusType(GET_PLAYER((PlayerTypes)iI).getTeam());

						if (eBonus != NO_BONUS)
						{
							if (GC.getBonusInfo(eBonus).getYieldChange(YIELD_FOOD) > 0)
							{
								if ((GC.getBonusInfo(eBonus).getTechCityTrade() == NO_TECH) || (GC.getTechInfo((TechTypes)(GC.getBonusInfo(eBonus).getTechCityTrade())).getEra() <= getStartEra()))
								{
									if (pLoopPlot->isWater())
									{
										iFoodBonus += 2;
									}
									else
									{
										iFoodBonus += 3;
									}
								}
							}
							else if (pLoopPlot->calculateBestNatureYield(YIELD_FOOD, GET_PLAYER((PlayerTypes)iI).getTeam()) >= 2)
						    {
						        iGoodNatureTileCount++;
						    }
						}
						else
						{
                            if (pLoopPlot->calculateBestNatureYield(YIELD_FOOD, GET_PLAYER((PlayerTypes)iI).getTeam()) >= 3)
						    {
						        iGoodNatureTileCount++;
						    }
						}
					}
				}
				
				int iTargetFoodBonusCount = 3;
				iTargetFoodBonusCount += (iGoodNatureTileCount == 0) ? 2 : 0;

				for (int iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
					if (iFoodBonus >= iTargetFoodBonusCount)
					{
						break;
					}

					CvPlot* pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iJ);

					if (pLoopPlot != NULL)
					{
						if (pLoopPlot != pStartingPlot)
						{
							if (pLoopPlot->getBonusType() == NO_BONUS)
							{
								for (int iK = 0; iK < GC.getNumBonusInfos(); iK++)
								{
									if (GC.getBonusInfo((BonusTypes)iK).isNormalize())
									{
										if (GC.getBonusInfo((BonusTypes)iK).getYieldChange(YIELD_FOOD) > 0)
										{
											if ((GC.getBonusInfo((BonusTypes)iK).getTechCityTrade() == NO_TECH) || (GC.getTechInfo((TechTypes)(GC.getBonusInfo((BonusTypes)iK).getTechCityTrade())).getEra() <= getStartEra()))
											{
												if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHasTech((TechTypes)(GC.getBonusInfo((BonusTypes)iK).getTechReveal())))
												{
													if (pLoopPlot->canHaveBonus(((BonusTypes)iK), bIgnoreLatitude))
													{
														pLoopPlot->setBonusType((BonusTypes)iK);
														if (pLoopPlot->isWater())
														{
															iFoodBonus += 2;
														}
														else
														{
															iFoodBonus += 3;
														}
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
			}
		}
	}
}


void CvGame::normalizeAddGoodTerrain()
{
	CvPlot* pStartingPlot;
	CvPlot* pLoopPlot;
	bool bChanged;
	int iGoodPlot;
	int iI, iJ, iK;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			pStartingPlot = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

			if (pStartingPlot != NULL)
			{
				iGoodPlot = 0;

				for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
					pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iJ);

					if (pLoopPlot != NULL)
					{
						if (pLoopPlot != pStartingPlot)
						{
							if ((pLoopPlot->calculateNatureYield(YIELD_FOOD, GET_PLAYER((PlayerTypes)iI).getTeam()) >= GC.getFOOD_CONSUMPTION_PER_POPULATION()) &&
								  (pLoopPlot->calculateNatureYield(YIELD_PRODUCTION, GET_PLAYER((PlayerTypes)iI).getTeam()) > 0))
							{
								iGoodPlot++;
							}
						}
					}
				}

				for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
					if (iGoodPlot >= 4)
					{
						break;
					}

					pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iJ);

					if (pLoopPlot != NULL)
					{
						if (pLoopPlot != pStartingPlot)
						{
							if (!(pLoopPlot->isWater()))
							{
								if (!(pLoopPlot->isHills()))
								{
									if (pLoopPlot->getBonusType() == NO_BONUS)
									{
										bChanged = false;

										if (pLoopPlot->calculateNatureYield(YIELD_FOOD, GET_PLAYER((PlayerTypes)iI).getTeam()) < GC.getFOOD_CONSUMPTION_PER_POPULATION())
										{
											for (iK = 0; iK < GC.getNumTerrainInfos(); iK++)
											{
												if (!(GC.getTerrainInfo((TerrainTypes)iK).isWater()))
												{
													if (GC.getTerrainInfo((TerrainTypes)iK).getYield(YIELD_FOOD) >= GC.getFOOD_CONSUMPTION_PER_POPULATION())
													{
														pLoopPlot->setTerrainType((TerrainTypes)iK);
														bChanged = true;
														break;
													}
												}
											}
										}

										if (pLoopPlot->calculateNatureYield(YIELD_PRODUCTION, GET_PLAYER((PlayerTypes)iI).getTeam()) == 0)
										{
											for (iK = 0; iK < GC.getNumFeatureInfos(); iK++)
											{
												if ((GC.getFeatureInfo((FeatureTypes)iK).getYieldChange(YIELD_FOOD) >= 0) &&
													  (GC.getFeatureInfo((FeatureTypes)iK).getYieldChange(YIELD_PRODUCTION) > 0))
												{
													if (GC.getFeatureInfo((FeatureTypes)iK).isTerrain(pLoopPlot->getTerrainType()))
													{
														pLoopPlot->setFeatureType((FeatureTypes)iK);
														bChanged = true;
														break;
													}
												}
											}
										}

										if (bChanged)
										{
											iGoodPlot++;
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
}


void CvGame::normalizeAddExtras()
{
	bool bIgnoreLatitude = pythonIsBonusIgnoreLatitudes();

	int iTotalValue = 0;
	int iPlayerCount = 0;
	int iBestValue = 0;
	int iWorstValue = MAX_INT;
	int	iI, iJ, iK;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			CvPlot* pStartingPlot = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

			if (pStartingPlot != NULL)
			{
				int iValue = GET_PLAYER((PlayerTypes)iI).AI_foundValue(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), -1, true);
				iTotalValue += iValue;
                iPlayerCount++;
                
                iBestValue = std::max(iValue, iBestValue);
                iWorstValue = std::min(iValue, iWorstValue);
			}
		}
	}

	//iTargetValue = (iTotalValue + iBestValue) / (iPlayerCount + 1);
	int iTargetValue = (iBestValue * 4) / 5;
	
	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			gDLL->callUpdater();	// allow window to update during launch
			CvPlot* pStartingPlot = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

			if (pStartingPlot != NULL)
			{
                int iCount = 0;
				int iFeatureCount = 0;
				int aiShuffle[NUM_CITY_PLOTS];
				shuffleArray(aiShuffle, NUM_CITY_PLOTS, getMapRand());

				for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
					if (GET_PLAYER((PlayerTypes)iI).AI_foundValue(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), -1, true) >= iTargetValue)
					{
						break;
					}
					if (getSorenRandNum((iCount + 2), "Setting Feature Type") <= 1)
					{
						CvPlot* pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), aiShuffle[iJ]);

						if (pLoopPlot != NULL)
						{
							if (pLoopPlot != pStartingPlot)
							{
								if (pLoopPlot->getBonusType() == NO_BONUS)
								{
									if (pLoopPlot->getFeatureType() == NO_FEATURE)
									{
										for (int iK = 0; iK < GC.getNumFeatureInfos(); iK++)
										{
											if ((GC.getFeatureInfo((FeatureTypes)iK).getYieldChange(YIELD_FOOD) + GC.getFeatureInfo((FeatureTypes)iK).getYieldChange(YIELD_PRODUCTION)) > 0)
											{
												if (pLoopPlot->canHaveFeature((FeatureTypes)iK))
												{
													pLoopPlot->setFeatureType((FeatureTypes)iK);
													iCount++;
													break;
												}
											}
										}
									}
									
									iFeatureCount += (pLoopPlot->getFeatureType() != NO_FEATURE) ? 1 : 0;
								}
							}
						}
					}
				}
				
				int iCoastFoodCount = 0;
				int iOceanFoodCount = 0;
				int iOtherCount = 0;
				int iWaterCount = 0;
				for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
					CvPlot* pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iJ);
					if (pLoopPlot != NULL)
					{
						if (pLoopPlot != pStartingPlot)
						{
							if (pLoopPlot->isWater())
							{
								iWaterCount++;
								if (pLoopPlot->getBonusType() != NO_BONUS)
								{
									if (pLoopPlot->isAdjacentToLand())
									{
										iCoastFoodCount++;
									}
									else
									{
										iOceanFoodCount++;                                        
									}
								}
							}
							else
							{
								if (pLoopPlot->getBonusType() != NO_BONUS)
								{
									iOtherCount++;
								}
							}
						}
					}
				}
				
			    bool bLandBias = (iWaterCount > NUM_CITY_PLOTS / 2);
                
                shuffleArray(aiShuffle, NUM_CITY_PLOTS, getMapRand());                

				for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
				    CvPlot* pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), aiShuffle[iJ]);

                    if ((pLoopPlot != NULL) && (pLoopPlot != pStartingPlot))
                    {
                        if (getSorenRandNum(((bLandBias && pLoopPlot->isWater()) ? 2 : 1), "Placing Bonuses") == 0)
                        {
                        	if ((iOtherCount * 3 + iOceanFoodCount * 2 + iCoastFoodCount * 2) >= 12)
                        	{
                        		break;
                        	}
                        	
                            if (GET_PLAYER((PlayerTypes)iI).AI_foundValue(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), -1, true) >= iTargetValue)
                            {
                                break;
                            }

						    bool bCoast = (pLoopPlot->isWater() && pLoopPlot->isAdjacentToLand());
						    bool bOcean = (pLoopPlot->isWater() && !bCoast);
							if ((pLoopPlot != pStartingPlot)
                                && !(bCoast && (iCoastFoodCount > 2))
                                && !(bOcean && (iOceanFoodCount > 2)))
							{
								for (int iPass = 0; iPass < 2; iPass++)
								{
									if (pLoopPlot->getBonusType() == NO_BONUS)
									{
										for (iK = 0; iK < GC.getNumBonusInfos(); iK++)
										{
											if (GC.getBonusInfo((BonusTypes)iK).isNormalize())
											{
											    //???no bonuses with negative yields?
												if ((GC.getBonusInfo((BonusTypes)iK).getYieldChange(YIELD_FOOD) >= 0) &&
													  (GC.getBonusInfo((BonusTypes)iK).getYieldChange(YIELD_PRODUCTION) >= 0))
												{
													if ((GC.getBonusInfo((BonusTypes)iK).getTechCityTrade() == NO_TECH) || (GC.getTechInfo((TechTypes)(GC.getBonusInfo((BonusTypes)iK).getTechCityTrade())).getEra() <= getStartEra()))
													{
														if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHasTech((TechTypes)(GC.getBonusInfo((BonusTypes)iK).getTechReveal())))
														{
															if ((iPass == 0) ? CvMapGenerator::GetInstance().canPlaceBonusAt(((BonusTypes)iK), pLoopPlot->getX(), pLoopPlot->getY(), bIgnoreLatitude) : pLoopPlot->canHaveBonus(((BonusTypes)iK), bIgnoreLatitude))
															{
																pLoopPlot->setBonusType((BonusTypes)iK);
																iCoastFoodCount += bCoast ? 1 : 0;
																iOceanFoodCount += bOcean ? 1 : 0;
																iOtherCount += !(bCoast || bOcean) ? 1 : 0;
																break;
															}
														}
													}
												}
											}
										}
										
										if (bLandBias && !pLoopPlot->isWater() && pLoopPlot->getBonusType() == NO_BONUS)
										{
											if (((iFeatureCount > 4) && (pLoopPlot->getFeatureType() != NO_FEATURE))
												&& ((iCoastFoodCount + iOceanFoodCount) > 2))
											{
												if (getSorenRandNum(2, "Clear feature to add bonus") == 0)
												{
													pLoopPlot->setFeatureType(NO_FEATURE);

													for (iK = 0; iK < GC.getNumBonusInfos(); iK++)
													{
														if (GC.getBonusInfo((BonusTypes)iK).isNormalize())
														{
															//???no bonuses with negative yields?
															if ((GC.getBonusInfo((BonusTypes)iK).getYieldChange(YIELD_FOOD) >= 0) &&
																  (GC.getBonusInfo((BonusTypes)iK).getYieldChange(YIELD_PRODUCTION) >= 0))
															{
																if ((GC.getBonusInfo((BonusTypes)iK).getTechCityTrade() == NO_TECH) || (GC.getTechInfo((TechTypes)(GC.getBonusInfo((BonusTypes)iK).getTechCityTrade())).getEra() <= getStartEra()))
																{
																	if ((iPass == 0) ? CvMapGenerator::GetInstance().canPlaceBonusAt(((BonusTypes)iK), pLoopPlot->getX(), pLoopPlot->getY(), bIgnoreLatitude) : pLoopPlot->canHaveBonus(((BonusTypes)iK), bIgnoreLatitude))
																	{
																		pLoopPlot->setBonusType((BonusTypes)iK);
																		iOtherCount++;
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
							}
						}
					}
				}
				
				shuffleArray(aiShuffle, NUM_CITY_PLOTS, getMapRand());

				for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
					if (GET_PLAYER((PlayerTypes)iI).AI_foundValue(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), -1, true) >= iTargetValue)
					{
						break;
					}
				
					CvPlot* pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), aiShuffle[iJ]);

					if (pLoopPlot != NULL)
					{
						if (pLoopPlot != pStartingPlot)
						{
							if (pLoopPlot->getBonusType() == NO_BONUS)
							{
								if (pLoopPlot->getFeatureType() == NO_FEATURE)
								{
									for (iK = 0; iK < GC.getNumFeatureInfos(); iK++)
									{
										if ((GC.getFeatureInfo((FeatureTypes)iK).getYieldChange(YIELD_FOOD) + GC.getFeatureInfo((FeatureTypes)iK).getYieldChange(YIELD_PRODUCTION)) > 0)
										{
											if (pLoopPlot->canHaveFeature((FeatureTypes)iK))
											{
												pLoopPlot->setFeatureType((FeatureTypes)iK);
												break;
											}
										}
									}
								}
							}
						}
					}
				}
				
				int iHillsCount = 0;
				
				for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
					CvPlot* pLoopPlot =plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), iJ);
					if (pLoopPlot != NULL)
					{
						if (pLoopPlot->isHills())
						{
							iHillsCount++;
						}
					}
				}
				shuffleArray(aiShuffle, NUM_CITY_PLOTS, getMapRand());
				for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
				{
					if (iHillsCount >= 3)
					{
						break;
					}
					CvPlot* pLoopPlot = plotCity(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), aiShuffle[iJ]);
					if (pLoopPlot != NULL)
					{
						if (!pLoopPlot->isWater())
						{
							if (!pLoopPlot->isHills())
							{
								if ((pLoopPlot->getFeatureType() == NO_FEATURE) ||
									!GC.getFeatureInfo(pLoopPlot->getFeatureType()).isRequiresFlatlands())
								{
									if ((pLoopPlot->getBonusType() == NO_BONUS) ||
										GC.getBonusInfo(pLoopPlot->getBonusType()).isHills())
									{
										pLoopPlot->setPlotType(PLOT_HILLS, false, true);									
										iHillsCount++;
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


void CvGame::normalizeStartingPlots()
{
	PROFILE_FUNC();

	if (!(GC.getInitCore().getWBMapScript()) || GC.getInitCore().getWBMapNoPlayers())
	{
		if (!PYTHON_CALL_FUNCTION(__FUNCTION__, gDLL->getPythonIFace()->getMapScriptModule(), "normalizeStartingPlotLocations", NULL)  || gDLL->getPythonIFace()->pythonUsingDefaultImpl())
		{
/************************************************************************************************/
/* Afforess	                  Start		 03/10/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			if (!isOption(GAMEOPTION_CULTURALLY_LINKED_STARTS))
			{
				normalizeStartingPlotLocations();
			}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		}
	}

	if (GC.getInitCore().getWBMapScript())
	{
		return;
	}

	if (!PYTHON_CALL_FUNCTION(__FUNCTION__, gDLL->getPythonIFace()->getMapScriptModule(), "normalizeAddRiver", NULL)  || gDLL->getPythonIFace()->pythonUsingDefaultImpl())
	{
		normalizeAddRiver();
	}

	if (!PYTHON_CALL_FUNCTION(__FUNCTION__, gDLL->getPythonIFace()->getMapScriptModule(), "normalizeRemovePeaks", NULL)  || gDLL->getPythonIFace()->pythonUsingDefaultImpl())
	{
		normalizeRemovePeaks();
	}

	if (!PYTHON_CALL_FUNCTION(__FUNCTION__, gDLL->getPythonIFace()->getMapScriptModule(), "normalizeAddLakes", NULL)  || gDLL->getPythonIFace()->pythonUsingDefaultImpl())
	{
		normalizeAddLakes();
	}

	if (!PYTHON_CALL_FUNCTION(__FUNCTION__, gDLL->getPythonIFace()->getMapScriptModule(), "normalizeRemoveBadFeatures", NULL)  || gDLL->getPythonIFace()->pythonUsingDefaultImpl())
	{
		normalizeRemoveBadFeatures();
	}

	if (!PYTHON_CALL_FUNCTION(__FUNCTION__, gDLL->getPythonIFace()->getMapScriptModule(), "normalizeRemoveBadTerrain", NULL)  || gDLL->getPythonIFace()->pythonUsingDefaultImpl())
	{
		normalizeRemoveBadTerrain();
	}

	if (!PYTHON_CALL_FUNCTION(__FUNCTION__, gDLL->getPythonIFace()->getMapScriptModule(), "normalizeAddFoodBonuses", NULL)  || gDLL->getPythonIFace()->pythonUsingDefaultImpl())
	{
		normalizeAddFoodBonuses();
	}

	if (!PYTHON_CALL_FUNCTION(__FUNCTION__, gDLL->getPythonIFace()->getMapScriptModule(), "normalizeAddGoodTerrain", NULL)  || gDLL->getPythonIFace()->pythonUsingDefaultImpl())
	{
		normalizeAddGoodTerrain();
	}

	if (!PYTHON_CALL_FUNCTION(__FUNCTION__, gDLL->getPythonIFace()->getMapScriptModule(), "normalizeAddExtras", NULL)  || gDLL->getPythonIFace()->pythonUsingDefaultImpl())
	{
		normalizeAddExtras();
	}
}

// For each of n teams, let the closeness score for that team be the average distance of an edge between two players on that team.
// This function calculates the closeness score for each team and returns the sum of those n scores.
// The lower the result, the better "clumped" the players' starting locations are.
//
// Note: for the purposes of this function, player i will be assumed to start in the location of player aiStartingLocs[i] 

int CvGame::getTeamClosenessScore(int** aaiDistances, int* aiStartingLocs)
{
	int iScore = 0;

	for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; iTeam++)
	{
		if (GET_TEAM((TeamTypes)iTeam).isAlive())
		{
			int iTeamTotalDist = 0;
			int iNumEdges = 0;
			for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; iPlayer++)
			{
				if (GET_PLAYER((PlayerTypes)iPlayer).isAlive())
				{
					if (GET_PLAYER((PlayerTypes)iPlayer).getTeam() == (TeamTypes)iTeam)
					{
						for (int iOtherPlayer = 0; iOtherPlayer < iPlayer; iOtherPlayer++)
						{
							if (GET_PLAYER((PlayerTypes)iOtherPlayer).getTeam() == (TeamTypes)iTeam)
							{
								// Add the edge between these two players that are on the same team
								iNumEdges++;
								int iPlayerStart = aiStartingLocs[iPlayer];
								int iOtherPlayerStart = aiStartingLocs[iOtherPlayer];

								if (iPlayerStart < iOtherPlayerStart) // Make sure that iPlayerStart > iOtherPlayerStart
								{
									int iTemp = iPlayerStart;
									iPlayerStart = iOtherPlayerStart;
									iOtherPlayerStart = iTemp;
								}
								else if (iPlayerStart == iOtherPlayerStart)
								{
									FAssertMsg(false, "Two players are (hypothetically) assigned to the same starting location!");
								}
								iTeamTotalDist += aaiDistances[iPlayerStart][iOtherPlayerStart];
							}
						}
					}
				}
			}

			int iTeamScore;
			if (iNumEdges == 0)
			{
				iTeamScore = 0;
			}
			else
			{
				iTeamScore = iTeamTotalDist/iNumEdges; // the avg distance between team edges is the team score
			}

			iScore += iTeamScore; 
		}
	}
	return iScore;
}


void CvGame::update()
{

	MEMORY_TRACE_FUNCTION();

#ifdef LOG_AI
	gPlayerLogLevel = getBugOptionINT("Autolog__BBAILevel", 0);
	gTeamLogLevel = gPlayerLogLevel;
	gCityLogLevel = gPlayerLogLevel;
	gUnitLogLevel = gPlayerLogLevel;

	if ( gPlayerLogLevel > 0 )
	{
		GC.setXMLLogging(true);
	}
#endif

	startProfilingDLL(false);
	if ( !gDLL->IsPitbossHost() )
	{
		CvPlot* lookatPlot = gDLL->getInterfaceIFace()->getLookAtPlot();
	
		if ( lookatPlot != NULL )
		{
			//	Sample th BUG setting in the main thread on entry to game update here (it requires a Python call
			//	so we don't want it happening in background, or more frequently than once per turn slice)
			bool bPagingEnabled = getBugOptionBOOL("MainInterface__EnableGraphicalPaging", true);
			GC.setGraphicalDetailPagingEnabled(bPagingEnabled);

			if ( m_bWasGraphicsPagingEnabled != bPagingEnabled)
			{
				for(int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
				{
					CvPlot*	pPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
					if ( pPlot != NULL )
					{
						if ( m_bWasGraphicsPagingEnabled )
						{
							pPlot->setShouldHaveFullGraphics(true);
						}
						else
						{
							pPlot->setShouldHaveFullGraphics(false);
						}
					}
				}
			}

			m_bWasGraphicsPagingEnabled = bPagingEnabled;

			if ( GC.getGraphicalDetailPagingEnabled() )
			{
				if ( (m_iLastLookatX != lookatPlot->getX_INLINE() || m_iLastLookatY != lookatPlot->getY_INLINE()) )
				{
					int pageInRange = GC.getGraphicalDetailPageInRange();
					CvPlot::notePageRenderStart((pageInRange*2+1)*(pageInRange*2+1));

					for(int iX = -pageInRange; iX <= pageInRange; iX++)
					{
						for(int iY = -pageInRange; iY <= pageInRange; iY++)
						{
							CvPlot* pPlot = plotXY(lookatPlot->getX_INLINE(),lookatPlot->getY_INLINE(),iX,iY);

							if ( pPlot != NULL )
							{
								pPlot->setShouldHaveFullGraphics(true);
							}
						}
					}

					m_iLastLookatX = lookatPlot->getX_INLINE();
					m_iLastLookatY = lookatPlot->getY_INLINE();
				}

				CvPlot::EvictGraphicsIfNecessary();
			}
		}
	}
	//OutputDebugString(CvString::format("Start profiling(false) for CvGame::update()\n").c_str());
	PROFILE_BEGIN("CvGame::update");

	if ( !m_plotGroupHashesInitialized )
	{
		PROFILE("CvGame::update.OneTimeInit");

		//	First opportunity after a load or save is here since sadly the DLL structure
		//	gives us no load/save-completed event
		CvTaggedSaveFormatWrapper&	wrapper = CvTaggedSaveFormatWrapper::getSaveFormatWrapper();

		//	Close will free any resources and display any warnings if we've just finished loading/saving
		wrapper.close();

		GC.getLoadedInitCore().checkVersions();

		for(int iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if ( GET_PLAYER((PlayerTypes)iI).isAlive() )
			{
				GET_PLAYER((PlayerTypes)iI).RecalculatePlotGroupHashes();
			}
		}

		m_plotGroupHashesInitialized = true;

		gDLL->getEngineIFace()->clearSigns();
		gDLL->getEngineIFace()->setResourceLayer(GC.getResourceLayer());

		//	Remove and re-add the GW on load
		processGreatWall(false, true, false);
		processGreatWall(true, true);
	}

	{
		PROFILE("CvGame::update.ViewportInit");

		//	Some processing that is done in viewport tranisitions has to occur
		//	over several messaging timeslices (empirically - the game engine
		//	gets things wrong if you don't give it a chnace to process messages
		//	in between).  To enact that we use a state machine which performs state
		//	transitions on each call to CvGame::update().
		if ( !gDLL->IsPitbossHost() )
		{		
			GC.getCurrentViewport()->processActionState();
		}		
		//	On each time slice put back the signs if they have been temporarily removed
		//	(as happens across a save or a map/viewport switch)
		GC.reprocessSigns();
		//	If a rebuild of the graphics is needed and about a second has gone by since a new refresh was last
		//	requested perform the rebuild (this amalgamates all the rebuild requests pertaining to a move or
		//	a sequence of moves)
		if ( m_lastGraphicUpdateRequestTickCount != -1 && (GetTickCount() - m_lastGraphicUpdateRequestTickCount) > 500 )
		{
			m_lastGraphicUpdateRequestTickCount = -1;

			gDLL->getEngineIFace()->RebuildAllPlots();
			gDLL->getInterfaceIFace()->setDirty(GlobeLayer_DIRTY_BIT, true);
		}
	}

	const int iActiveTurn = getGameTurn();

again:
	if (!gDLL->GetWorldBuilderMode() || isInAdvancedStart())
	{
		PROFILE("CvGame::update.Turn");

		sendPlayerOptions();
		// sample generic event
		CyArgsList pyArgs;
		pyArgs.add(getTurnSlice());
		CvEventReporter::getInstance().genericEvent("gameUpdate", pyArgs.makeFunctionArgs());

		if (getTurnSlice() == 0)
		{
			PROFILE("CvGame::update.AutoSave");

			gDLL->getEngineIFace()->AutoSave(true);
		}

		if (getNumGameTurnActive() == 0)
		{
			if (!isPbem() || !getPbemTurnSent())
			{
				doTurn();
			}
		}

		updateScore();

		updateWar();

		updateMoves();

		updateTimers();

		updateTurnTimer();

		AI_updateAssignWork();

		testAlive();

		if (GC.isENABLE_RESYNC_CODE())
		{
			if (isNetworkMultiPlayer())
			{
				if (getTurnSlice() == 0)
				{
					m_resyncPackets.clear();
					m_bOutOfSync = false;
				}
				else if (m_resyncPackets.size() > 0)
				{
					std::vector<CvGameResyncPacket*>::iterator it;
					int iCount = 20;
					for (it = m_resyncPackets.begin(); it != m_resyncPackets.end();)
					{
						CvGameResyncPacket* packet = (*it);
						it = m_resyncPackets.erase(it);
						gDLL->sendMessageData(packet);
						iCount--;
						if (iCount <= 0)
						{
							break;
						}
					}
				}
				else if (isMultiplayerHost())
				{
					checkOutOfSyncPlayers();
				}
			}
		}

		if ((isNetworkMultiPlayer() || getAIAutoPlay(getActivePlayer()) <= 0 && !gDLL->GetAutorun() && GAMESTATE_EXTENDED != getGameState()))
		{
			if (countHumanPlayersAlive() == 0)
			{
				setGameState(GAMESTATE_OVER);
			}
		}
		changeTurnSlice(1);
		m_iActiveTurnSlice += 1;

		if (NO_PLAYER != getActivePlayer() && GET_PLAYER(getActivePlayer()).getAdvancedStartPoints() >= 0 && !gDLL->getInterfaceIFace()->isInAdvancedStart())
		{
			gDLL->getInterfaceIFace()->setInAdvancedStart(true);
			gDLL->getInterfaceIFace()->setWorldBuilder(true);
		}
	}

	reportQueuedEvents();
	doQueuedUIActivity();

	//OutputDebugString(CvString::format("Stop profiling(false) after CvGame::update()\n").c_str());
	if (!isPitboss() && !isGameMultiPlayer())
	{
		const CvPlayerAI& kActivePlayer = GET_PLAYER(getActivePlayer());
		if (!kActivePlayer.hasBusyUnit() && (!kActivePlayer.isTurnActive() || kActivePlayer.isAutoMoves()))
		{
			updateTimers();

			if (iActiveTurn == getGameTurn())
			{
				goto again;
			}
		}
	}
	PROFILE_END();
	stopProfilingDLL(false);

}


void CvGame::updateScore(bool bForce)
{
	PROFILE_FUNC();

	MEMORY_TRACE_FUNCTION();

	bool abPlayerScored[MAX_CIV_PLAYERS];
	bool abTeamScored[MAX_CIV_TEAMS];
	int iScore;
	int iBestScore;
	PlayerTypes eBestPlayer;
	TeamTypes eBestTeam;
	int iI, iJ, iK;

	if (!isScoreDirty() && !bForce)
	{
		return;
	}

	setScoreDirty(false);

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		abPlayerScored[iI] = false;
	}

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		iBestScore = MIN_INT;
		eBestPlayer = NO_PLAYER;

		for (iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
		{
			if (!abPlayerScored[iJ])
			{
				iScore = GET_PLAYER((PlayerTypes)iJ).calculateScore(false);

				if (iScore >= iBestScore)
				{
					iBestScore = iScore;
					eBestPlayer = (PlayerTypes)iJ;
				}
			}
		}

		abPlayerScored[eBestPlayer] = true;

		setRankPlayer(iI, eBestPlayer);
		setPlayerRank(eBestPlayer, iI);
		setPlayerScore(eBestPlayer, iBestScore);
		GET_PLAYER(eBestPlayer).updateScoreHistory(getGameTurn(), iBestScore);
	}

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		abTeamScored[iI] = false;
	}

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		iBestScore = MIN_INT;
		eBestTeam = NO_TEAM;

		for (iJ = 0; iJ < MAX_CIV_TEAMS; iJ++)
		{
			if (!abTeamScored[iJ])
			{
				iScore = 0;

				for (iK = 0; iK < MAX_CIV_PLAYERS; iK++)
				{
					if (GET_PLAYER((PlayerTypes)iK).getTeam() == iJ)
					{
						iScore += getPlayerScore((PlayerTypes)iK);
					}
				}

				if (iScore >= iBestScore)
				{
					iBestScore = iScore;
					eBestTeam = (TeamTypes)iJ;
				}
			}
		}

		abTeamScored[eBestTeam] = true;

		setRankTeam(iI, eBestTeam);
		setTeamRank(eBestTeam, iI);
		setTeamScore(eBestTeam, iBestScore);
	}
}

void CvGame::updatePlotGroups(bool reInitialize)
{
	PROFILE_FUNC();

	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			GET_PLAYER((PlayerTypes)iI).updatePlotGroups(NULL, reInitialize);
		}
	}
}


void CvGame::updateBuildingCommerce()
{
	PROFILE_FUNC();

	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			GET_PLAYER((PlayerTypes)iI).updateBuildingCommerce();
		}
	}
}


void CvGame::updateCitySight(bool bIncrement)
{
	PROFILE_FUNC();

	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			GET_PLAYER((PlayerTypes)iI).updateCitySight(bIncrement, false);
		}
	}

	updatePlotGroups();
}


void CvGame::updateTradeRoutes()
{
	PROFILE_FUNC();

	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			GET_PLAYER((PlayerTypes)iI).updateTradeRoutes();
		}
	}
}


void CvGame::testExtendedGame()
{
	PROFILE_FUNC();

	int iI;

	if (getGameState() != GAMESTATE_OVER)
	{
		return;
	}

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).isHuman())
			{
				if (GET_PLAYER((PlayerTypes)iI).isExtendedGame())
				{
					setGameState(GAMESTATE_EXTENDED);
					break;
				}
			}
		}
	}
}


void CvGame::cityPushOrder(CvCity* pCity, OrderTypes eOrder, int iData, bool bAlt, bool bShift, bool bCtrl) const
{
	if (pCity->getProduction() > 0)
	{
		CvMessageControl::getInstance().sendPushOrder(pCity->getID(), eOrder, iData, bAlt, bShift, !bShift);
	}
	else if ((eOrder == ORDER_TRAIN) && (pCity->getProductionUnit() == iData))
	{
		CvMessageControl::getInstance().sendPushOrder(pCity->getID(), eOrder, iData, bAlt, !bCtrl, bCtrl);
	}
	else
	{
		CvMessageControl::getInstance().sendPushOrder(pCity->getID(), eOrder, iData, bAlt, bShift, bCtrl);
	}
}


void CvGame::selectUnit(CvUnit* pUnit, bool bClear, bool bToggle, bool bSound) const
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pEntityNode;
	CvSelectionGroup* pSelectionGroup;
	bool bSelectGroup;
	bool bGroup;

	if (gDLL->getInterfaceIFace()->getHeadSelectedUnit() == NULL)
	{
		bSelectGroup = true;
	}
	else if (gDLL->getInterfaceIFace()->getHeadSelectedUnit()->getGroup() != pUnit->getGroup())
	{
		bSelectGroup = true;
	}
	else if (pUnit->IsSelected() && !(gDLL->getInterfaceIFace()->mirrorsSelectionGroup()))
	{
		bSelectGroup = !bToggle;
	}
	else
	{
		bSelectGroup = false;
	}

	gDLL->getInterfaceIFace()->clearSelectedCities();

	if (bClear)
	{
		gDLL->getInterfaceIFace()->clearSelectionList();
		bGroup = false;
	}
	else
	{
		bGroup = gDLL->getInterfaceIFace()->mirrorsSelectionGroup();
	}

	if (bSelectGroup)
	{
		pSelectionGroup = pUnit->getGroup();

		gDLL->getInterfaceIFace()->selectionListPreChange();

		pEntityNode = pSelectionGroup->headUnitNode();

		while (pEntityNode != NULL)
		{
			FAssertMsg(::getUnit(pEntityNode->m_data), "null entity in selection group");
			gDLL->getInterfaceIFace()->insertIntoSelectionList(::getUnit(pEntityNode->m_data), false, bToggle, bGroup, bSound, true);

			pEntityNode = pSelectionGroup->nextUnitNode(pEntityNode);
		}

		gDLL->getInterfaceIFace()->selectionListPostChange();
	}
	else
	{
		gDLL->getInterfaceIFace()->insertIntoSelectionList(pUnit, false, bToggle, bGroup, bSound);
	}

	gDLL->getInterfaceIFace()->makeSelectionListDirty();
}


void CvGame::selectGroup(CvUnit* pUnit, bool bShift, bool bCtrl, bool bAlt) const
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pUnitPlot;
	bool bGroup;

	FAssertMsg(pUnit != NULL, "pUnit == NULL unexpectedly");

	if (bAlt || bCtrl)
	{
		gDLL->getInterfaceIFace()->clearSelectedCities();

		if (!bShift)
		{
			gDLL->getInterfaceIFace()->clearSelectionList();
			bGroup = true;
		}
		else
		{
			bGroup = gDLL->getInterfaceIFace()->mirrorsSelectionGroup();
		}

		pUnitPlot = pUnit->plot();

		pUnitNode = pUnitPlot->headUnitNode();

		gDLL->getInterfaceIFace()->selectionListPreChange();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = pUnitPlot->nextUnitNode(pUnitNode);

			if (pLoopUnit->getOwnerINLINE() == getActivePlayer())
			{
				if (pLoopUnit->canMove())
				{
					if (!isMPOption(MPOPTION_SIMULTANEOUS_TURNS) || getTurnSlice() - pLoopUnit->getLastMoveTurn() > GC.getDefineINT("MIN_TIMER_UNIT_DOUBLE_MOVES"))
					{
						if (bAlt || (pLoopUnit->getUnitType() == pUnit->getUnitType()))
						{
							gDLL->getInterfaceIFace()->insertIntoSelectionList(pLoopUnit, false, false, bGroup, false, true);
						}
					}
				}
			}
		}

		gDLL->getInterfaceIFace()->selectionListPostChange();
	}
	else
	{
		gDLL->getInterfaceIFace()->selectUnit(pUnit, !bShift, bShift, true);
	}
}


void CvGame::selectAll(CvPlot* pPlot) const
{
	CvUnit* pSelectUnit;
	CvUnit* pCenterUnit;

	pSelectUnit = NULL;

	if (pPlot != NULL)
	{
		pCenterUnit = pPlot->getDebugCenterUnit();

		if ((pCenterUnit != NULL) && (pCenterUnit->getOwnerINLINE() == getActivePlayer()))
		{
			pSelectUnit = pCenterUnit;
		}
	}

	if (pSelectUnit != NULL)
	{
		gDLL->getInterfaceIFace()->selectGroup(pSelectUnit, false, false, true);
	}
}


bool CvGame::selectionListIgnoreBuildingDefense() const
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pSelectedUnitNode;
	CvUnit* pSelectedUnit;
	bool bIgnoreBuilding;
	bool bAttackLandUnit;

	bIgnoreBuilding = false;
	bAttackLandUnit = false;

	pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

	while (pSelectedUnitNode != NULL)
	{
		pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);
		pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);

		if (pSelectedUnit != NULL)
		{
			if (pSelectedUnit->ignoreBuildingDefense())
			{
				bIgnoreBuilding = true;
			}

			if ((pSelectedUnit->getDomainType() == DOMAIN_LAND) && pSelectedUnit->canAttack())
			{
				bAttackLandUnit = true;
			}
		}
	}

	if (!bIgnoreBuilding && !bAttackLandUnit)
	{
		if (getBestLandUnit() != NO_UNIT)
		{
			bIgnoreBuilding = GC.getUnitInfo(getBestLandUnit()).isIgnoreBuildingDefense();
		}
	}

	return bIgnoreBuilding;
}


void CvGame::implementDeal(PlayerTypes eWho, PlayerTypes eOtherWho, CLinkList<TradeData>* pOurList, CLinkList<TradeData>* pTheirList, bool bForce)
{
	CvDeal* pDeal;

	FAssertMsg(eWho != NO_PLAYER, "Who is not assigned a valid value");
	FAssertMsg(eOtherWho != NO_PLAYER, "OtherWho is not assigned a valid value");
	FAssertMsg(eWho != eOtherWho, "eWho is not expected to be equal with eOtherWho");

	pDeal = addDeal();
	pDeal->init(pDeal->getID(), eWho, eOtherWho);
	pDeal->addTrades(pOurList, pTheirList, !bForce);
	if ((pDeal->getLengthFirstTrades() == 0) && (pDeal->getLengthSecondTrades() == 0))
	{
		pDeal->kill();
	}
}


void CvGame::verifyDeals()
{
	CvDeal* pLoopDeal;
	int iLoop = 0;

	for(pLoopDeal = firstDeal(&iLoop); pLoopDeal != NULL; pLoopDeal = nextDeal(&iLoop))
	{
		pLoopDeal->verify();
	}
}


/* Globeview configuration control:
If bStarsVisible, then there will be stars visible behind the globe when it is on
If bWorldIsRound, then the world will bend into a globe; otherwise, it will show up as a plane  */
void CvGame::getGlobeviewConfigurationParameters(TeamTypes eTeam, bool& bStarsVisible, bool& bWorldIsRound)
{
	if(GET_TEAM(eTeam).isMapCentering() || isCircumnavigated())
	{
		bStarsVisible = true;
		bWorldIsRound = true;
	}
	else
	{
		bStarsVisible = false;
		bWorldIsRound = false;
	}
}


int CvGame::getSymbolID(int iSymbol)
{
	return gDLL->getInterfaceIFace()->getSymbolID(iSymbol);
}


int CvGame::getAdjustedPopulationPercent(VictoryTypes eVictory) const
{
	int iPopulation;
	int iBestPopulation;
	int iNextBestPopulation;
	int iI;

	if (GC.getVictoryInfo(eVictory).getPopulationPercentLead() == 0)
	{
		return 0;
	}

	if (getTotalPopulation() == 0)
	{
		return 100;
	}

	iBestPopulation = 0;
	iNextBestPopulation = 0;

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			iPopulation = GET_TEAM((TeamTypes)iI).getTotalPopulation();

			if (iPopulation > iBestPopulation)
			{
				iNextBestPopulation = iBestPopulation;
				iBestPopulation = iPopulation;
			}
			else if (iPopulation > iNextBestPopulation)
			{
				iNextBestPopulation = iPopulation;
			}
		}
	}

	return std::min(100, (((iNextBestPopulation * 100) / getTotalPopulation()) + GC.getVictoryInfo(eVictory).getPopulationPercentLead()));
}


int CvGame::getProductionPerPopulation(HurryTypes eHurry)
{
	if (NO_HURRY == eHurry)
	{
		return 0;
	}
	return (GC.getHurryInfo(eHurry).getProductionPerPopulation() * 100) / std::max(1, GC.getGameSpeedInfo(getGameSpeedType()).getHurryPercent());
}


int CvGame::getAdjustedLandPercent(VictoryTypes eVictory) const
{
	int iPercent;

	if (GC.getVictoryInfo(eVictory).getLandPercent() == 0)
	{
		return 0;
	}

	iPercent = GC.getVictoryInfo(eVictory).getLandPercent();

	iPercent -= (countCivTeamsEverAlive() * 2);

	return std::max(iPercent, GC.getVictoryInfo(eVictory).getMinLandPercent());
}


bool CvGame::isTeamVote(VoteTypes eVote) const
{
	return (GC.getVoteInfo(eVote).isSecretaryGeneral() || GC.getVoteInfo(eVote).isVictory());
}


bool CvGame::isChooseElection(VoteTypes eVote) const
{
	return !(GC.getVoteInfo(eVote).isSecretaryGeneral());
}


bool CvGame::isTeamVoteEligible(TeamTypes eTeam, VoteSourceTypes eVoteSource) const
{
	CvTeam& kTeam = GET_TEAM(eTeam);
/* Munch edit 06/10/09, added by 45° on 02/17/13 */
/* so that the incumbent resident is forbidden from being reelected, but not forbidden from being elected to total victory */
/* to revert, delete all; should work with "normal" Diplomatic Victory; when Mastery Victory is enabled, Secretary General cannot be a candidate for */ 
/* Diplomatic Victory, which isn't a problem anyway, since Mastery Victory overrules Diplomatic Victory (added by 45°)*/
	int bSecretaryElection = 0;
	for (int iI = 0; iI < GC.getNumVoteInfos(); iI++)
	{
		if ((GC.getVoteInfo((VoteTypes)iI).isVoteSourceType(eVoteSource)) && (VoteSourceTypes(eVoteSource) != 1))  /* here's a check so that inelegibility applies to UN Secretary only and not to AP Resident) by 45° */
		{
			if (!GC.getVoteInfo((VoteTypes)iI).isVictory())
			{
				bSecretaryElection = 1;
			}
		}
	}

	int incumbentTeam = getSecretaryGeneral(eVoteSource);
	if (eTeam == incumbentTeam)
	{
		if (bSecretaryElection == 1) return false;
	}
/* end Munch edit */
	if (kTeam.isForceTeamVoteEligible(eVoteSource))
	{
		return true;
	}

	if (!kTeam.isFullMember(eVoteSource))
	{
		return false;
	}

	int iCount = 0;
	for (int iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		CvTeam& kLoopTeam = GET_TEAM((TeamTypes)iI);
		if (kLoopTeam.isAlive())
		{
			if (kLoopTeam.isForceTeamVoteEligible(eVoteSource))
			{
				++iCount;
			}
		}
	}

	int iExtraEligible = GC.getDefineINT("TEAM_VOTE_MIN_CANDIDATES") - iCount;
	if (iExtraEligible <= 0)
	{
		return false;
	}

	for (int iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if (iI != eTeam)
		{
			CvTeam& kLoopTeam = GET_TEAM((TeamTypes)iI);
			if (kLoopTeam.isAlive())
			{
				if (!kLoopTeam.isForceTeamVoteEligible(eVoteSource))
				{
					if (kLoopTeam.isFullMember(eVoteSource))
					{
						int iLoopVotes = kLoopTeam.getVotes(NO_VOTE, eVoteSource);
						int iVotes = kTeam.getVotes(NO_VOTE, eVoteSource);
						if (iLoopVotes > iVotes || (iLoopVotes == iVotes && iI < eTeam))
						{
/* Munch edit 22/10/09, added by 45° on 02/17/13 */
							iExtraEligible--;
							if (bSecretaryElection == 1 && iI == incumbentTeam) iExtraEligible++;
/* the purpose of this is to not 'reserve' an election space due to higher vote count for the incumbent, who is supposed to be now ineligible */
/* exception being if the teamvote is actually a victory vote rather than a secretary general vote */
/* to revert, delete all except iExtraEligible--; */
						}
					}
				}
			}
		}
	}

	return (iExtraEligible > 0);
}


int CvGame::countVote(const VoteTriggeredData& kData, PlayerVoteTypes eChoice) const
{
	int iCount = 0;

	for (int iI = 0; iI < MAX_CIV_PLAYERS; ++iI)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (getPlayerVote(((PlayerTypes)iI), kData.getID()) == eChoice)
			{
				iCount += GET_PLAYER((PlayerTypes)iI).getVotes(kData.kVoteOption.eVote, kData.eVoteSource);
			}
		}
	}

	return iCount;
}


int CvGame::countPossibleVote(VoteTypes eVote, VoteSourceTypes eVoteSource) const
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if ( GET_PLAYER((PlayerTypes)iI).isAlive() )
		{
			iCount += GET_PLAYER((PlayerTypes)iI).getVotes(eVote, eVoteSource);
		}
	}

	return iCount;
}



TeamTypes CvGame::findHighestVoteTeam(const VoteTriggeredData& kData) const
{
	TeamTypes eBestTeam = NO_TEAM;
	int iBestCount = 0;

	if (isTeamVote(kData.kVoteOption.eVote))
	{
		for (int iI = 0; iI < MAX_CIV_TEAMS; ++iI)
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				int iCount = countVote(kData, (PlayerVoteTypes)iI);

				if (iCount > iBestCount)
				{
					iBestCount = iCount;
					eBestTeam = (TeamTypes)iI;
				}
			}
		}
	}

	return eBestTeam;
}


int CvGame::getVoteRequired(VoteTypes eVote, VoteSourceTypes eVoteSource) const
{
	return ((countPossibleVote(eVote, eVoteSource) * GC.getVoteInfo(eVote).getPopulationThreshold()) / 100);
}


TeamTypes CvGame::getSecretaryGeneral(VoteSourceTypes eVoteSource) const
{
	int iI;

	if (!canHaveSecretaryGeneral(eVoteSource))
	{
		for (int iBuilding = 0; iBuilding < GC.getNumBuildingInfos(); ++iBuilding)
		{
			if (GC.getBuildingInfo((BuildingTypes)iBuilding).getVoteSourceType() == eVoteSource)
			{
				for (iI = 0; iI < MAX_CIV_PLAYERS; ++iI)
				{
					CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iI);
					if (kLoopPlayer.isAlive())
					{
						if (kLoopPlayer.getBuildingClassCount((BuildingClassTypes)GC.getBuildingInfo((BuildingTypes)iBuilding).getBuildingClassType()) > 0)
						{
							ReligionTypes eReligion = getVoteSourceReligion(eVoteSource);
							if (NO_RELIGION == eReligion || kLoopPlayer.getStateReligion() == eReligion)
							{
								return kLoopPlayer.getTeam();
							}
						}
					}
				}
			}
		}
	}
	else
	{
		for (iI = 0; iI < GC.getNumVoteInfos(); iI++)
		{
			if (GC.getVoteInfo((VoteTypes)iI).isVoteSourceType(eVoteSource))
			{
				if (GC.getVoteInfo((VoteTypes)iI).isSecretaryGeneral())
				{
					if (isVotePassed((VoteTypes)iI))
					{
						return ((TeamTypes)(getVoteOutcome((VoteTypes)iI)));
					}
				}
			}
		}
	}


	return NO_TEAM;
}

bool CvGame::canHaveSecretaryGeneral(VoteSourceTypes eVoteSource) const
{
	for (int iI = 0; iI < GC.getNumVoteInfos(); iI++)
	{
		if (GC.getVoteInfo((VoteTypes)iI).isVoteSourceType(eVoteSource))
		{
			if (GC.getVoteInfo((VoteTypes)iI).isSecretaryGeneral())
			{
				return true;
			}
		}
	}

	return false;
}

void CvGame::clearSecretaryGeneral(VoteSourceTypes eVoteSource)
{
	for (int j = 0; j < GC.getNumVoteInfos(); ++j)
	{
		CvVoteInfo& kVote = GC.getVoteInfo((VoteTypes)j);

		if (kVote.isVoteSourceType(eVoteSource))
		{
			if (kVote.isSecretaryGeneral())
			{
				VoteTriggeredData kData;
				kData.eVoteSource = eVoteSource;
				kData.kVoteOption.eVote = (VoteTypes)j;
				kData.kVoteOption.iCityId = -1;
				kData.kVoteOption.szText.clear();
				kData.kVoteOption.ePlayer = NO_PLAYER;
				setVoteOutcome(kData, NO_PLAYER_VOTE);
				setSecretaryGeneralTimer(eVoteSource, 0);
			}
		}
	}
}

void CvGame::updateSecretaryGeneral()
{
	PROFILE_FUNC();

	for (int i = 0; i < GC.getNumVoteSourceInfos(); ++i)
	{
		TeamTypes eSecretaryGeneral = getSecretaryGeneral((VoteSourceTypes)i);
		if (NO_TEAM != eSecretaryGeneral && !GET_TEAM(eSecretaryGeneral).isFullMember((VoteSourceTypes)i))
		{
			clearSecretaryGeneral((VoteSourceTypes)i);
		}
	}
}

int CvGame::countCivPlayersAlive() const
{
	int iCount = 0;

	for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iCount++;
		}
	}

	return iCount;
}


int CvGame::countCivPlayersEverAlive() const
{
	int iCount = 0;

	for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes) iI);
		if (kPlayer.isEverAlive())
		{
			if (kPlayer.getParent() == NO_PLAYER)
			{
				iCount++;
			}
		}
	}

	return iCount;
}


int CvGame::countCivTeamsAlive() const
{
	int iCount = 0;

	for (int iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			iCount++;
		}
	}

	return iCount;
}


int CvGame::countCivTeamsEverAlive() const
{
	bool	teamsAlive[MAX_CIV_TEAMS];
	int		iCount = 0;

	memset(teamsAlive,0,sizeof(bool)*MAX_CIV_TEAMS);

	for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes) iI);
		if (kPlayer.isEverAlive())
		{
			if (kPlayer.getParent() == NO_PLAYER)
			{
				FAssert(0 <= kPlayer.getTeam());
				FAssert(MAX_CIV_TEAMS > kPlayer.getTeam());

				if ( !teamsAlive[kPlayer.getTeam()] )
				{
					teamsAlive[kPlayer.getTeam()] = true;
					iCount++;
				}
			}
		}
	}

	return iCount;
}


int CvGame::countHumanPlayersAlive() const
{
	int iCount = 0;

/************************************************************************************************/
/* REVOLUTION_MOD                         02/01/08                                jdog5000      */
/*                                                                                              */
/* Allow humans to control barbs                                                                */
/************************************************************************************************/
/* original code
	for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
*/
	for (int iI = 0; iI < MAX_PLAYERS; iI++)
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes) iI);
		if (kPlayer.isAlive() && kPlayer.isHuman())
		{
			iCount++;
		}
	}

	return iCount;
}


int CvGame::countTotalCivPower()
{
	int iCount = 0;

	for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes) iI);
		if (kPlayer.isAlive())
		{
			iCount += kPlayer.getPower();
		}
	}

	return iCount;
}


int CvGame::countTotalNukeUnits()
{
	int iCount = 0;
	for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes) iI);
		if (kPlayer.isAlive())
		{
			iCount += kPlayer.getNumNukeUnits();
		}
	}

	return iCount;
}


int CvGame::countKnownTechNumTeams(TechTypes eTech)
{
	int iCount = 0;

	for (int iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isEverAlive())
		{
			if (GET_TEAM((TeamTypes)iI).isHasTech(eTech))
			{
				if (!GET_TEAM((TeamTypes)iI).isBarbarian())
				{
					iCount++;
				}
			}
		}
	}

	return iCount;
}


int CvGame::getNumFreeBonuses(BuildingTypes eBuilding)
{
	if (GC.getBuildingInfo(eBuilding).getNumFreeBonuses() == -1)
	{
		return GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getNumFreeBuildingBonuses();
	}
	else
	{
		return GC.getBuildingInfo(eBuilding).getNumFreeBonuses();
	}
}


int CvGame::countReligionLevels(ReligionTypes eReligion)
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iCount += GET_PLAYER((PlayerTypes)iI).getHasReligionCount(eReligion);
		}
	}

	return iCount;
}

int CvGame::countCorporationLevels(CorporationTypes eCorporation)
{
	int iCount = 0;

	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iI);
		if (kLoopPlayer.isAlive())
		{
			iCount += GET_PLAYER((PlayerTypes)iI).getHasCorporationCount(eCorporation);
		}
	}

	return iCount;
}

void CvGame::replaceCorporation(CorporationTypes eCorporation1, CorporationTypes eCorporation2)
{
	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iI);
		if (kLoopPlayer.isAlive())
		{
			int iIter = 0;
			for (CvCity* pCity = kLoopPlayer.firstCity(&iIter); NULL != pCity; pCity = kLoopPlayer.nextCity(&iIter))
			{
				if (pCity->isHasCorporation(eCorporation1))
				{
					pCity->setHasCorporation(eCorporation1, false, false, false);
					pCity->setHasCorporation(eCorporation2, true, true);
				}
			}

			for (CvUnit* pUnit = kLoopPlayer.firstUnit(&iIter); NULL != pUnit; pUnit = kLoopPlayer.nextUnit(&iIter))
			{
				if (pUnit->getUnitInfo().getCorporationSpreads(eCorporation1) > 0)
				{
					pUnit->kill(false);
				}
			}
		}
	}
}


int CvGame::calculateReligionPercent(ReligionTypes eReligion) const
{
	CvCity* pLoopCity;
	int iCount;
	int iLoop = 0;
	int iI;

	if (getTotalPopulation() == 0)
	{
		return 0;
	}

	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
			{
				if (pLoopCity->isHasReligion(eReligion))
				{
					iCount += ((pLoopCity->getPopulation() + (pLoopCity->getReligionCount() / 2)) / pLoopCity->getReligionCount());
				}
			}
		}
	}

	return ((iCount * 100) / getTotalPopulation());
}


int CvGame::goldenAgeLength() const
{
	int iLength;

	iLength = GC.getDefineINT("GOLDEN_AGE_LENGTH");

	iLength *= GC.getGameSpeedInfo(getGameSpeedType()).getGoldenAgePercent();
	iLength /= 100;

	return iLength;
}

int CvGame::victoryDelay(VictoryTypes eVictory) const
{
	FAssert(eVictory >= 0 && eVictory < GC.getNumVictoryInfos());

	int iLength = GC.getVictoryInfo(eVictory).getVictoryDelayTurns();

	iLength *= GC.getGameSpeedInfo(getGameSpeedType()).getVictoryDelayPercent();
	iLength /= 100;

	return iLength;
}



int CvGame::getImprovementUpgradeTime(ImprovementTypes eImprovement) const
{
	int iTime;

	iTime = GC.getImprovementInfo(eImprovement).getUpgradeTime();

	iTime *= GC.getGameSpeedInfo(getGameSpeedType()).getImprovementPercent();
	iTime /= 100;

	iTime *= GC.getEraInfo(getStartEra()).getImprovementPercent();
	iTime /= 100;

	return iTime;
}


bool CvGame::canTrainNukes() const
{
	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iI);
		if (kPlayer.isAlive())
		{
			for (int iJ = 0; iJ < GC.getNumUnitClassInfos(); iJ++)
			{
				UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(kPlayer.getCivilizationType()).getCivilizationUnits((UnitClassTypes)iJ);

				if (NO_UNIT != eUnit)
				{
					if (-1 != GC.getUnitInfo(eUnit).getNukeRange())
					{
						if (kPlayer.canTrain(eUnit))
						{
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}



/************************************************************************************************/
/* RevDCM	                  Start		 11/04/10                                phungus420     */
/*                                                                                              */
/* New World Logic                                                                              */
/************************************************************************************************/
EraTypes CvGame::getHighestEra() const
{
	int iI;
	int iHighestEra = 0;
	int iLoopEra;
	
	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iLoopEra = GET_PLAYER((PlayerTypes)iI).getCurrentEra();

			if(iLoopEra > iHighestEra)
			{
				iHighestEra = iLoopEra;
			}
		}
	}

	return EraTypes(iHighestEra);
}
/************************************************************************************************/
/* LIMITED_RELIGIONS               END                                                          */
/************************************************************************************************/
EraTypes CvGame::getCurrentEra() const
{
	int iEra;
	int iCount;
	int iI;

	iEra = 0;
	iCount = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iEra += GET_PLAYER((PlayerTypes)iI).getCurrentEra();
			iCount++;
		}
	}

	if (iCount > 0)
	{
		return ((EraTypes)(iEra / iCount));
	}

	return NO_ERA;
}


TeamTypes CvGame::getActiveTeam() const
{
	if (getActivePlayer() == NO_PLAYER)
	{
		return NO_TEAM;	
	}
	else
	{
		return (TeamTypes)GET_PLAYER(getActivePlayer()).getTeam();
	}
}


CivilizationTypes CvGame::getActiveCivilizationType() const
{
	if (getActivePlayer() == NO_PLAYER)
	{
		return NO_CIVILIZATION;
	}
	else
	{
		return (CivilizationTypes)GET_PLAYER(getActivePlayer()).getCivilizationType();
	}
}


bool CvGame::isNetworkMultiPlayer() const																	 
{
	return GC.getInitCore().getMultiplayer();
}


bool CvGame::isGameMultiPlayer() const																 
{
	return GC.getInitCore().getMultiplayer() || isPbem();
}


bool CvGame::isTeamGame() const																		 
{
	FAssert(countCivPlayersAlive() >= countCivTeamsAlive());
	return (countCivPlayersAlive() > countCivTeamsAlive());
}


bool CvGame::isModem()
{
	return gDLL->IsModem();
}
void CvGame::setModem(bool bModem)
{
	if (bModem)
	{
		gDLL->ChangeINIKeyValue("CONFIG", "Bandwidth", "modem");
	}
	else
	{
		gDLL->ChangeINIKeyValue("CONFIG", "Bandwidth", "broadband");
	}

	gDLL->SetModem(bModem);
}
/************************************************************************************************/
/* REVOLUTION_MOD                                                                 lemmy101      */
/*                                                                                jdog5000      */
/*                                                                                              */
/************************************************************************************************/
void CvGame::reviveActivePlayer()
{
	if (!GET_PLAYER(getActivePlayer()).isAlive())
	{
		if (isForcedAIAutoPlay(getActivePlayer()))
		{
			setForcedAIAutoPlay(getActivePlayer(), 0);
		}
		else setAIAutoPlay(getActivePlayer(), 0);

		GC.getInitCore().setSlotStatus(getActivePlayer(), SS_TAKEN);

		GET_PLAYER(getActivePlayer()).initUnit(((UnitTypes)0), 0, 0, NO_UNITAI, NO_DIRECTION, 0);
	}
}

void CvGame::reviveActivePlayer(PlayerTypes iPlayer)
{
	if (!GET_PLAYER(iPlayer).isAlive())
	{
		if (isForcedAIAutoPlay(iPlayer))
		{
			setForcedAIAutoPlay(iPlayer, 0);
		}
		else setAIAutoPlay(iPlayer, 0);

		GC.getInitCore().setSlotStatus(iPlayer, SS_TAKEN);

		GET_PLAYER(iPlayer).initUnit(((UnitTypes)0), 0, 0, NO_UNITAI, NO_DIRECTION, 0);
	}
}
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/


int CvGame::getNumHumanPlayers()
{
	return GC.getInitCore().getNumHumans();
}

int CvGame::getGameTurn()
{
	return GC.getInitCore().getGameTurn();
}


void CvGame::setGameTurn(int iNewValue)
{
	if (getGameTurn() != iNewValue)
	{
		GC.getInitCore().setGameTurn(iNewValue);
		FAssert(getGameTurn() >= 0);

		if (m_iDateTurn == iNewValue - 1)
		{
			m_currentDate.increment();
		}
		else
		{
			m_currentDate = CvDate::getDate(iNewValue);
		}
		m_iDateTurn = iNewValue;

		updateBuildingCommerce();

		setScoreDirty(true);

		gDLL->getInterfaceIFace()->setDirty(TurnTimer_DIRTY_BIT, true);
		gDLL->getInterfaceIFace()->setDirty(GameData_DIRTY_BIT, true);
	}
}


void CvGame::incrementGameTurn()
{
	setGameTurn(getGameTurn() + 1);
}


int CvGame::getTurnYear(int iGameTurn)
{
	// moved the body of this method to Game Core Utils so we have access for other games than the current one (replay screen in HOF)
	return getTurnYearForGame(iGameTurn, getStartYear(), getCalendar(), getGameSpeedType());
}


int CvGame::getGameTurnYear()
{
	return getTurnYear(getGameTurn());
}

CvDate CvGame::getCurrentDate()
{
	if (m_iDateTurn != getGameTurn())
	{
		m_currentDate = CvDate::getDate(getGameTurn());
		m_iDateTurn = getGameTurn();
	}
	return m_currentDate;
}


int CvGame::getElapsedGameTurns() const
{
	return m_iElapsedGameTurns;
}


void CvGame::incrementElapsedGameTurns()
{
	m_iElapsedGameTurns++;
}


int CvGame::getMaxTurns() const
{
	return GC.getInitCore().getMaxTurns();
}


void CvGame::setMaxTurns(int iNewValue)
{
	GC.getInitCore().setMaxTurns(iNewValue);
	FAssert(getMaxTurns() >= 0);
}


void CvGame::changeMaxTurns(int iChange)
{
	setMaxTurns(getMaxTurns() + iChange);
}


int CvGame::getMaxCityElimination() const
{
	return GC.getInitCore().getMaxCityElimination();
}


void CvGame::setMaxCityElimination(int iNewValue)
{
	GC.getInitCore().setMaxCityElimination(iNewValue);
	FAssert(getMaxCityElimination() >= 0);
}

int CvGame::getNumAdvancedStartPoints() const
{
	return GC.getInitCore().getNumAdvancedStartPoints();
}


void CvGame::setNumAdvancedStartPoints(int iNewValue)
{
	GC.getInitCore().setNumAdvancedStartPoints(iNewValue);
	FAssert(getNumAdvancedStartPoints() >= 0);
}

int CvGame::getStartTurn() const
{
	return m_iStartTurn;
}


void CvGame::setStartTurn(int iNewValue)
{
	m_iStartTurn = iNewValue;
}


int CvGame::getStartYear() const
{
	return m_iStartYear;
}


void CvGame::setStartYear(int iNewValue)
{
	m_iStartYear = iNewValue;
}


int CvGame::getEstimateEndTurn() const
{
	return m_iEstimateEndTurn;
}


void CvGame::setEstimateEndTurn(int iNewValue)
{
	m_iEstimateEndTurn = iNewValue;
}


int CvGame::getTurnSlice() const
{
	return m_iTurnSlice;
}


int CvGame::getMinutesPlayed() const
{
	return (getTurnSlice() / gDLL->getTurnsPerMinute());
}


void CvGame::setTurnSlice(int iNewValue)
{
	m_iTurnSlice = iNewValue;
}


void CvGame::changeTurnSlice(int iChange)
{
	PROFILE_FUNC();

	setTurnSlice(getTurnSlice() + iChange);
}


int CvGame::getCutoffSlice() const
{
	return m_iCutoffSlice;
}


void CvGame::setCutoffSlice(int iNewValue)
{
	m_iCutoffSlice = iNewValue;
}


void CvGame::changeCutoffSlice(int iChange)
{
	setCutoffSlice(getCutoffSlice() + iChange);
}


int CvGame::getTurnSlicesRemaining()
{
	return (getCutoffSlice() - getTurnSlice());
}


void CvGame::resetTurnTimer()
{
	// We should only use the turn timer if we are in multiplayer
	if (isMPOption(MPOPTION_TURN_TIMER))
	{
		if (getElapsedGameTurns() > 0 || !isOption(GAMEOPTION_ADVANCED_START))
		{
			// Determine how much time we should allow
			int iTurnLen = getMaxTurnLen();
			if (getElapsedGameTurns() == 0 && !isPitboss())
			{
				// Let's allow more time for the initial turn
				TurnTimerTypes eTurnTimer = GC.getInitCore().getTurnTimer();
				FAssertMsg(eTurnTimer >= 0 && eTurnTimer < GC.getNumTurnTimerInfos(), "Invalid TurnTimer selection in InitCore");
				iTurnLen = (iTurnLen * GC.getTurnTimerInfo(eTurnTimer).getFirstTurnMultiplier());
			}
			// Set the current turn slice to start the 'timer'
			setCutoffSlice(getTurnSlice() + iTurnLen);
		}
	}
}

void CvGame::incrementTurnTimer(int iNumTurnSlices)
{
	if (isMPOption(MPOPTION_TURN_TIMER))
	{
		// If the turn timer has expired, we shouldn't increment it as we've sent our turn complete message
		if (getTurnSlice() <= getCutoffSlice())
		{
			changeCutoffSlice(iNumTurnSlices);
		}
	}
}


int CvGame::getMaxTurnLen()
{
	if (isPitboss())
	{
		// Use the user provided input
		// Turn time is in hours
		return ( getPitbossTurnTime() * 3600 * 4);
	}
	else
	{
		int iMaxUnits = 0;
		int iMaxCities = 0;

		// Find out who has the most units and who has the most cities
		// Calculate the max turn time based on the max number of units and cities
		for (int i = 0; i < MAX_CIV_PLAYERS; ++i)
		{
			if (GET_PLAYER((PlayerTypes)i).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)i).getNumUnits() > iMaxUnits)
				{
					iMaxUnits = GET_PLAYER((PlayerTypes)i).getNumUnits();
				}
				if (GET_PLAYER((PlayerTypes)i).getNumCities() > iMaxCities)
				{
					iMaxCities = GET_PLAYER((PlayerTypes)i).getNumCities();
				}
			}
		}

		// Now return turn len based on base len and unit and city bonuses
		TurnTimerTypes eTurnTimer = GC.getInitCore().getTurnTimer();
		FAssertMsg(eTurnTimer >= 0 && eTurnTimer < GC.getNumTurnTimerInfos(), "Invalid TurnTimer Selection in InitCore");
		return ( GC.getTurnTimerInfo(eTurnTimer).getBaseTime() + 
			    (GC.getTurnTimerInfo(eTurnTimer).getCityBonus()*iMaxCities) +
				(GC.getTurnTimerInfo(eTurnTimer).getUnitBonus()*iMaxUnits) );
	}
}


int CvGame::getTargetScore() const
{
	return GC.getInitCore().getTargetScore();
}


void CvGame::setTargetScore(int iNewValue)
{
	GC.getInitCore().setTargetScore(iNewValue);
	FAssert(getTargetScore() >= 0);
}


int CvGame::getNumGameTurnActive()
{
	return m_iNumGameTurnActive;
}


int CvGame::countNumHumanGameTurnActive() const
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isHuman())
		{
			if (GET_PLAYER((PlayerTypes)iI).isTurnActive())
			{
				iCount++;
			}
		}
	}

	return iCount;
}


void CvGame::changeNumGameTurnActive(int iChange)
{
	m_iNumGameTurnActive = (m_iNumGameTurnActive + iChange);
	FAssert(getNumGameTurnActive() >= 0);
}


int CvGame::getNumCities() const																		
{
	return m_iNumCities;
}


int CvGame::getNumCivCities() const
{
	return (getNumCities() - GET_PLAYER(BARBARIAN_PLAYER).getNumCities());
}


void CvGame::changeNumCities(int iChange)
{
	m_iNumCities = (m_iNumCities + iChange);
	FAssert(getNumCities() >= 0);
}


int CvGame::getTotalPopulation() const
{
	return m_iTotalPopulation;
}


void CvGame::changeTotalPopulation(int iChange)
{
	m_iTotalPopulation = (m_iTotalPopulation + iChange);
	FAssert(getTotalPopulation() >= 0);
}


int CvGame::getTradeRoutes() const
{
	return m_iTradeRoutes;
}


void CvGame::changeTradeRoutes(int iChange)
{
	if (iChange != 0)
	{
		m_iTradeRoutes = (m_iTradeRoutes + iChange);
		FAssert(getTradeRoutes() >= 0);

		updateTradeRoutes();
	}
}


int CvGame::getFreeTradeCount() const
{
	return m_iFreeTradeCount;
}


bool CvGame::isFreeTrade() const
{
	return (getFreeTradeCount() > 0);
}


void CvGame::changeFreeTradeCount(int iChange)
{
	bool bOldFreeTrade;

	if (iChange != 0)
	{
		bOldFreeTrade = isFreeTrade();

		m_iFreeTradeCount = (m_iFreeTradeCount + iChange);
		FAssert(getFreeTradeCount() >= 0);

		if (bOldFreeTrade != isFreeTrade())
		{
			updateTradeRoutes();
		}
	}
}


int CvGame::getNoNukesCount() const
{
	return m_iNoNukesCount;
}


bool CvGame::isNoNukes() const
{
	if (GC.getGameINLINE().isOption(GAMEOPTION_NO_NUKES))
	{
		return true;
	}
	return (getNoNukesCount() > 0);
}


void CvGame::changeNoNukesCount(int iChange)
{
	m_iNoNukesCount = (m_iNoNukesCount + iChange);
	FAssert(getNoNukesCount() >= 0);
}


int CvGame::getSecretaryGeneralTimer(VoteSourceTypes eVoteSource) const
{
	FAssert(eVoteSource >= 0);
	FAssert(eVoteSource < GC.getNumVoteSourceInfos());
	return m_aiSecretaryGeneralTimer[eVoteSource];
}


void CvGame::setSecretaryGeneralTimer(VoteSourceTypes eVoteSource, int iNewValue)
{
	FAssert(eVoteSource >= 0);
	FAssert(eVoteSource < GC.getNumVoteSourceInfos());
	m_aiSecretaryGeneralTimer[eVoteSource] = iNewValue;
	FAssert(getSecretaryGeneralTimer(eVoteSource) >= 0);
}


void CvGame::changeSecretaryGeneralTimer(VoteSourceTypes eVoteSource, int iChange)
{
	setSecretaryGeneralTimer(eVoteSource, getSecretaryGeneralTimer(eVoteSource) + iChange);
}


int CvGame::getVoteTimer(VoteSourceTypes eVoteSource) const
{
	FAssert(eVoteSource >= 0);
	FAssert(eVoteSource < GC.getNumVoteSourceInfos());
	return m_aiVoteTimer[eVoteSource];
}


void CvGame::setVoteTimer(VoteSourceTypes eVoteSource, int iNewValue)
{
	FAssert(eVoteSource >= 0);
	FAssert(eVoteSource < GC.getNumVoteSourceInfos());
	m_aiVoteTimer[eVoteSource] = iNewValue;
	FAssert(getVoteTimer(eVoteSource) >= 0);
}


void CvGame::changeVoteTimer(VoteSourceTypes eVoteSource, int iChange)
{
	setVoteTimer(eVoteSource, getVoteTimer(eVoteSource) + iChange);
}


int CvGame::getNukesExploded() const
{
	return m_iNukesExploded;
}


void CvGame::changeNukesExploded(int iChange)
{
	m_iNukesExploded = (m_iNukesExploded + iChange);
}


int CvGame::getMaxPopulation() const
{
	return m_iMaxPopulation;
}


int CvGame::getMaxLand() const
{
	return m_iMaxLand;
}


int CvGame::getMaxTech() const
{
	return m_iMaxTech;
}


int CvGame::getMaxWonders() const
{
	return m_iMaxWonders;
}


int CvGame::getInitPopulation() const
{
	return m_iInitPopulation;
}


int CvGame::getInitLand() const
{
	return m_iInitLand;
}


int CvGame::getInitTech() const
{
	return m_iInitTech;
}


int CvGame::getInitWonders() const
{
	return m_iInitWonders;
}


void CvGame::initScoreCalculation()
{
	// initialize score calculation
	int iMaxFood = 0;
	for (int i = 0; i < GC.getMapINLINE().numPlotsINLINE(); i++)
	{
		CvPlot* pPlot = GC.getMapINLINE().plotByIndexINLINE(i);
		if (!pPlot->isWater() || pPlot->isAdjacentToLand())
		{
			iMaxFood += pPlot->calculateBestNatureYield(YIELD_FOOD, NO_TEAM);
		}
	}
	m_iMaxPopulation = getPopulationScore(iMaxFood / std::max(1, GC.getFOOD_CONSUMPTION_PER_POPULATION()));
	m_iMaxLand = getLandPlotsScore(GC.getMapINLINE().getLandPlots());
	m_iMaxTech = 0;
	for (int i = 0; i < GC.getNumTechInfos(); i++)
	{
		m_iMaxTech += getTechScore((TechTypes)i);
	}
	m_iMaxWonders = 0;
	for (int i = 0; i < GC.getNumBuildingClassInfos(); i++)
	{
		m_iMaxWonders += getWonderScore((BuildingClassTypes)i);
	}

	if (NO_ERA != getStartEra())
	{
		int iNumSettlers = GC.getEraInfo(getStartEra()).getStartingUnitMultiplier();
		m_iInitPopulation = getPopulationScore(iNumSettlers * (GC.getEraInfo(getStartEra()).getFreePopulation() + 1));
		m_iInitLand = getLandPlotsScore(iNumSettlers *  NUM_CITY_PLOTS);
	}
	else
	{
		m_iInitPopulation = 0;
		m_iInitLand = 0;
	}

	m_iInitTech = 0;
	for (int i = 0; i < GC.getNumTechInfos(); i++)
	{
		if (GC.getTechInfo((TechTypes)i).getEra() < getStartEra())
		{
			m_iInitTech += getTechScore((TechTypes)i);
		}
		else
		{
			// count all possible free techs as initial to lower the score from immediate retirement
			for (int iCiv = 0; iCiv < GC.getNumCivilizationInfos(); iCiv++)
			{
				if (GC.getCivilizationInfo((CivilizationTypes)iCiv).isPlayable())
				{
					if (GC.getCivilizationInfo((CivilizationTypes)iCiv).isCivilizationFreeTechs(i))
					{
						m_iInitTech += getTechScore((TechTypes)i);
						break;
					}
				}
			}
		}
	}
	m_iInitWonders = 0;
}

/************************************************************************************************/
/* REVOLUTION_MOD                                                                  lemmy101     */
/*                                                                                 jdog5000     */
/*                                                                                              */
/************************************************************************************************/
int CvGame::getAIAutoPlay(PlayerTypes iPlayer)
{
	return m_iAIAutoPlay[iPlayer];
}

void CvGame::setAIAutoPlay(PlayerTypes iPlayer, int iNewValue, bool bForced)
{
	FAssert(iNewValue >= 0);
	if(isForcedAIAutoPlay(iPlayer) && !bForced)
	{
		return;
	}

	if (GC.getLogging() )
	{
			TCHAR szOut[1024];
			sprintf(szOut, "setAutoPlay called for player %d - set to: %d\n", iPlayer, iNewValue);
			gDLL->messageControlLog(szOut);
	}

	int iOldValue= getAIAutoPlay(iPlayer);

	if (iOldValue != iNewValue)
	{
		m_iAIAutoPlay[iPlayer] = std::max(0, iNewValue);

		if (iNewValue > 0)
		{
			GET_PLAYER(iPlayer).setHumanDisabled(true);
		} else
		{
			GET_PLAYER(iPlayer).setHumanDisabled(false);
		}
	}
}

void CvGame::changeAIAutoPlay(PlayerTypes iPlayer, int iChange)
{
	setAIAutoPlay(iPlayer, (getAIAutoPlay(iPlayer) + iChange));
}


bool CvGame::isForcedAIAutoPlay(PlayerTypes iPlayer)
{
	FAssert(getForcedAIAutoPlay(iPlayer) >=0);

	if(getForcedAIAutoPlay(iPlayer) > 0)
	{
		return true;
	}
	return false;
}

int CvGame::getForcedAIAutoPlay(PlayerTypes iPlayer)
{
	return m_iForcedAIAutoPlay[iPlayer];
}

void CvGame::setForcedAIAutoPlay(PlayerTypes iPlayer, int iNewValue, bool bForced)
{
	FAssert(iNewValue >= 0);

	int iOldValue;
	if(bForced == true)
	{

		iOldValue = getForcedAIAutoPlay(iPlayer);

		if (iOldValue != iNewValue)
		{
			m_iForcedAIAutoPlay[iPlayer] = std::max(0, iNewValue);
			setAIAutoPlay(iPlayer, iNewValue, true);
		} else
		{
			setAIAutoPlay(iPlayer, iNewValue, true);
		}
	} else
	{
		m_iForcedAIAutoPlay[iPlayer] = 0;

		iOldValue = m_iAIAutoPlay[iPlayer];
		
		if(iOldValue != iNewValue)
		{
			setAIAutoPlay(iPlayer, iNewValue);
		}
	}
}

void CvGame::changeForcedAIAutoPlay(PlayerTypes iPlayer, int iChange)
{
	if(isForcedAIAutoPlay(iPlayer))
	{
		setForcedAIAutoPlay(iPlayer, (getAIAutoPlay(iPlayer) + iChange), true);
	} else
	{
		setForcedAIAutoPlay(iPlayer, (getAIAutoPlay(iPlayer) + iChange));
	}
}
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/


unsigned int CvGame::getInitialTime()
{
	return m_uiInitialTime;
}


void CvGame::setInitialTime(unsigned int uiNewValue)
{
	m_uiInitialTime = uiNewValue;
}


bool CvGame::isScoreDirty() const
{
	return m_bScoreDirty;
}


void CvGame::setScoreDirty(bool bNewValue)
{
	m_bScoreDirty = bNewValue;
}


bool CvGame::isCircumnavigated(TeamTypes eTeam) const
{
	if ( eTeam == NO_TEAM )
	{
		return (m_circumnavigatingTeam != NO_TEAM);
	}
	else
	{
		return (m_circumnavigatingTeam == eTeam);
	}
}


void CvGame::makeCircumnavigated(TeamTypes eTeam)
{
	m_circumnavigatingTeam = eTeam;
}

bool CvGame::circumnavigationAvailable() const
{
	if (isCircumnavigated())
	{
		return false;
	}

	if (GC.getDefineINT("CIRCUMNAVIGATE_FREE_MOVES") == 0)
	{
		return false;
	}

	CvMap& kMap = GC.getMapINLINE();

	if (!(kMap.isWrapXINLINE()) && !(kMap.isWrapYINLINE()))
	{
		return false;
	}

	if (kMap.getLandPlots() > ((kMap.numPlotsINLINE() * 2) / 3))
	{
		return false;
	}

	return true;
}

bool CvGame::isDiploVote(VoteSourceTypes eVoteSource) const
{
	return (getDiploVoteCount(eVoteSource) > 0);
}


int CvGame::getDiploVoteCount(VoteSourceTypes eVoteSource) const
{
	FAssert(eVoteSource >= 0 && eVoteSource < GC.getNumVoteSourceInfos());
	return m_aiDiploVote[eVoteSource];
}

/*void CvGame::startDeferredVoteSourceProcessing()
{
	FAssert(!m_bDeferVoteSourceProcessing);

	OutputDebugString("Start game defrral of vote source processing\n");

	m_bDeferVoteSourceProcessing = true;

	m_paiDeferredVoteSources = new int[GC.getNumVoteSourceInfos()];

	for(int iI = 0; iI < GC.getNumVoteSourceInfos(); iI++)
	{
		m_paiDeferredVoteSources[iI] = 0;
	}
}

void CvGame::endDeferredVoteSourceProcessing()
{
	FAssert(m_bDeferVoteSourceProcessing);

	OutputDebugString("End game defrral of vote source processing\n");

	for(int iI = 0; iI < GC.getNumVoteSourceInfos(); iI++)
	{
		changeDiploVote((VoteSourceTypes)iI, m_paiDeferredVoteSources[iI]);
	}

	SAFE_DELETE_ARRAY(m_paiDeferredVoteSources);

	m_bDeferVoteSourceProcessing = false;
}*/

void CvGame::changeDiploVote(VoteSourceTypes eVoteSource, int iChange)
{
	FAssert(eVoteSource >= 0 && eVoteSource < GC.getNumVoteSourceInfos());

	if (0 != iChange)
	{
	/*	if ( m_bDeferVoteSourceProcessing )
		{
			int iOldValue;

			//	Do this in a thread-safe manner
			do
			{
				iOldValue = m_paiDeferredVoteSources[eVoteSource];
			} while( InterlockedCompareExchange((volatile LONG*)&m_paiDeferredVoteSources[eVoteSource], iOldValue + iChange, iOldValue) != iOldValue );
		}
		else
		{*/
			if ( !m_bRecalculatingModifiers )
			{
				for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
				{
					GET_PLAYER((PlayerTypes)iPlayer).processVoteSourceBonus(eVoteSource, false);
				}
			}

			m_aiDiploVote[eVoteSource] += iChange;
			FAssert(getDiploVoteCount(eVoteSource) >= 0);

			if ( !m_bRecalculatingModifiers )
			{
				for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
				{
					GET_PLAYER((PlayerTypes)iPlayer).processVoteSourceBonus(eVoteSource, true);
				}
			}
		//}
	}
}

bool CvGame::canDoResolution(VoteSourceTypes eVoteSource, const VoteSelectionSubData& kData) const
{
	if (GC.getVoteInfo(kData.eVote).isVictory())
	{
	for (int iJ = 0; iJ < GC.getNumVictoryInfos(); iJ++)
	{
		if (GC.getVictoryInfo((VictoryTypes)iJ).isTotalVictory())
		{
			if (isVictoryValid((VictoryTypes)iJ))
			{
				return false; // 45° - No vote for diplomatic victory if Mastery victory is active
			}	
		}			
	}		
		for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; ++iTeam)
		{
			CvTeam& kTeam = GET_TEAM((TeamTypes)iTeam);

			if (kTeam.isVotingMember(eVoteSource))
			{
				if (kTeam.getVotes(kData.eVote, eVoteSource) >= getVoteRequired(kData.eVote, eVoteSource) || (isOption(GAMEOPTION_UNITED_NATIONS)))
				{
					// Can't vote on a winner if one team already has all the votes necessary to win or if UN Option is active
					return false;
				}
			}
		}
	}

	for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iPlayer);

		if ( kPlayer.isAlive() )
		{
			if (kPlayer.isVotingMember(eVoteSource))
			{
				if (!kPlayer.canDoResolution(eVoteSource, kData))
				{
					return false;
				}
			}
			else if (!kPlayer.isBarbarian() && !kPlayer.isMinorCiv())
			{
				// all players need to be able to vote for a diplo victory
				if (GC.getVoteInfo(kData.eVote).isVictory())
				{
					return false;
				}
			}
		}
	}

	return true;
}

bool CvGame::isValidVoteSelection(VoteSourceTypes eVoteSource, const VoteSelectionSubData& kData) const
{
	if (NO_PLAYER != kData.ePlayer)
	{
		CvPlayer& kPlayer = GET_PLAYER(kData.ePlayer);
		if (!kPlayer.isAlive() || kPlayer.isBarbarian() || kPlayer.isMinorCiv())
		{
			return false;
		}
	}

	if (NO_PLAYER != kData.eOtherPlayer)
	{
		CvPlayer& kPlayer = GET_PLAYER(kData.eOtherPlayer);
		if (!kPlayer.isAlive() || kPlayer.isBarbarian() || kPlayer.isMinorCiv())
		{
			return false;
		}
	}

	int iNumVoters = 0;
	for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; ++iTeam)
	{
		if (GET_TEAM((TeamTypes)iTeam).isVotingMember(eVoteSource))
		{
			++iNumVoters;
		}
	}
	if (iNumVoters  < GC.getVoteInfo(kData.eVote).getMinVoters())
	{
		return false;
	}

	if (GC.getVoteInfo(kData.eVote).isOpenBorders())
	{
		bool bOpenWithEveryone = true;
		for (int iTeam1 = 0; iTeam1 < MAX_CIV_TEAMS; ++iTeam1)
		{
			if (GET_TEAM((TeamTypes)iTeam1).isFullMember(eVoteSource))
			{
				for (int iTeam2 = iTeam1 + 1; iTeam2 < MAX_CIV_TEAMS; ++iTeam2)
				{
					CvTeam& kTeam2 = GET_TEAM((TeamTypes)iTeam2);

					if (kTeam2.isFullMember(eVoteSource))
					{
						if (!kTeam2.isOpenBorders((TeamTypes)iTeam1))
						{
							bOpenWithEveryone = false;
							break;
						}
					}
				}
			}
		}
		if (bOpenWithEveryone)
		{
			return false;
		}
	}
	else if (GC.getVoteInfo(kData.eVote).isDefensivePact())
	{
		bool bPactWithEveryone = true;
		for (int iTeam1 = 0; iTeam1 < MAX_CIV_TEAMS; ++iTeam1)
		{
			if (GET_TEAM((TeamTypes)iTeam1).isFullMember(eVoteSource))
			{
				for (int iTeam2 = iTeam1 + 1; iTeam2 < MAX_CIV_TEAMS; ++iTeam2)
				{
					CvTeam& kTeam2 = GET_TEAM((TeamTypes)iTeam2);

					if (kTeam2.isFullMember(eVoteSource))
					{
						if (!kTeam2.isDefensivePact((TeamTypes)iTeam1))
						{
							bPactWithEveryone = false;
							break;
						}
					}
				}
			}
		}
		if (bPactWithEveryone)
		{
			return false;
		}
	}
	else if (GC.getVoteInfo(kData.eVote).isForcePeace())
	{
		CvPlayer& kPlayer = GET_PLAYER(kData.ePlayer);

		if (GET_TEAM(kPlayer.getTeam()).isAVassal())
		{
			return false;
		}

		if (!kPlayer.isFullMember(eVoteSource))
		{
			return false;
		}

		bool bValid = false;

		for (int iTeam2 = 0; iTeam2 < MAX_CIV_TEAMS; ++iTeam2)
		{
			if (atWar(kPlayer.getTeam(), (TeamTypes)iTeam2))
			{
				CvTeam& kTeam2 = GET_TEAM((TeamTypes)iTeam2);

				if (kTeam2.isVotingMember(eVoteSource))
				{
					bValid = true;
					break;
				}
			}
		}

		if (!bValid)
		{
			return false;
		}
	}
	else if (GC.getVoteInfo(kData.eVote).isForceNoTrade())
	{
		CvPlayer& kPlayer = GET_PLAYER(kData.ePlayer);

		if (kPlayer.isFullMember(eVoteSource))
		{
			return false;
		}

		bool bNoTradeWithEveryone = true;
		for (int iPlayer2 = 0; iPlayer2 < MAX_CIV_PLAYERS; ++iPlayer2)
		{
			CvPlayer& kPlayer2 = GET_PLAYER((PlayerTypes)iPlayer2);
			if (kPlayer2.isAlive() && kPlayer2.getTeam() != kPlayer.getTeam())
			{
				if (kPlayer2.isFullMember(eVoteSource))
				{
					if (kPlayer2.canStopTradingWithTeam(kPlayer.getTeam()))
					{
						bNoTradeWithEveryone = false;
						break;
					}
				}
			}
		}
		// Not an option if already at war with everyone
		if (bNoTradeWithEveryone)
		{
			return false;
		}
	}
	else if (GC.getVoteInfo(kData.eVote).isForceWar())
	{
		CvPlayer& kPlayer = GET_PLAYER(kData.ePlayer);
		CvTeam& kTeam = GET_TEAM(kPlayer.getTeam());

		if (kTeam.isAVassal())
		{
			return false;
		}

		if (kPlayer.isFullMember(eVoteSource))
		{
			return false;
		}

		bool bAtWarWithEveryone = true;
		for (int iTeam2 = 0; iTeam2 < MAX_CIV_TEAMS; ++iTeam2)
		{
			if (iTeam2 != kPlayer.getTeam())
			{
				CvTeam& kTeam2 = GET_TEAM((TeamTypes)iTeam2);
				if (kTeam2.isFullMember(eVoteSource))
				{
					if (!kTeam2.isAtWar(kPlayer.getTeam()) && kTeam2.canChangeWarPeace(kPlayer.getTeam()))
					{
						bAtWarWithEveryone = false;
						break;
					}
				}
			}
		}
		// Not an option if already at war with everyone
		if (bAtWarWithEveryone)
		{
			return false;
		}

		// Can be passed against a non-member only if he is already at war with a member 
		if (!kPlayer.isVotingMember(eVoteSource))
		{
			bool bValid = false;
			for (int iTeam2 = 0; iTeam2 < MAX_CIV_TEAMS; ++iTeam2)
			{
				if (atWar(kPlayer.getTeam(), (TeamTypes)iTeam2))
				{
					CvTeam& kTeam2 = GET_TEAM((TeamTypes)iTeam2);

					if (kTeam2.isFullMember(eVoteSource))
					{
						bValid = true;
						break;
					}
				}
			}

			if (!bValid)
			{
				return false;
			}
		}
	}
	else if (GC.getVoteInfo(kData.eVote).isAssignCity())
	{
		CvPlayer& kPlayer = GET_PLAYER(kData.ePlayer);
		if (kPlayer.isFullMember(eVoteSource) || !kPlayer.isVotingMember(eVoteSource))
		{
			return false;
		}

		CvCity* pCity = kPlayer.getCity(kData.iCityId);
		FAssert(NULL != pCity);
		if (NULL == pCity)
		{
			return false;
		}

		if (NO_PLAYER == kData.eOtherPlayer)
		{
			return false;
		}

		CvPlayer& kOtherPlayer = GET_PLAYER(kData.eOtherPlayer);
		if (kOtherPlayer.getTeam() == kPlayer.getTeam())
		{
			return false;
		}

		if (atWar(kPlayer.getTeam(), GET_PLAYER(kData.eOtherPlayer).getTeam()))
		{
			return false;
		}

		if (!kOtherPlayer.isFullMember(eVoteSource))
		{
			return false;			
		}

		if (kOtherPlayer.isHuman() && isOption(GAMEOPTION_ONE_CITY_CHALLENGE))
		{
			return false;
		}
	}

	if (!canDoResolution(eVoteSource, kData))
	{
		return false;
	}

	return true;
}


bool CvGame::isDebugMode() const
{
	return m_bDebugModeCache;
}


void CvGame::toggleDebugMode()
{
	m_bDebugMode = ((m_bDebugMode) ? false : true);
	updateDebugModeCache();
	
	gDLL->getEngineIFace()->RebuildAllPlots();

	GC.getMapINLINE().setupGraphical();
	GC.getMapINLINE().updateVisibility();
	GC.getMapINLINE().updateSymbols();
	GC.getMapINLINE().updateFlagSymbols();
	GC.getMapINLINE().updateMinimapColor();

	gDLL->getInterfaceIFace()->setDirty(GameData_DIRTY_BIT, true);
	gDLL->getInterfaceIFace()->setDirty(Score_DIRTY_BIT, true);
	gDLL->getInterfaceIFace()->setDirty(MinimapSection_DIRTY_BIT, true);
	gDLL->getInterfaceIFace()->setDirty(UnitInfo_DIRTY_BIT, true);
	gDLL->getInterfaceIFace()->setDirty(CityInfo_DIRTY_BIT, true);
	gDLL->getInterfaceIFace()->setDirty(GlobeLayer_DIRTY_BIT, true);

	//gDLL->getEngineIFace()->SetDirty(GlobeTexture_DIRTY_BIT, true);
	gDLL->getEngineIFace()->SetDirty(MinimapTexture_DIRTY_BIT, true);
	gDLL->getEngineIFace()->SetDirty(CultureBorders_DIRTY_BIT, true);

	if (m_bDebugMode)
	{
		gDLL->getEngineIFace()->PushFogOfWar(FOGOFWARMODE_OFF);
	}
	else
	{
		gDLL->getEngineIFace()->PopFogOfWar();
	}
	gDLL->getEngineIFace()->setFogOfWarFromStack();
}

void CvGame::noteGraphicRebuildNeeded()
{
	m_lastGraphicUpdateRequestTickCount = GetTickCount();
}

void CvGame::updateDebugModeCache()
{
	if ((gDLL->getChtLvl() > 0) || (gDLL->GetWorldBuilderMode()))
	{
		m_bDebugModeCache = m_bDebugMode;
	}
	else
	{
		m_bDebugModeCache = false;
	}
}

int CvGame::getPitbossTurnTime() const
{
	return GC.getInitCore().getPitbossTurnTime();
}

void CvGame::setPitbossTurnTime(int iHours)
{
	GC.getInitCore().setPitbossTurnTime(iHours);
}


bool CvGame::isHotSeat() const
{
	return (GC.getInitCore().getHotseat());
}

bool CvGame::isPbem() const
{
	return (GC.getInitCore().getPbem());
}



bool CvGame::isPitboss() const
{
	return (GC.getInitCore().getPitboss());
}

bool CvGame::isSimultaneousTeamTurns() const
{
	if (!isNetworkMultiPlayer())
	{
		return false;
	}

	if (isMPOption(MPOPTION_SIMULTANEOUS_TURNS))
	{
		return false;
	}
	
	return true;
}

bool CvGame::isFinalInitialized() const
{
	return m_bFinalInitialized;
}


void CvGame::setFinalInitialized(bool bNewValue)
{
	PROFILE_FUNC();

	int iI;

	if (isFinalInitialized() != bNewValue)
	{
		m_bFinalInitialized = bNewValue;

		if (isFinalInitialized())
		{
			updatePlotGroups();

			GC.getMapINLINE().updateIrrigated();

			for (iI = 0; iI < MAX_TEAMS; iI++)
			{
				if (GET_TEAM((TeamTypes)iI).isAlive())
				{
					GET_TEAM((TeamTypes)iI).AI_updateAreaStragies();
				}
			}
		}
	}
}


bool CvGame::getPbemTurnSent() const
{
	return m_bPbemTurnSent;
}


void CvGame::setPbemTurnSent(bool bNewValue)
{
	m_bPbemTurnSent = bNewValue;
}


bool CvGame::getHotPbemBetweenTurns() const
{
	return m_bHotPbemBetweenTurns;
}


void CvGame::setHotPbemBetweenTurns(bool bNewValue)
{
	m_bHotPbemBetweenTurns = bNewValue;
}


bool CvGame::isPlayerOptionsSent() const
{
	return m_bPlayerOptionsSent;
}


void CvGame::sendPlayerOptions(bool bForce)
{
	int iI;

	if (getActivePlayer() == NO_PLAYER)
	{
		return;
	}

	if (!isPlayerOptionsSent() || bForce)
	{
		m_bPlayerOptionsSent = true;

		for (iI = 0; iI < NUM_PLAYEROPTION_TYPES; iI++)
		{
			gDLL->sendPlayerOption(((PlayerOptionTypes)iI), gDLL->getPlayerOption((PlayerOptionTypes)iI));
		}

		for (int i=0; i<MAX_PLAYERS; i++)
		{
			SAFE_DELETE(GET_PLAYER((PlayerTypes)i).m_pBuildLists);
			GET_PLAYER((PlayerTypes)i).m_pBuildLists = new CvBuildLists();
		}
		GET_PLAYER(GC.getGameINLINE().getActivePlayer()).m_pBuildLists->readFromFile();
		CvMessageControl::getInstance().sendBuildLists();
	}
}


PlayerTypes CvGame::getActivePlayer() const
{
	return GC.getInitCore().getActivePlayer();
}


void CvGame::setActivePlayer(PlayerTypes eNewValue, bool bForceHotSeat)
{
	PlayerTypes eOldActivePlayer = getActivePlayer();
	if (eOldActivePlayer != eNewValue)
	{
		int iActiveNetId = ((NO_PLAYER != eOldActivePlayer) ? GET_PLAYER(eOldActivePlayer).getNetID() : -1);
		GC.getInitCore().setActivePlayer(eNewValue);

	//if (GET_PLAYER(eNewValue).isHuman() && (isHotSeat() || isPbem() || bForceHotSeat))
		if (eNewValue != NO_PLAYER && GET_PLAYER(eNewValue).isHuman() && (isHotSeat() || isPbem() || bForceHotSeat)) // K-Mod
		{
			gDLL->getPassword(eNewValue);
			setHotPbemBetweenTurns(false);
			gDLL->getInterfaceIFace()->dirtyTurnLog(eNewValue);

			if (NO_PLAYER != eOldActivePlayer)
			{
				int iInactiveNetId = GET_PLAYER(eNewValue).getNetID();
				GET_PLAYER(eNewValue).setNetID(iActiveNetId);
				GET_PLAYER(eOldActivePlayer).setNetID(iInactiveNetId);
			}

			GET_PLAYER(eNewValue).showMissedMessages();

			if (countHumanPlayersAlive() == 1 && isPbem())
			{
				// Nobody else left alive
				GC.getInitCore().setType(GAME_HOTSEAT_NEW);
			}

			if (isHotSeat() || bForceHotSeat)
			{
				sendPlayerOptions(true);
			}
		}

		if (GC.IsGraphicsInitialized())
		{
			GC.getMapINLINE().updateFog();
			GC.getMapINLINE().updateVisibility();
			GC.getMapINLINE().updateSymbols();
			GC.getMapINLINE().updateMinimapColor();

			updateUnitEnemyGlow();

			gDLL->getInterfaceIFace()->setEndTurnMessage(false);

			gDLL->getInterfaceIFace()->clearSelectedCities();
			gDLL->getInterfaceIFace()->clearSelectionList();

			gDLL->getInterfaceIFace()->setDirty(PercentButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(ResearchButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(GameData_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(MinimapSection_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(CityInfo_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(UnitInfo_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(Flag_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(GlobeLayer_DIRTY_BIT, true);

			gDLL->getEngineIFace()->SetDirty(CultureBorders_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(BlockadedPlots_DIRTY_BIT, true);
		}
	}
}

void CvGame::updateUnitEnemyGlow()
{
	PROFILE_FUNC();

	//update unit enemy glow
	for(int i=0;i<MAX_PLAYERS;i++)
	{
		PlayerTypes playerType = (PlayerTypes) i;
		int iLoop = 0;
		for(CvUnit *pLoopUnit = GET_PLAYER(playerType).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(playerType).nextUnit(&iLoop))
		{
			//update glow
			if ( !pLoopUnit->isUsingDummyEntities() )
			{
				gDLL->getEntityIFace()->updateEnemyGlow(pLoopUnit->getUnitEntity());
			}
		}
	}
}

HandicapTypes CvGame::getHandicapType() const
{
	return m_eHandicap;
}

void CvGame::setHandicapType(HandicapTypes eHandicap)
{
	m_eHandicap = eHandicap;
}

PlayerTypes CvGame::getPausePlayer() const
{
	return m_ePausePlayer;
}


bool CvGame::isPaused() const
{
	return (getPausePlayer() != NO_PLAYER);
}


void CvGame::setPausePlayer(PlayerTypes eNewValue)
{
	m_ePausePlayer = eNewValue;
}


UnitTypes CvGame::getBestLandUnit() const
{
	if ( m_eBestLandUnit == NO_UNIT )
	{
		//	This can occur after a load in which the previous best land unit was deleted from the assets
		//	Just recalculate on demand
		for( int iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if ( GET_PLAYER((PlayerTypes)iI).isAlive() )
			{
				int iLoop = 0;
				for(CvUnit* pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
				{
					if (pLoopUnit->baseCombatStr() > 0)
					{
						if (m_eBestLandUnit == NO_UNIT || (pLoopUnit->baseCombatStr() > GC.getGameINLINE().getBestLandUnitCombat()))
						{
							m_eBestLandUnit = pLoopUnit->getUnitType();
						}
					}
				}
			}
		}
	}

	return m_eBestLandUnit;
}


int CvGame::getBestLandUnitCombat() const
{
	if (getBestLandUnit() == NO_UNIT)
	{
		return 1;
	}

	return std::max(1, GC.getUnitInfo(getBestLandUnit()).getCombat());
}


void CvGame::setBestLandUnit(UnitTypes eNewValue)
{
	if (getBestLandUnit() != eNewValue)
	{
		m_eBestLandUnit = eNewValue;

		gDLL->getInterfaceIFace()->setDirty(UnitInfo_DIRTY_BIT, true);
	}
}


TeamTypes CvGame::getWinner() const
{
	return m_eWinner;
}


VictoryTypes CvGame::getVictory() const
{
	return m_eVictory;
}


void CvGame::setWinner(TeamTypes eNewWinner, VictoryTypes eNewVictory)
{
	CvWString szBuffer;

	if ((getWinner() != eNewWinner) || (getVictory() != eNewVictory))
	{
		m_eWinner = eNewWinner;
		m_eVictory = eNewVictory;

/************************************************************************************************/
/* AI_AUTO_PLAY_MOD                        07/09/08                                jdog5000      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		CvEventReporter::getInstance().victory(eNewWinner, eNewVictory);
/************************************************************************************************/
/* AI_AUTO_PLAY_MOD                        END                                                  */
/************************************************************************************************/

		if (getVictory() != NO_VICTORY)
		{
			if (getWinner() != NO_TEAM)
			{
				szBuffer = gDLL->getText("TXT_KEY_GAME_WON", GET_TEAM(getWinner()).getName().GetCString(), GC.getVictoryInfo(getVictory()).getTextKeyWide());
				addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, GET_TEAM(getWinner()).getLeaderID(), szBuffer, -1, -1, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
			}
/************************************************************************************************/
/* REVOLUTION_MOD                                                                 lemmy101      */
/*                                                                                jdog5000      */
/*                                                                                              */
/************************************************************************************************/
			if ((getAIAutoPlay(getActivePlayer()) > 0) || gDLL->GetAutorun())
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/
			{
				setGameState(GAMESTATE_EXTENDED);
			}
			else
			{
				setGameState(GAMESTATE_OVER);
			}
		}

		gDLL->getInterfaceIFace()->setDirty(Center_DIRTY_BIT, true);

/************************************************************************************************/
/* AI_AUTO_PLAY_MOD                        07/09/08                                jdog5000      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/* original code
		CvEventReporter::getInstance().victory(eNewWinner, eNewVictory);
*/
/************************************************************************************************/
/* AI_AUTO_PLAY_MOD                        END                                                  */
/************************************************************************************************/

		gDLL->getInterfaceIFace()->setDirty(Soundtrack_DIRTY_BIT, true);
	}
}


GameStateTypes CvGame::getGameState() const
{
	return m_eGameState;
}


void CvGame::setGameState(GameStateTypes eNewValue)
{
	CvPopupInfo* pInfo;
	int iI;

	if (getGameState() != eNewValue)
	{
		m_eGameState = eNewValue;

		if (eNewValue == GAMESTATE_OVER)
		{
			CvEventReporter::getInstance().gameEnd();

// BUG - AutoSave - start
			PYTHON_CALL_FUNCTION2(__FUNCTION__, PYBugModule, "gameEndSave");
// BUG - AutoSave - end

			showEndGameSequence();

			for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).isHuman())
				{
					// One more turn?
					pInfo = new CvPopupInfo(BUTTONPOPUP_EXTENDED_GAME);
					if (NULL != pInfo)
					{
						GET_PLAYER((PlayerTypes)iI).addPopup(pInfo);
					}
				}
			}
		}

		gDLL->getInterfaceIFace()->setDirty(Cursor_DIRTY_BIT, true);
	}
}


GameSpeedTypes CvGame::getGameSpeedType() const
{
	return GC.getInitCore().getGameSpeed();
}


EraTypes CvGame::getStartEra() const
{
	return GC.getInitCore().getEra();
}


CalendarTypes CvGame::getCalendar() const
{
	return GC.getInitCore().getCalendar();
}


PlayerTypes CvGame::getRankPlayer(int iRank) const																
{
	FAssertMsg(iRank >= 0, "iRank is expected to be non-negative (invalid Rank)");
	FAssertMsg(iRank < MAX_PLAYERS, "iRank is expected to be within maximum bounds (invalid Rank)");
	return (PlayerTypes)m_aiRankPlayer[iRank];
}


void CvGame::setRankPlayer(int iRank, PlayerTypes ePlayer)													
{
	FAssertMsg(iRank >= 0, "iRank is expected to be non-negative (invalid Rank)");
	FAssertMsg(iRank < MAX_PLAYERS, "iRank is expected to be within maximum bounds (invalid Rank)");

	if (getRankPlayer(iRank) != ePlayer)
	{
		m_aiRankPlayer[iRank] = ePlayer;

		gDLL->getInterfaceIFace()->setDirty(Score_DIRTY_BIT, true);
	}
}


int CvGame::getPlayerRank(PlayerTypes ePlayer) const														 
{
	FAssertMsg(ePlayer >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(ePlayer < MAX_PLAYERS, "ePlayer is expected to be within maximum bounds (invalid Index)");
	return m_aiPlayerRank[ePlayer];
}
 

void CvGame::setPlayerRank(PlayerTypes ePlayer, int iRank)													
{
	FAssertMsg(ePlayer >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(ePlayer < MAX_PLAYERS, "ePlayer is expected to be within maximum bounds (invalid Index)");
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      09/03/09                       poyuzhe & jdog5000     */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
	// From Sanguo Mod Performance, ie the CAR Mod
	// Attitude cache
	if (iRank != m_aiPlayerRank[ePlayer])
	{
		for (int iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				GET_PLAYER(ePlayer).AI_invalidateAttitudeCache((PlayerTypes)iI);
				GET_PLAYER((PlayerTypes)iI).AI_invalidateAttitudeCache(ePlayer);
			}
		}
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
	m_aiPlayerRank[ePlayer] = iRank;
	FAssert(getPlayerRank(ePlayer) >= 0);
}


int CvGame::getPlayerScore(PlayerTypes ePlayer)	const																
{
	FAssertMsg(ePlayer >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(ePlayer < MAX_PLAYERS, "ePlayer is expected to be within maximum bounds (invalid Index)");
	return m_aiPlayerScore[ePlayer];
}


void CvGame::setPlayerScore(PlayerTypes ePlayer, int iScore)													
{
	FAssertMsg(ePlayer >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(ePlayer < MAX_PLAYERS, "ePlayer is expected to be within maximum bounds (invalid Index)");

	if (getPlayerScore(ePlayer) != iScore)
	{
		m_aiPlayerScore[ePlayer] = iScore;
		FAssert(getPlayerScore(ePlayer) >= 0);

		gDLL->getInterfaceIFace()->setDirty(Score_DIRTY_BIT, true);
	}
}


TeamTypes CvGame::getRankTeam(int iRank) const																	
{
	FAssertMsg(iRank >= 0, "iRank is expected to be non-negative (invalid Rank)");
	FAssertMsg(iRank < MAX_TEAMS, "iRank is expected to be within maximum bounds (invalid Index)");
	return (TeamTypes)m_aiRankTeam[iRank];
}


void CvGame::setRankTeam(int iRank, TeamTypes eTeam)														
{
	FAssertMsg(iRank >= 0, "iRank is expected to be non-negative (invalid Rank)");
	FAssertMsg(iRank < MAX_TEAMS, "iRank is expected to be within maximum bounds (invalid Index)");

	if (getRankTeam(iRank) != eTeam)
	{
		m_aiRankTeam[iRank] = eTeam;

		gDLL->getInterfaceIFace()->setDirty(Score_DIRTY_BIT, true);
	}
}


int CvGame::getTeamRank(TeamTypes eTeam) const																	
{
	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");
	return m_aiTeamRank[eTeam];
}


void CvGame::setTeamRank(TeamTypes eTeam, int iRank)														
{
	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");
	m_aiTeamRank[eTeam] = iRank;
	FAssert(getTeamRank(eTeam) >= 0);
}


int CvGame::getTeamScore(TeamTypes eTeam) const																	
{
	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");
	return m_aiTeamScore[eTeam];
}


void CvGame::setTeamScore(TeamTypes eTeam, int iScore)		
{
	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");
	m_aiTeamScore[eTeam] = iScore;
	FAssert(getTeamScore(eTeam) >= 0);
}


bool CvGame::isOption(GameOptionTypes eIndex) const
{
/*************************************************************************************************/
/**	Xienwolf Tweak							02/06/09											**/
/**																								**/
/**						Rigid assurance that we never have bad options							**/
/*************************************************************************************************/
	if (eIndex >= NUM_GAMEOPTION_TYPES || eIndex < 0)
	{
		return false;
	}
/*************************************************************************************************/
/**	Tweak									END													**/
/*************************************************************************************************/
	//Afforess BarbarianCiv does not work in MP!
	//TODO: fix barbarian civ
	//45deg: testing if it works, OOS might have been caused by something else!
/*	if (eIndex == GAMEOPTION_NO_BARBARIAN_CIV && (isNetworkMultiPlayer() || isPitboss()))
	{
		return true;
	}*/
	//Afforess hidden options are always off
	if (!GC.getGameOptionInfo(eIndex).getVisible())
	{
		return false;
	}
	return GC.getInitCore().getOption(eIndex);
}


void CvGame::setOption(GameOptionTypes eIndex, bool bEnabled)
{
	GC.getInitCore().setOption(eIndex, bEnabled);
}


bool CvGame::isMPOption(MultiplayerOptionTypes eIndex) const
{
	return GC.getInitCore().getMPOption(eIndex);
}


void CvGame::setMPOption(MultiplayerOptionTypes eIndex, bool bEnabled)
{
	GC.getInitCore().setMPOption(eIndex, bEnabled);
}


bool CvGame::isForcedControl(ForceControlTypes eIndex) const
{
	return GC.getInitCore().getForceControl(eIndex);
}


void CvGame::setForceControl(ForceControlTypes eIndex, bool bEnabled)
{
	GC.getInitCore().setForceControl(eIndex, bEnabled);
}


int CvGame::getUnitCreatedCount(UnitTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumUnitInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiUnitCreatedCount[eIndex];
}


void CvGame::incrementUnitCreatedCount(UnitTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumUnitInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiUnitCreatedCount[eIndex]++;
}


int CvGame::getUnitClassCreatedCount(UnitClassTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumUnitClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiUnitClassCreatedCount[eIndex];
}


bool CvGame::isUnitClassMaxedOut(UnitClassTypes eIndex, int iExtra)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumUnitClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (!isWorldUnitClass(eIndex))
	{
		return false;
	}

	FAssertMsg(getUnitClassCreatedCount(eIndex) <= GC.getUnitClassInfo(eIndex).getMaxGlobalInstances(), "Index is expected to be within maximum bounds (invalid Index)");

	return ((getUnitClassCreatedCount(eIndex) + iExtra) >= GC.getUnitClassInfo(eIndex).getMaxGlobalInstances());
}


void CvGame::incrementUnitClassCreatedCount(UnitClassTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumUnitClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiUnitClassCreatedCount[eIndex]++;
}


int CvGame::getBuildingClassCreatedCount(BuildingClassTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBuildingClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiBuildingClassCreatedCount[eIndex];
}


bool CvGame::isBuildingClassMaxedOut(BuildingClassTypes eIndex, int iExtra)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBuildingClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (!isWorldWonderClass(eIndex))
	{
		return false;
	}

	return ((getBuildingClassCreatedCount(eIndex) + iExtra) >= GC.getBuildingClassInfo(eIndex).getMaxGlobalInstances());
}


void CvGame::incrementBuildingClassCreatedCount(BuildingClassTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBuildingClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiBuildingClassCreatedCount[eIndex]++;
}


int CvGame::getProjectCreatedCount(ProjectTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumProjectInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiProjectCreatedCount[eIndex];
}


bool CvGame::isProjectMaxedOut(ProjectTypes eIndex, int iExtra)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumProjectInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (!isWorldProject(eIndex))
	{
		return false;
	}

	FAssertMsg(getProjectCreatedCount(eIndex) <= GC.getProjectInfo(eIndex).getMaxGlobalInstances(), "Index is expected to be within maximum bounds (invalid Index)");

	return ((getProjectCreatedCount(eIndex) + iExtra) >= GC.getProjectInfo(eIndex).getMaxGlobalInstances());
}


void CvGame::incrementProjectCreatedCount(ProjectTypes eIndex, int iExtra)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumProjectInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	int iOldValue;

	//	Do this in a thread-safe manner
	do
	{
		iOldValue = m_paiProjectCreatedCount[eIndex];
	} while( InterlockedCompareExchange((volatile LONG*)&m_paiProjectCreatedCount[eIndex], iOldValue + iExtra, iOldValue) != iOldValue );
	//m_paiProjectCreatedCount[eIndex] += iExtra;
}


int CvGame::getForceCivicCount(CivicTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumCivicInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiForceCivicCount[eIndex];
}


bool CvGame::isForceCivic(CivicTypes eIndex) const
{
	return (getForceCivicCount(eIndex) > 0);
}


bool CvGame::isForceCivicOption(CivicOptionTypes eCivicOption) const
{
	int iI;

	for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
	{
		if (GC.getCivicInfo((CivicTypes)iI).getCivicOptionType() == eCivicOption)
		{
			if (isForceCivic((CivicTypes)iI))
			{
				return true;
			}
		}
	}

	return false;
}


void CvGame::changeForceCivicCount(CivicTypes eIndex, int iChange)
{
	bool bOldForceCivic;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumCivicInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		bOldForceCivic = isForceCivic(eIndex);

		m_paiForceCivicCount[eIndex] += iChange;
		FAssert(getForceCivicCount(eIndex) >= 0);

		if (bOldForceCivic != isForceCivic(eIndex))
		{
			verifyCivics();
		}
	}
}


PlayerVoteTypes CvGame::getVoteOutcome(VoteTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumVoteInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiVoteOutcome[eIndex];
}


bool CvGame::isVotePassed(VoteTypes eIndex) const
{
	PlayerVoteTypes ePlayerVote = getVoteOutcome(eIndex);

	if (isTeamVote(eIndex))
	{
		return (ePlayerVote >= 0 && ePlayerVote < MAX_CIV_TEAMS);
	}
	else
	{
		return (ePlayerVote == PLAYER_VOTE_YES);
	}
}


void CvGame::setVoteOutcome(const VoteTriggeredData& kData, PlayerVoteTypes eNewValue)
{
	bool bOldPassed;

	VoteTypes eIndex = kData.kVoteOption.eVote;
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumVoteInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (getVoteOutcome(eIndex) != eNewValue)
	{
		bOldPassed = isVotePassed(eIndex);

		m_paiVoteOutcome[eIndex] = eNewValue;

		if (bOldPassed != isVotePassed(eIndex))
		{
			processVote(kData, ((isVotePassed(eIndex)) ? 1 : -1));
		}
	}

	for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iPlayer);
		if (kPlayer.isAlive())
		{
			kPlayer.setVote(kData.getID(), NO_PLAYER_VOTE);
		}
	}
}


int CvGame::getReligionGameTurnFounded(ReligionTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumReligionInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiReligionGameTurnFounded[eIndex];
}


bool CvGame::isReligionFounded(ReligionTypes eIndex)
{
	return (getReligionGameTurnFounded(eIndex) != -1);
}


void CvGame::makeReligionFounded(ReligionTypes eIndex, PlayerTypes ePlayer)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumReligionInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (!isReligionFounded(eIndex))
	{
		FAssertMsg(getGameTurn() != -1, "getGameTurn() is not expected to be equal with -1");
		m_paiReligionGameTurnFounded[eIndex] = getGameTurn();

		GC.getGameINLINE().reportEvent(PLAYER_EVENT_RELIGION_FOUNDED, ePlayer, eIndex, -1);
/************************************************************************************************/
/* Afforess                                     12/7/09                                         */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		// Sanguo Mod Performance start, added by poyuzhe 07.26.09
		for (int iI = 0; iI < GC.getMAX_PLAYERS(); iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive() && GET_PLAYER((PlayerTypes)iI).getStateReligion() == eIndex)
			{
				GET_PLAYER(ePlayer).AI_invalidateAttitudeCache((PlayerTypes)iI);
				GET_PLAYER((PlayerTypes)iI).AI_invalidateAttitudeCache(ePlayer);
			}
		}
		// Sanguo Mod Performance, end
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	
	}
}

bool CvGame::isReligionSlotTaken(ReligionTypes eReligion) const
{
	FAssertMsg(eReligion >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eReligion < GC.getNumReligionInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_abReligionSlotTaken[eReligion];
}

void CvGame::setReligionSlotTaken(ReligionTypes eReligion, bool bTaken)
{
	FAssertMsg(eReligion >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eReligion < GC.getNumReligionInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_abReligionSlotTaken[eReligion] = bTaken;
}


/************************************************************************************************/
/* RevDCM	                  Start		 4/29/10                                                */
/*                                                                                              */
/* OC_LIMITED_RELIGIONS                                                                         */
/************************************************************************************************/
bool CvGame::isTechCanFoundReligion(TechTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTechInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	return m_abTechCanFoundReligion[eIndex];
}

void CvGame::setTechCanFoundReligion(TechTypes eIndex, bool bUsed)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTechInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_abTechCanFoundReligion[eIndex] = !bUsed;
}

bool CvGame::isGameStart()
{
	return m_iElapsedGameTurns < 10;
}

int CvGame::countNumReligionsFounded()
{
	int iCount = 0;

	for (int iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		if( isReligionFounded((ReligionTypes)iI) )
		{
			iCount++;
		}
	}

	return iCount;
}

int CvGame::countNumReligionTechsDiscovered()
{
	int iCount = 0;

	for (int iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		TechTypes eTech = TechTypes(GC.getReligionInfo((ReligionTypes)iI).getTechPrereq());
		{
			if( countKnownTechNumTeams(eTech) > 0)
			{
				iCount++;
			}
		}
	}

	return iCount;
}
/************************************************************************************************/
/* LIMITED_RELIGIONS               END                                                          */
/************************************************************************************************/


int CvGame::getCorporationGameTurnFounded(CorporationTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumCorporationInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiCorporationGameTurnFounded[eIndex];
}


bool CvGame::isCorporationFounded(CorporationTypes eIndex)
{
	return (getCorporationGameTurnFounded(eIndex) != -1);
}


void CvGame::makeCorporationFounded(CorporationTypes eIndex, PlayerTypes ePlayer)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumCorporationInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (!isCorporationFounded(eIndex))
	{
		FAssertMsg(getGameTurn() != -1, "getGameTurn() is not expected to be equal with -1");
		m_paiCorporationGameTurnFounded[eIndex] = getGameTurn();

		GC.getGameINLINE().reportEvent(PLAYER_EVENT_CORP_FOUNDED, ePlayer, eIndex, -1);
	}
}

bool CvGame::isVictoryValid(VictoryTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumVictoryInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return GC.getInitCore().getVictory(eIndex);
}

void CvGame::setVictoryValid(VictoryTypes eIndex, bool bValid)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumVictoryInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	GC.getInitCore().setVictory(eIndex, bValid);
}


bool CvGame::isSpecialUnitValid(SpecialUnitTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumSpecialUnitInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_pabSpecialUnitValid[eIndex];
}


void CvGame::makeSpecialUnitValid(SpecialUnitTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumSpecialUnitInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_pabSpecialUnitValid[eIndex] = true;
}


bool CvGame::isSpecialBuildingValid(SpecialBuildingTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumSpecialBuildingInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_pabSpecialBuildingValid[eIndex];
}


void CvGame::makeSpecialBuildingValid(SpecialBuildingTypes eIndex, bool bAnnounce)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumSpecialBuildingInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (!m_pabSpecialBuildingValid[eIndex])
	{
		m_pabSpecialBuildingValid[eIndex] = true;


		if (bAnnounce)
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_SPECIAL_BUILDING_VALID", GC.getSpecialBuildingInfo(eIndex).getTextKeyWide());

			for (int iI = 0; iI < MAX_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).isAlive())
				{
					MEMORY_TRACK_EXEMPT();

					AddDLLMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PROJECT_COMPLETED", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
				}
			}
		}
	}
}


//TB Nukefix
//bool CvGame::isNukesValid() const
//{
//	return m_bNukesValid;
//}


//void CvGame::makeNukesValid(bool bValid)
//{
//	m_bNukesValid = bValid;
//}

bool CvGame::isInAdvancedStart() const
{
	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
	{
		if ((GET_PLAYER((PlayerTypes)iPlayer).getAdvancedStartPoints() >= 0) && GET_PLAYER((PlayerTypes)iPlayer).isHuman())
		{
			return true;
		}
	}

	return false;
}

void CvGame::setVoteChosen(int iSelection, int iVoteId)
{
	VoteSelectionData* pVoteSelectionData = getVoteSelection(iVoteId);
	if (NULL != pVoteSelectionData)
	{
		addVoteTriggered(*pVoteSelectionData, iSelection);
	}

	deleteVoteSelection(iVoteId);
}


CvCity* CvGame::getHolyCity(ReligionTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumReligionInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return getCity(m_paHolyCity[eIndex]);
}


void CvGame::setHolyCity(ReligionTypes eIndex, CvCity* pNewValue, bool bAnnounce)
{
	CvWString szBuffer;
	CvCity* pOldValue;
	CvCity* pHolyCity;
	int iI;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumReligionInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	pOldValue = getHolyCity(eIndex);

	if (pOldValue != pNewValue)
	{
		  // religion visibility now part of espionage
		//updateCitySight(false, true);

		if (pNewValue != NULL)
		{
			m_paHolyCity[eIndex] = pNewValue->getIDInfo();
		}
		else
		{
			m_paHolyCity[eIndex].reset();
		}

		// religion visibility now part of espionage
		//updateCitySight(true, true);

		if (pOldValue != NULL)
		{
			pOldValue->changeReligionInfluence(eIndex, -(GC.getDefineINT("HOLY_CITY_INFLUENCE")));

			pOldValue->updateReligionCommerce();

			pOldValue->setInfoDirty(true);
		}

		if (getHolyCity(eIndex) != NULL)
		{
			pHolyCity = getHolyCity(eIndex);

			pHolyCity->setHasReligion(eIndex, true, bAnnounce, true);
			pHolyCity->changeReligionInfluence(eIndex, GC.getDefineINT("HOLY_CITY_INFLUENCE"));

			pHolyCity->updateReligionCommerce();

			pHolyCity->setInfoDirty(true);

			if (bAnnounce)
			{
				if (isFinalInitialized() && !(gDLL->GetWorldBuilderMode()))
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_REL_FOUNDED", GC.getReligionInfo(eIndex).getTextKeyWide(), pHolyCity->getNameKey());
					addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, pHolyCity->getOwnerINLINE(), szBuffer, pHolyCity->getX_INLINE(), pHolyCity->getY_INLINE(), (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));

					for (iI = 0; iI < MAX_PLAYERS; iI++)
					{
						if (GET_PLAYER((PlayerTypes)iI).isAlive())
						{
							MEMORY_TRACK_EXEMPT();

							if (pHolyCity->isRevealed(GET_PLAYER((PlayerTypes)iI).getTeam(), false))
							{
								szBuffer = gDLL->getText("TXT_KEY_MISC_REL_FOUNDED", GC.getReligionInfo(eIndex).getTextKeyWide(), pHolyCity->getNameKey());
								AddDLLMessage(((PlayerTypes)iI), false, GC.getDefineINT("EVENT_MESSAGE_TIME_LONG"), szBuffer, GC.getReligionInfo(eIndex).getSound(), MESSAGE_TYPE_MAJOR_EVENT, GC.getReligionInfo(eIndex).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"), pHolyCity->getX_INLINE(), pHolyCity->getY_INLINE());
							}
							else
							{
								szBuffer = gDLL->getText("TXT_KEY_MISC_REL_FOUNDED_UNKNOWN", GC.getReligionInfo(eIndex).getTextKeyWide());
								AddDLLMessage(((PlayerTypes)iI), false, GC.getDefineINT("EVENT_MESSAGE_TIME_LONG"), szBuffer, GC.getReligionInfo(eIndex).getSound(), MESSAGE_TYPE_MAJOR_EVENT, GC.getReligionInfo(eIndex).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
							}
						}
					}
				}
			}
		}

		AI_makeAssignWorkDirty();

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      09/26/09                       poyuzhe & jdog5000     */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
		// From Sanguo Mod Performance, ie the CAR Mod
		// Attitude cache
		if (GC.getGameINLINE().isFinalInitialized())
		{
			for (int iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
			{
				if (GET_PLAYER((PlayerTypes)iJ).isAlive() && GET_PLAYER((PlayerTypes)iJ).getStateReligion() == eIndex)
				{
					if( pNewValue != NULL )
					{
						GET_PLAYER(pNewValue->getOwnerINLINE()).AI_invalidateAttitudeCache((PlayerTypes)iJ);
						GET_PLAYER((PlayerTypes)iJ).AI_invalidateAttitudeCache(pNewValue->getOwnerINLINE());
					}
					
					if( pOldValue != NULL )
					{
						GET_PLAYER(pOldValue->getOwnerINLINE()).AI_invalidateAttitudeCache((PlayerTypes)iJ);
						GET_PLAYER((PlayerTypes)iJ).AI_invalidateAttitudeCache(pOldValue->getOwnerINLINE());
					}
				}
			}
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
	}
}


CvCity* CvGame::getHeadquarters(CorporationTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumCorporationInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return getCity(m_paHeadquarters[eIndex]);
}


void CvGame::setHeadquarters(CorporationTypes eIndex, CvCity* pNewValue, bool bAnnounce)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumCorporationInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	CvCity* pOldValue = getHeadquarters(eIndex);

	if (pOldValue != pNewValue)
	{
		if (pNewValue != NULL)
		{
			m_paHeadquarters[eIndex] = pNewValue->getIDInfo();
		}
		else
		{
			m_paHeadquarters[eIndex].reset();
		}

		if (pOldValue != NULL)
		{
			pOldValue->updateCorporation();

			pOldValue->setInfoDirty(true);
		}

		CvCity* pHeadquarters = getHeadquarters(eIndex);

/************************************************************************************************/
/* Afforess	                  Start		 06/17/10                                               */
/*                                                                                              */
/* Set and Replace Corporation HQ's                                                             */
/************************************************************************************************/
		if (pOldValue != NULL)
		{
			for (int i = 0; i < GC.getNumBuildingInfos(); i++)
			{
				if ((CorporationTypes)GC.getBuildingInfo((BuildingTypes)i).getGlobalCorporationCommerce() == eIndex)
				{
					pOldValue->setNumRealBuilding((BuildingTypes)i, 0);
					break;
				}
			}
		}
		if (pNewValue != NULL)
		{
			for (int i = 0; i < GC.getNumBuildingInfos(); i++)
			{
				if ((CorporationTypes)GC.getBuildingInfo((BuildingTypes)i).getGlobalCorporationCommerce() == eIndex)
				{
					pNewValue->setNumRealBuilding((BuildingTypes)i, 1);
					break;
				}
			}
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

		if (NULL != pHeadquarters)
		{

			pHeadquarters->setHasCorporation(eIndex, true, bAnnounce);
			pHeadquarters->updateCorporation();
			pHeadquarters->setInfoDirty(true);

			if (bAnnounce)
			{
				if (isFinalInitialized() && !(gDLL->GetWorldBuilderMode()))
				{
					CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_CORPORATION_FOUNDED", GC.getCorporationInfo(eIndex).getTextKeyWide(), pHeadquarters->getNameKey());
					addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, pHeadquarters->getOwnerINLINE(), szBuffer, pHeadquarters->getX_INLINE(), pHeadquarters->getY_INLINE(), (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));

					for (int iI = 0; iI < MAX_PLAYERS; iI++)
					{
						if (GET_PLAYER((PlayerTypes)iI).isAlive())
						{
							MEMORY_TRACK_EXEMPT();

							if (pHeadquarters->isRevealed(GET_PLAYER((PlayerTypes)iI).getTeam(), false))
							{
								AddDLLMessage(((PlayerTypes)iI), false, GC.getDefineINT("EVENT_MESSAGE_TIME_LONG"), szBuffer, GC.getCorporationInfo(eIndex).getSound(), MESSAGE_TYPE_MAJOR_EVENT, GC.getCorporationInfo(eIndex).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"), pHeadquarters->getX_INLINE(), pHeadquarters->getY_INLINE());
							}
							else
							{
								CvWString szBuffer2 = gDLL->getText("TXT_KEY_MISC_CORPORATION_FOUNDED_UNKNOWN", GC.getCorporationInfo(eIndex).getTextKeyWide());
								AddDLLMessage(((PlayerTypes)iI), false, GC.getDefineINT("EVENT_MESSAGE_TIME_LONG"), szBuffer2, GC.getCorporationInfo(eIndex).getSound(), MESSAGE_TYPE_MAJOR_EVENT, GC.getCorporationInfo(eIndex).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
							}
						}
					}
				}
			}
		}

		AI_makeAssignWorkDirty();
	}
}


PlayerVoteTypes CvGame::getPlayerVote(PlayerTypes eOwnerIndex, int iVoteId) const
{
	FAssert(eOwnerIndex >= 0);
	FAssert(eOwnerIndex < MAX_CIV_PLAYERS);
	FAssert(NULL != getVoteTriggered(iVoteId));

	return GET_PLAYER(eOwnerIndex).getVote(iVoteId);
}


void CvGame::setPlayerVote(PlayerTypes eOwnerIndex, int iVoteId, PlayerVoteTypes eNewValue)
{
	FAssert(eOwnerIndex >= 0);
	FAssert(eOwnerIndex < MAX_CIV_PLAYERS);
	FAssert(NULL != getVoteTriggered(iVoteId));

	GET_PLAYER(eOwnerIndex).setVote(iVoteId, eNewValue);
}


void CvGame::castVote(PlayerTypes eOwnerIndex, int iVoteId, PlayerVoteTypes ePlayerVote)
{
	VoteTriggeredData* pTriggeredData = getVoteTriggered(iVoteId);
	if (NULL != pTriggeredData)
	{
		CvVoteInfo& kVote = GC.getVoteInfo(pTriggeredData->kVoteOption.eVote);
		if (kVote.isAssignCity())
		{
			FAssert(pTriggeredData->kVoteOption.ePlayer != NO_PLAYER);
			CvPlayer& kCityPlayer = GET_PLAYER(pTriggeredData->kVoteOption.ePlayer);

			if (GET_PLAYER(eOwnerIndex).getTeam() != kCityPlayer.getTeam())
			{
				switch (ePlayerVote)
				{
				case PLAYER_VOTE_YES:
					kCityPlayer.AI_changeMemoryCount(eOwnerIndex, MEMORY_VOTED_AGAINST_US, 1);
					break;
				case PLAYER_VOTE_NO:
					kCityPlayer.AI_changeMemoryCount(eOwnerIndex, MEMORY_VOTED_FOR_US, 1);
					break;
				default:
					break;
				}
			}
		}
		else if (isTeamVote(pTriggeredData->kVoteOption.eVote))
		{
			if ((PlayerVoteTypes)GET_PLAYER(eOwnerIndex).getTeam() != ePlayerVote)
			{
				for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
				{
					CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
					if (kLoopPlayer.isAlive())
					{
						if (kLoopPlayer.getTeam() != GET_PLAYER(eOwnerIndex).getTeam() && kLoopPlayer.getTeam() == (TeamTypes)ePlayerVote)
						{
							kLoopPlayer.AI_changeMemoryCount(eOwnerIndex, MEMORY_VOTED_FOR_US, 1);
						}
					}
				}
			}
		}

		setPlayerVote(eOwnerIndex, iVoteId, ePlayerVote);
	}
}


std::string CvGame::getScriptData() const
{
	return m_szScriptData;
}


void CvGame::setScriptData(std::string szNewValue)
{
	m_szScriptData = szNewValue;
}

const CvWString & CvGame::getName()
{
	return GC.getInitCore().getGameName();
}


void CvGame::setName(const TCHAR* szName)
{
	GC.getInitCore().setGameName(szName);
}


bool CvGame::isDestroyedCityName(CvWString& szName) const
{
	std::vector<CvWString>::const_iterator it;

	for (it = m_aszDestroyedCities.begin(); it != m_aszDestroyedCities.end(); ++it)
	{
		if (*it == szName)
		{
			return true;
		}
	}

	return false;
}

void CvGame::addDestroyedCityName(const CvWString& szName)
{
	m_aszDestroyedCities.push_back(szName);
}

bool CvGame::isGreatPersonBorn(CvWString& szName) const
{
	std::vector<CvWString>::const_iterator it;

	for (it = m_aszGreatPeopleBorn.begin(); it != m_aszGreatPeopleBorn.end(); ++it)
	{
		if (*it == szName)
		{
			return true;
		}
	}

	return false;
}

void CvGame::addGreatPersonBornName(const CvWString& szName)
{
	m_aszGreatPeopleBorn.push_back(szName);
}


// Protected Functions...

void CvGame::doTurn()
{
	MEMORY_TRACE_FUNCTION();
	PROFILE_BEGIN("CvGame::doTurn()");

	int aiShuffle[MAX_PLAYERS];
	int iLoopPlayer;
	int iI;

	gDLL->getEngineIFace()->AutoSave();

	// END OF TURN
	CvEventReporter::getInstance().beginGameTurn( getGameTurn() );

	doUpdateCacheOnTurn();

	updateScore();

	// solve property system
	m_PropertySolver.doTurn();

	doDeals();

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			//Afforess: BBAI logging
			GC.setTurnActiveTeam((TeamTypes)iI);
			
			GET_TEAM((TeamTypes)iI).doTurn();
		}
	}
	
	//Afforess: BBAI logging
	GC.setTurnActiveTeam(NO_TEAM);

	GC.getMapINLINE().doTurn();

	createBarbarianCities();

	createBarbarianUnits();

	doSpawns();

	doGlobalWarming();

/************************************************************************************************/
/* RevDCM	                  Start		09/08/10                                                */
/*                                                                                              */
/* OC_LIMITED_RELIGIONS                                                                         */
/************************************************************************************************/
	if(isGameStart())
	{
		doHolyCity();
	}
/************************************************************************************************/
/* REVDCM                                  END                                                  */
/************************************************************************************************/

	doHeadquarters();

	doDiploVote();

	doDarkAge();

/************************************************************************************************/
/* REVOLUTION_MOD                                                                 lemmy101      */
/*                                                                                jdog5000      */
/*                                                                                              */
/************************************************************************************************/

	CvEventReporter::getInstance().preEndGameTurn(getGameTurn());

	gDLL->getInterfaceIFace()->setEndTurnMessage(false);
	gDLL->getInterfaceIFace()->setHasMovedUnit(false);

	for(int n=0; n < MAX_PLAYERS; n++)
	{
		if(isForcedAIAutoPlay((PlayerTypes)n))
		{
			if (getForcedAIAutoPlay((PlayerTypes)n) > 0)
			{
				changeForcedAIAutoPlay((PlayerTypes)n, -1);
			}
		} else
		{
			if (getAIAutoPlay((PlayerTypes)n) > 0)
			{
				changeAIAutoPlay((PlayerTypes)n, -1);
			}
		}
	}
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/


	CvEventReporter::getInstance().endGameTurn(getGameTurn());

	incrementGameTurn();
	incrementElapsedGameTurns();

	if (isMPOption(MPOPTION_SIMULTANEOUS_TURNS))
	{
		shuffleArray(aiShuffle, MAX_PLAYERS, getSorenRand());

		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			iLoopPlayer = aiShuffle[iI];

			if (GET_PLAYER((PlayerTypes)iLoopPlayer).isAlive())
			{
				GET_PLAYER((PlayerTypes)iLoopPlayer).setTurnActive(true);
			}
		}
	}
	else if (isSimultaneousTeamTurns())
	{
		for (iI = 0; iI < MAX_TEAMS; iI++)
		{
			CvTeam& kTeam = GET_TEAM((TeamTypes)iI);
			if (kTeam.isAlive())
			{
				kTeam.setTurnActive(true);
				FAssert(getNumGameTurnActive() == kTeam.getAliveCount());
/*************************************************************************************************/
/* UNOFFICIAL_PATCH                       06/10/10                       snarko & jdog5000       */
/*                                                                                               */
/* Bugfix                                                                                        */
/*************************************************************************************************/
/* original bts code
			}

			break;
*/
				// Break only after first found alive player
				break;
			}
/*************************************************************************************************/
/* UNOFFICIAL_PATCH                         END                                                  */
/*************************************************************************************************/
		}
	}
	else
	{
		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (isPbem() && GET_PLAYER((PlayerTypes)iI).isHuman())
				{
					if (iI == getActivePlayer())
					{
						// Nobody else left alive
						GC.getInitCore().setType(GAME_HOTSEAT_NEW);
						GET_PLAYER((PlayerTypes)iI).setTurnActive(true);
					}
					else if (!getPbemTurnSent())
					{
						gDLL->sendPbemTurn((PlayerTypes)iI);
					}
				}
				else
				{
					GET_PLAYER((PlayerTypes)iI).setTurnActive(true);
					FAssert(getNumGameTurnActive() == 1);
				}

				break;
			}
		}
	}
/************************************************************************************************/
/* Afforess	                  Start		 01/12/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	doIncreasingDifficulty();
	doFlexibleDifficulty();
	doFinalFive();
	doHightoLow();
	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		setPreviousRequest((PlayerTypes)iI, false);
	}
	doFoundCorporations();
	doCultureDecay();

	CvGameSpeedInfo &kGameSpeed = GC.getGameSpeedInfo(getGameSpeedType());
	int iEstimateEndTurn = 0;
	for (iI = 0; iI < kGameSpeed.getNumTurnIncrements(); iI++)
	{
		iEstimateEndTurn += kGameSpeed.getGameTurnInfo(iI).iNumGameTurnsPerIncrement;
	}

	EraTypes eStartEra = getStartEra();
	//EraTypes eEndEra = (EraTypes)(isOption(GAMEOPTION_NO_FUTURE) ? GC.getInfoTypeForString("ERA_MODERN") : GC.getNumEraInfos() - 1);
	/*	f1rpo (era options):  [Note that GAMEOPTION_NO_FUTURE only disables the
		transhuman era, not the future era. Therefore, the above should've been
		GC.getInfoTypeForString("ERA_MODERN")+1.] */
	EraTypes eEndEra = (EraTypes)(getPenultimateEra() + 1);
	int iEraCount = /* f1rpo: bugfix: */ std::max(1,
			eEndEra - eStartEra);
	//Assuming we divide turns evenly per era
	float fExpectedEra = (float)((getElapsedGameTurns() / (float)(iEstimateEndTurn / iEraCount)) + eStartEra);

	float fAverageEra = 0;
	int iPlayers = 0;
	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			fAverageEra += (float)GET_PLAYER((PlayerTypes)iI).getCurrentEra();
			iPlayers++;
		}
	}
	fAverageEra /= iPlayers;

	logMsg("Expected Era: %.2f - Average Era: %.2f", fExpectedEra, fAverageEra);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	testVictory();

	gDLL->getEngineIFace()->SetDirty(GlobePartialTexture_DIRTY_BIT, true);
	gDLL->getEngineIFace()->DoTurn();

	PROFILE_END();

	stopProfilingDLL(true);

	//gDLL->getEngineIFace()->AutoSave();
}


void CvGame::doDeals()
{
	MEMORY_TRACE_FUNCTION();

	CvDeal* pLoopDeal;
	int iLoop;

	verifyDeals();

	for(pLoopDeal = firstDeal(&iLoop); pLoopDeal != NULL; pLoopDeal = nextDeal(&iLoop))
	{
		pLoopDeal->doTurn();
	}
}

//Enumerates all currently possible spawn plots for a spawning rule, for use in a thread, local density is not checked
void enumSpawnPlots(int iSpawnInfo, std::vector<CvPlot*>* plots)
{
	CvSpawnInfo& spawnInfo = GC.getSpawnInfo((SpawnTypes)iSpawnInfo);
	//GC.getGameINLINE().logMsg("Spawn thread start for %s", spawnInfo.getType());
	plots->clear();
	
	//	Enables in this turn year?
	int iTurnYear = GC.getGameINLINE().getTurnYear(GC.getGameINLINE().getGameTurn());
	if (!(iTurnYear >= spawnInfo.getStartDate() && iTurnYear <= spawnInfo.getEndDate()))
		return;
	
	if (spawnInfo.getTreatAsBarbarian() && GC.getGameINLINE().isOption(GAMEOPTION_NO_BARBARIANS))
	{
		return;
	}
	
	if ( spawnInfo.getRateOverride() == 0 )
	{
		return;
	}

	bool bNoTerrainFeatureBonus = (spawnInfo.getNumTerrains() == 0) && (spawnInfo.getNumFeatures() == 0) && (spawnInfo.getNumBonuses() == 0) && !spawnInfo.getPeaks();

	for (int i = 0; i < GC.getMapINLINE().numPlotsINLINE(); ++i)
	{
		CvPlot* pPlot = GC.getMapINLINE().plotByIndexINLINE(i);

		if ( pPlot->plotCount(PUF_isOtherPlayer, BARBARIAN_PLAYER) == 0 )
		{
			if ( spawnInfo.getNeutralOnly() && (pPlot->getOwnerINLINE() != NO_PLAYER && pPlot->getOwnerINLINE() != BARBARIAN_PLAYER) )
			{
				continue;
			}

			bool bValid = true;
			if (pPlot->isHills())
				bValid = spawnInfo.getHills();
			else if (pPlot->isFlatlands())
				bValid = spawnInfo.getFlatlands();

			// Check latitude
			if (bValid)
			{
				int lat;
				if (spawnInfo.getLatitudeAbs())
					lat = pPlot->getLatitude();
				else
					lat = pPlot->getLatitudeRaw();
				bValid = (lat >= spawnInfo.getMinLatitude()) && (lat <= spawnInfo.getMaxLatitude());
				
				int iLongitude = pPlot->getLongitudeMinutes() / 60;
				int iMinLongitude = spawnInfo.getMinLongitude();
				int iMaxLongitude = spawnInfo.getMaxLongitude();
				if (iMinLongitude <= iMaxLongitude)
				{
					bValid = bValid && (iLongitude >= iMinLongitude) && (iLongitude <= iMaxLongitude);
				}
				else
				{
					bValid = bValid && ((iLongitude >= iMinLongitude) || (iLongitude <= iMaxLongitude));
				}

				bValid = bValid && (!spawnInfo.getFreshWaterOnly() || pPlot->isFreshWater());
			}

			if ( bValid )
			{
				if (bNoTerrainFeatureBonus)
				{
					bValid = true;
				}
				else if (pPlot->isPeak())
				{
					bValid = spawnInfo.getPeaks();
				}
				else 
				{
					bValid = false;
					BonusTypes bonusType = pPlot->getBonusType();
					for(int k = 0; k < spawnInfo.getNumBonuses(); k++) // Check if bonus for spawning is present
					{
						if ( spawnInfo.getBonus(k) == bonusType )
						{
							bValid = true;
							break;
						}
					}

					if ( !bValid)
					{
						FeatureTypes featureType = pPlot->getFeatureType();
						TerrainTypes terrainType = pPlot->getTerrainType();
						if (featureType == NO_FEATURE)
						{
							for(int k = 0; k < spawnInfo.getNumTerrains(); k++) // Check if terrain for spawning is present
							{
								if ( spawnInfo.getTerrain(k) == terrainType )
								{
									bValid = true;
									break;
								}
							}
						}
						else
						{
							for(int k = 0; k < spawnInfo.getNumFeatures(); k++) // Check if feature for spawning is present
							{
								if ( spawnInfo.getFeature(k) == featureType )
								{
									bValid = true;
									break;
								}
							}

							if (bValid)  // need also proper terrain
							{

								bValid = spawnInfo.getNumFeatureTerrains() == 0;
								
								for(int k = 0; k < spawnInfo.getNumFeatureTerrains(); k++) // Check if terrain for spawning is present
								{
									if ( spawnInfo.getFeatureTerrain(k) == terrainType )
									{
										bValid = true;
										break;
									}
								}
							}
						}
					}
				}
			}

			if ( bValid )
			{
				if ( spawnInfo.getNotInView() )
				{
					if (pPlot->isVisibleToCivTeam())
					{
						continue;
					}
				}

				BoolExpr* pCondition = spawnInfo.getSpawnCondition();

				if (pCondition)
				{
					if (!pCondition->evaluate(pPlot->getGameObject()))
					{
						continue;
					}
				}

				// Valid spawning spot
				plots->push_back(pPlot);
			}
		}
	}
}

void enumSpawnPlotsThreaded(int iThread, std::vector<std::vector<CvPlot*> >* validPlots)
{
	PROFILE_THREAD("SpawnsThread");

	for(int j = iThread; j < GC.getNumSpawnInfos(); j += NUM_THREADS)
	{
		if ( GC.getSpawnInfo((SpawnTypes)j).getRateOverride() == 0 )
		{
			continue;
		}

		enumSpawnPlots(j, &((*validPlots)[j]));
	}
}

void CvGame::doSpawns()
{
	MEMORY_TRACE_FUNCTION();
	PROFILE_FUNC();
	const bool bUseThreads = GC.getDefineBOOL("USE_MULTIPLE_THREADS_SPAWNING");

	std::vector<std::vector<CvPlot*> > validPlots;
	validPlots.resize(GC.getNumSpawnInfos());

	//std::vector<boost::thread*> threads;
	boost::thread* threads[NUM_THREADS];

	if (bUseThreads)
	{
		for(int j = 0; j < NUM_THREADS; j++)
		{
			threads[j] = new boost::thread(boost::bind(enumSpawnPlotsThreaded, j, &validPlots));
		}
	}

	//	We need to know the number of each unit type in each area, which is not soemthign CvArea
	//	can do (nor worth implementing it there), so do a pre-pass here to build that data
	std::map< int, std::map<int,int> >	areaPopulationMap;

	for (int i = 0; i < GC.getMapINLINE().numPlotsINLINE(); ++i)
	{
		const CvPlot* pPlot = GC.getMapINLINE().plotByIndexINLINE(i);

		CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();

		while (pUnitNode != NULL)
		{
			const CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);

			// Only care about barbs
			if (pLoopUnit->getOwnerINLINE() == BARBARIAN_PLAYER)
			{
				if (areaPopulationMap[pPlot->getArea()].find(pLoopUnit->getUnitType()) != areaPopulationMap[pPlot->getArea()].end())
				{
					areaPopulationMap[pPlot->getArea()][pLoopUnit->getUnitType()]++;
				}
				else areaPopulationMap[pPlot->getArea()][pLoopUnit->getUnitType()] = 1;
			}
			pUnitNode = pPlot->nextUnitNode(pUnitNode);
		}
	}

	if (bUseThreads)
	{
		for(int j = 0; j < NUM_THREADS; j++)
		{
			threads[j]->join();
			delete threads[j];
		}
	}
	const bool bRagingBarbs = GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS);

	for (int j = 0; j < GC.getNumSpawnInfos(); j++)
	{
		const CvSpawnInfo& spawnInfo = GC.getSpawnInfo(static_cast<SpawnTypes>(j));

		if (spawnInfo.getRateOverride() == 0)
		{
			continue;
		}

		if (!bUseThreads)
		{
			enumSpawnPlots(j, &(validPlots[j]));
		}

		std::vector<CvPlot*> spawnPlots = validPlots[j];

		int iPlotNum = spawnPlots.size();
		logMsg("Spawn thread finished and joined for %s, found %d valid plots.", spawnInfo.getType(), iPlotNum);

		if (iPlotNum == 0) continue;

		std::random_shuffle(spawnPlots.begin(), spawnPlots.end());

		double adjustedSpawnRate = (double)spawnInfo.getTurnRate();
		const double fGlobalSpawnRate = (double)spawnInfo.getGlobalTurnRate() * iPlotNum / 100.0;
		if (adjustedSpawnRate <= 0)
		{
			adjustedSpawnRate = fGlobalSpawnRate;
		}
		else if (fGlobalSpawnRate > 0)
		{
			adjustedSpawnRate = 1 / (1 / adjustedSpawnRate + 1 / fGlobalSpawnRate);
		}

		if (!spawnInfo.getNoSpeedNormalization())
		{
			adjustedSpawnRate = adjustedSpawnRate * (GC.getGameSpeedInfo(getGameSpeedType()).getTrainPercent() + 666) / 300;
		}

		//	Adjust for any rate override
		adjustedSpawnRate = adjustedSpawnRate * 100 / spawnInfo.getRateOverride();

		int iMinAreaPlotsPerPlayerUnit = spawnInfo.getMinAreaPlotsPerPlayerUnit();
		int iMinAreaPlotsPerUnitType = spawnInfo.getMinAreaPlotsPerUnitType();

		//	Double for barbarians if raging
		if (bRagingBarbs && spawnInfo.getTreatAsBarbarian())
		{
			adjustedSpawnRate /= std::max(1, GC.getDefineINT("RAGING_BARBARIAN_SPAWN_RATE"));
			const int iRagingDensityModifier = std::max(1, GC.getDefineINT("RAGING_BARBARIAN_SPAWN_UNIT_DENSITY"));
			iMinAreaPlotsPerPlayerUnit /= iRagingDensityModifier;
			iMinAreaPlotsPerUnitType /= iRagingDensityModifier;
		}

		logMsg("Spawn chance per plot for %s is 1 to %d .", spawnInfo.getType(), (int)adjustedSpawnRate);

		const UnitTypes eUnit = spawnInfo.getUnitType();
		const int iMaxLocalDensity = spawnInfo.getMaxLocalDensity();
		int spawnCount = 0;

		for (std::vector<CvPlot*>::iterator it = spawnPlots.begin(); it < spawnPlots.end(); ++it)
		{
			// Safety valve - Halt all spawns if there are over a certain number of total extant barbarian units,
			//	as the game can run into fatal issues when it uses up the entire id space of units per player (8191).
			if (GET_PLAYER(BARBARIAN_PLAYER).getNumUnits() > MAX_BARB_UNITS_FOR_SPAWNING)
			{
				break;
			}
			CvPlot* pPlot = *it;
			const int iTotalAreaSize = pPlot->area()->getNumTiles();

			// Toffer - Bar spawn if player has the max unit density allowed in this area.
			//	Ignore small areas, max local density limit will be adequate for those.
			if (iTotalAreaSize > std::max(49, iMinAreaPlotsPerPlayerUnit))
			{
				const int iPlayerUnitsInArea = pPlot->area()->getUnitsPerPlayer(BARBARIAN_PLAYER);
				if (iPlayerUnitsInArea > 0 && iTotalAreaSize / iPlayerUnitsInArea <= iMinAreaPlotsPerPlayerUnit)
				{
					continue;
				}
			}
			int iLocalSpawnRate = (int)adjustedSpawnRate;

			//	Adjust spawn rate for area ownership percentage
			if (!pPlot->isWater() && pPlot->area()->getNumCities() == pPlot->area()->getCitiesPerPlayer(BARBARIAN_PLAYER))
			{
				iLocalSpawnRate *= 10; // No non-barb cities so dramatically reduce spawns since its essentially an unplayed-in area for now
			}
			else // Adjust spawn rate upward by up to tripling the normal rate in proportion with how little wilderness is left in this area
			{
				iLocalSpawnRate *= 100;
				iLocalSpawnRate /= 100 + 200 * pPlot->area()->getNumOwnedTiles() / pPlot->area()->getNumTiles();
			}

			if (getSorenRandNum(std::max(1, iLocalSpawnRate), "Unit spawn") == 0)
			{
				const int iArea = pPlot->getArea();
				// Check area unit type density not exceeded if specified
				if (iMinAreaPlotsPerUnitType > 0
				&& areaPopulationMap[iArea].find(eUnit) != areaPopulationMap[iArea].end()
				&& areaPopulationMap[iArea][eUnit] >= iTotalAreaSize / iMinAreaPlotsPerUnitType)
				{
					continue; // Total area density exceeded
				}

				// 49 plot local area, radius = 3
				int iRange = 3;
				const int TotalLocalArea = (iRange * 2 + 1) * (iRange * 2 + 1);
				int localAreaSize = 0;
				int localCount = 0;

				for (int iX = -iRange; iX < iRange; iX++)
				{
					for (int iY = -iRange; iY < iRange; iY++)
					{
						CvPlot* pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iX, iY);
						if (pLoopPlot != NULL && pLoopPlot->area() == pPlot->area())
						{
							localAreaSize++;
							localCount += pLoopPlot->plotCount(PUF_isUnitType, eUnit);
						}
					}
				}
				// Max local density limit for this specific unit type.
				if (localCount == 0 || localCount * 100 / localAreaSize < iMaxLocalDensity * 100 / TotalLocalArea)
				{
					// Spawn a new unit
					CvUnit* pUnit = (
						GET_PLAYER(BARBARIAN_PLAYER).initUnit(
							eUnit, pPlot->getX_INLINE(), pPlot->getY_INLINE(),
							(UnitAITypes)GC.getUnitInfo(eUnit).getDefaultUnitAIType(), NO_DIRECTION,
							GC.getGameINLINE().getSorenRandNum(10000, "AI Unit Birthmark 6")
						)
					);
					FAssertMsg(pUnit != NULL, "pUnit is expected to be assigned a valid unit object");
					pUnit->finishMoves();
					spawnCount++;
					areaPopulationMap[iArea][eUnit]++;

					// Spawn unit group
					for (int k = 0; k < spawnInfo.getNumSpawnGroup(); k++)
					{
						pUnit = (
							GET_PLAYER(BARBARIAN_PLAYER).initUnit(
								spawnInfo.getSpawnGroup(k), pPlot->getX_INLINE(), pPlot->getY_INLINE(),
								(UnitAITypes)GC.getUnitInfo(spawnInfo.getSpawnGroup(k)).getDefaultUnitAIType(), NO_DIRECTION,
								GC.getGameINLINE().getSorenRandNum(10000, "AI Unit Birthmark 7")
							)
						);
						FAssertMsg(pUnit != NULL, "pUnit is expected to be assigned a valid unit object");
						pUnit->finishMoves();
						spawnCount++;
						areaPopulationMap[iArea][spawnInfo.getSpawnGroup(k)]++;
					}
				}
			}
		}
		logMsg("%d units spawned for %s", spawnCount, spawnInfo.getType());
	}
}

void CvGame::doGlobalWarming()
{//GWMod Start M.A.
	MEMORY_TRACE_FUNCTION();

	int iGlobalWarmingDefense = 0;
	int iTreeHuggerDefenseBonus = GC.getDefineINT("TREEHUGGER_DEFENSE_BONUS");
	bool abTreeHugger[MAX_PLAYERS];

	for (int iI = 0; iI < MAX_PLAYERS; iI++)//GWMod Loop to look for environmentalism witten by EmperorFool
	{
		abTreeHugger[iI] = false;
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			for (int iJ = 0; iJ < GC.getNumCivicOptionInfos(); iJ++)
			{
				CivicTypes eCivic = GET_PLAYER((PlayerTypes)iI).getCivics((CivicOptionTypes)iJ);
				if (eCivic != NO_CIVIC && GC.getCivicInfo(eCivic).getExtraHealth() != 0)
				{
					abTreeHugger[iI] = true;
					break;
				}
			}
		}
	}

	for (int i = 0; i < GC.getMapINLINE().numPlotsINLINE(); ++i)
	{
		CvPlot* pPlot = GC.getMapINLINE().plotByIndexINLINE(i);
		if (pPlot->getFeatureType() != NO_FEATURE)
		{
			int iFeatureWarmingDefense = GC.getFeatureInfo(pPlot->getFeatureType()).getWarmingDefense();
			if (iFeatureWarmingDefense > 0)
			{
				PlayerTypes eCulturalOwner = pPlot->getOwner();
				if (eCulturalOwner != NO_PLAYER)
				{
					if (abTreeHugger[eCulturalOwner])
					{
						iGlobalWarmingDefense += (iFeatureWarmingDefense) * (iTreeHuggerDefenseBonus);
					}
					else
					{
						iGlobalWarmingDefense += iFeatureWarmingDefense;
					}
				}
				else
				{
					iGlobalWarmingDefense += iFeatureWarmingDefense;
				}
			}
		}
	}
	iGlobalWarmingDefense = iGlobalWarmingDefense * GC.getDefineINT("GLOBAL_WARMING_FOREST") / std::max(1, GC.getMapINLINE().getLandPlots());

	int iUnhealthWeight = GC.getDefineINT("GLOBAL_WARMING_UNHEALTH_WEIGHT");
	int iBonusWeight = GC.getDefineINT("GLOBAL_WARMING_BONUS_WEIGHT");
	int iPowerWeight = GC.getDefineINT("GLOBAL_WARMING_POWER_WEIGHT");
	int iGlobalWarmingValue = 0;
	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes) iPlayer);
		if (kPlayer.isAlive())
		{
			int iLoop;
			for (CvCity* pCity = kPlayer.firstCity(&iLoop); pCity != NULL; pCity = kPlayer.nextCity(&iLoop))
			{
				iGlobalWarmingValue -= (pCity->totalBadBuildingHealth() * iUnhealthWeight) + (pCity->getBonusBadHealth() * iBonusWeight) + (pCity->getPowerBadHealth() * iPowerWeight); //GWMod Changed to be total building bad health and to include power and bonuses M.A.
			}
		}
	}
	iGlobalWarmingValue /= GC.getMapINLINE().numPlotsINLINE();

	TerrainTypes eWarmingTerrain = ((TerrainTypes)(GC.getDefineINT("GLOBAL_WARMING_TERRAIN")));
	TerrainTypes eFrozenTerrain = ((TerrainTypes)(GC.getDefineINT("FROZEN_TERRAIN")));
	TerrainTypes eColdTerrain = ((TerrainTypes)(GC.getDefineINT("COLD_TERRAIN")));
/************************************************************************************************/
/* Afforess                                     12/7/09                                         */
/*                                                                                              */
/*        RoM Has a new terrain, which needs to be defined here                                 */
/************************************************************************************************/
	TerrainTypes eMarshTerrain = ((TerrainTypes)(GC.getDefineINT("MARSH_TERRAIN")));
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	
	TerrainTypes eTemperateTerrain = ((TerrainTypes)(GC.getDefineINT("TEMPERATE_TERRAIN")));
	TerrainTypes eDryTerrain = ((TerrainTypes)(GC.getDefineINT("DRY_TERRAIN")));
	TerrainTypes eBarrenTerrain = ((TerrainTypes)(GC.getDefineINT("BARREN_TERRAIN")));
	TerrainTypes eShallowsTerrain = ((TerrainTypes)(GC.getDefineINT("SHALLOW_WATER_TERRAIN")));

	FeatureTypes eColdFeature = ((FeatureTypes)(GC.getDefineINT("COLD_FEATURE")));
	FeatureTypes eTemperateFeature = ((FeatureTypes)(GC.getDefineINT("TEMPERATE_FEATURE")));
	FeatureTypes eWarmFeature = ((FeatureTypes)(GC.getDefineINT("WARM_FEATURE")));
	FeatureTypes eFalloutFeature = ((FeatureTypes)(GC.getDefineINT("NUKE_FEATURE")));

	//Global Warming
	for (int iI = 0; iI < iGlobalWarmingValue; iI++)
	{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                                                          LunarMongoose      */
/*                                                                                              */
/* Gamespeed scaling                                                                            */
/************************************************************************************************/
/* original bts code
		if (getSorenRandNum(100, "Global Warming") + iGlobalWarmingDefense < GC.getDefineINT("GLOBAL_WARMING_PROB"))
		{
*/
		int iOdds = GC.getDefineINT("GLOBAL_WARMING_PROB") - iGlobalWarmingDefense;
		iOdds *= 100;
		iOdds /= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getVictoryDelayPercent();

		if (getSorenRandNum(100, "Global Warming") < iOdds)
		{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
			CvPlot* pPlot = GC.getMapINLINE().syncRandPlot(RANDPLOT_NOT_CITY); // GWMod removed check for water tile M.A.

			if (pPlot != NULL)
			{
				//FeatureTypes eFeature = pPlot->getFeatureType();
				bool bChanged = false;

				if (pPlot->getFeatureType() != NO_FEATURE)
				{
					if (pPlot->getFeatureType() != GC.getDefineINT("NUKE_FEATURE"))
					{// GWMod won't remove features if underlaying terrain can melt
						if (pPlot->getFeatureType() != eColdFeature)
						{
							if ((pPlot->calculateBestNatureYield(YIELD_FOOD, NO_TEAM) > 1) && (pPlot->getFeatureType() == eTemperateFeature))
							{
								pPlot->setFeatureType(eWarmFeature);
								bChanged = true;
							}
							else if (pPlot->getTerrainType() == eColdTerrain)
							{
								pPlot->setTerrainType(eTemperateTerrain);
								bChanged = true;
							}
							else if (pPlot->getTerrainType() == eFrozenTerrain)
							{
								pPlot->setTerrainType(eColdTerrain);
								bChanged = true;
							}
/************************************************************************************************/
/* Afforess                                     12/7/09                                         */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
							else if (pPlot->getTerrainType() == eMarshTerrain)
							{
								pPlot->setTerrainType(eTemperateTerrain);
								bChanged = true;
							}
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	
							else
					{
						pPlot->setFeatureType(NO_FEATURE);
						bChanged = true;
					}
				}
						else
				{
							pPlot->setFeatureType(NO_FEATURE);
							bChanged = true;
						}
					}
				}
				else if (!pPlot->isWater())  // GWMod added check for water tile M.A.
				{// GWMod stepped terrain changes M.A.
					if (pPlot->getTerrainType() == eBarrenTerrain)
					{
						if (GC.getDefineINT("GW_MOD_ENABLED"))
						{
							if (pPlot->isCoastalLand())
							{
								if (!pPlot->isHills() && !pPlot->isPeak())
								{
									pPlot->setTerrainType(eShallowsTerrain);
									bChanged = true;
								}
							}
						}
					}
					else if (pPlot->getTerrainType() == eDryTerrain)
					{
						pPlot->setTerrainType(eBarrenTerrain);
						bChanged = true;
					}
					else if (pPlot->getTerrainType() == eTemperateTerrain)
					{
						pPlot->setTerrainType(eDryTerrain);
						bChanged = true;
					}
					else if (pPlot->getTerrainType() == eColdTerrain)
					{
						pPlot->setTerrainType(eTemperateTerrain);
						bChanged = true;
					}
/************************************************************************************************/
/* Afforess                                     12/7/09                                         */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
					else if (pPlot->getTerrainType() == eMarshTerrain)
					{
						pPlot->setTerrainType(eTemperateTerrain);
						bChanged = true;
					}
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	
					else if (pPlot->getTerrainType() == eFrozenTerrain)
					{
						pPlot->setTerrainType(eColdTerrain);
						bChanged = true;
					}
				}

				if (bChanged)
				{
					pPlot->setImprovementType(NO_IMPROVEMENT);

					CvCity* pCity = GC.getMapINLINE().findCity(pPlot->getX_INLINE(), pPlot->getY_INLINE());
					if (pCity != NULL)
					{
						if (pPlot->isVisible(pCity->getTeam(), false))
						{
							MEMORY_TRACK_EXEMPT();

							CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_GLOBAL_WARMING_NEAR_CITY", pCity->getNameKey());
							AddDLLMessage(pCity->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_GLOBALWARMING", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
						}
					}
				}
			}
		}
	}
	//Nuclear Winter
	int iNuclearWinterValue = 0;
	iNuclearWinterValue += getNukesExploded() * GC.getDefineINT("GLOBAL_WARMING_NUKE_WEIGHT") / 100;

	for (int iI = 0; iI < iNuclearWinterValue; iI++)
	{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                                                          LunarMongoose      */
/*                                                                                              */
/* Gamespeed scaling                                                                            */
/************************************************************************************************/
/* original Global Warming Mod code
		if (getSorenRandNum(100, "Nuclear Fallout") + iGlobalWarmingDefense < GC.getDefineINT("NUCLEAR_WINTER_PROB"))
		{
*/
		int iOdds = GC.getDefineINT("NUCLEAR_WINTER_PROB"); //Fuyu: iGlobalWarmingDefense does no longer protect from nuclear winters
		iOdds *= 100;
		iOdds /= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getVictoryDelayPercent();

		if (getSorenRandNum(100, "Nuclear Fallout") < iOdds)
		{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/

			CvPlot* pPlot = GC.getMapINLINE().syncRandPlot(RANDPLOT_LAND | RANDPLOT_NOT_CITY);
			//FeatureTypes eFeature = pPlot->getFeatureType();

			if (pPlot != NULL)
			{
				bool bChanged = false;
				if (pPlot->getFeatureType() != NO_FEATURE)
				{
					if (pPlot->getFeatureType() != GC.getDefineINT("NUKE_FEATURE"))
					{
						if (pPlot->getFeatureType() != eColdFeature)
						{
							pPlot->setFeatureType(NO_FEATURE);
							bChanged = true;
						}
					}
				}
				else
				{
					pPlot->setFeatureType(eFalloutFeature);
					bChanged = true;
				}
				if (getSorenRandNum(100, "Nuclear Winter") + iGlobalWarmingDefense < GC.getDefineINT("NUCLEAR_WINTER_PROB"))
				{
					if (pPlot->getTerrainType() == eColdTerrain)
					{
						pPlot->setTerrainType(eFrozenTerrain);
						bChanged = true;
					}
					if (pPlot->calculateTotalBestNatureYield(NO_TEAM) > 1)
					{
						pPlot->setTerrainType(eColdTerrain);
						bChanged = true;
					}
				}
				if (bChanged)
				{
					pPlot->setImprovementType(NO_IMPROVEMENT);

					CvCity* pCity = GC.getMapINLINE().findCity(pPlot->getX_INLINE(), pPlot->getY_INLINE());
					if (pCity != NULL)
					{
						if (pPlot->isVisible(pCity->getTeam(), false))
						{
							MEMORY_TRACK_EXEMPT();

							CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_NUCLEAR_WINTER_NEAR_CITY", pCity->getNameKey());
							AddDLLMessage(pCity->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_GLOBALWARMING", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
						}
					}
				}
			}
		}
	}
}//GWMod end M.A.


void CvGame::doHolyCity()
{
/************************************************************************************************/
/* RevDCM	                  Start		09/08/10                                                */
/*                                                                                              */
/* OC_LIMITED_RELIGIONS                                                                         */
/************************************************************************************************/
	if (isGameStart())
	{
		doHolyCityGameStart();
		return;
	}
/************************************************************************************************/
/* REVDCM                                  END                                                  */
/************************************************************************************************/
	PlayerTypes eBestPlayer;
	TeamTypes eBestTeam;
	int iValue;
	int iBestValue;
	int iI, iJ, iK;

	int iRandOffset = getSorenRandNum(GC.getNumReligionInfos(), "Holy City religion offset");
	for (int iLoop = 0; iLoop < GC.getNumReligionInfos(); ++iLoop)
	{
		iI = ((iLoop + iRandOffset) % GC.getNumReligionInfos());

		if (!isReligionSlotTaken((ReligionTypes)iI))
		{
			iBestValue = MAX_INT;
			eBestTeam = NO_TEAM;

			for (iJ = 0; iJ < MAX_TEAMS; iJ++)
			{
				if (GET_TEAM((TeamTypes)iJ).isAlive())
				{
					if (GET_TEAM((TeamTypes)iJ).isHasTech((TechTypes)(GC.getReligionInfo((ReligionTypes)iI).getTechPrereq())))
					{
						if (GET_TEAM((TeamTypes)iJ).getNumCities() > 0)
						{
							iValue = getSorenRandNum(10, "Found Religion (Team)");

							for (iK = 0; iK < GC.getNumReligionInfos(); iK++)
							{
								int iReligionCount = GET_TEAM((TeamTypes)iJ).getHasReligionCount((ReligionTypes)iK);

								if (iReligionCount > 0)
								{
									iValue += iReligionCount * 20;
								}
							}

							if (iValue < iBestValue)
							{
								iBestValue = iValue;
								eBestTeam = ((TeamTypes)iJ);
							}
						}
					}
				}
			}

			if (eBestTeam != NO_TEAM)
			{
				iBestValue = MAX_INT;
				eBestPlayer = NO_PLAYER;

				for (iJ = 0; iJ < MAX_PLAYERS; iJ++)
				{
					if (GET_PLAYER((PlayerTypes)iJ).isAlive())
					{
						if (GET_PLAYER((PlayerTypes)iJ).getTeam() == eBestTeam)
						{
							if (GET_PLAYER((PlayerTypes)iJ).getNumCities() > 0)
							{
								iValue = getSorenRandNum(10, "1 Found Religion (Player)");

								if (!(GET_PLAYER((PlayerTypes)iJ).isHuman()))
								{
									iValue += 10;
								}

								for (iK = 0; iK < GC.getNumReligionInfos(); iK++)
								{
									int iReligionCount = GET_PLAYER((PlayerTypes)iJ).getHasReligionCount((ReligionTypes)iK);

									if (iReligionCount > 0)
									{
										iValue += iReligionCount * 20;
									}
								}

								if (iValue < iBestValue)
								{
									iBestValue = iValue;
									eBestPlayer = ((PlayerTypes)iJ);
								}
							}
						}
					}
				}

				if (eBestPlayer != NO_PLAYER)
				{
					ReligionTypes eReligion = (ReligionTypes)iI;

					if (isOption(GAMEOPTION_PICK_RELIGION))
					{
						eReligion = GET_PLAYER(eBestPlayer).AI_chooseReligion();
					}

					if (NO_RELIGION != eReligion)
					{
/************************************************************************************************/
/* RevDCM	                  Start		 4/29/10                                                */
/*                                                                                              */
/* OC_LIMITED_RELIGIONS                                                                         */
/************************************************************************************************/
						if(GET_PLAYER(eBestPlayer).canFoundReligion())
						{
							GET_PLAYER(eBestPlayer).foundReligion(eReligion, (ReligionTypes)iI, false);
						}
/************************************************************************************************/
/* REVDCM                                  END                                                  */
/************************************************************************************************/
					}
				}
			}
		}
	}
}


/************************************************************************************************/
/* RevDCM	                  Start		09/08/10                                                */
/*                                                                                              */
/* OC_LIMITED_RELIGIONS                                                                         */
/************************************************************************************************/
void CvGame::doHolyCityGameStart()
{
	int iLoopTeam;
	int iLoopReligion;
	int iLoopPlayer;
	TeamTypes eLoopTeam;
	ReligionTypes eLoopReligion;
	PlayerTypes eLoopPlayer;
	int iLotteryIncrement = MAX_TEAMS * GC.getNumReligionInfos();


	bool *pb_religionCanFound = new bool[GC.getNumReligionInfos()];
	for(iLoopReligion = 0; iLoopReligion < GC.getNumReligionInfos(); ++iLoopReligion)
	{
		pb_religionCanFound[iLoopReligion] = false;

		eLoopReligion = ReligionTypes(iLoopReligion);
		if(eLoopReligion != NO_RELIGION)
		{
			CvReligionInfo& kLoopReligion = GC.getReligionInfo(eLoopReligion);

			for(iLoopTeam = 0; iLoopTeam < MAX_TEAMS; iLoopTeam++)
			{
				eLoopTeam = TeamTypes(iLoopTeam);
				if(eLoopTeam != NO_TEAM)
				{
					CvTeam& kLoopTeam = GET_TEAM(eLoopTeam);
					if(kLoopTeam.isHasTech(TechTypes(kLoopReligion.getTechPrereq())))
					{
						if(!(isReligionSlotTaken(eLoopReligion)))
						{
							pb_religionCanFound[eLoopReligion] = true;
							break;
						}
					}
				}
			}
		}
	}


	bool *pb_teamCanFound = new bool[MAX_TEAMS];
	for(iLoopTeam = 0; iLoopTeam < MAX_TEAMS; ++iLoopTeam)
	{
		bool bValid = false;
		eLoopTeam = TeamTypes(iLoopTeam);
		if(eLoopTeam != NO_TEAM && eLoopTeam != BARBARIAN_TEAM)
		{
			CvTeam& kLoopTeam = GET_TEAM(eLoopTeam);

			if(kLoopTeam.isAlive())
			{

				for (iLoopPlayer = 0; iLoopPlayer < MAX_PLAYERS; iLoopPlayer++)
				{
					eLoopPlayer = PlayerTypes(iLoopPlayer);
					if(eLoopPlayer != NO_PLAYER && eLoopPlayer != BARBARIAN_PLAYER)
					{
						CvPlayer& kLoopPlayer = GET_PLAYER(eLoopPlayer);

						if(kLoopPlayer.getTeam() == eLoopTeam && kLoopPlayer.isAlive())
						{
							if(kLoopPlayer.canFoundReligion())
							{
								pb_teamCanFound[eLoopTeam] = true;
								bValid = true;
								break;
							}
						}
					}
				}
				if(bValid)
				{
					continue;
				}
			}
		}

		pb_teamCanFound[eLoopTeam] = false;
	}


	int iTeamsLeft = 0;
	for(iLoopTeam = 0; iLoopTeam < MAX_TEAMS; iLoopTeam++)
	{
		if(pb_teamCanFound[iLoopTeam])
		{
			iTeamsLeft++;
		}
	}

	for(iLoopReligion = 0; iLoopReligion < GC.getNumReligionInfos(); ++iLoopReligion)
	{
		if(iTeamsLeft == 0)
		{
			break;
		}

		eLoopReligion = ReligionTypes(iLoopReligion); 
		int iScore = MAX_INT;
		int iTempScore;
		TeamTypes eBestTeam = NO_TEAM;

		if(eLoopReligion != NO_RELIGION)
		{
			CvReligionInfo& kLoopReligion = GC.getReligionInfo(eLoopReligion);

			if(pb_religionCanFound[eLoopReligion])
			{

				for(iLoopTeam = 0; iLoopTeam < MAX_TEAMS; iLoopTeam++)
				{
					eLoopTeam = TeamTypes(iLoopTeam);
					if(eLoopTeam != NO_TEAM && eLoopTeam != BARBARIAN_TEAM)
					{
						CvTeam& kLoopTeam = GET_TEAM(eLoopTeam);

						if( pb_teamCanFound[eLoopTeam] && kLoopTeam.isHasTech(TechTypes(kLoopReligion.getTechPrereq())) )
						{
							iTempScore = getSorenRandNum(iLotteryIncrement, "Early Religion Lottery");
							for (iLoopPlayer = 0; iLoopPlayer < MAX_PLAYERS; iLoopPlayer++)
							{
								eLoopPlayer = PlayerTypes(iLoopPlayer);
								if(eLoopPlayer != NO_PLAYER)
								{
									CvPlayer& kLoopPlayer = GET_PLAYER(eLoopPlayer);

									if(kLoopPlayer.getTeam() == eLoopTeam)
									{
										int iILoopReligion;
										for(iILoopReligion = 0; iILoopReligion < GC.getNumReligionInfos(); iILoopReligion++)
										{
											if(kLoopPlayer.hasHolyCity(ReligionTypes(iILoopReligion)))
											{
												iTempScore += iLotteryIncrement;
											}
										}
									}
								}
							}

							if(iTempScore < iScore)
							{
								iScore = iTempScore;
								eBestTeam = eLoopTeam;
							}
						}
					}
				}

				if(eBestTeam != NO_TEAM && eBestTeam != BARBARIAN_TEAM)
				{
					pb_teamCanFound[eBestTeam] = false;
					doHolyCityPlayer(eBestTeam, eLoopReligion);
					iTeamsLeft--;
				}
			}
		}
	}


	if(pb_religionCanFound)
	{
		delete [] pb_religionCanFound;
	}

	if(pb_teamCanFound)
	{
		delete [] pb_teamCanFound;
	}
}

void CvGame::doHolyCityPlayer(TeamTypes eTeam, ReligionTypes eReligion)
{
	FAssert(eTeam >= 0 && eTeam < MAX_TEAMS);
	FAssert(eReligion >= 0 && eReligion < GC.getNumReligionInfos());

	int iValue = MAX_INT;
	int iLoopPlayer;
	int iLoopReligion;
	PlayerTypes eLoopPlayer;
	PlayerTypes eBestPlayer = NO_PLAYER;
	int iLotteryIncrement = MAX_PLAYERS * GC.getNumReligionInfos();

	for (iLoopPlayer = 0; iLoopPlayer < MAX_PLAYERS; iLoopPlayer++)
	{
		int iTempValue;
		eLoopPlayer = PlayerTypes(iLoopPlayer);
		if(eLoopPlayer != NO_PLAYER  && eLoopPlayer != BARBARIAN_PLAYER)
		{
			CvPlayer& kLoopPlayer = GET_PLAYER(eLoopPlayer);

			if( kLoopPlayer.getTeam() == eTeam && kLoopPlayer.isAlive() )
			{
				if(kLoopPlayer.canFoundReligion())
				{
					iTempValue = getSorenRandNum(iLotteryIncrement, "Early Religion Founding (Player)");
					for(iLoopReligion = 0; iLoopReligion < GC.getNumReligionInfos(); iLoopReligion++)
					{
						if(kLoopPlayer.hasHolyCity(ReligionTypes(iLoopReligion)))
						{
							iTempValue += iLotteryIncrement;
						}
					}
					if(iTempValue < iValue)
					{
						iValue = iTempValue;
						eBestPlayer = eLoopPlayer;
					}
				}
			}
		}
	}

	if(eBestPlayer != NO_PLAYER  && eBestPlayer != BARBARIAN_PLAYER)
	{
		GET_PLAYER(eBestPlayer).foundReligion(eReligion, eReligion, false);
	}
}
/************************************************************************************************/
/* REVDCM                                  END                                                  */
/************************************************************************************************/


void CvGame::doHeadquarters()
{
	MEMORY_TRACE_FUNCTION();

	long lResult = 0;
	PYTHON_CALL_FUNCTION4(__FUNCTION__, PYGameModule, "doHeadquarters", NULL, &lResult);
	if (lResult == 1)
	{
		return;
	}

	if (getElapsedGameTurns() < 5)
	{
		return;
	}

	for (int iI = 0; iI < GC.getNumCorporationInfos(); iI++)
	{
		CvCorporationInfo& kCorporation = GC.getCorporationInfo((CorporationTypes)iI);
		if (!isCorporationFounded((CorporationTypes)iI))
		{
			int iBestValue = MAX_INT;
			TeamTypes eBestTeam = NO_TEAM;

			for (int iJ = 0; iJ < MAX_TEAMS; iJ++)
			{
				CvTeam& kLoopTeam = GET_TEAM((TeamTypes)iJ);
				if (kLoopTeam.isAlive())
				{
					if (NO_TECH != kCorporation.getTechPrereq() && kLoopTeam.isHasTech((TechTypes)(kCorporation.getTechPrereq())))
					{
						if (kLoopTeam.getNumCities() > 0)
						{
							bool bHasBonus = false;
							for (int i = 0; i < GC.getNUM_CORPORATION_PREREQ_BONUSES(); ++i)
							{
								if (NO_BONUS != kCorporation.getPrereqBonus(i) && kLoopTeam.hasBonus((BonusTypes)kCorporation.getPrereqBonus(i)))
								{
									bHasBonus = true;
									break;
								}
							}

							if (bHasBonus)
							{
								int iValue = getSorenRandNum(10, "Found Corporation (Team)");

								for (int iK = 0; iK < GC.getNumCorporationInfos(); iK++)
								{
									int iCorporationCount = GET_PLAYER((PlayerTypes)iJ).getHasCorporationCount((CorporationTypes)iK);

									if (iCorporationCount > 0)
									{
										iValue += iCorporationCount * 20;
									}
								}

								if (iValue < iBestValue)
								{
									iBestValue = iValue;
									eBestTeam = ((TeamTypes)iJ);
								}
							}
						}
					}
				}
			}

			if (eBestTeam != NO_TEAM)
			{
				iBestValue = MAX_INT;
				PlayerTypes eBestPlayer = NO_PLAYER;

				for (int iJ = 0; iJ < MAX_PLAYERS; iJ++)
				{
					CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iJ);
					if (kLoopPlayer.isAlive())
					{
						if (kLoopPlayer.getTeam() == eBestTeam)
						{
							if (kLoopPlayer.getNumCities() > 0)
							{
								bool bHasBonus = false;
								for (int i = 0; i < GC.getNUM_CORPORATION_PREREQ_BONUSES(); ++i)
								{
									if (NO_BONUS != kCorporation.getPrereqBonus(i) && kLoopPlayer.hasBonus((BonusTypes)kCorporation.getPrereqBonus(i)))
									{
										bHasBonus = true;
										break;
									}
								}

								if (bHasBonus)
								{
									int iValue = getSorenRandNum(10, "2 Found Religion (Player)");

									if (!kLoopPlayer.isHuman())
									{
										iValue += 10;
									}

									for (int iK = 0; iK < GC.getNumCorporationInfos(); iK++)
									{
										int iCorporationCount = GET_PLAYER((PlayerTypes)iJ).getHasCorporationCount((CorporationTypes)iK);

										if (iCorporationCount > 0)
										{
											iValue += iCorporationCount * 20;
										}
									}

									if (iValue < iBestValue)
									{
										iBestValue = iValue;
										eBestPlayer = ((PlayerTypes)iJ);
									}
								}
							}
						}
					}
				}

				if (eBestPlayer != NO_PLAYER)
				{
					GET_PLAYER(eBestPlayer).foundCorporation((CorporationTypes)iI);
				}
			}
		}
	}
}


void CvGame::doDiploVote()
{
	MEMORY_TRACE_FUNCTION();

	doVoteResults();

	doVoteSelection();
}


void CvGame::createBarbarianCities()
{
	MEMORY_TRACE_FUNCTION();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iTargetCities;
	int iBestValue;
	int iI;

	if (getMaxCityElimination() > 0)
	{
		return;
	}

	if (isOption(GAMEOPTION_NO_BARBARIANS))
	{
		return;
	}

	if (GC.getEraInfo(getCurrentEra()).isNoBarbCities())
	{
		return;
	}

	if (GC.getHandicapInfo(getHandicapType()).getUnownedTilesPerBarbarianCity() <= 0)
	{
		return;
	}

/************************************************************************************************/
/* REVOLUTION_MOD                         02/01/09                                jdog5000      */
/*                                                                                              */
/* For BarbarianCiv, allows earlier barb cities                                                 */
/************************************************************************************************/
/* original code
	if (getNumCivCities() < (countCivPlayersAlive() * 2))
	{
		return;
	}

	if (getElapsedGameTurns() < (((GC.getHandicapInfo(getHandicapType()).getBarbarianCityCreationTurnsElapsed() * GC.getGameSpeedInfo(getGameSpeedType()).getBarbPercent()) / 100) / std::max(getStartEra() + 1, 1)))
	{
		return;
	}

	if (getSorenRandNum(100, "Barb City Creation") >= GC.getHandicapInfo(getHandicapType()).getBarbarianCityCreationProb())
	{
		return;
	}
*/
	if (!isOption(GAMEOPTION_NO_BARBARIAN_CIV))
	{
		if( getNumCivCities() <= (countCivPlayersAlive()))
		{
			return;
		}
	}
	else if (getNumCivCities() < (countCivPlayersAlive() * 2))
	{
		return;
	}

	if (getElapsedGameTurns() < (((GC.getHandicapInfo(getHandicapType()).getBarbarianCityCreationTurnsElapsed() * GC.getGameSpeedInfo(getGameSpeedType()).getBarbPercent()) / 100) / std::max(getStartEra() + 1, 1)))
	{
		return;
	}

	int iRand = getSorenRandNum(100, "Barb City Creation");
	if (!isOption(GAMEOPTION_NO_BARBARIAN_CIV))
	{
		if( countCivPlayersAlive() + GET_PLAYER(BARBARIAN_PLAYER).getNumCities() < GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getDefaultPlayers() )
		{
			iRand *= 2;
			iRand /= 3;
		}
	}

	if ( iRand >= GC.getHandicapInfo(getHandicapType()).getBarbarianCityCreationProb())
	{
		return;
	}
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/

	iBestValue = 0;
	pBestPlot = NULL;
	
	int iTargetCitiesMultiplier = 100;
	{
		int iTargetBarbCities = (getNumCivCities() * 5 * GC.getHandicapInfo(getHandicapType()).getBarbarianCityCreationProb()) / 100;
		int iBarbCities = GET_PLAYER(BARBARIAN_PLAYER).getNumCities();
		if (iBarbCities < iTargetBarbCities)
		{
			iTargetCitiesMultiplier += (300 * (iTargetBarbCities - iBarbCities)) / iTargetBarbCities;
		}
		
		if (isOption(GAMEOPTION_RAGING_BARBARIANS))
		{
			iTargetCitiesMultiplier *= 3;
			iTargetCitiesMultiplier /= 2;
		}
	}

/************************************************************************************************/
/* REVOLUTION_MOD                         04/19/08                                jdog5000      */
/*                                                                                              */
/* For BarbarianCiv, reduces emphasis on creating barb cities in                                */
/* large, occupied areas, allows barbs more readily in unoccupied areas                         */
/************************************************************************************************/
	// New variable for emphaizing spawning cities on populated continents
	int iOccupiedAreaMultiplier = 50;
	int iOwnedPlots = 0;

	if(!isOption(GAMEOPTION_NO_BARBARIAN_CIV))
	{
		for( iI = 0; iI < GC.getMAX_PLAYERS(); iI++ )
		{
			iOwnedPlots += GET_PLAYER((PlayerTypes)iI).getTotalLand();
		}
	
		// When map mostly open, emphasize areas with other civs
		iOccupiedAreaMultiplier += 100 - (iOwnedPlots)/GC.getMapINLINE().getLandPlots();
		
		// If raging barbs is on, emphasize areas with other civs
		if( isOption(GAMEOPTION_RAGING_BARBARIANS) )
		{
			iOccupiedAreaMultiplier *= 3;
			iOccupiedAreaMultiplier /= 2;
		}
	}
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (!(pLoopPlot->isWater()))
		{
			if (!(pLoopPlot->isVisibleToCivTeam()))
			{
				iTargetCities = pLoopPlot->area()->getNumUnownedTiles();

				if (pLoopPlot->area()->getNumCities() == pLoopPlot->area()->getCitiesPerPlayer(BARBARIAN_PLAYER))
				{
					iTargetCities *= 3;
				}
								
				int iUnownedTilesThreshold = GC.getHandicapInfo(getHandicapType()).getUnownedTilesPerBarbarianCity();
				
				if (pLoopPlot->area()->getNumTiles() < (iUnownedTilesThreshold / 3))
				{
					iTargetCities *= iTargetCitiesMultiplier;
					iTargetCities /= 100;
				}				

				iTargetCities /= std::max(1, iUnownedTilesThreshold);

				if (pLoopPlot->area()->getCitiesPerPlayer(BARBARIAN_PLAYER) < iTargetCities)
				{
					int iValue = GET_PLAYER(BARBARIAN_PLAYER).AI_foundValue(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getDefineINT("MIN_BARBARIAN_CITY_STARTING_DISTANCE"));
				
/************************************************************************************************/
/* REVOLUTION_MOD                         02/01/09                                jdog5000      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/* original code
					if (iTargetCitiesMultiplier > 100)
					{
						iValue *= pLoopPlot->area()->getNumOwnedTiles();
					}

					iValue += (100 + getSorenRandNum(50, "Barb City Found"));
					iValue /= 100;
*/
					if (!isOption(GAMEOPTION_NO_BARBARIAN_CIV))
					{
						if( pLoopPlot->area()->getNumCities() == pLoopPlot->area()->getCitiesPerPlayer(BARBARIAN_PLAYER) )
						{
							// Counteracts the AI_foundValue emphasis on empty areas
							iValue *= 2;
							iValue /= 3;
						}
	
						if( iTargetCitiesMultiplier > 100 )		// Either raging barbs is set or fewer barb cities than desired
						{
							// Emphasis on placing barb cities in populated areas
							iValue += (iOccupiedAreaMultiplier*(pLoopPlot->area()->getNumCities() - pLoopPlot->area()->getCitiesPerPlayer(BARBARIAN_PLAYER)))/getNumCivCities();
						}
					}
					else
					{
						if (iTargetCitiesMultiplier > 100)
						{
							iValue *= pLoopPlot->area()->getNumOwnedTiles();
						}
					}

					if( pLoopPlot->isBestAdjacentFound(BARBARIAN_PLAYER) ) 
					{
						iValue *= 120 + getSorenRandNum(30, "Barb City Found");
						iValue /= 100;
					}
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = pLoopPlot;
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		GET_PLAYER(BARBARIAN_PLAYER).found(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
	}
}


void CvGame::createBarbarianUnits()
{
	MEMORY_TRACE_FUNCTION();

	/*CvUnit* pLoopUnit;*/
	CvArea* pLoopArea;
	CvPlot* pPlot;
	UnitAITypes eBarbUnitAI;
	UnitTypes eBestUnit;
	UnitTypes eLoopUnit;
	int iNeededBarbs;
	int iDivisor;
	int iValue;
	int iBestValue;
	int iLoop = 0;
	int iI, iJ;

	if (isOption(GAMEOPTION_NO_BARBARIANS))
	{
		return;
	}

	int iBarbsTurnsElapsed = GC.getHandicapInfo(getHandicapType()).getBarbarianCreationTurnsElapsed();
	if (isOption(GAMEOPTION_RAGING_BARBARIANS))
	{
		iBarbsTurnsElapsed = 1;
	}

	bool bAnimals = GC.getEraInfo(getCurrentEra()).isNoBarbUnits();

#ifdef C2C_BUILD
//TB animal mod begin
	if (!GC.getEraInfo(getCurrentEra()).isNoBarbUnits())
	{
		if (getSorenRandNum(100, "Animal or Barb") < 50)
		{
			bAnimals = true;
		}
	}
//TB animal mod end
#endif

	if (getNumCivCities() < ((countCivPlayersAlive() * 3) / 2) && !isOption(GAMEOPTION_ONE_CITY_CHALLENGE))
	{
		bAnimals = true;
	}

	if (getElapsedGameTurns() < ((iBarbsTurnsElapsed * GC.getGameSpeedInfo(getGameSpeedType()).getBarbPercent()) / 100))
	{
		bAnimals = true;
	}

	if (bAnimals)
	{
		createAnimals();
	}
	else
	{
		for(pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
		{
			if (pLoopArea->isWater())
			{
				eBarbUnitAI = UNITAI_ATTACK_SEA;
				if (isOption(GAMEOPTION_RAGING_BARBARIANS))
				{
					iDivisor = 20;
				}
				else
				{
					iDivisor = GC.getHandicapInfo(getHandicapType()).getUnownedWaterTilesPerBarbarianUnit();
				}
			}
			else
			{
				eBarbUnitAI = UNITAI_ATTACK;
				if (isOption(GAMEOPTION_RAGING_BARBARIANS))
				{
					iDivisor = 1;
				}
				else
				{
					iDivisor = GC.getHandicapInfo(getHandicapType()).getUnownedTilesPerBarbarianUnit();
				}
			}

			/*if (isOption(GAMEOPTION_RAGING_BARBARIANS))
			{
				iDivisor = std::max(1, (iDivisor / 2));
			}*/

			if (iDivisor > 0)
			{
				iNeededBarbs = ((pLoopArea->getNumUnownedTiles() / iDivisor) - pLoopArea->getUnitsPerPlayer(BARBARIAN_PLAYER)); // XXX eventually need to measure how many barbs of eBarbUnitAI we have in this area...

				//Afforess increase barb spawning
				if (isOption(GAMEOPTION_RAGING_BARBARIANS))
				{
					iNeededBarbs += 75;
				}
				if (iNeededBarbs > 0)
				{
					if (!isOption(GAMEOPTION_RAGING_BARBARIANS))
					{
						iNeededBarbs = ((iNeededBarbs / 4) + 1);
					}

					/********************************************************************************/
					/* 	BETTER_BTS_AI_MOD						9/25/08				jdog5000	*/
					/* 																			*/
					/* 	Barbarian AI															*/
					/********************************************************************************/
					// Limit construction of barb ships based on player navies
					// Keeps barb ship count in check in early game since generation is greatly increased for BTS 3.17
					if( pLoopArea->isWater() )
					{
						int iPlayerSeaUnits = 0;
						for( int iI = 0; iI < MAX_CIV_PLAYERS; iI++ )
						{
							if( GET_PLAYER((PlayerTypes)iI).isAlive() )
							{
								iPlayerSeaUnits += GET_PLAYER((PlayerTypes)iI).AI_totalWaterAreaUnitAIs(pLoopArea,UNITAI_ATTACK_SEA);
								iPlayerSeaUnits += GET_PLAYER((PlayerTypes)iI).AI_totalWaterAreaUnitAIs(pLoopArea,UNITAI_EXPLORE_SEA);
								iPlayerSeaUnits += GET_PLAYER((PlayerTypes)iI).AI_totalWaterAreaUnitAIs(pLoopArea,UNITAI_ASSAULT_SEA);
								iPlayerSeaUnits += GET_PLAYER((PlayerTypes)iI).AI_totalWaterAreaUnitAIs(pLoopArea,UNITAI_SETTLER_SEA);
							}
						}

						if( pLoopArea->getUnitsPerPlayer(BARBARIAN_PLAYER) > (iPlayerSeaUnits/*/3 + 1*/) )	//45deg: increased barb navy
						{
							iNeededBarbs = 0;
						}
					}
					/********************************************************************************/
					/* 	BETTER_BTS_AI_MOD						END								*/
					/********************************************************************************/

					for (iI = 0; iI < iNeededBarbs; iI++)
					{
						pPlot = GC.getMapINLINE().syncRandPlot((RANDPLOT_NOT_VISIBLE_TO_CIV | RANDPLOT_ADJACENT_LAND | RANDPLOT_PASSIBLE), pLoopArea->getID(), GC.getDefineINT("MIN_BARBARIAN_STARTING_DISTANCE"));

						if (pPlot != NULL)
						{
							eBestUnit = NO_UNIT;
							iBestValue = 0;

							for (iJ = 0; iJ < GC.getNumUnitClassInfos(); iJ++)
							{
								bool bValid = false;
								eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(GET_PLAYER(BARBARIAN_PLAYER).getCivilizationType()).getCivilizationUnits(iJ)));

								if (eLoopUnit != NO_UNIT)
								{
									CvUnitInfo& kUnit = GC.getUnitInfo(eLoopUnit);

									bValid = (kUnit.getCombat() > 0 && !kUnit.isOnlyDefensive());

									if (bValid)
									{
										if (pLoopArea->isWater() && kUnit.getDomainType() != DOMAIN_SEA)
										{
											bValid = false;
										}
										else if (!pLoopArea->isWater() && kUnit.getDomainType() != DOMAIN_LAND)
										{
											bValid = false;
										}
									}

									if (bValid)
									{
										if (!GET_PLAYER(BARBARIAN_PLAYER).canTrain(eLoopUnit))
										{
											bValid = false;
										}
									}

									if (bValid)
									{
										if (NO_BONUS != kUnit.getPrereqAndBonus())
										{
											if (!GET_TEAM(BARBARIAN_TEAM).isHasTech((TechTypes)GC.getBonusInfo((BonusTypes)kUnit.getPrereqAndBonus()).getTechCityTrade()))
											{
												bValid = false;
											}
										}
									}

									//	Koshling - don't allow direct spawning of barbarian units that have building pre-reqs
									if (bValid)
									{
										if (NO_BUILDING != kUnit.getPrereqBuilding())
										{
											bValid = false;
										}
									}
									//45deg - don't allow barbarian trade caravans
									if (bValid)
									{
										if (kUnit.getBaseHurry() != 0)
										{
											bValid = false;
										}
									}									
									
									if (bValid)
									{
										bool bFound = false;
										bool bRequires = false;
										for (int i = 0; i < GC.getNUM_UNIT_PREREQ_OR_BONUSES(); ++i)
										{
											if (NO_BONUS != kUnit.getPrereqOrBonuses(i))
											{
												TechTypes eTech = (TechTypes)GC.getBonusInfo((BonusTypes)kUnit.getPrereqOrBonuses(i)).getTechCityTrade();
												if (NO_TECH != eTech)
												{
													bRequires = true;
													
													if (GET_TEAM(BARBARIAN_TEAM).isHasTech(eTech))
													{
														bFound = true;
														break;
													}
												}
											}
										}

										if (bRequires && !bFound)
										{
											bValid = false;
										}
									}

									if (bValid)
									{
										iValue = (1 + getSorenRandNum(1000, "Barb Unit Selection"));

										if (kUnit.getUnitAIType(eBarbUnitAI))
										{
											iValue += 200;
										}
										// Thomas SG - AC: Advanced Cargo START
										if (kUnit.getTotalCargoSpace() > 0)
										{
											iValue += (kUnit.getTotalCargoSpace() - kUnit.getTotalSpecialCargoSpace()) * 50 * getSorenRandNum(100, "Barb Cargo Selection");
										}
										// Thomas SG - AC: Advanced Cargo END										

										if (iValue > iBestValue)
										{
											eBestUnit = eLoopUnit;
											iBestValue = iValue;
										}
									}
								}
							}

							if (eBestUnit != NO_UNIT)
							{
								
/************************************************************************************************/
/* Afforess	                  Start		 08/29/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
								CvUnit* pUnit = GET_PLAYER(BARBARIAN_PLAYER).initUnit(eBestUnit, pPlot->getX_INLINE(), pPlot->getY_INLINE(), eBarbUnitAI, NO_DIRECTION, GC.getGameINLINE().getSorenRandNum(10000, "AI Unit Birthmark 8"));
								loadPirateShip(pUnit);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
							}
						}
					}
				}
/************************************************************************************************/
/* REVOLUTION_MOD                         01/31/09                                jdog5000      */
/*                                                                                              */
/* For BarbarianCiv                                                                             */
/************************************************************************************************/
				// Give barb cities in occupied areas free workers so that if the city settles
				// it has some infrastructure
				if (!isOption(GAMEOPTION_NO_BARBARIAN_CIV))
				{
					int iBarbCities = pLoopArea->getCitiesPerPlayer(BARBARIAN_PLAYER);
					if( (iBarbCities > 0) && (pLoopArea->getNumCities() - iBarbCities > 0) )
					{
						CvCity* pLoopCity;
						int iCityLoop = 0;
						for (pLoopCity = GET_PLAYER(BARBARIAN_PLAYER).firstCity(&iCityLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(BARBARIAN_PLAYER).nextCity(&iCityLoop))
						{
							if( pLoopCity->area() == pLoopArea && pLoopCity->getOriginalOwner() == BARBARIAN_PLAYER )
							{
								if( (pLoopCity->getPopulation() > 1) || (getGameTurn() - pLoopCity->getGameTurnAcquired()) > (10 * GC.getGameSpeedInfo(getGameSpeedType()).getTrainPercent())/100 )
								{
									if( pLoopCity->AI_getWorkersHave() == 0 && pLoopCity->AI_getWorkersNeeded() > 0)
									{
										if( (7*pLoopCity->getPopulation()) > getSorenRandNum(100, "Barb - workers") )
										{
											int iUnitValue;

											eBestUnit = pLoopCity->AI_bestUnitAI(UNITAI_WORKER, iUnitValue);
											if (eBestUnit != NO_UNIT)
											{
												GET_PLAYER(BARBARIAN_PLAYER).initUnit(eBestUnit, pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), UNITAI_WORKER, NO_DIRECTION, GC.getGameINLINE().getSorenRandNum(10000, "AI Unit Birthmark 9"));
												pLoopCity->AI_setChooseProductionDirty(true);
											}
										}
									}
								}
							}
						}							
					}
				}
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/
			}
		}
/************************************************************************************************/
/* Afforess	                  Start		 09/01/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		// TB Note: Commented out as a test for now to see if this would be preferred as I suspect.
		//Kill Stranded Units
		//for (pLoopUnit = GET_PLAYER(BARBARIAN_PLAYER).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(BARBARIAN_PLAYER).nextUnit(&iLoop))
		//{
		//	if (pLoopUnit->getGroup()->isStranded())
		//	{
		//		pLoopUnit->kill(false);
		//		break;
		//	}
		//}
		////Kill off extra ships
		//int iRand = 10000;
		//int iBarbSeaUnits = 0;
		//for(pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
		//{
		//	if (pLoopArea->isWater())
		//	{
		//		iBarbSeaUnits	+= GET_PLAYER(BARBARIAN_PLAYER).AI_totalWaterAreaUnitAIs(pLoopArea,UNITAI_ATTACK_SEA);
		//		iBarbSeaUnits += GET_PLAYER(BARBARIAN_PLAYER).AI_totalWaterAreaUnitAIs(pLoopArea,UNITAI_EXPLORE_SEA);
		//		iBarbSeaUnits += GET_PLAYER(BARBARIAN_PLAYER).AI_totalWaterAreaUnitAIs(pLoopArea,UNITAI_ASSAULT_SEA);
		//		iBarbSeaUnits += GET_PLAYER(BARBARIAN_PLAYER).AI_totalWaterAreaUnitAIs(pLoopArea,UNITAI_SETTLER_SEA);
		//	}
		//}
		//iRand /= std::max(iBarbSeaUnits, 1);
		//if (iBarbSeaUnits > 0)
		//{
		//	for (pLoopUnit = GET_PLAYER(BARBARIAN_PLAYER).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(BARBARIAN_PLAYER).nextUnit(&iLoop))
		//	{
		//		if (pLoopUnit->plot()->isCity() && pLoopUnit->getDomainType() == DOMAIN_SEA)
		//		{
		//			if (getSorenRandNum(iRand, "Remove Stranded Barbarian Ships") == 0)
		//			{
		//				pLoopUnit->kill(false);
		//			}
		//		}
		//	}
		//}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

//		for (pLoopUnit = GET_PLAYER(BARBARIAN_PLAYER).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(BARBARIAN_PLAYER).nextUnit(&iLoop))
//		{
//			if (pLoopUnit->isAnimal())
//			{
//				pLoopUnit->kill(false);
//				break;
//			}
//		}
	}
}


void CvGame::createAnimals()
{
	PROFILE_FUNC();

	CvArea* pLoopArea;
	CvPlot* pPlot;
	UnitTypes eBestUnit;
	UnitTypes eLoopUnit;
	int iNeededAnimals;
	int iValue;
	int iBestValue;
	int iLoop = 0;
	int iI, iJ;

	if (GC.getEraInfo(getCurrentEra()).isNoAnimals())
	{
		return;
	}

	if (GC.getHandicapInfo(getHandicapType()).getUnownedTilesPerGameAnimal() <= 0)
	{
		return;
	}

	if (getNumCivCities() < countCivPlayersAlive())
	{
		return;
	}

	if (getElapsedGameTurns() < 5)
	{
		return;
	}

	for(pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
	{
/************************************************************************************************/
/* Afforess                                     12/7/09                                         */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		/*
		if (!(pLoopArea->isWater()))
		*/
		if( pLoopArea->isWater() )
		{
			iValue = (1 + getSorenRandNum(100, "Water Animal Spawn Check"));
			if(iValue > getWaterAnimalSpawnChance())
			{
				continue;
			}
		}

/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	
			if (!(GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS)))
			{
				iNeededAnimals = ((pLoopArea->getNumUnownedTiles() / GC.getHandicapInfo(getHandicapType()).getUnownedTilesPerGameAnimal()) - pLoopArea->getUnitsPerPlayer(BARBARIAN_PLAYER));
			}
			else
			{
				iNeededAnimals = ((pLoopArea->getNumUnownedTiles() / GC.getHandicapInfo(getHandicapType()).getUnownedTilesPerGameAnimal()) - (pLoopArea->getUnitsPerPlayer(BARBARIAN_PLAYER))/2);
			}
			if (iNeededAnimals > 0)
			{
/************************************************************************************************/
/* Afforess	                  Start		 03/15/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
//				iNeededAnimals = ((iNeededAnimals / 5) + 1);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
				for (iI = 0; iI < iNeededAnimals; iI++)
				{
					pPlot = GC.getMapINLINE().syncRandPlot((RANDPLOT_NOT_VISIBLE_TO_CIV | RANDPLOT_PASSIBLE), pLoopArea->getID(), GC.getDefineINT("MIN_ANIMAL_STARTING_DISTANCE"));

					if (pPlot != NULL)
					{
						eBestUnit = NO_UNIT;
						iBestValue = 0;

						for (iJ = 0; iJ < GC.getNumUnitClassInfos(); iJ++)
						{
							eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(GET_PLAYER(BARBARIAN_PLAYER).getCivilizationType()).getCivilizationUnits(iJ)));

							if (eLoopUnit != NO_UNIT)
							{
								if (GC.getUnitInfo(eLoopUnit).getUnitAIType(UNITAI_ANIMAL))
								{
									if ((pPlot->getFeatureType() != NO_FEATURE) ? GC.getUnitInfo(eLoopUnit).getFeatureNative(pPlot->getFeatureType()) : GC.getUnitInfo(eLoopUnit).getTerrainNative(pPlot->getTerrainType()))
									{
										iValue = (1 + getSorenRandNum(1000, "Animal Unit Selection"));

										if (iValue > iBestValue)
										{
											eBestUnit = eLoopUnit;
											iBestValue = iValue;
										}
									}
								}
							}
						}

						if (eBestUnit != NO_UNIT)
						{
							GET_PLAYER(BARBARIAN_PLAYER).initUnit(eBestUnit, pPlot->getX_INLINE(), pPlot->getY_INLINE(), UNITAI_ANIMAL, NO_DIRECTION, GC.getGameINLINE().getSorenRandNum(10000, "AI Unit Birthmark 10"));
						}
					}
				}
			}
/************************************************************************************************/
/* Afforess                                     12/7/09                                         */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		//}
		// < Water Animals End   >
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	
	}
}


void CvGame::updateWar()
{
	PROFILE_FUNC();

	int iI, iJ;

	if (isOption(GAMEOPTION_ALWAYS_WAR))
	{
		for (iI = 0; iI < MAX_TEAMS; iI++)
		{
			CvTeam& kTeam1 = GET_TEAM((TeamTypes)iI);
			if (kTeam1.isAlive() && kTeam1.isHuman())
			{
				for (iJ = 0; iJ < MAX_TEAMS; iJ++)
				{
					CvTeam& kTeam2 = GET_TEAM((TeamTypes)iJ);
					if (kTeam2.isAlive() && !kTeam2.isHuman())
					{
						FAssert(iI != iJ);

						if (kTeam1.isHasMet((TeamTypes)iJ))
						{
							if (!kTeam1.isAtWar((TeamTypes)iJ))
							{
								kTeam1.declareWar(((TeamTypes)iJ), false, NO_WARPLAN);
							}
						}
					}
				}
			}
		}
	}
}


void CvGame::updateMoves()
{
	PROFILE_FUNC();

	MEMORY_TRACE_FUNCTION();

	CvSelectionGroup* pLoopSelectionGroup;
	int aiShuffle[MAX_PLAYERS];
	int iLoop = 0;
	int iI;

	if (isMPOption(MPOPTION_SIMULTANEOUS_TURNS))
	{
		shuffleArray(aiShuffle, MAX_PLAYERS, getSorenRand());
	}
	else
	{
		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			aiShuffle[iI] = iI;
		}
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& player = GET_PLAYER((PlayerTypes)(aiShuffle[iI]));

		if (player.isAlive())
		{
			if (player.isTurnActive())
			{
				//	For the human we want auto-moves last so the player can change
				//	orders on their automated units before hitting enter, but for the AI
				//	it is desirable to start with automated units in case the automation is broken
				//	be events, so that we know this before all other units process
				if ( player.isHuman() )
				{
					PROFILE("CvGame::updateMoves.Human");

					if (player.isAutoMoves())
					{
						if ( player.hasReadyUnautomatedUnit(false) )
						{
							//	Pre-emptorary turn end so make sure we run the unit AIs now
							player.AI_unitUpdate();
						}

						for(pLoopSelectionGroup = player.firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = player.nextSelectionGroup(&iLoop))
						{
							pLoopSelectionGroup->autoMission();
						}

						if (!(player.hasBusyUnit()))
						{
							player.setAutoMoves(false);
						}
					}
					else
					{
						if ( !player.hasReadyUnautomatedUnit(false) )
						{
							player.AI_unitUpdate();
						}

						//	A unit ready to move at this point is one the player needs to interact with
						if ( player.hasReadyUnautomatedUnit(true) )
						{
							player.setTurnHadUIInteraction(true);
						}
					}
				}
				else
				{
					PROFILE("CvGame::updateMoves.AI");

					CvPathGenerator::EnableMaxPerformance(true);

					//	Always try to do automations first for the AI
					for(pLoopSelectionGroup = player.firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = player.nextSelectionGroup(&iLoop))
					{
						pLoopSelectionGroup->autoMission();
					}

					CvPathGenerator::EnableMaxPerformance(false);

					if (!(player.isAutoMoves()))
					{
						player.AI_unitUpdate();

						if (!(player.hasBusyUnit()) && !(player.hasReadyUnit(true)))
						{
							player.setAutoMoves(true);
						}
					}
					else
					{
						if (!(player.hasBusyUnit()))
						{
							player.setAutoMoves(false);
						}
					}
				}
			}
		}
	}
}


void CvGame::verifyCivics()
{
	PROFILE_FUNC();

	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			GET_PLAYER((PlayerTypes)iI).verifyCivics();
		}
	}
}


void CvGame::updateTimers()
{
	PROFILE_FUNC();

	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			GET_PLAYER((PlayerTypes)iI).updateTimers();
		}
	}
}


void CvGame::updateTurnTimer()
{
	PROFILE_FUNC();

	int iI;

	// Are we using a turn timer?
	if (isMPOption(MPOPTION_TURN_TIMER))
	{
		if (getElapsedGameTurns() > 0 || !isOption(GAMEOPTION_ADVANCED_START))
		{
			// Has the turn expired?
			if (getTurnSlice() > getCutoffSlice())
			{
				for (iI = 0; iI < MAX_PLAYERS; iI++)
				{
					if (GET_PLAYER((PlayerTypes)iI).isAlive() && GET_PLAYER((PlayerTypes)iI).isTurnActive())
					{
						GET_PLAYER((PlayerTypes)iI).setEndTurn(true);

						if (!isMPOption(MPOPTION_SIMULTANEOUS_TURNS) && !isSimultaneousTeamTurns())
						{
							break;
						}
					}
				}
			}
		}
	}
}


void CvGame::testAlive()
{
	PROFILE_FUNC();

	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		GET_PLAYER((PlayerTypes)iI).verifyAlive();
	}
}
/************************************************************************************************/
/* Afforess                                     12/7/09                                         */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
bool CvGame::testVictory(VictoryTypes eVictory, TeamTypes eTeam, bool* pbEndScore)// removed const
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	
{
	PROFILE_FUNC();

	FAssert(eVictory >= 0 && eVictory < GC.getNumVictoryInfos());
	FAssert(eTeam >=0 && eTeam < MAX_CIV_TEAMS);
	FAssert(GET_TEAM(eTeam).isAlive());

	bool bValid = isVictoryValid(eVictory);
	if (pbEndScore)
	{
		*pbEndScore = false;
	}

	if (bValid)
	{
		if (GC.getVictoryInfo(eVictory).isEndScore())
		{
			if (pbEndScore)
			{
				*pbEndScore = true;
			}

			if (getMaxTurns() == 0)
			{
				bValid = false;
			}
			else if (getElapsedGameTurns() < getMaxTurns())
			{
				bValid = false;
			}
			else
			{
				bool bFound = false;

				for (int iK = 0; iK < MAX_CIV_TEAMS; iK++)
				{
					if (GET_TEAM((TeamTypes)iK).isAlive())
					{
						if (iK != eTeam)
						{
							if (getTeamScore((TeamTypes)iK) >= getTeamScore(eTeam))
							{
								bFound = true;
								break;
							}
						}
					}
				}

				if (bFound)
				{
					bValid = false;
				}
			}
		}
	}

	if (bValid)
	{
		if (GC.getVictoryInfo(eVictory).isTargetScore())
		{
			if (getTargetScore() == 0)
			{
				bValid = false;
			}
			else if (getTeamScore(eTeam) < getTargetScore())
			{
				bValid = false;
			}
			else
			{
				bool bFound = false;

				for (int iK = 0; iK < MAX_CIV_TEAMS; iK++)
				{
					if (GET_TEAM((TeamTypes)iK).isAlive())
					{
						if (iK != eTeam)
						{
							if (getTeamScore((TeamTypes)iK) >= getTeamScore(eTeam))
							{
								bFound = true;
								break;
							}
						}
					}
				}

				if (bFound)
				{
					bValid = false;
				}
			}
		}
	}

	if (bValid)
	{
		if (GC.getVictoryInfo(eVictory).isConquest())
		{
			if (GET_TEAM(eTeam).getNumCities() == 0)
			{
				bValid = false;
			}
			else
			{
				bool bFound = false;

				for (int iK = 0; iK < MAX_CIV_TEAMS; iK++)
				{
					if (GET_TEAM((TeamTypes)iK).isAlive())
					{
						if (iK != eTeam && !GET_TEAM((TeamTypes)iK).isVassal(eTeam))
						{
							if (GET_TEAM((TeamTypes)iK).getNumCities() > 0)
							{
								bFound = true;
								break;
							}
						}
					}
				}

				if (bFound)
				{
					bValid = false;
				}
			}
		}
	}

	if (bValid)
	{
		if (GC.getVictoryInfo(eVictory).isDiploVote())
		{
			bool bFound = false;

			for (int iK = 0; iK < GC.getNumVoteInfos(); iK++)
			{
				if (GC.getVoteInfo((VoteTypes)iK).isVictory())
				{
					if (getVoteOutcome((VoteTypes)iK) == eTeam)
					{
						bFound = true;
						break;
					}
				}
			}

			if (!bFound)
			{
				bValid = false;
			}
		}
	}

	if (bValid)
	{
		if (getAdjustedPopulationPercent(eVictory) > 0)
		{
			if (100 * GET_TEAM(eTeam).getTotalPopulation() < getTotalPopulation() * getAdjustedPopulationPercent(eVictory))
			{
				bValid = false;
			}
		}
	}

	if (bValid)
	{
		if (getAdjustedLandPercent(eVictory) > 0)
		{
			if (100 * GET_TEAM(eTeam).getTotalLand() < GC.getMapINLINE().getLandPlots() * getAdjustedLandPercent(eVictory))
			{
				bValid = false;
			}
		}
	}

	if (bValid)
	{
		if (GC.getVictoryInfo(eVictory).getReligionPercent() > 0)
		{
			bool bFound = false;

			if (getNumCivCities() > (countCivPlayersAlive() * 2))
			{
				for (int iK = 0; iK < GC.getNumReligionInfos(); iK++)
				{
					if (GET_TEAM(eTeam).hasHolyCity((ReligionTypes)iK))
					{
						if (calculateReligionPercent((ReligionTypes)iK) >= GC.getVictoryInfo(eVictory).getReligionPercent())
						{
							bFound = true;
							break;
						}
					}

					if (bFound)
					{
						break;
					}
				}
			}

			if (!bFound)
			{
				bValid = false;
			}
		}
	}

	if (bValid)
	{
		if ((GC.getVictoryInfo(eVictory).getCityCulture() != NO_CULTURELEVEL) && (GC.getVictoryInfo(eVictory).getNumCultureCities() > 0))
		{
			int iCount = 0;

			for (int iK = 0; iK < MAX_CIV_PLAYERS; iK++)
			{
				if (GET_PLAYER((PlayerTypes)iK).isAlive())
				{
					if (GET_PLAYER((PlayerTypes)iK).getTeam() == eTeam)
					{
						int iLoop = 0;
						for (CvCity* pLoopCity = GET_PLAYER((PlayerTypes)iK).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iK).nextCity(&iLoop))
						{
							if (pLoopCity->getCultureLevel() >= GC.getVictoryInfo(eVictory).getCityCulture())
							{
								iCount++;
							}
						}
					}
				}
			}

			if (iCount < GC.getVictoryInfo(eVictory).getNumCultureCities())
			{
				bValid = false;
			}
		}
	}

	if (bValid)
	{
		if (GC.getVictoryInfo(eVictory).getTotalCultureRatio() > 0)
		{
			unsigned long long iThreshold;
			iThreshold = ((GET_TEAM(eTeam).countTotalCulture() * 100) / GC.getVictoryInfo(eVictory).getTotalCultureRatio());

			bool bFound = false;

			for (int iK = 0; iK < MAX_CIV_TEAMS; iK++)
			{
				if (GET_TEAM((TeamTypes)iK).isAlive())
				{
					if (iK != eTeam)
					{
						if (GET_TEAM((TeamTypes)iK).countTotalCulture() > iThreshold)
						{
							bFound = true;
							break;
						}
					}
				}
			}

			if (bFound)
			{
				bValid = false;
			}
		}
	}

	if (bValid)
	{
		int qNeeded;
#if defined QC_MASTERY_VICTORY
		bool bAGBuilt = false; // code added for Ascension Gate & Mastery Victory (by 45°)
#endif		
		for (int iK = 0; iK < GC.getNumBuildingClassInfos(); iK++)
		{
			qNeeded = GC.getBuildingClassInfo((BuildingClassTypes) iK).getVictoryThreshold(eVictory);
			if (qNeeded > GET_TEAM(eTeam).getBuildingClassCount((BuildingClassTypes)iK))
			{
				bValid = false;
				break;
			}
#if defined QC_MASTERY_VICTORY
			else if ((qNeeded > 0) && AGBuilt != 0)
			{
				bAGBuilt = true;
			}
#endif
		}
#if defined QC_MASTERY_VICTORY //code added for Ascension Gate & Mastery Victory (by 45°)
//Sevo Begin--VCM
		if (bValid && bAGBuilt) 
		{
			AscensionGateBuilt[eTeam] = true;	  	
		}
//Sevo End VCM
#endif // 
	}

	if (bValid)
	{// God-Emperor: fix the starshipLaunched setting by moving it in here and checking more carefully
		int iNeeded;
#if defined QC_MASTERY_VICTORY
		bool bSSParts = false; // God-Emperor: not just any projects, the spaceship parts projects
#endif
		for (int iK = 0; iK < GC.getNumProjectInfos(); iK++)
		{
			iNeeded = GC.getProjectInfo((ProjectTypes) iK).getVictoryThreshold(eVictory);
			if ( iNeeded > GET_TEAM(eTeam).getProjectCount((ProjectTypes)iK))
			{
				bValid = false;
				break;
			}
#if defined QC_MASTERY_VICTORY
			else if ((iNeeded > 0) && GC.getProjectInfo((ProjectTypes) iK).isSpaceship())
			{// God-Emperor: since only one of the two needs to be false to skip setting starshipLaunched this can be an "else if" rather than an "if".
				bSSParts = true;
			}
#endif
		}		
#if defined QC_MASTERY_VICTORY
//Sevo Begin--VCM
		if (bValid && bSSParts) // God-Emperor: do this here. If both are true there are enough SS parts done to launch.
		{
			starshipLaunched[eTeam] = true;	  	
		}
//Sevo End VCM
#endif
	}
	return bValid;
}

void CvGame::testVictory()
{
	PROFILE_FUNC();

	MEMORY_TRACE_FUNCTION();

	bool	bEndScore = false;
	int		iI, iJ;

	if (getVictory() != NO_VICTORY)
	{
		return;
	}

	if (getGameState() == GAMESTATE_EXTENDED)
	{
		return;
	}

	updateScore();

	long lResult = 1;
	PYTHON_CALL_FUNCTION4(__FUNCTION__, PYGameModule, "isVictoryTest", NULL, &lResult);
	if (lResult == 0)
	{
		return;
	}

	std::vector<std::vector<int> > aaiWinners;

	for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		CvTeam& kLoopTeam = GET_TEAM((TeamTypes)iI);
		if (kLoopTeam.isAlive())
		{
			if (!(kLoopTeam.isMinorCiv()))
			{
				for (int iJ = 0; iJ < GC.getNumVictoryInfos(); iJ++)
				{
					if (testVictory((VictoryTypes)iJ, (TeamTypes)iI, &bEndScore))
					{
						if (kLoopTeam.getVictoryCountdown((VictoryTypes)iJ) < 0)
						{
							if (kLoopTeam.getVictoryDelay((VictoryTypes)iJ) == 0)
							{
								kLoopTeam.setVictoryCountdown((VictoryTypes)iJ, 0);
							}
						}

						//update victory countdown
						if (kLoopTeam.getVictoryCountdown((VictoryTypes)iJ) > 0)
						{
							kLoopTeam.changeVictoryCountdown((VictoryTypes)iJ, -1);
						}

						if (kLoopTeam.getVictoryCountdown((VictoryTypes)iJ) == 0)
						{
							if (getSorenRandNum(100, "Victory Success") < kLoopTeam.getLaunchSuccessRate((VictoryTypes)iJ))
							{
								std::vector<int> aWinner;
								aWinner.push_back(iI);
								aWinner.push_back(iJ);
								aaiWinners.push_back(aWinner);
							}
							else
							{
								kLoopTeam.resetVictoryProgress();
							}
						}
					}
				}
			}
		}

	}

/************************************************************************************************/
/* Afforess                                     12/7/09                                         */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	bool bForceEndGame = false;
	for (iJ = 0; iJ < GC.getNumVictoryInfos(); iJ++)
	{
		if (GC.getVictoryInfo((VictoryTypes)iJ).isTotalVictory())
		{
			if (isVictoryValid((VictoryTypes)iJ))
			{
				aaiWinners.clear(); // sorry, folks, no winners today by usual means, only Mastery Victory is achievable;)			
				if (getMaxTurns() == 0)
				{
					break;
				}//There's more game yet to play.	
				if (getElapsedGameTurns() < getMaxTurns())
				{
					if (isModderGameOption(MODDERGAMEOPTION_MERCY_RULE))
					{
						int iTotalScore = 0;
						int iTopScore = 0;
						int iTempScore = 0;

						TeamTypes eBestTeam = (TeamTypes)0;
						for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
						{
							if (GET_TEAM((TeamTypes)iI).isAlive())
							{
								if (!(GET_TEAM((TeamTypes)iI).isMinorCiv()) || GC.getGameINLINE().isOption(GAMEOPTION_START_AS_MINORS))
								{
									iTempScore = GET_TEAM((TeamTypes)iI).getTotalVictoryScore();
									iTotalScore += iTempScore;
									if (iTempScore > iTopScore)
									{
										iTopScore = iTempScore;
										eBestTeam = (TeamTypes)iI;
									}
								}
							}
						}
						//Remove the top player
						iTotalScore -= iTopScore;
						//One player is dominating! Mercy Rule!
						if (iTopScore > iTotalScore)
						{
							if (getMercyRuleCounter() == 0)
							{
								//Ten Turns remain!
								int iTurns = 10;
								iTurns *= GC.getGameSpeedInfo(getGameSpeedType()).getVictoryDelayPercent();
								iTurns /= 100;
								setMercyRuleCounter(iTurns);
								//Inform Players
								for (iI = 0; iI < MAX_PLAYERS; iI++)
								{
									if (GET_PLAYER((PlayerTypes)iI).isAlive())
									{
										if (GET_PLAYER((PlayerTypes)iI).isHuman())
										{
											MEMORY_TRACK_EXEMPT();

											if (GET_PLAYER((PlayerTypes)iI).getTeam() == eBestTeam)
												AddDLLMessage((PlayerTypes)iI, true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_WINNER_MERCY_RULE_HAS_BEGUN", getMercyRuleCounter()).GetCString(), "AS2D_FEAT_ACCOMPLISHED", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"));
											else
												AddDLLMessage((PlayerTypes)iI, true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_LOSER_MERCY_RULE_HAS_BEGUN", getMercyRuleCounter()).GetCString(), "AS2D_FIRSTTOTECH", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"));
										}
									}
								}
							}
							else 
							{
								changeMercyRuleCounter(-1);
								//Times Up!
								if (getMercyRuleCounter() == 0)
								{
									bForceEndGame = true;
								}
								else
								{
									for (iI = 0; iI < MAX_PLAYERS; iI++)
									{
										if (GET_PLAYER((PlayerTypes)iI).isAlive())
										{
											if (GET_PLAYER((PlayerTypes)iI).isHuman())
											{
												MEMORY_TRACK_EXEMPT();

												if (GET_PLAYER((PlayerTypes)iI).getTeam() == eBestTeam)
													AddDLLMessage((PlayerTypes)iI, true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_WINNER_MERCY_RULE_TURNS", getMercyRuleCounter()).GetCString(), "AS2D_FEAT_ACCOMPLISHED", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"));
												else
													AddDLLMessage((PlayerTypes)iI, true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_LOSER_MERCY_RULE_TURNS", getMercyRuleCounter()).GetCString(), "AS2D_FIRSTTOTECH", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT"));
											}
										}
									}
								}
							}
						}
						//Abort any existing mercy countdown
						else
						{
							if (getMercyRuleCounter() > 0)
							{
								setMercyRuleCounter(0);
								for (iI = 0; iI < MAX_PLAYERS; iI++)
								{
									if (GET_PLAYER((PlayerTypes)iI).isAlive())
									{
										if (GET_PLAYER((PlayerTypes)iI).isHuman())
										{
											MEMORY_TRACK_EXEMPT();

											AddDLLMessage((PlayerTypes)iI, true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_MERCY_RULE_ABORTED").GetCString(), "AS2D_FEAT_ACCOMPLISHED", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"));
										}
									}
								}
							}
						}
					}
					else
						break;
				}
				if (getElapsedGameTurns() >= getMaxTurns() || bForceEndGame)
				{
				  int score, topScore = 0;
					// End of game and Total Victory is selected.  Calculate the topscore
					for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
					{
						if (GET_TEAM((TeamTypes)iI).isAlive())
						{
							if (!(GET_TEAM((TeamTypes)iI).isMinorCiv()))
							{
								score = GET_TEAM((TeamTypes)iI).getTotalVictoryScore();
								if (score > topScore)
								{
									topScore = score;
								}
							}
						}
					}				
					for (iI = 0; iI < MAX_CIV_TEAMS; iI++) // have to do again because of ties, determine winners
					{
						if (GET_TEAM((TeamTypes)iI).isAlive())
						{
							if (!(GET_TEAM((TeamTypes)iI).isMinorCiv()))
							{
								score = GET_TEAM((TeamTypes)iI).getTotalVictoryScore();
								if (score >= topScore)
								{
									std::vector<int> aWinner;
									aWinner.push_back(iI);
									aWinner.push_back(iJ);
									aaiWinners.push_back(aWinner);
								}
							}
						}
					}					
				}				
			}
		}
	}
	if (isOption(GAMEOPTION_UNITED_NATIONS) && !m_bDiploVictoryEnabled)
	{
		//Find the diplomatic victory
		BuildingTypes eUnitedNations = NO_BUILDING;
		for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
		{
			if (GC.getBuildingInfo((BuildingTypes)iI).getVictoryPrereq() != NO_VICTORY)
			{
				if (GC.getBuildingInfo((BuildingTypes)iI).getVoteSourceType() != NO_VOTESOURCE)
				{
					eUnitedNations = (BuildingTypes)iI;
					break;
				}
			}
		}
		if (eUnitedNations != NO_BUILDING)
		{
			if (getVictory() == GC.getBuildingInfo(eUnitedNations).getVictoryPrereq())
			{
				aaiWinners.clear();
			}
		}
	}
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	
	if (!aaiWinners.empty())
	{
		int iWinner = getSorenRandNum(aaiWinners.size(), "Victory tie breaker");
		setWinner(((TeamTypes)aaiWinners[iWinner][0]), ((VictoryTypes)aaiWinners[iWinner][1]));
	}

	if (getVictory() == NO_VICTORY)
	{
		if (getMaxTurns() > 0)
		{
			if (getElapsedGameTurns() >= getMaxTurns())
			{
				if (!bEndScore)
				{
/************************************************************************************************/
/* REVOLUTION_MOD                                                                 lemmy101      */
/*                                                                                jdog5000      */
/*                                                                                              */
/************************************************************************************************/
					if ((getAIAutoPlay(getActivePlayer()) > 0) || gDLL->GetAutorun())
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/

					{
						setGameState(GAMESTATE_EXTENDED);
					}
					else
					{
						setGameState(GAMESTATE_OVER);
					}
				}
			}
		}
	}
}


void CvGame::processVote(const VoteTriggeredData& kData, int iChange)
{
	PROFILE_FUNC();

	CvVoteInfo& kVote = GC.getVoteInfo(kData.kVoteOption.eVote);

	changeTradeRoutes(kVote.getTradeRoutes() * iChange);
	changeFreeTradeCount(kVote.isFreeTrade() ? iChange : 0);
	changeNoNukesCount(kVote.isNoNukes() ? iChange : 0);

	for (int iI = 0; iI < GC.getNumCivicInfos(); iI++)
	{
		changeForceCivicCount((CivicTypes)iI, kVote.isForceCivic(iI) ? iChange : 0);
	}


	if (iChange > 0)
	{
		if (kVote.isOpenBorders())
		{
			for (int iTeam1 = 0; iTeam1 < MAX_CIV_PLAYERS; ++iTeam1)
			{
				CvTeam& kLoopTeam1 = GET_TEAM((TeamTypes)iTeam1);
				if (kLoopTeam1.isVotingMember(kData.eVoteSource))
				{
					for (int iTeam2 = iTeam1 + 1; iTeam2 < MAX_CIV_PLAYERS; ++iTeam2)
					{
						CvTeam& kLoopTeam2 = GET_TEAM((TeamTypes)iTeam2);
						if (kLoopTeam2.isVotingMember(kData.eVoteSource))
						{
							kLoopTeam1.signOpenBorders((TeamTypes)iTeam2);
						}
					}
				}
			}

			setVoteOutcome(kData, NO_PLAYER_VOTE);
		}
		else if (kVote.isDefensivePact())
		{
			for (int iTeam1 = 0; iTeam1 < MAX_CIV_PLAYERS; ++iTeam1)
			{
				CvTeam& kLoopTeam1 = GET_TEAM((TeamTypes)iTeam1);
				if (kLoopTeam1.isVotingMember(kData.eVoteSource))
				{
					for (int iTeam2 = iTeam1 + 1; iTeam2 < MAX_CIV_PLAYERS; ++iTeam2)
					{
						CvTeam& kLoopTeam2 = GET_TEAM((TeamTypes)iTeam2);
						if (kLoopTeam2.isVotingMember(kData.eVoteSource))
						{
							kLoopTeam1.signDefensivePact((TeamTypes)iTeam2);
						}
					}
				}
			}

			setVoteOutcome(kData, NO_PLAYER_VOTE);
		}
		else if (kVote.isForcePeace())
		{
			FAssert(NO_PLAYER != kData.kVoteOption.ePlayer);
			CvPlayer& kPlayer = GET_PLAYER(kData.kVoteOption.ePlayer);

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/02/09                                jdog5000      */
/*                                                                                              */
/* AI logging                                                                                   */
/************************************************************************************************/
			if( gTeamLogLevel >= 1 )
			{
				logBBAIForTeam(kPlayer.getTeam(), "  Vote for forcing peace against team %d (%S) passes", kPlayer.getTeam(), kPlayer.getCivilizationDescription(0) );
			}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

			for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
			{
				CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
				if (kLoopPlayer.isAlive() && kLoopPlayer.getTeam() != kPlayer.getTeam())
				{
					if (kLoopPlayer.isVotingMember(kData.eVoteSource))
					{
						kLoopPlayer.forcePeace(kData.kVoteOption.ePlayer);
					}
				}
			}

			setVoteOutcome(kData, NO_PLAYER_VOTE);
		}
		else if (kVote.isForceNoTrade())
		{
			FAssert(NO_PLAYER != kData.kVoteOption.ePlayer);
			CvPlayer& kPlayer = GET_PLAYER(kData.kVoteOption.ePlayer);

			for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
			{
				CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
				if (kLoopPlayer.isAlive() && kLoopPlayer.isVotingMember(kData.eVoteSource))
				{
					if (kLoopPlayer.canStopTradingWithTeam(kPlayer.getTeam()))
					{
						kLoopPlayer.stopTradingWithTeam(kPlayer.getTeam());
					}
				}
			}

			setVoteOutcome(kData, NO_PLAYER_VOTE);
		}
		else if (kVote.isForceWar())
		{
			FAssert(NO_PLAYER != kData.kVoteOption.ePlayer);
			CvPlayer& kPlayer = GET_PLAYER(kData.kVoteOption.ePlayer);

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/02/09                                jdog5000      */
/*                                                                                              */
/* AI logging                                                                                   */
/************************************************************************************************/
			if( gTeamLogLevel >= 1 )
			{
				logBBAIForTeam(kPlayer.getTeam(), "  Vote for war against team %d (%S) passes", kPlayer.getTeam(), kPlayer.getCivilizationDescription(0) );
			}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

			for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
			{
				CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
				if (kLoopPlayer.isAlive() && kLoopPlayer.isVotingMember(kData.eVoteSource))
				{
					if (GET_TEAM(kLoopPlayer.getTeam()).canChangeWarPeace(kPlayer.getTeam()))
					{
						GET_TEAM(kLoopPlayer.getTeam()).declareWar(kPlayer.getTeam(), false, WARPLAN_DOGPILE);
					}
				}
			}

			setVoteOutcome(kData, NO_PLAYER_VOTE);
		}
		else if (kVote.isAssignCity())
		{
			FAssert(NO_PLAYER != kData.kVoteOption.ePlayer);
			CvPlayer& kPlayer = GET_PLAYER(kData.kVoteOption.ePlayer);
			CvCity* pCity = kPlayer.getCity(kData.kVoteOption.iCityId);
			FAssert(NULL != pCity);

			if (NULL != pCity)
			{
				if (NO_PLAYER != kData.kVoteOption.eOtherPlayer && kData.kVoteOption.eOtherPlayer != pCity->getOwnerINLINE())
				{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/02/09                                jdog5000      */
/*                                                                                              */
/* AI logging                                                                                   */
/************************************************************************************************/
					if( gTeamLogLevel >= 1 )
					{
						logBBAI("  Vote for assigning %S to %d (%S) passes", pCity->getName().GetCString(), GET_PLAYER(kData.kVoteOption.eOtherPlayer).getTeam(), GET_PLAYER(kData.kVoteOption.eOtherPlayer).getCivilizationDescription(0) );
					}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
					GET_PLAYER(kData.kVoteOption.eOtherPlayer).acquireCity(pCity, false, true, true);
				}
			}

			setVoteOutcome(kData, NO_PLAYER_VOTE);
		}
	}
}


int CvGame::getIndexAfterLastDeal()
{
	return m_deals.getIndexAfterLast();
}


int CvGame::getNumDeals()
{
	return m_deals.getCount();
}


 CvDeal* CvGame::getDeal(int iID)																		
{
	return ((CvDeal *)(m_deals.getAt(iID)));
}


CvDeal* CvGame::addDeal()
{
	return ((CvDeal *)(m_deals.add()));
}


 void CvGame::deleteDeal(int iID)
{
	m_deals.removeAt(iID);
	gDLL->getInterfaceIFace()->setDirty(Foreign_Screen_DIRTY_BIT, true);
}

CvDeal* CvGame::firstDeal(int *pIterIdx, bool bRev)
{
	return !bRev ? m_deals.beginIter(pIterIdx) : m_deals.endIter(pIterIdx);
}


CvDeal* CvGame::nextDeal(int *pIterIdx, bool bRev)
{
	return !bRev ? m_deals.nextIter(pIterIdx) : m_deals.prevIter(pIterIdx);
}


 CvRandom& CvGame::getMapRand()																					
{
	return m_mapRand;
}


int CvGame::getMapRandNum(int iNum, const char* pszLog)
{
	return m_mapRand.get(iNum, pszLog);
}


CvRandom& CvGame::getSorenRand()																					
{
	return m_sorenRand;
}

void CvGame::logRandomResult(const wchar* szStreamName, const char* pszLog, int iMax, int iResult)
{
	if (GC.isXMLLogging() || isNetworkMultiPlayer())
	{
		if (isFinalInitialized())
		{
			static CRITICAL_SECTION cLogSection;
			static bool				bLogSectionInitialized = false;
			static int				iLine = 0;
			TCHAR szFile[1024];
			sprintf(szFile, "RandomLogger - Player %d - Set %d.log", GC.getGameINLINE().getActivePlayer(), getGameTurn()/50);
			TCHAR szOut[1024];

			if ( !bLogSectionInitialized )
			{
				InitializeCriticalSectionAndSpinCount(&cLogSection, 4000);
				bLogSectionInitialized = true;

				sprintf(szOut, "Line\tTurn\tStream\tReason\tMax\tResult\n");
				gDLL->logMsg(szFile,szOut, false, false);
			}

			EnterCriticalSection(&cLogSection);
			sprintf(szOut, "%d\t%d\t%S\t%s\t%d\t%d\n", ++iLine, getGameTurn()+1, szStreamName, pszLog, iMax, iResult);
			gDLL->logMsg(szFile,szOut, false, false);
			LeaveCriticalSection(&cLogSection);
		}
	}
}

int CvGame::getSorenRandNum(int iNum, const char* pszLog)
{
/*************************************************************************************************/
/**	Xienwolf Tweak							12/10/08											**/
/**																								**/
/**					Reveals all Random checks if Debugging is enabled							**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
	return m_sorenRand.get(iNum, pszLog);
/**								----  End Original Code  ----									**/
	int iScale = 0;
	while(iNum > MAX_UNSIGNED_SHORT)
	{
		iNum /= 2;
		iScale++;
	}

    int Result = m_sorenRand.get(iNum, pszLog);

	while(iScale-- > 0)
	{
		Result *= 2;
	}

	logRandomResult(L"Global", pszLog, iNum, Result);

	return Result;
/*************************************************************************************************/
/**	Tweak									END													**/
/*************************************************************************************************/
}

bool CvGame::isMultiplayerHost()
{
	if (isNetworkMultiPlayer())
	{
		PlayerTypes eHostPlayer = NO_PLAYER;
		if (!isPitboss())
		{
			for (int i = 0; i < MAX_PLAYERS; i++)
			{
				if (GET_PLAYER((PlayerTypes)i).isAlive() && GET_PLAYER((PlayerTypes)i).isHuman())
				{
					eHostPlayer = (PlayerTypes)i;
					break;
				}
			}
			if (eHostPlayer == NO_PLAYER)
			{
				return false; //XXX never should happen?
			}
		}

		//We can send the OOS player our data to attempt to resync
		if ((isPitboss() && gDLL->IsPitbossHost()) || eHostPlayer == getActivePlayer())
		{
			return true;
		}
	}
	return false;
}

#define MAX_RESYNC_PACKET_LEN 1250
void CvGame::checkOutOfSyncPlayers()
{
	int iOurSync = gDLL->GetOptionsOOS(getActivePlayer());
	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; iPlayer++)
	{
		if (iPlayer != getActivePlayer() && GET_PLAYER((PlayerTypes)iPlayer).isHuman() && GET_PLAYER((PlayerTypes)iPlayer).isAlive())
		{
			int iNetID = GET_PLAYER((PlayerTypes)iPlayer).getNetID();
			if (gDLL->isConnected(iNetID))
			{
				int iTheirSync = gDLL->GetSyncOOS(iNetID);

				int iResyncDelay = (100 + getElapsedGameTurns()) * (GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getGridHeight() * GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getGridWidth());
				iResyncDelay /= 100;
				iResyncDelay = std::max(iResyncDelay, 100);
				if (iOurSync != iTheirSync && m_iActiveTurnSlice + iResyncDelay > m_iLastResyncTurnSlice)
				{
					m_iLastResyncTurnSlice = m_iActiveTurnSlice;

					ByteBuffer* pBuffer = new ByteBuffer();
					resync(true, pBuffer);
					GC.getMapINLINE().resync(true, pBuffer);
					for (int iI = 0; iI < MAX_PLAYERS; iI++)
					{
						GET_PLAYER((PlayerTypes)iI).resync(true, pBuffer);
					}
					for (int iI = 0; iI < MAX_TEAMS; iI++)
					{
						GET_TEAM((TeamTypes)iI).resync(true, pBuffer);
					}

					byte* compressed = new byte[pBuffer->size()];
					for (int i = 0; i < (int)pBuffer->size(); i++)
					{
						compressed[i] = pBuffer->get(i);
					}
					unsigned long iSize = pBuffer->size();
					unsigned long iOriginalSize = iSize;
					gDLL->Compress(&compressed, &iSize);

					int iNumPackets = (int)(iSize / MAX_RESYNC_PACKET_LEN);
					if (iNumPackets * MAX_RESYNC_PACKET_LEN < (int)iSize) iNumPackets++;

					for (int i = 0; i < iNumPackets; i++)
					{
						int iLen = std::min(MAX_RESYNC_PACKET_LEN, (int)(iSize - (i * MAX_RESYNC_PACKET_LEN)));
						byte* buffer = new byte[iLen];
						for (int j = 0; j < iLen; j++)
						{
							int iPos = (i * MAX_RESYNC_PACKET_LEN) + j;
							buffer[j] = compressed[iPos];
						}

						CvGameResyncPacket* pResyncPacket = new CvGameResyncPacket(buffer, iLen, i, iNumPackets, (int)iOriginalSize);
						m_resyncPackets.push_back(pResyncPacket);
					}

					SAFE_DELETE(pBuffer);
					SAFE_DELETE_ARRAY(compressed);
				}
			}
		}
	}
}


void CvGame::handleResyncPacket(byte* pBuffer, int iBufferLen, int iPacketNum, int iNumPackets, int iOriginalBufferLen)
{
	if (isMultiplayerHost())
	{
		return;
	}
	logDebugMsg("Receiving game resync packet (%d of %d packets total), packet size: %d [orig size: %d]", iPacketNum, iNumPackets, iBufferLen, iOriginalBufferLen);

	//We are creating a copy of the resync packet sent because we don't know if the exe will de-allocate the one they sent us
	byte* copyBuffer = new byte[iBufferLen];
	for (int i = 0; i < iBufferLen; i++)
	{
		copyBuffer[i] = pBuffer[i];
	}
	CvGameResyncPacket* pResyncPacket = new CvGameResyncPacket(copyBuffer, iBufferLen, iPacketNum, iNumPackets, iOriginalBufferLen);
	m_resyncReceiveBuffer[iPacketNum] = pResyncPacket;

	if (iNumPackets == m_resyncReceiveBuffer.size())
	{
		logDebugMsg("Received final game resync packet");
		m_bOutOfSync = false;

		ByteBuffer buffer;
		for (int i = 0; i < iNumPackets; i++)
		{
			stdext::hash_map<int, CvGameResyncPacket*>::iterator search = m_resyncReceiveBuffer.find(i);
			if (search != m_resyncReceiveBuffer.end())
			{
				CvGameResyncPacket* packet = search->second;
				for (int j = 0; j < packet->m_iBufferLen; j++)
				{
					buffer.put(packet->m_buffer[j]);
				}

				SAFE_DELETE(packet);
			}
		}

		m_resyncReceiveBuffer.clear();

		byte* uncompressed = new byte[iOriginalBufferLen];
		for (int i = 0; i < (int)buffer.size(); i++)
		{
			uncompressed[i] = buffer.get(i);
		}
		unsigned long iBufferSize = buffer.size();
		gDLL->Uncompress(&uncompressed, &iBufferSize, iOriginalBufferLen);
		buffer.clear();
		if (iBufferSize == iOriginalBufferLen)
		{
			//Request seed update
			//Note: this is a total hack
			//Part of the resync process includes updating the random seed
			//However, because it takes multiple turn slices to send over the entire game state
			//the seeds are out of date by the time they arrive
			//So instead we request the latest copy as soon as we begin resyncing
			//So they arrive immediately after the resync finishes
			gDLL->sendMessageData(new CvRandomSeedUpdate(1, 1));

			for (unsigned long i = 0; i < iBufferSize; i++)
			{
				buffer.put(uncompressed[i]);
			}
			GC.getGameINLINE().logDebugMsg("Total uncompressed size: %d (expected: %d) ", buffer.size(), iOriginalBufferLen);

			gDLL->getInterfaceIFace()->clearSelectionList();
			resync(false, &buffer);
			GC.getMapINLINE().resync(false, &buffer);
			for (int iI = 0; iI < MAX_PLAYERS; iI++)
			{
				GET_PLAYER((PlayerTypes)iI).resync(false, &buffer);
			}
			for (int iI = 0; iI < MAX_TEAMS; iI++)
			{
				GET_TEAM((TeamTypes)iI).resync(false, &buffer);
			}
			gDLL->getInterfaceIFace()->makeSelectionListDirty();
			GC.getCurrentViewport()->processActionState();
			noteGraphicRebuildNeeded();
			updateTestEndTurn();

			for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
			{
				GC.getMapINLINE().plotByIndexINLINE(iI)->enableCenterUnitRecalc(true);
			}
		}
		else
		{
			//Should not happen unless someone bungles the code
			FAssert(false);
		}

		logDebugMsg("Finished OOS Resync");
	}
}

int CvGame::calculateSyncChecksum()
{
	PROFILE_FUNC();

	CvUnit* pLoopUnit;
	int iMultiplier;
	int iValue;
	int iLoop = 0;
	int iI, iJ;

	iValue = 0;

	iValue += getMapRand().getSeed();
	iValue += getSorenRand().getSeed();

	iValue += getNumCities();
	iValue += getTotalPopulation();
	iValue += getNumDeals();

	iValue += GC.getMapINLINE().getOwnedPlots();
	iValue += GC.getMapINLINE().getNumAreas();

	int iNumPlayers = 0;
	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iNumPlayers++;
		}
	}
	iValue += iNumPlayers * 424242;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isEverAlive())
		{
			iMultiplier = getPlayerScore((PlayerTypes)iI);

			switch (getTurnSlice() % 4)
			{
			case 0:
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getTotalPopulation() * 543271);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getTotalLand() * 327382);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getGold() * 107564);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getAssets() * 327455);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getPower() * 135647);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getNumCities() * 436432);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getNumUnits() * 324111);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getNumSelectionGroups() * 215356);
				break;
				//8
			case 1:
				for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
				{
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).calculateTotalYield((YieldTypes)iJ) * 432754);
				}

				for (iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
				{
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).getCommerceRate((CommerceTypes)iJ) * 432789);
				}
				break;
				//7
			case 2:
				for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
				{
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).getNumAvailableBonuses((BonusTypes)iJ) * 945732);
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).getBonusImport((BonusTypes)iJ) * 326443);
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).getBonusExport((BonusTypes)iJ) * 932211);
				}

				for (iJ = 0; iJ < GC.getNumImprovementInfos(); iJ++)
				{
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).getImprovementCount((ImprovementTypes)iJ) * 883422);
				}

				for (iJ = 0; iJ < GC.getNumBuildingClassInfos(); iJ++)
				{
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).getBuildingClassCountPlusMaking((BuildingClassTypes)iJ) * 954531);
				}

				for (iJ = 0; iJ < GC.getNumUnitClassInfos(); iJ++)
				{
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).getUnitClassCountPlusMaking((UnitClassTypes)iJ) * 754843);
				}

				for (iJ = 0; iJ < NUM_UNITAI_TYPES; iJ++)
				{
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).AI_totalUnitAIs((UnitAITypes)iJ) * 643383);
				}
				break;

			case 3:
				for (pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
				{
					iMultiplier += (pLoopUnit->getX_INLINE() * 876543);
					iMultiplier += (pLoopUnit->getY_INLINE() * 985310);
					iMultiplier += (pLoopUnit->getDamage() * 736373);
					iMultiplier += (pLoopUnit->getExperience100() * 820622);
					iMultiplier += (pLoopUnit->getLevel() * 367291);
					PromotionIterator itEnd = pLoopUnit->getPromotionEnd();
					for (PromotionIterator it = pLoopUnit->getPromotionBegin(); it != itEnd; ++it)
					{
						if (it->second.m_bHasPromotion)
						{
							iMultiplier += ((int)it->first) * 572861;
						}
					}
				}
				break;
			}

			if (iMultiplier != 0)
			{
				iValue *= iMultiplier;
			}
		}
	}

	return iValue;
}


int CvGame::calculateOptionsChecksum()
{
	PROFILE_FUNC();

	int iValue;
	int iI, iJ;

	iValue = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		for (iJ = 0; iJ < NUM_PLAYEROPTION_TYPES; iJ++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isOption((PlayerOptionTypes)iJ))
			{
				iValue += (iI * 943097);
				iValue += (iJ * 281541);
			}
		}
	}

	return iValue;
}


void CvGame::addReplayMessage(ReplayMessageTypes eType, PlayerTypes ePlayer, CvWString pszText, int iPlotX, int iPlotY, ColorTypes eColor)
{
	int iGameTurn = getGameTurn();
	CvReplayMessage* pMessage = new CvReplayMessage(iGameTurn, eType, ePlayer);
	if (NULL != pMessage)
	{
		pMessage->setPlot(iPlotX, iPlotY);
		pMessage->setText(pszText);
		if (NO_COLOR == eColor)
		{
			eColor = (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE");
		}
		pMessage->setColor(eColor);
		m_listReplayMessages.push_back(pMessage);
	}
}

void CvGame::clearReplayMessageMap()
{
	for (ReplayMessageList::const_iterator itList = m_listReplayMessages.begin(); itList != m_listReplayMessages.end(); ++itList)
	{
		const CvReplayMessage* pMessage = *itList;
		if (NULL != pMessage)
		{
			delete pMessage;
		}
	}
	m_listReplayMessages.clear();
}

int CvGame::getReplayMessageTurn(uint i) const
{
	if (i >= m_listReplayMessages.size())
	{
		return (-1);
	}
	const CvReplayMessage* pMessage =  m_listReplayMessages[i];
	if (NULL == pMessage)
	{
		return (-1);
	}
	return pMessage->getTurn();
}

ReplayMessageTypes CvGame::getReplayMessageType(uint i) const
{
	if (i >= m_listReplayMessages.size())
	{
		return (NO_REPLAY_MESSAGE);
	}
	const CvReplayMessage* pMessage =  m_listReplayMessages[i];
	if (NULL == pMessage)
	{
		return (NO_REPLAY_MESSAGE);
	}
	return pMessage->getType();
}

int CvGame::getReplayMessagePlotX(uint i) const
{
	if (i >= m_listReplayMessages.size())
	{
		return (-1);
	}
	const CvReplayMessage* pMessage =  m_listReplayMessages[i];
	if (NULL == pMessage)
	{
		return (-1);
	}
	return pMessage->getPlotX();
}

int CvGame::getReplayMessagePlotY(uint i) const
{
	if (i >= m_listReplayMessages.size())
	{
		return (-1);
	}
	const CvReplayMessage* pMessage =  m_listReplayMessages[i];
	if (NULL == pMessage)
	{
		return (-1);
	}
	return pMessage->getPlotY();
}

PlayerTypes CvGame::getReplayMessagePlayer(uint i) const
{
	if (i >= m_listReplayMessages.size())
	{
		return (NO_PLAYER);
	}
	const CvReplayMessage* pMessage =  m_listReplayMessages[i];
	if (NULL == pMessage)
	{
		return (NO_PLAYER);
	}
	return pMessage->getPlayer();
}

LPCWSTR CvGame::getReplayMessageText(uint i) const
{
	if (i >= m_listReplayMessages.size())
	{
		return (NULL);
	}
	const CvReplayMessage* pMessage =  m_listReplayMessages[i];
	if (NULL == pMessage)
	{
		return (NULL);
	}
	return pMessage->getText().GetCString();
}

ColorTypes CvGame::getReplayMessageColor(uint i) const
{
	if (i >= m_listReplayMessages.size())
	{
		return (NO_COLOR);
	}
	const CvReplayMessage* pMessage =  m_listReplayMessages[i];
	if (NULL == pMessage)
	{
		return (NO_COLOR);
	}
	return pMessage->getColor();
}


uint CvGame::getNumReplayMessages() const
{
	return m_listReplayMessages.size();
}

// Private Functions...

void CvGame::read(FDataStreamBase* pStream)
{
	int iI;
	CvTaggedSaveFormatWrapper&	wrapper = CvTaggedSaveFormatWrapper::getSaveFormatWrapper();

	reset(NO_HANDICAP);

	wrapper.AttachToStream(pStream);

	WRAPPER_READ_OBJECT_START(wrapper);

	uint uiFlag=0;
	WRAPPER_READ(wrapper,"CvGame",&uiFlag);	// flags for expansion

	if ((uiFlag & GAME_SAVE_UI_FLAG_VALUE_AND_BASE) == 0)
	{
		int iEndTurnMessagesSent = 0;
		WRAPPER_READ(wrapper,"CvGame",&iEndTurnMessagesSent);
	}
	WRAPPER_READ(wrapper,"CvGame",&m_iElapsedGameTurns);
	WRAPPER_READ(wrapper,"CvGame",&m_iStartTurn);
	WRAPPER_READ(wrapper,"CvGame",&m_iStartYear);
	WRAPPER_READ(wrapper,"CvGame",&m_iEstimateEndTurn);
	WRAPPER_READ(wrapper,"CvGame",&m_iTurnSlice);
	WRAPPER_READ(wrapper,"CvGame",&m_iCutoffSlice);
	WRAPPER_READ(wrapper,"CvGame",&m_iNumGameTurnActive);
	WRAPPER_READ(wrapper,"CvGame",&m_iNumCities);
	WRAPPER_READ(wrapper,"CvGame",&m_iTotalPopulation);
	WRAPPER_READ(wrapper,"CvGame",&m_iTradeRoutes);
	WRAPPER_READ(wrapper,"CvGame",&m_iFreeTradeCount);
	WRAPPER_READ(wrapper,"CvGame",&m_iNoNukesCount);
	WRAPPER_READ(wrapper,"CvGame",&m_iNukesExploded);
	WRAPPER_READ(wrapper,"CvGame",&m_iMaxPopulation);
	WRAPPER_READ(wrapper,"CvGame",&m_iMaxLand);
	WRAPPER_READ(wrapper,"CvGame",&m_iMaxTech);
	WRAPPER_READ(wrapper,"CvGame",&m_iMaxWonders);
	WRAPPER_READ(wrapper,"CvGame",&m_iInitPopulation);
	WRAPPER_READ(wrapper,"CvGame",&m_iInitLand);
	WRAPPER_READ(wrapper,"CvGame",&m_iInitTech);
	WRAPPER_READ(wrapper,"CvGame",&m_iInitWonders);

/************************************************************************************************/
/* Afforess                                     12/7/09                                         */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	WRAPPER_READ(wrapper,"CvGame",&m_iCurrentVoteID);
	WRAPPER_READ(wrapper,"CvGame",&m_iWaterAnimalSpawnChance);
	WRAPPER_READ(wrapper,"CvGame",&m_iCutLosersCounter);
	WRAPPER_READ(wrapper,"CvGame",&m_iHighToLowCounter);
	WRAPPER_READ(wrapper,"CvGame",&m_iIncreasingDifficultyCounter);
	WRAPPER_READ(wrapper,"CvGame",&m_iMercyRuleCounter);
	WRAPPER_READ(wrapper,"CvGame",&m_bDiploVictoryEnabled);
	WRAPPER_READ(wrapper,"CvGame",&m_iDarkAgeCount);

	WRAPPER_READ_ARRAY(wrapper, "CvGame", MAX_PLAYERS, m_aiFlexibleDifficultyTimer);
	
	calculateRiverBuildings();
	calculateCoastalBuildings();
	calculateNumWonders();
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	
	// m_uiInitialTime not saved

	WRAPPER_READ(wrapper,"CvGame",&m_bScoreDirty);
	//	Replaced old-style boolean for circumnavigation with the team id of the circumnavigator
	WRAPPER_SKIP_ELEMENT(wrapper,"CvGame",m_bCircumnavigated, SAVE_VALUE_ANY);
	WRAPPER_READ(wrapper,"CvGame",(int*)&m_circumnavigatingTeam);
	// m_bDebugMode not saved
	WRAPPER_READ(wrapper,"CvGame",&m_bFinalInitialized);
	// m_bPbemTurnSent not saved
	WRAPPER_READ(wrapper,"CvGame",&m_bHotPbemBetweenTurns);
	// m_bPlayerOptionsSent not saved
	// TB Nukefix (Correction)
	WRAPPER_SKIP_ELEMENT(wrapper,"CvGame",&m_bNukesValid, SAVE_VALUE_ANY);

	WRAPPER_READ(wrapper,"CvGame",(int*)&m_eHandicap);
	WRAPPER_READ(wrapper,"CvGame",(int*)&m_eAIHandicap); // f1rpo.autoplay
	WRAPPER_READ(wrapper,"CvGame",(int*)&m_ePausePlayer);

	m_eBestLandUnit = NO_UNIT;
	WRAPPER_READ_CLASS_ENUM_ALLOW_MISSING(wrapper,"CvGame",REMAPPED_CLASS_TYPE_UNITS,(int*)&m_eBestLandUnit);

	WRAPPER_READ(wrapper,"CvGame",(int*)&m_eWinner);
	WRAPPER_READ(wrapper,"CvGame",(int*)&m_eVictory);
	WRAPPER_READ(wrapper,"CvGame",(int*)&m_eGameState);

	WRAPPER_READ_STRING(wrapper,"CvGame",m_szScriptData);

	if ((uiFlag & GAME_SAVE_UI_FLAG_VALUE_AND_BASE) == 0)
	{
		std::vector<int> aiEndTurnMessagesReceived(MAX_PLAYERS);
		WRAPPER_READ_ARRAY(wrapper,"CvGame",MAX_PLAYERS, &aiEndTurnMessagesReceived[0]);
	}
	WRAPPER_READ_ARRAY(wrapper,"CvGame",MAX_PLAYERS, m_aiRankPlayer);
	WRAPPER_READ_ARRAY(wrapper,"CvGame",MAX_PLAYERS, m_aiPlayerRank);
	WRAPPER_READ_ARRAY(wrapper,"CvGame",MAX_PLAYERS, m_aiPlayerScore);
/************************************************************************************************/
/* Afforess                                     12/7/09                                         */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
#if defined QC_MASTERY_VICTORY
	WRAPPER_READ_ARRAY(wrapper,"CvGame",MAX_TEAMS, starshipLaunched);
	WRAPPER_READ_ARRAY(wrapper,"CvGame",MAX_TEAMS, diplomaticVictoryAchieved);
	WRAPPER_READ_ARRAY(wrapper,"CvGame",MAX_TEAMS, AscensionGateBuilt); // code added for Ascension Gate & Mastery Victory (by 45°)
#endif
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	
	WRAPPER_READ_ARRAY(wrapper,"CvGame",MAX_TEAMS, m_aiRankTeam);
	WRAPPER_READ_ARRAY(wrapper,"CvGame",MAX_TEAMS, m_aiTeamRank);
	WRAPPER_READ_ARRAY(wrapper,"CvGame",MAX_TEAMS, m_aiTeamScore);
/************************************************************************************************/
/* REVOLUTION_MOD                                                                 lemmy101      */
/*                                                                                jdog5000      */
/*                                                                                              */
/************************************************************************************************/
	WRAPPER_READ_ARRAY(wrapper,"CvGame",MAX_PLAYERS, m_iAIAutoPlay);
	WRAPPER_READ_ARRAY(wrapper,"CvGame",MAX_PLAYERS, m_iForcedAIAutoPlay);
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/

	WRAPPER_READ_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_UNITS, GC.getNumUnitInfos(), m_paiUnitCreatedCount);
	WRAPPER_READ_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_UNIT_CLASSES, GC.getNumUnitClassInfos(), m_paiUnitClassCreatedCount);
	WRAPPER_READ_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_BUILDING_CLASSES, GC.getNumBuildingClassInfos(), m_paiBuildingClassCreatedCount);
	WRAPPER_READ_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_PROJECTS, GC.getNumProjectInfos(), m_paiProjectCreatedCount);
	WRAPPER_READ_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_CIVICS, GC.getNumCivicInfos(), m_paiForceCivicCount);
	WRAPPER_READ_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_VOTES, GC.getNumVoteInfos(), (int*)m_paiVoteOutcome);
	WRAPPER_READ_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_RELIGIONS, GC.getNumReligionInfos(), m_paiReligionGameTurnFounded);
	WRAPPER_READ_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_CORPORATIONS, GC.getNumCorporationInfos(), m_paiCorporationGameTurnFounded);
	WRAPPER_READ_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_VOTE_SOURCES, GC.getNumVoteSourceInfos(), m_aiSecretaryGeneralTimer);
	WRAPPER_READ_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_VOTE_SOURCES, GC.getNumVoteSourceInfos(), m_aiVoteTimer);
	WRAPPER_READ_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_VOTE_SOURCES, GC.getNumVoteSourceInfos(), m_aiDiploVote);

	WRAPPER_READ_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_SPECIAL_UNITS, GC.getNumSpecialUnitInfos(), m_pabSpecialUnitValid);
	WRAPPER_READ_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_SPECIAL_BUILDINGS, GC.getNumSpecialBuildingInfos(), m_pabSpecialBuildingValid);
	WRAPPER_READ_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_RELIGIONS, GC.getNumReligionInfos(), m_abReligionSlotTaken);
	/************************************************************************************************/
	/* RevDCM	                  Start		 4/29/10                                                */
	/*                                                                                              */
	/* OC_LIMITED_RELIGIONS                                                                         */
	/************************************************************************************************/
	if (GC.getInitCore().getGameSaveSvnRev() < 936)
	{
		bool bSkip = false;
		WRAPPER_READ_DECORATED(wrapper, "CvGame", &bSkip, "m_bGameStart");
	}
	WRAPPER_READ_CLASS_ARRAY_ALLOW_MISSING(wrapper, "CvGame", REMAPPED_CLASS_TYPE_TECHS, GC.getNumTechInfos(), m_abTechCanFoundReligion);
	/************************************************************************************************/
	/* LIMITED_RELIGIONS               END                                                          */
	/************************************************************************************************/

	//	If ther latest asets define NEW religions we'll need to initialize their tech can-found flag
	if ( wrapper.isUsingTaggedFormat() )
	{
		for(iI = 0; iI < GC.getNumReligionInfos(); iI++)
		{
			bool bReligionIsNew = true;

			for(int iJ = 0; iJ < wrapper.getNumClassEnumValues(REMAPPED_CLASS_TYPE_RELIGIONS); iJ++)
			{
				if ( wrapper.getNewClassEnumValue(REMAPPED_CLASS_TYPE_RELIGIONS, iJ) == iI )
				{
					bReligionIsNew = false;
					break;
				}
			}

			if ( bReligionIsNew )
			{
				CvReligionInfo&	newReligion = GC.getReligionInfo((ReligionTypes)iI);
				TechTypes	eFoundingTech = (TechTypes)newReligion.getTechPrereq();

				setTechCanFoundReligion(eFoundingTech, false);
			}
		}
	}

	for (iI=0;iI<wrapper.getNumClassEnumValues(REMAPPED_CLASS_TYPE_RELIGIONS);iI++)
	{
		int	eOwner = 0;
		int	iID = 0;
		int	iRemappedReligion = wrapper.getNewClassEnumValue(REMAPPED_CLASS_TYPE_RELIGIONS, iI);

		WRAPPER_READ_DECORATED(wrapper,"CvGame",&eOwner, "m_paHolyCity[].eOwner");
		WRAPPER_READ_DECORATED(wrapper,"CvGame",&iID, "m_paHolyCity[].iID");

		if ( iRemappedReligion != -1 )
		{
			m_paHolyCity[iRemappedReligion].eOwner = (PlayerTypes)eOwner;
			m_paHolyCity[iRemappedReligion].iID = iID;
		}
	}

	for (iI=0;iI<wrapper.getNumClassEnumValues(REMAPPED_CLASS_TYPE_CORPORATIONS);iI++)
	{
		int eOwner = 0;
		int	iID = 0;
		int	iRemappedCorporation = wrapper.getNewClassEnumValue(REMAPPED_CLASS_TYPE_CORPORATIONS, iI);

		WRAPPER_READ_DECORATED(wrapper,"CvGame",&eOwner,"m_paHeadquarters[].eOwner");
		WRAPPER_READ_DECORATED(wrapper,"CvGame",&iID,"m_paHeadquarters[].iID");

		if ( iRemappedCorporation != -1 )
		{
			m_paHeadquarters[iRemappedCorporation].eOwner = (PlayerTypes)eOwner;
			m_paHeadquarters[iRemappedCorporation].iID = iID;
		}
	}

	{
		CvWString szBuffer;
		uint iSize = 0;

		m_aszDestroyedCities.clear();
		WRAPPER_READ_DECORATED(wrapper,"CvGame",&iSize,"DestroyedCityCount");
		for (uint i = 0; i < iSize; i++)
		{
			WRAPPER_READ_STRING_DECORATED(wrapper,"CvGame",szBuffer,"DestroyedCityName");
			m_aszDestroyedCities.push_back(szBuffer);
		}

		m_aszGreatPeopleBorn.clear();
		WRAPPER_READ_DECORATED(wrapper,"CvGame",&iSize,"GreatPeopleBornCount");
		for (uint i = 0; i < iSize; i++)
		{
			WRAPPER_READ_STRING_DECORATED(wrapper,"CvGame",szBuffer,"GreatPersonName");
			m_aszGreatPeopleBorn.push_back(szBuffer);
		}
	}

	ReadStreamableFFreeListTrashArray(m_deals, pStream);
	ReadStreamableFFreeListTrashArray(m_voteSelections, pStream);
	ReadStreamableFFreeListTrashArray(m_votesTriggered, pStream);

	m_mapRand.read(pStream);
	m_sorenRand.read(pStream);

	{
		clearReplayMessageMap();
		ReplayMessageList::_Alloc::size_type iSize;
		WRAPPER_READ(wrapper,"CvGame",&iSize);
		for (ReplayMessageList::_Alloc::size_type i = 0; i < iSize; i++)
		{
			CvReplayMessage* pMessage = new CvReplayMessage(0);
			if (NULL != pMessage)
			{
				pMessage->read(*pStream);
			}
			m_listReplayMessages.push_back(pMessage);
		}
	}
	// m_pReplayInfo not saved

	WRAPPER_READ(wrapper,"CvGame",&m_iNumSessions);
	if (!isNetworkMultiPlayer())
	{
		++m_iNumSessions;
	}

	{
		unsigned int iSize = 0;
		m_aPlotExtraYields.clear();
		WRAPPER_READ_DECORATED(wrapper,"CvGame",&iSize,"PlotYieldCount");
		for (unsigned int i = 0; i < iSize; ++i)
		{
			PlotExtraYield kPlotYield;
			kPlotYield.read(pStream);
			m_aPlotExtraYields.push_back(kPlotYield);
		}
	}

	{
		unsigned int iSize = 0;
		m_aPlotExtraCosts.clear();
		WRAPPER_READ_DECORATED(wrapper,"CvGame",&iSize,"PlotExtraCostsCount");
		for (unsigned int i = 0; i < iSize; ++i)
		{
			PlotExtraCost kPlotCost;
			kPlotCost.read(pStream);
			m_aPlotExtraCosts.push_back(kPlotCost);
		}
	}

	{
		unsigned int iSize = 0;
		m_mapVoteSourceReligions.clear();
		WRAPPER_READ_DECORATED(wrapper,"CvGame",&iSize,"VoteSourceReligionsCount");
		for (unsigned int i = 0; i < iSize; ++i)
		{
			VoteSourceTypes eVoteSource = NO_VOTESOURCE;
			ReligionTypes eReligion = NO_RELIGION;
			WRAPPER_READ_CLASS_ENUM_DECORATED(wrapper,"CvGame",REMAPPED_CLASS_TYPE_VOTE_SOURCES,(int*)&eVoteSource, "VoteSourceReligionMapSource");
			WRAPPER_READ_CLASS_ENUM_DECORATED(wrapper,"CvGame",REMAPPED_CLASS_TYPE_RELIGIONS,(int*)&eReligion, "VoteSourceReligionMapReligion");
			m_mapVoteSourceReligions[eVoteSource] = eReligion;
		}
	}

	{
		unsigned int iSize = 0;
		m_aeInactiveTriggers.clear();
		WRAPPER_READ_DECORATED(wrapper,"CvGame",&iSize,"InactiveTriggersCount");
		for (unsigned int i = 0; i < iSize; ++i)
		{
			int iTrigger = -2;
			WRAPPER_READ_CLASS_ENUM_DECORATED_ALLOW_MISSING(wrapper,"CvGame",REMAPPED_CLASS_TYPE_EVENT_TRIGGERS,&iTrigger,"InactiveTrigger");
			
			if ( iTrigger == -2 && wrapper.isUsingTaggedFormat() )
			{
				//	Consume the value for older formats since the information cannot be
				//	believed anyway
				WRAPPER_READ_DECORATED(wrapper, "CvGame", &iTrigger, "InactiveTrigger");
			}
			else if ( iTrigger != -1 )
			{
				m_aeInactiveTriggers.push_back((EventTriggerTypes)iTrigger);
			}
		}
	}

	// Get the active player information from the initialization structure
	if (!isGameMultiPlayer())
	{
		for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isHuman())
			{
				setActivePlayer((PlayerTypes)iI);
				break;
			}
		}
		addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getActivePlayer(), gDLL->getText("TXT_KEY_MISC_RELOAD", m_iNumSessions));
	}

	if (isOption(GAMEOPTION_NEW_RANDOM_SEED) && !GC.getDefineINT("DISABLE_RAND_SEED_ON_LOAD"))
	{
		if (!isNetworkMultiPlayer())
		{
			m_sorenRand.reseed(timeGetTime());
		}
	}

	WRAPPER_READ(wrapper,"CvGame",&m_iShrineBuildingCount);
	//	Retain bug-compatibility with the old format which needlessly saved the entire max
	//	size the array could possibly be here
	if ( wrapper.isUsingTaggedFormat() )
	{
		WRAPPER_READ_CLASS_ENUM_ARRAY_ALLOW_MISSING(wrapper,"CvGame",REMAPPED_CLASS_TYPE_BUILDINGS,m_iShrineBuildingCount, m_aiShrineBuilding);
		WRAPPER_READ_CLASS_ENUM_ARRAY_ALLOW_MISSING(wrapper,"CvGame",REMAPPED_CLASS_TYPE_RELIGIONS,m_iShrineBuildingCount, m_aiShrineReligion);
	}
	else
	{
		WRAPPER_READ_CLASS_ENUM_ARRAY(wrapper,"CvGame",REMAPPED_CLASS_TYPE_BUILDINGS,GC.getNumBuildingInfos(), m_aiShrineBuilding);
		WRAPPER_READ_CLASS_ENUM_ARRAY(wrapper,"CvGame",REMAPPED_CLASS_TYPE_RELIGIONS,GC.getNumBuildingInfos(), m_aiShrineReligion);
	}
	WRAPPER_READ(wrapper,"CvGame",&m_iNumCultureVictoryCities);
	WRAPPER_READ(wrapper,"CvGame",&m_eCultureVictoryCultureLevel);

	bool bDarkAgeState = false;
	WRAPPER_READ_DECORATED(wrapper,"CvGame", &bDarkAgeState, "m_bDarkAgeState");
	if (bDarkAgeState)
	{
		int iDelayTurns = 0;
		WRAPPER_READ_DECORATED(wrapper,"CvGame", &iDelayTurns, "m_iDarkAgeDelayTurns");
		int iLenTurns = 0;
		WRAPPER_READ_DECORATED(wrapper,"CvGame", &iLenTurns, "m_iDarkAgeiLenTurns");
		CvWString szStartTxtKey;
		WRAPPER_READ_STRING_DECORATED(wrapper,"CvGame", szStartTxtKey, "m_iDarkAgeszStartTxtKey");
		CvWString szEndTxtKey;
		WRAPPER_READ_STRING_DECORATED(wrapper,"CvGame", szEndTxtKey, "m_iDarkAgeszEndTxtKey");

		triggerDarkAge(iDelayTurns, iLenTurns, szStartTxtKey, szEndTxtKey);
	}

	m_Properties.readWrapper(pStream);
/************************************************************************************************/
/* Afforess	                  Start		 07/27/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	setFutureEras();
	int iCurrentHandicap = range(getHandicapType(), 0, GC.getNumHandicapInfos() - 1);
	setHandicapType((HandicapTypes)iCurrentHandicap);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	m_plotGroupHashesInitialized = false;	//	Force calculation in first update timeslice

	WRAPPER_READ_OBJECT_END(wrapper);
}

#define TAGGED_SAVES 1

void CvGame::write(FDataStreamBase* pStream)
{
	int iI;
	CvTaggedSaveFormatWrapper&	wrapper = CvTaggedSaveFormatWrapper::getSaveFormatWrapper();

	uint uiFlag = GAME_SAVE_UI_FLAG_VALUE_AND_BASE;

	wrapper.AttachToStream(pStream);

	WRAPPER_WRITE_OBJECT_START(wrapper);

	WRAPPER_WRITE(wrapper, "CvGame", uiFlag);		// flag for expansion

	WRAPPER_WRITE(wrapper, "CvGame", m_iElapsedGameTurns);
	WRAPPER_WRITE(wrapper, "CvGame", m_iStartTurn);
	WRAPPER_WRITE(wrapper, "CvGame", m_iStartYear);
	WRAPPER_WRITE(wrapper, "CvGame", m_iEstimateEndTurn);
	WRAPPER_WRITE(wrapper, "CvGame", m_iTurnSlice);
	WRAPPER_WRITE(wrapper, "CvGame", m_iCutoffSlice);
	WRAPPER_WRITE(wrapper, "CvGame", m_iNumGameTurnActive);
	WRAPPER_WRITE(wrapper, "CvGame", m_iNumCities);
	WRAPPER_WRITE(wrapper, "CvGame", m_iTotalPopulation);
	WRAPPER_WRITE(wrapper, "CvGame", m_iTradeRoutes);
	WRAPPER_WRITE(wrapper, "CvGame", m_iFreeTradeCount);
	WRAPPER_WRITE(wrapper, "CvGame", m_iNoNukesCount);
	WRAPPER_WRITE(wrapper, "CvGame", m_iNukesExploded);
	WRAPPER_WRITE(wrapper, "CvGame", m_iMaxPopulation);
	WRAPPER_WRITE(wrapper, "CvGame", m_iMaxLand);
	WRAPPER_WRITE(wrapper, "CvGame", m_iMaxTech);
	WRAPPER_WRITE(wrapper, "CvGame", m_iMaxWonders);
	WRAPPER_WRITE(wrapper, "CvGame", m_iInitPopulation);
	WRAPPER_WRITE(wrapper, "CvGame", m_iInitLand);
	WRAPPER_WRITE(wrapper, "CvGame", m_iInitTech);
	WRAPPER_WRITE(wrapper, "CvGame", m_iInitWonders);

/************************************************************************************************/
/* Afforess                                     12/7/09                                         */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	WRAPPER_WRITE(wrapper, "CvGame", m_iCurrentVoteID);
	WRAPPER_WRITE(wrapper, "CvGame", m_iWaterAnimalSpawnChance);
	WRAPPER_WRITE(wrapper, "CvGame", m_iCutLosersCounter);
	WRAPPER_WRITE(wrapper, "CvGame", m_iHighToLowCounter);
	WRAPPER_WRITE(wrapper, "CvGame", m_iIncreasingDifficultyCounter);
	WRAPPER_WRITE(wrapper, "CvGame", m_iMercyRuleCounter);
	WRAPPER_WRITE(wrapper, "CvGame", m_bDiploVictoryEnabled);
	WRAPPER_WRITE(wrapper, "CvGame", m_iDarkAgeCount);
	WRAPPER_WRITE_ARRAY(wrapper, "CvGame", MAX_PLAYERS, m_aiFlexibleDifficultyTimer);
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	

	// m_uiInitialTime not saved

	WRAPPER_WRITE(wrapper, "CvGame", m_bScoreDirty);
	WRAPPER_WRITE(wrapper, "CvGame", (int)m_circumnavigatingTeam);
	// m_bDebugMode not saved
	WRAPPER_WRITE(wrapper, "CvGame", m_bFinalInitialized);
	// m_bPbemTurnSent not saved
	WRAPPER_WRITE(wrapper, "CvGame", m_bHotPbemBetweenTurns);
	// m_bPlayerOptionsSent not saved
	// TB Nukefix
	// WRAPPER_WRITE(wrapper, "CvGame", m_bNukesValid);

	WRAPPER_WRITE(wrapper, "CvGame", m_eHandicap);
	WRAPPER_WRITE(wrapper, "CvGame", m_eAIHandicap); // f1rpo.autoplay
	WRAPPER_WRITE(wrapper, "CvGame", m_ePausePlayer);
	WRAPPER_WRITE_CLASS_ENUM(wrapper, "CvGame", REMAPPED_CLASS_TYPE_UNITS, m_eBestLandUnit);
	WRAPPER_WRITE(wrapper, "CvGame", m_eWinner);
	WRAPPER_WRITE(wrapper, "CvGame", m_eVictory);
	WRAPPER_WRITE(wrapper, "CvGame", m_eGameState);

	WRAPPER_WRITE_STRING(wrapper, "CvGame", m_szScriptData);

	WRAPPER_WRITE_ARRAY(wrapper, "CvGame", MAX_PLAYERS, m_aiRankPlayer);
	WRAPPER_WRITE_ARRAY(wrapper, "CvGame", MAX_PLAYERS, m_aiPlayerRank);
	WRAPPER_WRITE_ARRAY(wrapper, "CvGame", MAX_PLAYERS, m_aiPlayerScore);
/************************************************************************************************/
/* Afforess                                     12/7/09                                         */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
#if defined QC_MASTERY_VICTORY

	WRAPPER_WRITE_ARRAY(wrapper, "CvGame", MAX_TEAMS, starshipLaunched);
	WRAPPER_WRITE_ARRAY(wrapper, "CvGame", MAX_TEAMS, diplomaticVictoryAchieved);
	WRAPPER_WRITE_ARRAY(wrapper, "CvGame", MAX_TEAMS, AscensionGateBuilt); // code added for Ascension Gate & Mastery Victory (by 45°)
#endif
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	
	WRAPPER_WRITE_ARRAY(wrapper, "CvGame", MAX_TEAMS, m_aiRankTeam);
	WRAPPER_WRITE_ARRAY(wrapper, "CvGame", MAX_TEAMS, m_aiTeamRank);
	WRAPPER_WRITE_ARRAY(wrapper, "CvGame", MAX_TEAMS, m_aiTeamScore);
/************************************************************************************************/
/* REVOLUTION_MOD                                                                 lemmy101      */
/*                                                                                jdog5000      */
/*                                                                                              */
/************************************************************************************************/
	WRAPPER_WRITE_ARRAY(wrapper, "CvGame", MAX_PLAYERS, m_iAIAutoPlay);
	WRAPPER_WRITE_ARRAY(wrapper, "CvGame", MAX_PLAYERS, m_iForcedAIAutoPlay);
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/

	WRAPPER_WRITE_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_UNITS, GC.getNumUnitInfos(), m_paiUnitCreatedCount);
	WRAPPER_WRITE_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_UNIT_CLASSES, GC.getNumUnitClassInfos(), m_paiUnitClassCreatedCount);
	WRAPPER_WRITE_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_BUILDING_CLASSES, GC.getNumBuildingClassInfos(), m_paiBuildingClassCreatedCount);
	WRAPPER_WRITE_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_PROJECTS, GC.getNumProjectInfos(), m_paiProjectCreatedCount);
	WRAPPER_WRITE_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_CIVICS, GC.getNumCivicInfos(), m_paiForceCivicCount);
	WRAPPER_WRITE_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_VOTES, GC.getNumVoteInfos(), (int*)m_paiVoteOutcome);
	WRAPPER_WRITE_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_RELIGIONS, GC.getNumReligionInfos(), m_paiReligionGameTurnFounded);
	WRAPPER_WRITE_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_CORPORATIONS, GC.getNumCorporationInfos(), m_paiCorporationGameTurnFounded);
	WRAPPER_WRITE_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_VOTE_SOURCES, GC.getNumVoteSourceInfos(), m_aiSecretaryGeneralTimer);
	WRAPPER_WRITE_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_VOTE_SOURCES, GC.getNumVoteSourceInfos(), m_aiVoteTimer);
	WRAPPER_WRITE_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_VOTE_SOURCES, GC.getNumVoteSourceInfos(), m_aiDiploVote);

	WRAPPER_WRITE_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_SPECIAL_UNITS, GC.getNumSpecialUnitInfos(), m_pabSpecialUnitValid);
	WRAPPER_WRITE_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_SPECIAL_BUILDINGS, GC.getNumSpecialBuildingInfos(), m_pabSpecialBuildingValid);
	WRAPPER_WRITE_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_RELIGIONS, GC.getNumReligionInfos(), m_abReligionSlotTaken);

/************************************************************************************************/
/* RevDCM	                  Start		 4/29/10                                                */
/*                                                                                              */
/* OC_LIMITED_RELIGIONS                                                                         */
/************************************************************************************************/
	WRAPPER_WRITE_CLASS_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_TECHS, GC.getNumTechInfos(), m_abTechCanFoundReligion);
/************************************************************************************************/
/* LIMITED_RELIGIONS               END                                                          */
/************************************************************************************************/

	for (iI=0;iI<GC.getNumReligionInfos();iI++)
	{
		WRAPPER_WRITE(wrapper, "CvGame", m_paHolyCity[iI].eOwner);
		WRAPPER_WRITE(wrapper, "CvGame", m_paHolyCity[iI].iID);
	}

	for (iI=0;iI<GC.getNumCorporationInfos();iI++)
	{
		WRAPPER_WRITE(wrapper, "CvGame", m_paHeadquarters[iI].eOwner);
		WRAPPER_WRITE(wrapper, "CvGame", m_paHeadquarters[iI].iID);
	}

	{
		std::vector<CvWString>::iterator ws_it;

		WRAPPER_WRITE_DECORATED(wrapper, "CvGame", m_aszDestroyedCities.size(), "DestroyedCityCount");
		for (ws_it = m_aszDestroyedCities.begin(); ws_it != m_aszDestroyedCities.end(); ++ws_it)
		{
			WRAPPER_WRITE_STRING_DECORATED(wrapper, "CvGame", *ws_it, "DestroyedCityName");
		}

		WRAPPER_WRITE_DECORATED(wrapper, "CvGame", m_aszGreatPeopleBorn.size(), "GreatPeopleBornCount");
		for (ws_it = m_aszGreatPeopleBorn.begin(); ws_it != m_aszGreatPeopleBorn.end(); ++ws_it)
		{
			WRAPPER_WRITE_STRING_DECORATED(wrapper, "CvGame", *ws_it, "GreatPersonName");
		}
	}

	WriteStreamableFFreeListTrashArray(m_deals, pStream);
	WriteStreamableFFreeListTrashArray(m_voteSelections, pStream);
	WriteStreamableFFreeListTrashArray(m_votesTriggered, pStream);

	m_mapRand.write(pStream);
	m_sorenRand.write(pStream);

	ReplayMessageList::_Alloc::size_type iSize = m_listReplayMessages.size();
	WRAPPER_WRITE(wrapper, "CvGame", iSize);
	ReplayMessageList::const_iterator it;
	for (it = m_listReplayMessages.begin(); it != m_listReplayMessages.end(); ++it)
	{
		const CvReplayMessage* pMessage = *it;
		if (NULL != pMessage)
		{
			pMessage->write(*pStream);
		}
	}
	// m_pReplayInfo not saved

	WRAPPER_WRITE(wrapper, "CvGame", m_iNumSessions);

	WRAPPER_WRITE_DECORATED(wrapper, "CvGame", m_aPlotExtraYields.size(), "PlotYieldCount");
	for (std::vector<PlotExtraYield>::iterator it = m_aPlotExtraYields.begin(); it != m_aPlotExtraYields.end(); ++it)
	{
		(*it).write(pStream);
	}

	WRAPPER_WRITE_DECORATED(wrapper, "CvGame", m_aPlotExtraCosts.size(), "PlotExtraCostsCount");
	for (std::vector<PlotExtraCost>::iterator it = m_aPlotExtraCosts.begin(); it != m_aPlotExtraCosts.end(); ++it)
	{
		(*it).write(pStream);
	}

	WRAPPER_WRITE_DECORATED(wrapper, "CvGame", m_mapVoteSourceReligions.size(), "VoteSourceReligionsCount");
	for (stdext::hash_map<VoteSourceTypes, ReligionTypes>::iterator it = m_mapVoteSourceReligions.begin(); it != m_mapVoteSourceReligions.end(); ++it)
	{
		WRAPPER_WRITE_CLASS_ENUM_DECORATED(wrapper, "CvGame", REMAPPED_CLASS_TYPE_VOTE_SOURCES, it->first, "VoteSourceReligionMapSource");
		WRAPPER_WRITE_CLASS_ENUM_DECORATED(wrapper, "CvGame", REMAPPED_CLASS_TYPE_RELIGIONS, it->second, "VoteSourceReligionMapReligion");
	}

	WRAPPER_WRITE_DECORATED(wrapper, "CvGame", m_aeInactiveTriggers.size(), "InactiveTriggersCount");
	for (std::vector<EventTriggerTypes>::iterator e_it = m_aeInactiveTriggers.begin(); e_it != m_aeInactiveTriggers.end(); ++e_it)
	{
		WRAPPER_WRITE_CLASS_ENUM_DECORATED(wrapper, "CvGame", REMAPPED_CLASS_TYPE_EVENT_TRIGGERS, *e_it, "InactiveTrigger");
	}

	WRAPPER_WRITE(wrapper, "CvGame", m_iShrineBuildingCount);
	//	Retain bug-compatibility with the old format which needlessly saved the entire max
	//	size the array could possibly be here
	if ( wrapper.isUsingTaggedFormat() )
	{
		WRAPPER_WRITE_CLASS_ENUM_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_BUILDINGS, m_iShrineBuildingCount, m_aiShrineBuilding);
		WRAPPER_WRITE_CLASS_ENUM_ARRAY(wrapper, "CvGame", REMAPPED_CLASS_TYPE_RELIGIONS, m_iShrineBuildingCount, m_aiShrineReligion);
	}
	else
	{
		WRAPPER_WRITE_CLASS_ENUM_ARRAY(wrapper,"CvGame",REMAPPED_CLASS_TYPE_BUILDINGS,GC.getNumBuildingInfos(), m_aiShrineBuilding);
		WRAPPER_WRITE_CLASS_ENUM_ARRAY(wrapper,"CvGame",REMAPPED_CLASS_TYPE_RELIGIONS,GC.getNumBuildingInfos(), m_aiShrineReligion);
	}
	WRAPPER_WRITE(wrapper, "CvGame", m_iNumCultureVictoryCities);
	WRAPPER_WRITE(wrapper, "CvGame", m_eCultureVictoryCultureLevel);

	bool bDarkAgeState = m_pDarkAgeState != NULL;
	WRAPPER_WRITE_DECORATED(wrapper,"CvGame", bDarkAgeState, "m_bDarkAgeState");
	if (bDarkAgeState)
	{
		WRAPPER_WRITE_DECORATED(wrapper,"CvGame", m_pDarkAgeState->iDelayTurns, "m_iDarkAgeDelayTurns");
		WRAPPER_WRITE_DECORATED(wrapper,"CvGame", m_pDarkAgeState->iLenTurns, "m_iDarkAgeiLenTurns");
		WRAPPER_WRITE_STRING_DECORATED(wrapper,"CvGame", m_pDarkAgeState->szStartTxtKey, "m_iDarkAgeszStartTxtKey");
		WRAPPER_WRITE_STRING_DECORATED(wrapper,"CvGame", m_pDarkAgeState->szEndTxtKey, "m_iDarkAgeszEndTxtKey");
	}

	m_Properties.writeWrapper(pStream);

	WRAPPER_WRITE_OBJECT_END(wrapper);
}

void CvGame::writeReplay(FDataStreamBase& stream, PlayerTypes ePlayer)
{
	SAFE_DELETE(m_pReplayInfo);
	m_pReplayInfo = new CvReplayInfo();
	if (m_pReplayInfo)
	{
		m_pReplayInfo->createInfo(ePlayer);

		m_pReplayInfo->write(stream);
	}
}

void CvGame::saveReplay(PlayerTypes ePlayer)
{
	//	First opportunity after a load of a won game to close the wrapper.  This is important since
	//	hall of fame replay info does not use max compatability format and so must not have the
	//	current wrapper selecting that
	CvTaggedSaveFormatWrapper&	wrapper = CvTaggedSaveFormatWrapper::getSaveFormatWrapper();

	//	Close will free any resources and display any warnings if we've just finished loading/saving
	wrapper.close();

	gDLL->getEngineIFace()->SaveReplay(ePlayer);
}


void CvGame::showEndGameSequence()
{
	CvPopupInfo* pInfo;
	CvWString szBuffer;
	int iI;

	long iHours = getMinutesPlayed() / 60;
	long iMinutes = getMinutesPlayed() % 60;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		CvPlayer& player = GET_PLAYER((PlayerTypes)iI);
		if (player.isHuman())
		{
			addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, (PlayerTypes)iI, gDLL->getText("TXT_KEY_MISC_TIME_SPENT", iHours, iMinutes));

			pInfo = new CvPopupInfo(BUTTONPOPUP_TEXT);
			if (NULL != pInfo)
			{
				if ((getWinner() != NO_TEAM) && (getVictory() != NO_VICTORY))
				{
					pInfo->setText(gDLL->getText("TXT_KEY_GAME_WON", GET_TEAM(getWinner()).getName().GetCString(), GC.getVictoryInfo(getVictory()).getTextKeyWide()));
				}
				else
				{
					pInfo->setText(gDLL->getText("TXT_KEY_MISC_DEFEAT"));
				}
				player.addPopup(pInfo);
			}

			if (getWinner() == player.getTeam())
			{
				if (!CvString(GC.getVictoryInfo(getVictory()).getMovie()).empty())
				{
					// show movie
					pInfo = new CvPopupInfo(BUTTONPOPUP_PYTHON_SCREEN);
					if (NULL != pInfo)
					{
						pInfo->setText(L"showVictoryMovie");
						pInfo->setData1((int)getVictory());
						player.addPopup(pInfo);
					}
				}
				else if (GC.getVictoryInfo(getVictory()).isDiploVote())
				{
					pInfo = new CvPopupInfo(BUTTONPOPUP_PYTHON_SCREEN);
					if (NULL != pInfo)
					{
						pInfo->setText(L"showUnVictoryScreen");
						player.addPopup(pInfo);
					}
				}
			}

			// show replay
			pInfo = new CvPopupInfo(BUTTONPOPUP_PYTHON_SCREEN);
			if (NULL != pInfo)
			{
				pInfo->setText(L"showReplay"); 
				pInfo->setData1(iI);
				pInfo->setOption1(false); // don't go to HOF on exit
				player.addPopup(pInfo);
			}

			// show top cities / stats
			pInfo = new CvPopupInfo(BUTTONPOPUP_PYTHON_SCREEN);
			if (NULL != pInfo)
			{
				pInfo->setText(L"showInfoScreen");
				pInfo->setData1(0);
				pInfo->setData2(1);
				player.addPopup(pInfo);
			}

			// show Dan
			pInfo = new CvPopupInfo(BUTTONPOPUP_PYTHON_SCREEN);
			if (NULL != pInfo)
			{
				pInfo->setText(L"showDanQuayleScreen");
				player.addPopup(pInfo);
			}

			// show Hall of Fame
			pInfo = new CvPopupInfo(BUTTONPOPUP_PYTHON_SCREEN);
			if (NULL != pInfo)
			{
				pInfo->setText(L"showHallOfFame");
				player.addPopup(pInfo);
			}
		}
	}
}

CvReplayInfo* CvGame::getReplayInfo() const
{
	return m_pReplayInfo;
}

void CvGame::setReplayInfo(CvReplayInfo* pReplay)
{
	SAFE_DELETE(m_pReplayInfo);
	m_pReplayInfo = pReplay;
}

bool CvGame::hasSkippedSaveChecksum() const
{
	return gDLL->hasSkippedSaveChecksum();
}

/************************************************************************************************/
/* REVOLUTION_MOD                                                                 jdog5000      */
/*                                                                                lemmy101      */
/*                                                                                              */
/************************************************************************************************/
//
// for logging
//
void CvGame::logMsg(char* format, ... )
{
	// <f1rpo>
	if (!GC.getLogging())
		return; // </f1rpo>
	static char buf[2048];
	_vsnprintf( buf, 2048-4, format, (char*)(&format+1) );
	if (GC.getGameINLINE().getActivePlayer() != -1)
		gDLL->logMsg("ANewDawn.log", buf);
	else
		gDLL->logMsg("ANewDawnPitboss.log", buf);
}

void CvGame::logDebugMsg(char* format, ...)
{
#ifdef _DEBUG
	// <f1rpo>
	if (!GC.getLogging())
		return; // </f1rpo>
	TCHAR szOut[1024];
	if (GC.getGameINLINE().getActivePlayer() != -1)
		sprintf(szOut, "Player %d - Multiplayer Game Log.log", GC.getGameINLINE().getActivePlayer());
	else
		sprintf(szOut, "Pitboss Multiplayer Game Log.log", GC.getGameINLINE().getActivePlayer());

	static char buf[2048];
	_vsnprintf(buf, 2048 - 4, format, (char*)(&format + 1));
	gDLL->logMsg(szOut, buf);
#endif 
}


void CvGame::logMsgTo(const TCHAR* logFile, char* format, ...)
{
	static char buf[2048];
	_vsnprintf(buf, 2048 - 4, format, (char*)(&format + 1));
	gDLL->logMsg(logFile, buf);
}
//
// 
//
void CvGame::addPlayer(PlayerTypes eNewPlayer, LeaderHeadTypes eLeader, CivilizationTypes eCiv, bool bSetAlive, bool bDeclareWar)
{
	// UNOFFICIAL_PATCH Start
	// * Fixed bug with colonies who occupy recycled player slots showing the old leader or civ names.
	CvWString szEmptyString = L"";
	LeaderHeadTypes eOldLeader = GET_PLAYER(eNewPlayer).getLeaderType();
	if ( (eOldLeader != NO_LEADER) && (eOldLeader != eLeader) ) 
	{
		GC.getInitCore().setLeaderName(eNewPlayer, szEmptyString);
	}
	CivilizationTypes eOldCiv = GET_PLAYER(eNewPlayer).getCivilizationType();
	if ( (eOldCiv != NO_CIVILIZATION) && (eOldCiv != eCiv) ) 
	{
		GC.getInitCore().setCivAdjective(eNewPlayer, szEmptyString);
		GC.getInitCore().setCivDescription(eNewPlayer, szEmptyString);
		GC.getInitCore().setCivShortDesc(eNewPlayer, szEmptyString);
	}
	// UNOFFICIAL_PATCH End
	PlayerColorTypes eColor = (PlayerColorTypes)GC.getCivilizationInfo(eCiv).getDefaultPlayerColor();

	for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       12/30/08                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original bts code
		if (eColor == NO_PLAYERCOLOR || GET_PLAYER((PlayerTypes)iI).getPlayerColor() == eColor)
*/
		// Don't invalidate color choice if it's taken by this player
		if (eColor == NO_PLAYERCOLOR || (GET_PLAYER((PlayerTypes)iI).getPlayerColor() == eColor && (PlayerTypes)iI != eNewPlayer) )
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
		{
			for (int iK = 0; iK < GC.getNumPlayerColorInfos(); iK++)
			{
				if (iK != GC.getCivilizationInfo((CivilizationTypes)GC.getDefineINT("BARBARIAN_CIVILIZATION")).getDefaultPlayerColor())
				{
					bool bValid = true;

					for (int iL = 0; iL < MAX_CIV_PLAYERS; iL++)
					{
						if (GET_PLAYER((PlayerTypes)iL).getPlayerColor() == iK)
						{
							bValid = false;
							break;
						}
					}

					if (bValid)
					{
						eColor = (PlayerColorTypes)iK;
						iI = MAX_CIV_PLAYERS;
						break;
					}
				}
			}
		}
	}

	//TeamTypes eTeam = GET_PLAYER(eNewPlayer).getTeam();
	GC.getInitCore().setLeader(eNewPlayer, eLeader);
	GC.getInitCore().setCiv(eNewPlayer, eCiv);
	GC.getInitCore().setSlotStatus(eNewPlayer, SS_COMPUTER);
	GC.getInitCore().setColor(eNewPlayer, eColor);
	//GET_TEAM(eTeam).init(eTeam);
	//GET_PLAYER(eNewPlayer).init(eNewPlayer);

	// Team init now handled when appropriate by CvPlayer::initInGame
	// Standard player init 
	GET_PLAYER(eNewPlayer).initInGame(eNewPlayer, bSetAlive, bDeclareWar);
}

void CvGame::changeHumanPlayer( PlayerTypes eOldHuman, PlayerTypes eNewHuman )
{
	int iI;

	// It's a multiplayer game, eep!
	if(GC.getInitCore().getMultiplayer())
	{
		int netID = GC.getInitCore().getNetID(eOldHuman);
		GC.getInitCore().setNetID(eNewHuman, netID);
		GC.getInitCore().setNetID(eOldHuman, -1);
	}

	GET_PLAYER(eNewHuman).setIsHuman(true);
	if(getActivePlayer()==eOldHuman)
	setActivePlayer(eNewHuman, false);
	
	for (iI = 0; iI < NUM_PLAYEROPTION_TYPES; iI++)
	{
		GET_PLAYER(eNewHuman).setOption( (PlayerOptionTypes)iI, GET_PLAYER(eOldHuman).isOption((PlayerOptionTypes)iI) );
	}

	for (iI = 0; iI < NUM_PLAYEROPTION_TYPES; iI++)
	{
		gDLL->sendPlayerOption(((PlayerOptionTypes)iI), GET_PLAYER(eNewHuman).isOption((PlayerOptionTypes)iI));
	}

	GET_PLAYER(eOldHuman).setIsHuman(false);
}
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/
bool CvGame::isCompetingCorporation(CorporationTypes eCorporation1, CorporationTypes eCorporation2) const
{
/************************************************************************************************/
/* Afforess	                  Start		 02/09/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (GC.getCorporationInfo(eCorporation1).isCompetingCorporation(eCorporation2) || GC.getCorporationInfo(eCorporation2).isCompetingCorporation(eCorporation1))
		return true;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	for (int i = 0; i < GC.getNUM_CORPORATION_PREREQ_BONUSES(); ++i)
	{
		if (GC.getCorporationInfo(eCorporation1).getPrereqBonus(i) != NO_BONUS)
		{
			for (int j = 0; j < GC.getNUM_CORPORATION_PREREQ_BONUSES(); ++j)
			{
				if (GC.getCorporationInfo(eCorporation2).getPrereqBonus(j) != NO_BONUS)
				{
					if (GC.getCorporationInfo(eCorporation1).getPrereqBonus(i) == GC.getCorporationInfo(eCorporation2).getPrereqBonus(j))
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

int CvGame::getPlotExtraYield(int iX, int iY, YieldTypes eYield) const
{
	for (std::vector<PlotExtraYield>::const_iterator it = m_aPlotExtraYields.begin(); it != m_aPlotExtraYields.end(); ++it)
	{
		if ((*it).m_iX == iX && (*it).m_iY == iY)
		{
			return (*it).m_aeExtraYield[eYield];
		}
	}

	return 0;
}

void CvGame::setPlotExtraYield(int iX, int iY, YieldTypes eYield, int iExtraYield)
{
	bool bFound = false;

	for (std::vector<PlotExtraYield>::iterator it = m_aPlotExtraYields.begin(); it != m_aPlotExtraYields.end(); ++it)
	{
		if ((*it).m_iX == iX && (*it).m_iY == iY)
		{
			(*it).m_aeExtraYield[eYield] += iExtraYield;
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		PlotExtraYield kExtraYield;
		kExtraYield.m_iX = iX;
		kExtraYield.m_iY = iY;
		for (int i = 0; i < NUM_YIELD_TYPES; ++i)
		{
			if (eYield == i)
			{
				kExtraYield.m_aeExtraYield.push_back(iExtraYield);
			}
			else
			{
				kExtraYield.m_aeExtraYield.push_back(0);
			}
		}
		m_aPlotExtraYields.push_back(kExtraYield);
	}

	CvPlot* pPlot = GC.getMapINLINE().plot(iX, iY);
	if (NULL != pPlot)
	{
		pPlot->updateYield();
	}
}

void CvGame::removePlotExtraYield(int iX, int iY)
{
	for (std::vector<PlotExtraYield>::iterator it = m_aPlotExtraYields.begin(); it != m_aPlotExtraYields.end(); ++it)
	{
		if ((*it).m_iX == iX && (*it).m_iY == iY)
		{
			m_aPlotExtraYields.erase(it);
			break;
		}
	}

	CvPlot* pPlot = GC.getMapINLINE().plot(iX, iY);
	if (NULL != pPlot)
	{
		pPlot->updateYield();
	}
}

int CvGame::getPlotExtraCost(int iX, int iY) const
{
	for (std::vector<PlotExtraCost>::const_iterator it = m_aPlotExtraCosts.begin(); it != m_aPlotExtraCosts.end(); ++it)
	{
		if ((*it).m_iX == iX && (*it).m_iY == iY)
		{
			return (*it).m_iCost;
		}
	}

	return 0;
}

void CvGame::changePlotExtraCost(int iX, int iY, int iCost)
{
	bool bFound = false;

	for (std::vector<PlotExtraCost>::iterator it = m_aPlotExtraCosts.begin(); it != m_aPlotExtraCosts.end(); ++it)
	{
		if ((*it).m_iX == iX && (*it).m_iY == iY)
		{
			(*it).m_iCost += iCost;
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		PlotExtraCost kExtraCost;
		kExtraCost.m_iX = iX;
		kExtraCost.m_iY = iY;
		kExtraCost.m_iCost = iCost;
		m_aPlotExtraCosts.push_back(kExtraCost);
	}
}

void CvGame::removePlotExtraCost(int iX, int iY)
{
	for (std::vector<PlotExtraCost>::iterator it = m_aPlotExtraCosts.begin(); it != m_aPlotExtraCosts.end(); ++it)
	{
		if ((*it).m_iX == iX && (*it).m_iY == iY)
		{
			m_aPlotExtraCosts.erase(it);
			break;
		}
	}
}

ReligionTypes CvGame::getVoteSourceReligion(VoteSourceTypes eVoteSource) const
{
	stdext::hash_map<VoteSourceTypes, ReligionTypes>::const_iterator it;

	it = m_mapVoteSourceReligions.find(eVoteSource);
	if (it == m_mapVoteSourceReligions.end())
	{
		return NO_RELIGION;
	}

	return it->second;
}

void CvGame::setVoteSourceReligion(VoteSourceTypes eVoteSource, ReligionTypes eReligion, bool bAnnounce)
{
	m_mapVoteSourceReligions[eVoteSource] = eReligion;

	if (bAnnounce)
	{
		if (NO_RELIGION != eReligion)
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_VOTE_SOURCE_RELIGION", GC.getReligionInfo(eReligion).getTextKeyWide(), GC.getReligionInfo(eReligion).getAdjectiveKey(), GC.getVoteSourceInfo(eVoteSource).getTextKeyWide());

			for (int iI = 0; iI < MAX_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).isAlive())
				{
					MEMORY_TRACK_EXEMPT();

					AddDLLMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getReligionInfo(eReligion).getSound(), MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
				}
			}
		}
	}
}


// CACHE: cache frequently used values
///////////////////////////////////////


int CvGame::getShrineBuildingCount(ReligionTypes eReligion)
{
	int	iShrineBuildingCount = 0;

	if (eReligion == NO_RELIGION)
		iShrineBuildingCount = m_iShrineBuildingCount;
	else for (int iI = 0; iI < m_iShrineBuildingCount; iI++)
		if (m_aiShrineReligion[iI] == eReligion)
			iShrineBuildingCount++;

	return iShrineBuildingCount;
}

BuildingTypes CvGame::getShrineBuilding(int eIndex, ReligionTypes eReligion)
{
	FAssertMsg(eIndex >= 0 && eIndex < m_iShrineBuildingCount, "invalid index to CvGame::getShrineBuilding");

	BuildingTypes eBuilding = NO_BUILDING;

	if (eIndex >= 0 && eIndex < m_iShrineBuildingCount)
	{
		if (eReligion == NO_RELIGION)
			eBuilding = (BuildingTypes) m_aiShrineBuilding[eIndex];
		else for (int iI = 0, iReligiousBuilding = 0; iI < m_iShrineBuildingCount; iI++)
			if (m_aiShrineReligion[iI] == (int) eReligion)
			{
				if (iReligiousBuilding == eIndex)
				{
					// found it
					eBuilding = (BuildingTypes) m_aiShrineBuilding[iI];
					break;
				}

				iReligiousBuilding++;
			}
	}

	return eBuilding;
}

void CvGame::changeShrineBuilding(BuildingTypes eBuilding, ReligionTypes eReligion, bool bRemove)
{
	FAssertMsg(eBuilding >= 0 && eBuilding < GC.getNumBuildingInfos(), "invalid index to CvGame::changeShrineBuilding");
	FAssertMsg(bRemove || m_iShrineBuildingCount < GC.getNumBuildingInfos(), "trying to add too many buildings to CvGame::changeShrineBuilding");

	if (bRemove)
	{
		bool bFound = false;

		for (int iI = 0; iI < m_iShrineBuildingCount; iI++)
		{
			if (!bFound)
			{
				// note, eReligion is not important if we removing, since each building is always one religion
				if (m_aiShrineBuilding[iI] == (int) eBuilding)
					bFound = true;
			}
			
			if (bFound)
			{
				int iToMove = iI + 1;
				if (iToMove < m_iShrineBuildingCount)
				{
					m_aiShrineBuilding[iI] = m_aiShrineBuilding[iToMove];
					m_aiShrineReligion[iI] = m_aiShrineReligion[iToMove];
				}
				else
				{
					m_aiShrineBuilding[iI] = (int) NO_BUILDING;
					m_aiShrineReligion[iI] = (int) NO_RELIGION;
				}
			}

		if (bFound)
			m_iShrineBuildingCount--;

		}
	}
	else if (m_iShrineBuildingCount < GC.getNumBuildingInfos())
	{
		// add this item to the end
		m_aiShrineBuilding[m_iShrineBuildingCount] = eBuilding;
		m_aiShrineReligion[m_iShrineBuildingCount] = eReligion;
		m_iShrineBuildingCount++;
	}
	
}

bool CvGame::culturalVictoryValid()
{
	if (m_iNumCultureVictoryCities > 0)
	{
		return true;
	}

	return false;
}

int CvGame::culturalVictoryNumCultureCities()
{
	return m_iNumCultureVictoryCities;
}

CultureLevelTypes CvGame::culturalVictoryCultureLevel()
{
	if (m_iNumCultureVictoryCities > 0)
	{
		return (CultureLevelTypes) m_eCultureVictoryCultureLevel;
	}
	
	return NO_CULTURELEVEL;
}

int CvGame::getCultureThreshold(CultureLevelTypes eLevel) const
{
	int iThreshold = GC.getCultureLevelInfo(eLevel).getSpeedThreshold(getGameSpeedType());
	if (isOption(GAMEOPTION_NO_ESPIONAGE))
	{
		iThreshold *= 100 + GC.getDefineINT("NO_ESPIONAGE_CULTURE_LEVEL_MODIFIER");
		iThreshold /= 100;
	}
	return iThreshold;
}

void CvGame::doUpdateCacheOnTurn()
{
	MEMORY_TRACE_FUNCTION();
	int	iI;
	
	// reset shrine count
	m_iShrineBuildingCount = 0;

	for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
	{
		CvBuildingInfo&	kBuildingInfo = GC.getBuildingInfo((BuildingTypes) iI);
		
		// if it is for holy city, then its a shrine-thing, add it
		if (kBuildingInfo.getHolyCity() != NO_RELIGION)
		{
			changeShrineBuilding((BuildingTypes) iI, (ReligionTypes) kBuildingInfo.getReligionType());
		}
	}

	// reset cultural victories
	m_iNumCultureVictoryCities = 0;
	for (iI = 0; iI < GC.getNumVictoryInfos(); iI++)
	{
		if (isVictoryValid((VictoryTypes) iI))
		{
			CvVictoryInfo& kVictoryInfo = GC.getVictoryInfo((VictoryTypes) iI);
			if (kVictoryInfo.getCityCulture() > 0)
			{
				int iNumCultureCities = kVictoryInfo.getNumCultureCities();
				if (iNumCultureCities > m_iNumCultureVictoryCities)
				{
					m_iNumCultureVictoryCities = iNumCultureCities;
					m_eCultureVictoryCultureLevel = kVictoryInfo.getCityCulture();
				}
			}
		}
	}
}

VoteSelectionData* CvGame::getVoteSelection(int iID) const
{
	return ((VoteSelectionData*)(m_voteSelections.getAt(iID)));
}

VoteSelectionData* CvGame::addVoteSelection(VoteSourceTypes eVoteSource)
{
	VoteSelectionData* pData = ((VoteSelectionData*)(m_voteSelections.add()));

	if  (NULL != pData)
	{
		pData->eVoteSource = eVoteSource;

		for (int iI = 0; iI < GC.getNumVoteInfos(); iI++)
		{
			if (GC.getVoteInfo((VoteTypes)iI).isVoteSourceType(eVoteSource))
			{
				if (isChooseElection((VoteTypes)iI))
				{
					VoteSelectionSubData kData;
					kData.eVote = (VoteTypes)iI;
					kData.iCityId = -1;
					kData.ePlayer = NO_PLAYER;
					kData.eOtherPlayer = NO_PLAYER;

					if (GC.getVoteInfo(kData.eVote).isOpenBorders())
					{
						if (isValidVoteSelection(eVoteSource, kData))
						{
							kData.szText = gDLL->getText("TXT_KEY_POPUP_ELECTION_OPEN_BORDERS", getVoteRequired(kData.eVote, eVoteSource), countPossibleVote(kData.eVote, eVoteSource));
							pData->aVoteOptions.push_back(kData);
						}
					}
					else if (GC.getVoteInfo(kData.eVote).isDefensivePact())
					{
						if (isValidVoteSelection(eVoteSource, kData))
						{
							kData.szText = gDLL->getText("TXT_KEY_POPUP_ELECTION_DEFENSIVE_PACT", getVoteRequired(kData.eVote, eVoteSource), countPossibleVote(kData.eVote, eVoteSource));
							pData->aVoteOptions.push_back(kData);
						}
					}
					else if (GC.getVoteInfo(kData.eVote).isForcePeace())
					{
						for (int iTeam1 = 0; iTeam1 < MAX_CIV_TEAMS; ++iTeam1)
						{
							CvTeam& kTeam1 = GET_TEAM((TeamTypes)iTeam1);

							if (kTeam1.isAlive())
							{
								kData.ePlayer = kTeam1.getLeaderID();

								if (isValidVoteSelection(eVoteSource, kData))
								{
									kData.szText = gDLL->getText("TXT_KEY_POPUP_ELECTION_FORCE_PEACE", kTeam1.getName().GetCString(), getVoteRequired(kData.eVote, eVoteSource), countPossibleVote(kData.eVote, eVoteSource));
									pData->aVoteOptions.push_back(kData);
								}
							}
						}
					}
					else if (GC.getVoteInfo(kData.eVote).isForceNoTrade())
					{
						for (int iTeam1 = 0; iTeam1 < MAX_CIV_TEAMS; ++iTeam1)
						{
							CvTeam& kTeam1 = GET_TEAM((TeamTypes)iTeam1);

							if (kTeam1.isAlive())
							{
								kData.ePlayer = kTeam1.getLeaderID();

								if (isValidVoteSelection(eVoteSource, kData))
								{
									kData.szText = gDLL->getText("TXT_KEY_POPUP_ELECTION_FORCE_NO_TRADE", kTeam1.getName().GetCString(), getVoteRequired(kData.eVote, eVoteSource), countPossibleVote(kData.eVote, eVoteSource));
									pData->aVoteOptions.push_back(kData);
								}
							}
						}
					}
					else if (GC.getVoteInfo(kData.eVote).isForceWar())
					{
						for (int iTeam1 = 0; iTeam1 < MAX_CIV_TEAMS; ++iTeam1)
						{
							CvTeam& kTeam1 = GET_TEAM((TeamTypes)iTeam1);

							if (kTeam1.isAlive())
							{
								kData.ePlayer = kTeam1.getLeaderID();

								if (isValidVoteSelection(eVoteSource, kData))
								{
									kData.szText = gDLL->getText("TXT_KEY_POPUP_ELECTION_FORCE_WAR", kTeam1.getName().GetCString(), getVoteRequired(kData.eVote, eVoteSource), countPossibleVote(kData.eVote, eVoteSource));
									pData->aVoteOptions.push_back(kData);
								}
							}
						}
					}
					else if (GC.getVoteInfo(kData.eVote).isAssignCity())
					{
						for (int iPlayer1 = 0; iPlayer1 < MAX_CIV_PLAYERS; ++iPlayer1)
						{
							CvPlayer& kPlayer1 = GET_PLAYER((PlayerTypes)iPlayer1);

							if ( kPlayer1.isAlive() )
							{
								int iLoop = 0;
								for (CvCity* pLoopCity = kPlayer1.firstCity(&iLoop); NULL != pLoopCity; pLoopCity = kPlayer1.nextCity(&iLoop))
								{
									PlayerTypes eNewOwner = pLoopCity->plot()->findHighestCulturePlayer();
									if (NO_PLAYER != eNewOwner)
									{
										kData.ePlayer = (PlayerTypes)iPlayer1;
										kData.iCityId =	pLoopCity->getID();
										kData.eOtherPlayer = eNewOwner;

										if (isValidVoteSelection(eVoteSource, kData))
										{
											kData.szText = gDLL->getText("TXT_KEY_POPUP_ELECTION_ASSIGN_CITY", kPlayer1.getCivilizationAdjectiveKey(), pLoopCity->getNameKey(), GET_PLAYER(eNewOwner).getNameKey(), getVoteRequired(kData.eVote, eVoteSource), countPossibleVote(kData.eVote, eVoteSource));
											pData->aVoteOptions.push_back(kData);
										}
									}
								}
							}
						}
					}
					else
					{
						kData.szText = gDLL->getText("TXT_KEY_POPUP_ELECTION_OPTION", GC.getVoteInfo(kData.eVote).getTextKeyWide(), getVoteRequired(kData.eVote, eVoteSource), countPossibleVote(kData.eVote, eVoteSource));
						if (isVotePassed(kData.eVote))
						{
							kData.szText += gDLL->getText("TXT_KEY_POPUP_PASSED");
						}

						if (canDoResolution(eVoteSource, kData))
						{
							pData->aVoteOptions.push_back(kData);
						}
					}
				}
			}
		}

		if (pData->aVoteOptions.empty())
		{
			deleteVoteSelection(pData->getID());
			pData = NULL;
		}
	}

	return pData;
}

void CvGame::deleteVoteSelection(int iID)
{
	m_voteSelections.removeAt(iID);
}

VoteTriggeredData* CvGame::getVoteTriggered(int iID) const
{
	return ((VoteTriggeredData*)(m_votesTriggered.getAt(iID)));
}

VoteTriggeredData* CvGame::addVoteTriggered(const VoteSelectionData& kData, int iChoice)
{
	if (-1 == iChoice || iChoice >= (int)kData.aVoteOptions.size())
	{
		return NULL;
	}
	
	return addVoteTriggered(kData.eVoteSource, kData.aVoteOptions[iChoice]);
}

VoteTriggeredData* CvGame::addVoteTriggered(VoteSourceTypes eVoteSource, const VoteSelectionSubData& kOptionData)
{
	VoteTriggeredData* pData = ((VoteTriggeredData*)(m_votesTriggered.add()));

	if (NULL != pData)
	{
		pData->eVoteSource = eVoteSource;
		pData->kVoteOption = kOptionData;

		for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iI);
			if (kPlayer.isAlive() && kPlayer.isVotingMember(eVoteSource))
			{
				if (kPlayer.isHuman())
				{
					//45deg: added DarkLunaPhantom code - Human vassals are forced to vote for their master in the same manner as AI vassals are.
					bool bForced = false;
					
					if (GC.getGameINLINE().isTeamVote(kOptionData.eVote))
					{
						for (int iJ = 0; iJ < MAX_TEAMS; iJ++)
						{
							if (GET_TEAM((TeamTypes)iJ).isAlive())
							{
								if (GC.getGameINLINE().isTeamVoteEligible((TeamTypes)iJ, eVoteSource))
								{
									if (GET_TEAM(kPlayer.getTeam()).isVassal((TeamTypes)iJ))
									{
										if (!GC.getGameINLINE().isTeamVoteEligible(kPlayer.getTeam(), eVoteSource))
										{
											castVote(((PlayerTypes)iI), pData->getID(), GET_PLAYER((PlayerTypes)iI).AI_diploVote(kOptionData, eVoteSource, false));
											bForced = true;
											break;
										}
									}
								}
							}
						}
					}
					
					if (!bForced)
 					{
						CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_DIPLOVOTE);
						if (NULL != pInfo)
						{
							pInfo->setData1(pData->getID());
							gDLL->getInterfaceIFace()->addPopup(pInfo, (PlayerTypes)iI);
						}
 					}
					// DarkLunaPhantom - end
 				}
				else
				{
					castVote(((PlayerTypes)iI), pData->getID(), GET_PLAYER((PlayerTypes)iI).AI_diploVote(kOptionData, eVoteSource, false));
				}
			}
		}
	}

	return pData;
}

void CvGame::deleteVoteTriggered(int iID)
{
	m_votesTriggered.removeAt(iID);
}

void CvGame::doVoteResults()
{
	int iLoop = 0;
	for (VoteTriggeredData* pVoteTriggered = m_votesTriggered.beginIter(&iLoop); NULL != pVoteTriggered; pVoteTriggered = m_votesTriggered.nextIter(&iLoop))
	{
		CvWString szBuffer;
		CvWString szMessage;
		VoteTypes eVote = pVoteTriggered->kVoteOption.eVote;
		VoteSourceTypes eVoteSource = pVoteTriggered->eVoteSource;
		bool bPassed = false;

		if (!canDoResolution(eVoteSource, pVoteTriggered->kVoteOption))
		{
			for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
			{
				CvPlayer& kPlayer = GET_PLAYER((PlayerTypes) iPlayer);
				if (kPlayer.isAlive() && kPlayer.isVotingMember(eVoteSource))
				{
					MEMORY_TRACK_EXEMPT();

					CvWString szMessage;
					szMessage.Format(L"%s: %s", gDLL->getText("TXT_KEY_ELECTION_CANCELLED").GetCString(), GC.getVoteInfo(eVote).getDescription());
					AddDLLMessage((PlayerTypes)iPlayer, false, GC.getEVENT_MESSAGE_TIME(), szMessage, "AS2D_NEW_ERA", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));					
				}
			}
		}
		else
		{
			bool bAllVoted = true;
			for (int iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
			{
				PlayerTypes ePlayer = (PlayerTypes) iJ;
				if (GET_PLAYER(ePlayer).isAlive() && GET_PLAYER(ePlayer).isVotingMember(eVoteSource))
				{
					if (getPlayerVote(ePlayer, pVoteTriggered->getID()) == NO_PLAYER_VOTE)
					{
						//give player one more turn to submit vote
						setPlayerVote(ePlayer, pVoteTriggered->getID(), NO_PLAYER_VOTE_CHECKED);
						bAllVoted = false;
						break;
					}
					else if (getPlayerVote(ePlayer, pVoteTriggered->getID()) == NO_PLAYER_VOTE_CHECKED)
					{
						//default player vote to abstain
						setPlayerVote(ePlayer, pVoteTriggered->getID(), PLAYER_VOTE_ABSTAIN);
					}
				}
			}

			if (!bAllVoted)
			{
				continue;
			}

			if (isTeamVote(eVote))
			{
				TeamTypes eTeam = findHighestVoteTeam(*pVoteTriggered);

				if (NO_TEAM != eTeam)
				{
					bPassed = countVote(*pVoteTriggered, (PlayerVoteTypes)eTeam) >= getVoteRequired(eVote, eVoteSource);
				}

				szBuffer = GC.getVoteInfo(eVote).getDescription();

				if (eTeam != NO_TEAM)
				{
					szBuffer += NEWLINE + gDLL->getText("TXT_KEY_POPUP_DIPLOMATIC_VOTING_VICTORY", GET_TEAM(eTeam).getName().GetCString(), countVote(*pVoteTriggered, (PlayerVoteTypes)eTeam), getVoteRequired(eVote, eVoteSource), countPossibleVote(eVote, eVoteSource));
				}

				for (int iI = MAX_CIV_TEAMS; iI >= 0; --iI)
				{
					for (int iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
					{
						if (GET_PLAYER((PlayerTypes)iJ).isAlive() && GET_PLAYER((PlayerTypes)iJ).isVotingMember(eVoteSource))
						{
							if (getPlayerVote(((PlayerTypes)iJ), pVoteTriggered->getID()) == (PlayerVoteTypes)iI)
							{
								szBuffer += NEWLINE + gDLL->getText("TXT_KEY_POPUP_VOTES_FOR", GET_PLAYER((PlayerTypes)iJ).getNameKey(), GET_TEAM((TeamTypes)iI).getName().GetCString(), GET_PLAYER((PlayerTypes)iJ).getVotes(eVote, eVoteSource));
							}
						}
					}
				}

				for (int iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
				{
					if (GET_PLAYER((PlayerTypes)iJ).isAlive() && GET_PLAYER((PlayerTypes)iJ).isVotingMember(eVoteSource))
					{
						if (getPlayerVote(((PlayerTypes)iJ), pVoteTriggered->getID()) == PLAYER_VOTE_ABSTAIN)
						{
							szBuffer += NEWLINE + gDLL->getText("TXT_KEY_POPUP_ABSTAINS", GET_PLAYER((PlayerTypes)iJ).getNameKey(), GET_PLAYER((PlayerTypes)iJ).getVotes(eVote, eVoteSource));
						}
					}
				}

				if (NO_TEAM != eTeam && bPassed)
				{
					setVoteOutcome(*pVoteTriggered, (PlayerVoteTypes)eTeam);
				}
				else
				{
					setVoteOutcome(*pVoteTriggered, PLAYER_VOTE_ABSTAIN);
				}
			}
			else
			{
				bPassed = countVote(*pVoteTriggered, PLAYER_VOTE_YES) >= getVoteRequired(eVote, eVoteSource);

				// Defying resolution
				if (bPassed)
				{
					for (int iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
					{
						if (getPlayerVote((PlayerTypes)iJ, pVoteTriggered->getID()) == PLAYER_VOTE_NEVER)
						{
							bPassed = false;

							GET_PLAYER((PlayerTypes)iJ).setDefiedResolution(eVoteSource, pVoteTriggered->kVoteOption);
						}
					}
				}

				if (bPassed)
				{
					for (int iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
					{
						if (GET_PLAYER((PlayerTypes)iJ).isAlive() && GET_PLAYER((PlayerTypes)iJ).isVotingMember(eVoteSource))
						{
							if (getPlayerVote(((PlayerTypes)iJ), pVoteTriggered->getID()) == PLAYER_VOTE_YES)
							{
								GET_PLAYER((PlayerTypes)iJ).setEndorsedResolution(eVoteSource, pVoteTriggered->kVoteOption);
							}
						}
					}
				}

				szBuffer += NEWLINE + gDLL->getText((bPassed ? "TXT_KEY_POPUP_DIPLOMATIC_VOTING_SUCCEEDS" : "TXT_KEY_POPUP_DIPLOMATIC_VOTING_FAILURE"), GC.getVoteInfo(eVote).getTextKeyWide(), countVote(*pVoteTriggered, PLAYER_VOTE_YES), getVoteRequired(eVote, eVoteSource), countPossibleVote(eVote, eVoteSource));

				for (int iI = PLAYER_VOTE_NEVER; iI <= PLAYER_VOTE_YES; ++iI)
				{
					for (int iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
					{
						if (GET_PLAYER((PlayerTypes)iJ).isAlive() && GET_PLAYER((PlayerTypes)iJ).isVotingMember(eVoteSource))
						{
							if (getPlayerVote(((PlayerTypes)iJ), pVoteTriggered->getID()) == (PlayerVoteTypes)iI)
							{
								switch ((PlayerVoteTypes)iI)
								{
								case PLAYER_VOTE_ABSTAIN:
									szBuffer += NEWLINE + gDLL->getText("TXT_KEY_POPUP_ABSTAINS", GET_PLAYER((PlayerTypes)iJ).getNameKey(), GET_PLAYER((PlayerTypes)iJ).getVotes(eVote, eVoteSource));
									break;
								case PLAYER_VOTE_NEVER:
									szBuffer += NEWLINE + gDLL->getText("TXT_KEY_POPUP_VOTES_YES_NO", GET_PLAYER((PlayerTypes)iJ).getNameKey(), L"TXT_KEY_POPUP_VOTE_NEVER", GET_PLAYER((PlayerTypes)iJ).getVotes(eVote, eVoteSource));
									break;
								case PLAYER_VOTE_NO:
									szBuffer += NEWLINE + gDLL->getText("TXT_KEY_POPUP_VOTES_YES_NO", GET_PLAYER((PlayerTypes)iJ).getNameKey(), L"TXT_KEY_POPUP_NO", GET_PLAYER((PlayerTypes)iJ).getVotes(eVote, eVoteSource));
									break;
								case PLAYER_VOTE_YES:
									szBuffer += NEWLINE + gDLL->getText("TXT_KEY_POPUP_VOTES_YES_NO", GET_PLAYER((PlayerTypes)iJ).getNameKey(), L"TXT_KEY_POPUP_YES", GET_PLAYER((PlayerTypes)iJ).getVotes(eVote, eVoteSource));
									break;
								default:
									FAssert(false);
									break;
								}
							}
						}
					}
				}

				setVoteOutcome(*pVoteTriggered, bPassed ? PLAYER_VOTE_YES : PLAYER_VOTE_NO);
			}

			for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
			{
				CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iI);
				if (kPlayer.isHuman())
				{
					bool bShow = kPlayer.isVotingMember(pVoteTriggered->eVoteSource);

					if (bShow)
					{
						CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_TEXT);
						if (NULL != pInfo)
						{
							pInfo->setText(szBuffer);
							gDLL->getInterfaceIFace()->addPopup(pInfo, (PlayerTypes)iI);
						}
					}

					if (!bShow)
					{
						if (iI == pVoteTriggered->kVoteOption.ePlayer && GET_PLAYER(pVoteTriggered->kVoteOption.ePlayer).isVotingMember(pVoteTriggered->eVoteSource))
						{
							bShow = true;
						}
					}

					if (!bShow)
					{
						if (iI == pVoteTriggered->kVoteOption.eOtherPlayer && GET_PLAYER(pVoteTriggered->kVoteOption.eOtherPlayer).isVotingMember(pVoteTriggered->eVoteSource))
						{
							bShow = true;
						}
					}

					if (bShow && bPassed)
					{
						MEMORY_TRACK_EXEMPT();

						CvWString szMessage = gDLL->getText("TXT_KEY_VOTE_RESULTS", GC.getVoteSourceInfo(eVoteSource).getTextKeyWide(), pVoteTriggered->kVoteOption.szText.GetCString());
						AddDLLMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szMessage, "AS2D_NEW_ERA", MESSAGE_TYPE_MINOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"));
					}

				}
			}
		}

		if (!bPassed && GC.getVoteInfo(eVote).isSecretaryGeneral())
		{
			setSecretaryGeneralTimer(eVoteSource, 0);
		}

		deleteVoteTriggered(pVoteTriggered->getID());
	}
}

void CvGame::doVoteSelection()
{
	for (int iI = 0; iI < GC.getNumVoteSourceInfos(); ++iI)
	{
		VoteSourceTypes eVoteSource = (VoteSourceTypes)iI;

		if (isDiploVote(eVoteSource))
		{
			if (getVoteTimer(eVoteSource) > 0)
			{
				changeVoteTimer(eVoteSource, -1);
			}
			else
			{
/************************************************************************************************/
/* Afforess	                  Start		 06/26/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
				setVoteTimer(eVoteSource, (GC.getVoteSourceInfo(eVoteSource).getVoteInterval() * GC.getGameSpeedInfo(getGameSpeedType()).getVictoryDelayPercent()) / 100);
*/
				setVoteTimer(eVoteSource, (GC.getVoteSourceInfo(eVoteSource).getVoteInterval() * GC.getGameSpeedInfo(getGameSpeedType()).getTrainPercent()) / 100);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
				for (int iTeam1 = 0; iTeam1 < MAX_CIV_TEAMS; ++iTeam1)
				{
					CvTeam& kTeam1 = GET_TEAM((TeamTypes)iTeam1);

					if (kTeam1.isAlive() && kTeam1.isVotingMember(eVoteSource))
					{
						for (int iTeam2 = iTeam1 + 1; iTeam2 < MAX_CIV_TEAMS; ++iTeam2)
						{
							CvTeam& kTeam2 = GET_TEAM((TeamTypes)iTeam2);

							if (kTeam2.isAlive() && kTeam2.isVotingMember(eVoteSource))
							{
								kTeam1.meet((TeamTypes)iTeam2, true);
							}
						}
					}
				}

				TeamTypes eSecretaryGeneral = getSecretaryGeneral(eVoteSource);
				PlayerTypes eSecretaryPlayer;

				if (eSecretaryGeneral != NO_TEAM)
				{
					eSecretaryPlayer = GET_TEAM(eSecretaryGeneral).getSecretaryID();
				}
				else
				{
					eSecretaryPlayer = NO_PLAYER;
				}

				bool bSecretaryGeneralVote = false;
				if (canHaveSecretaryGeneral(eVoteSource))
				{
					if (getSecretaryGeneralTimer(eVoteSource) > 0)
					{
						changeSecretaryGeneralTimer(eVoteSource, -1);
					}
					else
					{
						setSecretaryGeneralTimer(eVoteSource, GC.getDefineINT("DIPLO_VOTE_SECRETARY_GENERAL_INTERVAL"));

						for (int iJ = 0; iJ < GC.getNumVoteInfos(); iJ++)
						{
							if (GC.getVoteInfo((VoteTypes)iJ).isSecretaryGeneral() && GC.getVoteInfo((VoteTypes)iJ).isVoteSourceType(iI))
							{
								VoteSelectionSubData kOptionData;
								kOptionData.iCityId = -1;
								kOptionData.ePlayer = NO_PLAYER;
								kOptionData.eVote = (VoteTypes)iJ;
								kOptionData.szText = gDLL->getText("TXT_KEY_POPUP_ELECTION_OPTION", GC.getVoteInfo((VoteTypes)iJ).getTextKeyWide(), getVoteRequired((VoteTypes)iJ, eVoteSource), countPossibleVote((VoteTypes)iJ, eVoteSource));
								addVoteTriggered(eVoteSource, kOptionData);
								bSecretaryGeneralVote = true;
								break;
							}
						}
					}
				}

				if (!bSecretaryGeneralVote && eSecretaryGeneral != NO_TEAM && eSecretaryPlayer != NO_PLAYER)
				{
					VoteSelectionData* pData = addVoteSelection(eVoteSource);
					if (NULL != pData)
					{
						if (GET_PLAYER(eSecretaryPlayer).isHuman())
						{
							CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CHOOSEELECTION);
							if (NULL != pInfo)
							{
								pInfo->setData1(pData->getID());
								gDLL->getInterfaceIFace()->addPopup(pInfo, eSecretaryPlayer);
							}
						}
						else
						{
							setVoteChosen(GET_TEAM(eSecretaryGeneral).AI_chooseElection(*pData), pData->getID());
						}
					}
					else
					{
						setVoteTimer(eVoteSource, 0);
					}
				}
			}
		}
	}
}

bool CvGame::isEventActive(EventTriggerTypes eTrigger) const
{
	for (std::vector<EventTriggerTypes>::const_iterator it = m_aeInactiveTriggers.begin(); it != m_aeInactiveTriggers.end(); ++it)
	{
		if (*it == eTrigger)
		{
			return false;
		}
	}

	return true;
}

void CvGame::initEvents()
{
	for (int iTrigger = 0; iTrigger < GC.getNumEventTriggerInfos(); ++iTrigger)
	{
		if (isOption(GAMEOPTION_NO_EVENTS) || getSorenRandNum(100, "Event Active?") >= GC.getEventTriggerInfo((EventTriggerTypes)iTrigger).getPercentGamesActive())
		{
			m_aeInactiveTriggers.push_back((EventTriggerTypes)iTrigger);
		}
	}
}

bool CvGame::isCivEverActive(CivilizationTypes eCivilization) const
{
	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
		if (kLoopPlayer.isEverAlive())
		{
			if (kLoopPlayer.getCivilizationType() == eCivilization)
			{
				return true;
			}
		}
	}

	return false;
}

bool CvGame::isLeaderEverActive(LeaderHeadTypes eLeader) const
{
	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
		if (kLoopPlayer.isEverAlive())
		{
			if (kLoopPlayer.getLeaderType() == eLeader)
			{
				return true;
			}
		}
	}

	return false;
}

bool CvGame::isUnitEverActive(UnitTypes eUnit) const
{
	for (int iCiv = 0; iCiv < GC.getNumCivilizationInfos(); ++iCiv)
	{
		if (isCivEverActive((CivilizationTypes)iCiv))
		{
			if (eUnit == GC.getCivilizationInfo((CivilizationTypes)iCiv).getCivilizationUnits(GC.getUnitInfo(eUnit).getUnitClassType()))
			{
				return true;
			}
		}
	}

	return false;
}

bool CvGame::isBuildingEverActive(BuildingTypes eBuilding) const
{
	for (int iCiv = 0; iCiv < GC.getNumCivilizationInfos(); ++iCiv)
	{
		if (isCivEverActive((CivilizationTypes)iCiv))
		{
			if (eBuilding == GC.getCivilizationInfo((CivilizationTypes)iCiv).getCivilizationBuildings(GC.getBuildingInfo(eBuilding).getBuildingClassType()))
			{
				return true;
			}
		}
	}

	return false;
}

void CvGame::processBuilding(BuildingTypes eBuilding, int iChange)
{
	PROFILE_FUNC();

	for (int iI = 0; iI < GC.getNumVoteSourceInfos(); ++iI)
	{
		if (GC.getBuildingInfo(eBuilding).getVoteSourceType() == (VoteSourceTypes)iI)
		{
			changeDiploVote((VoteSourceTypes)iI, iChange);
		}
	}
}

bool CvGame::pythonIsBonusIgnoreLatitudes() const
{
	long lResult = -1;
	if (PYTHON_CALL_FUNCTION4(__FUNCTION__, gDLL->getPythonIFace()->getMapScriptModule(), "isBonusIgnoreLatitude", NULL, &lResult))
	{
		if (!gDLL->getPythonIFace()->pythonUsingDefaultImpl() && lResult != -1)
		{
			return (lResult != 0);
		}
	}

	return false;
}

// RevolutionDCM - BarbarianWorld - thanks Kael
void CvGame::foundBarbarianCity()
{
    bool bValid = true;
	int iBestValue = 0;
	int iDist = 0;
	int iValue = 0;
	CvPlot* pPlotI = NULL;
	CvPlot* pBestPlot = NULL;
	CvPlot* pLoopPlot = NULL;
	for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		bValid = true;
		iValue = 0;
		if (pLoopPlot->isWater())
		{
		    bValid = false;
		}
		else if (pLoopPlot->isImpassable())
		{
		    bValid = false;
		}
		else if (pLoopPlot->isCity())
		{
		    bValid = false;
		}
		else if (pLoopPlot->getImprovementType() != NO_IMPROVEMENT)
		{
		    bValid = false;
		}
		else 
		{
			bool bOccupiedArea = false;
			for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).isAlive())
				{
					pPlotI = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

					if (pPlotI != NULL)
					{
						iDist = GC.getMapINLINE().calculatePathDistance(pPlotI, pLoopPlot);
						if (iDist >= 0)
						{
							bOccupiedArea = true;
						
							if (iDist < 5)
							{
								bValid = false;
							}
							else if( iDist < 10 )
							{
								iValue -= (10 - iDist)*7;
							}
						}
					}
				}
			}

			if( bOccupiedArea )
			{
				iValue += (!isOption(GAMEOPTION_NO_BARBARIAN_CIV) ? 1000 : 250);
			}
			else
			{
				if( !isOption(GAMEOPTION_NO_BARBARIAN_CIV) )
				{
					bValid = false;
				}
			}

		}

        if (bValid)
        {
            iValue += GET_PLAYER(BARBARIAN_PLAYER).AI_foundValue(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), GC.getDefineINT("MIN_BARBARIAN_CITY_STARTING_DISTANCE"));
            iValue += getSorenRandNum(250, "Barb City Found");
			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				pBestPlot = pLoopPlot;
			}
		}
	}

	if (pBestPlot != NULL)
	{
		GET_PLAYER(BARBARIAN_PLAYER).found(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
	}
}
// RevolutionDCM End


/************************************************************************************************/
/* RevolutionDCM	                  Start		 05/31/10                        Afforess       */
/*                                                                                              */
/* Battle Effects                                                                               */
/************************************************************************************************/
void CvGame::drawBattleEffects()
{
	if (GC.isDCM_BATTLE_EFFECTS())
	{
		CvPlot* pLoopPlot;
		for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
		{
			pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
			if (pLoopPlot->isBattle())
			{
				gDLL->getEngineIFace()->TriggerEffect(pLoopPlot->getBattleEffect(), pLoopPlot->getPoint(), 0);
			}
		}
	}
}
/************************************************************************************************/
/* RevolutionDCM	             Battle Effects END                                             */
/************************************************************************************************/

// BUG - MapFinder - start
//#include "ximage.h"

// from HOF Mod - Dianthus
bool CvGame::takeJPEGScreenShot(std::string fileName) const
{
//RevolutionDCM - start BULL link problems
	//HWND hwnd = GetDesktopWindow();
	//RECT r;
	//GetWindowRect(hwnd,&r);

	//int xScreen,yScreen;	//check if the window is out of the screen or maximixed <Qiang>
	//int xshift = 0, yshift = 0;
	//xScreen = GetSystemMetrics(SM_CXSCREEN);
	//yScreen = GetSystemMetrics(SM_CYSCREEN);
	//if(r.right > xScreen)
	//		r.right = xScreen;
	//if(r.bottom > yScreen)
	//		r.bottom = yScreen;
	//if(r.left < 0){
	//		xshift = -r.left;
	//		r.left = 0;
	//}
	//if(r.top < 0){
	//		yshift = -r.top;
	//		r.top = 0;
	//}

	//int w=r.right-r.left;
	//int h=r.bottom-r.top;
	//if(w <= 0 || h <= 0) return false;

	//// bring the window at the top most level
	//// TODO ::SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

	//// prepare the DCs
	//HDC dstDC = ::GetDC(NULL);
	//HDC srcDC = ::GetWindowDC(hwnd); //full window (::GetDC(hwnd); = clientarea)
	//HDC memDC = ::CreateCompatibleDC(dstDC);

	//// copy the screen to the bitmap
	//HBITMAP bm =::CreateCompatibleBitmap(dstDC, w, h);
	//HBITMAP oldbm = (HBITMAP)::SelectObject(memDC,bm);
	//::BitBlt(memDC, 0, 0, w, h, srcDC, xshift, yshift, SRCCOPY);

	//// restore the position
	//// TODO ::SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	//// TODO ::SetWindowPos(m_hWnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

	//CxImage image;
	//image.CreateFromHBITMAP(bm);
	//bool result = image.Save((const TCHAR*)fileName.c_str(),CXIMAGE_FORMAT_JPG);

	//// free objects
	//DeleteObject(SelectObject(memDC,oldbm));
	//DeleteObject(memDC);

	//return result;
	return false;
//RevolutionDCM - end BULL link problems
}
// BUG - MapFinder - end

/************************************************************************************************/
/* Afforess                                     12/7/09                                         */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
int CvGame::getWaterAnimalSpawnChance() const
{
	return m_iWaterAnimalSpawnChance;
}

void CvGame::setWaterAnimalSpawnChance(int iNewValue)
{
	m_iWaterAnimalSpawnChance = iNewValue;
}

void CvGame::changeWaterAnimalSpawnChance(int iChange)
{
	setWaterAnimalSpawnChance(getWaterAnimalSpawnChance() + iChange);
}

#if defined QC_MASTERY_VICTORY

bool CvGame::getStarshipLaunched(int ID)
{
  CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)ID);
  
	return starshipLaunched[kPlayer.getTeam()];
}

bool CvGame::getDiplomaticVictoryAchieved(int ID)
{
  CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)ID);
  	
	return diplomaticVictoryAchieved[kPlayer.getTeam()];
}

// Code Added for Ascension Gate & Mastery Victory (by 45°) - START

bool CvGame::getAscensionGateBuilt(int ID)
{
  CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)ID);
  
	return AscensionGateBuilt[kPlayer.getTeam()];
}

// Code Added for Ascension Gate & Mastery Victory (by 45°) - END

#endif

int CvGame::getCurrentVoteID() const
{
    return m_iCurrentVoteID;
}

void CvGame::setCurrentVoteID(int iNewValue)
{
    if (iNewValue != getCurrentVoteID())
    {
        m_iCurrentVoteID = iNewValue;
    }
}

int CvGame::getXResolution() const
{
	return m_iXResolution;
}

void CvGame::setXResolution(int iNewValue)
{
	m_iXResolution = iNewValue;
}

void CvGame::changeXResolution(int iChange)
{
	setXResolution(getXResolution() + iChange);
}

int CvGame::getYResolution() const
{
	return m_iYResolution;
}

void CvGame::setYResolution(int iNewValue)
{
	m_iYResolution = iNewValue;
}

void CvGame::changeYResolution(int iChange)
{
	setYResolution(getYResolution() + iChange);
}

int CvGame::getCutLosersCounter()
{
	return m_iCutLosersCounter;
}

void CvGame::changeCutLosersCounter(int iChange)
{
	m_iCutLosersCounter += iChange;
}

int CvGame::getHighToLowCounter()
{
	return m_iHighToLowCounter;
}

void CvGame::changeHighToLowCounter(int iChange)
{
	m_iHighToLowCounter += iChange;
}

int CvGame::getIncreasingDifficultyCounter()
{
	return m_iIncreasingDifficultyCounter;
}

void CvGame::changeIncreasingDifficultyCounter(int iChange)
{
	m_iIncreasingDifficultyCounter += iChange;
}

void CvGame::doFinalFive()
{
	MEMORY_TRACE_FUNCTION();

	int iI;
	if (!GC.getGameINLINE().isGameMultiPlayer())
	{
	    if (isOption(GAMEOPTION_CHALLENGE_CUT_LOSERS))
	    {
	        if (countCivPlayersAlive() > 5)
	        {
	            changeCutLosersCounter(1);
	            if (getCutLosersCounter() >= ((GC.getDefineINT("CUT_LOSERS_TURN_INCREMENT") * GC.getGameSpeedInfo(getGameSpeedType()).getAnarchyPercent()) / 100))
	            {
	                GET_PLAYER(getRankPlayer(countCivPlayersAlive() -1)).setAlive(false);
	                changeCutLosersCounter(getCutLosersCounter() * -1);
					 for (iI = 0; iI < MAX_PLAYERS; iI++)
	                {
	                    if (GET_PLAYER((PlayerTypes)iI).isAlive())
	                    {
							MEMORY_TRACK_EXEMPT();

							AddDLLMessage((PlayerTypes)iI, true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_LOW_PLAYER_DROPPED").GetCString(), "AS2D_FIRSTTOTECH", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT"));
						}
					}
	            }
	        }
	    }
	}
}

void CvGame::doHightoLow()
{
	MEMORY_TRACE_FUNCTION();

	int iI;
	if (isOption(GAMEOPTION_CHALLENGE_HIGH_TO_LOW))
	{
		if (!GC.getGameINLINE().isGameMultiPlayer())
		{
			if (getGameTurn() >= GC.getDefineINT("HIGH_TO_LOW_FIRST_TURN_CHECK"))
			{
				if (getHighToLowCounter() <  GC.getDefineINT("HIGH_TO_LOW_NUM_REASSIGNMENTS", 2))
				{
					for (iI = 0; iI < MAX_PLAYERS; iI++)
					{
						if (GET_PLAYER((PlayerTypes)iI).isAlive())
						{
							if (GET_PLAYER((PlayerTypes)iI).isHuman())
							{
								if (getPlayerRank((PlayerTypes)iI) == 0)
								{
									MEMORY_TRACK_EXEMPT();

									GC.getInitCore().reassignPlayerAdvanced((PlayerTypes)iI, getRankPlayer(countCivPlayersAlive() -1));
									changeHighToLowCounter(1);
									AddDLLMessage((PlayerTypes)iI, true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_PLAYER_REASSIGNED").GetCString(), "AS2D_WELOVEKING", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"));
								}
							}
						}
					}
				}
			}
		}
	}
}
void CvGame::doIncreasingDifficulty()
{
	MEMORY_TRACE_FUNCTION();

	CvCity* pLoopCity;
	int iLoop = 0;
	int iI;
	if (isOption(GAMEOPTION_CHALLENGE_INCREASING_DIFFICULTY))
	{
		changeIncreasingDifficultyCounter(1);
		if (getIncreasingDifficultyCounter() >= ((GC.getDefineINT("INCREASING_DIFFICULTY_TURN_CHECK_INCREMENTS") * GC.getGameSpeedInfo(getGameSpeedType()).getAnarchyPercent()) / 100))
		{
			if (getHandicapType() < (GC.getNumHandicapInfos() - 1))
			{
				setHandicapType((HandicapTypes)(getHandicapType() + 1));
				changeIncreasingDifficultyCounter(getIncreasingDifficultyCounter() * -1);
				 for (iI = 0; iI < MAX_PLAYERS; iI++)
				{
					if (GET_PLAYER((PlayerTypes)iI).isAlive())
					{
						if (GET_PLAYER((PlayerTypes)iI).isHuman())
						{
							GC.getInitCore().setHandicap((PlayerTypes)iI, (HandicapTypes)getHandicapType());
							GET_PLAYER((PlayerTypes)iI).AI_makeAssignWorkDirty();
							for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
							{
								pLoopCity->setInfoDirty(true);
							}
							MEMORY_TRACK_EXEMPT();

							AddDLLMessage((PlayerTypes)iI, true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_DIFFICULTY_INCREASED").GetCString(), "AS2D_FEAT_ACCOMPLISHED", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT"));
						}
					}
				}
			}
		}
	}
}
void CvGame::doFlexibleDifficulty()
{
	MEMORY_TRACE_FUNCTION();

	CvCity* pLoopCity;
	int iLoop = 0;
	int iI, iJ;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayerAI& kPlayer = GET_PLAYER((PlayerTypes)iI);
		CvTeamAI& kTeam = GET_TEAM(kPlayer.getTeam());
		//Check we are rebalancing the score for the team leader (and ignoring vassals)
		if (kPlayer.isAlive() && kTeam.getLeaderID() == kPlayer.getID() && !kPlayer.isBarbarian())
		{
			int iTurns = kPlayer.getModderOption(MODDEROPTION_FLEXIBLE_DIFFICULTY_TURN_INCREMENTS);
			int iTimer = getFlexibleDifficultyTimer((PlayerTypes)iI);
//			if (!isPitboss())
//			{
				if (iTurns <= 0 || (!kPlayer.isHuman() && isModderGameOption(MODDERGAMEOPTION_AI_USE_FLEXIBLE_DIFFICULTY)))
				{
					iTurns = GC.getDefineINT("DEFAULT_FLEXIBLE_DIFFICULTY_TURN_INCREMENTS", 10);
					iTurns *= GC.getGameSpeedInfo(getGameSpeedType()).getGoldenAgePercent();
					iTurns /= 100;
				}
//			}
			/*else	//45deg: hack for Pitboss games: using the same timer for both humans and AI, otherwise the game might go OOS
			{
				if (iTurns <= 0 || isModderGameOption(MODDERGAMEOPTION_AI_USE_FLEXIBLE_DIFFICULTY))
				{
					iTurns = GC.getDefineINT("DEFAULT_FLEXIBLE_DIFFICULTY_TURN_INCREMENTS", 15);
					iTurns *= GC.getGameSpeedInfo(getGameSpeedType()).getGoldenAgePercent();
					iTurns /= 180;
				}
			}			*/ //45deg:removed Pitboss hack, it doesn't work like this

			logMsg("[Flexible Difficulty] (%d / %d) turns until next flexible difficulty check for Player: %S", iTimer, iTurns, kPlayer.getName());
			
			//Increase timer
			iTimer++;
			setFlexibleDifficultyTimer((PlayerTypes)iI, iTimer);
			
			if (iTimer >= iTurns)
			{
				logMsg("[Flexible Difficulty] Player: %S, Checking Flexible Difficulty", kPlayer.getName());

				if (kPlayer.isModderOption(MODDEROPTION_FLEXIBLE_DIFFICULTY) || (!kPlayer.isHuman() && isModderGameOption(MODDERGAMEOPTION_AI_USE_FLEXIBLE_DIFFICULTY)) || gDLL->IsPitbossHost())
				{
					logMsg("[Flexible Difficulty] Player: %S has Flexible Difficulty Enabled", kPlayer.getName());
					int iMinHandiCap = kPlayer.getModderOption(MODDEROPTION_FLEXIBLE_DIFFICULTY_MIN_DIFFICULTY) - 1;
					int iMaxHandicap = kPlayer.getModderOption(MODDEROPTION_FLEXIBLE_DIFFICULTY_MAX_DIFFICULTY) - 1;
					if (iMaxHandicap < 0)
						iMaxHandicap = MAX_INT;

					if (!kPlayer.isHuman() && isModderGameOption(MODDERGAMEOPTION_AI_USE_FLEXIBLE_DIFFICULTY))
					{
//						iMinHandiCap = 0;
//						iMaxHandicap = GC.getNumHandicapInfos() - 1;
						iMinHandiCap = GC.getDefineINT("DEFAULT_FLEXIBLE_DIFFICULTY_AI_MIN_DIFFICULTY", -1);
						iMaxHandicap = GC.getDefineINT("DEFAULT_FLEXIBLE_DIFFICULTY_AI_MAX_DIFFICULTY", -1);
						if (iMaxHandicap < 0)
							iMaxHandicap = MAX_INT;
					}
						
					//Reset counter
					setFlexibleDifficultyTimer((PlayerTypes)iI, 0);

					//int iMinRank = kPlayer.getModderOption(MODDEROPTION_FLEXIBLE_DIFFICULTY_MIN_RANK);
					//int iMaxRank = kPlayer.getModderOption(MODDEROPTION_FLEXIBLE_DIFFICULTY_MAX_RANK);
					//if (iMaxRank < 0)
					//	iMaxRank = MAX_INT;
					//int iCurrentRank = getPlayerRank((PlayerTypes)iI);

					int iBestScore = MIN_INT;
					int iAliveCount = 0;
					int iTotalScore = 0;
					for (iJ = 0; iJ < MAX_PLAYERS; iJ++)
					{
						if (GET_PLAYER((PlayerTypes)iJ).isAlive() && GET_TEAM(GET_PLAYER((PlayerTypes)iJ).getTeam()).getLeaderID() == GET_PLAYER((PlayerTypes)iJ).getID() && !GET_PLAYER((PlayerTypes)iJ).isBarbarian())
						{
							if (GC.getGameINLINE().isOption(GAMEOPTION_NO_REVOLUTION))  //45deg: when calculating average score, we'll skip smaller rebel civs if Revolution is ON
							{
								iAliveCount++;
								int iScore = getPlayerScore((PlayerTypes)iJ);
								iTotalScore += iScore;
								if (iScore > iBestScore)
								{
									iBestScore = iScore;
								}
							}
							else 
							{
								if (getStartEra() == 0)	//45deg: check on StartEra for Advanced Starts
								{
									if (GET_PLAYER((PlayerTypes)iJ).getCurrentEra() > 1)
									{
										if (GET_PLAYER((PlayerTypes)iJ).getNumCities() > 1) 
										{
											iAliveCount++;
											int iScore = getPlayerScore((PlayerTypes)iJ);
											iTotalScore += iScore;
											if (iScore > iBestScore)
											{
												iBestScore = iScore;
											}
										}
									}
									else
									{
										iAliveCount++;
										int iScore = getPlayerScore((PlayerTypes)iJ);
										iTotalScore += iScore;
										if (iScore > iBestScore)
										{
											iBestScore = iScore;
										}
									}	
								}
								else
								{
									iAliveCount++;
									int iScore = getPlayerScore((PlayerTypes)iJ);
									iTotalScore += iScore;
									if (iScore > iBestScore)
									{
										iBestScore = iScore;
									}
								}								
							}							
						}
					}
					FAssertMsg(iAliveCount > 0, "iAliveCount can not be <= 0");
					int iMeanScore = iTotalScore / iAliveCount;
					double variance = 0.0;
					for (iJ = 0; iJ < MAX_PLAYERS; iJ++)
					{
						if (GET_PLAYER((PlayerTypes)iJ).isAlive() && GET_TEAM(GET_PLAYER((PlayerTypes)iJ).getTeam()).getLeaderID() == GET_PLAYER((PlayerTypes)iJ).getID() && !GET_PLAYER((PlayerTypes)iJ).isBarbarian())
						{
							int iScore = getPlayerScore((PlayerTypes)iJ);
							//variance is sum of squared difference from mean
							variance += (iMeanScore - iScore) * (iMeanScore - iScore);
						}
					}
					variance /= iAliveCount;
					double stddev = sqrt(variance);

					int iCurrentScore = getPlayerScore((PlayerTypes)iI);
					
					logMsg("[Flexible Difficulty] Player: %S, Score: %d, Difficulty: %S, Avg Score: %d, Std Dev: %f", kPlayer.getName(), iCurrentScore, GC.getHandicapInfo((HandicapTypes)kPlayer.getHandicapType()).getDescription(), iMeanScore, stddev);

					//Increased Difficulty (player's score is > 1 std dev away)
					if (iCurrentScore > (iMeanScore + stddev))
					{
						logMsg("[Flexible Difficulty] Player: %S score is > 1 std dev above average.", kPlayer.getName());
						//if (iCurrentRank < iMaxRank && iCurrentRank > iMinRank)
						{
							if (kPlayer.getHandicapType() < (GC.getNumHandicapInfos() - 1))
							{
								if (kPlayer.getHandicapType() < iMaxHandicap)
								{
									logMsg("[Flexible Difficulty] Player: %S difficulty is increasing from %S to %S", kPlayer.getName(), GC.getHandicapInfo((HandicapTypes)kPlayer.getHandicapType()).getDescription(), GC.getHandicapInfo((HandicapTypes)(kPlayer.getHandicapType() + 1)).getDescription());

									//Change the difficulty
									GC.getInitCore().setHandicap((PlayerTypes)iI, (HandicapTypes)(kPlayer.getHandicapType() + 1));

									//Clean the interface
									kPlayer.AI_makeAssignWorkDirty();
									for (pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
									{
										pLoopCity->setInfoDirty(true);
									}

									//Inform all human players of any other players difficulty changes
									for (iJ = 0; iJ < MAX_PLAYERS; iJ++)
									{
										if (GET_PLAYER((PlayerTypes)iJ).isAlive() && GET_PLAYER((PlayerTypes)iJ).isHuman() && GET_TEAM(GET_PLAYER((PlayerTypes)iJ).getTeam()).isHasMet(kPlayer.getTeam()))
										{
											//Difficulty increased for us
											if (iJ == iI)
											{
												MEMORY_TRACK_EXEMPT();
												AddDLLMessage((PlayerTypes)iJ, true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_DIFFICULTY_INCREASED").GetCString(), "AS2D_FEAT_ACCOMPLISHED", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT"));
											}
											//Difficulty increased for someone else
											else
											{
												MEMORY_TRACK_EXEMPT();
												AddDLLMessage((PlayerTypes)iJ, true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_DIFFICULTY_INCREASED_FOR_PLAYER", kPlayer.getCivilizationDescriptionKey()).GetCString(), "AS2D_FEAT_ACCOMPLISHED", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT"));
											}
										}
									}
								}
							}
						}
					}

					
					
// 45° - START
// Difficulty decreases if player score is between (average + 1 std dev) and (average - 1/2 std dev) and difficulty is above Noble			

					if (iCurrentScore < (iMeanScore + stddev) && (iCurrentScore > (iMeanScore - (stddev/2))) && kPlayer.getHandicapType() > GC.getDefineINT("STANDARD_HANDICAP"))
					{
						logMsg("[Flexible Difficulty] Player: %S score is < 1 std dev above average but above average.", kPlayer.getName());
						//if (iCurrentRank < iMaxRank && iCurrentRank > iMinRank)
						{
							if (kPlayer.getHandicapType() <= (GC.getNumHandicapInfos() - 1))
							{
								if (kPlayer.getHandicapType() <= iMaxHandicap)
								{
									logMsg("[Flexible Difficulty] Player: %S difficulty is decreasing from %S to %S", kPlayer.getName(), GC.getHandicapInfo((HandicapTypes)kPlayer.getHandicapType()).getDescription(), GC.getHandicapInfo((HandicapTypes)(kPlayer.getHandicapType() - 1)).getDescription());

									//Change the difficulty
									GC.getInitCore().setHandicap((PlayerTypes)iI, (HandicapTypes)(kPlayer.getHandicapType() - 1));

									//Clean the interface
									kPlayer.AI_makeAssignWorkDirty();
									for (pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
									{
										pLoopCity->setInfoDirty(true);
									}

									//Inform all human players of any other players difficulty changes
									for (iJ = 0; iJ < MAX_PLAYERS; iJ++)
									{
										if (GET_PLAYER((PlayerTypes)iJ).isAlive() && GET_PLAYER((PlayerTypes)iJ).isHuman() && GET_TEAM(GET_PLAYER((PlayerTypes)iJ).getTeam()).isHasMet(kPlayer.getTeam()))
										{
											//Difficulty decreased for us
											if (iJ == iI)
											{
												MEMORY_TRACK_EXEMPT();
												AddDLLMessage((PlayerTypes)iJ, true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_DIFFICULTY_DECREASED").GetCString(), "AS2D_FEAT_ACCOMPLISHED", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT"));
											}
											//Difficulty decreased for someone else
											else
											{
												MEMORY_TRACK_EXEMPT();
												AddDLLMessage((PlayerTypes)iJ, true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_DIFFICULTY_DECREASED_FOR_PLAYER", kPlayer.getCivilizationDescriptionKey()).GetCString(), "AS2D_FEAT_ACCOMPLISHED", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT"));
											}
										}
									}
								}
							}
						}
					}

// 45° - END		
					//Decreased Difficulty (player's score is > 1/2 std dev away)
					if (iCurrentScore < (iMeanScore - (stddev/2)))
					{
						logMsg("[Flexible Difficulty] Player: %S score is > 1/2 std dev below average.", kPlayer.getName());
						//if (iCurrentRank < iMaxRank && iCurrentRank > iMinRank)
						{
							if (kPlayer.getHandicapType() > 0)
							{
								if (kPlayer.getHandicapType() > iMinHandiCap)
								{
									logMsg("[Flexible Difficulty] Player: %S difficulty is decreasing from %S to %S", kPlayer.getName(), GC.getHandicapInfo((HandicapTypes)kPlayer.getHandicapType()).getDescription(), GC.getHandicapInfo((HandicapTypes)(kPlayer.getHandicapType() - 1)).getDescription());
									
									//Change the difficulty
									GC.getInitCore().setHandicap((PlayerTypes)iI, (HandicapTypes)(kPlayer.getHandicapType() - 1));
									
									//Clean the interface
									kPlayer.AI_makeAssignWorkDirty();
									for (pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
									{
										pLoopCity->setInfoDirty(true);
									}

									//Inform all human players of any other players difficulty changes
									for (iJ = 0; iJ < MAX_PLAYERS; iJ++)
									{
										if (GET_PLAYER((PlayerTypes)iJ).isAlive() && GET_PLAYER((PlayerTypes)iJ).isHuman() && GET_TEAM(GET_PLAYER((PlayerTypes)iJ).getTeam()).isHasMet(kPlayer.getTeam()))
										{
											//Difficulty decreased for us
											if (iJ == iI)
											{
												MEMORY_TRACK_EXEMPT();
												AddDLLMessage((PlayerTypes)iJ, true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_DIFFICULTY_DECREASED").GetCString(), "AS2D_FIRSTTOTECH", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"));
											}
											//Difficulty decreased for someone else
											else
											{
												MEMORY_TRACK_EXEMPT();
												AddDLLMessage((PlayerTypes)iJ, true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_DIFFICULTY_DECREASED_FOR_PLAYER", kPlayer.getCivilizationDescriptionKey()).GetCString(), "AS2D_FIRSTTOTECH", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT"));
											}
										}
									}
								}
							}
						}
					}
					
// 45° - START
// Difficulty increases if player score is between (average - 1/2 std dev) and (average + 1 std dev) and difficulty is below Noble			

					if (iCurrentScore > (iMeanScore - (stddev/2)) && (iCurrentScore < (iMeanScore + stddev)) && kPlayer.getHandicapType() < GC.getDefineINT("STANDARD_HANDICAP"))
					{
						logMsg("[Flexible Difficulty] Player: %S score is > 1/2 std dev below average but below average.", kPlayer.getName());
						//if (iCurrentRank < iMaxRank && iCurrentRank > iMinRank)
						{
							if (kPlayer.getHandicapType() >= 0)
							{
								if (kPlayer.getHandicapType() >= iMinHandiCap)
								{
									logMsg("[Flexible Difficulty] Player: %S difficulty is increasing from %S to %S", kPlayer.getName(), GC.getHandicapInfo((HandicapTypes)kPlayer.getHandicapType()).getDescription(), GC.getHandicapInfo((HandicapTypes)(kPlayer.getHandicapType() + 1)).getDescription());

									//Change the difficulty
									GC.getInitCore().setHandicap((PlayerTypes)iI, (HandicapTypes)(kPlayer.getHandicapType() + 1));

									//Clean the interface
									kPlayer.AI_makeAssignWorkDirty();
									for (pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
									{
										pLoopCity->setInfoDirty(true);
									}

									//Inform all human players of any other players difficulty changes
									for (iJ = 0; iJ < MAX_PLAYERS; iJ++)
									{
										if (GET_PLAYER((PlayerTypes)iJ).isAlive() && GET_PLAYER((PlayerTypes)iJ).isHuman() && GET_TEAM(GET_PLAYER((PlayerTypes)iJ).getTeam()).isHasMet(kPlayer.getTeam()))
										{
											//Difficulty increased for us
											if (iJ == iI)
											{
												MEMORY_TRACK_EXEMPT();
												AddDLLMessage((PlayerTypes)iJ, true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_DIFFICULTY_INCREASED").GetCString(), "AS2D_FEAT_ACCOMPLISHED", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT"));
											}
											//Difficulty increased for someone else
											else
											{
												MEMORY_TRACK_EXEMPT();
												AddDLLMessage((PlayerTypes)iJ, true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_DIFFICULTY_INCREASED_FOR_PLAYER", kPlayer.getCivilizationDescriptionKey()).GetCString(), "AS2D_FEAT_ACCOMPLISHED", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WARNING_TEXT"));
											}
										}
									}
								}
							}
						}
					}
// 45° - END						
				}
			}
		}
	}

	averageHandicaps();
}

void CvGame::averageHandicaps()
{
	int iI;
	int iAverageHandicap = 0;
	int iHumanCount = 0;
	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive() && GET_PLAYER((PlayerTypes)iI).isHuman())
		{
			iAverageHandicap += GET_PLAYER((PlayerTypes)iI).getHandicapType();
			iHumanCount++;
		}
	}
	if (iHumanCount > 0)
	{
		iAverageHandicap /= iHumanCount;
		iAverageHandicap = range(iAverageHandicap, 0, GC.getNumHandicapInfos() - 1);
		setHandicapType((HandicapTypes)iAverageHandicap);
	}
}

int CvGame::getMercyRuleCounter()
{
	return m_iMercyRuleCounter;
}

void CvGame::changeMercyRuleCounter(int iChange)
{
	m_iMercyRuleCounter += iChange;
}

void CvGame::setMercyRuleCounter(int iNewVal)
{
	m_iMercyRuleCounter = iNewVal;
}

void CvGame::clearLandmarks(bool bClear)
{
	for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);	
		if (bClear)
		{
			if (pLoopPlot->getLandmarkType() != NO_LANDMARK)
			{
				pLoopPlot->setLandmarkType(NO_LANDMARK);
				pLoopPlot->setLandmarkMessage("");
				pLoopPlot->setLandmarkName("");
				pLoopPlot->removeSignForAllPlayers();
			}
		}
		else
		{
			if (!CvWString(pLoopPlot->getLandmarkName()).empty() && pLoopPlot->getLandmarkType() == NO_LANDMARK)
			{
				pLoopPlot->removeSignForAllPlayers();
			}
		}
			
	}
}


void CvGame::pruneLandmarks()
{
	CvPlot* pLoopPlot;
	int iWorldSize;
	iWorldSize = GC.getMapINLINE().getWorldSize();
	
	for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		if (pLoopPlot->getLandmarkType() != NO_LANDMARK)
		{
			removeAdjacentLandmarks(pLoopPlot, pLoopPlot, std::max(1, iWorldSize / 2));
		}
	}
}
			
void CvGame::removeAdjacentLandmarks(CvPlot* pCenterPlot, CvPlot* pExceptionPlot, int iRadius)
{
	if (pCenterPlot == NULL || iRadius <= 0)
	{
		return;
	}
	CvPlot* pAdjacentPlot;
	for (int iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(pCenterPlot->getX_INLINE(), pCenterPlot->getY_INLINE(), ((DirectionTypes)iI));
		if (pAdjacentPlot != NULL && (pExceptionPlot == NULL || pExceptionPlot != pAdjacentPlot))
		{
			if (pAdjacentPlot->getLandmarkType() != NO_LANDMARK)
			{
				pAdjacentPlot->setLandmarkType(NO_LANDMARK);
				pAdjacentPlot->setLandmarkMessage("");
				pAdjacentPlot->setLandmarkName("");
			}
			removeAdjacentLandmarks(pAdjacentPlot, pExceptionPlot, iRadius - 1);
		}
	}
}

void CvGame::findMountainRanges()
{
	CvPlot* pLoopPlot;
	int iPeakCount;
	const int iMinimumRangeSize = GC.getDefineINT("MINIMUM_MOUNTAIN_RANGE_SIZE") + GC.getMapINLINE().getWorldSize();

	for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		iPeakCount = 0;
		if (!pLoopPlot->isCountedPlot())
		{
			//Mountain Ranges
			if (pLoopPlot->isPeak())
			{
				iPeakCount += countPeaks(pLoopPlot);
				pLoopPlot->setCountedPlot(true);
			}
			if (iPeakCount > iMinimumRangeSize)
			{
				pLoopPlot->setLandmarkType(LANDMARK_MOUNTAIN_RANGE);
			}
		}
	}
	//Clear Plot Cache when done
	for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		pLoopPlot->setCountedPlot(false);
	}
}

void CvGame::findLonePeaks()
{
	CvPlot* pLoopPlot;
	CvPlot* pAdjacentPlot;
	int iI;
	//int iPeakCount = 0;
	bool bFoundPeak;
	for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		if (pLoopPlot->isPeak())
		{
			bFoundPeak = false;
			int iHillCount = 0;
			for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
			{
				pAdjacentPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), ((DirectionTypes)iI));
				if (pAdjacentPlot != NULL)
				{//Check to see if it is alone
					if (pAdjacentPlot->isPeak())
					{
						bFoundPeak = true;
						break;
					}
					if (pAdjacentPlot->isHills())
						iHillCount++;
					//if a peak is surrounded by hills, don't count it as alone
					if (iHillCount > 2)
					{
						bFoundPeak = true;
						break;
					}
				}
			}//Lone Peak Found
			if (!bFoundPeak)
			{
				pLoopPlot->setLandmarkType(LANDMARK_PEAK);
			}
		}
	}
}

void CvGame::findBays()
{
// O O O
// X O X
// Y X X
//(X and Y are land, O, water. We are looking for Y, which should only be adjacent to 1 water tile... the bay!)
	CvPlot* pLoopPlot;
	CvPlot* pAdjacentPlot;
	int iI;
	for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		if (!pLoopPlot->isCountedPlot())
		{
			//Coast
			if (pLoopPlot->isCoastalLand())
			{
				//Find the Water
				int iCoastCount = 0;
				CvPlot* pCoast = NULL;
				for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
				{
					pAdjacentPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), ((DirectionTypes)iI));
					if (pAdjacentPlot != NULL)
					{
						if (pAdjacentPlot->isWater() && !pAdjacentPlot->isLake())
						{
							iCoastCount++;
							pCoast = pAdjacentPlot;
						}
					}
				}
				//Bay!
				if (iCoastCount == 1 && !pCoast->isCountedPlot())
				{	
					markBayPlots(pCoast);
					pCoast->setCountedPlot(true);
					pCoast->setLandmarkType(LANDMARK_BAY);
				}
			}
		}
	}
	//Clear Plot Cache when done
	for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		pLoopPlot->setCountedPlot(false);
	}
}

void CvGame::findForests()
{
	CvPlot* pLoopPlot;
	int iForestCount;
	const int iMinimumForestSize = GC.getDefineINT("MINIMUM_FOREST_SIZE") + GC.getMapINLINE().getWorldSize() * 2;

	for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		iForestCount = 0;
		if (!pLoopPlot->isCountedPlot())
		{
			//Forest
			if (pLoopPlot->getFeatureType() == (FeatureTypes)GC.getInfoTypeForString("FEATURE_FOREST"))
			{
				iForestCount++;
				pLoopPlot->setCountedPlot(true);
				iForestCount += countForest(pLoopPlot, iForestCount);
				if (iForestCount > iMinimumForestSize)
				{
					pLoopPlot->setLandmarkType(LANDMARK_FOREST);
				}
			}
		}
	}
	for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		pLoopPlot->setCountedPlot(false);
	}
}
void CvGame::findJungles()
{
	CvPlot* pLoopPlot;
	int iJungleCount;
	const int iMinimumJungleSize = GC.getDefineINT("MINIMUM_JUNGLE_SIZE") + GC.getMapINLINE().getWorldSize() * 2;

	for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		iJungleCount = 0;
		if (!pLoopPlot->isCountedPlot())
		{
			//Forest
			if (pLoopPlot->getFeatureType() == (FeatureTypes)GC.getInfoTypeForString("FEATURE_JUNGLE"))
			{
				iJungleCount++;
				pLoopPlot->setCountedPlot(true);
				iJungleCount += countJungle(pLoopPlot, iJungleCount);
				if (iJungleCount > iMinimumJungleSize)
				{
					pLoopPlot->setCountedPlot(true);
					pLoopPlot->setLandmarkType(LANDMARK_JUNGLE);
				}
			}
		}
	}
	for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		pLoopPlot->setCountedPlot(false);
	}
}

void CvGame::findDeserts()
{
	CvPlot* pLoopPlot;
	int iDesertCount;
	for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		iDesertCount = 0;
		if (pLoopPlot->getTerrainType() == (TerrainTypes)GC.getInfoTypeForString("TERRAIN_DESERT") && !pLoopPlot->isHills() && !pLoopPlot->isPeak())
		{
			if (!pLoopPlot->isCountedPlot())
			{
				iDesertCount++;
				pLoopPlot->setCountedPlot(true);
				iDesertCount += countDesert(pLoopPlot);
				if (iDesertCount > 5)
				{
					pLoopPlot->setLandmarkType(LANDMARK_DESERT);
				}			
			}
		}
	}
	for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		pLoopPlot->setCountedPlot(false);
	}
}

void CvGame::findLakes()
{
	CvPlot* pLoopPlot;
	for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		if (pLoopPlot->isLake())
		{
			if (!pLoopPlot->isCountedPlot())
			{
				pLoopPlot->setCountedPlot(true);
				markLakePlots(pLoopPlot);
				pLoopPlot->setLandmarkType(LANDMARK_LAKE);
			}
		}
	}
	for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		pLoopPlot->setCountedPlot(false);
	}
}

//Finds all adjacent lake plots to the given plot and marks them as counted.
//Used to ensure no more than 1 sign gets placed on each lake.
void CvGame::markLakePlots(CvPlot* pPlot)
{
	int iI;
	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		CvPlot* pAdjacentPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));
		if (pAdjacentPlot != NULL)
		{
			if (!pAdjacentPlot->isCountedPlot())
			{
				if (pAdjacentPlot->isLake())
				{
					pAdjacentPlot->setCountedPlot(true);
					markLakePlots(pAdjacentPlot);
				}
			}
		}
	}
}

//Returns vowels based on the most common letters and the frequency that they occur
char CvGame::getRandomVowel()
{
	int iRand = getSorenRandNum(38100, "Random Vowel");

	char ch;
	if (iRand < 8167)
		ch = 'a';
	else if (iRand < 20869)
		ch = 'e';
	else if (iRand < 27835)
		ch = 'i';
	else if (iRand < 35342)
		ch = 'o';
	else
		ch = 'u';
	return ch;
}

char CvGame::getRandomConsonant()
{
	int iRand = 0;
	iRand += getSorenRandNum(34550, "Random Consonant");
	iRand += getSorenRandNum(34550, "Random Consonant");
	char ch;
	if (iRand < 1492)
		ch = 'b';
	else if (iRand < 4274)
		ch = 'c';
	else if (iRand < 8527)
		ch = 'd';
	else if (iRand < 10755)
		ch = 'f';
	else if (iRand < 12770)
		ch = 'g';
	else if (iRand < 18864)
		ch = 'h';
	else if (iRand < 19017)
		ch = 'j';
	else if (iRand < 19789)
		ch = 'k';
	else if (iRand < 23814)
		ch = 'l';
	else if (iRand < 26220)
		ch = 'm';
	else if (iRand < 32969)
		ch = 'n';
	else if (iRand < 34898)
		ch = 'p';
	else if (iRand < 34993)
		ch = 'q';
	else if (iRand < 40980)
		ch = 'r';
	else if (iRand < 47307)
		ch = 's';
	else if (iRand < 56363)
		ch = 't';
	else if (iRand < 57341)
		ch = 'v';
	else if (iRand < 59701)
		ch = 'w';
	else if (iRand < 59851)
		ch = 'x';
	else if (iRand < 61825)
		ch = 'y';
	else
		ch = 'z';

   return ch;
}
CvWString CvGame::generateRandomWord(int iMaxLength)
{
	int iLength;
	CvWString szName;
	char chNextLetter;
	
	bool bBreak = false;
	int iRand;
	//4 letter word minimum
	iLength = std::min(iMaxLength, std::max(4, getSorenRandNum(iMaxLength, "Random Word Length")));
	for (int iI = 0; iI < iLength; iI++)
	{
		iRand = getSorenRandNum(1000, "RandomLetter");
		if (iRand <= 381 || bBreak)
		{
			chNextLetter = getRandomVowel();
			bBreak = false;
		}
		else
		{
			chNextLetter = getRandomConsonant();
			bBreak = getSorenRandNum(3, "Break Consonants") == 0;
		}
		//Civ's RNG sucks
		getSorenRandNum(100, "Throw Away Number");
		//Capitalize First Letter
		if (iI == 0)
			chNextLetter = (char)toupper(chNextLetter);
		szName += chNextLetter;
	}
	return szName;
}

bool CvGame::isValidName(CvWString szName)
{
	int iConsonantCount = 0;
	int iVowelCount = 0;
	int iVowelStreak = 0;
	int iConsonantStreak = 0;
	for (int iI = 0; iI < (int)szName.length(); iI++)
	{
		char ch = tolower(szName[iI]);
		if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u')
		{
			iVowelCount++;
			iVowelStreak++;
			iConsonantStreak = 0;
		}
		else
		{
			iConsonantCount++;
			iConsonantStreak++;
			iVowelStreak = 0;
		}
		if (iConsonantStreak > 3)
		{
			return false;
		}
		if (iVowelStreak > 4)
		{
			return false;
		}
	}
	//More than 75% of the word is vowels
	if ((iVowelCount * 100 / std::max(1, iVowelCount + iConsonantCount)) >= 75)
	{
		return false;
	}
	//More than 70% of the word is consonants
	if ((iConsonantCount * 100 / std::max(1, iVowelCount + iConsonantCount)) >= 70)
	{
		return false;
	}
	
	return true;
}

CvWString CvGame::getRandomName(int iMaxLength)
{
	CvWString szName;
	CvWString szRandomName;
	int iLength = 0;
	int iMaxTries = 3;
	iMaxLength = std::max(4, iMaxLength);
	while (true)
	{
		szName = generateRandomWord(iMaxLength);
		if (isValidName(szName))
		{
			//First word is always valid
			if (iLength == 0)
			{
				szRandomName += szName;
				iLength += szName.length();
			}
			else
			{
				int iCurLength = iMaxLength - iLength - szName.length();
				if (iCurLength > 0)
				{
					szRandomName += ' ';
					szRandomName += szName; 
					iLength += szName.length();
				}
				else
				{
					iMaxTries--;
				}
				iCurLength = iMaxLength - iLength;
				if (iMaxTries <= 0 || iCurLength < 4)
				{
					break;
				}
			}
		}
	}
	return szRandomName;
}

int CvGame::countDesert(CvPlot* pPlot)
{
	int iI;
	int iDesert = 0;
	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		CvPlot* pAdjacentPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));
		if (pAdjacentPlot != NULL)
		{
			if (!pAdjacentPlot->isCountedPlot())
			{
				if (pAdjacentPlot->getTerrainType() == (TerrainTypes)GC.getInfoTypeForString("TERRAIN_DESERT") && !pAdjacentPlot->isHills() && !pAdjacentPlot->isPeak())
				{
					pAdjacentPlot->setCountedPlot(true);
					iDesert++;
					iDesert += countDesert(pAdjacentPlot);
				}
			}
		}
	}
	return iDesert;
}

int CvGame::countJungle(CvPlot* pPlot, int iJungle)
{
	int iI;
	//This is a big jungle
	if (iJungle > GC.getDefineINT("MINIMUM_JUNGLE_SIZE") * 2)
		return iJungle;
	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		CvPlot* pAdjacentPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));
		if (pAdjacentPlot != NULL)
		{
			if (!pAdjacentPlot->isCountedPlot())
			{
				if (pAdjacentPlot->getFeatureType() == (FeatureTypes)GC.getInfoTypeForString("FEATURE_JUNGLE"))
				{
					iJungle++;
					pAdjacentPlot->setCountedPlot(true);
					iJungle += countJungle(pAdjacentPlot, iJungle);
				}
			}
		}
	}
	return iJungle;
}

int CvGame::countForest(CvPlot* pPlot, int iForest)
{
	int iI;
	//This is a big forest
	if (iForest > GC.getDefineINT("MINIMUM_FOREST_SIZE") * 2)
		return iForest;
	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		CvPlot* pAdjacentPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));
		if (pAdjacentPlot != NULL)
		{
			if (!pAdjacentPlot->isCountedPlot())
			{
				if (pAdjacentPlot->getFeatureType() == (FeatureTypes)GC.getInfoTypeForString("FEATURE_FOREST"))
				{
					iForest++;
					pAdjacentPlot->setCountedPlot(true);
					iForest += countForest(pAdjacentPlot, iForest);
				}
			}
		}
	}
	return iForest;
}

//Maximum length of message is 21 chars
void CvGame::addLandmarkSigns()
{
	CvWString szSign;
	CvWString szName;
	CvWString szTextKey;
	for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		if (pLoopPlot->getLandmarkType() != NO_LANDMARK)
		{
			szTextKey = "TXT_KEY_LANDMARK";
			szSign.clear();
			szName.clear();
			switch (pLoopPlot->getLandmarkType())
			{
				case LANDMARK_BAY:
					szTextKey.append((CvWString)"_BAY");
					szTextKey.append(CvWString::format(L"_%d", (getSorenRandNum(GC.getDefineINT("NUM_BAY_NAMES"), "Landmark Naming"))));
					break;
				case LANDMARK_ISLAND:
					break;
				case LANDMARK_FOREST:
					szTextKey.append((CvWString)"_FOREST");
					szTextKey.append(CvWString::format(L"_%d", (getSorenRandNum(GC.getDefineINT("NUM_FOREST_NAMES"), "Landmark Naming"))));
					break;
				case LANDMARK_JUNGLE:
					szTextKey.append((CvWString)"_JUNGLE");
					szTextKey.append(CvWString::format(L"_%d", (getSorenRandNum(GC.getDefineINT("NUM_JUNGLE_NAMES"), "Landmark Naming"))));
					break;
				case LANDMARK_PEAK:
					szTextKey.append((CvWString)"_PEAK");
					szTextKey.append(CvWString::format(L"_%d", (getSorenRandNum(GC.getDefineINT("NUM_PEAK_NAMES"), "Landmark Naming"))));
					break;
				case LANDMARK_MOUNTAIN_RANGE:
					szTextKey.append((CvWString)"_MOUNTAINS");
					szTextKey.append(CvWString::format(L"_%d", (getSorenRandNum(GC.getDefineINT("NUM_MOUNTAINS_NAMES"), "Landmark Naming"))));
					break;
				case LANDMARK_PLAINS:
					break;
				case LANDMARK_DESERT:
					szTextKey.append((CvWString)"_DESERT");
					szTextKey.append(CvWString::format(L"_%d", (getSorenRandNum(GC.getDefineINT("NUM_DESERT_NAMES"), "Landmark Naming"))));
					break;
				case LANDMARK_OCEAN:
				case LANDMARK_LAKE:
					szTextKey.append((CvWString)"_LAKE");
					szTextKey.append(CvWString::format(L"_%d", (getSorenRandNum(GC.getDefineINT("NUM_LAKE_NAMES"), "Landmark Naming"))));
					break;
				default:
					FAssertMsg(false, "Unknown Landmark Type");
			}
			szSign = szTextKey;
			CvWString szDummy = " ";
			szTextKey = gDLL->getText(szTextKey, szDummy.GetCString());
			szName = getRandomName(21- szTextKey.length());
			szSign = gDLL->getText(szSign, szName.GetCString());
			
			pLoopPlot->setLandmarkMessage(szSign);
			pLoopPlot->setLandmarkName(szName);
		}
	}
}

//Marks nearby tiles as counted so that they will not recieve multiple signs
void CvGame::markBayPlots(CvPlot* pPlot)
{
	int iI, iJ;
	CvPlot* pAdjacentPlot;
	//Marks all plots around the given plot, and all plots around the adjacent plots as counted
	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));
		if (pAdjacentPlot != NULL)
		{
			if (!pAdjacentPlot->isCountedPlot())
			{
				if (pAdjacentPlot->isWater())
				{
					pAdjacentPlot->setCountedPlot(true);
					for (iJ = 0; iJ < NUM_DIRECTION_TYPES; ++iJ)
					{
						CvPlot* pAdjacentPlot2 = plotDirection(pAdjacentPlot->getX_INLINE(), pAdjacentPlot->getY_INLINE(), ((DirectionTypes)iJ));
						if (pAdjacentPlot2 != NULL)
						{
							if (!pAdjacentPlot2->isCountedPlot())
							{
								if (pAdjacentPlot2->isWater())
								{
									pAdjacentPlot2->setCountedPlot(true);
								}
							}
						}
					}
				}
			}
		}
	}
}

int CvGame::countPeaks(CvPlot* pPlot, bool bCountHill)
{
	int iI;
	int iPeak = 0;
	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		CvPlot* pAdjacentPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));
		if (pAdjacentPlot != NULL)
		{
			if (!pAdjacentPlot->isCountedPlot())
			{
				if (pAdjacentPlot->isPeak())
				{
					iPeak++;
					pAdjacentPlot->setCountedPlot(true);
					iPeak += countPeaks(pAdjacentPlot, true);
				}
				//Use hills to find sparser mountain ranges
				else if (pAdjacentPlot->isHills() && bCountHill)
				{
					pAdjacentPlot->setCountedPlot(true);
					iPeak += countPeaks(pAdjacentPlot, false);
				}
			}
		}
	}
	return iPeak;
}

void CvGame::updateInitialSigns()
{
	PROFILE_FUNC();

	for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		if (pLoopPlot->getLandmarkType() != NO_LANDMARK)
		{
			for (int iJ = 0; iJ < MAX_PLAYERS; ++iJ)
			{
				if (GET_PLAYER((PlayerTypes)iJ).isAlive() && !GET_PLAYER((PlayerTypes)iJ).isBarbarian())
				{
					if (pLoopPlot->isRevealed(GET_PLAYER((PlayerTypes)iJ).getTeam(), false))
					{
						pLoopPlot->addSign((PlayerTypes)iJ, pLoopPlot->getLandmarkMessage());
					}
				}
			}
		}
	}
}
bool CvGame::isPreviousRequest(PlayerTypes ePlayer) const																	
{
	FAssertMsg(ePlayer >= 0, "ePlayer is expected to be non-negative (invalid Index)");
	FAssertMsg(ePlayer < MAX_PLAYERS, "ePlayer is expected to be within maximum bounds (invalid Index)");
	return m_abPreviousRequest[ePlayer];
}


void CvGame::setPreviousRequest(PlayerTypes ePlayer, bool bNewValue)		
{
	FAssertMsg(ePlayer >= 0, "ePlayer is expected to be non-negative (invalid Index)");
	FAssertMsg(ePlayer < MAX_PLAYERS, "ePlayer is expected to be within maximum bounds (invalid Index)");
	m_abPreviousRequest[ePlayer] = bNewValue;
}

void CvGame::setModderGameOption(ModderGameOptionTypes eIndex, int iNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_MODDERGAMEOPTION_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiModderGameOption[eIndex] = iNewValue;
}

void CvGame::setModderGameOption(ModderGameOptionTypes eIndex, bool bNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_MODDERGAMEOPTION_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiModderGameOption[eIndex] = bNewValue;
}

void CvGame::doFoundCorporations()
{
	MEMORY_TRACE_FUNCTION();

	int iI;
	for (iI = 0; iI < GC.getNumCorporationInfos(); iI++)
	{
		doFoundCorporation((CorporationTypes)iI, false);
	}
}
void CvGame::doFoundCorporation(CorporationTypes eCorporation, bool bForce)
{
	if (!GC.getGameINLINE().isOption(GAMEOPTION_REALISTIC_CORPORATIONS))
	{
		return;
	}
	CvCity* pLoopCity;
	CvCity* pBestCity;
	int iSpread;
	int iBestSpread;
	int iLoop = 0;
	int iI;
	pBestCity = NULL;
	
	
	int iCheckTurns = GC.getDefineINT("CORPORATION_FOUND_CHECK_TURNS");
	iCheckTurns *= GC.getGameSpeedInfo(getGameSpeedType()).getAnarchyPercent();
	iCheckTurns /= 100;
	
	if (getElapsedGameTurns() % std::max(1, iCheckTurns) != 0 && !bForce)
	{
		return;
	}
	
	if (!canEverSpread(eCorporation))
	{
		return;
	}
	
	if (GC.getGame().isModderGameOption(MODDERGAMEOPTION_NO_AUTO_CORPORATION_FOUNDING))
	{
		bool bValid = false;
		for (int iI = 0; iI < GC.getNumUnitInfos(); iI++)
		{
			if (canEverTrain((UnitTypes)iI))
			{
				if (GC.getUnitInfo((UnitTypes)iI).getCorporationSpreads(eCorporation) > 0)
				{
					bValid = true;
					break;
				}
			}
		}
		if (bValid)
		{
			return;
		}
	}
	
	iBestSpread = getSorenRandNum(GC.getDefineINT("RAND_CORPORATION_BEST_SPREAD"), "Corporation Founding Best Spread");
	
	if (getHeadquarters(eCorporation) == NULL)
	{
		TechTypes ePrereqTech = (TechTypes)GC.getCorporationInfo(eCorporation).getTechPrereq();
		//Find the prereq tech for corporate HQ
		if (ePrereqTech == NO_TECH)
		{
			for (int i = 0; i < GC.getNumBuildingInfos(); i++)
			{
				if ((CorporationTypes)GC.getBuildingInfo((BuildingTypes)i).getGlobalCorporationCommerce() == eCorporation)
				{
					ePrereqTech = (TechTypes)GC.getBuildingInfo((BuildingTypes)i).getPrereqAndTech();
					if (ePrereqTech != NO_TECH)
					{
						//Found a tech, exit the loop, else keep looking
						break;
					}
				}
			}
		}
		
		if (ePrereqTech != NO_TECH)
		{
			if (countKnownTechNumTeams(ePrereqTech) > GC.getDefineINT("MINIMUM_PLAYERS_WITH_TECH_FOR_AUTO_CORPORATION_FOUNDING"))
			{
				TechTypes eObsoleteTech = (TechTypes)GC.getCorporationInfo(eCorporation).getObsoleteTech();
				for (iI = 0; iI < MAX_PLAYERS; iI++)
				{
					CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iI);
					if (kPlayer.isAlive())
					{
						if (eObsoleteTech == NO_TECH || !GET_TEAM(kPlayer.getTeam()).isHasTech(eObsoleteTech))
						{
							for (pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
							{
								iSpread = pLoopCity->getCorporationInfluence(eCorporation);
											
								iSpread *= GC.getCorporationInfo(eCorporation).getSpread();
								
								iSpread /= 100;
								
								if (iSpread > 0)
								{
									//Random Luck
									iSpread += getSorenRandNum(100, "Corporation Founding Rand Luck");
									if (iSpread > iBestSpread)
									{
										pBestCity = pLoopCity;
										iBestSpread = iSpread;
									}
								}
							}
							//Discourage players with more than 2 corporations from getting more
							if (pBestCity != NULL)
							{
								if (pBestCity->getOwnerINLINE() == iI)
								{
									int iCorporationCount = 0;
									for (int iJ = 0; iJ < GC.getNumCorporationInfos(); iJ++)
									{
										if (getHeadquarters((CorporationTypes)iJ) != NULL)
										{
											if (getHeadquarters((CorporationTypes)iJ)->getOwnerINLINE() == iI)
											{
												iCorporationCount++;
											}
										}
									}
									iBestSpread /= std::max(1, iCorporationCount);
								}
							}
						}
						
					}
				}
				if (pBestCity != NULL)
				{
					if ((getSorenRandNum(GC.getDefineINT("CORPORATION_SPREAD_RAND"), "Corporation Founding Rand")  < iBestSpread) || bForce)
					{
						setHeadquarters(eCorporation, pBestCity, true);
					}
				}
			}
		}
	}
}

int CvGame::getAverageCorporationInfluence(CvCity* pCity, CorporationTypes eCorporation) const
{
	CvCity* pLoopCity;
	int iSpread = GC.getCorporationInfo(eCorporation).getSpread();
	int iTotalSpread = 0;
	int iLoop = 0;
	int iI;
	int iPlayerCount = 0;
	int iRandThreshold = 0;
	
	if (GC.getGameINLINE().getHeadquarters(eCorporation) == NULL)
	{
		return 100;
	}
	
	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iI);
		if (kPlayer.isAlive())
		{
			if (kPlayer.isNoCorporations())
			{
				continue;
			}
			if (kPlayer.isNoForeignCorporations() && GC.getGameINLINE().getHeadquarters(eCorporation)->getOwner() != pCity->getOwner())
			{
				continue;
			}
			iPlayerCount++;
			for (pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
			{
				if (pLoopCity->isConnectedTo(pCity))
				{
					iSpread = pLoopCity->getCorporationInfluence(eCorporation);
					
					iSpread *= GC.getCorporationInfo(eCorporation).getSpread();
					
					iSpread /= 100;
					
					if (iSpread > 0)
					{
						iSpread /= std::max(1, (((GC.getDefineINT("CORPORATION_SPREAD_DISTANCE_DIVISOR") * plotDistance(pCity->getX_INLINE(), pCity->getY_INLINE(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE())) / GC.getMapINLINE().maxPlotDistance()) - 5));
						
						iRandThreshold = std::max(iRandThreshold, iSpread);
					}
				}
			}
			iRandThreshold *= GET_PLAYER(pCity->getOwner()).getCorporationSpreadModifier() + 100;
			iRandThreshold /= 100;
			iRandThreshold *= GET_PLAYER(pCity->getOwner()).getCorporationInfluence(eCorporation);
			iRandThreshold /= 100;
			
			iTotalSpread += iRandThreshold;
		}
	}
	
	if (iPlayerCount > 0)
	{
		iTotalSpread /= iPlayerCount;
	}
	else
	{
		iTotalSpread = 0;
	}
	
	return iTotalSpread;
}

bool CvGame::canEverResearch(TechTypes eTech) const
{
	if (GC.getTechInfo(eTech).isCanPassPeaks()
		// advc.opt: Switched; game option checks aren't super fast.
		&& !isOption(GAMEOPTION_MOUNTAINS))
	{
		return false;
	}
	/*if (isOption(GAMEOPTION_NO_FUTURE))
	{
		if (GC.getTechInfo(eTech).getEra() > GC.getInfoTypeForString("ERA_MODERN"))
		{
			if (!GC.getTechInfo(eTech).isRepeat())
			{
				return false;
			}
		}
	}*/
	// <f1rpo> (era options)
	if (GC.getTechInfo(eTech).getEra() > getPenultimateEra()
		&& !GC.getTechInfo(eTech).isRepeat())
	{
		return false;
	} // </f1rpo>
	if (GC.getTechInfo(eTech).getPrereqGameOption() != NO_GAMEOPTION)
	{
		if (!isOption((GameOptionTypes)GC.getTechInfo(eTech).getPrereqGameOption()))
		{
			return false;
		}
	}
	return true;
}


void CvGame::setFutureEras()
{
	/*TechTypes eFutureTech = NO_TECH;
	for (int iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		if (GC.getTechInfo((TechTypes)iI).isRepeat())
		{
			eFutureTech = (TechTypes)iI;
			break;
		}
	}*/
	// <f1rpo> (era options - not needed, but ...)
	std::vector<TechTypes> aFutureTech;
	FOR_EACH_ENUM(Tech)
	{
		if (GC.getTechInfo(eLoopTech).isRepeat())
			aFutureTech.push_back(eLoopTech);
	}
	//if (eFutureTech != NO_TECH)
	// <f1rpo>
	for (size_t i = 0; i < aFutureTech.size(); i++)
	{
		TechTypes eFutureTech = aFutureTech[i]; // </f1rpo>
		bool bSet = false;
		for (int iI = 0; iI < GC.getNUM_OR_TECH_PREREQS(); iI++)
		{
			if (GC.getTechInfo(eFutureTech).getOriginalPrereqOrTechs(iI) != NO_TECH)
			{
				bSet = true;
				break;
			}
		
		}
		if (!bSet)
		{
			for (int iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
			{
				for (int iI = 0; iI < GC.getNUM_OR_TECH_PREREQS(); iI++)
				{
					GC.getTechInfo((TechTypes)iJ).setOriginalPrereqOrTech(iI, GC.getTechInfo((TechTypes)iJ).getPrereqOrTechs(iI));
					GC.getTechInfo((TechTypes)iJ).setOriginalPrereqAndTech(iI, GC.getTechInfo((TechTypes)iJ).getPrereqAndTechs(iI));
				}
			}
		}
		else
		{
			for (int iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
			{
				for (int iI = 0; iI < GC.getNUM_OR_TECH_PREREQS(); iI++)
				{
					GC.getTechInfo((TechTypes)iJ).setPrereqOrTech(iI, GC.getTechInfo((TechTypes)iJ).getOriginalPrereqOrTechs(iI));
					GC.getTechInfo((TechTypes)iJ).setPrereqAndTech(iI, GC.getTechInfo((TechTypes)iJ).getOriginalPrereqAndTechs(iI));
				}
			}
			int iBestX = 0;
			for (int iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
			{
				if (GC.getTechInfo((TechTypes)iJ).getGridX() > iBestX)
				{
					iBestX = GC.getTechInfo((TechTypes)iJ).getGridX();
				}
			}
			GC.getTechInfo(eFutureTech).setGridX(iBestX + 1);
		}
	}
	/*if (isOption(GAMEOPTION_NO_FUTURE)) {
		if (eFutureTech != NO_TECH)*/
	// <f1rpo>
	for (size_t i = 0; i < aFutureTech.size(); i++)
	{
		TechTypes eFutureTech = aFutureTech[i];
		if (getPenultimateEra() < GC.getTechInfo(eFutureTech).getEra() - 1) // </f1rpo>
		{
			//Remove existing techs prereqs
			for (int iI = 0; iI < GC.getNUM_OR_TECH_PREREQS(); iI++)
			{
				GC.getTechInfo(eFutureTech).setPrereqOrTech(iI, NO_TECH);
				GC.getTechInfo(eFutureTech).setPrereqAndTech(iI, NO_TECH);
			}
			//Find the next best tech to require the future tech
			int iBestX = 0;
			for (int iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
			{
				if (canEverResearch((TechTypes)iJ) && !GC.getTechInfo((TechTypes)iJ).isRepeat())
				{
					if (GC.getTechInfo((TechTypes)iJ).getGridX() > iBestX)
					{
						iBestX = GC.getTechInfo((TechTypes)iJ).getGridX();
					}
				}
			}
			int iCount = 0;
			for (int iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
			{
				if (GC.getTechInfo((TechTypes)iJ).getGridX() == iBestX)
				{
					if (canEverResearch((TechTypes)iJ))
					{
						if (iCount > GC.getNUM_OR_TECH_PREREQS())
						{
							break;
						}
						GC.getTechInfo(eFutureTech).setPrereqOrTech(iCount, iJ);
						iCount++;
					}
				}
			}
			GC.getTechInfo(eFutureTech).setGridX(iBestX + 1);
		}
	}
	
	//Remove Prerequisite Techs for which can never be researched
	for (int iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		if (!canEverResearch((TechTypes)iI))
		{
			for (int iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
			{
				for (int iK = 0; iK < GC.getNUM_OR_TECH_PREREQS(); iK++)
				{
					if (GC.getTechInfo((TechTypes)iJ).getPrereqOrTechs(iK) == iI)
					{
						GC.getTechInfo((TechTypes)iJ).setPrereqOrTech(iK, NO_TECH);
					}
					if (GC.getTechInfo((TechTypes)iJ).getPrereqAndTechs(iK) == iI)
					{
						GC.getTechInfo((TechTypes)iJ).setPrereqAndTech(iK, NO_TECH);
					}
				}
			}
		}
	}
}

int CvGame::getFlexibleDifficultyTimer(PlayerTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiFlexibleDifficultyTimer[eIndex];
}

void CvGame::setFlexibleDifficultyTimer(PlayerTypes eIndex, int iNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiFlexibleDifficultyTimer[eIndex] = iNewValue;
}

void CvGame::calculateRiverBuildings()
{
	m_iRiverBuildings = 0;
	for (int iJ = 0; iJ < GC.getNumBuildingInfos(); iJ++)
	{
		if (GC.getBuildingInfo((BuildingTypes)iJ).isRiver())
		{
			m_iRiverBuildings++;
		}
	}
}

int CvGame::getRiverBuildings() const
{
	return m_iRiverBuildings;
}

void CvGame::calculateCoastalBuildings()
{
	m_iCoastalBuildings = 0;
	for (int iJ = 0; iJ < GC.getNumBuildingInfos(); iJ++)
	{
		if (GC.getBuildingInfo((BuildingTypes)iJ).isWater())
		{
			m_iCoastalBuildings++;
		}
	}
}

int CvGame::getCoastalBuildings() const
{
	return m_iCoastalBuildings;
}

bool CvGame::canExitToMainMenu() const
{
	if (gDLL->getInterfaceIFace()->isDebugMenuCreated())
	{
		return false;
	}
	// if (GC.getGameINLINE().isOption(GAMEOPTION_NO_FUTURE))
	// {
		// return false;
	// }
	//if (!GC.getGameINLINE().isOption(GAMEOPTION_ADVANCED_ECONOMY))
	//{
	//	return false;
	//}
	return true;
}

void CvGame::calculateNumWonders()
{
	m_iNumWonders = 0;
	for (int iJ = 0; iJ < GC.getNumBuildingClassInfos(); iJ++)
	{
		if (isLimitedWonderClass((BuildingClassTypes)iJ))
		{
			m_iNumWonders++;
		}
	}
}

int CvGame::getNumWonders() const
{
	return m_iNumWonders;
}

bool CvGame::canEverConstruct(BuildingTypes eBuilding) const
{
	if (eBuilding == NO_BUILDING)
	{
		return false;
	}
	CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);
	
	if (kBuilding.getPrereqGameOption() != NO_GAMEOPTION)
	{
		if (!(GC.getGameINLINE().isOption((GameOptionTypes)kBuilding.getPrereqGameOption())))
		{
			return false;
		}
	}

	if (kBuilding.getNotGameOption() != NO_GAMEOPTION)
	{
		if (GC.getGameINLINE().isOption((GameOptionTypes)kBuilding.getNotGameOption()))
		{
			return false;
		}
	}
	
	if (kBuilding.getPrereqCorporation() != NO_CORPORATION)
	{
		if (!canEverSpread((CorporationTypes)kBuilding.getPrereqCorporation()))
		{
			return false;
		}
	}
	
	if (kBuilding.getFoundsCorporation() != NO_CORPORATION)
	{
		if (!canEverSpread((CorporationTypes)kBuilding.getFoundsCorporation()))
		{
			return false;
		}

		if (GC.getGameINLINE().isOption(GAMEOPTION_REALISTIC_CORPORATIONS))
		{
			return false;
		}
	}
	
	bool bFound = false;
	bool bRequires = false;
	for (int iI = 0; iI < GC.getNumGameSpeedInfos(); iI++)
	{
		if (kBuilding.isPrereqOrGameSpeed(iI))
		{
			bRequires = true;
			if (getGameSpeedType() == iI)
			{
				bFound = true;
				break;
			}
		}
	}
	if (!bFound && bRequires)
	{
		return false;
	}
	
	return true;
}

bool CvGame::canEverTrain(UnitTypes eUnit) const
{
	if (eUnit == NO_UNIT)
	{
		return false;
	}
	CvUnitInfo& kUnit = GC.getUnitInfo(eUnit);
	
	if (kUnit.getPrereqGameOption() != NO_GAMEOPTION)
	{
		if (!(GC.getGameINLINE().isOption((GameOptionTypes)kUnit.getPrereqGameOption())))
		{
			return false;
		}
	}
	
	if (kUnit.getNotGameOption() != NO_GAMEOPTION)
	{
		if (GC.getGameINLINE().isOption((GameOptionTypes)kUnit.getNotGameOption()))
		{
			return false;
		}
	}
	
	if (kUnit.getPrereqCorporation() != NO_CORPORATION)
	{
		if (!canEverSpread((CorporationTypes)kUnit.getPrereqCorporation()))
		{
			return false;
		}
	}
	
	if (kUnit.getPrereqBuilding() != NO_BUILDING)
	{
		if (!canEverConstruct((BuildingTypes)kUnit.getPrereqBuilding()))
		{
			return false;
		}
	}
	
	
	return true;
}

bool CvGame::canEverSpread(CorporationTypes eCorporation) const
{
	if (eCorporation == NO_CORPORATION)
	{
		return false;
	}
	CvCorporationInfo& kCorporation = GC.getCorporationInfo(eCorporation);
	
	if (kCorporation.getPrereqGameOption() != NO_GAMEOPTION)
	{
		if (!(GC.getGameINLINE().isOption((GameOptionTypes)kCorporation.getPrereqGameOption())))
		{
			return false;
		}
	}
	
	return true;
}

void CvGame::loadPirateShip(CvUnit* pUnit)
{
	if (pUnit->getDomainType() != DOMAIN_SEA)
	{
		return;
	}

	for (int iI = 0; iI < pUnit->cargoSpace(); iI++)
	{
		UnitTypes eBestUnit = NO_UNIT;
		int iBestValue = 0;

		for (int iJ = 0; iJ < GC.getNumUnitClassInfos(); iJ++)
		{
			bool bValid = false;
			UnitTypes eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(GET_PLAYER(BARBARIAN_PLAYER).getCivilizationType()).getCivilizationUnits(iJ)));

			if (eLoopUnit != NO_UNIT)
			{
				CvUnitInfo& kUnit = GC.getUnitInfo(eLoopUnit);

				bValid = (kUnit.getCombat() > 0 && !kUnit.isOnlyDefensive());

				if (bValid)
				{
					if (kUnit.getDomainType() != DOMAIN_LAND)
					{
						bValid = false;
					}
				}

				if (bValid)
				{
					if (!GET_PLAYER(BARBARIAN_PLAYER).canTrain(eLoopUnit))
					{
						bValid = false;
					}
				}

				if (bValid)
				{
					if (NO_BONUS != kUnit.getPrereqAndBonus())
					{
						if (!GET_TEAM(BARBARIAN_TEAM).isHasTech((TechTypes)GC.getBonusInfo((BonusTypes)kUnit.getPrereqAndBonus()).getTechCityTrade()))
						{
							bValid = false;
						}
					}
				}

				if (bValid)
				{
					bool bFound = false;
					bool bRequires = false;
					for (int i = 0; i < GC.getNUM_UNIT_PREREQ_OR_BONUSES(); ++i)
					{
						if (NO_BONUS != kUnit.getPrereqOrBonuses(i))
						{
							TechTypes eTech = (TechTypes)GC.getBonusInfo((BonusTypes)kUnit.getPrereqOrBonuses(i)).getTechCityTrade();
							if (NO_TECH != eTech)
							{
								bRequires = true;
								
								if (GET_TEAM(BARBARIAN_TEAM).isHasTech(eTech))
								{
									bFound = true;
									break;
								}
							}
						}
					}

					if (bRequires && !bFound)
					{
						bValid = false;
					}
				}

				if (bValid)
				{
					int iValue = (1 + getSorenRandNum(1000, "Barb Unit Selection"));

					if (kUnit.getUnitAIType(UNITAI_ATTACK))
					{
						iValue += 250;
					}

					if (iValue > iBestValue)
					{
						eBestUnit = eLoopUnit;
						iBestValue = iValue;
					}
				}
			}
		}

		if (eBestUnit != NO_UNIT)
		{
			CvUnit* pPirate = GET_PLAYER(BARBARIAN_PLAYER).initUnit(eBestUnit, pUnit->plot()->getX_INLINE(), pUnit->plot()->getY_INLINE(), UNITAI_ATTACK, NO_DIRECTION, GC.getGameINLINE().getSorenRandNum(10000, "AI Unit Birthmark 11"));
			pPirate->setTransportUnit(pUnit);
			pUnit->AI_setUnitAIType(UNITAI_ASSAULT_SEA);
		}
	}
}
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/

void CvGame::recalculateModifiers()
{
	OutputDebugString(CvString::format("Start profiling(false) for modifier recalc\n").c_str());
	startProfilingDLL(false);
	PROFILE_FUNC();

	//	Clear area modifiers
	int iLoop;
	int	iI;
	int iPlayer;
	CvArea* pLoopArea;

	m_bRecalculatingModifiers = true;

	//	Inhibit plot group manipulation until we rebuild at the end of everything else
	for(iI = 0; iI < MAX_PLAYERS; iI++ )
	{
		if ( GET_PLAYER((PlayerTypes)iI).isAlive() )
		{
			GET_PLAYER((PlayerTypes)iI).inhibitPlotGroupCalcsUntilFullRebuild();
		}
	}

	// AIAndy: Recalculate which info class replacements are currently active
	GC.updateReplacements();

	for (iI = 0; iI < GC.getNumVoteSourceInfos(); iI++)
	{
		m_aiDiploVote[iI] = 0;
	}

	for (iI = 0; iI < GC.getNumSpecialUnitInfos(); iI++)
	{
		m_pabSpecialUnitValid[iI] = false;
		if (GC.getSpecialUnitInfo((SpecialUnitTypes)iI).isValid())
		{
			makeSpecialUnitValid((SpecialUnitTypes)iI);
		}
	}

	for (iI = 0; iI < GC.getNumSpecialBuildingInfos(); iI++)
	{
		m_pabSpecialBuildingValid[iI] = false;
		if (GC.getSpecialBuildingInfo((SpecialBuildingTypes)iI).isValid())
		{
			makeSpecialBuildingValid((SpecialBuildingTypes)iI);
		}
	}

	//TB Nukefix
	//m_bNukesValid = false;

	// AIAndy: Clear properties for recalculate, should be before any recalculation of any team because change propagation can cause cross team propagation (depending on usage)
	// units are not supposed to have that kind of property (needs extra code if that changes)
	getProperties()->clearForRecalculate();

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
		pLoopPlot->getProperties()->clearForRecalculate();

		//	We will recalculate visibility from first principles
		pLoopPlot->clearVisibilityCounts();

		//	Fix any spurious routes that are on water tiles
		RouteTypes eRoute = pLoopPlot->getRouteType();
		if ( eRoute != NO_ROUTE && pLoopPlot->isWater() )
		{
			if ( !GC.getRouteInfo(eRoute).isSeaTunnel() )
			{
				pLoopPlot->setRouteType(NO_ROUTE, false);
			}
		}

		//	Recalc blockades from scratch
		pLoopPlot->resetBlockadedCounts();
	}

	for(iI = 0; iI < MAX_TEAMS; iI++)
	{
		if ( GET_TEAM((TeamTypes)iI).isAlive() )
		{
			GET_TEAM((TeamTypes)iI).getProperties()->clearForRecalculate();
		}
	}

	for (iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
		if (kLoopPlayer.isAlive())
		{
			kLoopPlayer.getProperties()->clearForRecalculate();
			int iLoop = 0;
			for (CvCity* pCity = kLoopPlayer.firstCity(&iLoop); pCity != NULL; pCity = kLoopPlayer.nextCity(&iLoop))
			{
				pCity->getProperties()->clearForRecalculate();
			}
		}
	}


	for(pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
	{
		pLoopArea->clearModifierTotals();
	}

	for(iI = 0; iI < MAX_TEAMS; iI++)
	{
		if ( GET_TEAM((TeamTypes)iI).isAlive() )
		{
			GET_TEAM((TeamTypes)iI).recalculateModifiers();
		}
	}

	for(iI = 0; iI < GC.getNumVoteSourceInfos(); iI++ )
	{
		for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
		{
			GET_PLAYER((PlayerTypes)iPlayer).processVoteSourceBonus((VoteSourceTypes)iI, true);
		}
	}

	updatePlotGroups(true);
	updateTradeRoutes();

	//	Recheck for disabled buildings everywhere (this has to be done after plot group establishment
	//	or else resource dependencies will mean it gets the wrong answer, which will in turn force
	//	(automatic) recalculation the following turn (which is very inefficient)
	for (iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
		if (kLoopPlayer.isAlive())
		{
			int iLoop = 0;
			for (CvCity* pCity = kLoopPlayer.firstCity(&iLoop); pCity != NULL; pCity = kLoopPlayer.nextCity(&iLoop))
			{
				pCity->doVicinityBonus();
				pCity->checkBuildings(true, true, true, true, true, false);
			}
		}
	}

	GC.getMapINLINE().updateSight(true, false);

	gDLL->getEngineIFace()->RebuildAllPlots();

	GC.getMapINLINE().setupGraphical();
	GC.getMapINLINE().updateVisibility();
	GC.getMapINLINE().updateFog();

	m_bRecalculatingModifiers = false;

	//	Force a one-time reclaculation of all city commerce after coming out
	//	of the in-recalc section (which inhibits this doing any work)
	for (iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
		if (kLoopPlayer.isAlive())
		{
			int iLoop = 0;
			for (CvCity* pCity = kLoopPlayer.firstCity(&iLoop); pCity != NULL; pCity = kLoopPlayer.nextCity(&iLoop))
			{
				pCity->updateBuildingCommerce();
				pCity->updateCommerce(NO_COMMERCE, true);
			}
		}
	}

	stopProfilingDLL(false);
	OutputDebugString(CvString::format("Stop profiling(false) after modifier recalc\n").c_str());
}

CvProperties* CvGame::getProperties()
{
	return &m_Properties;
}

const CvProperties* CvGame::getPropertiesConst() const
{
	return &m_Properties;
}


/*********************************/
/***** Parallel Maps - Begin *****/
/*********************************/

MapTypes CvGame::getCurrentMap() const
{
	FAssert(m_eCurrentMap > NO_MAP);
	FAssert(m_eCurrentMap < GC.getNumMapInfos());
	return m_eCurrentMap;
}

void CvGame::setCurrentMap(MapTypes eNewMap)
{
	FAssert(eNewMap > NO_MAP);
	FAssert(eNewMap < GC.getNumMapInfos());
	m_eCurrentMap = eNewMap;
}

void CvGame::processGreatWall(bool bIn, bool bForce, bool bSeeded) const
{
	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iI);
		if (kLoopPlayer.isAlive())
		{
			int iIter = 0;
			for (CvCity* pCity = kLoopPlayer.firstCity(&iIter); NULL != pCity; pCity = kLoopPlayer.nextCity(&iIter))
			{
				if ( pCity->processGreatWall(bIn, bForce, bSeeded) )
				{
					//	There can be only onme!
					return;
				}
			}
		}
	}
}
/*******************************/
/***** Parallel Maps - End *****/
/*******************************/
void CvGame::setOptionEdits()
{
	int iI;

	//Traits
	for (iI = 0; iI < GC.getNumTraitInfos(); iI++)
	{
		GC.getTraitInfo((TraitTypes)iI).setOptionEditsIndex();
	}
}

void CvGame::resetOptionEdits()
{
	int iI;

	//Traits
	for (iI = 0; iI < GC.getNumTraitInfos(); iI++)
	{
		GC.getTraitInfo((TraitTypes)iI).resetOptionEdits();
	}
}

void CvGame::reportEvent(PythonEventTypes type, PlayerTypes ePlayer, int iData1, int iData2)
{
	PythonEventData	data;

	data.type = type;
	data.ePlayer = ePlayer;
	data.iData1 = iData1;
	data.iData2 = iData2;

	EnterCriticalSection(&m_reportableEventsSection);
	m_reportableEvents.push_back(data);
	LeaveCriticalSection(&m_reportableEventsSection);
}

void CvGame::reportQueuedEvents()
{
	size_t	i = 0;
	bool	bHasMoreData;

	//	Python processing of events can result in more events recursively, so cannot use 
	//	a standard iterator here as the list may be getting appended to during processing
	do
	{
		PythonEventData eventData;
		bHasMoreData = false;

		EnterCriticalSection(&m_reportableEventsSection);
		if ( i < m_reportableEvents.size() )
		{
			eventData = m_reportableEvents[i++];
			bHasMoreData = true;
		}
		LeaveCriticalSection(&m_reportableEventsSection);

		if ( bHasMoreData )
		{
			switch(eventData.type)
			{
			case CITY_EVENT_HURRY:
				{
					CvCity*	pCity = GET_PLAYER(eventData.ePlayer).getCity(eventData.iData2);

					if ( pCity != NULL )
					{
						CvEventReporter::getInstance().cityHurry(pCity, (HurryTypes)eventData.iData1);
					}
				}
				break;
			case CITY_EVENT_CORP_SPREAD:
				{
					CvCity*	pCity = GET_PLAYER(eventData.ePlayer).getCity(eventData.iData2);

					if ( pCity != NULL )
					{
						CvEventReporter::getInstance().corporationSpread((CorporationTypes)eventData.iData1, eventData.ePlayer, pCity);
					}
				}
				break;
			case CITY_EVENT_CORP_REMOVE:
				{
					CvCity*	pCity = GET_PLAYER(eventData.ePlayer).getCity(eventData.iData2);

					if ( pCity != NULL )
					{
						CvEventReporter::getInstance().corporationRemove((CorporationTypes)eventData.iData1, eventData.ePlayer, pCity);
					}
				}
				break;
			case PLAYER_EVENT_CORP_FOUNDED:
				{
					CvEventReporter::getInstance().corporationFounded((CorporationTypes)eventData.iData1, eventData.ePlayer);
				}
				break;
			case CITY_EVENT_RELIGION_SPREAD:
				{
					CvCity*	pCity = GET_PLAYER(eventData.ePlayer).getCity(eventData.iData2);

					if ( pCity != NULL )
					{
						CvEventReporter::getInstance().religionSpread((ReligionTypes)eventData.iData1, eventData.ePlayer, pCity);
					}
				}
				break;
			case CITY_EVENT_RELIGION_REMOVE:
				{
					CvCity*	pCity = GET_PLAYER(eventData.ePlayer).getCity(eventData.iData2);

					if ( pCity != NULL )
					{
						CvEventReporter::getInstance().religionRemove((ReligionTypes)eventData.iData1, eventData.ePlayer, pCity);
					}
				}
				break;
			case PLAYER_EVENT_RELIGION_FOUNDED:
				{
					CvEventReporter::getInstance().religionFounded((ReligionTypes)eventData.iData1, eventData.ePlayer);
				}
				break;
			case PLOT_EVENT_REVEALED:
				{
					CvPlot*	pPlot = GC.getMapINLINE().plotByIndexINLINE(eventData.iData2);

					if ( pPlot != NULL )
					{
						CvEventReporter::getInstance().plotRevealed(pPlot, (TeamTypes)eventData.iData1);
					}
				}
				break;
			case TEAM_EVENT_TECHACQUIRED:
				{
					TechTypes eTech = (TechTypes)eventData.iData2;
					bool	  bAnnounce = ((eventData.iData1 & 0x80000000) != 0);
					TeamTypes eTeam = (TeamTypes)(eventData.iData1 & 0x7FFFFFFF);

					CvEventReporter::getInstance().techAcquired(eTech, eTeam, eventData.ePlayer, bAnnounce);
				}
			default:
				break;
			}
		}
	} while(bHasMoreData);

	m_reportableEvents.clear();
}

void CvGame::queueUIActivity(UIActivityInfo& info)
{
	m_queuedUIActivity.push_back(info);
}

void CvGame::doQueuedUIActivity(void)
{
	PROFILE_FUNC();

	for(std::vector<UIActivityInfo>::const_iterator itr = m_queuedUIActivity.begin(); itr != m_queuedUIActivity.end(); ++itr)
	{
		switch(itr->eType)
		{
		case UIACTIVITY_PLACE_GREAT_WALL:
			{
				CvCity*	pCity = GET_PLAYER(itr->ePlayer).getCity(itr->iData2);

				pCity->processGreatWall((itr->iData1 != 0), true);
			}
			break;
		case UIACTIVITY_CHANGE_CIVIC_POPUP:
			{
				CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CHANGECIVIC);
				if (NULL != pInfo)
				{
					pInfo->setData1(itr->iData1);
					pInfo->setData2(itr->iData2);
					gDLL->getInterfaceIFace()->addPopup(pInfo, itr->ePlayer);
				}
			}
			break;
		case UIACTIVITY_CHOOSE_TECH_POPUP:
			{
				GET_PLAYER(itr->ePlayer).chooseTech(itr->iData1, itr->szwStrData);
			}
			break;
		default:
			break;
		}
	}

	m_queuedUIActivity.clear();
}

bool CvGame::isEarlyGame() const
{
	int iEarlyTurnThreshold = GC.getDefineINT("EARLY_GAME_TURN_THRESHOLD", 100);
	iEarlyTurnThreshold *= GC.getGameSpeedInfo(getGameSpeedType()).getTrainPercent();
	iEarlyTurnThreshold /= 100;
	return getElapsedGameTurns() < iEarlyTurnThreshold;
}

void CvGame::doCultureDecay()
{
	int iCultureHalvingTurns = GC.getDefineINT("CULTURE_DECAY_TURNS", 25); // note: GoldenAgePercent is 200% in normal speed, so this is effectively 2x
	if (iCultureHalvingTurns > 0)
	{
		iCultureHalvingTurns *= GC.getGameSpeedInfo(getGameSpeedType()).getGoldenAgePercent();
		iCultureHalvingTurns /= 100;
		iCultureHalvingTurns = std::max(10, iCultureHalvingTurns);
		if ((getElapsedGameTurns() + 1) % iCultureHalvingTurns == 0)
		{
			logMsg("Culture Decay Occuring, Elapsed Turns: %d, iCultureHalvingTurns: %d", getElapsedGameTurns(), iCultureHalvingTurns);
			for (int iPlot = 0; iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
			{
				CvPlot* pPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
				pPlot->doCulturalDecay();
			}
		}
	}
}

bool CvGame::isInWeakestPlayerThreshold(PlayerTypes ePlayer) const
{
	//Bottom percentage of players by score that are considered "weakest"
	int iWeakestThreshold = GC.getDefineINT("WEAKEST_PLAYER_THRESHOLD", 33);
	int iKnownCount = 0;
	int iKnownRank = 0;
	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive() && GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isHasMet(GET_PLAYER((PlayerTypes)iI).getTeam()))
		{
			iKnownCount++;
			if (getPlayerScore((PlayerTypes)iI) > getPlayerScore(ePlayer))
			{
				iKnownRank++;
			}
		}
	}
	int iBottomPlayers = std::max(1, iKnownCount * iWeakestThreshold / 100);
	return (iKnownCount - iKnownRank < iBottomPlayers);
}

bool CvGame::isInStrongestPlayerThreshold(PlayerTypes ePlayer) const
{
	//Top percentage of players by score that are considered "strongest"
	int iWeakestThreshold = GC.getDefineINT("STRONGEST_PLAYER_THRESHOLD", 33);
	int iKnownCount = 0;
	int iKnownRank = 0;
	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive() && GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isHasMet(GET_PLAYER((PlayerTypes)iI).getTeam()))
		{
			iKnownCount++;
			if (getPlayerScore((PlayerTypes)iI) > getPlayerScore(ePlayer))
			{
				iKnownRank++;
			}
		}
	}
	int iTopPlayers = std::max(1, iKnownCount * iWeakestThreshold / 100);
	return (iKnownRank < iTopPlayers);
}

int CvGame::getNumWeakestPlayers(PlayerTypes ePlayer) const
{
	int iCount = 0;
	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive() && isInWeakestPlayerThreshold((PlayerTypes)iI))
		{
			if (ePlayer == iI || !GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isAtWar(GET_PLAYER((PlayerTypes)iI).getTeam()))
			{
				iCount++;
			}
		}
	}
	return iCount;
}

int CvGame::getNumStrongestPlayers(PlayerTypes ePlayer) const
{
	int iCount = 0;
	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive() && isInStrongestPlayerThreshold((PlayerTypes)iI))
		{
			if (ePlayer == iI || !GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isAtWar(GET_PLAYER((PlayerTypes)iI).getTeam()))
			{
				iCount++;
			}
		}
	}
	return iCount;
}

int CvGame::getCeaseRelationsTurnAmt() const
{
	int iTurns = GC.getDefineINT("CEASE_CONTACT_TRADE_TURNS", 20);
	iTurns *= GC.getGameSpeedInfo(getGameSpeedType()).getGoldenAgePercent();
	iTurns /= 100;
	//Round to nearest multiple of 5
	iTurns = ((iTurns + 5) / 5) * 5;
	//Keep between 10 and 100
	return std::max(10, std::min(100, iTurns));
}

bool CvGame::isAnyoneHasUnitZoneOfControl() const
{
	return m_bAnyoneHasUnitZoneOfControl;
}

void CvGame::toggleAnyoneHasUnitZoneOfControl()
{
	m_bAnyoneHasUnitZoneOfControl = true;
}

void CvGame::resync(bool bWrite, ByteBuffer *pBuffer)
{
	RESYNC_INT(bWrite, pBuffer, m_iElapsedGameTurns);
	RESYNC_INT(bWrite, pBuffer, m_iStartTurn);
	RESYNC_INT(bWrite, pBuffer, m_iStartYear);
	RESYNC_INT(bWrite, pBuffer, m_iEstimateEndTurn);
	RESYNC_INT(bWrite, pBuffer, m_iTurnSlice);
	RESYNC_INT(bWrite, pBuffer, m_iCutoffSlice);
	RESYNC_INT(bWrite, pBuffer, m_iNumGameTurnActive);
	RESYNC_INT(bWrite, pBuffer, m_iNumCities);
	RESYNC_INT(bWrite, pBuffer, m_iTotalPopulation);
	RESYNC_INT(bWrite, pBuffer, m_iTradeRoutes);
	RESYNC_INT(bWrite, pBuffer, m_iFreeTradeCount);
	RESYNC_INT(bWrite, pBuffer, m_iNoNukesCount);
	RESYNC_INT(bWrite, pBuffer, m_iNukesExploded);
	RESYNC_INT(bWrite, pBuffer, m_iMaxPopulation);
	RESYNC_INT(bWrite, pBuffer, m_iMaxLand);
	RESYNC_INT(bWrite, pBuffer, m_iMaxLand);
	RESYNC_INT(bWrite, pBuffer, m_iMaxTech);
	RESYNC_INT(bWrite, pBuffer, m_iMaxWonders);
	RESYNC_INT(bWrite, pBuffer, m_iInitPopulation);
	RESYNC_INT(bWrite, pBuffer, m_iInitLand);
	RESYNC_INT(bWrite, pBuffer, m_iInitTech);
	RESYNC_INT(bWrite, pBuffer, m_iInitWonders);
	RESYNC_INT(bWrite, pBuffer, m_iCurrentVoteID);
	RESYNC_INT(bWrite, pBuffer, m_iWaterAnimalSpawnChance);
	RESYNC_INT(bWrite, pBuffer, m_iCutLosersCounter);
	RESYNC_INT(bWrite, pBuffer, m_iHighToLowCounter);
	RESYNC_INT(bWrite, pBuffer, m_iIncreasingDifficultyCounter);
	RESYNC_INT(bWrite, pBuffer, m_iMercyRuleCounter);
	RESYNC_BOOL(bWrite, pBuffer, m_bDiploVictoryEnabled);
	RESYNC_INT(bWrite, pBuffer, m_iMercyRuleCounter);
	RESYNC_INT_ARRAY(bWrite, pBuffer, MAX_PLAYERS, m_aiFlexibleDifficultyTimer);
	RESYNC_BOOL(bWrite, pBuffer, m_bScoreDirty);
	RESYNC_INT_WITH_CAST(bWrite, pBuffer, m_circumnavigatingTeam, TeamTypes);
	RESYNC_BOOL(bWrite, pBuffer, m_bFinalInitialized);
	RESYNC_BOOL(bWrite, pBuffer, m_bHotPbemBetweenTurns);
	RESYNC_INT_WITH_CAST(bWrite, pBuffer, m_eHandicap, HandicapTypes);
	RESYNC_INT_WITH_CAST(bWrite, pBuffer, m_eAIHandicap, HandicapTypes); // f1rpo.autoplay
	RESYNC_INT_WITH_CAST(bWrite, pBuffer, m_ePausePlayer, PlayerTypes);
	RESYNC_INT_WITH_CAST(bWrite, pBuffer, m_eBestLandUnit, UnitTypes);
	RESYNC_INT_WITH_CAST(bWrite, pBuffer, m_eWinner, TeamTypes);
	RESYNC_INT_WITH_CAST(bWrite, pBuffer, m_eVictory, VictoryTypes);
	RESYNC_INT_WITH_CAST(bWrite, pBuffer, m_eGameState, GameStateTypes);

	if (bWrite) pBuffer->putString(m_szScriptData); else pBuffer->getString(m_szScriptData);

	RESYNC_INT_ARRAY(bWrite, pBuffer, MAX_PLAYERS, m_aiRankPlayer);
	RESYNC_INT_ARRAY(bWrite, pBuffer, MAX_PLAYERS, m_aiPlayerRank);
	RESYNC_INT_ARRAY(bWrite, pBuffer, MAX_PLAYERS, m_aiPlayerScore);
	RESYNC_INT_ARRAY(bWrite, pBuffer, MAX_TEAMS, starshipLaunched);
	RESYNC_INT_ARRAY(bWrite, pBuffer, MAX_TEAMS, diplomaticVictoryAchieved);
	RESYNC_INT_ARRAY(bWrite, pBuffer, MAX_TEAMS, AscensionGateBuilt);
	RESYNC_INT_ARRAY(bWrite, pBuffer, MAX_TEAMS, m_aiRankTeam);
	RESYNC_INT_ARRAY(bWrite, pBuffer, MAX_TEAMS, m_aiTeamRank);
	RESYNC_INT_ARRAY(bWrite, pBuffer, MAX_TEAMS, m_aiTeamScore);
	RESYNC_INT_ARRAY(bWrite, pBuffer, MAX_PLAYERS, m_iAIAutoPlay);
	RESYNC_INT_ARRAY(bWrite, pBuffer, MAX_PLAYERS, m_iForcedAIAutoPlay);

	RESYNC_INT_ARRAY(bWrite, pBuffer, GC.getNumUnitInfos(), m_paiUnitCreatedCount);
	RESYNC_INT_ARRAY(bWrite, pBuffer, GC.getNumUnitClassInfos(), m_paiUnitClassCreatedCount);
	RESYNC_INT_ARRAY(bWrite, pBuffer, GC.getNumBuildingClassInfos(), m_paiBuildingClassCreatedCount);
	RESYNC_INT_ARRAY(bWrite, pBuffer, GC.getNumProjectInfos(), m_paiProjectCreatedCount);
	RESYNC_INT_ARRAY(bWrite, pBuffer, GC.getNumCivicInfos(), m_paiForceCivicCount);
	RESYNC_INT_ARRAY_WITH_CAST(bWrite, pBuffer, GC.getNumVoteInfos(), m_paiVoteOutcome, PlayerVoteTypes);
	RESYNC_INT_ARRAY(bWrite, pBuffer, GC.getNumReligionInfos(), m_paiReligionGameTurnFounded);
	RESYNC_INT_ARRAY(bWrite, pBuffer, GC.getNumCorporationInfos(), m_paiCorporationGameTurnFounded);
	RESYNC_INT_ARRAY(bWrite, pBuffer, GC.getNumVoteSourceInfos(), m_aiSecretaryGeneralTimer);
	RESYNC_INT_ARRAY(bWrite, pBuffer, GC.getNumVoteSourceInfos(), m_aiVoteTimer);
	RESYNC_INT_ARRAY(bWrite, pBuffer, GC.getNumVoteSourceInfos(), m_aiDiploVote);

	RESYNC_BOOL_ARRAY(bWrite, pBuffer, GC.getNumSpecialUnitInfos(), m_pabSpecialUnitValid);
	RESYNC_BOOL_ARRAY(bWrite, pBuffer, GC.getNumSpecialBuildingInfos(), m_pabSpecialBuildingValid);
	RESYNC_BOOL_ARRAY(bWrite, pBuffer, GC.getNumReligionInfos(), m_abReligionSlotTaken);

	RESYNC_BOOL_ARRAY(bWrite, pBuffer, GC.getNumTechInfos(), m_abTechCanFoundReligion);

	for (int iI = 0; iI<GC.getNumReligionInfos(); iI++)
	{
		RESYNC_INT_WITH_CAST(bWrite, pBuffer, m_paHolyCity[iI].eOwner, PlayerTypes);
		RESYNC_INT(bWrite, pBuffer, m_paHolyCity[iI].iID);
	}

	for (int iI = 0; iI<GC.getNumCorporationInfos(); iI++)
	{
		RESYNC_INT_WITH_CAST(bWrite, pBuffer, m_paHeadquarters[iI].eOwner, PlayerTypes);
		RESYNC_INT(bWrite, pBuffer, m_paHeadquarters[iI].iID);
	}

	{
		std::vector<CvWString>::iterator ws_it;

		if (bWrite)
		{
			pBuffer->putInt(m_aszDestroyedCities.size());
			for (ws_it = m_aszDestroyedCities.begin(); ws_it != m_aszDestroyedCities.end(); ++ws_it)
			{
				pBuffer->putString(*ws_it);
			}
		}
		else
		{
			m_aszDestroyedCities.clear();
			int iSize = pBuffer->getInt();
			for (int i = 0; i < iSize; i++)
			{
				CvWString szCityName;
				pBuffer->getString(szCityName);
				m_aszDestroyedCities.push_back(szCityName);
			}
		}

		if (bWrite)
		{
			pBuffer->putInt(m_aszGreatPeopleBorn.size());
			for (ws_it = m_aszGreatPeopleBorn.begin(); ws_it != m_aszGreatPeopleBorn.end(); ++ws_it)
			{
				pBuffer->putString(*ws_it);
			}
		}
		else
		{
			m_aszGreatPeopleBorn.clear();
			int iSize = pBuffer->getInt();
			for (int i = 0; i < iSize; i++)
			{
				CvWString szGreatPerson;
				pBuffer->getString(szGreatPerson);
				m_aszGreatPeopleBorn.push_back(szGreatPerson);
			}
		}
	}

	ResyncStreamableFFreeListTrashArray(bWrite, pBuffer, m_deals);
	ResyncStreamableFFreeListTrashArray(bWrite, pBuffer, m_voteSelections);
	ResyncStreamableFFreeListTrashArray(bWrite, pBuffer, m_votesTriggered);

	if (bWrite)
	{
		pBuffer->putLong(m_mapRand.getSeed());
		pBuffer->putLong(m_sorenRand.getSeed());
	}
	else
	{
		m_mapRand.reseed((unsigned long)pBuffer->getLong());
		m_sorenRand.reseed((unsigned long)pBuffer->getLong());
	}

	{
		ReplayMessageList::const_iterator it;
		if (bWrite)
		{
			pBuffer->putInt(m_listReplayMessages.size());
			for (it = m_listReplayMessages.begin(); it != m_listReplayMessages.end(); ++it)
			{
				const CvReplayMessage* pMessage = *it;
				if (NULL != pMessage)
				{
					pBuffer->put((byte)1);
					pBuffer->putInt(pMessage->getTurn());
					pBuffer->putInt(pMessage->getType());
					pBuffer->putInt(pMessage->getPlotX());
					pBuffer->putInt(pMessage->getPlotY());
					pBuffer->putInt(pMessage->getPlayer());
					pBuffer->putString(pMessage->getText());
					pBuffer->putInt(pMessage->getColor());
				}
				else
				{
					pBuffer->put((byte)0);
				}
			}
		}
		else
		{
			m_listReplayMessages.clear();
			int iSize = pBuffer->getInt();
			for (int i = 0; i < iSize; i++)
			{
				bool bValid = pBuffer->get() != 0;
				if (bValid)
				{
					int iTurn = pBuffer->getInt();
					ReplayMessageTypes eType = (ReplayMessageTypes)pBuffer->getInt();
					int iPlotX = pBuffer->getInt();
					int iPlotY = pBuffer->getInt();
					PlayerTypes ePlayer = (PlayerTypes)pBuffer->getInt();
					CvWString szText;
					pBuffer->getString(szText);
					ColorTypes eColor = (ColorTypes)pBuffer->getInt();
					CvReplayMessage* pReplayMessage = new CvReplayMessage(iTurn, eType, ePlayer);
					pReplayMessage->setPlot(iPlotX, iPlotY);
					pReplayMessage->setText(szText);
					pReplayMessage->setColor(eColor);
					m_listReplayMessages.push_back(pReplayMessage);
				}
			}
		}
	}

	RESYNC_INT(bWrite, pBuffer, m_iNumSessions);

	if (bWrite)
	{
		pBuffer->putInt(m_aPlotExtraYields.size());
		for (std::vector<PlotExtraYield>::iterator it = m_aPlotExtraYields.begin(); it != m_aPlotExtraYields.end(); ++it)
		{
			pBuffer->putInt((*it).m_iX);
			pBuffer->putInt((*it).m_iY);
			for (int i = 0; i < NUM_YIELD_TYPES; ++i)
			{
				pBuffer->putInt((*it).m_aeExtraYield[i]);
			}
		}
	}
	else
	{
		m_aPlotExtraYields.clear();
		int iSize = pBuffer->getInt();
		for (int i = 0; i < iSize; i++)
		{
			PlotExtraYield pExtraYield;
			pExtraYield.m_iX = pBuffer->getInt();
			pExtraYield.m_iY = pBuffer->getInt();
			pExtraYield.m_aeExtraYield.clear();
			for (int i = 0; i < NUM_YIELD_TYPES; ++i)
			{
				pExtraYield.m_aeExtraYield.push_back(pBuffer->getInt());
			}
			m_aPlotExtraYields.push_back(pExtraYield);
		}
	}

	if (bWrite)
	{
		pBuffer->putInt(m_aPlotExtraCosts.size());
		for (std::vector<PlotExtraCost>::iterator it = m_aPlotExtraCosts.begin(); it != m_aPlotExtraCosts.end(); ++it)
		{
			pBuffer->putInt((*it).m_iX);
			pBuffer->putInt((*it).m_iY);
			pBuffer->putInt((*it).m_iCost);
		}
	}
	else
	{
		m_aPlotExtraCosts.clear();
		int iSize = pBuffer->getInt();
		for (int i = 0; i < iSize; i++)
		{
			PlotExtraCost pExtraCost;
			pExtraCost.m_iX = pBuffer->getInt();
			pExtraCost.m_iY = pBuffer->getInt();
			pExtraCost.m_iCost = pBuffer->getInt();
			m_aPlotExtraCosts.push_back(pExtraCost);
		}
	}

	if (bWrite)
	{
		pBuffer->putInt(m_mapVoteSourceReligions.size());
		for (stdext::hash_map<VoteSourceTypes, ReligionTypes>::iterator it = m_mapVoteSourceReligions.begin(); it != m_mapVoteSourceReligions.end(); ++it)
		{
			pBuffer->putInt(it->first);
			pBuffer->putInt(it->second);
		}
	}
	else
	{
		m_mapVoteSourceReligions.clear();
		int iSize = pBuffer->getInt();
		for (int i = 0; i < iSize; i++)
		{
			m_mapVoteSourceReligions[(VoteSourceTypes)pBuffer->getInt()] = (ReligionTypes)pBuffer->getInt();
		}
	}

	if (bWrite)
	{
		pBuffer->putInt(m_aeInactiveTriggers.size());
		for (std::vector<EventTriggerTypes>::iterator e_it = m_aeInactiveTriggers.begin(); e_it != m_aeInactiveTriggers.end(); ++e_it)
		{
			pBuffer->putInt(*e_it);
		}
	}
	else
	{
		m_aeInactiveTriggers.clear();
		int iSize = pBuffer->getInt();
		for (int i = 0; i < iSize; i++)
		{
			m_aeInactiveTriggers.push_back((EventTriggerTypes)pBuffer->getInt());
		}
	}

	RESYNC_INT(bWrite, pBuffer, m_iShrineBuildingCount);

	RESYNC_INT_ARRAY(bWrite, pBuffer, GC.getNumBuildingInfos(), m_aiShrineBuilding);
	RESYNC_INT_ARRAY(bWrite, pBuffer, GC.getNumBuildingInfos(), m_aiShrineReligion);

	RESYNC_INT(bWrite, pBuffer, m_iNumCultureVictoryCities);
	RESYNC_INT(bWrite, pBuffer, m_eCultureVictoryCultureLevel);

}

void CvGame::checkInitialResourceAllocations()
{
	int* paiBonusAmounts = new int[GC.getNumBonusInfos()];
	for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		paiBonusAmounts[iI] = 0;
	}

	for (int iK = 0; iK < GC.getMapINLINE().numPlotsINLINE(); iK++)
	{
		CvPlot* pPlot = GC.getMapINLINE().plotByIndexINLINE(iK);
		if (pPlot->getBonusType() != NO_BONUS)
		{
			paiBonusAmounts[pPlot->getBonusType()]++;
		}
	}

	TCHAR szOut[1024];
	sprintf(szOut, "ResourceAllocation.log");
	for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		CvBonusInfo& kBonus = GC.getBonusInfo((BonusTypes)iI);

		int iMinimum = kBonus.getMinimumPerWorld();
		if (iMinimum > 0)
		{
			iMinimum *= GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getResearchPercent();
			iMinimum /= 100;
		}

		int iMaximum = kBonus.getMaximumPerWorld();
		if (iMaximum > 0)
		{
			iMaximum *= GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getResearchPercent();
			iMaximum /= 100;
		}

		logMsgTo(szOut, "[Intial Resource Allocation] Resource: %S, Resource Amount: %d, Minimum: %d, Maximum: %d", kBonus.getDescription(), paiBonusAmounts[iI], iMinimum, iMaximum);
	}

	for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		CvBonusInfo& kBonus = GC.getBonusInfo((BonusTypes)iI);

		int iMaximum = kBonus.getMaximumPerWorld();
		if (iMaximum > 0)
		{
			iMaximum *= GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getResearchPercent();
			iMaximum /= 100;
		}

		int iRemoved = 0;

		if (iMaximum > 0 && paiBonusAmounts[iI] > iMaximum)
		{
			for (int iAmt = 0; iAmt < (iMaximum - paiBonusAmounts[iI]); iAmt++)
			{
				CvPlot* pBestPlot = NULL;
				unsigned long long iBestValue = 0;
				for (int iJ = 0; iJ < GC.getMapINLINE().numPlotsINLINE(); iJ++)
				{
					CvPlot* pPlot = GC.getMapINLINE().plotByIndexINLINE(iJ);
					if (pPlot->getBonusType() == iI)
					{
						unsigned long long iValue = 100000 + (((int)getMapRand().get(MAX_SHORT, "Maximum Resource Allocation")) * 3);
						if (paiBonusAmounts[iI] > 0)
						{
							int iClosestDistance = MAX_INT;
							for (int iK = 0; iK < GC.getMapINLINE().numPlotsINLINE(); iK++)
							{
								CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iK);
								if (pLoopPlot->getBonusType() == iI)
								{
									int iDistance = plotDistance(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), pPlot->getX_INLINE(), pPlot->getY_INLINE());
									if (iDistance < iClosestDistance)
									{
										iClosestDistance = iDistance;
									}
								}
							}
							iValue /= iClosestDistance;
						}
						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = pPlot;
						}
					}
				}
				if (pBestPlot != NULL)
				{
					logMsgTo(szOut, "Failed to remove extra %S, no suitable locations on the map found!", kBonus.getDescription());
					pBestPlot->setBonusType(NO_BONUS);
					iRemoved++;
				}
				else
				{
					break;
				}
			}
		}

		if (iRemoved > 0)
		{
			logMsgTo(szOut, "Removed %d allocations of resource: %S", iRemoved, kBonus.getDescription());
		}
	}

	for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		CvBonusInfo& kBonus = GC.getBonusInfo((BonusTypes)iI);

		int iMinimum = kBonus.getMinimumPerWorld();
		if (iMinimum > 0)
		{
			iMinimum *= GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getResearchPercent();
			iMinimum /= 100;
		}

		int iAdded = 0;

		if (iMinimum > 0 && paiBonusAmounts[iI] < iMinimum)
		{
			for (int iAmt = 0; iAmt < (iMinimum - paiBonusAmounts[iI]); iAmt++)
			{
				CvPlot* pBestPlot = NULL;
				unsigned long long iBestValue = 0;
				for (int iJ = 0; iJ < GC.getMapINLINE().numPlotsINLINE(); iJ++)
				{
					CvPlot* pPlot = GC.getMapINLINE().plotByIndexINLINE(iJ);
					if (pPlot->canHaveBonus((BonusTypes)iI))
					{
						unsigned long long iValue = 100000 + (((int)getMapRand().get(MAX_SHORT, "Minimum Resource Allocation")) * 3);
						if (paiBonusAmounts[iI] > 0)
						{
							int iClosestDistance = MAX_INT;
							int iFarthestDistance = 1;
							for (int iK = 0; iK < GC.getMapINLINE().numPlotsINLINE(); iK++)
							{
								CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iK);
								if (pLoopPlot->getBonusType() == iI)
								{
									int iDistance = plotDistance(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), pPlot->getX_INLINE(), pPlot->getY_INLINE());
									if (iDistance < iClosestDistance)
									{
										iClosestDistance = iDistance;
									}
									else if (iDistance > iFarthestDistance)
									{
										iFarthestDistance = iDistance;
									}
								}
							}
							iValue *= iClosestDistance;
							iValue /= iFarthestDistance;
						}
						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = pPlot;
						}
					}
				}
				if (pBestPlot != NULL)
				{
					pBestPlot->setBonusType((BonusTypes)iI);
					iAdded++;
				}
				else
				{
					logMsgTo(szOut, "Failed to place extra %S, no suitable locations on the map found!", kBonus.getDescription());
					break;
				}
			}
		}

		if (iAdded > 0)
		{
			logMsgTo(szOut, "Added %d allocations of resource: %S", iAdded, kBonus.getDescription());
		}
	}

	for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		paiBonusAmounts[iI] = 0;
	}

	for (int iK = 0; iK < GC.getMapINLINE().numPlotsINLINE(); iK++)
	{
		CvPlot* pPlot = GC.getMapINLINE().plotByIndexINLINE(iK);
		if (pPlot->getBonusType() != NO_BONUS)
		{
			paiBonusAmounts[pPlot->getBonusType()]++;
		}
	}

	for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		CvBonusInfo& kBonus = GC.getBonusInfo((BonusTypes)iI);

		int iMinimum = kBonus.getMinimumPerWorld();
		if (iMinimum > 0)
		{
			iMinimum *= GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getResearchPercent();
			iMinimum /= 100;
		}

		int iMaximum = kBonus.getMaximumPerWorld();
		if (iMaximum > 0)
		{
			iMaximum *= GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getResearchPercent();
			iMaximum /= 100;
		}

		logMsgTo(szOut, "[Final Resource Allocation] Resource: %S, Resource Amount: %d, Minimum: %d, Maximum: %d", kBonus.getDescription(), paiBonusAmounts[iI], iMinimum, iMaximum);
	}

	SAFE_DELETE_ARRAY(paiBonusAmounts);
}

bool CvGame::isDarkAge() const
{
	if (m_pDarkAgeState != NULL)
	{
		if (m_pDarkAgeState->iDelayTurns == 0)
		{
			return true;
		}
	}
	return false;
}

bool CvGame::isDarkAgePending() const
{
	if (m_pDarkAgeState != NULL)
	{
		if (m_pDarkAgeState->iDelayTurns > 0)
		{
			return true;
		}
	}
	return false;
}

void CvGame::triggerDarkAge(int iDelayTurns, int iLenTurns, CvWString szStartTxtKey, CvWString szEndTxtKey)
{
	if (isDarkAge() || isDarkAgePending() || m_pDarkAgeState != NULL)
	{
		FAssert(false);
		return;
	}

	m_iDarkAgeCount++;
	
	m_pDarkAgeState = new DarkAgeData;
	m_pDarkAgeState->iDelayTurns = iDelayTurns;
	m_pDarkAgeState->iLenTurns = iLenTurns;
	m_pDarkAgeState->szStartTxtKey = szStartTxtKey;
	m_pDarkAgeState->szEndTxtKey = szEndTxtKey;
}

void CvGame::doDarkAge()
{
	if (m_pDarkAgeState != NULL)
	{
		if (m_pDarkAgeState->iDelayTurns > 0)
		{
			m_pDarkAgeState->iDelayTurns--;

			if (m_pDarkAgeState->iDelayTurns == 0)
			{
				CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_DARK_AGE, 0, 0, 0);
				for (int iI = 0; iI < MAX_PLAYERS; iI++)
				{
					if (GET_PLAYER((PlayerTypes)iI).isAlive())
					{
						if (GET_PLAYER((PlayerTypes)iI).isHuman())
						{
							if (pInfo)
							{
								pInfo->setText(m_pDarkAgeState->szStartTxtKey);
								gDLL->getInterfaceIFace()->addPopup(pInfo, (PlayerTypes)iI, true);
							}
						}

						MEMORY_TRACK_EXEMPT();
						AddDLLMessage((PlayerTypes)iI, true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText(m_pDarkAgeState->szStartTxtKey).GetCString(), "AS2D_GLOBALWARMING", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"));
					}
				}
				for (int iI = 0; iI < MAX_PLAYERS; iI++)
				{
					if (GET_PLAYER((PlayerTypes)iI).isAlive())
					{
						GET_PLAYER((PlayerTypes)iI).updateYield();
					}
				}
			}
		}

		else if (m_pDarkAgeState->iLenTurns > 0)
		{
			m_pDarkAgeState->iLenTurns--;

			if (m_pDarkAgeState->iLenTurns == 0)
			{
				CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_DARK_AGE, 0, 0, 0);
				for (int iI = 0; iI < MAX_PLAYERS; iI++)
				{
					if (GET_PLAYER((PlayerTypes)iI).isAlive())
					{
						if (GET_PLAYER((PlayerTypes)iI).isHuman())
						{
							if (pInfo)
							{
								pInfo->setText(m_pDarkAgeState->szEndTxtKey);
								gDLL->getInterfaceIFace()->addPopup(pInfo, (PlayerTypes)iI, true);
							}
						}
						MEMORY_TRACK_EXEMPT();
						AddDLLMessage((PlayerTypes)iI, true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText(m_pDarkAgeState->szEndTxtKey).GetCString(), "AS2D_WELOVEKING", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"));
					}
				}
				SAFE_DELETE(m_pDarkAgeState);
				m_pDarkAgeState = NULL;
				for (int iI = 0; iI < MAX_PLAYERS; iI++)
				{
					if (GET_PLAYER((PlayerTypes)iI).isAlive())
					{
						GET_PLAYER((PlayerTypes)iI).updateYield();
					}
				}
			}
		}
	}
}

int CvGame::getNumDarkAgesEverOccurred() const
{
	return m_iDarkAgeCount;
}
