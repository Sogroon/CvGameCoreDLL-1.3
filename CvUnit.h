#pragma once

// unit.h

#ifndef CIV4_UNIT_H
#define CIV4_UNIT_H

#include "CvDLLEntity.h"
//#include "CvEnums.h"
#include "CvStructs.h"  // don't disable! used for modules
#include "CvGameObject.h"
#include "CvCity.h"
#include "ByteBuffer.h"

#pragma warning( disable: 4251 )		// needs to have dll-interface to be used by clients of class

class CvPlot;
class CvArea;
class CvUnitInfo;
class CvSelectionGroup;
class FAStarNode;
class CvArtInfoUnit;

enum UnitValueFlags
{
	UNITVALUE_FLAGS_DEFENSIVE = 1,
	UNITVALUE_FLAGS_OFFENSIVE = 2,
	UNITVALUE_FLAGS_UTILITY = 4
};
#define	UNITVALUE_FLAGS_ALL (UNITVALUE_FLAGS_DEFENSIVE | UNITVALUE_FLAGS_OFFENSIVE | UNITVALUE_FLAGS_UTILITY)

inline UnitValueFlags operator|(UnitValueFlags a, UnitValueFlags b) 
{return static_cast<UnitValueFlags>(static_cast<int>(a) | static_cast<int>(b));} 

inline UnitValueFlags& operator|=(UnitValueFlags& a, const UnitValueFlags b) 
{ a = a | b; return a;} 

/************************************************************************************************/
/* Afforess	                  Start		 02/22/10                                               */
/*                                                                                              */
/* Defender Withdraw                                                                            */
/************************************************************************************************/
struct CombatResult
{
	bool bDefenderWithdrawn;
	CvPlot* pPlot;
	int iAttacksCount;
	bool bDeathMessaged;
};
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
struct DllExport CombatDetails					// Exposed to Python
{
	int iExtraCombatPercent;
	int iAnimalCombatModifierTA;
	int iAIAnimalCombatModifierTA;
	int iAnimalCombatModifierAA;
	int iAIAnimalCombatModifierAA;
	int iBarbarianCombatModifierTB;
	int iAIBarbarianCombatModifierTB;
	int iBarbarianCombatModifierAB;
	int iAIBarbarianCombatModifierAB;
	int iPlotDefenseModifier;
	int iFortifyModifier;
	int iCityDefenseModifier;
	int iHillsAttackModifier;
	int iHillsDefenseModifier;
	int iFeatureAttackModifier;
	int iFeatureDefenseModifier;
	int iTerrainAttackModifier;
	int iTerrainDefenseModifier;
	int iCityAttackModifier;
	int iDomainDefenseModifier;
	int iCityBarbarianDefenseModifier;
	int iClassDefenseModifier;
	int iClassAttackModifier;
	int iCombatModifierT;
	int iCombatModifierA;
	int iDomainModifierA;
	int iDomainModifierT;
	int iAnimalCombatModifierA;
	int iAnimalCombatModifierT;
	int iRiverAttackModifier;
	int iAmphibAttackModifier;
	int iKamikazeModifier;
	int iModifierTotal;
	int iBaseCombatStr;
	int iCombat;
	int iMaxCombatStr;
	int iCurrHitPoints;
	int iMaxHitPoints;
	int iCurrCombatStr;
	PlayerTypes eOwner;
	PlayerTypes eVisualOwner;
	std::wstring sUnitName;
};

class PromotionKeyedInfo
{
public:
	PromotionKeyedInfo() :	m_bHasPromotion(false)
	{
	}
	bool	m_bHasPromotion;
};

typedef std::map<PromotionTypes, PromotionKeyedInfo>::iterator PromotionIterator;

class UnitCombatKeyedInfo
{
public:
	UnitCombatKeyedInfo() :	m_iExtraUnitCombatModifier(0),
								m_iSubCombatTypeCount(0),
								m_iOngoingTrainingCount(0),
								m_iRemovesUnitCombatTypeCount(0),
								m_iExtraFlankingStrengthbyUnitCombatType(0),
								m_iExtraWithdrawVSUnitCombatType(0),
								m_iExtraRepelVSUnitCombatType(0),
								m_iExtraKnockbackVSUnitCombatType(0),
								m_iExtraPunctureVSUnitCombatType(0),
								m_iExtraArmorVSUnitCombatType(0),
								m_iExtraDodgeVSUnitCombatType(0),
								m_iExtraPrecisionVSUnitCombatType(0),
								m_iExtraCriticalVSUnitCombatType(0),
								m_iExtraRoundStunVSUnitCombatType(0)
	{
	}

	int	m_iExtraUnitCombatModifier;
	int m_iSubCombatTypeCount;
	int m_iOngoingTrainingCount;
	int m_iRemovesUnitCombatTypeCount;
	int m_iExtraFlankingStrengthbyUnitCombatType;
	int m_iExtraWithdrawVSUnitCombatType;
	int m_iExtraRepelVSUnitCombatType;
	int m_iExtraKnockbackVSUnitCombatType;
	int m_iExtraPunctureVSUnitCombatType;
	int m_iExtraArmorVSUnitCombatType;
	int m_iExtraDodgeVSUnitCombatType;
	int m_iExtraPrecisionVSUnitCombatType;
	int m_iExtraCriticalVSUnitCombatType;
	int m_iExtraRoundStunVSUnitCombatType;
};

class CvUnit : public CvDLLEntity
{

public:

	CvUnit(bool bIsDummy = false);
	virtual ~CvUnit();

	void* operator new(size_t size) { return GC.getGameINLINE().getAllocator().allocate(size); }
	void operator delete(void* instance) { GC.getGameINLINE().getAllocator().deAllocate(instance); }
	void* operator new[](size_t size) { return GC.getGameINLINE().getAllocator().allocate(size); }
	void operator delete[](void* instance) { GC.getGameINLINE().getAllocator().deAllocate(instance); }

	CvGameObjectUnit* getGameObject() {return &m_GameObject;};
	const CvGameObjectUnit* getGameObjectConst() const {return (const CvGameObjectUnit*)&m_GameObject;};

protected:
	CvGameObjectUnit m_GameObject;

public:
	// < M.A.D. Nukes Start >
	bool isMADEnabled() const;
	void setMADEnabled(bool bValue);
	CvPlot* getMADTargetPlot() const;
	int getMADTargetPlotX() const;
	int getMADTargetPlotY() const;
	void setMADTargetPlot(CvPlot* pPlot);
	bool setMADTargetPlot(int iX, int iY);
	bool clearMADTargetPlot();
	PlayerTypes getMADTargetPlotOwner();
	void setMADTargetPlotOwner(PlayerTypes pPlayer);
	void doMADNukes(bool bForceRetarget);
protected:
	bool m_bMADEnabled;
	int m_iMADTargetPlotX;
	int m_iMADTargetPlotY;
	PlayerTypes m_pMADTargetPlotOwner;
public:
	// < M.A.D. Nukes End   >
	
	bool m_bForcedMove;
	
	void reloadEntity(bool bForceLoad = false);
	void init(int iID, UnitTypes eUnit, UnitAITypes eUnitAI, PlayerTypes eOwner, int iX, int iY, DirectionTypes eFacingDirection, int iBirthmark);
	void uninit();
	void changeIdentity(UnitTypes eUnit);
	void reset(int iID = 0, UnitTypes eUnit = NO_UNIT, PlayerTypes eOwner = NO_PLAYER, bool bConstructorCall = false, bool bIdentityChange = false);
	void setupGraphical();

	void convert(CvUnit* pUnit);																					// Exposed to Python
	void kill(bool bDelay, PlayerTypes ePlayer = NO_PLAYER, bool bMessaged = false);														// Exposed to Python
	void killUnconditional(bool bDelay, PlayerTypes ePlayer, bool bMessaged = false);	//	Used internally

	DllExport void NotifyEntity(MissionTypes eMission);

	void doTurn();

	void updateCombat(bool bQuick = false);
	void updateAirCombat(bool bQuick = false);
	void updateAirStrike(CvPlot* pPlot, bool bQuick, bool bFinish);

	bool isActionRecommended(int iAction);
	
	bool isBetterDefenderThan(CvUnit* pDefender, CvUnit* pAttacker) const;						// Exposed to Python 

	bool canDoCommand(CommandTypes eCommand, int iData1, int iData2, bool bTestVisible = false, bool bTestBusy = true);	// Exposed to Python
	void doCommand(CommandTypes eCommand, int iData1, int iData2);																// Exposed to Python

#ifdef USE_OLD_PATH_GENERATOR
	FAStarNode* getPathLastNode() const;
	int getPathMovementRemaining() const;
#else
	int getPathMovementRemaining() const;
#endif
	CvPlot* getPathEndTurnPlot() const;																																						// Exposed to Python
	bool generatePath(const CvPlot* pToPlot, int iFlags = 0, bool bReuse = false, int* piPathTurns = NULL, int iMaxTurns = MAX_INT, int iOptimizationLimit = -1) const;	// Exposed to Python

	bool canEnterTerritory(TeamTypes eTeam, bool bIgnoreRightOfPassage = false) const;						// Exposed to Python
	bool canEnterArea(TeamTypes eTeam, const CvArea* pArea, bool bIgnoreRightOfPassage = false) const;						// Exposed to Python
	TeamTypes getDeclareWarMove(const CvPlot* pPlot) const;															// Exposed to Python
/************************************************************************************************/
/* Afforess	                  Start		 06/17/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	bool canMoveInto(const CvPlot* pPlot, bool bAttack = false, bool bDeclareWar = false, bool bIgnoreLoad = false, bool bIgnoreTileLimit = false, bool bIgnoreLocation = false, bool bIgnoreAttack = false) const;	// Exposed to Python
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	bool canMoveOrAttackInto(const CvPlot* pPlot, bool bDeclareWar = false) const;								// Exposed to Python
	bool canMoveThrough(const CvPlot* pPlot, bool bDeclareWar = false) const;																								// Exposed to Python
	void attack(CvPlot* pPlot, bool bQuick);
	void attackForDamage(CvUnit *pDefender, int attackerDamageChange, int defenderDamageChange);
	void fightInterceptor(const CvPlot* pPlot, bool bQuick);
	void move(CvPlot* pPlot, bool bShow);
/************************************************************************************************/
/* Afforess	                  Start		 06/13/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	bool jumpToNearestValidPlot(bool bKill = true);								// Exposed to Python
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	bool canAutomate(AutomateTypes eAutomate) const;																							// Exposed to Python
	void automate(AutomateTypes eAutomate);

	bool canScrap() const;																																				// Exposed to Python
	void scrap();
/************************************************************************************************/
/* Afforess	                  Start		 02/14/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	bool canTradeUnit(PlayerTypes eReceivingPlayer);
	
	void tradeUnit(PlayerTypes eReceivingPlayer);
	bool spyNuke(int iX, int iY, bool bCaught);
	bool canClaimTerritory(const CvPlot* pPlot, bool bAdjacentOwner = false) const;
	bool claimTerritory();
	int getMaxHurryFood(CvCity* pCity) const;
	int getHurryFood(const CvPlot* pPlot) const;
	bool canHurryFood(const CvPlot* pPlot) const;
	bool hurryFood();
	
	bool canPerformInquisition(const CvPlot* pPlot) const;
	bool performInquisition();
	//SAD
	int surroundedDefenseModifier(const CvPlot *pPlot, const CvUnit *pDefender) const;

	int getCanMovePeaksCount() const;
	bool isCanMovePeaks() const;
	void changeCanMovePeaksCount(int iChange);
	//	Koshling - enhanced mountaineering mode to differentiate between ability to move through
	//	mountains, and ability to lead a stack through mountains
	int getCanLeadThroughPeaksCount() const;
	bool isCanLeadThroughPeaks() const;
	void changeCanLeadThroughPeaksCount(int iChange);
	
	DllExport PlayerTypes getNationality() const;
	void setNationality(PlayerTypes eNewNationality);
 	void combatWon(CvUnit* pLoser);

	int interceptionChance(const CvPlot* pPlot) const;
	
	int getRandomMinExperienceTimes100() const;

	//Great Commanders... By KillmePlease
	
	//for combat units:
	CvUnit* getCommander() const;
	void tryUseCommander();			//assigns m_pUsedCommander by call to getCommander() and consumes command points from used commander.
	bool isCommander() const;																//Exposed to python
	void setCommander(bool bNewVal);
	void nullUsedCommander();	//delete m_pUsedCommander
	void clearCommanderCache(void) ;	//	Should be called prior to each turn

	CvUnit* getUsedCommander() const;

	//for commander units:
	int controlPointsLeft() const;		//number of units can be commanded this turn		//Exposed to python
	int controlPoints() const;																	//Exposed to python
	int commandRange() const;																//Exposed to python
	bool isOnslaught() const;
	//from promotions:
	int getExtraControlPoints() const;			//control
	void changeExtraControlPoints(int iChange);
	int getExtraCommandRange() const;			//command
	void changeExtraCommandRange(int iChange);
	int getOnslaughtCount() const;
	void changeOnslaughtCount(int iChange);
	
	int getZoneOfControlCount() const;
	bool isZoneOfControl() const;
	void changeZoneOfControlCount(int iChange);
	
	bool sleepForEspionage();
	PlayerTypes getOriginalOwner() const;
	
	int getExperience100() const;												// Exposed to Python
	void setExperience100(int iNewValue, int iMax = -1);												// Exposed to Python
	void changeExperience100(int iChange, int iMax = -1, bool bFromCombat = false, bool bInBorders = false, bool bUpdateGlobal = false);												// Exposed to Python
	
	void doBattleFieldPromotions(CvUnit* pDefender, CombatDetails cdDefenderDetails, const CvPlot* pPlot, bool bAttackerHasLostNoHP, bool bAttackerWithdrawn, int iAttackerInitialDamage, int iWinningOdds, int iInitialAttXP, int iInitialAttGGXP, int iDefenderInitialDamage, int iInitialDefXP, int iInitialDefGGXP, bool &bAttackerPromoted, bool &bDefenderPromoted);
	void doDynamicXP(CvUnit* pDefender, const CvPlot* pPlot, int iAttackerInitialDamage, int iWinningOdds, int iDefenderInitialDamage, int iInitialAttXP, int iInitialDefXP, int iInitialAttGGXP, int iInitialDefGGXP, bool bPromotion, bool bDefPromotion);
	
	void changeTerrainProtected(TerrainTypes eIndex, int iNewValue);
	bool isTerrainProtected(TerrainTypes eIndex) const;
	int getTerrainProtectedCount(TerrainTypes eIndex) const;
	
	void doCommerceAttacks(const CvUnit* pDefender, const CvPlot* pPlot);
	
	bool isAutoPromoting() const;
	void setAutoPromoting(bool bNewValue);
	bool isAutoUpgrading() const;
	void setAutoUpgrading(bool bNewValue);
	
	const CvWString getDescription(uint uiForm = 0) const;

	bool canShadow() const;
	bool canShadowAt(CvPlot* pShadowPlot, CvUnit* pShadowUnit = NULL) const;
	
	void setShadowUnit(CvUnit* pUnit);
	CvUnit* getShadowUnit() const;

	TechTypes getDesiredDiscoveryTech() const;
	void setDesiredDiscoveryTech(TechTypes eTech);
	void waitForTech(int iFlag, int eTech);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	bool canGift(bool bTestVisible = false, bool bTestTransport = true);																											// Exposed to Python 
	void gift(bool bTestTransport = true);

	bool canLoadUnit(const CvUnit* pUnit, const CvPlot* pPlot) const;															// Exposed to Python
	void loadUnit(CvUnit* pUnit);

	bool canLoad(const CvPlot* pPlot) const;																											// Exposed to Python
	void load();
	bool shouldLoadOnMove(const CvPlot* pPlot) const;

	bool canUnload() const;																																				// Exposed to Python
	void unload();

	bool canUnloadAll() const;																																		// Exposed to Python
	void unloadAll();

	bool canHold(const CvPlot* pPlot) const;																											// Exposed to Python
	bool canSleep(const CvPlot* pPlot) const;																						// Exposed to Python
	bool canFortify(const CvPlot* pPlot) const;																					// Exposed to Python
	bool canAirPatrol(const CvPlot* pPlot) const;																									// Exposed to Python
	void airCircle(bool bStart);

	bool canSeaPatrol(const CvPlot* pPlot) const;																									// Exposed to Python

	bool canHeal(const CvPlot* pPlot) const;																											// Exposed to Python
	bool canSentry(const CvPlot* pPlot) const;																										// Exposed to Python

	int healRate(const CvPlot* pPlot) const;
	int healTurns(const CvPlot* pPlot) const;
	void doHeal();

	bool canAirlift(const CvPlot* pPlot) const;																										// Exposed to Python
	bool canAirliftAt(const CvPlot* pPlot, int iX, int iY) const;																	// Exposed to Python
	bool airlift(int iX, int iY);

	bool isNukeVictim(const CvPlot* pPlot, TeamTypes eTeam, int iRange = -1) const; // Exposed to Python
	bool canNuke(const CvPlot* pPlot) const;																											// Exposed to Python
/************************************************************************************************/
/* Afforess	                  Start		 09/09/10                                               */
/*                                                                                              */
/*  M.A.D Nukes                                                                                 */
/************************************************************************************************/
	bool canNukeAt(const CvPlot* pPlot, int iX, int iY, bool bTestAtWar = true) const;																	// Exposed to Python
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	bool nuke(int iX, int iY);
	// <f1rpo>
	int nukeInterceptionChance(CvPlot const& kTarget, TeamTypes* pBestTeam = NULL,
			bool (* const pTeamsAffected)[MAX_PLAYERS] = NULL) const; // </f1rpo>

	bool canRecon(const CvPlot* pPlot) const;																											// Exposed to Python
	bool canReconAt(const CvPlot* pPlot, int iX, int iY) const;																		// Exposed to Python
	bool recon(int iX, int iY);

	bool canAirBomb(const CvPlot* pPlot) const;																										// Exposed to Python
	bool canAirBombAt(const CvPlot* pPlot, int iX, int iY) const;																	// Exposed to Python
	bool airBomb(int iX, int iY);

	CvCity* bombardTarget(const CvPlot* pPlot) const;																							// Exposed to Python
	bool canBombard(const CvPlot* pPlot, bool bIgnoreHasAttacked = false) const;																										// Exposed to Python
	bool bombard();

	bool canParadrop(const CvPlot* pPlot) const;																											// Exposed to Python
	bool canParadropAt(const CvPlot* pPlot, int iX, int iY) const;																		// Exposed to Python
	bool paradrop(int iX, int iY);

	bool canPillage(const CvPlot* pPlot) const;																										// Exposed to Python
	bool pillage();

	//TSHEEP Assassin Mission
	bool canAssassin(const CvPlot* pPlot, bool bTestVisible) const;																										// Exposed to Python
	//int assassinCost(const CvUnit* pTargetUnit) const;
	//int assassinProb(const CvUnit* pTargetUnit) const;
	//bool assassin();
	//TSHEEP Other functions
	bool awardSpyExperience(TeamTypes eTargetTeam, int iModifier);
	//TSHEEP End
	//RevolutionDCM start
	bool canBribe(const CvPlot* pPlot, bool bTestVisible) const;
	//RevolutionDCM end

	bool canPlunder(const CvPlot* pPlot, bool bTestVisible = false) const;																					// Exposed to Python
	bool plunder();
	void updatePlunder(int iChange, bool bUpdatePlotGroups);

	int sabotageCost(const CvPlot* pPlot) const;																									// Exposed to Python
	int sabotageProb(const CvPlot* pPlot, ProbabilityTypes eProbStyle = PROBABILITY_REAL) const;	// Exposed to Python
	bool canSabotage(const CvPlot* pPlot, bool bTestVisible = false) const;												// Exposed to Python
	bool sabotage();

	int destroyCost(const CvPlot* pPlot) const;																										// Exposed to Python
	int destroyProb(const CvPlot* pPlot, ProbabilityTypes eProbStyle = PROBABILITY_REAL) const;		// Exposed to Python
	bool canDestroy(const CvPlot* pPlot, bool bTestVisible = false) const;												// Exposed to Python
	bool destroy();

	int stealPlansCost(const CvPlot* pPlot) const;																									// Exposed to Python
	int stealPlansProb(const CvPlot* pPlot, ProbabilityTypes eProbStyle = PROBABILITY_REAL) const;	// Exposed to Python
	bool canStealPlans(const CvPlot* pPlot, bool bTestVisible = false) const;												// Exposed to Python
	bool stealPlans();

/************************************************************************************************/
/* Great Diplomat MOD               START                                      Stolenrays 		*/
/************************************************************************************************/
	bool canBribeBarbarian(const CvPlot* pPlot, bool bTestVisible = false) const;												// Exposed to Python																							// Exposed to Python															// Exposed to Python
	bool BribeBarbarian();
	
	bool canGoodwill(const CvPlot* pPlot, bool bTestVisible = false) const;												// Exposed to Python																							// Exposed to Python															// Exposed to Python
	bool Goodwill();
	
	bool canUpgradeImprovements(const CvPlot* pPlot, CommandTypes eCommand) const;
	void upgradeImprovements(const CvPlot* pPlot, CommandTypes eCommand);

	bool canColonizeBarbarians(const CvPlot* plot) const;
	void colonizeBarbarians();
	
	bool canForcePeace() const;
	bool canForcePeaceWith(PlayerTypes ePlayer) const;
	void tryForcePeace();
	void applyForcePeace(PlayerTypes ePlayer);	
/************************************************************************************************/
/* Great Diplomat MOD               END                                                  		*/
/************************************************************************************************/
	
	bool canFound(const CvPlot* pPlot, bool bTestVisible = false) const;																		// Exposed to Python
	bool found();

	bool canSpread(const CvPlot* pPlot, ReligionTypes eReligion, bool bTestVisible = false) const;					// Exposed to Python
	bool spread(ReligionTypes eReligion);

	bool canSpreadCorporation(const CvPlot* pPlot, CorporationTypes eCorporation, bool bTestVisible = false) const;					// Exposed to Python
	bool spreadCorporation(CorporationTypes eCorporation);
	int spreadCorporationCost(CorporationTypes eCorporation, CvCity* pCity) const;

	bool canJoin(const CvPlot* pPlot, SpecialistTypes eSpecialist) const;																		// Exposed to Python
	bool join(SpecialistTypes eSpecialist);

	bool canConstruct(const CvPlot* pPlot, BuildingTypes eBuilding, bool bTestVisible = false) const;				// Exposed to Python
	bool construct(BuildingTypes eBuilding);

	TechTypes getDiscoveryTech() const;																														// Exposed to Python
	int getDiscoverResearch(TechTypes eTech) const;																								// Exposed to Python
	bool canDiscover(const CvPlot* pPlot) const;																									// Exposed to Python
	bool discover();

	int getMaxHurryProduction(CvCity* pCity) const;																													// Exposed to Python
	int getHurryProduction(const CvPlot* pPlot) const;																						// Exposed to Python
	bool canHurry(const CvPlot* pPlot, bool bTestVisible = false) const;													// Exposed to Python
	bool hurry();

	int getTradeGold(const CvPlot* pPlot) const;																									// Exposed to Python
	bool canTrade(const CvPlot* pPlot, bool bTestVisible = false) const;													// Exposed to Python
	bool trade();

	int getGreatWorkCulture(const CvPlot* pPlot) const;																						// Exposed to Python
	bool canGreatWork(const CvPlot* pPlot) const;																									// Exposed to Python
	bool greatWork();

	bool doOutcomeMission(MissionTypes eMission);

	int getEspionagePoints(const CvPlot* pPlot) const;																									// Exposed to Python
	bool canInfiltrate(const CvPlot* pPlot, bool bTestVisible = false) const;													// Exposed to Python
	bool infiltrate();

	bool canEspionage(const CvPlot* pPlot, bool bTestVisible = false) const;
	bool espionage(EspionageMissionTypes eMission, int iData);
	bool testSpyIntercepted(PlayerTypes eTargetPlayer, int iModifier = 0);
	int getSpyInterceptPercent(TeamTypes eTargetTeam) const;
	bool isIntruding() const;

	bool canGoldenAge(const CvPlot* pPlot, bool bTestVisible = false) const;																// Exposed to Python
	bool goldenAge();

	bool canBuild(const CvPlot* pPlot, BuildTypes eBuild, bool bTestVisible = false) const;				// Exposed to Python
	bool build(BuildTypes eBuild);

	bool canPromote(PromotionTypes ePromotion, int iLeaderUnitId) const;																												// Exposed to Python 
	void promote(PromotionTypes ePromotion, int iLeaderUnitId);																																// Exposed to Python 

	int canLead(const CvPlot* pPlot, int iUnitId) const;
	bool lead(int iUnitId);

	int canGiveExperience(const CvPlot* pPlot) const;																												// Exposed to Python 
	bool giveExperience();																																// Exposed to Python 
	int getStackExperienceToGive(int iNumUnits) const;

	int upgradePrice(UnitTypes eUnit) const;																											// Exposed to Python
	bool upgradeAvailable(UnitTypes eFromUnit, UnitClassTypes eToUnitClass) const;					// Exposed to Python
	bool canUpgrade(UnitTypes eUnit, bool bTestVisible = false) const;														// Exposed to Python
	bool isReadyForUpgrade() const;
	bool hasUpgrade(bool bSearch = false) const;																											// Exposed to Python
	bool hasUpgrade(UnitTypes eUnit, bool bSearch = false) const;
	CvCity* getUpgradeCity(bool bSearch = false) const;
	CvCity* getUpgradeCity(UnitTypes eUnit, bool bSearch = false, int* iSearchValue = NULL) const;
	void upgrade(UnitTypes eUnit);

	HandicapTypes getHandicapType() const;																// Exposed to Python		
	CivilizationTypes getCivilizationType() const;							// Exposed to Python								
	const wchar* getVisualCivAdjective(TeamTypes eForTeam) const;
// Thomas SG - AC: Advanced Cargo START
//	DllExport SpecialUnitTypes getSpecialUnitType() const;								// Exposed to Python								 
// Thomas SG - AC: Advanced Cargo END
	UnitTypes getCaptureUnitType(CivilizationTypes eCivilization) const;	// Exposed to Python								
	UnitCombatTypes getUnitCombatType() const;									// Exposed to Python								
	DllExport DomainTypes getDomainType() const;													// Exposed to Python								
	InvisibleTypes getInvisibleType() const;										// Exposed to Python								
	int getNumSeeInvisibleTypes() const;									// Exposed to Python
	InvisibleTypes getSeeInvisibleType(int i) const;									// Exposed to Python
																																				
// Thomas SG - AC: Advanced Cargo START
	SpecialUnitTypes getSpecialUnitType(int i) const;										// Exposed to Python								
	int getNumSpecialUnitTypes() const;													// Exposed to Python							
	int getNumSpecialCargos() const;									
	SpecialUnitTypes getSpecialCargo(int i) const;
// Thomas SG - AC: Advanced Cargo END
	int flavorValue(FlavorTypes eFlavor) const;														// Exposed to Python		

	bool isBarbarian() const;																							// Exposed to Python
	bool isHuman() const;																									// Exposed to Python

	int visibilityRange(const CvPlot* pPlot = NULL) const;																					// Exposed to Python

	int baseMoves() const;																			// Exposed to Python
	int maxMoves() const;																									// Exposed to Python
	int movesLeft() const;																			// Exposed to Python			
	DllExport bool canMove() const;																				// Exposed to Python			
	DllExport bool hasMoved() const;																			// Exposed to Python			
																																				
	int airRange() const;																				// Exposed to Python			
	int nukeRange() const;																			// Exposed to Python			 

	bool canBuildRoute() const;																						// Exposed to Python
	DllExport BuildTypes getBuildType() const;														// Exposed to Python
	int workRate(bool bMax) const;															// Exposed to Python

	bool isAnimal() const;																								// Exposed to Python
	bool isNoBadGoodies() const;																					// Exposed to Python
	bool isOnlyDefensive() const;																					// Exposed to Python
	bool isNoCapture() const;																							// Exposed to Python 
	bool isRivalTerritory() const;																				// Exposed to Python 
	bool isMilitaryHappiness() const;																			// Exposed to Python
	bool isInvestigate() const;																						// Exposed to Python
	bool isCounterSpy() const;																						// Exposed to Python
	bool isSpy() const;
	bool isFound() const;																				// Exposed to Python
/********************************************************************************/
/**		REVOLUTION_MOD							1/1/08				DPII		*/
/**																				*/
/**		 																		*/
/********************************************************************************/
	/*bool isCanBeRebel() const;
	bool isCanRebelCapture() const;
	bool isCannotDefect() const;
	bool isCanQuellRebellion() const;
	*/
/********************************************************************************/
/**		REVOLUTION_MOD							END								*/
/********************************************************************************/
	bool isGoldenAge() const;																							// Exposed to Python
	bool canCoexistWithEnemyUnit(TeamTypes eTeam) const;																				// Exposed to Python

	DllExport bool isFighting() const;																		// Exposed to Python						
	DllExport bool isAttacking() const;																		// Exposed to Python						
	DllExport bool isDefending() const;																		// Exposed to Python						
	bool isCombat() const;																								// Exposed to Python		

	bool isMilitaryUnit() const;
	bool isMilitaryAirUnit() const;
	bool isMilitaryNavalUnit() const;
	bool isMilitaryLandUnit() const;

	DllExport int maxHitPoints() const;																		// Exposed to Python						
	DllExport int currHitPoints() const;																	// Exposed to Python						
	bool isHurt() const;																				// Exposed to Python						
	DllExport bool isDead() const;																				// Exposed to Python						

	void setBaseCombatStr(int iCombat);																																										// Exposed to Python
	int baseCombatStr() const;																																										// Exposed to Python
	/*** Dexy - Surround and Destroy START ****/
	int maxCombatStr(const CvPlot* pPlot, const CvUnit* pAttacker, CombatDetails* pCombatDetails = NULL, bool bSurroundedModifier = true) const;		// Exposed to Python
	int currCombatStr(const CvPlot* pPlot, const CvUnit* pAttacker, CombatDetails* pCombatDetails = NULL, bool bSurroundedModifier = true) const;	// Exposed to Python
	// OLD CODE
	// int maxCombatStr(const CvPlot* pPlot, const CvUnit* pAttacker, CombatDetails* pCombatDetails = NULL) const;		// Exposed to Python
	// int currCombatStr(const CvPlot* pPlot, const CvUnit* pAttacker, CombatDetails* pCombatDetails = NULL) const;	// Exposed to Python
	/*** Dexy - Surround and Destroy  END  ****/
	int currFirepower(const CvPlot* pPlot, const CvUnit* pAttacker) const;																				// Exposed to Python
	int currEffectiveStr(const CvPlot* pPlot, const CvUnit* pAttacker, CombatDetails* pCombatDetails = NULL) const;
	DllExport float maxCombatStrFloat(const CvPlot* pPlot, const CvUnit* pAttacker) const;																	// Exposed to Python
	DllExport float currCombatStrFloat(const CvPlot* pPlot, const CvUnit* pAttacker) const;																	// Exposed to Python

	DllExport bool canFight() const;																									// Exposed to Python
	bool canAttack() const;																														// Exposed to Python
	bool canAttack(const CvUnit& defender) const;
	bool canDefend(const CvPlot* pPlot = NULL) const;																	// Exposed to Python
	bool canSiege(TeamTypes eTeam) const;																							// Exposed to Python

	int airBaseCombatStr() const;																						// Exposed to Python
	int airMaxCombatStr(const CvUnit* pOther) const;																						// Exposed to Python
	int airCurrCombatStr(const CvUnit* pOther) const;																						// Exposed to Python
	DllExport float airMaxCombatStrFloat(const CvUnit* pOther) const;																			// Exposed to Python
	DllExport float airCurrCombatStrFloat(const CvUnit* pOther) const;																		// Exposed to Python
	int combatLimit() const;																												// Exposed to Python
	int airCombatLimit() const;																												// Exposed to Python
	DllExport bool canAirAttack() const;																							// Exposed to Python
	DllExport bool canAirDefend(const CvPlot* pPlot = NULL) const;										// Exposed to Python
	int airCombatDamage(const CvUnit* pDefender) const;																// Exposed to Python
	int rangeCombatDamage(const CvUnit* pDefender) const;																// Exposed to Python
	CvUnit* bestInterceptor(const CvPlot* pPlot) const;																// Exposed to Python
	CvUnit* bestSeaPillageInterceptor(CvUnit* pPillager, int iMinOdds) const;																// Exposed to Python

	bool isAutomated() const;																								// Exposed to Python
	DllExport bool isWaiting() const;																									// Exposed to Python 
	bool isFortifyable() const;																							// Exposed to Python
	int fortifyModifier() const;																						// Exposed to Python

	int experienceNeeded() const;																						// Exposed to Python
	int attackXPValue() const;																												// Exposed to Python	
	int defenseXPValue() const;																												// Exposed to Python	
	int maxXPValue() const;																														// Exposed to Python	

	int firstStrikes() const;																								// Exposed to Python
	int chanceFirstStrikes() const;																					// Exposed to Python 
	int maxFirstStrikes() const;																						// Exposed to Python 
	DllExport bool isRanged() const;																									// Exposed to Python

	bool alwaysInvisible() const;																						// Exposed to Python
	bool immuneToFirstStrikes() const;
	bool isPirate() const;																			// Exposed to Python
																			
	bool noDefensiveBonus() const;																					// Exposed to Python
	bool ignoreBuildingDefense() const;																								// Exposed to Python
	bool canMoveImpassable() const;																										// Exposed to Python
	bool canMoveAllTerrain() const;																										// Exposed to Python
	bool flatMovementCost() const;																										// Exposed to Python
	bool ignoreTerrainCost() const;																										// Exposed to Python
	bool isNeverInvisible() const;																										// Exposed to Python
	DllExport bool isInvisible(TeamTypes eTeam, bool bDebug, bool bCheckCargo = true) const;										// Exposed to Python
	bool isNukeImmune() const;																												// Exposed to Python
/************************************************************************************************/
/* REVDCM_OC                              02/16/10                                phungus420    */
/*                                                                                              */
/* Inquisitions                                                                                 */
/************************************************************************************************/
	bool isInquisitor() const;										// Exposed to Python
/************************************************************************************************/
/* REVDCM_OC                               END                                                  */
/************************************************************************************************/

	int maxInterceptionProbability(bool bIgnoreCommanders = false) const;																	// Exposed to Python
	int currInterceptionProbability() const;																// Exposed to Python
	int evasionProbability(bool bIgnoreCommanders = false) const;																										// Exposed to Python
	int withdrawalProbability() const;																			// Exposed to Python

	bool hasCombatType(UnitCombatTypes eCombatType) const;
	bool hasSubCombatType(UnitCombatTypes eCombatType) const;
	int collateralDamage() const;																						// Exposed to Python
	int collateralDamageLimit() const;																								// Exposed to Python
	int collateralDamageMaxUnits() const;																							// Exposed to Python

	int cityAttackModifier() const;																					// Exposed to Python
	int cityDefenseModifier() const;																				// Exposed to Python
	int animalCombatModifier() const;																				// Exposed to Python
	int hillsAttackModifier() const;																				// Exposed to Python
	int hillsDefenseModifier() const;																				// Exposed to Python
	int terrainAttackModifier(TerrainTypes eTerrain) const;								// Exposed to Python
	int terrainDefenseModifier(TerrainTypes eTerrain) const;								// Exposed to Python
	int featureAttackModifier(FeatureTypes eFeature) const;								// Exposed to Python
	int featureDefenseModifier(FeatureTypes eFeature) const;								// Exposed to Python
	int unitClassAttackModifier(UnitClassTypes eUnitClass) const;						// Exposed to Python
	int unitClassDefenseModifier(UnitClassTypes eUnitClass) const;					// Exposed to Python
	int unitCombatModifier(UnitCombatTypes eUnitCombat) const;							// Exposed to Python
	int domainModifier(DomainTypes eDomain) const;													// Exposed to Python

	int bombardRate() const;																								// Exposed to Python
	int airBombBaseRate() const;																											// Exposed to Python
	int airBombCurrRate() const;																											// Exposed to Python

// Thomas SG - AC: Advanced Cargo START																// Exposed to Python
	int getSpecialCargoSpace(int iI) const;																	// Exposed to Python
	int getTotalSpecialCargoSpace() const;																	// Exposed to Python
// Thomas SG - AC: Advanced Cargo END
	DomainTypes domainCargo() const;																									// Exposed to Python
	int cargoSpace() const;																									// Exposed to Python
	void changeCargoSpace(int iChange);																									// Exposed to Python
// Thomas SG - AC: Advanced Cargo START																// Exposed to Python
	int totalCargoSpace() const;																									// Exposed to Python
	void changeTotalCargoSpace(int iChange);																									// Exposed to Python
// Thomas SG - AC: Advanced Cargo END
	bool isFull() const;																															// Exposed to Python
	int cargoSpaceAvailable(SpecialUnitTypes eSpecialCargo = NO_SPECIALUNIT, DomainTypes eDomainCargo = NO_DOMAIN) const;	// Exposed to Python
	bool hasCargo() const;																									// Exposed to Python
	bool canCargoAllMove() const;																											// Exposed to Python
	bool canCargoEnterArea(TeamTypes eTeam, const CvArea* pArea, bool bIgnoreRightOfPassage) const;
	int getUnitAICargo(UnitAITypes eUnitAI) const;																		// Exposed to Python

	DllExport int getID() const;																											// Exposed to Python
	int getIndex() const;
	DllExport IDInfo getIDInfo() const;
	void setID(int iID);

	int getGroupID() const;																														// Exposed to Python
	bool isInGroup() const;																														// Exposed to Python
	bool isGroupHead() const;																								// Exposed to Python
	DllExport CvSelectionGroup* getGroup() const;																			// Exposed to Python
	bool canJoinGroup(const CvPlot* pPlot, CvSelectionGroup* pSelectionGroup) const;
	DllExport void joinGroup(CvSelectionGroup* pSelectionGroup, bool bRemoveSelected = false, bool bRejoin = true);

	DllExport int getHotKeyNumber();																													// Exposed to Python
	void setHotKeyNumber(int iNewValue);																											// Exposed to Python

	DllExport int getViewportX() const;																																// Exposed to Python
	inline int getX_INLINE() const
	{
		return m_iX;
	}
	inline int getX() const
	{
		return m_iX;
	}
	DllExport int getViewportY() const;																																// Exposed to Python
	inline int getY() const
	{
		return m_iY;
	}
	inline int getY_INLINE() const
	{
		return m_iY;
	}
	bool isInViewport(void) const;
	void setXY(int iX, int iY, bool bGroup = false, bool bUpdate = true, bool bShow = false, bool bCheckPlotVisible = false);	// Exposed to Python
	bool isTempUnit(void) const;
	bool at(int iX, int iY) const;																														// Exposed to Python
	void addMission(CvMissionDefinition* pMission);
	DllExport bool atPlot(const CvPlot* pPlot) const;																					// Exposed to Python
	CvPlot* plot() const;																														// Exposed to Python
	DllExport CvPlot* plotExternal() const;																														// Exposed to Python
	int getArea() const;																																			// Exposed to Python
	CvArea* area() const;																																			// Exposed to Python
	bool onMap() const;

	int getLastMoveTurn() const;
	void setLastMoveTurn(int iNewValue);

	CvPlot* getReconPlot() const;																															// Exposed to Python 
	void setReconPlot(CvPlot* pNewValue);																			// Exposed to Python

	int getGameTurnCreated() const;																														// Exposed to Python
	void setGameTurnCreated(int iNewValue);

	DllExport int getDamage() const;																													// Exposed to Python
	void setDamage(int iNewValue, PlayerTypes ePlayer = NO_PLAYER, bool bNotifyEntity = true);														// Exposed to Python
	void changeDamage(int iChange, PlayerTypes ePlayer = NO_PLAYER);													// Exposed to Python
	void setupPreCombatDamage(void);
	int getPreCombatDamage(void) const;

	int getMoves() const;																																			// Exposed to Python
	void setMoves(int iNewValue);																										// Exposed to Python
	void changeMoves(int iChange);																														// Exposed to Python
	void finishMoves();																																				// Exposed to Python

	int getExperience() const;																											// Exposed to Python
	void setExperience(int iNewValue, int iMax = -1);																// Exposed to Python
	void changeExperience(int iChange, int iMax = -1, bool bFromCombat = false, bool bInBorders = false, bool bUpdateGlobal = false);																				// Exposed to Python

	int getLevel() const;																														// Exposed to Python					
	void setLevel(int iNewValue);
	void changeLevel(int iChange);

	int getCargo() const;																														// Exposed to Python					
	void changeCargo(int iChange);
	void getCargoUnits(std::vector<CvUnit*>& aUnits) const;

	CvPlot* getAttackPlot() const;
	void setAttackPlot(const CvPlot* pNewValue, bool bAirCombat);
	bool isAirCombat() const;

	DllExport int getCombatTimer() const;
	void setCombatTimer(int iNewValue);
	void changeCombatTimer(int iChange);

	int getCombatFirstStrikes() const;			
	void setCombatFirstStrikes(int iNewValue);			
	void changeCombatFirstStrikes(int iChange);			

	int getFortifyTurns() const;																															// Exposed to Python
	void setFortifyTurns(int iNewValue);
	void changeFortifyTurns(int iChange);

	int getBlitzCount() const;			
	bool isBlitz() const;																														// Exposed to Python					
	void changeBlitzCount(int iChange);																												
																																														
	int getAmphibCount() const;																																
	bool isAmphib() const;																													// Exposed to Python					
	void changeAmphibCount(int iChange);																											
																																														
	int getRiverCount() const;																																
	bool isRiver() const;																														// Exposed to Python					
	void changeRiverCount(int iChange);																												
																																														
	int getEnemyRouteCount() const;																									
	bool isEnemyRoute() const;																											// Exposed to Python					
	void changeEnemyRouteCount(int iChange);																									
																																														
	int getAlwaysHealCount() const;																														
	bool isAlwaysHeal() const;																											// Exposed to Python					
	void changeAlwaysHealCount(int iChange);																									
																																														
	int getHillsDoubleMoveCount() const;																											
	bool isHillsDoubleMove() const;																									// Exposed to Python					
	void changeHillsDoubleMoveCount(int iChange);																							
																																														
	int getImmuneToFirstStrikesCount() const;																									
	void changeImmuneToFirstStrikesCount(int iChange);
	int getIsPirateCount() const;
	void changeIsPirateCount(int iChange);	
	int getOneUpCount() const;
	bool isOneUp() const;
	void changeOneUpCount(int iChange);
	int getSurvivorChance() const;
	void changeSurvivorChance(int iChange);
																																														
	int getExtraVisibilityRange() const;																						// Exposed to Python					
	void changeExtraVisibilityRange(int iChange);

	int getExtraMoves() const;																																// Exposed to Python
	void changeExtraMoves(int iChange);

	int getExtraMoveDiscount() const;																								// Exposed to Python
	void changeExtraMoveDiscount(int iChange);

	int getExtraAirRange() const;																																// Exposed to Python
	void changeExtraAirRange(int iChange);

	int getExtraIntercept(bool bIgnoreCommanders = false) const;																																// Exposed to Python
	void changeExtraIntercept(int iChange);

	int getExtraEvasion(bool bIgnoreCommanders = false) const;																																// Exposed to Python
	void changeExtraEvasion(int iChange);

	int getExtraFirstStrikes() const;																													// Exposed to Python
	void changeExtraFirstStrikes(int iChange);

	int getExtraChanceFirstStrikes() const;																										// Exposed to Python
	void changeExtraChanceFirstStrikes(int iChange);

	int getExtraWithdrawal(bool bIgnoreCommanders = false) const;																														// Exposed to Python
	void changeExtraWithdrawal(int iChange);

	
	int getExtraStrength (bool bIgnoreCommanders = false) const;
	int getSubCombatTypeCount(UnitCombatTypes eCombatType) const;																											
	bool hasExtraSubCombatType(UnitCombatTypes eCombatType) const;																									// Exposed to Python					
	void changeSubCombatTypeCount(UnitCombatTypes eCombatType, int iChange);

	int getRemovesUnitCombatTypeCount(UnitCombatTypes eCombatType) const;																											
	bool hasRemovesUnitCombatType(UnitCombatTypes eCombatType) const;																									// Exposed to Python					
	void changeRemovesUnitCombatTypeCount(UnitCombatTypes eCombatType, int iChange);
	int getExtraCollateralDamage() const;																											// Exposed to Python
	void changeExtraCollateralDamage(int iChange);

	int getExtraBombardRate() const;																													// Exposed to Python
	void changeExtraBombardRate(int iChange);

	int getExtraEnemyHeal() const;																									// Exposed to Python					
	void changeExtraEnemyHeal(int iChange);																										
																																														
	int getExtraNeutralHeal() const;																								// Exposed to Python					
	void changeExtraNeutralHeal(int iChange);																									
																																														
	int getExtraFriendlyHeal() const;																								// Exposed to Python					
	void changeExtraFriendlyHeal(int iChange);																								
																																														
	int getSameTileHeal() const;																										// Exposed to Python					
	void changeSameTileHeal(int iChange);																											
																																														
	int getAdjacentTileHeal() const;																								// Exposed to Python					
	void changeAdjacentTileHeal(int iChange);																									
																																														
	int getExtraCombatPercent() const;																							// Exposed to Python					
	void changeExtraCombatPercent(int iChange);

	int getExtraCityAttackPercent() const;																										// Exposed to Python
	void changeExtraCityAttackPercent(int iChange);

	int getExtraCityDefensePercent() const;																										// Exposed to Python
	void changeExtraCityDefensePercent(int iChange);

	int getExtraHillsAttackPercent() const;																									// Exposed to Python
	void changeExtraHillsAttackPercent(int iChange);

	int getExtraHillsDefensePercent() const;																									// Exposed to Python
	void changeExtraHillsDefensePercent(int iChange);

	int getRevoltProtection() const;																									// Exposed to Python
	void changeRevoltProtection(int iChange);

	int getCollateralDamageProtection() const;																									// Exposed to Python
	void changeCollateralDamageProtection(int iChange);

	int getPillageChange() const;																									// Exposed to Python
	void changePillageChange(int iChange);

	int getUpgradeDiscount() const;																									// Exposed to Python
	void changeUpgradeDiscount(int iChange);

	int getExperiencePercent() const;																									// Exposed to Python
	void changeExperiencePercent(int iChange);

	int getKamikazePercent() const;																									// Exposed to Python
	void changeKamikazePercent(int iChange);

	DllExport DirectionTypes getFacingDirection(bool checkLineOfSightProperty) const;
	void setFacingDirection(DirectionTypes facingDirection);
	void rotateFacingDirectionClockwise();
	void rotateFacingDirectionCounterClockwise();

	DllExport bool isSuicide() const;																											// Exposed to Python
	int getDropRange() const;

	bool isMadeAttack() const;																																// Exposed to Python
	void setMadeAttack(bool bNewValue);																							// Exposed to Python

	bool isMadeInterception() const;																													// Exposed to Python
	void setMadeInterception(bool bNewValue);																				// Exposed to Python

	DllExport bool isPromotionReady() const;																									// Exposed to Python
	void setPromotionReady(bool bNewValue);																					// Exposed to Python
	void testPromotionReady();

	bool isDelayedDeath() const;
	void startDelayedDeath();
	bool doDelayedDeath();

	bool isCombatFocus() const;

	DllExport bool isInfoBarDirty() const;
	DllExport void setInfoBarDirty(bool bNewValue);

	bool isBlockading() const;
	void setBlockading(bool bNewValue);
	void collectBlockadeGold();

	DllExport PlayerTypes getOwner() const;																									// Exposed to Python
#ifdef _USRDLL
	inline PlayerTypes getOwnerINLINE() const
	{
		return m_eOwner;
	}
#endif
	DllExport PlayerTypes getVisualOwner(TeamTypes eForTeam = NO_TEAM) const;																									// Exposed to Python
	PlayerTypes getCombatOwner(TeamTypes eForTeam, const CvPlot* pPlot) const;																									// Exposed to Python
	DllExport TeamTypes getTeam() const;																										// Exposed to Python

	PlayerTypes getCapturingPlayer() const;
	void setCapturingPlayer(PlayerTypes eNewValue);

	DllExport const UnitTypes getUnitType() const;																					// Exposed to Python
	CvUnitInfo &getUnitInfo() const;
	UnitClassTypes getUnitClassType() const;	// Exposed to Python

	DllExport const UnitTypes getLeaderUnitType() const;
	void setLeaderUnitType(UnitTypes leaderUnitType);

	DllExport CvUnit* getCombatUnit() const;
	void setCombatUnit(CvUnit* pUnit, bool bAttacking = false);
	bool showSeigeTower(CvUnit* pDefender) const; // K-Mod

	CvUnit* getTransportUnit() const;																							// Exposed to Python
	bool isCargo() const;																													// Exposed to Python
	void setTransportUnit(CvUnit* pTransportUnit);																							// Exposed to Python

	int getExtraDomainModifier(DomainTypes eIndex) const;																		// Exposed to Python
	void changeExtraDomainModifier(DomainTypes eIndex, int iChange);

	DllExport const CvWString getName(uint uiForm = 0) const;																// Exposed to Python
// BUG - Unit Name - start
	bool isDescInName() const;
// BUG - Unit Name - end
	DllExport const wchar* getNameKey() const;																							// Exposed to Python
	const CvWString& getNameNoDesc() const;																				// Exposed to Python
	void setName(const CvWString szNewValue);																			// Exposed to Python

	// Script data needs to be a narrow string for pickling in Python
	std::string getScriptData() const;																											// Exposed to Python
	void setScriptData(std::string szNewValue);																							// Exposed to Python

	int getTerrainDoubleMoveCount(TerrainTypes eIndex) const;
	bool isTerrainDoubleMove(TerrainTypes eIndex) const;													// Exposed to Python 
	void changeTerrainDoubleMoveCount(TerrainTypes eIndex, int iChange);

	int getFeatureDoubleMoveCount(FeatureTypes eIndex) const;
	bool isFeatureDoubleMove(FeatureTypes eIndex) const;													// Exposed to Python 
	void changeFeatureDoubleMoveCount(FeatureTypes eIndex, int iChange);

	int getExtraTerrainAttackPercent(TerrainTypes eIndex) const;														// Exposed to Python
	void changeExtraTerrainAttackPercent(TerrainTypes eIndex, int iChange);						
	int getExtraTerrainDefensePercent(TerrainTypes eIndex) const;														// Exposed to Python
	void changeExtraTerrainDefensePercent(TerrainTypes eIndex, int iChange);						
	int getExtraFeatureAttackPercent(FeatureTypes eIndex) const;														// Exposed to Python
	void changeExtraFeatureAttackPercent(FeatureTypes eIndex, int iChange);
	int getExtraFeatureDefensePercent(FeatureTypes eIndex) const;														// Exposed to Python
	void changeExtraFeatureDefensePercent(FeatureTypes eIndex, int iChange);

	int getExtraUnitCombatModifier(UnitCombatTypes eIndex) const;														// Exposed to Python
	void changeExtraUnitCombatModifier(UnitCombatTypes eIndex, int iChange);

	bool canAcquirePromotion(PromotionTypes ePromotion, bool bIgnoreHas = false) const;															// Exposed to Python 
	bool canAcquirePromotionAny() const;																										// Exposed to Python
	bool isPromotionValid(PromotionTypes ePromotion) const;															// Exposed to Python 
	bool isHasPromotion(PromotionTypes eIndex) const;															// Exposed to Python
	void setHasPromotion(PromotionTypes eIndex, bool bNewValue);									// Exposed to Python

	DllExport int getSubUnitCount() const;
	DllExport int getSubUnitsAlive() const;
	int getSubUnitsAlive(int iDamage) const;

	bool isTargetOf(const CvUnit& attacker) const;

	bool isEnemy(TeamTypes eTeam, const CvPlot* pPlot = NULL) const;
	bool isPotentialEnemy(TeamTypes eTeam, const CvPlot* pPlot = NULL) const;

	int getTriggerValue(EventTriggerTypes eTrigger, const CvPlot* pPlot, bool bCheckPlot) const;
	bool canApplyEvent(EventTypes eEvent) const;
	void applyEvent(EventTypes eEvent);

	int getImmobileTimer() const;															// Exposed to Python
	void setImmobileTimer(int iNewValue);													// Exposed to Python
	void changeImmobileTimer(int iChange);

//Team Project (2)
	bool isCanRespawn() const;
	void setCanRespawn(bool bNewValue);

	bool isSurvivor() const;
	void setSurvivor(bool bNewValue);

	bool potentialWarAction(const CvPlot* pPlot) const;
	bool willRevealByMove(const CvPlot* pPlot) const;

	bool isAlwaysHostile(const CvPlot* pPlot) const;

	bool verifyStackValid();

	DllExport const CvArtInfoUnit* getArtInfo(int i, EraTypes eEra) const;										// Exposed to Python
	DllExport const TCHAR* getButton() const;										// Exposed to Python
	DllExport int getGroupSize() const;
	DllExport int getGroupDefinitions() const;
	DllExport int getUnitGroupRequired(int i) const;
	DllExport bool isRenderAlways() const;
	DllExport float getAnimationMaxSpeed() const;
	DllExport float getAnimationPadTime() const;
	DllExport const char* getFormationType() const;
	DllExport bool isMechUnit() const;
	DllExport bool isRenderBelowWater() const;
	DllExport int getRenderPriority(UnitSubEntityTypes eUnitSubEntity, int iMeshGroupType, int UNIT_MAX_SUB_TYPES) const;

	DllExport bool shouldShowEnemyGlow(TeamTypes eForTeam) const;
	DllExport bool shouldShowFoundBorders() const;

	DllExport void cheat(bool bCtrl, bool bAlt, bool bShift);
	DllExport float getHealthBarModifier() const;
	DllExport void getLayerAnimationPaths(std::vector<AnimationPathTypes>& aAnimationPaths) const;
	DllExport int getSelectionSoundScript() const;

/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
// Dale - AB: Bombing START
	bool canAirBomb1(const CvPlot* pPlot) const;
	bool canAirBomb1At(const CvPlot* pPlot, int iX, int iY) const;
	bool airBomb1(int iX, int iY);
	bool canAirBomb2(const CvPlot* pPlot) const;
	bool canAirBomb2At(const CvPlot* pPlot, int iX, int iY) const;
	bool airBomb2(int iX, int iY);
	bool canAirBomb3(const CvPlot* pPlot) const;
	bool canAirBomb3At(const CvPlot* pPlot, int iX, int iY) const;
	bool airBomb3(int iX, int iY);
	bool canAirBomb4(const CvPlot* pPlot) const;
	bool canAirBomb4At(const CvPlot* pPlot, int iX, int iY) const;
	bool airBomb4(int iX, int iY);
	bool canAirBomb5(const CvPlot* pPlot) const;
	bool canAirBomb5At(const CvPlot* pPlot, int iX, int iY) const;
	bool airBomb5(int iX, int iY);
// Dale - AB: Bombing END

	int getVolleyRange() const;
	int getVolleyAccuracy(const int iDistance) const;
	int getVolleyDamage(const CvUnit* pVictim) const;
	bool canReduceCityDefense(const CvPlot* pFromPlot = NULL, const bool bIgnoreMadeAttack = false) const;
	bool canVolley() const;
	bool canVolleyAt(const CvPlot* pFromPlot, int iX, int iY) const;
	bool doVolley(int iX, int iY);

// Dale - SA: Stack Attack START
	void updateStackCombat(bool bQuick = false);
// Dale - SA: Stack Attack END
// Dale - SA: Opp Fire START
	void doOpportunityFire();
// Dale - SA: Opp Fire END
// Dale - SA: Active Defense START
	void doActiveDefense();
// Dale - SA: Active Defense END
// Dale - FE: Fighters START
	bool canFEngage(const CvPlot* pPlot) const;
	bool canFEngageAt(const CvPlot* pPlot, int iX, int iY) const;
	bool fighterEngage(int iX, int iY);
// Dale - FE: Fighters END
/************************************************************************************************/
/* RevolutionDCM	                  Start		 05/31/10                        Afforess       */
/*                                                                                              */
/* Battle Effects                                                                               */
/************************************************************************************************/
	void setBattlePlot(CvPlot* pPlot, const CvUnit* pDefender = NULL);
/************************************************************************************************/
/* RevolutionDCM	             Battle Effects END                                             */
/************************************************************************************************/
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/

	void read(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);

	//Afforess - MP Resync
	void resync(bool bWrite, ByteBuffer* pBuffer);

	virtual void AI_init(UnitAITypes eUnitAI, int iBirthmark) = 0;
	virtual void AI_uninit() = 0;
	virtual void AI_reset(UnitAITypes eUnitAI = NO_UNITAI, bool bConstructor = false) = 0;
	virtual bool AI_update() = 0;
	virtual bool AI_follow() = 0;
	virtual void AI_upgrade() = 0;
	virtual void AI_promote() = 0;
	virtual int AI_groupFirstVal() = 0;
	virtual int AI_groupSecondVal() = 0;
	virtual int AI_attackOdds(const CvPlot* pPlot, bool bPotentialEnemy, CvUnit** ppDefender = NULL) = 0;
	//	Variant to test a specific defender AS IF it was in the specified plot
	virtual int AI_attackOddsAtPlot(const CvPlot* pPlot, CvUnit* pDefender, bool modifyPredictedResults = false) = 0;
	virtual bool AI_bestCityBuild(CvCity* pCity, CvPlot** ppBestPlot = NULL, BuildTypes* peBestBuild = NULL, CvPlot* pIgnorePlot = NULL, CvUnit* pUnit = NULL) = 0;
	virtual bool AI_isCityAIType() const = 0;
	virtual UnitAITypes AI_getUnitAIType() const = 0;																				// Exposed to Python
	virtual void AI_setUnitAIType(UnitAITypes eNewValue) = 0;
    virtual int AI_sacrificeValue(const CvPlot* pPlot) const = 0;
	virtual bool AI_isAwaitingContract(void) const = 0;
	virtual bool AI_isCityGarrison(const CvCity* pCity) const = 0;
	virtual void AI_setAsGarrison(const CvCity* pCity) = 0;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      04/05/10                                jdog5000      */
/*                                                                                              */
/* Unit AI                                                                                      */
/************************************************************************************************/
	virtual bool AI_load(UnitAITypes eUnitAI, MissionAITypes eMissionAI, UnitAITypes eTransportedUnitAI = NO_UNITAI, int iMinCargo = -1, int iMinCargoSpace = -1, int iMaxCargoSpace = -1, int iMaxCargoOurUnitAI = -1, int iFlags = 0, int iMaxPath = MAX_INT, int iMaxTransportPath = MAX_INT) = 0;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
	virtual void AI_flushValueCache() = 0;
	virtual int	AI_genericUnitValueTimes100(UnitValueFlags eFlags) const = 0;
	virtual int AI_getBirthmark() const = 0;

	inline int getMovementCharacteristicsHash(void) const { return m_movementCharacteristicsHash; }

protected:
/************************************************************************************************/
/* Afforess	                  Start		 06/14/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	int m_iCanMovePeaksCount; 
	//	Koshling - enhanced mountaineering mode to differentiate between ability to move through
	//	mountains, and ability to lead a stack through mountains
	int m_iCanLeadThroughPeaksCount; 

	int m_movementCharacteristicsHash;

	PlayerTypes m_eNationality;
	CombatResult m_combatResult;
	int m_iSleepTimer;
	PlayerTypes m_eOriginalOwner;
	bool m_bCommander;
	int* m_paiTerrainProtected;
	int m_iZoneOfControlCount;
	
	bool m_bAutoPromoting;
	bool m_bAutoUpgrading;
	IDInfo m_shadowUnit;
	TechTypes m_eDesiredDiscoveryTech;
	//Great Commanders... By KillmePlease
	int m_iOnslaughtCount;
	int m_iExtraControlPoints;
	int m_iExtraCommandRange;
	//auxillary members:
	int m_iControlPointsLeft;
	int m_iCommanderID;			//id of commander. used for game save/load
	mutable int m_iCommanderCacheTurn;
	mutable int m_iCachedCommander;
#define	NO_COMMANDER_ID	-2	//	Pseudo-id used to signify an assertion that the unit has no commander
	int m_iPreCombatDamage;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	int m_iID;
	int m_iGroupID;
	int m_iHotKeyNumber;
	int m_iX;
	int m_iY;
	int m_iLastMoveTurn;
	int m_iReconX;
	int m_iReconY;
	int m_iGameTurnCreated;
	int m_iDamage;
	int m_iMoves;
	int m_iExperience;
	int m_iLevel;
	int m_iCargo;
	int m_iCargoCapacity;
	// Thomas SG - AC: Advanced Cargo START
	int m_iTotalCargoCapacity;
	// Thomas SG - AC: Advanced Cargo END
	int m_iAttackPlotX;
	int m_iAttackPlotY;
	int m_iCombatTimer;
	int m_iCombatFirstStrikes;
	int m_iCombatDamage;
	int m_iFortifyTurns;
	int m_iBlitzCount;
	int m_iAmphibCount;
	int m_iRiverCount;
	int m_iEnemyRouteCount;
	int m_iAlwaysHealCount;
	int m_iHillsDoubleMoveCount;
	int m_iImmuneToFirstStrikesCount;
	int m_iIsPirateCount;	
	int m_iOneUpCount;
	int m_iSurvivorChance;
	int m_iExtraVisibilityRange;
	int m_iExtraMoves;
	int m_iExtraMoveDiscount;
	int m_iExtraAirRange;
	int m_iExtraIntercept;
	int m_iExtraEvasion;
	int m_iExtraFirstStrikes;
	int m_iExtraChanceFirstStrikes;
	int m_iExtraWithdrawal;
	int m_iExtraStrength;
	//int m_iRetrainsAvailable;
	int m_iExtraCollateralDamage;
	int m_iExtraBombardRate;
	int m_iExtraEnemyHeal;
	int m_iExtraNeutralHeal;
	int m_iExtraFriendlyHeal;
	int m_iSameTileHeal;
	int m_iAdjacentTileHeal;
	int m_iExtraCombatPercent;
	int m_iExtraCityAttackPercent;
	int m_iExtraCityDefensePercent;
	int m_iExtraHillsAttackPercent;
	int m_iExtraHillsDefensePercent;
	int m_iRevoltProtection;
	int m_iCollateralDamageProtection;
	int m_iPillageChange;
	int m_iUpgradeDiscount;
	int m_iExperiencePercent;
	int m_iKamikazePercent;
	int m_iBaseCombat;
	DirectionTypes m_eFacingDirection;
	int m_iImmobileTimer;
//Team Project (2)
/*****************************************************************************************************/
/**  Author: TheLadiesOgre                                                                          **/
/**  Date: 21.09.2009                                                                               **/
/**  ModComp: TLOTags                                                                               **/
/**  Reason Added: New Tag Definition                                                               **/
/**  Notes:                                                                                         **/
/*****************************************************************************************************/
	bool m_bCanRespawn;
	bool m_bSurvivor;
/*****************************************************************************************************/
/**  TheLadiesOgre; 21.09.2009; TLOTags                                                             **/
/*****************************************************************************************************/

	bool m_bMadeAttack;
	bool m_bMadeInterception;
	bool m_bPromotionReady;
	bool m_bDeathDelay;
	bool m_bCombatFocus;
	bool m_bInfoBarDirty;
	bool m_bBlockading;
	bool m_bAirCombat;

	PlayerTypes m_eOwner;
	PlayerTypes m_eCapturingPlayer;
	UnitTypes m_eUnitType;
	UnitTypes m_eLeaderUnitType;
	CvUnitInfo *m_pUnitInfo;

	IDInfo m_combatUnit;
	IDInfo m_transportUnit;

	int* m_aiExtraDomainModifier;

	CvWString m_szName;
	CvString m_szScriptData;

	mutable int	m_maxMoveCache;
	mutable int	m_iMaxMoveCacheTurn;

	std::map<PromotionTypes, PromotionKeyedInfo>	m_promotionKeyedInfo;
	std::map<UnitCombatTypes, UnitCombatKeyedInfo>	m_unitCombatKeyedInfo;

	bool* m_pabHasPromotion;

	int* m_paiTerrainDoubleMoveCount;
	int* m_paiFeatureDoubleMoveCount;
	int* m_paiExtraTerrainAttackPercent;
	int* m_paiExtraTerrainDefensePercent;
	int* m_paiExtraFeatureAttackPercent;
	int* m_paiExtraFeatureDefensePercent;
	int* m_paiExtraUnitCombatModifier;

	bool canAdvance(const CvPlot* pPlot, int iThreshold) const;
	void collateralCombat(const CvPlot* pPlot, CvUnit* pSkipUnit = NULL);
	void flankingStrikeCombat(const CvPlot* pPlot, int iAttackerStrength, int iAttackerFirepower, int iDefenderOdds, int iDefenderDamage, CvUnit* pSkipUnit = NULL);

	bool interceptTest(const CvPlot* pPlot);
	CvUnit* airStrikeTarget(const CvPlot* pPlot) const;
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
	// Dale - SA: Stack Attack START
public:
	bool canAirStrike(const CvPlot* pPlot) const;
	bool airStrike(CvPlot* pPlot);
protected:
	// Dale - SA: Stack Attack END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/

	int planBattle( CvBattleDefinition & kBattleDefinition ) const;
	int computeUnitsToDie( const CvBattleDefinition & kDefinition, bool bRanged, BattleUnitTypes iUnit ) const;
	bool verifyRoundsValid( const CvBattleDefinition & battleDefinition ) const;
	void increaseBattleRounds( CvBattleDefinition & battleDefinition ) const;
	int computeWaveSize( bool bRangedRound, int iAttackerMax, int iDefenderMax ) const;

	void getDefenderCombatValues(CvUnit& kDefender, const CvPlot* pPlot, int iOurStrength, int iOurFirepower, int& iTheirOdds, int& iTheirStrength, int& iOurDamage, int& iTheirDamage, CombatDetails* pTheirDetails = NULL, CvUnit* pDefender = NULL) const;

	bool isCombatVisible(const CvUnit* pDefender) const;
	void resolveCombat(CvUnit* pDefender, CvPlot* pPlot, CvBattleDefinition& kBattle);
	void resolveAirCombat(CvUnit* pInterceptor, CvPlot* pPlot, CvAirMissionDefinition& kBattle);
	void checkRemoveSelectionAfterAttack();

/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              Start                                                 */
/************************************************************************************************/

	// ------ BEGIN InfluenceDrivenWar -------------------------------
	float doVictoryInfluence(CvUnit* pLoserUnit, bool bAttacking, bool bWithdrawal);
	void influencePlots(CvPlot* pCentralPlot, PlayerTypes eTargetPlayer, float fLocationMultiplier);
	float doPillageInfluence();
	// ------ END InfluenceDrivenWar ---------------------------------
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              End                                                   */
/************************************************************************************************/
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/21/10                                jdog5000      */
/*                                                                                              */
/* Lead From Behind                                                                             */
/************************************************************************************************/
// From Lead From Behind by UncutDragon
public:
	int defenderValue(const CvUnit* pAttacker) const;
	bool isBetterDefenderThan(const CvUnit* pDefender, const CvUnit* pAttacker, int* pBestDefenderRank) const;
	virtual void LFBgetBetterAttacker(CvUnit** ppAttacker, const CvPlot* pPlot, bool bPotentialEnemy, int& iAIAttackOdds, int& iAttackerValue) const = 0;
	int LFBgetAttackerRank(const CvUnit* pDefender, int& iUnadjustedRank) const;
	int LFBgetDefenderRank(const CvUnit* pAttacker) const;
protected:
	int LFBgetDefenderOdds(const CvUnit* pAttacker) const;
	int LFBgetValueAdjustedOdds(int iOdds) const;
	int LFBgetRelativeValueRating() const;
	bool LFBisBetterDefenderThan(const CvUnit* pDefender, const CvUnit* pAttacker, int* pBestDefenderRank) const;
	int LFBgetDefenderCombatOdds(const CvUnit* pAttacker) const;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
	const UnitCombatKeyedInfo*	findUnitCombatKeyedInfo(UnitCombatTypes eUnitCombat) const;
	UnitCombatKeyedInfo*	findOrCreateUnitCombatKeyedInfo(UnitCombatTypes eUnitCombat);

	// AIAndy: Properties
	CvProperties m_Properties;
public:
	CvProperties* getProperties();
	const CvProperties* getPropertiesConst() const;

	PromotionIterator getPromotionBegin();
	PromotionIterator getPromotionEnd();

	//	KOSHLING - inform the AI of unit losses so that it can adjust internal counts
	virtual void AI_killed(void) = 0;
	//	Set an override to the group head priority for this unit, to allow a unit that
	//	would not nomally lead a stack to do so - priority will be preserved when other groups
	//	joing one lead by a unit with an override, but reset if we join another group
	virtual void AI_setLeaderPriority(int iPriority) = 0;	//	 -1 means reset to default
	virtual int AI_getPredictedHitPoints(void) const = 0;
	virtual void AI_setPredictedHitPoints(int iPredictedHitPoints) = 0;
	virtual bool AI_getHasAttacked(void) = 0;
	virtual int AI_beneficialPropertyValueToCity(CvCity* pCity) const = 0;

	bool isUsingDummyEntities(void);
	static bool isDummyEntity(const CvEntity* entity);
	static bool isRealEntity(const CvEntity* entity);

private:
	bool	bGraphicsSetup;
public:
	bool meetsUnitSelectionCriteria(CvUnitSelectionCriteria* criteria) const;
	bool shouldUseWithdrawalOddsCap() const;
	bool isPursuitinUse() const;
	//int getRetrainsAvailable() const;
// Movement Limits by 45deg - START
	bool isInsideMovementLimits (const CvPlot* pPlot) const;
	bool isOutsideMovementLimits (const CvPlot* pPlot) const;
// Movement Limits by 45deg - END
};

#endif
