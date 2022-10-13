//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:	CvUnitInfo.cpp
//
//  PURPOSE: Info class for units
//
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2003 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------
#include "CvGameCoreDLL.h"
#include "CvInfos.h"
#include "CvGlobals.h"
#include "CvArtFileMgr.h"
#include "CvXMLLoadUtility.h"
#include "CvDLLXMLIFaceBase.h"
#include "CvGameTextMgr.h"
#include "CvGameCoreUtils.h"
#include "CvGameAI.h"
#include "CvPlayerAI.h"


//======================================================================================================
//					CvUnitInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvUnitInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvUnitInfo::CvUnitInfo() :
// Toffer - New range combat
m_iVolleyRange(0),
m_iVolleyAccuracy(0),
m_iVolleyAccuracyMin(0),
m_iVolleyEfficiency(0),
m_iVolleyRounds(1),
// ! Toffer
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
// Dale - AB: Bombing START
m_bDCMAirBomb1(0),
m_bDCMAirBomb2(0),
m_bDCMAirBomb3(0),
m_bDCMAirBomb4(0),
m_bDCMAirBomb5(0),
// Dale - AB: Bombing END
// Dale - FE: Fighters START
m_bDCMFighterEngage(0),
// Dale - FE: Fighters END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
m_iPursuit(0), // f1rpo: Better initialize this
m_iAIWeight(0),
m_iProductionCost(0),
m_iHurryCostModifier(0),
m_iAdvancedStartCost(0),
m_iAdvancedStartCostIncrease(0),
m_iMinAreaSize(0),
m_iMoves(0),
m_iAirRange(0),
m_iAirUnitCap(0),
m_iDropRange(0),
m_iNukeRange(0),
m_iWorkRate(0),
m_iBaseDiscover(0),
m_iDiscoverMultiplier(0),
m_iBaseHurry(0),
m_iHurryMultiplier(0),
m_iBaseTrade(0),
m_iTradeMultiplier(0),
m_iGreatWorkCulture(0),
m_iEspionagePoints(0),
m_iCombat(0),
m_iCombatLimit(0),
m_iAirCombat(0),
m_iAirCombatLimit(0),
m_iXPValueAttack(0),
m_iXPValueDefense(0),
m_iFirstStrikes(0),
m_iChanceFirstStrikes(0),
m_iInterceptionProbability(0),
//ls612: Advanced Nuke Interception
//m_iNukeInterceptionProbability(0),
//m_iNukeInterceptionRange(0),
m_iEvasionProbability(0),
m_iWithdrawalProbability(0),
m_iCollateralDamage(0),
m_iCollateralDamageLimit(0),
m_iCollateralDamageMaxUnits(0),
m_iCityAttackModifier(0),
m_iCityDefenseModifier(0),
m_iAnimalCombatModifier(0),
m_iHillsAttackModifier(0),
m_iHillsDefenseModifier(0),
m_iBombRate(0),
m_iBombardRate(0),
// Thomas SG - AC: Advanced Cargo START
//m_iSpecialCargo(0),
// Thomas SG - AC: Advanced Cargo END
m_iDomainCargo(0),					
m_iCargoSpace(0),
// Thomas SG - AC: Advanced Cargo START
m_iTotalCargoSpace(0),
// Thomas SG - AC: Advanced Cargo END						
m_iConscriptionValue(0),
m_iCultureGarrisonValue(0),
m_iExtraCost(0),							
m_iAssetValue(0),						
m_iPowerValue(0),						
m_iUnitClassType(NO_UNITCLASS),
// Thomas SG - AC: Advanced Cargo START
//m_iSpecialUnitType(NO_SPECIALUNIT),
// Thomas SG - AC: Advanced Cargo END
m_iUnitCaptureClassType(NO_UNITCLASS),
m_iUnitCombatType(NO_UNITCOMBAT),
m_iDomainType(NO_DOMAIN),
m_iDefaultUnitAIType(NO_UNITAI),
m_iInvisibleType(NO_INVISIBLE),
m_iAdvisorType(NO_ADVISOR),

/********************************************************************************/
/**		REVDCM									2/16/10				phungus420	*/
/**																				*/
/**		CanTrain																*/
/********************************************************************************/
m_iMaxStartEra(NO_ERA),
m_iForceObsoleteTech(NO_TECH),
m_bStateReligion(false),
m_iPrereqGameOption(NO_GAMEOPTION),
m_iNotGameOption(NO_GAMEOPTION),
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

m_iHolyCity(NO_RELIGION),
m_iReligionType(NO_RELIGION),
m_iStateReligion(NO_RELIGION),
m_iPrereqReligion(NO_RELIGION),
m_iPrereqCorporation(NO_CORPORATION),		
m_iPrereqBuilding(NO_BUILDING),
m_iOverrideResourceBuilding(NO_BUILDING),
m_iPrereqAndTech(NO_TECH),
m_iPrereqAndBonus(NO_BONUS),
m_iGroupSize(0),
m_iGroupDefinitions(0),
m_iUnitMeleeWaveSize(0),
m_iUnitRangedWaveSize(0),
m_iNumUnitNames(0),
m_iCommandType(NO_COMMAND),
m_iLeaderExperience(0),
m_bAnimal(false),
m_bFoodProduction(false),
m_bNoBadGoodies(false),
m_bOnlyDefensive(false),
m_bNoCapture(false),
m_bQuickCombat(false),
m_bRivalTerritory(false),
m_bMilitaryHappiness(false),
m_bMilitarySupport(false),
m_bMilitaryProduction(false),
m_bPillage(false),
m_bSpy(false),
m_bSabotage(false),
m_bDestroy(false),
m_bStealPlans(false),
/************************************************************************************************/
/* Great Diplomat MOD               START                                      Stolenrays 		*/
/************************************************************************************************/
m_bBribeBarbarian(false),
m_bGoodwill(false),
m_bBarbariansColonisator(false),
m_bForcePeace(false),
m_bUpgradeImprovements(false),
/************************************************************************************************/
/* Great Diplomat MOD               END                                                  		*/
/************************************************************************************************/
m_bInvestigate(false),
m_bCounterSpy(false),
m_bFound(false),
m_bGoldenAge(false),
m_bInvisible(false),
m_bFirstStrikeImmune(false),
m_bNoDefensiveBonus(false),
m_bIgnoreBuildingDefense(false),
m_bCanMoveImpassable(false),
m_bCanMoveAllTerrain(false),
m_bFlatMovementCost(false),
m_bIgnoreTerrainCost(false),
m_bNukeImmune(false),
m_bPrereqBonuses(false),
m_bPrereqReligion(false),
m_bMechanized(false),
m_bRenderBelowWater(false),
m_bRenderAlways(false),
m_bSuicide(false),
m_bLineOfSight(false),
m_bHiddenNationality(false),
m_bAlwaysHostile(false),
m_bNoRevealMap(false),
/************************************************************************************************/
/* REVOLUTION_MOD                                 01/01/08                        DPII          */
/*                                                                                              */
/* CoreComponent                                                                                */
/************************************************************************************************/
m_bInquisitor(false),
m_bCanBeRebel(false),
m_bCanRebelCapture(false),
m_bCannotDefect(false),
m_bCanQuellRebellion(false),
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/

//ls612: Can't Enter non-Owned cities
m_bNoNonOwnedEntry(false),
m_iLeaderPromotion(NO_PROMOTION),
m_fUnitMaxSpeed(0.0f),
m_fUnitPadTime(0.0f),

/********************************************************************************/
/**		REVDCM									2/16/10				phungus420	*/
/**																				*/
/**		CanTrain																*/
/********************************************************************************/
m_pbPrereqOrCivics(NULL),
m_pbPrereqBuildingClass(NULL),
m_piPrereqBuildingClassOverrideTech(NULL),
m_piPrereqBuildingClassOverrideEra(NULL),
m_pbForceObsoleteUnitClass(NULL),
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

m_pbUpgradeUnitClass(NULL),
m_pbTargetUnitClass(NULL),
m_pbTargetUnitCombat(NULL),
m_pbDefenderUnitClass(NULL),
m_pbDefenderUnitCombat(NULL),
m_piFlankingStrikeUnitClass(NULL),
m_pbUnitAIType(NULL),
m_pbNotUnitAIType(NULL),
m_pbBuilds(NULL),
m_piReligionSpreads(NULL),
m_piCorporationSpreads(NULL),
m_piTerrainPassableTech(NULL),
m_piFeaturePassableTech(NULL),
m_pbGreatPeoples(NULL),
m_pbBuildings(NULL),
m_pbForceBuildings(NULL),
m_pbTerrainImpassable(NULL),
m_pbFeatureImpassable(NULL),
m_piPrereqAndTechs(NULL),
m_piPrereqOrBonuses(NULL),
m_piProductionTraits(NULL),
m_piFlavorValue(NULL),
m_piTerrainAttackModifier(NULL),
m_piTerrainDefenseModifier(NULL),
m_piFeatureAttackModifier(NULL),
m_piFeatureDefenseModifier(NULL),
m_piUnitClassAttackModifier(NULL),
m_piUnitClassDefenseModifier(NULL),
m_piUnitCombatModifier(NULL),
m_piUnitCombatCollateralImmune(NULL),
m_piDomainModifier(NULL),
m_piBonusProductionModifier(NULL),
m_piUnitGroupRequired(NULL),
m_pbTerrainNative(NULL),
m_pbFeatureNative(NULL),
m_pbFreePromotions(NULL),
m_paszEarlyArtDefineTags(NULL),
m_paszLateArtDefineTags(NULL),
m_paszMiddleArtDefineTags(NULL),
m_paszUnitNames(NULL)
/************************************************************************************************/
/* Afforess	                  Start		 Last Update: 3/8/10                                    */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
,m_iPrereqVicinityBonus(NO_BONUS)
,m_iBaseFoodChange(0)
,m_iControlPoints(0)
,m_iCommandRange(0)
,m_bWorkerTrade(false)
,m_bMilitaryTrade(false)
,m_bForceUpgrade(false)
,m_bGreatGeneral(false)
,m_bSlave(false)
,m_bRequiresStateReligionInCity(false)
,m_abHasCombatType(NULL)
,m_piPrereqOrVicinityBonuses(NULL)
,m_pbPassableRouteNeeded(NULL)
,m_paszClassicalArtDefineTags(NULL)
,m_paszRennArtDefineTags(NULL)
,m_paszIndustrialArtDefineTags(NULL)
,m_paszFutureArtDefineTags(NULL)
,m_paszCivilizationNames(NULL)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
,m_PropertyManipulators()
,m_pExprTrainCondition(NULL)
{
	m_zobristValue = GC.getGameINLINE().getSorenRand().getInt();
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvUnitInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvUnitInfo::~CvUnitInfo()
{

/********************************************************************************/
/**		REVDCM									2/16/10				phungus420	*/
/**																				*/
/**		CanTrain																*/
/********************************************************************************/
	SAFE_DELETE_ARRAY(m_pbPrereqOrCivics);
	SAFE_DELETE_ARRAY(m_pbPrereqBuildingClass);
	SAFE_DELETE_ARRAY(m_piPrereqBuildingClassOverrideTech);
	SAFE_DELETE_ARRAY(m_piPrereqBuildingClassOverrideEra);
	SAFE_DELETE_ARRAY(m_pbForceObsoleteUnitClass);
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

	SAFE_DELETE_ARRAY(m_pbUpgradeUnitClass);
	SAFE_DELETE_ARRAY(m_pbTargetUnitClass);
	SAFE_DELETE_ARRAY(m_pbTargetUnitCombat);
	SAFE_DELETE_ARRAY(m_pbDefenderUnitClass);
	SAFE_DELETE_ARRAY(m_pbDefenderUnitCombat);
	SAFE_DELETE_ARRAY(m_piFlankingStrikeUnitClass);
	SAFE_DELETE_ARRAY(m_pbUnitAIType);
	SAFE_DELETE_ARRAY(m_pbNotUnitAIType);
	SAFE_DELETE_ARRAY(m_pbBuilds);
	SAFE_DELETE_ARRAY(m_piReligionSpreads);
	SAFE_DELETE_ARRAY(m_piCorporationSpreads);
	SAFE_DELETE_ARRAY(m_piTerrainPassableTech);
	SAFE_DELETE_ARRAY(m_piFeaturePassableTech);
	SAFE_DELETE_ARRAY(m_pbGreatPeoples);
	SAFE_DELETE_ARRAY(m_pbBuildings);
	SAFE_DELETE_ARRAY(m_pbForceBuildings);
	SAFE_DELETE_ARRAY(m_pbTerrainImpassable);
	SAFE_DELETE_ARRAY(m_pbFeatureImpassable);
	SAFE_DELETE_ARRAY(m_piPrereqAndTechs);
	SAFE_DELETE_ARRAY(m_piPrereqOrBonuses);
	SAFE_DELETE_ARRAY(m_piProductionTraits);
	SAFE_DELETE_ARRAY(m_piFlavorValue);
	SAFE_DELETE_ARRAY(m_piTerrainAttackModifier);
	SAFE_DELETE_ARRAY(m_piTerrainDefenseModifier);
	SAFE_DELETE_ARRAY(m_piFeatureAttackModifier);
	SAFE_DELETE_ARRAY(m_piFeatureDefenseModifier);
	SAFE_DELETE_ARRAY(m_piUnitClassAttackModifier);
	SAFE_DELETE_ARRAY(m_piUnitClassDefenseModifier);
	SAFE_DELETE_ARRAY(m_piUnitCombatModifier);
	SAFE_DELETE_ARRAY(m_piUnitCombatCollateralImmune);
	SAFE_DELETE_ARRAY(m_piDomainModifier);
	SAFE_DELETE_ARRAY(m_piBonusProductionModifier);
	SAFE_DELETE_ARRAY(m_piUnitGroupRequired);
	SAFE_DELETE_ARRAY(m_pbTerrainNative);
	SAFE_DELETE_ARRAY(m_pbFeatureNative);
	SAFE_DELETE_ARRAY(m_pbFreePromotions);
	SAFE_DELETE_ARRAY(m_paszEarlyArtDefineTags);
	SAFE_DELETE_ARRAY(m_paszLateArtDefineTags);
	SAFE_DELETE_ARRAY(m_paszMiddleArtDefineTags);
	SAFE_DELETE_ARRAY(m_paszUnitNames);
/************************************************************************************************/
/* Afforess	                  Start		 Last Update: 3/8/10                                    */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	SAFE_DELETE_ARRAY(m_pbPrereqBuildingClass);
	SAFE_DELETE_ARRAY(m_piPrereqOrVicinityBonuses);
	SAFE_DELETE_ARRAY(m_pbPassableRouteNeeded);
	SAFE_DELETE_ARRAY(m_paszClassicalArtDefineTags);
	SAFE_DELETE_ARRAY(m_paszRennArtDefineTags);
	SAFE_DELETE_ARRAY(m_paszIndustrialArtDefineTags);
	SAFE_DELETE_ARRAY(m_paszFutureArtDefineTags);
	SAFE_DELETE_ARRAY(m_paszCivilizationNames);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	SAFE_DELETE(m_pExprTrainCondition);

	for (int i=0; i<(int)m_aOutcomeMissions.size(); i++)
	{
		SAFE_DELETE(m_aOutcomeMissions[i]);
	}
}

const wchar* CvUnitInfo::getExtraHoverText() const
{
	if (!m_szExtraHoverTextKey.empty())
	{
		return gDLL->getText(m_szExtraHoverTextKey);
	}
	else
	{
		return L"";
	}
}

// Toffer - New ranged combat
int CvUnitInfo::getVolleyRange() const
{
	return m_iVolleyRange;
}

int CvUnitInfo::getVolleyAccuracy() const
{
	return m_iVolleyAccuracy;
}

int CvUnitInfo::getVolleyAccuracyMin() const
{
	return m_iVolleyAccuracyMin;
}

int CvUnitInfo::getVolleyEfficiency() const
{
	return m_iVolleyEfficiency;
}

int CvUnitInfo::getVolleyRounds() const
{
	return m_iVolleyRounds;
}


// Dale - AB: Bombing START
bool CvUnitInfo::getDCMAirBomb1() const
{
	return m_bDCMAirBomb1;
}

bool CvUnitInfo::getDCMAirBomb2() const
{
	return m_bDCMAirBomb2;
}

bool CvUnitInfo::getDCMAirBomb3() const
{
	return m_bDCMAirBomb3;
}

bool CvUnitInfo::getDCMAirBomb4() const
{
	return m_bDCMAirBomb4;
}

bool CvUnitInfo::getDCMAirBomb5() const
{
	return m_bDCMAirBomb5;
}
// Dale - AB: Bombing END
// Dale - FE: Fighters START
bool CvUnitInfo::getDCMFighterEngage() const
{
	return m_bDCMFighterEngage;
}
// Dale - FE: Fighters END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/

int CvUnitInfo::getAIWeight() const			
{
	return m_iAIWeight;
}

int CvUnitInfo::getProductionCost() const			
{
	return m_iProductionCost;
}

int CvUnitInfo::getHurryCostModifier() const		
{
	return m_iHurryCostModifier;
}

int CvUnitInfo::getAdvancedStartCost() const			
{
	return m_iAdvancedStartCost;
}

int CvUnitInfo::getAdvancedStartCostIncrease() const			
{
	return m_iAdvancedStartCostIncrease;
}

int CvUnitInfo::getMinAreaSize() const
{
	return m_iMinAreaSize;
}

int CvUnitInfo::getMoves() const		
{
	return m_iMoves;
}

int CvUnitInfo::getAirRange() const
{
	return m_iAirRange;
}

int CvUnitInfo::getAirUnitCap() const
{
	return m_iAirUnitCap;
}

int CvUnitInfo::getDropRange() const
{
	return m_iDropRange;
}

int CvUnitInfo::getNukeRange() const
{
	return m_iNukeRange;
}

int CvUnitInfo::getWorkRate() const
{
	return m_iWorkRate;
}

int CvUnitInfo::getBaseDiscover() const
{
	return m_iBaseDiscover;
}

int CvUnitInfo::getDiscoverMultiplier() const
{
	return m_iDiscoverMultiplier;
}

int CvUnitInfo::getBaseHurry() const
{
	return m_iBaseHurry;
}

int CvUnitInfo::getHurryMultiplier() const
{
	return m_iHurryMultiplier;
}

int CvUnitInfo::getBaseTrade() const
{
	return m_iBaseTrade;
}

int CvUnitInfo::getTradeMultiplier() const			
{
	return m_iTradeMultiplier;
}

int CvUnitInfo::getGreatWorkCulture() const		
{
	return m_iGreatWorkCulture;
}

int CvUnitInfo::getEspionagePoints() const		
{
	return m_iEspionagePoints;
}

int CvUnitInfo::getCombat() const	
{
	return m_iCombat;
}

void CvUnitInfo::setCombat(int iNum)
{
	m_iCombat = iNum;
}

int CvUnitInfo::getCombatLimit() const	
{
	return m_iCombatLimit;
}

int CvUnitInfo::getAirCombat() const
{
	return m_iAirCombat;
}

int CvUnitInfo::getAirCombatLimit() const
{
	return m_iAirCombatLimit;
}

int CvUnitInfo::getXPValueAttack() const
{
	return m_iXPValueAttack;
}

int CvUnitInfo::getXPValueDefense() const			
{
	return m_iXPValueDefense;
}

int CvUnitInfo::getFirstStrikes() const
{
	return m_iFirstStrikes;
}

int CvUnitInfo::getChanceFirstStrikes() const	
{
	return m_iChanceFirstStrikes;
}

int CvUnitInfo::getInterceptionProbability() const
{
	return m_iInterceptionProbability;
}

//ls612: Advanced Nuke Interception
//int CvUnitInfo::getNukeInterceptionProbability() const
//{
//	return m_iNukeInterceptionProbability;
//}
//
//int CvUnitInfo::getNukeInterceptionRange() const
//{
//	return m_iNukeInterceptionRange;
//}

int CvUnitInfo::getEvasionProbability() const	
{
	return m_iEvasionProbability;
}

int CvUnitInfo::getWithdrawalProbability() const
{
	return m_iWithdrawalProbability;
}

int CvUnitInfo::getCollateralDamage() const		
{
	return m_iCollateralDamage;
}

int CvUnitInfo::getCollateralDamageLimit() const
{
	return m_iCollateralDamageLimit;
}

int CvUnitInfo::getCollateralDamageMaxUnits() const
{
	return m_iCollateralDamageMaxUnits;
}

int CvUnitInfo::getCityAttackModifier() const
{
	return m_iCityAttackModifier;
}

int CvUnitInfo::getCityDefenseModifier() const
{
	return m_iCityDefenseModifier;
}

int CvUnitInfo::getAnimalCombatModifier() const
{
	return m_iAnimalCombatModifier;
}

int CvUnitInfo::getHillsAttackModifier() const
{
	return m_iHillsAttackModifier;
}

int CvUnitInfo::getHillsDefenseModifier() const
{
	return m_iHillsDefenseModifier;
}

int CvUnitInfo::getBombRate() const
{
	return m_iBombRate;
}

int CvUnitInfo::getBombardRate() const
{
	return m_iBombardRate;
}

// Thomas SG - AC: Advanced Cargo START
//int CvUnitInfo::getSpecialCargo() const
//{
//	return m_iSpecialCargo;
//}
// Thomas SG - AC: Advanced Cargo END

int CvUnitInfo::getDomainCargo() const
{
	return m_iDomainCargo;
}

int CvUnitInfo::getCargoSpace() const
{
	return m_iCargoSpace;
}

int CvUnitInfo::getConscriptionValue() const
{
	return m_iConscriptionValue;
}

int CvUnitInfo::getCultureGarrisonValue() const
{
	return m_iCultureGarrisonValue;
}

int CvUnitInfo::getExtraCost() const
{
	return m_iExtraCost;
}

int CvUnitInfo::getAssetValue() const
{
	return m_iAssetValue;
}

int CvUnitInfo::getPowerValue() const
{
	return m_iPowerValue;
}

int CvUnitInfo::getUnitClassType() const
{
	return m_iUnitClassType;
}

// Thomas SG - AC: Advanced Cargo START
int CvUnitInfo::getSpecialUnitType(int i) const
{
	int iSize = m_aiSpecialUnitTypes.size();
	if ((i + 1) > iSize)
	{
		return -1;
	}
	FAssert(i < iSize);

	return m_aiSpecialUnitTypes[i];
}

int CvUnitInfo::getNumSpecialUnitTypes() const
{
	return (int)m_aiSpecialUnitTypes.size();
}
// Thomas SG - AC: Advanced Cargo END

int CvUnitInfo::getUnitCaptureClassType() const
{
	return m_iUnitCaptureClassType;
}

int CvUnitInfo::getUnitCombatType() const			
{
	return m_iUnitCombatType;
}

int CvUnitInfo::getDomainType() const
{
	return m_iDomainType;
}

int CvUnitInfo::getDefaultUnitAIType() const
{
	return m_iDefaultUnitAIType;
}

int CvUnitInfo::getInvisibleType() const
{
	return m_iInvisibleType;
}

int CvUnitInfo::getSeeInvisibleType(int i) const
{
	FAssert(i < (int)m_aiSeeInvisibleTypes.size());

	return m_aiSeeInvisibleTypes[i];
}

int CvUnitInfo::getNumSeeInvisibleTypes() const
{
	return (int)m_aiSeeInvisibleTypes.size();
}
// Thomas SG - AC: Advanced Cargo START
int CvUnitInfo::getSpecialCargo(int i) const
{
	FAssert(i < (int)m_aiSpecialCargos.size());

	return m_aiSpecialCargos[i];
}

int CvUnitInfo::getNumSpecialCargos() const
{
	return (int)m_aiSpecialCargos.size();
}

int CvUnitInfo::getSpecialCargoSpace(int i) const
{
	FAssert(i < (int)m_aiSpecialCargoSpaces.size());

	return m_aiSpecialCargoSpaces[i];
}

int CvUnitInfo::getNumSpecialCargoSpaces() const
{
	return (int)m_aiSpecialCargoSpaces.size();
}

int CvUnitInfo::getTotalCargoSpace() const
{
	return m_iTotalCargoSpace;
}

int CvUnitInfo::getTotalSpecialCargoSpace() const
{
	int iTotalSpecialCargoSpace = 0;
	for (int iI = 0;iI < getNumSpecialCargoSpaces();iI++)
	{
		if (getSpecialCargoSpace(iI) > 0)
		{
			iTotalSpecialCargoSpace += getSpecialCargoSpace(iI);
		}
	}
	return iTotalSpecialCargoSpace;
}
// Thomas SG - AC: Advanced Cargo END
int CvUnitInfo::getAdvisorType() const
{
	return m_iAdvisorType;
}

/********************************************************************************/
/**		REVDCM									2/16/10				phungus420	*/
/**																				*/
/**		CanTrain																*/
/********************************************************************************/
int CvUnitInfo::getMaxStartEra() const				
{
	return m_iMaxStartEra;
}

int CvUnitInfo::getForceObsoleteTech() const
{
	return m_iForceObsoleteTech;
}

bool CvUnitInfo::isStateReligion() const				
{
	return m_bStateReligion;
}

int CvUnitInfo::getPrereqGameOption() const
{
	return m_iPrereqGameOption;
}

int CvUnitInfo::getNotGameOption() const
{
	return m_iNotGameOption;
}
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

int CvUnitInfo::getHolyCity() const
{
	return m_iHolyCity;
}

int CvUnitInfo::getReligionType() const
{
	return m_iReligionType;
}

int CvUnitInfo::getStateReligion() const
{
	return m_iStateReligion;
}

int CvUnitInfo::getPrereqReligion() const			
{
	return m_iPrereqReligion;
}

int CvUnitInfo::getPrereqCorporation() const			
{
	return m_iPrereqCorporation;
}

int CvUnitInfo::getPrereqBuilding() const			
{
	return m_iPrereqBuilding;
}

int CvUnitInfo::getOverrideResourceBuilding() const			
{
	return m_iOverrideResourceBuilding;
}

int CvUnitInfo::getPrereqOrBuildingsNum() const			
{
	return m_aePrereqOrBuildings.size();
}

BuildingTypes CvUnitInfo::getPrereqOrBuilding(int i) const			
{
	FAssertMsg(i < getPrereqOrBuildingsNum(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_aePrereqOrBuildings[i];
}

int CvUnitInfo::getPrereqAndTech() const
{
	return m_iPrereqAndTech;
}

int CvUnitInfo::getPrereqAndBonus() const			
{
	return m_iPrereqAndBonus;
}

int CvUnitInfo::getGroupSize() const// the initial number of individuals in the unit group
{
	return m_iGroupSize;
}

int CvUnitInfo::getGroupDefinitions() const// the number of UnitMeshGroups for this unit
{
	return m_iGroupDefinitions;
}

int CvUnitInfo::getMeleeWaveSize() const
{
	return m_iUnitMeleeWaveSize;
}

int CvUnitInfo::getRangedWaveSize() const
{
	return m_iUnitRangedWaveSize;
}

int CvUnitInfo::getNumUnitNames() const
{
	return m_iNumUnitNames;
}

bool CvUnitInfo::isAnimal() const
{
	return m_bAnimal;
}

bool CvUnitInfo::isFoodProduction() const
{
	return m_bFoodProduction;
}

bool CvUnitInfo::isNoBadGoodies() const	
{
	return m_bNoBadGoodies;
}

bool CvUnitInfo::isOnlyDefensive() const
{
	return m_bOnlyDefensive;
}

bool CvUnitInfo::isNoCapture() const	
{
	return m_bNoCapture;
}

bool CvUnitInfo::isQuickCombat() const	
{
	return m_bQuickCombat;
}

bool CvUnitInfo::isRivalTerritory() const
{
	return m_bRivalTerritory;
}

bool CvUnitInfo::isMilitaryHappiness() const
{
	return m_bMilitaryHappiness;
}

bool CvUnitInfo::isMilitarySupport() const
{
	return m_bMilitarySupport;
}

bool CvUnitInfo::isMilitaryProduction() const
{
	return m_bMilitaryProduction;
}

bool CvUnitInfo::isPillage() const			
{
	return m_bPillage;
}

bool CvUnitInfo::isSpy() const			
{
	return m_bSpy;
}

bool CvUnitInfo::isSabotage() const			
{
	return m_bSabotage;
}

bool CvUnitInfo::isDestroy() const
{
	return m_bDestroy;
}

bool CvUnitInfo::isStealPlans() const
{
	return m_bStealPlans;
}

/************************************************************************************************/
/* Great Diplomat MOD               START                                      Stolenrays 		*/
/************************************************************************************************/
bool CvUnitInfo::isBribeBarbarian() const
{
	return m_bBribeBarbarian;
}

bool CvUnitInfo::isGoodwill() const
{
	return m_bGoodwill;
}

bool CvUnitInfo::isBarbariansColonisator() const
{
	return m_bBarbariansColonisator;
}

bool CvUnitInfo::canForcePeace() const
{
	return m_bForcePeace;
}

bool CvUnitInfo::isUpgradeImprovements() const
{
	return m_bUpgradeImprovements;
}
/************************************************************************************************/
/* Great Diplomat MOD               END                                                  		*/
/************************************************************************************************/

bool CvUnitInfo::isInvestigate() const	
{
	return m_bInvestigate;
}

bool CvUnitInfo::isCounterSpy() const		
{
	return m_bCounterSpy;
}

bool CvUnitInfo::isFound() const				
{
	return m_bFound;
}

bool CvUnitInfo::isGoldenAge() const		
{
	return m_bGoldenAge;
}

bool CvUnitInfo::isInvisible() const		
{
	return m_bInvisible;
}

void CvUnitInfo::setInvisible(bool bEnable)
{
	m_bInvisible = bEnable;
}

bool CvUnitInfo::isFirstStrikeImmune() const
{
	return m_bFirstStrikeImmune;
}

bool CvUnitInfo::isNoDefensiveBonus() const			
{
	return m_bNoDefensiveBonus;
}

bool CvUnitInfo::isIgnoreBuildingDefense() const		
{
	return m_bIgnoreBuildingDefense;
}

bool CvUnitInfo::isCanMoveImpassable() const
{
	return m_bCanMoveImpassable;
}

bool CvUnitInfo::isCanMoveAllTerrain() const
{
	return m_bCanMoveAllTerrain;
}

bool CvUnitInfo::isFlatMovementCost() const
{
	return m_bFlatMovementCost;
}

bool CvUnitInfo::isIgnoreTerrainCost() const
{
	return m_bIgnoreTerrainCost;
}

bool CvUnitInfo::isNukeImmune() const		
{
	return m_bNukeImmune;
}

bool CvUnitInfo::isPrereqBonuses() const
{
	return m_bPrereqBonuses;
}

bool CvUnitInfo::isPrereqReligion() const
{
	return m_bPrereqReligion;
}

bool CvUnitInfo::isMechUnit() const
{
	return m_bMechanized;
}

bool CvUnitInfo::isRenderBelowWater() const
{
	return m_bRenderBelowWater;
}

bool CvUnitInfo::isRenderAlways() const
{
	return m_bRenderAlways;
}

bool CvUnitInfo::isSuicide() const
{
	return m_bSuicide;
}

bool CvUnitInfo::isLineOfSight() const
{
	return m_bLineOfSight;
}

bool CvUnitInfo::isHiddenNationality() const
{
	return m_bHiddenNationality;
}

bool CvUnitInfo::isAlwaysHostile() const
{
	return m_bAlwaysHostile;
}

bool CvUnitInfo::isNoRevealMap() const		
{
	return m_bNoRevealMap;
}

/************************************************************************************************/
/* REVOLUTION_MOD                                 01/01/08                        DPII          */
/*                                                                                              */
/* CoreComponent                                                                                */
/************************************************************************************************/
bool CvUnitInfo::isInquisitor() const
{
	return m_bInquisitor;
}

bool CvUnitInfo::isCanBeRebel() const
{
	return m_bCanBeRebel;
}

bool CvUnitInfo::isCanRebelCapture() const
{
	return m_bCanRebelCapture;
}

bool CvUnitInfo::isCannotDefect() const
{
	return m_bCannotDefect;
}

bool CvUnitInfo::isCanQuellRebellion() const
{
	return m_bCanQuellRebellion;
}
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/

//ls612: Can't enter non-Owned cities
bool CvUnitInfo::isNoNonOwnedEntry() const
{
	return m_bNoNonOwnedEntry;
}

float CvUnitInfo::getUnitMaxSpeed() const			
{
	return m_fUnitMaxSpeed;
}

float CvUnitInfo::getUnitPadTime() const			
{
	return m_fUnitPadTime;
}

int CvUnitInfo::getCommandType() const
{
	return m_iCommandType;
}

void CvUnitInfo::setCommandType(int iNewType)
{
	m_iCommandType = iNewType;
}

BoolExpr* CvUnitInfo::getTrainCondition()
{
	return m_pExprTrainCondition;
}

// BUG - Unit Experience - start
/*
 * Returns true if this unit type is eligible to receive experience points.
 */
bool CvUnitInfo::canAcquireExperience() const
{
	if (m_iUnitCombatType != NO_UNITCOMBAT)
	{
		for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
		{
			if (GC.getPromotionInfo((PromotionTypes)iI).getUnitCombat(m_iUnitCombatType))
			{
				return true;
			}
		}
	}

	return false;
}
// BUG - Unit Experience - end


// Arrays

/********************************************************************************/
/**		REVDCM									2/16/10				phungus420	*/
/**																				*/
/**		CanTrain																*/
/********************************************************************************/
bool CvUnitInfo::isPrereqOrCivics(int i) const	
{
	FAssertMsg(i < GC.getNumCivicInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbPrereqOrCivics ? m_pbPrereqOrCivics[i] : false;
}

bool CvUnitInfo::isPrereqBuildingClass(int i) const
{
	FAssertMsg(i < GC.getNumBuildingClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbPrereqBuildingClass ? m_pbPrereqBuildingClass[i] : false;
}

int CvUnitInfo::getPrereqBuildingClassOverrideTech(int i) const
{
	FAssertMsg(i < GC.getNumBuildingClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piPrereqBuildingClassOverrideTech ? m_piPrereqBuildingClassOverrideTech[i] : -1;
}

int CvUnitInfo::getPrereqBuildingClassOverrideEra(int i) const
{
	FAssertMsg(i < GC.getNumBuildingClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piPrereqBuildingClassOverrideEra ? m_piPrereqBuildingClassOverrideEra[i] : -1;
}

bool CvUnitInfo::getForceObsoleteUnitClass(int i) const
{
	FAssertMsg(i < GC.getNumUnitClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbForceObsoleteUnitClass ? m_pbForceObsoleteUnitClass[i] : false;
}
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

int CvUnitInfo::getPrereqAndTechs(int i) const	
{
	FAssertMsg(i < GC.getNUM_UNIT_AND_TECH_PREREQS(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piPrereqAndTechs ? m_piPrereqAndTechs[i] : -1;
}

int CvUnitInfo::getPrereqOrBonuses(int i) const
{
	FAssertMsg(i < GC.getNUM_UNIT_PREREQ_OR_BONUSES(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piPrereqOrBonuses ? m_piPrereqOrBonuses[i] : -1;
}

int CvUnitInfo::getProductionTraits(int i) const			
{
	FAssertMsg(i < GC.getNumTraitInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piProductionTraits ? m_piProductionTraits[i] : 0;
}

int CvUnitInfo::getFlavorValue(int i) const				
{
	FAssertMsg(i < GC.getNumFlavorTypes(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piFlavorValue ? m_piFlavorValue[i] : 0;
}

int CvUnitInfo::getTerrainAttackModifier(int i) const			
{
	FAssertMsg(i < GC.getNumTerrainInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piTerrainAttackModifier ? m_piTerrainAttackModifier[i] : 0;
}

int CvUnitInfo::getTerrainDefenseModifier(int i) const			
{
	FAssertMsg(i < GC.getNumTerrainInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piTerrainDefenseModifier ? m_piTerrainDefenseModifier[i] : 0;
}

int CvUnitInfo::getFeatureAttackModifier(int i) const			
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piFeatureAttackModifier ? m_piFeatureAttackModifier[i] : 0;
}

int CvUnitInfo::getFeatureDefenseModifier(int i) const			
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piFeatureDefenseModifier ? m_piFeatureDefenseModifier[i] : 0;
}

int CvUnitInfo::getUnitClassAttackModifier(int i) const		
{
	FAssertMsg(i < GC.getNumUnitClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piUnitClassAttackModifier ? m_piUnitClassAttackModifier[i] : 0;
}

int CvUnitInfo::getUnitClassDefenseModifier(int i) const
{
	FAssertMsg(i < GC.getNumUnitClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piUnitClassDefenseModifier ? m_piUnitClassDefenseModifier[i] : 0;
}

int CvUnitInfo::getUnitCombatModifier(int i) const
{
	FAssertMsg(i < GC.getNumUnitCombatInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piUnitCombatModifier ? m_piUnitCombatModifier[i] : 0;
}

int CvUnitInfo::getUnitCombatCollateralImmune(int i) const
{
	FAssertMsg(i < GC.getNumUnitCombatInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piUnitCombatCollateralImmune ? m_piUnitCombatCollateralImmune[i] : 0;
}

int CvUnitInfo::getDomainModifier(int i) const
{
	FAssertMsg(i < NUM_DOMAIN_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piDomainModifier ? m_piDomainModifier[i] : 0;
}

int CvUnitInfo::getBonusProductionModifier(int i) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piBonusProductionModifier ? m_piBonusProductionModifier[i] : 0;
}

int CvUnitInfo::getUnitGroupRequired(int i) const		
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piUnitGroupRequired ? m_piUnitGroupRequired[i] : NULL;
}

bool CvUnitInfo::getUpgradeUnitClass(int i) const
{
	FAssertMsg(i < GC.getNumUnitClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbUpgradeUnitClass ? m_pbUpgradeUnitClass[i] : false;
}

bool CvUnitInfo::getTargetUnitClass(int i) const
{
	FAssertMsg(i < GC.getNumUnitClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbTargetUnitClass ? m_pbTargetUnitClass[i] : false;
}

bool CvUnitInfo::getTargetUnitCombat(int i) const
{
	FAssertMsg(i < GC.getNumUnitCombatInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbTargetUnitCombat ? m_pbTargetUnitCombat[i] : false;
}

bool CvUnitInfo::getDefenderUnitClass(int i) const
{
	FAssertMsg(i < GC.getNumUnitClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbDefenderUnitClass ? m_pbDefenderUnitClass[i] : false;
}

bool CvUnitInfo::getDefenderUnitCombat(int i) const
{
	FAssertMsg(i < GC.getNumUnitCombatInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbDefenderUnitCombat ? m_pbDefenderUnitCombat[i] : false;
}

int CvUnitInfo::getFlankingStrikeUnitClass(int i) const
{
	FAssertMsg(i < GC.getNumUnitClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piFlankingStrikeUnitClass ? m_piFlankingStrikeUnitClass[i] : -1;
}

bool CvUnitInfo::getUnitAIType(int i) const			
{
	FAssertMsg(i < NUM_UNITAI_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbUnitAIType ? m_pbUnitAIType[i] : false;
}

bool CvUnitInfo::getNotUnitAIType(int i) const			
{
	FAssertMsg(i < NUM_UNITAI_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbNotUnitAIType ? m_pbNotUnitAIType[i] : false;
}

bool CvUnitInfo::getBuilds(int i) const									
{
	FAssertMsg(i < GC.getNumBuildInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbBuilds ? m_pbBuilds[i] : false;
}

int CvUnitInfo::getReligionSpreads(int i) const
{
	FAssertMsg(i < GC.getNumReligionInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piReligionSpreads ? m_piReligionSpreads[i] : -1;
}

int CvUnitInfo::getCorporationSpreads(int i) const
{
	FAssertMsg(i < GC.getNumCorporationInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piCorporationSpreads ? m_piCorporationSpreads[i] : -1;
}

int CvUnitInfo::getTerrainPassableTech(int i) const
{
	FAssertMsg(i < GC.getNumTerrainInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piTerrainPassableTech ? m_piTerrainPassableTech[i] : -1;
}

int CvUnitInfo::getFeaturePassableTech(int i) const
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piFeaturePassableTech ? m_piFeaturePassableTech[i] : -1;
}

bool CvUnitInfo::getGreatPeoples(int i) const
{
	FAssertMsg(i < GC.getNumSpecialistInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbGreatPeoples ? m_pbGreatPeoples[i] : false;
}

bool CvUnitInfo::getHasBuildings(void) const
{
	return (m_pbBuildings != NULL);
}

bool CvUnitInfo::getBuildings(int i) const
{
	FAssertMsg(i < GC.getNumBuildingInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbBuildings ? m_pbBuildings[i] : false;
}

bool CvUnitInfo::getForceBuildings(int i) const
{
	FAssertMsg(i < GC.getNumBuildingInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbForceBuildings ? m_pbForceBuildings[i] : false;
}

bool CvUnitInfo::getTerrainImpassable(int i) const		
{
	FAssertMsg(i < GC.getNumTerrainInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbTerrainImpassable ? m_pbTerrainImpassable[i] : false;
}

bool CvUnitInfo::getFeatureImpassable(int i) const		
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbFeatureImpassable ? m_pbFeatureImpassable[i] : false;
}

bool CvUnitInfo::getTerrainNative(int i) const
{
	FAssertMsg(i < GC.getNumTerrainInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbTerrainNative ? m_pbTerrainNative[i] : false;
}

bool CvUnitInfo::getFeatureNative(int i) const
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbFeatureNative ? m_pbFeatureNative[i] : false;
}

bool CvUnitInfo::getFreePromotions(int i) const
{
	FAssertMsg(i < GC.getNumPromotionInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbFreePromotions ? m_pbFreePromotions[i] : false;
}

int CvUnitInfo::getLeaderPromotion() const
{
	return m_iLeaderPromotion;
}

int CvUnitInfo::getLeaderExperience() const
{
	return m_iLeaderExperience;
}

CvOutcomeList* CvUnitInfo::getKillOutcomeList()
{
	return &m_KillOutcomeList;
}

int CvUnitInfo::getNumActionOutcomes()
{
	return m_aOutcomeMissions.size();
}

MissionTypes CvUnitInfo::getActionOutcomeMission(int index)
{
	return m_aOutcomeMissions[index]->getMission();
}

CvOutcomeList* CvUnitInfo::getActionOutcomeList(int index)
{
	return m_aOutcomeMissions[index]->getOutcomeList();
}

CvOutcomeList* CvUnitInfo::getActionOutcomeListByMission(MissionTypes eMission)
{
	for (int i = 0; i < (int) m_aOutcomeMissions.size(); i++)
	{
		if (m_aOutcomeMissions[i]->getMission() == eMission)
		{
			return m_aOutcomeMissions[i]->getOutcomeList();
		}
	}
	return NULL;
}

CvOutcomeMission* CvUnitInfo::getOutcomeMission(int index)
{
	return m_aOutcomeMissions[index];
}

CvOutcomeMission* CvUnitInfo::getOutcomeMissionByMission(MissionTypes eMission)
{
	for (int i = 0; i < (int) m_aOutcomeMissions.size(); i++)
	{
		if (m_aOutcomeMissions[i]->getMission() == eMission)
		{
			return m_aOutcomeMissions[i];
		}
	}
	return NULL;
}

const TCHAR* CvUnitInfo::getEarlyArtDefineTag(int i, UnitArtStyleTypes eStyle) const
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");

	if (NO_UNIT_ARTSTYLE != eStyle)
	{
		int iIndex = GC.getInfoTypeForString(getType());
		if (-1 != iIndex)
		{
			const TCHAR* pcTag = GC.getUnitArtStyleTypeInfo(eStyle).getEarlyArtDefineTag(i, iIndex);
			if (NULL != pcTag)
			{
				return pcTag;
			}
		}
	}

	return (m_paszEarlyArtDefineTags) ? m_paszEarlyArtDefineTags[i] : NULL;
}

void CvUnitInfo::setEarlyArtDefineTag(int i, const TCHAR* szVal)
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	m_paszEarlyArtDefineTags[i] = szVal;
}

const TCHAR* CvUnitInfo::getLateArtDefineTag(int i, UnitArtStyleTypes eStyle) const
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");

	if (NO_UNIT_ARTSTYLE != eStyle)
	{
		int iIndex = GC.getInfoTypeForString(getType());
		if (-1 != iIndex)
		{
			const TCHAR* pcTag = GC.getUnitArtStyleTypeInfo(eStyle).getLateArtDefineTag(i, iIndex);
			if (NULL != pcTag)
			{
				return pcTag;
			}
		}

	}

	return (m_paszLateArtDefineTags) ? m_paszLateArtDefineTags[i] : NULL;
}

void CvUnitInfo::setLateArtDefineTag(int i, const TCHAR* szVal)
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	m_paszLateArtDefineTags[i] = szVal;
}

const TCHAR* CvUnitInfo::getMiddleArtDefineTag(int i, UnitArtStyleTypes eStyle) const
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");

	if (NO_UNIT_ARTSTYLE != eStyle)
	{
		int iIndex = GC.getInfoTypeForString(getType());
		if (-1 != iIndex)
		{
			const TCHAR* pcTag = GC.getUnitArtStyleTypeInfo(eStyle).getMiddleArtDefineTag(i, iIndex);
			if (NULL != pcTag)
			{
				return pcTag;
			}
		}

	}

	return (m_paszMiddleArtDefineTags) ? m_paszMiddleArtDefineTags[i] : NULL;
}

void CvUnitInfo::setMiddleArtDefineTag(int i, const TCHAR* szVal)
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	m_paszMiddleArtDefineTags[i] = szVal;
}

const TCHAR* CvUnitInfo::getUnitNames(int i) const
{
	FAssertMsg(i < getNumUnitNames(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return (m_paszUnitNames) ? m_paszUnitNames[i] : NULL;
}

const TCHAR* CvUnitInfo::getFormationType() const
{
	return m_szFormationType;
}

const TCHAR* CvUnitInfo::getButton() const
{
	return m_szArtDefineButton;
}

void CvUnitInfo::updateArtDefineButton()
{
	m_szArtDefineButton = getArtInfo(0, NO_ERA, NO_UNIT_ARTSTYLE)->getButton();
}

/************************************************************************************************/
/* Afforess	                  Start		 Last Update: 3/10/10                                   */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
const TCHAR* CvUnitInfo::getClassicalArtDefineTag(int i, UnitArtStyleTypes eStyle) const
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");

	if (NO_UNIT_ARTSTYLE != eStyle)
	{
		int iIndex = GC.getInfoTypeForString(getType());
		if (-1 != iIndex)
		{
			const TCHAR* pcTag = GC.getUnitArtStyleTypeInfo(eStyle).getClassicalArtDefineTag(i, iIndex);
			if (NULL != pcTag)
			{
				return pcTag;
			}
		}

	}

	return (m_paszClassicalArtDefineTags) ? m_paszClassicalArtDefineTags[i] : NULL;
}

void CvUnitInfo::setClassicalArtDefineTag(int i, const TCHAR* szVal)
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	m_paszClassicalArtDefineTags[i] = szVal;
}

const TCHAR* CvUnitInfo::getRennArtDefineTag(int i, UnitArtStyleTypes eStyle) const
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");

	if (NO_UNIT_ARTSTYLE != eStyle)
	{
		int iIndex = GC.getInfoTypeForString(getType());
		if (-1 != iIndex)
		{
			const TCHAR* pcTag = GC.getUnitArtStyleTypeInfo(eStyle).getRennArtDefineTag(i, iIndex);
			if (NULL != pcTag)
			{
				return pcTag;
			}
		}

	}

	return (m_paszRennArtDefineTags) ? m_paszRennArtDefineTags[i] : NULL;
}

void CvUnitInfo::setRennArtDefineTag(int i, const TCHAR* szVal)
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	m_paszRennArtDefineTags[i] = szVal;
}

const TCHAR* CvUnitInfo::getIndustrialArtDefineTag(int i, UnitArtStyleTypes eStyle) const
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");

	if (NO_UNIT_ARTSTYLE != eStyle)
	{
		int iIndex = GC.getInfoTypeForString(getType());
		if (-1 != iIndex)
		{
			const TCHAR* pcTag = GC.getUnitArtStyleTypeInfo(eStyle).getIndustrialArtDefineTag(i, iIndex);
			if (NULL != pcTag)
			{
				return pcTag;
			}
		}

	}

	return (m_paszIndustrialArtDefineTags) ? m_paszIndustrialArtDefineTags[i] : NULL;
}

void CvUnitInfo::setIndustrialArtDefineTag(int i, const TCHAR* szVal)
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	m_paszIndustrialArtDefineTags[i] = szVal;
}

const TCHAR* CvUnitInfo::getFutureArtDefineTag(int i, UnitArtStyleTypes eStyle) const
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");

	if (NO_UNIT_ARTSTYLE != eStyle)
	{
		int iIndex = GC.getInfoTypeForString(getType());
		if (-1 != iIndex)
		{
			const TCHAR* pcTag = GC.getUnitArtStyleTypeInfo(eStyle).getFutureArtDefineTag(i, iIndex);
			if (NULL != pcTag)
			{
				return pcTag;
			}
		}

	}

	return (m_paszFutureArtDefineTags) ? m_paszFutureArtDefineTags[i] : NULL;
}

void CvUnitInfo::setFutureArtDefineTag(int i, const TCHAR* szVal)
{
	FAssertMsg(i < getGroupDefinitions(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	m_paszFutureArtDefineTags[i] = szVal;
}

const CvArtInfoUnit* CvUnitInfo::getArtInfo(int i, EraTypes eEra, UnitArtStyleTypes eStyle) const
{
	if ((eEra > 5) && (getFutureArtDefineTag(i, eStyle) != NULL) && !CvString(getFutureArtDefineTag(i, eStyle)).empty())
	{
		return ARTFILEMGR.getUnitArtInfo(getFutureArtDefineTag(i, eStyle));
	}
	else if ((eEra > 4) && (getLateArtDefineTag(i, eStyle) != NULL) && !CvString(getLateArtDefineTag(i, eStyle)).empty())
	{
		return ARTFILEMGR.getUnitArtInfo(getLateArtDefineTag(i, eStyle));
	}
	else if ((eEra > 3) && (getIndustrialArtDefineTag(i, eStyle) != NULL) && !CvString(getIndustrialArtDefineTag(i, eStyle)).empty())
	{
		return ARTFILEMGR.getUnitArtInfo(getIndustrialArtDefineTag(i, eStyle));
	}
	else if ((eEra > 2) && (getRennArtDefineTag(i, eStyle) != NULL) && !CvString(getRennArtDefineTag(i, eStyle)).empty())
	{
		return ARTFILEMGR.getUnitArtInfo(getRennArtDefineTag(i, eStyle));
	}
	else if ((eEra > 1) && (getMiddleArtDefineTag(i, eStyle) != NULL) && !CvString(getMiddleArtDefineTag(i, eStyle)).empty())
	{
		return ARTFILEMGR.getUnitArtInfo(getMiddleArtDefineTag(i, eStyle));
	}
	else if ((eEra > 0) && (getClassicalArtDefineTag(i, eStyle) != NULL) && !CvString(getClassicalArtDefineTag(i, eStyle)).empty())
	{
		return ARTFILEMGR.getUnitArtInfo(getClassicalArtDefineTag(i, eStyle));
	}
	else
	{
		return ARTFILEMGR.getUnitArtInfo(getEarlyArtDefineTag(i, eStyle));
	}
}

void CvUnitInfo::setPowerValue(int iNewValue)
{
	m_iPowerValue = iNewValue;
}

int CvUnitInfo::getPrereqVicinityBonus() const			
{
	return m_iPrereqVicinityBonus;
}

int CvUnitInfo::getBaseFoodChange() const
{
	return m_iBaseFoodChange;
}

bool CvUnitInfo::isWorkerTrade() const
{
    return m_bWorkerTrade;
}

bool CvUnitInfo::isMilitaryTrade() const
{
	return m_bMilitaryTrade;
}

bool CvUnitInfo::isForceUpgrade() const
{
	return m_bForceUpgrade;
}

bool CvUnitInfo::isGreatGeneral() const
{
	return m_bGreatGeneral;
}

bool CvUnitInfo::isSlave() const
{
	return m_bSlave;
}

int CvUnitInfo::getControlPoints() const
{
	return m_iControlPoints;
}

int CvUnitInfo::getCommandRange() const
{
	return m_iCommandRange;
}

bool CvUnitInfo::isRequiresStateReligionInCity() const		
{
	return m_bRequiresStateReligionInCity;
}

bool CvUnitInfo::getPassableRouteNeeded(int i) const		
{
	FAssertMsg(i < GC.getNumRouteInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbPassableRouteNeeded ? m_pbPassableRouteNeeded[i] : false;
}

int CvUnitInfo::getPrereqOrVicinityBonuses(int i) const
{
	FAssertMsg(i < GC.getNUM_UNIT_PREREQ_OR_BONUSES(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piPrereqOrVicinityBonuses ? m_piPrereqOrVicinityBonuses[i] : -1;
}

CvWString CvUnitInfo::getCivilizationName(int i) const
{
	FAssertMsg(i < GC.getNumCivilizationInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paszCivilizationNames[i];
}

int CvUnitInfo::getCivilizationNamesVectorSize()					{return m_aszCivilizationNamesforPass3.size();}
CvWString CvUnitInfo::getCivilizationNamesNamesVectorElement(int i)	{return m_aszCivilizationNamesforPass3[i];}
CvWString CvUnitInfo::getCivilizationNamesValuesVectorElement(int i)		{return m_aszCivilizationNamesValueforPass3[i];}

// Sanguo Mod Performance start, added by poyuzhe 07.27.09
std::vector<int> CvUnitInfo::getUpgradeUnitClassTypes() const
{
	return m_aiUpgradeUnitClassTypes;
}

void CvUnitInfo::addUpgradeUnitClassTypes(int i)
{
	FAssert (i > -1 && i < GC.getNumUnitClassInfos());
	if (find(m_aiUpgradeUnitClassTypes.begin(), m_aiUpgradeUnitClassTypes.end(), i) == m_aiUpgradeUnitClassTypes.end())
	{
		m_aiUpgradeUnitClassTypes.push_back(i);
	}
}

bool CvUnitInfo::isUpgradeUnitClassTypes(int i)
{
	FAssert (i > -1 && i < GC.getNumUnitClassInfos());
	if (find(m_aiUpgradeUnitClassTypes.begin(), m_aiUpgradeUnitClassTypes.end(), i) == m_aiUpgradeUnitClassTypes.end())
	{
		return false;
	}
	return true;
}
// Sanguo Mod Performance, end
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
int CvUnitInfo::getPursuit() const			
{
	return m_iPursuit;
}

void CvUnitInfo::read(FDataStreamBase* stream)
{
	CvHotkeyInfo::read(stream);

	uint uiFlag=0;
	stream->Read(&uiFlag);	// flags for expansion

	stream->Read(&m_iVolleyRange);
	stream->Read(&m_iVolleyAccuracy);
	stream->Read(&m_iVolleyAccuracyMin);
	stream->Read(&m_iVolleyEfficiency);
	stream->Read(&m_iVolleyRounds);
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
	// Dale - AB: Bombing START
	stream->Read(&m_bDCMAirBomb1);
	stream->Read(&m_bDCMAirBomb2);
	stream->Read(&m_bDCMAirBomb3);
	stream->Read(&m_bDCMAirBomb4);
	stream->Read(&m_bDCMAirBomb5);
	// Dale - AB: Bombing END
	// Dale - FE: Fighters START
	stream->Read(&m_bDCMFighterEngage);
	// Dale - FE: Fighters END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
	stream->Read(&m_iAIWeight);
	stream->Read(&m_iProductionCost);
	stream->Read(&m_iHurryCostModifier);
	stream->Read(&m_iAdvancedStartCost);
	stream->Read(&m_iAdvancedStartCostIncrease);
	stream->Read(&m_iMinAreaSize);
	stream->Read(&m_iMoves);
	stream->Read(&m_iAirRange);
	stream->Read(&m_iAirUnitCap);
	stream->Read(&m_iDropRange);
	stream->Read(&m_iNukeRange);
	stream->Read(&m_iWorkRate);
	stream->Read(&m_iBaseDiscover);
	stream->Read(&m_iDiscoverMultiplier);
	stream->Read(&m_iBaseHurry);
	stream->Read(&m_iHurryMultiplier);
	stream->Read(&m_iBaseTrade);
	stream->Read(&m_iTradeMultiplier);
	stream->Read(&m_iGreatWorkCulture);
	stream->Read(&m_iEspionagePoints);
	stream->Read(&m_iCombat);
	stream->Read(&m_iCombatLimit);
	stream->Read(&m_iAirCombat);
	stream->Read(&m_iAirCombatLimit);
	stream->Read(&m_iXPValueAttack);
	stream->Read(&m_iXPValueDefense);
	stream->Read(&m_iFirstStrikes);
	stream->Read(&m_iChanceFirstStrikes);
	stream->Read(&m_iInterceptionProbability);
	//ls612: Advanced Nuke Interception
	//stream->Read(&m_iNukeInterceptionProbability);
	//stream->Read(&m_iNukeInterceptionRange);
	stream->Read(&m_iEvasionProbability);
	stream->Read(&m_iWithdrawalProbability);
	stream->Read(&m_iCollateralDamage);
	stream->Read(&m_iCollateralDamageLimit);
	stream->Read(&m_iCollateralDamageMaxUnits);
	stream->Read(&m_iCityAttackModifier);
	stream->Read(&m_iCityDefenseModifier);
	stream->Read(&m_iAnimalCombatModifier);
	stream->Read(&m_iHillsAttackModifier);
	stream->Read(&m_iHillsDefenseModifier);
	stream->Read(&m_iBombRate);
	stream->Read(&m_iBombardRate);
// Thomas SG - AC: Advanced Cargo START
//	stream->Read(&m_iSpecialCargo);
// Thomas SG - AC: Advanced Cargo END
	stream->Read(&m_iDomainCargo);
	stream->Read(&m_iCargoSpace);
// Thomas SG - AC: Advanced Cargo START
	stream->Read(&m_iTotalCargoSpace);
// Thomas SG - AC: Advanced Cargo END	
	stream->Read(&m_iConscriptionValue);
	stream->Read(&m_iCultureGarrisonValue);
	stream->Read(&m_iExtraCost);
	stream->Read(&m_iAssetValue);
	stream->Read(&m_iPowerValue);
	stream->Read(&m_iUnitClassType);
// Thomas SG - AC: Advanced Cargo START
//	stream->Read(&m_iSpecialUnitType);

	int iNumSpecialUnitTypes = 0;
	stream->Read(&iNumSpecialUnitTypes);
	for(int i=0;i<iNumSpecialUnitTypes;i++)
	{
		int iSpecialUnitType = 0;
		stream->Read(&iSpecialUnitType);
		m_aiSpecialUnitTypes.push_back(iSpecialUnitType);
	}
// Thomas SG - AC: Advanced Cargo END
	stream->Read(&m_iUnitCaptureClassType);
	stream->Read(&m_iUnitCombatType);
	stream->Read(&m_iDomainType);
	stream->Read(&m_iDefaultUnitAIType);
	stream->Read(&m_iInvisibleType);
	
	int iNumInvisibleTypes = 0;
	stream->Read(&iNumInvisibleTypes);
	for(int i=0;i<iNumInvisibleTypes;i++)
	{
		int iSeeInvisibleType = 0;
		stream->Read(&iSeeInvisibleType);
		m_aiSeeInvisibleTypes.push_back(iSeeInvisibleType);
	}

	stream->Read(&m_iAdvisorType);

/********************************************************************************/
/**		REVDCM									2/16/10				phungus420	*/
/**																				*/
/**		CanTrain																*/
/********************************************************************************/
	stream->Read(&m_iMaxStartEra);
	stream->Read(&m_iForceObsoleteTech);
	stream->Read(&m_bStateReligion);
	stream->Read(&m_iPrereqGameOption);
	stream->Read(&m_iNotGameOption);
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

	stream->Read(&m_iHolyCity);
	stream->Read(&m_iReligionType);
	stream->Read(&m_iStateReligion);
	stream->Read(&m_iPrereqReligion);
	stream->Read(&m_iPrereqCorporation);
	stream->Read(&m_iPrereqBuilding);
	
	int iNum = 0;
	int iBuilding = 0;
	stream->Read(&iNum);
	for (int i = 0; i < iNum; i++)
	{
		stream->Read(&iBuilding);
		m_aePrereqOrBuildings.push_back((BuildingTypes) i);
	}
	stream->Read(&m_iOverrideResourceBuilding);
	stream->Read(&m_iPrereqAndTech);
	stream->Read(&m_iPrereqAndBonus);
	stream->Read(&m_iGroupSize);
	stream->Read(&m_iGroupDefinitions);
	stream->Read(&m_iUnitMeleeWaveSize);
	stream->Read(&m_iUnitRangedWaveSize);
	stream->Read(&m_iNumUnitNames);
	stream->Read(&m_iCommandType);
// Thomas SG - AC: Advanced Cargo START
	int iNumSpecialCargos = 0;
	stream->Read(&iNumSpecialCargos);
	for(int i=0;i<iNumSpecialCargos;i++)
	{
		int iSpecialCargo = 0;
		stream->Read(&iSpecialCargo);
		m_aiSpecialCargos.push_back(iSpecialCargo);
	}

	int iNumSpecialCargoSpaces = 0;
	stream->Read(&iNumSpecialCargoSpaces);
	for(int i=0;i<iNumSpecialCargoSpaces;i++)
	{
		int iSpecialCargoSpace = 0;
		stream->Read(&iSpecialCargoSpace);
		m_aiSpecialCargoSpaces.push_back(iSpecialCargoSpace);
	}
// Thomas SG - AC: Advanced Cargo END	
	stream->Read(&m_bAnimal);
	stream->Read(&m_bFoodProduction);
	stream->Read(&m_bNoBadGoodies);
	stream->Read(&m_bOnlyDefensive);
	stream->Read(&m_bNoCapture);
	stream->Read(&m_bQuickCombat);
	stream->Read(&m_bRivalTerritory);
	stream->Read(&m_bMilitaryHappiness);
	stream->Read(&m_bMilitarySupport);
	stream->Read(&m_bMilitaryProduction);
	stream->Read(&m_bPillage);
	stream->Read(&m_bSpy);
	stream->Read(&m_bSabotage);
	stream->Read(&m_bDestroy);
	stream->Read(&m_bStealPlans);
/************************************************************************************************/
/* Great Diplomat MOD               START                                      Stolenrays 		*/
/************************************************************************************************/
	stream->Read(&m_bBribeBarbarian);
	stream->Read(&m_bGoodwill);
	stream->Read(&m_bBarbariansColonisator);
	stream->Read(&m_bForcePeace);	
	stream->Read(&m_bUpgradeImprovements);
/************************************************************************************************/
/* Great Diplomat MOD               END                                                  		*/
/************************************************************************************************/
	stream->Read(&m_bInvestigate);
	stream->Read(&m_bCounterSpy);
	stream->Read(&m_bFound);
	stream->Read(&m_bGoldenAge);
	stream->Read(&m_bInvisible);
	stream->Read(&m_bFirstStrikeImmune);
	stream->Read(&m_bNoDefensiveBonus);
	stream->Read(&m_bIgnoreBuildingDefense);
	stream->Read(&m_bCanMoveImpassable);
	stream->Read(&m_bCanMoveAllTerrain);
	stream->Read(&m_bFlatMovementCost);
	stream->Read(&m_bIgnoreTerrainCost);
	stream->Read(&m_bNukeImmune);
	stream->Read(&m_bPrereqBonuses);
	stream->Read(&m_bPrereqReligion);
	stream->Read(&m_bMechanized);
	stream->Read(&m_bRenderBelowWater);
	stream->Read(&m_bRenderAlways);
	stream->Read(&m_bSuicide);
	stream->Read(&m_bLineOfSight);
	stream->Read(&m_bHiddenNationality);
	stream->Read(&m_bAlwaysHostile);
	stream->Read(&m_bNoRevealMap);
/************************************************************************************************/
/* REVOLUTION_MOD                                 01/01/08                        DPII          */
/*                                                                                              */
/* CoreComponent                                                                                */
/************************************************************************************************/
	stream->Read(&m_bInquisitor);
	stream->Read(&m_bCanBeRebel);
	stream->Read(&m_bCanRebelCapture);
	stream->Read(&m_bCannotDefect);
	stream->Read(&m_bCanQuellRebellion);
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/
	//ls612: Can't enter non-Owned Cities
	stream->Read(&m_bNoNonOwnedEntry);

	stream->Read(&m_fUnitMaxSpeed);
	stream->Read(&m_fUnitPadTime);

	bool bPresent = false;
	SAFE_DELETE_ARRAY(m_piPrereqAndTechs);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piPrereqAndTechs = new int[GC.getNUM_UNIT_AND_TECH_PREREQS()];
		stream->Read(GC.getNUM_UNIT_AND_TECH_PREREQS(), m_piPrereqAndTechs);
	}

	SAFE_DELETE_ARRAY(m_piPrereqOrBonuses);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piPrereqOrBonuses = new int[GC.getNUM_UNIT_PREREQ_OR_BONUSES()];
		stream->Read(GC.getNUM_UNIT_PREREQ_OR_BONUSES(), m_piPrereqOrBonuses);
	}

	SAFE_DELETE_ARRAY(m_piProductionTraits);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piProductionTraits = new int[GC.getNumTraitInfos()];
		stream->Read(GC.getNumTraitInfos(), m_piProductionTraits);
	}

	SAFE_DELETE_ARRAY(m_piFlavorValue);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piFlavorValue = new int[GC.getNumFlavorTypes()];
		stream->Read(GC.getNumFlavorTypes(), m_piFlavorValue);
	}

	SAFE_DELETE_ARRAY(m_piTerrainAttackModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piTerrainAttackModifier = new int[GC.getNumTerrainInfos()];
		stream->Read(GC.getNumTerrainInfos(), m_piTerrainAttackModifier);
	}

	SAFE_DELETE_ARRAY(m_piTerrainDefenseModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piTerrainDefenseModifier = new int[GC.getNumTerrainInfos()];
		stream->Read(GC.getNumTerrainInfos(), m_piTerrainDefenseModifier);
	}

	SAFE_DELETE_ARRAY(m_piFeatureAttackModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piFeatureAttackModifier = new int[GC.getNumFeatureInfos()];
		stream->Read(GC.getNumFeatureInfos(), m_piFeatureAttackModifier);
	}

	SAFE_DELETE_ARRAY(m_piFeatureDefenseModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piFeatureDefenseModifier = new int[GC.getNumFeatureInfos()];
		stream->Read(GC.getNumFeatureInfos(), m_piFeatureDefenseModifier);
	}

	SAFE_DELETE_ARRAY(m_piUnitClassAttackModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piUnitClassAttackModifier = new int[GC.getNumUnitClassInfos()];
		stream->Read(GC.getNumUnitClassInfos(), m_piUnitClassAttackModifier);
	}

	SAFE_DELETE_ARRAY(m_piUnitClassDefenseModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piUnitClassDefenseModifier = new int[GC.getNumUnitClassInfos()];
		stream->Read(GC.getNumUnitClassInfos(), m_piUnitClassDefenseModifier);
	}

	SAFE_DELETE_ARRAY(m_piUnitCombatModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piUnitCombatModifier = new int[GC.getNumUnitCombatInfos()];
		stream->Read(GC.getNumUnitCombatInfos(), m_piUnitCombatModifier);
	}

	SAFE_DELETE_ARRAY(m_piUnitCombatCollateralImmune);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piUnitCombatCollateralImmune = new int[GC.getNumUnitCombatInfos()];
		stream->Read(GC.getNumUnitCombatInfos(), m_piUnitCombatCollateralImmune);
	}

	SAFE_DELETE_ARRAY(m_piDomainModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piDomainModifier = new int[NUM_DOMAIN_TYPES];
		stream->Read(NUM_DOMAIN_TYPES, m_piDomainModifier);
	}

	SAFE_DELETE_ARRAY(m_piBonusProductionModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piBonusProductionModifier = new int[GC.getNumBonusInfos()];
		stream->Read(GC.getNumBonusInfos(), m_piBonusProductionModifier);
	}

	SAFE_DELETE_ARRAY(m_piUnitGroupRequired);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piUnitGroupRequired = new int[m_iGroupDefinitions];
		stream->Read(m_iGroupDefinitions, m_piUnitGroupRequired);
	}

/********************************************************************************/
/**		REVDCM									2/16/10				phungus420	*/
/**																				*/
/**		CanTrain																*/
/********************************************************************************/
	SAFE_DELETE_ARRAY(m_pbPrereqOrCivics);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbPrereqOrCivics = new bool[GC.getNumCivicInfos()];
		stream->Read(GC.getNumCivicInfos(), m_pbPrereqOrCivics);
	}

	SAFE_DELETE_ARRAY(m_pbPrereqBuildingClass);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbPrereqBuildingClass = new bool[GC.getNumBuildingClassInfos()];
		stream->Read(GC.getNumBuildingClassInfos(), m_pbPrereqBuildingClass);
	}

	SAFE_DELETE_ARRAY(m_piPrereqBuildingClassOverrideTech);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piPrereqBuildingClassOverrideTech = new int[GC.getNumBuildingClassInfos()];
		stream->Read(GC.getNumBuildingClassInfos(), m_piPrereqBuildingClassOverrideTech);
	}

	SAFE_DELETE_ARRAY(m_piPrereqBuildingClassOverrideEra);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piPrereqBuildingClassOverrideEra = new int[GC.getNumBuildingClassInfos()];
		stream->Read(GC.getNumBuildingClassInfos(), m_piPrereqBuildingClassOverrideEra);
	}

	SAFE_DELETE_ARRAY(m_pbForceObsoleteUnitClass);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbForceObsoleteUnitClass = new bool[GC.getNumUnitClassInfos()];
		stream->Read(GC.getNumUnitClassInfos(), m_pbForceObsoleteUnitClass);
	}
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

	SAFE_DELETE_ARRAY(m_pbUpgradeUnitClass);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbUpgradeUnitClass = new bool[GC.getNumUnitClassInfos()];
		stream->Read(GC.getNumUnitClassInfos(), m_pbUpgradeUnitClass);
	}

	SAFE_DELETE_ARRAY(m_pbTargetUnitClass);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbTargetUnitClass = new bool[GC.getNumUnitClassInfos()];
		stream->Read(GC.getNumUnitClassInfos(), m_pbTargetUnitClass);
	}

	SAFE_DELETE_ARRAY(m_pbTargetUnitCombat);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbTargetUnitCombat = new bool[GC.getNumUnitCombatInfos()];
		stream->Read(GC.getNumUnitCombatInfos(), m_pbTargetUnitCombat);
	}

	SAFE_DELETE_ARRAY(m_pbDefenderUnitClass);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbDefenderUnitClass = new bool[GC.getNumUnitClassInfos()];
		stream->Read(GC.getNumUnitClassInfos(), m_pbDefenderUnitClass);
	}

	SAFE_DELETE_ARRAY(m_pbDefenderUnitCombat);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbDefenderUnitCombat = new bool[GC.getNumUnitCombatInfos()];
		stream->Read(GC.getNumUnitCombatInfos(), m_pbDefenderUnitCombat);
	}

	SAFE_DELETE_ARRAY(m_piFlankingStrikeUnitClass);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piFlankingStrikeUnitClass = new int[GC.getNumUnitClassInfos()];
		stream->Read(GC.getNumUnitClassInfos(), m_piFlankingStrikeUnitClass);
	}

	SAFE_DELETE_ARRAY(m_pbUnitAIType);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbUnitAIType = new bool[NUM_UNITAI_TYPES];
		stream->Read(NUM_UNITAI_TYPES, m_pbUnitAIType);
	}

	SAFE_DELETE_ARRAY(m_pbNotUnitAIType);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbNotUnitAIType = new bool[NUM_UNITAI_TYPES];
		stream->Read(NUM_UNITAI_TYPES, m_pbNotUnitAIType);
	}

	SAFE_DELETE_ARRAY(m_pbBuilds);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbBuilds = new bool[GC.getNumBuildInfos()];
		stream->Read(GC.getNumBuildInfos(), m_pbBuilds);
	}

	SAFE_DELETE_ARRAY(m_piReligionSpreads);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piReligionSpreads = new int[GC.getNumReligionInfos()];
		stream->Read(GC.getNumReligionInfos(), m_piReligionSpreads);
	}

	SAFE_DELETE_ARRAY(m_piCorporationSpreads);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piCorporationSpreads = new int[GC.getNumCorporationInfos()];
		stream->Read(GC.getNumCorporationInfos(), m_piCorporationSpreads);
	}

	SAFE_DELETE_ARRAY(m_piTerrainPassableTech);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piTerrainPassableTech = new int[GC.getNumTerrainInfos()];
		stream->Read(GC.getNumTerrainInfos(), m_piTerrainPassableTech);
	}

	SAFE_DELETE_ARRAY(m_piFeaturePassableTech);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piFeaturePassableTech = new int[GC.getNumFeatureInfos()];
		stream->Read(GC.getNumFeatureInfos(), m_piFeaturePassableTech);
	}

	SAFE_DELETE_ARRAY(m_pbGreatPeoples);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbGreatPeoples = new bool[GC.getNumSpecialistInfos()];
		stream->Read(GC.getNumSpecialistInfos(), m_pbGreatPeoples);
	}

	SAFE_DELETE_ARRAY(m_pbBuildings);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbBuildings = new bool[GC.getNumBuildingInfos()];
		stream->Read(GC.getNumBuildingInfos(), m_pbBuildings);
	}

	SAFE_DELETE_ARRAY(m_pbForceBuildings);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbForceBuildings = new bool[GC.getNumBuildingInfos()];
		stream->Read(GC.getNumBuildingInfos(), m_pbForceBuildings);
	}

	SAFE_DELETE_ARRAY(m_pbTerrainNative);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbTerrainNative = new bool[GC.getNumTerrainInfos()];
		stream->Read(GC.getNumTerrainInfos(), m_pbTerrainNative);
	}

	SAFE_DELETE_ARRAY(m_pbFeatureNative);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbFeatureNative = new bool[GC.getNumFeatureInfos()];
		stream->Read(GC.getNumFeatureInfos(), m_pbFeatureNative);
	}

	SAFE_DELETE_ARRAY(m_pbTerrainImpassable);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbTerrainImpassable = new bool[GC.getNumTerrainInfos()];
		stream->Read(GC.getNumTerrainInfos(), m_pbTerrainImpassable);
	}

	SAFE_DELETE_ARRAY(m_pbFeatureImpassable);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbFeatureImpassable = new bool[GC.getNumFeatureInfos()];
		stream->Read(GC.getNumFeatureInfos(), m_pbFeatureImpassable);
	}

	SAFE_DELETE_ARRAY(m_pbFreePromotions);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbFreePromotions = new bool[GC.getNumPromotionInfos()];
		stream->Read(GC.getNumPromotionInfos(), m_pbFreePromotions);
	}

	stream->Read(&m_iLeaderPromotion);
	stream->Read(&m_iLeaderExperience);

	SAFE_DELETE_ARRAY(m_paszEarlyArtDefineTags);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_paszEarlyArtDefineTags = new CvString [m_iGroupDefinitions];
		stream->ReadString(m_iGroupDefinitions, m_paszEarlyArtDefineTags);
	}

	SAFE_DELETE_ARRAY(m_paszLateArtDefineTags);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_paszLateArtDefineTags = new CvString [m_iGroupDefinitions];
		stream->ReadString(m_iGroupDefinitions, m_paszLateArtDefineTags);
	}

	SAFE_DELETE_ARRAY(m_paszMiddleArtDefineTags);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_paszMiddleArtDefineTags = new CvString [m_iGroupDefinitions];
		stream->ReadString(m_iGroupDefinitions, m_paszMiddleArtDefineTags);
	}

	SAFE_DELETE_ARRAY(m_paszUnitNames);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_paszUnitNames = new CvString[m_iNumUnitNames];
		stream->ReadString(m_iNumUnitNames, m_paszUnitNames);
	}

	stream->ReadString(m_szFormationType);

/************************************************************************************************/
/* Afforess	                  Start		 Last Update: 3/8/10                                    */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	stream->Read(&m_bRequiresStateReligionInCity);
	stream->Read(&m_bWorkerTrade);
	stream->Read(&m_bMilitaryTrade);
	stream->Read(&m_bForceUpgrade);
	stream->Read(&m_bGreatGeneral);
	stream->Read(&m_bSlave);
	stream->Read(&m_iPrereqVicinityBonus);
	stream->Read(&m_iBaseFoodChange);
	stream->Read(&m_iControlPoints);
	stream->Read(&m_iCommandRange);

	SAFE_DELETE_ARRAY(m_piPrereqOrVicinityBonuses);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piPrereqOrVicinityBonuses = new int[GC.getNUM_UNIT_PREREQ_OR_BONUSES()];
		stream->Read(GC.getNUM_UNIT_PREREQ_OR_BONUSES(), m_piPrereqOrVicinityBonuses);
	}

	SAFE_DELETE_ARRAY(m_pbPrereqBuildingClass);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbPrereqBuildingClass = new bool[GC.getNumBuildingClassInfos()];
		stream->Read(GC.getNumBuildingClassInfos(), m_pbPrereqBuildingClass);
	}

	SAFE_DELETE_ARRAY(m_pbPassableRouteNeeded);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbPassableRouteNeeded = new bool[GC.getNumRouteInfos()];
		stream->Read(GC.getNumRouteInfos(), m_pbPassableRouteNeeded);
	}

	SAFE_DELETE_ARRAY(m_paszClassicalArtDefineTags);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_paszClassicalArtDefineTags = new CvString [m_iGroupDefinitions];
		stream->ReadString(m_iGroupDefinitions, m_paszClassicalArtDefineTags);
	}

	SAFE_DELETE_ARRAY(m_paszRennArtDefineTags);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_paszRennArtDefineTags = new CvString [m_iGroupDefinitions];
		stream->ReadString(m_iGroupDefinitions, m_paszRennArtDefineTags);
	}

	SAFE_DELETE_ARRAY(m_paszIndustrialArtDefineTags);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_paszIndustrialArtDefineTags = new CvString [m_iGroupDefinitions];
		stream->ReadString(m_iGroupDefinitions, m_paszIndustrialArtDefineTags);
	}

	SAFE_DELETE_ARRAY(m_paszFutureArtDefineTags);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_paszFutureArtDefineTags = new CvString [m_iGroupDefinitions];
		stream->ReadString(m_iGroupDefinitions, m_paszFutureArtDefineTags);
	}
	
	int iNumCivs = 0;
	SAFE_DELETE_ARRAY(m_paszCivilizationNames);
	stream->Read(&iNumCivs);
	if (iNumCivs)
	{
		m_paszCivilizationNames = new CvWString [iNumCivs];
		stream->ReadString(iNumCivs, m_paszCivilizationNames);
	}

	int iNumUpgradeUnitClassTypes = 0;
	stream->Read(&iNumUpgradeUnitClassTypes);
	for(int i=0; i<iNumUpgradeUnitClassTypes;i++)
	{
		int iUnitClassType = 0;
		stream->Read(&iUnitClassType);
		m_aiUpgradeUnitClassTypes.push_back(iUnitClassType);
	}

	getKillOutcomeList()->read(stream);

	int iNumOutcomeMissions = 0;
	stream->Read(&iNumOutcomeMissions);
	for(int i=0; i<iNumOutcomeMissions; i++)
	{
		m_aOutcomeMissions.push_back(new CvOutcomeMission());
		m_aOutcomeMissions[i]->read(stream);
	}

	m_PropertyManipulators.read(stream);

	bool bExpr = false;
	stream->Read(&bExpr);
	if (bExpr)
	{
		m_pExprTrainCondition = BoolExpr::readExpression(stream);
	}

	updateArtDefineButton();

	stream->ReadString(m_szExtraHoverTextKey);
}
//boolean vectors without delayed resolution
int CvUnitInfo::getSubCombatType(int i) const
{
	return m_aiSubCombatTypes[i];
}

int CvUnitInfo::getNumSubCombatTypes() const
{
	return (int)m_aiSubCombatTypes.size();
}

bool CvUnitInfo::isSubCombatType(int i)
{
	FAssert (i > -1 && i < GC.getNumUnitCombatInfos());
	if (find(m_aiSubCombatTypes.begin(), m_aiSubCombatTypes.end(), i) == m_aiSubCombatTypes.end())
	{
		return false;
	}
	return true;
}

void CvUnitInfo::write(FDataStreamBase* stream)
{
	CvHotkeyInfo::write(stream);

	uint uiFlag=0;
	stream->Write(uiFlag); // flag for expansion

	stream->Write(m_iVolleyRange);
	stream->Write(m_iVolleyAccuracy);
	stream->Write(m_iVolleyAccuracyMin);
	stream->Write(m_iVolleyEfficiency);
	stream->Write(m_iVolleyRounds);
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
	// Dale - AB: Bombing START
	stream->Write(m_bDCMAirBomb1);
	stream->Write(m_bDCMAirBomb2);
	stream->Write(m_bDCMAirBomb3);
	stream->Write(m_bDCMAirBomb4);
	stream->Write(m_bDCMAirBomb5);
	// Dale - AB: Bombing END
	// Dale - FE: Fighters START
	stream->Write(m_bDCMFighterEngage);
	// Dale - FE: Fighters END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
	stream->Write(m_iAIWeight);
	stream->Write(m_iProductionCost);
	stream->Write(m_iHurryCostModifier);
	stream->Write(m_iAdvancedStartCost);
	stream->Write(m_iAdvancedStartCostIncrease);
	stream->Write(m_iMinAreaSize);
	stream->Write(m_iMoves);
	stream->Write(m_iAirRange);
	stream->Write(m_iAirUnitCap);
	stream->Write(m_iDropRange);
	stream->Write(m_iNukeRange);
	stream->Write(m_iWorkRate);
	stream->Write(m_iBaseDiscover);
	stream->Write(m_iDiscoverMultiplier);
	stream->Write(m_iBaseHurry);
	stream->Write(m_iHurryMultiplier);
	stream->Write(m_iBaseTrade);
	stream->Write(m_iTradeMultiplier);
	stream->Write(m_iGreatWorkCulture);
	stream->Write(m_iEspionagePoints);
	stream->Write(m_iCombat);
	stream->Write(m_iCombatLimit);
	stream->Write(m_iAirCombat);
	stream->Write(m_iAirCombatLimit);
	stream->Write(m_iXPValueAttack);
	stream->Write(m_iXPValueDefense);
	stream->Write(m_iFirstStrikes);
	stream->Write(m_iChanceFirstStrikes);
	stream->Write(m_iInterceptionProbability);
	//ls612: Advanced Nuke Intercept
	//stream->Write(m_iNukeInterceptionProbability);
	//stream->Write(m_iNukeInterceptionRange);
	stream->Write(m_iEvasionProbability);
	stream->Write(m_iWithdrawalProbability);
	stream->Write(m_iCollateralDamage);
	stream->Write(m_iCollateralDamageLimit);
	stream->Write(m_iCollateralDamageMaxUnits);
	stream->Write(m_iCityAttackModifier);
	stream->Write(m_iCityDefenseModifier);
	stream->Write(m_iAnimalCombatModifier);
	stream->Write(m_iHillsAttackModifier);
	stream->Write(m_iHillsDefenseModifier);
	stream->Write(m_iBombRate);
	stream->Write(m_iBombardRate);
// Thomas SG - AC: Advanced Cargo START
//	stream->Write(m_iSpecialCargo);
// Thomas SG - AC: Advanced Cargo END
	stream->Write(m_iDomainCargo);
	stream->Write(m_iCargoSpace);
// Thomas SG - AC: Advanced Cargo START
	stream->Write(m_iTotalCargoSpace);
// Thomas SG - AC: Advanced Cargo END	
	stream->Write(m_iConscriptionValue);
	stream->Write(m_iCultureGarrisonValue);
	stream->Write(m_iExtraCost);
	stream->Write(m_iAssetValue);
	stream->Write(m_iPowerValue);
	stream->Write(m_iUnitClassType);
// Thomas SG - AC: Advanced Cargo START
//	stream->Write(m_iSpecialUnitType);

	stream->Write((int)m_aiSpecialUnitTypes.size());
	for(int i=0;i<(int)m_aiSpecialUnitTypes.size();i++)
	{
		stream->Write(m_aiSpecialUnitTypes[i]);
	}
// Thomas SG - AC: Advanced Cargo END
	stream->Write(m_iUnitCaptureClassType);
	stream->Write(m_iUnitCombatType);
	stream->Write(m_iDomainType);
	stream->Write(m_iDefaultUnitAIType);
	stream->Write(m_iInvisibleType);

	stream->Write((int)m_aiSeeInvisibleTypes.size());
	for(int i=0;i<(int)m_aiSeeInvisibleTypes.size();i++)
	{
		stream->Write(m_aiSeeInvisibleTypes[i]);
	}
	
	stream->Write(m_iAdvisorType);

/********************************************************************************/
/**		REVDCM									2/16/10				phungus420	*/
/**																				*/
/**		CanTrain																*/
/********************************************************************************/
	stream->Write(m_iMaxStartEra);
	stream->Write(m_iForceObsoleteTech);
	stream->Write(m_bStateReligion);
	stream->Write(m_iPrereqGameOption);
	stream->Write(m_iNotGameOption);
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

	stream->Write(m_iHolyCity);
	stream->Write(m_iReligionType);
	stream->Write(m_iStateReligion);
	stream->Write(m_iPrereqReligion);
	stream->Write(m_iPrereqCorporation);
	stream->Write(m_iPrereqBuilding);

	int iNum = m_aePrereqOrBuildings.size();
	stream->Write(iNum);
	for (int i = 0; i < iNum; i++)
	{
		stream->Write(m_aePrereqOrBuildings[i]);
	}
	stream->Write(m_iOverrideResourceBuilding);
	stream->Write(m_iPrereqAndTech);
	stream->Write(m_iPrereqAndBonus);
	stream->Write(m_iGroupSize);
	stream->Write(m_iGroupDefinitions);
	stream->Write(m_iUnitMeleeWaveSize);
	stream->Write(m_iUnitRangedWaveSize);
	stream->Write(m_iNumUnitNames);
	stream->Write(m_iCommandType);
// Thomas SG - AC: Advanced Cargo START
	stream->Write((int)m_aiSpecialCargos.size());
	for(int i=0;i<(int)m_aiSpecialCargos.size();i++)
	{
		stream->Write(m_aiSpecialCargos[i]);
	}

	stream->Write((int)m_aiSpecialCargoSpaces.size());
	for(int i=0;i<(int)m_aiSpecialCargoSpaces.size();i++)
	{
		stream->Write(m_aiSpecialCargoSpaces[i]);
	}
// Thomas SG - AC: Advanced Cargo END	
	stream->Write(m_bAnimal);
	stream->Write(m_bFoodProduction);
	stream->Write(m_bNoBadGoodies);
	stream->Write(m_bOnlyDefensive);
	stream->Write(m_bNoCapture);
	stream->Write(m_bQuickCombat);
	stream->Write(m_bRivalTerritory);
	stream->Write(m_bMilitaryHappiness);
	stream->Write(m_bMilitarySupport);
	stream->Write(m_bMilitaryProduction);
	stream->Write(m_bPillage);
	stream->Write(m_bSpy);
	stream->Write(m_bSabotage);
	stream->Write(m_bDestroy);
	stream->Write(m_bStealPlans);
/************************************************************************************************/
/* Great Diplomat MOD               START                                      Stolenrays 		*/
/************************************************************************************************/
	stream->Write(m_bBribeBarbarian); 
	stream->Write(m_bGoodwill); 
	stream->Write(m_bBarbariansColonisator);
	stream->Write(m_bForcePeace);	
	stream->Write(m_bUpgradeImprovements);
/************************************************************************************************/
/* Great Diplomat MOD               END                                                  		*/
/************************************************************************************************/
	stream->Write(m_bInvestigate);
	stream->Write(m_bCounterSpy);
	stream->Write(m_bFound);
	stream->Write(m_bGoldenAge);
	stream->Write(m_bInvisible);
	stream->Write(m_bFirstStrikeImmune);
	stream->Write(m_bNoDefensiveBonus);
	stream->Write(m_bIgnoreBuildingDefense);
	stream->Write(m_bCanMoveImpassable);
	stream->Write(m_bCanMoveAllTerrain);
	stream->Write(m_bFlatMovementCost);
	stream->Write(m_bIgnoreTerrainCost);
	stream->Write(m_bNukeImmune);
	stream->Write(m_bPrereqBonuses);
	stream->Write(m_bPrereqReligion);
	stream->Write(m_bMechanized);
	stream->Write(m_bRenderBelowWater);
	stream->Write(m_bRenderAlways);
	stream->Write(m_bSuicide);
	stream->Write(m_bLineOfSight);
	stream->Write(m_bHiddenNationality);
	stream->Write(m_bAlwaysHostile);
	stream->Write(m_bNoRevealMap);
/************************************************************************************************/
/* REVOLUTION_MOD                                 01/01/08                        DPII          */
/*                                                                                              */
/* CoreComponent                                                                                */
/************************************************************************************************/
	stream->Write(m_bInquisitor);
	stream->Write(m_bCanBeRebel);
	stream->Write(m_bCanRebelCapture);
	stream->Write(m_bCannotDefect);
	stream->Write(m_bCanQuellRebellion);
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/
	//ls612: Can't enter non-Owned cities
	stream->Write(m_bNoNonOwnedEntry);

	stream->Write(m_fUnitMaxSpeed);
	stream->Write(m_fUnitPadTime);

	if (m_piPrereqAndTechs)
	{
		stream->Write(true);
		stream->Write(GC.getNUM_UNIT_AND_TECH_PREREQS(), m_piPrereqAndTechs);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piPrereqOrBonuses)
	{
		stream->Write(true);
		stream->Write(GC.getNUM_UNIT_PREREQ_OR_BONUSES(), m_piPrereqOrBonuses);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piProductionTraits)
	{
		stream->Write(true);
		stream->Write(GC.getNumTraitInfos(), m_piProductionTraits);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piFlavorValue)
	{
		stream->Write(true);
		stream->Write(GC.getNumFlavorTypes(), m_piFlavorValue);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piTerrainAttackModifier)
	{
		stream->Write(true);
		stream->Write(GC.getNumTerrainInfos(), m_piTerrainAttackModifier);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piTerrainDefenseModifier)
	{
		stream->Write(true);
		stream->Write(GC.getNumTerrainInfos(), m_piTerrainDefenseModifier);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piFeatureAttackModifier)
	{
		stream->Write(true);
		stream->Write(GC.getNumFeatureInfos(), m_piFeatureAttackModifier);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piFeatureDefenseModifier)
	{
		stream->Write(true);
		stream->Write(GC.getNumFeatureInfos(), m_piFeatureDefenseModifier);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piUnitClassAttackModifier)
	{
		stream->Write(true);
		stream->Write(GC.getNumUnitClassInfos(), m_piUnitClassAttackModifier);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piUnitClassDefenseModifier)
	{
		stream->Write(true);
		stream->Write(GC.getNumUnitClassInfos(), m_piUnitClassDefenseModifier);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piUnitCombatModifier)
	{
		stream->Write(true);
		stream->Write(GC.getNumUnitCombatInfos(), m_piUnitCombatModifier);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piUnitCombatCollateralImmune)
	{
		stream->Write(true);
		stream->Write(GC.getNumUnitCombatInfos(), m_piUnitCombatCollateralImmune);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piDomainModifier)
	{
		stream->Write(true);
		stream->Write(NUM_DOMAIN_TYPES, m_piDomainModifier);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piBonusProductionModifier)
	{
		stream->Write(true);
		stream->Write(GC.getNumBonusInfos(), m_piBonusProductionModifier);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piUnitGroupRequired)
	{
		stream->Write(true);
		stream->Write(m_iGroupDefinitions, m_piUnitGroupRequired);
	}
	else
	{
		stream->Write(false);
	}
	


/********************************************************************************/
/**		REVDCM									2/16/10				phungus420	*/
/**																				*/
/**		CanTrain																*/
/********************************************************************************/
	if (m_pbPrereqOrCivics)
	{
		stream->Write(true);
		stream->Write(GC.getNumCivicInfos(), m_pbPrereqOrCivics);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_pbPrereqBuildingClass)
	{
		stream->Write(true);
		stream->Write(GC.getNumBuildingClassInfos(), m_pbPrereqBuildingClass);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piPrereqBuildingClassOverrideTech)
	{
		stream->Write(true);
		stream->Write(GC.getNumBuildingClassInfos(), m_piPrereqBuildingClassOverrideTech);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piPrereqBuildingClassOverrideEra)
	{
		stream->Write(true);
		stream->Write(GC.getNumBuildingClassInfos(), m_piPrereqBuildingClassOverrideEra);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_pbForceObsoleteUnitClass)
	{
		stream->Write(true);
		stream->Write(GC.getNumUnitClassInfos(), m_pbForceObsoleteUnitClass);
	}
	else
	{
		stream->Write(false);
	}
	
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

	if (m_pbUpgradeUnitClass)
	{
		stream->Write(true);
		stream->Write(GC.getNumUnitClassInfos(), m_pbUpgradeUnitClass);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_pbTargetUnitClass)
	{
		stream->Write(true);
		stream->Write(GC.getNumUnitClassInfos(), m_pbTargetUnitClass);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_pbTargetUnitCombat)
	{
		stream->Write(true);
		stream->Write(GC.getNumUnitCombatInfos(), m_pbTargetUnitCombat);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_pbDefenderUnitClass)
	{
		stream->Write(true);
		stream->Write(GC.getNumUnitClassInfos(), m_pbDefenderUnitClass);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_pbDefenderUnitCombat)
	{
		stream->Write(true);
		stream->Write(GC.getNumUnitCombatInfos(), m_pbDefenderUnitCombat);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piFlankingStrikeUnitClass)
	{
		stream->Write(true);
		stream->Write(GC.getNumUnitClassInfos(), m_piFlankingStrikeUnitClass);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_pbUnitAIType)
	{
		stream->Write(true);
		stream->Write(NUM_UNITAI_TYPES, m_pbUnitAIType);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_pbNotUnitAIType)
	{
		stream->Write(true);
		stream->Write(NUM_UNITAI_TYPES, m_pbNotUnitAIType);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_pbBuilds)
	{
		stream->Write(true);
		stream->Write(GC.getNumBuildInfos(), m_pbBuilds);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piReligionSpreads)
	{
		stream->Write(true);
		stream->Write(GC.getNumReligionInfos(), m_piReligionSpreads);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piCorporationSpreads)
	{
		stream->Write(true);
		stream->Write(GC.getNumCorporationInfos(), m_piCorporationSpreads);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piTerrainPassableTech)
	{
		stream->Write(true);
		stream->Write(GC.getNumTerrainInfos(), m_piTerrainPassableTech);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_piFeaturePassableTech)
	{
		stream->Write(true);
		stream->Write(GC.getNumFeatureInfos(), m_piFeaturePassableTech);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_pbGreatPeoples)
	{
		stream->Write(true);
		stream->Write(GC.getNumSpecialistInfos(), m_pbGreatPeoples);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_pbBuildings)
	{
		stream->Write(true);
		stream->Write(GC.getNumBuildingInfos(), m_pbBuildings);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_pbForceBuildings)
	{
		stream->Write(true);
		stream->Write(GC.getNumBuildingInfos(), m_pbForceBuildings);
	}
	else
	{
		stream->Write(false);
	}

	if (m_pbTerrainNative)
	{
		stream->Write(true);
		stream->Write(GC.getNumTerrainInfos(), m_pbTerrainNative);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_pbFeatureNative)
	{
		stream->Write(true);
		stream->Write(GC.getNumFeatureInfos(), m_pbFeatureNative);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_pbTerrainImpassable)
	{
		stream->Write(true);
		stream->Write(GC.getNumTerrainInfos(), m_pbTerrainImpassable);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_pbFeatureImpassable)
	{
		stream->Write(true);
		stream->Write(GC.getNumFeatureInfos(), m_pbFeatureImpassable);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_pbFreePromotions)
	{
		stream->Write(true);
		stream->Write(GC.getNumPromotionInfos(), m_pbFreePromotions);
	}
	else
	{
		stream->Write(false);
	}
	

	stream->Write(m_iLeaderPromotion);
	stream->Write(m_iLeaderExperience);

	if (m_paszEarlyArtDefineTags)
	{
		stream->Write(true);
		stream->WriteString(m_iGroupDefinitions, m_paszEarlyArtDefineTags);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_paszLateArtDefineTags)
	{
		stream->Write(true);
		stream->WriteString(m_iGroupDefinitions, m_paszLateArtDefineTags);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_paszMiddleArtDefineTags)
	{
		stream->Write(true);
		stream->WriteString(m_iGroupDefinitions, m_paszMiddleArtDefineTags);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_paszUnitNames)
	{
		stream->Write(true);
		stream->WriteString(m_iNumUnitNames, m_paszUnitNames);
	}
	else
	{
		stream->Write(false);
	}
	

	stream->WriteString(m_szFormationType);
/************************************************************************************************/
/* Afforess	                  Start		 Last Update: 3/8/10                                    */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	stream->Write(m_bRequiresStateReligionInCity);
	stream->Write(m_bWorkerTrade);
	stream->Write(m_bMilitaryTrade);
	stream->Write(m_bForceUpgrade);
	stream->Write(m_bGreatGeneral);
	stream->Write(m_bSlave);
	stream->Write(m_iPrereqVicinityBonus);
	stream->Write(m_iBaseFoodChange);
	stream->Write(m_iControlPoints);
	stream->Write(m_iCommandRange);

	if (m_piPrereqOrVicinityBonuses)
	{
		stream->Write(true);
		stream->Write(GC.getNUM_UNIT_PREREQ_OR_BONUSES(), m_piPrereqOrVicinityBonuses);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_pbPrereqBuildingClass)
	{
		stream->Write(true);
		stream->Write(GC.getNumBuildingClassInfos(), m_pbPrereqBuildingClass);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_pbPassableRouteNeeded)
	{
		stream->Write(true);
		stream->Write(GC.getNumRouteInfos(), m_pbPassableRouteNeeded);
	}
	else
	{
		stream->Write(false);
	}
	
	
	if (m_paszClassicalArtDefineTags)
	{
		stream->Write(true);
		stream->WriteString(m_iGroupDefinitions, m_paszClassicalArtDefineTags);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_paszRennArtDefineTags)
	{
		stream->Write(true);
		stream->WriteString(m_iGroupDefinitions, m_paszRennArtDefineTags);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_paszIndustrialArtDefineTags)
	{
		stream->Write(true);
		stream->WriteString(m_iGroupDefinitions, m_paszIndustrialArtDefineTags);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_paszFutureArtDefineTags)
	{
		stream->Write(true);
		stream->WriteString(m_iGroupDefinitions, m_paszFutureArtDefineTags);
	}
	else
	{
		stream->Write(false);
	}
	
	if (m_paszCivilizationNames)
	{
		stream->Write(GC.getNumCivilizationInfos());
		stream->WriteString(GC.getNumCivilizationInfos(), m_paszCivilizationNames);
	}
	else
	{
		stream->Write((int)0);
	}
	
	
	stream->Write((int)m_aiUpgradeUnitClassTypes.size());
	for(int i=0;i<(int)m_aiUpgradeUnitClassTypes.size();i++)
	{
		stream->Write(m_aiUpgradeUnitClassTypes[i]);
	}

	getKillOutcomeList()->write(stream);

	int iNumOutcomeMissions = (int)m_aOutcomeMissions.size();
	stream->Write(iNumOutcomeMissions);
	for(int i=0; i<iNumOutcomeMissions; i++)
	{
		m_aOutcomeMissions[i]->write(stream);
	}

	m_PropertyManipulators.write(stream);


	if (m_pExprTrainCondition)
	{
		stream->Write(true);
		m_pExprTrainCondition->write(stream);
	}
	else
	{
		stream->Write(false);
	}


	stream->WriteString(m_szExtraHoverTextKey);
}


void CvUnitInfo::getCheckSum(unsigned int &iSum)
{
	CheckSum(iSum, m_iVolleyRange);
	CheckSum(iSum, m_iVolleyAccuracy);
	CheckSum(iSum, m_iVolleyAccuracyMin);
	CheckSum(iSum, m_iVolleyEfficiency);
	CheckSum(iSum, m_iVolleyRounds);

	CheckSum(iSum, m_bDCMAirBomb1);
	CheckSum(iSum, m_bDCMAirBomb2);
	CheckSum(iSum, m_bDCMAirBomb3);
	CheckSum(iSum, m_bDCMAirBomb4);
	CheckSum(iSum, m_bDCMAirBomb5);
	CheckSum(iSum, m_bDCMFighterEngage);

	CheckSum(iSum, m_iAIWeight);
	CheckSum(iSum, m_iProductionCost);
	CheckSum(iSum, m_iHurryCostModifier);
	CheckSum(iSum, m_iAdvancedStartCost);
	CheckSum(iSum, m_iAdvancedStartCostIncrease);
	CheckSum(iSum, m_iMinAreaSize);
	CheckSum(iSum, m_iMoves);
	CheckSum(iSum, m_iAirRange);
	CheckSum(iSum, m_iAirUnitCap);
	CheckSum(iSum, m_iDropRange);
	CheckSum(iSum, m_iNukeRange);
	CheckSum(iSum, m_iWorkRate);
	CheckSum(iSum, m_iBaseDiscover);
	CheckSum(iSum, m_iDiscoverMultiplier);
	CheckSum(iSum, m_iBaseHurry);
	CheckSum(iSum, m_iHurryMultiplier);
	CheckSum(iSum, m_iBaseTrade);
	CheckSum(iSum, m_iTradeMultiplier);
	CheckSum(iSum, m_iGreatWorkCulture);
	CheckSum(iSum, m_iEspionagePoints);
	CheckSum(iSum, m_iCombat);
	CheckSum(iSum, m_iCombatLimit);
	CheckSum(iSum, m_iAirCombat);
	CheckSum(iSum, m_iAirCombatLimit);
	CheckSum(iSum, m_iXPValueAttack);
	CheckSum(iSum, m_iXPValueDefense);
	CheckSum(iSum, m_iFirstStrikes);
	CheckSum(iSum, m_iChanceFirstStrikes);
	CheckSum(iSum, m_iInterceptionProbability);
	//ls612: Advanced Nuke Interception
	//CheckSum(iSum, m_iNukeInterceptionProbability);
	//CheckSum(iSum, m_iNukeInterceptionRange);
	CheckSum(iSum, m_iEvasionProbability);
	CheckSum(iSum, m_iWithdrawalProbability);
	CheckSum(iSum, m_iCollateralDamage);
	CheckSum(iSum, m_iCollateralDamageLimit);
	CheckSum(iSum, m_iCollateralDamageMaxUnits);
	CheckSum(iSum, m_iCityAttackModifier);
	CheckSum(iSum, m_iCityDefenseModifier);
	CheckSum(iSum, m_iAnimalCombatModifier);
	CheckSum(iSum, m_iHillsAttackModifier);
	CheckSum(iSum, m_iHillsDefenseModifier);
	CheckSum(iSum, m_iBombRate);
	CheckSum(iSum, m_iBombardRate);
	//CheckSum(iSum, m_iSpecialCargo); removed by 45deg for advanced cargo mod
	CheckSum(iSum, m_iDomainCargo);
	CheckSum(iSum, m_iCargoSpace);
	CheckSum(iSum, m_iConscriptionValue);
	CheckSum(iSum, m_iCultureGarrisonValue);
	CheckSum(iSum, m_iExtraCost);
	CheckSum(iSum, m_iAssetValue);
	CheckSum(iSum, m_iPowerValue);
	CheckSum(iSum, m_iUnitClassType);
	//CheckSum(iSum, m_iSpecialUnitType); removed by 45deg for advanced cargo mod
	CheckSum(iSum, m_iUnitCaptureClassType);
	CheckSum(iSum, m_iUnitCombatType);
	CheckSum(iSum, m_iDomainType);
	CheckSum(iSum, m_iDefaultUnitAIType);
	CheckSum(iSum, m_iInvisibleType);
	
	CheckSumC(iSum, m_aiSeeInvisibleTypes);

	CheckSum(iSum, m_iAdvisorType);

	CheckSum(iSum, m_iMaxStartEra);
	CheckSum(iSum, m_iForceObsoleteTech);
	CheckSum(iSum, m_bStateReligion);
	CheckSum(iSum, m_iPrereqGameOption);
	CheckSum(iSum, m_iNotGameOption);

	CheckSum(iSum, m_iHolyCity);
	CheckSum(iSum, m_iReligionType);
	CheckSum(iSum, m_iStateReligion);
	CheckSum(iSum, m_iPrereqReligion);
	CheckSum(iSum, m_iPrereqCorporation);
	CheckSum(iSum, m_iPrereqBuilding);
	
	CheckSumC(iSum, m_aePrereqOrBuildings);
	CheckSum(iSum, m_iOverrideResourceBuilding);	
	CheckSum(iSum, m_iPrereqAndTech);
	CheckSum(iSum, m_iPrereqAndBonus);
	CheckSum(iSum, m_iGroupSize);
	CheckSum(iSum, m_iGroupDefinitions);
	CheckSum(iSum, m_iUnitMeleeWaveSize);
	CheckSum(iSum, m_iUnitRangedWaveSize);
	CheckSum(iSum, m_iNumUnitNames);
	CheckSum(iSum, m_iCommandType);

	CheckSum(iSum, m_bAnimal);
	CheckSum(iSum, m_bFoodProduction);
	CheckSum(iSum, m_bNoBadGoodies);
	CheckSum(iSum, m_bOnlyDefensive);
	CheckSum(iSum, m_bNoCapture);
	CheckSum(iSum, m_bQuickCombat);
	CheckSum(iSum, m_bRivalTerritory);
	CheckSum(iSum, m_bMilitaryHappiness);
	CheckSum(iSum, m_bMilitarySupport);
	CheckSum(iSum, m_bMilitaryProduction);
	CheckSum(iSum, m_bPillage);
	CheckSum(iSum, m_bSpy);
	CheckSum(iSum, m_bSabotage);
	CheckSum(iSum, m_bDestroy);
	CheckSum(iSum, m_bStealPlans);
	CheckSum(iSum, m_bInvestigate);
	CheckSum(iSum, m_bCounterSpy);
	CheckSum(iSum, m_bFound);
	CheckSum(iSum, m_bGoldenAge);
	CheckSum(iSum, m_bInvisible);
	CheckSum(iSum, m_bFirstStrikeImmune);
	CheckSum(iSum, m_bNoDefensiveBonus);
	CheckSum(iSum, m_bIgnoreBuildingDefense);
	CheckSum(iSum, m_bCanMoveImpassable);
	CheckSum(iSum, m_bCanMoveAllTerrain);
	CheckSum(iSum, m_bFlatMovementCost);
	CheckSum(iSum, m_bIgnoreTerrainCost);
	CheckSum(iSum, m_bNukeImmune);
	CheckSum(iSum, m_bPrereqBonuses);
	CheckSum(iSum, m_bPrereqReligion);
	CheckSum(iSum, m_bMechanized);
	CheckSum(iSum, m_bRenderBelowWater);
	CheckSum(iSum, m_bRenderAlways);
	CheckSum(iSum, m_bSuicide);
	CheckSum(iSum, m_bLineOfSight);
	CheckSum(iSum, m_bHiddenNationality);
	CheckSum(iSum, m_bAlwaysHostile);
	CheckSum(iSum, m_bNoRevealMap);

	CheckSum(iSum, m_bInquisitor);
	CheckSum(iSum, m_bCanBeRebel);
	CheckSum(iSum, m_bCanRebelCapture);
	CheckSum(iSum, m_bCannotDefect);
	CheckSum(iSum, m_bCanQuellRebellion);

	CheckSum(iSum, m_bNoNonOwnedEntry);

	//CheckSum(iSum, m_fUnitMaxSpeed);
	//CheckSum(iSum, m_fUnitPadTime);

	CheckSumI(iSum, GC.getNUM_UNIT_AND_TECH_PREREQS(), m_piPrereqAndTechs);
	CheckSumI(iSum, GC.getNUM_UNIT_PREREQ_OR_BONUSES(), m_piPrereqOrBonuses);
	CheckSumI(iSum, GC.getNumTraitInfos(), m_piProductionTraits);
	CheckSumI(iSum, GC.getNumFlavorTypes(), m_piFlavorValue);
	CheckSumI(iSum, GC.getNumTerrainInfos(), m_piTerrainAttackModifier);
	CheckSumI(iSum, GC.getNumTerrainInfos(), m_piTerrainDefenseModifier);
	CheckSumI(iSum, GC.getNumFeatureInfos(), m_piFeatureAttackModifier);
	CheckSumI(iSum, GC.getNumFeatureInfos(), m_piFeatureDefenseModifier);
	CheckSumI(iSum, GC.getNumUnitClassInfos(), m_piUnitClassAttackModifier);
	CheckSumI(iSum, GC.getNumUnitClassInfos(), m_piUnitClassDefenseModifier);
	CheckSumI(iSum, GC.getNumUnitCombatInfos(), m_piUnitCombatModifier);
	CheckSumI(iSum, GC.getNumUnitCombatInfos(), m_piUnitCombatCollateralImmune);
	CheckSumI(iSum, NUM_DOMAIN_TYPES, m_piDomainModifier);
	CheckSumI(iSum, GC.getNumBonusInfos(), m_piBonusProductionModifier);
	//CheckSumI(iSum, m_iGroupDefinitions, m_piUnitGroupRequired);

	CheckSumI(iSum, GC.getNumCivicInfos(), m_pbPrereqOrCivics);
	CheckSumI(iSum, GC.getNumBuildingClassInfos(), m_pbPrereqBuildingClass);
	CheckSumI(iSum, GC.getNumBuildingClassInfos(), m_piPrereqBuildingClassOverrideTech);
	CheckSumI(iSum, GC.getNumBuildingClassInfos(), m_piPrereqBuildingClassOverrideEra);
	CheckSumI(iSum, GC.getNumUnitClassInfos(), m_pbForceObsoleteUnitClass);
	CheckSumI(iSum, GC.getNumUnitClassInfos(), m_pbUpgradeUnitClass);
	CheckSumI(iSum, GC.getNumUnitClassInfos(), m_pbTargetUnitClass);
	CheckSumI(iSum, GC.getNumUnitCombatInfos(), m_pbTargetUnitCombat);
	CheckSumI(iSum, GC.getNumUnitClassInfos(), m_pbDefenderUnitClass);
	CheckSumI(iSum, GC.getNumUnitCombatInfos(), m_pbDefenderUnitCombat);
	CheckSumI(iSum, GC.getNumUnitClassInfos(), m_piFlankingStrikeUnitClass);
	CheckSumI(iSum, NUM_UNITAI_TYPES, m_pbUnitAIType);
	CheckSumI(iSum, NUM_UNITAI_TYPES, m_pbNotUnitAIType);
	CheckSumI(iSum, GC.getNumBuildInfos(), m_pbBuilds);
	CheckSumI(iSum, GC.getNumReligionInfos(), m_piReligionSpreads);
	CheckSumI(iSum, GC.getNumCorporationInfos(), m_piCorporationSpreads);
	CheckSumI(iSum, GC.getNumTerrainInfos(), m_piTerrainPassableTech);
	CheckSumI(iSum, GC.getNumFeatureInfos(), m_piFeaturePassableTech);
	CheckSumI(iSum, GC.getNumSpecialistInfos(), m_pbGreatPeoples);
	CheckSumI(iSum, GC.getNumBuildingInfos(), m_pbBuildings);
	CheckSumI(iSum, GC.getNumBuildingInfos(), m_pbForceBuildings);
	CheckSumI(iSum, GC.getNumTerrainInfos(), m_pbTerrainNative);
	CheckSumI(iSum, GC.getNumFeatureInfos(), m_pbFeatureNative);
	CheckSumI(iSum, GC.getNumTerrainInfos(), m_pbTerrainImpassable);
	CheckSumI(iSum, GC.getNumFeatureInfos(), m_pbFeatureImpassable);
	CheckSumI(iSum, GC.getNumPromotionInfos(), m_pbFreePromotions);

	CheckSum(iSum, m_iLeaderPromotion);
	CheckSum(iSum, m_iLeaderExperience);

	CheckSum(iSum, m_bRequiresStateReligionInCity);
	CheckSum(iSum, m_bWorkerTrade);
	CheckSum(iSum, m_bMilitaryTrade);
	CheckSum(iSum, m_bForceUpgrade);
	CheckSum(iSum, m_bGreatGeneral);
	CheckSum(iSum, m_bSlave);
	CheckSum(iSum, m_iPrereqVicinityBonus);
	CheckSum(iSum, m_iBaseFoodChange);
	CheckSum(iSum, m_iControlPoints);
	CheckSum(iSum, m_iCommandRange);

	CheckSumI(iSum, GC.getNUM_UNIT_PREREQ_OR_BONUSES(), m_piPrereqOrVicinityBonuses);
	CheckSumI(iSum, GC.getNumBuildingClassInfos(), m_pbPrereqBuildingClass);
	CheckSumI(iSum, GC.getNumRouteInfos(), m_pbPassableRouteNeeded);

	CheckSumC(iSum, m_aiUpgradeUnitClassTypes);

	getKillOutcomeList()->getCheckSum(iSum);

	for (int i=0; i<(int)m_aOutcomeMissions.size(); i++)
	{
		m_aOutcomeMissions[i]->getCheckSum(iSum);
	}

	m_PropertyManipulators.getCheckSum(iSum);

	if (m_pExprTrainCondition)
		m_pExprTrainCondition->getCheckSum(iSum);

	CheckSum(iSum, m_szExtraHoverTextKey);
}

//
// read from xml
//
bool CvUnitInfo::read(CvXMLLoadUtility* pXML)
{
	MEMORY_TRACE_FUNCTION();

	CvString szTextVal;
	if (!CvHotkeyInfo::read(pXML))
	{
		return false;
	}

	int j=0;				//loop counter
	int k=0;				//loop counter
	int iNumSibs=0;				// the number of siblings the current xml node has
	int iIndexVal;

	pXML->GetChildXmlValByName(szTextVal, "Class");
	m_iUnitClassType = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "Special");
// Thomas SG - AC: Advanced Cargo START
//	m_iSpecialUnitType = pXML->FindInInfoClass(szTextVal);

	std::vector<CvString> tokens_Special;
	szTextVal.getTokens(",", tokens_Special);
	for(int i=0;i<(int)tokens_Special.size();i++)
	{
		int iSpecialUnitType = pXML->FindInInfoClass(tokens_Special[i]);
		if(iSpecialUnitType != NO_SPECIALUNIT)
		{
			m_aiSpecialUnitTypes.push_back(iSpecialUnitType);
		}
	}
// Thomas SG - AC: Advanced Cargo END
	// EXTRA HOVER TEXT
	pXML->GetChildXmlValByName(m_szExtraHoverTextKey, "ExtraHoverText");

	pXML->GetChildXmlValByName(szTextVal, "Capture");
	m_iUnitCaptureClassType = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "Combat");
	m_iUnitCombatType = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "Domain");
	m_iDomainType = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "DefaultUnitAI");
	m_iDefaultUnitAIType = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "Invisible");
	m_iInvisibleType = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "SeeInvisible");
	std::vector<CvString> tokens;
	szTextVal.getTokens(",", tokens);
	for(int i=0;i<(int)tokens.size();i++)
	{
		int iInvisibleType = pXML->FindInInfoClass(tokens[i]);
		if(iInvisibleType != NO_INVISIBLE)
		{
			m_aiSeeInvisibleTypes.push_back(iInvisibleType);
		}
	}
// Thomas SG - AC: Advanced Cargo START
	pXML->GetChildXmlValByName(szTextVal, "SpecialCargo");
	std::vector<CvString> tokens_SpecialCargo;
	szTextVal.getTokens(",", tokens_SpecialCargo);
	for(int i=0;i<(int)tokens_SpecialCargo.size();i++)
	{
		int iSpecialCargo = pXML->FindInInfoClass(tokens_SpecialCargo[i]);
		if(iSpecialCargo != NO_SPECIALUNIT)
		{
			m_aiSpecialCargos.push_back(iSpecialCargo);
		}
	}

	pXML->GetChildXmlValByName(szTextVal, "iSpecialCargoSpace");
	std::vector<CvString> tokens_SpecialCargoSpace;
	szTextVal.getTokens(",", tokens_SpecialCargoSpace);
	for(int i=0;i<(int)tokens_SpecialCargoSpace.size();i++)
	{
		int iSpecialCargoSpace = atoi(tokens_SpecialCargoSpace[i]);
		
		m_aiSpecialCargoSpaces.push_back(iSpecialCargoSpace);
	}
// Thomas SG - AC: Advanced Cargo END
	pXML->GetChildXmlValByName(szTextVal, "Advisor");
	m_iAdvisorType = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(&m_bAnimal, "bAnimal");
	pXML->GetChildXmlValByName(&m_bFoodProduction, "bFood");
	pXML->GetChildXmlValByName(&m_bNoBadGoodies, "bNoBadGoodies");
	pXML->GetChildXmlValByName(&m_bOnlyDefensive, "bOnlyDefensive");
	pXML->GetChildXmlValByName(&m_bNoCapture, "bNoCapture");
	pXML->GetChildXmlValByName(&m_bQuickCombat, "bQuickCombat");
	pXML->GetChildXmlValByName(&m_bRivalTerritory, "bRivalTerritory");
	pXML->GetChildXmlValByName(&m_bMilitaryHappiness, "bMilitaryHappiness");
	pXML->GetChildXmlValByName(&m_bMilitarySupport, "bMilitarySupport");
	pXML->GetChildXmlValByName(&m_bMilitaryProduction, "bMilitaryProduction");
	pXML->GetChildXmlValByName(&m_bPillage, "bPillage");
	pXML->GetChildXmlValByName(&m_bSpy, "bSpy");
	pXML->GetChildXmlValByName(&m_bSabotage, "bSabotage");
	pXML->GetChildXmlValByName(&m_bDestroy, "bDestroy");
	pXML->GetChildXmlValByName(&m_bStealPlans, "bStealPlans");
/************************************************************************************************/
/* Great Diplomat MOD               START                                      Stolenrays 		*/
/************************************************************************************************/
	pXML->GetChildXmlValByName(&m_bBribeBarbarian, "bBribeBarbarian"); 
	pXML->GetChildXmlValByName(&m_bGoodwill, "bGoodwill"); 
	pXML->GetChildXmlValByName(&m_bBarbariansColonisator, "bBarbariansColonisator");
	pXML->GetChildXmlValByName(&m_bForcePeace, "bForcePeace");	
	pXML->GetChildXmlValByName(&m_bUpgradeImprovements, "bUpgradeImprovements");
/************************************************************************************************/
/* Great Diplomat MOD               END                                                  		*/
/************************************************************************************************/
	pXML->GetChildXmlValByName(&m_bInvestigate, "bInvestigate");
	pXML->GetChildXmlValByName(&m_bCounterSpy, "bCounterSpy");
	pXML->GetChildXmlValByName(&m_bFound, "bFound");
	pXML->GetChildXmlValByName(&m_bGoldenAge, "bGoldenAge");
	pXML->GetChildXmlValByName(&m_bInvisible, "bInvisible");
	pXML->GetChildXmlValByName(&m_bFirstStrikeImmune, "bFirstStrikeImmune");
	pXML->GetChildXmlValByName(&m_bNoDefensiveBonus, "bNoDefensiveBonus");
	pXML->GetChildXmlValByName(&m_bIgnoreBuildingDefense, "bIgnoreBuildingDefense");
	pXML->GetChildXmlValByName(&m_bCanMoveImpassable, "bCanMoveImpassable");
	pXML->GetChildXmlValByName(&m_bCanMoveAllTerrain, "bCanMoveAllTerrain");
	pXML->GetChildXmlValByName(&m_bFlatMovementCost, "bFlatMovementCost");
	pXML->GetChildXmlValByName(&m_bIgnoreTerrainCost, "bIgnoreTerrainCost");
	pXML->GetChildXmlValByName(&m_bNukeImmune, "bNukeImmune");
	pXML->GetChildXmlValByName(&m_bPrereqBonuses, "bPrereqBonuses");
	pXML->GetChildXmlValByName(&m_bPrereqReligion, "bPrereqReligion");
	pXML->GetChildXmlValByName(&m_bMechanized,"bMechanized",false);
	pXML->GetChildXmlValByName(&m_bRenderBelowWater,"bRenderBelowWater",false);
	pXML->GetChildXmlValByName(&m_bRenderAlways,"bRenderAlways",false);
	pXML->GetChildXmlValByName(&m_bSuicide,"bSuicide");
	pXML->GetChildXmlValByName(&m_bLineOfSight,"bLineOfSight",false);
	pXML->GetChildXmlValByName(&m_bHiddenNationality,"bHiddenNationality",false);
	pXML->GetChildXmlValByName(&m_bAlwaysHostile,"bAlwaysHostile",false);
	pXML->GetChildXmlValByName(&m_bNoRevealMap,"bNoRevealMap",false);
/************************************************************************************************/
/* REVOLUTION_MOD                                 01/01/08                        DPII          */
/*                                                                                              */
/* CoreComponent                                                                                */
/************************************************************************************************/
	pXML->GetChildXmlValByName(&m_bInquisitor,"bInquisitor",false);
	pXML->GetChildXmlValByName(&m_bCanBeRebel, "bCanBeRebel");
	pXML->GetChildXmlValByName(&m_bCanRebelCapture, "bCanRebelCapture");
	pXML->GetChildXmlValByName(&m_bCannotDefect, "bCannotDefect");
	pXML->GetChildXmlValByName(&m_bCanQuellRebellion, "bCanQuellRebellion");
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/
	//ls612: Can't enter non-Owned cities
	pXML->GetChildXmlValByName(&m_bNoNonOwnedEntry, "bOnlyFriendlyEntry");

	pXML->SetVariableListTagPair(&m_pbUpgradeUnitClass, "UnitClassUpgrades", sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());
	pXML->SetVariableListTagPair(&m_pbTargetUnitClass, "UnitClassTargets", sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());
	pXML->SetVariableListTagPair(&m_pbTargetUnitCombat, "UnitCombatTargets", sizeof(GC.getUnitCombatInfo((UnitCombatTypes)0)), GC.getNumUnitCombatInfos());
	pXML->SetVariableListTagPair(&m_pbDefenderUnitClass, "UnitClassDefenders", sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());
	pXML->SetVariableListTagPair(&m_pbDefenderUnitCombat, "UnitCombatDefenders", sizeof(GC.getUnitCombatInfo((UnitCombatTypes)0)), GC.getNumUnitCombatInfos());
	pXML->SetVariableListTagPair(&m_piFlankingStrikeUnitClass, "FlankingStrikes", sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos(), -1);
	pXML->SetVariableListTagPair(&m_pbUnitAIType, "UnitAIs", sizeof(GC.getUnitAIInfo((UnitAITypes)0)), NUM_UNITAI_TYPES);
	pXML->SetVariableListTagPair(&m_pbNotUnitAIType, "NotUnitAIs", sizeof(GC.getUnitAIInfo((UnitAITypes)0)), NUM_UNITAI_TYPES);

	pXML->SetVariableListTagPair(&m_pbBuilds, "Builds", sizeof(GC.getBuildInfo((BuildTypes)0)), GC.getNumBuildInfos());

	pXML->SetVariableListTagPair(&m_piReligionSpreads, "ReligionSpreads", sizeof(GC.getReligionInfo((ReligionTypes)0)), GC.getNumReligionInfos(),-1);
	pXML->SetVariableListTagPair(&m_piCorporationSpreads, "CorporationSpreads", sizeof(GC.getCorporationInfo((CorporationTypes)0)), GC.getNumCorporationInfos(), -1);

	CvString* pszTemp = NULL;
	pXML->SetVariableListTagPair(&pszTemp, "TerrainPassableTechs", sizeof(GC.getTerrainInfo((TerrainTypes)0)), GC.getNumTerrainInfos());
	if ( pszTemp != NULL )
	{
		m_piTerrainPassableTech = new int[GC.getNumTerrainInfos()];
		for (int i = 0; i < GC.getNumTerrainInfos(); ++i)
		{
			m_piTerrainPassableTech[i] = pszTemp[i].IsEmpty() ? NO_TECH : pXML->FindInInfoClass(pszTemp[i]);
		}
		SAFE_DELETE_ARRAY(pszTemp);
	}
	else
	{
		m_piTerrainPassableTech = NULL;
	}

	pXML->SetVariableListTagPair(&pszTemp, "FeaturePassableTechs", sizeof(GC.getFeatureInfo((FeatureTypes)0)), GC.getNumFeatureInfos());
	if ( pszTemp != NULL )
	{
		m_piFeaturePassableTech = new int[GC.getNumFeatureInfos()];
		for (int i = 0; i < GC.getNumFeatureInfos(); ++i)
		{
			m_piFeaturePassableTech[i] = pszTemp[i].IsEmpty() ? NO_TECH : pXML->FindInInfoClass(pszTemp[i]);
		}
		SAFE_DELETE_ARRAY(pszTemp);
	}
	else
	{
		m_piFeaturePassableTech = NULL;
	}

	pXML->SetVariableListTagPair(&m_pbGreatPeoples, "GreatPeoples", sizeof(GC.getSpecialistInfo((SpecialistTypes)0)), GC.getNumSpecialistInfos());

	pXML->SetVariableListTagPair(&m_pbBuildings, "Buildings", sizeof(GC.getBuildingInfo((BuildingTypes)0)), GC.getNumBuildingInfos());
	pXML->SetVariableListTagPair(&m_pbForceBuildings, "ForceBuildings", sizeof(GC.getBuildingInfo((BuildingTypes)0)), GC.getNumBuildingInfos());
	
/********************************************************************************/
/**		REVDCM									2/16/10				phungus420	*/
/**																				*/
/**		CanTrain																*/
/********************************************************************************/
	pXML->GetChildXmlValByName(szTextVal, "MaxStartEra");
	m_iMaxStartEra = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "ForceObsoleteTech");
	m_iForceObsoleteTech = pXML->FindInInfoClass(szTextVal);
	
	pXML->GetChildXmlValByName(&m_bStateReligion, "bStateReligion");

	pXML->GetChildXmlValByName(szTextVal, "PrereqGameOption");
	m_iPrereqGameOption = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "NotGameOption");
	m_iNotGameOption = pXML->FindInInfoClass(szTextVal);

	pXML->SetVariableListTagPair(&m_pbPrereqOrCivics, "PrereqOrCivics", sizeof(GC.getCivicInfo((CivicTypes)0)), GC.getNumCivicInfos());

	pXML->SetVariableListTagPair(&m_pbPrereqBuildingClass, "PrereqBuildingClasses", sizeof(GC.getBuildingClassInfo((BuildingClassTypes)0)), GC.getNumBuildingClassInfos());

	pXML->SetVariableListTagPair(&m_piPrereqBuildingClassOverrideTech, "PrereqBuildingClasses", sizeof(GC.getBuildingClassInfo((BuildingClassTypes)0)), GC.getNumBuildingClassInfos(),
		"TechOverride", GC.getNumTechInfos());

	pXML->SetVariableListTagPair(&m_piPrereqBuildingClassOverrideEra, "PrereqBuildingClasses", sizeof(GC.getBuildingClassInfo((BuildingClassTypes)0)), GC.getNumBuildingClassInfos(),
		"EraOverride", GC.getNumEraInfos());

	pXML->SetVariableListTagPair(&m_pbForceObsoleteUnitClass, "ForceObsoleteUnitClasses", sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

	pXML->GetChildXmlValByName(szTextVal, "HolyCity");
	m_iHolyCity = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "ReligionType");
	m_iReligionType = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "StateReligion");
	m_iStateReligion = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "PrereqReligion");
	m_iPrereqReligion = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "PrereqCorporation");
	m_iPrereqCorporation = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "PrereqBuilding");
	m_iPrereqBuilding = pXML->FindInInfoClass(szTextVal);

	if(GETXML->SetToChildByTagName( pXML->GetXML(), "PrereqOrBuildings"))
	{
		if(GETXML->SetToChild( pXML->GetXML() ))
		{

			if (GETXML->LocateFirstSiblingNodeByTagName(pXML->GetXML(), "BuildingType"))
			{
				do
				{
					int iPrereqOrBuilding;

					pXML->GetXmlVal(szTextVal);
					iPrereqOrBuilding = pXML->FindInInfoClass(szTextVal);

					m_aePrereqOrBuildings.push_back((BuildingTypes)iPrereqOrBuilding);
				} while(GETXML->NextSibling(pXML->GetXML()));
			}
			GETXML->SetToParent( pXML->GetXML() );
		}
		GETXML->SetToParent( pXML->GetXML() );
	}

	pXML->GetChildXmlValByName(szTextVal, "OverrideResourceBuilding");
	m_iOverrideResourceBuilding = pXML->FindInInfoClass(szTextVal);
	
	pXML->GetChildXmlValByName(szTextVal, "PrereqTech");
	m_iPrereqAndTech = pXML->FindInInfoClass(szTextVal);

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"TechTypes"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = GETXML->GetNumChildren(pXML->GetXML());
			FAssertMsg((0 < GC.getNUM_UNIT_AND_TECH_PREREQS()) ,"Allocating zero or less memory in SetGlobalUnitInfo");

			if (0 < iNumSibs)
			{
				pXML->CvXMLLoadUtility::InitList(&m_piPrereqAndTechs, GC.getNUM_UNIT_AND_TECH_PREREQS(), -1);
				if (pXML->GetChildXmlVal(szTextVal))
				{
					FAssertMsg((iNumSibs <= GC.getNUM_UNIT_AND_TECH_PREREQS()) ,"There are more siblings than memory allocated for them in SetGlobalUnitInfo");
					for (j=0;j<iNumSibs;j++)
					{
						m_piPrereqAndTechs[j] = pXML->FindInInfoClass(szTextVal);
						if (!pXML->GetNextXmlVal(szTextVal))
						{
							break;
						}
					}

					GETXML->SetToParent(pXML->GetXML());
				}
			}
			else
			{
				SAFE_DELETE_ARRAY(m_piPrereqAndTechs);
			}
		}

		GETXML->SetToParent(pXML->GetXML());
	}
/************************************************************************************************/
/* XMLCOPY                                 10/08/07                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	else
	{
		SAFE_DELETE_ARRAY(m_piPrereqAndTechs);
	}
/************************************************************************************************/
/* XMLCOPY                                 END                                                  */
/************************************************************************************************/

	pXML->GetChildXmlValByName(szTextVal, "BonusType");
	m_iPrereqAndBonus = pXML->FindInInfoClass(szTextVal);

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"PrereqBonuses"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = GETXML->GetNumChildren(pXML->GetXML());
			FAssertMsg((0 < GC.getNUM_UNIT_PREREQ_OR_BONUSES()),"Allocating zero or less memory in SetGlobalUnitInfo");

			if (0 < iNumSibs)
			{
				pXML->CvXMLLoadUtility::InitList(&m_piPrereqOrBonuses, GC.getNUM_UNIT_PREREQ_OR_BONUSES(), -1);
				if (pXML->GetChildXmlVal(szTextVal))
				{
					FAssertMsg((iNumSibs <= GC.getNUM_UNIT_PREREQ_OR_BONUSES()) , "There are more siblings than memory allocated for them in SetGlobalUnitInfo");
					for (j=0;j<iNumSibs;j++)
					{
						m_piPrereqOrBonuses[j] = pXML->FindInInfoClass(szTextVal);
						if (!pXML->GetNextXmlVal(szTextVal))
						{
							break;
						}
					}

					GETXML->SetToParent(pXML->GetXML());
				}
			}
			else
			{
				SAFE_DELETE_ARRAY(m_piPrereqOrBonuses);
			}
		}

		GETXML->SetToParent(pXML->GetXML());
	}
/************************************************************************************************/
/* XMLCOPY                                 10/08/07                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	else
	{
		SAFE_DELETE_ARRAY(m_piPrereqOrBonuses);
	}
/************************************************************************************************/
/* XMLCOPY                                 END                                                  */
/************************************************************************************************/

	pXML->SetVariableListTagPair(&m_piProductionTraits, "ProductionTraits", sizeof(GC.getTraitInfo((TraitTypes)0)), GC.getNumTraitInfos());

	pXML->SetVariableListTagPair(&m_piFlavorValue, "Flavors", GC.getFlavorTypes(), GC.getNumFlavorTypes());

	pXML->GetChildXmlValByName(&m_iAIWeight, "iAIWeight");
	pXML->GetChildXmlValByName(&m_iProductionCost, "iCost");
	pXML->GetChildXmlValByName(&m_iHurryCostModifier, "iHurryCostModifier");
	pXML->GetChildXmlValByName(&m_iAdvancedStartCost, "iAdvancedStartCost");
	pXML->GetChildXmlValByName(&m_iAdvancedStartCostIncrease, "iAdvancedStartCostIncrease");
	pXML->GetChildXmlValByName(&m_iMinAreaSize, "iMinAreaSize");
	pXML->GetChildXmlValByName(&m_iMoves, "iMoves");
	pXML->GetChildXmlValByName(&m_iAirRange, "iAirRange");
	pXML->GetChildXmlValByName(&m_iAirUnitCap, "iAirUnitCap");
	pXML->GetChildXmlValByName(&m_iDropRange, "iDropRange");
	pXML->GetChildXmlValByName(&m_iNukeRange, "iNukeRange");
	pXML->GetChildXmlValByName(&m_iWorkRate, "iWorkRate");
	pXML->GetChildXmlValByName(&m_iBaseDiscover, "iBaseDiscover");
	pXML->GetChildXmlValByName(&m_iDiscoverMultiplier, "iDiscoverMultiplier");
	pXML->GetChildXmlValByName(&m_iBaseHurry, "iBaseHurry");
	pXML->GetChildXmlValByName(&m_iHurryMultiplier, "iHurryMultiplier");
	pXML->GetChildXmlValByName(&m_iBaseTrade, "iBaseTrade");
	pXML->GetChildXmlValByName(&m_iTradeMultiplier, "iTradeMultiplier");
	pXML->GetChildXmlValByName(&m_iGreatWorkCulture, "iGreatWorkCulture");
	pXML->GetChildXmlValByName(&m_iEspionagePoints, "iEspionagePoints");

	pXML->SetVariableListTagPair(&m_pbTerrainImpassable, "TerrainImpassables", sizeof(GC.getTerrainInfo((TerrainTypes)0)), GC.getNumTerrainInfos(), false);
	pXML->SetVariableListTagPair(&m_pbFeatureImpassable, "FeatureImpassables", sizeof(GC.getFeatureInfo((FeatureTypes)0)), GC.getNumFeatureInfos(), false);

	pXML->GetChildXmlValByName(&m_iCombat, "iCombat");
	pXML->GetChildXmlValByName(&m_iCombatLimit, "iCombatLimit");
	pXML->GetChildXmlValByName(&m_iAirCombat, "iAirCombat");
	pXML->GetChildXmlValByName(&m_iAirCombatLimit, "iAirCombatLimit");
	pXML->GetChildXmlValByName(&m_iXPValueAttack, "iXPValueAttack");
	pXML->GetChildXmlValByName(&m_iXPValueDefense, "iXPValueDefense");
	pXML->GetChildXmlValByName(&m_iFirstStrikes, "iFirstStrikes");
	pXML->GetChildXmlValByName(&m_iChanceFirstStrikes, "iChanceFirstStrikes");
	pXML->GetChildXmlValByName(&m_iInterceptionProbability, "iInterceptionProbability");
	//ls612: Advanced Nuke Interception
	//pXML->GetChildXmlValByName(&m_iNukeInterceptionProbability, "iNukeInterceptionProbability");
	//pXML->GetChildXmlValByName(&m_iNukeInterceptionRange, "iNukeInterceptionRange");
	pXML->GetChildXmlValByName(&m_iEvasionProbability, "iEvasionProbability");
	pXML->GetChildXmlValByName(&m_iWithdrawalProbability, "iWithdrawalProb");
	pXML->GetChildXmlValByName(&m_iCollateralDamage, "iCollateralDamage");
	pXML->GetChildXmlValByName(&m_iCollateralDamageLimit, "iCollateralDamageLimit");
	pXML->GetChildXmlValByName(&m_iCollateralDamageMaxUnits, "iCollateralDamageMaxUnits");
	pXML->GetChildXmlValByName(&m_iCityAttackModifier, "iCityAttack");
	pXML->GetChildXmlValByName(&m_iCityDefenseModifier, "iCityDefense");
	pXML->GetChildXmlValByName(&m_iAnimalCombatModifier, "iAnimalCombat");
	pXML->GetChildXmlValByName(&m_iHillsAttackModifier, "iHillsAttack");
	pXML->GetChildXmlValByName(&m_iHillsDefenseModifier, "iHillsDefense");

	pXML->SetVariableListTagPair(&m_pbTerrainNative, "TerrainNatives", sizeof(GC.getTerrainInfo((TerrainTypes)0)), GC.getNumTerrainInfos());
	pXML->SetVariableListTagPair(&m_pbFeatureNative, "FeatureNatives", sizeof(GC.getFeatureInfo((FeatureTypes)0)), GC.getNumFeatureInfos());

	pXML->SetVariableListTagPair(&m_piTerrainAttackModifier, "TerrainAttacks", sizeof(GC.getTerrainInfo((TerrainTypes)0)), GC.getNumTerrainInfos());
	pXML->SetVariableListTagPair(&m_piTerrainDefenseModifier, "TerrainDefenses", sizeof(GC.getTerrainInfo((TerrainTypes)0)), GC.getNumTerrainInfos());
	pXML->SetVariableListTagPair(&m_piFeatureAttackModifier, "FeatureAttacks", sizeof(GC.getFeatureInfo((FeatureTypes)0)), GC.getNumFeatureInfos());
	pXML->SetVariableListTagPair(&m_piFeatureDefenseModifier, "FeatureDefenses", sizeof(GC.getFeatureInfo((FeatureTypes)0)), GC.getNumFeatureInfos());

	pXML->SetVariableListTagPair(&m_piUnitClassAttackModifier, "UnitClassAttackMods", sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());
	pXML->SetVariableListTagPair(&m_piUnitClassDefenseModifier, "UnitClassDefenseMods", sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());

	pXML->SetVariableListTagPair(&m_piUnitCombatModifier, "UnitCombatMods", sizeof(GC.getUnitCombatInfo((UnitCombatTypes)0)), GC.getNumUnitCombatInfos());
	pXML->SetVariableListTagPair(&m_piUnitCombatCollateralImmune, "UnitCombatCollateralImmunes", sizeof(GC.getUnitCombatInfo((UnitCombatTypes)0)), GC.getNumUnitCombatInfos());
	pXML->SetVariableListTagPair(&m_piDomainModifier, "DomainMods", sizeof(GC.getDomainInfo((DomainTypes)0)), NUM_DOMAIN_TYPES);

	pXML->SetVariableListTagPair(&m_piBonusProductionModifier, "BonusProductionModifiers", sizeof(GC.getBonusInfo((BonusTypes)0)), GC.getNumBonusInfos());

	pXML->GetChildXmlValByName(&m_iBombRate, "iBombRate");
	pXML->GetChildXmlValByName(&m_iBombardRate, "iBombardRate");

// Thomas SG - AC: Advanced Cargo START
//	pXML->GetChildXmlValByName(szTextVal, "SpecialCargo");
//	m_iSpecialCargo = pXML->FindInInfoClass(szTextVal);
// Thomas SG - AC: Advanced Cargo END

	pXML->GetChildXmlValByName(szTextVal, "DomainCargo");
	m_iDomainCargo = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(&m_iCargoSpace, "iCargo");
// Thomas SG - AC: Advanced Cargo START
	pXML->GetChildXmlValByName(&m_iTotalCargoSpace, "iTotalCargo");
// Thomas SG - AC: Advanced Cargo END	
	pXML->GetChildXmlValByName(&m_iConscriptionValue, "iConscription");
	pXML->GetChildXmlValByName(&m_iCultureGarrisonValue, "iCultureGarrison");
	pXML->GetChildXmlValByName(&m_iExtraCost, "iExtraCost");
	pXML->GetChildXmlValByName(&m_iAssetValue, "iAsset");
	pXML->GetChildXmlValByName(&m_iPowerValue, "iPower");

	// Read the mesh groups elements
	if ( GETXML->SetToChildByTagName(pXML->GetXML(),"UnitMeshGroups") )
	{
		pXML->GetChildXmlValByName( &m_iGroupSize, "iGroupSize");
		m_iGroupDefinitions = iIndexVal = GETXML->NumOfChildrenByTagName(pXML->GetXML(), "UnitMeshGroup");
		m_piUnitGroupRequired = new int[ iIndexVal ];
		pXML->GetChildXmlValByName( &m_iUnitMeleeWaveSize, "iMeleeWaveSize" );
		pXML->GetChildXmlValByName( &m_iUnitRangedWaveSize, "iRangedWaveSize" );
		pXML->GetChildXmlValByName( &m_fUnitMaxSpeed, "fMaxSpeed");
		pXML->GetChildXmlValByName( &m_fUnitPadTime, "fPadTime");
/************************************************************************************************/
/* Afforess	                  Start		 03/10/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		m_paszEarlyArtDefineTags = new CvString[ iIndexVal ];
		m_paszClassicalArtDefineTags = new CvString[ iIndexVal ];
		m_paszMiddleArtDefineTags = new CvString[ iIndexVal ];
		m_paszRennArtDefineTags = new CvString[ iIndexVal ];
		m_paszIndustrialArtDefineTags = new CvString[ iIndexVal ];
		m_paszLateArtDefineTags = new CvString[ iIndexVal ];
		m_paszFutureArtDefineTags = new CvString[ iIndexVal ];
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

		if (GETXML->SetToChildByTagName(pXML->GetXML(), "UnitMeshGroup"))
		{
			for ( k = 0; k < iIndexVal; k++ )
			{
/************************************************************************************************/
/* Afforess	                  Start		 03/10/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
				pXML->GetChildXmlValByName( &m_piUnitGroupRequired[k], "iRequired");
				pXML->GetChildXmlValByName(szTextVal, "EarlyArtDefineTag");
				setEarlyArtDefineTag(k, szTextVal);
				pXML->GetChildXmlValByName(szTextVal, "ClassicalArtDefineTag");
				setClassicalArtDefineTag(k, szTextVal);
				pXML->GetChildXmlValByName(szTextVal, "MiddleArtDefineTag");
				setMiddleArtDefineTag(k, szTextVal);
				pXML->GetChildXmlValByName(szTextVal, "RennArtDefineTag");
				setRennArtDefineTag(k, szTextVal);
				pXML->GetChildXmlValByName(szTextVal, "IndustrialArtDefineTag");
				setIndustrialArtDefineTag(k, szTextVal);
				pXML->GetChildXmlValByName(szTextVal, "LateArtDefineTag");
				setLateArtDefineTag(k, szTextVal);
				pXML->GetChildXmlValByName(szTextVal, "FutureArtDefineTag");
				setFutureArtDefineTag(k, szTextVal);
				GETXML->NextSibling(pXML->GetXML());
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			}
			GETXML->SetToParent(pXML->GetXML());
		}
		GETXML->SetToParent(pXML->GetXML());
	}

	pXML->GetChildXmlValByName(m_szFormationType, "FormationType");

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"UniqueNames"))
	{
		pXML->SetStringList(&m_paszUnitNames, &m_iNumUnitNames);
		GETXML->SetToParent(pXML->GetXML());
	}

	pXML->SetVariableListTagPair(&m_pbFreePromotions, "FreePromotions", sizeof(GC.getPromotionInfo((PromotionTypes)0)), GC.getNumPromotionInfos());

	pXML->GetChildXmlValByName(szTextVal, "LeaderPromotion");
	m_iLeaderPromotion = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(&m_iLeaderExperience, "iLeaderExperience");

	// Toffer - New ranged combat
	pXML->GetChildXmlValByName(&m_iVolleyRange, "iVolleyRange");
	pXML->GetChildXmlValByName(&m_iVolleyAccuracy, "iVolleyAccuracy");
	pXML->GetChildXmlValByName(&m_iVolleyAccuracyMin, "iVolleyAccuracyMin");
	pXML->GetChildXmlValByName(&m_iVolleyEfficiency, "iVolleyEfficiency");
	pXML->GetChildXmlValByName(&m_iVolleyRounds, "iVolleyRounds");
	if ( m_iVolleyRounds < 1 ) m_iVolleyRounds = 1;
	// ! Toffer

	// Dale - AB: Bombing START
	pXML->GetChildXmlValByName(&m_bDCMAirBomb1, "bDCMAirBomb1");
	pXML->GetChildXmlValByName(&m_bDCMAirBomb2, "bDCMAirBomb2");
	pXML->GetChildXmlValByName(&m_bDCMAirBomb3, "bDCMAirBomb3");
	pXML->GetChildXmlValByName(&m_bDCMAirBomb4, "bDCMAirBomb4");
	pXML->GetChildXmlValByName(&m_bDCMAirBomb5, "bDCMAirBomb5");
	// Dale - AB: Bombing END
	// Dale - FE: Fighters START
	pXML->GetChildXmlValByName(&m_bDCMFighterEngage, "bDCMFighterEngage");
	// Dale - FE: Fighters END

/************************************************************************************************/
/* Afforess	                  Start		 Last Update: 3/8/10                                    */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	pXML->GetChildXmlValByName(&m_bWorkerTrade, "bWorkerTrade", false);
	pXML->GetChildXmlValByName(&m_bMilitaryTrade, "bMilitaryTrade", false);
	pXML->GetChildXmlValByName(&m_bForceUpgrade, "bForceUpgrade", false);
	pXML->GetChildXmlValByName(&m_bGreatGeneral, "bGreatGeneral", false);
	pXML->GetChildXmlValByName(&m_bSlave, "bSlave", false);
	pXML->GetChildXmlValByName(&m_bRequiresStateReligionInCity,"bRequiresStateReligionInCity",false);
	pXML->GetChildXmlValByName(szTextVal, "VicinityBonusType");
	m_iPrereqVicinityBonus = pXML->FindInInfoClass(szTextVal);
	pXML->SetVariableListTagPair(&m_pbPassableRouteNeeded, "PassableRouteNeededs", sizeof(GC.getRouteInfo((RouteTypes)0)), GC.getNumRouteInfos(), false);
	pXML->SetVariableListTagPair(&m_pbPrereqBuildingClass, "PrereqBuildingClasses", sizeof(GC.getBuildingClassInfo((BuildingClassTypes)0)), GC.getNumBuildingClassInfos());
	pXML->GetChildXmlValByName(&m_iBaseFoodChange, "iBaseFoodChange");
	pXML->GetChildXmlValByName(&m_iControlPoints, "iControlPoints");
	pXML->GetChildXmlValByName(&m_iCommandRange, "iCommandRange");
	
//	pXML->SetVariableListTagPair(&m_paszCivilizationNames, "", sizeof(GC.getCivilizationInfo((CivilizationTypes)0)), GC.getNumCivilizationInfos(), "");


	if (GETXML->SetToChildByTagName(pXML->GetXML(),"CivilizationNames"))
	{
		if (pXML->SkipToNextVal())
		{
			int iTemp = 0;
			CvString szTemp;
			int iNumSibs = GETXML->GetNumChildren(pXML->GetXML());
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				if (iNumSibs > 0)
				{
					for (int i=0;i<iNumSibs;i++)
					{
						if (pXML->GetChildXmlVal(szTextVal))
						{
							m_aszCivilizationNamesforPass3.push_back(szTextVal);
							pXML->GetNextXmlVal(szTemp);
							m_aszCivilizationNamesValueforPass3.push_back(szTemp);
							
							GETXML->SetToParent(pXML->GetXML());
						}

						if (!GETXML->NextSibling(pXML->GetXML()))
						{
							break;
						}
					}

					GETXML->SetToParent(pXML->GetXML());
				}
			}
		}

		GETXML->SetToParent(pXML->GetXML());
	}
	
	if (GETXML->SetToChildByTagName(pXML->GetXML(),"PrereqVicinityBonuses"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = GETXML->GetNumChildren(pXML->GetXML());
			FAssertMsg((0 < GC.getNUM_UNIT_PREREQ_OR_BONUSES()),"Allocating zero or less memory in SetGlobalUnitInfo");

			if (0 < iNumSibs)
			{
				pXML->CvXMLLoadUtility::InitList(&m_piPrereqOrVicinityBonuses, GC.getNUM_UNIT_PREREQ_OR_BONUSES(), -1);
				if (pXML->GetChildXmlVal(szTextVal))
				{
					FAssertMsg((iNumSibs <= GC.getNUM_UNIT_PREREQ_OR_BONUSES()) , "There are more siblings than memory allocated for them in SetGlobalUnitInfo");
					for (j=0;j<iNumSibs;j++)
					{
						m_piPrereqOrVicinityBonuses[j] = pXML->FindInInfoClass(szTextVal);
						if (!pXML->GetNextXmlVal(szTextVal))
						{
							break;
						}
					}

					GETXML->SetToParent(pXML->GetXML());
				}
			}
			else
			{
				SAFE_DELETE_ARRAY(m_piPrereqOrVicinityBonuses);
			}
		}

		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_piPrereqOrVicinityBonuses);
	}

/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	m_PropertyManipulators.read(pXML);

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"TrainCondition"))
	{
		m_pExprTrainCondition = BoolExpr::read(pXML);
		GETXML->SetToParent(pXML->GetXML());
	}

/************************************************************************************************/
/* XMLCOPY                                 10/10/07                                MRGENIE      */
/*                                                                                              */
/* if we're using a module, assuming the meshgroup isn't set, we update first after             */
/* the copyNonDefaults method                                                                   */
/************************************************************************************************/
/*  
	updateArtDefineButton();
*/
	if (m_iGroupSize != 0)
	{
		updateArtDefineButton();
	}
/************************************************************************************************/
/* XMLCOPY                                 END                                                  */
/************************************************************************************************/

	//	Auto-correct units that are animals but don't have the naimal AI set as their default
	//	(this situation currently confuses the AI so disallow it)
	//TB Animal Search note: THIS will have to be unwritted to allow for multiple animal AI types!!!  leaving alone for now but WILL be removed eventually
	if ( isAnimal() && getDefaultUnitAIType() != UNITAI_ANIMAL )
	{
		FAssertMsg(false, CvString::format("Auto-corrected: UnitAI not set to UNITAI_ANIMAL for animal unit %s", m_szType.c_str()).c_str());
		m_iDefaultUnitAIType = UNITAI_ANIMAL;
	}

	return true;
}

/************************************************************************************************/
/* XMLCOPY                                 10/08/07                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
void CvUnitInfo::copyNonDefaults(CvUnitInfo* pClassInfo, CvXMLLoadUtility* pXML)
{	
	CvHotkeyInfo::copyNonDefaults(pClassInfo, pXML);

	bool bDefault = false;
	int iDefault = 0;
	int iTextDefault = -1;
	CvString cDefault = CvString::format("").GetCString();
	
	if ( m_iUnitClassType == iTextDefault )	m_iUnitClassType = pClassInfo->getUnitClassType();
	//if ( m_iSpecialUnitType == iTextDefault )	m_iSpecialUnitType = pClassInfo->getSpecialUnitType(); revomed by 45deg for Advanced Cargo Mod
	if ( m_iUnitCaptureClassType == iTextDefault )	m_iUnitCaptureClassType = pClassInfo->getUnitCaptureClassType();
	if ( m_iUnitCombatType == iTextDefault )	m_iUnitCombatType = pClassInfo->getUnitCombatType();
	if ( m_iDomainType == iTextDefault )	m_iDomainType = pClassInfo->getDomainType();
	if ( m_iDefaultUnitAIType == iTextDefault )	m_iDefaultUnitAIType = pClassInfo->getDefaultUnitAIType();
	if ( m_iInvisibleType == iTextDefault )	m_iInvisibleType = pClassInfo->getInvisibleType();

	if ( getNumSeeInvisibleTypes() < 1 )
	{
		for(int i=0; i < pClassInfo->getNumSeeInvisibleTypes(); i++)
		{
			int iSeeInvisibleType = pClassInfo->getSeeInvisibleType(i);
			if(iSeeInvisibleType != NO_INVISIBLE)
			{
				m_aiSeeInvisibleTypes.push_back(iSeeInvisibleType);
			}
		}
	}
	if ( m_iAdvisorType == iTextDefault )	m_iAdvisorType = pClassInfo->getAdvisorType();
	
	if ( m_bAnimal == bDefault )	m_bAnimal = pClassInfo->isAnimal() ;
	if ( m_bFoodProduction == bDefault )	m_bFoodProduction = pClassInfo->isFoodProduction();
	if ( m_bNoBadGoodies == bDefault )	m_bNoBadGoodies = pClassInfo->isNoBadGoodies();
	if ( m_bOnlyDefensive == bDefault )	m_bOnlyDefensive = pClassInfo->isOnlyDefensive();
	if ( m_bNoCapture == bDefault )	m_bNoCapture = pClassInfo->isNoCapture();
	if ( m_bQuickCombat == bDefault )	m_bQuickCombat = pClassInfo->isQuickCombat();
	if ( m_bRivalTerritory == bDefault )	m_bRivalTerritory = pClassInfo->isRivalTerritory();
	if ( m_bMilitaryHappiness == bDefault )	m_bMilitaryHappiness = pClassInfo->isMilitaryHappiness();
	if ( m_bMilitarySupport == bDefault )	m_bMilitarySupport = pClassInfo->isMilitarySupport();
	if ( m_bMilitaryProduction == bDefault )	m_bMilitaryProduction = pClassInfo->isMilitaryProduction();
	if ( m_bPillage == bDefault )	m_bPillage = pClassInfo->isPillage();
	if ( m_bSpy == bDefault )	m_bSpy = pClassInfo->isSpy();
	if ( m_bSabotage == bDefault )	m_bSabotage = pClassInfo->isSabotage();
	if ( m_bDestroy == bDefault )	m_bDestroy = pClassInfo->isDestroy();
	if ( m_bStealPlans == bDefault )	m_bStealPlans = pClassInfo->isStealPlans();
	if ( m_bInvestigate == bDefault )	m_bInvestigate = pClassInfo->isInvestigate();
	if ( m_bCounterSpy == bDefault )	m_bCounterSpy = pClassInfo->isCounterSpy();
	if ( m_bFound == bDefault )	m_bFound = pClassInfo->isFound();	
	if ( m_bGoldenAge == bDefault )	m_bGoldenAge = pClassInfo->isGoldenAge();
	if ( m_bInvisible == bDefault )	m_bInvisible = pClassInfo->isInvisible();
	if ( m_bFirstStrikeImmune == bDefault )	m_bFirstStrikeImmune = pClassInfo->isFirstStrikeImmune();
	if ( m_bNoDefensiveBonus == bDefault )	m_bNoDefensiveBonus = pClassInfo->isNoDefensiveBonus();
	if ( m_bIgnoreBuildingDefense == bDefault )	m_bIgnoreBuildingDefense = pClassInfo->isIgnoreBuildingDefense();
	if ( m_bCanMoveImpassable == bDefault )	m_bCanMoveImpassable = pClassInfo->isCanMoveImpassable();
	if ( m_bCanMoveAllTerrain == bDefault )	m_bCanMoveAllTerrain = pClassInfo->isCanMoveAllTerrain();
	if ( m_bFlatMovementCost == bDefault )	m_bFlatMovementCost = pClassInfo->isFlatMovementCost();
	if ( m_bIgnoreTerrainCost == bDefault )	m_bIgnoreTerrainCost = pClassInfo->isIgnoreTerrainCost();
	if ( m_bNukeImmune == bDefault )	m_bNukeImmune = pClassInfo->isNukeImmune();
	if ( m_bPrereqBonuses == bDefault )	m_bPrereqBonuses = pClassInfo->isPrereqBonuses();
	if ( m_bPrereqReligion == bDefault )	m_bPrereqReligion = pClassInfo->isPrereqReligion();
	if ( m_bMechanized == bDefault )	m_bMechanized = pClassInfo->isMechUnit();
	if ( m_bRenderBelowWater == bDefault )	m_bRenderBelowWater = pClassInfo->isRenderBelowWater();
	if ( m_bRenderAlways == bDefault )	m_bRenderAlways = pClassInfo->isRenderAlways();
	if ( m_bSuicide == bDefault )	m_bSuicide = pClassInfo->isSuicide();
	if ( m_bLineOfSight == bDefault )	m_bLineOfSight = pClassInfo->isLineOfSight();
	if ( m_bHiddenNationality == bDefault )	m_bHiddenNationality = pClassInfo->isHiddenNationality();
	if ( m_bAlwaysHostile == bDefault )	m_bAlwaysHostile = pClassInfo->isAlwaysHostile();
	if ( m_bNoRevealMap == bDefault )	m_bNoRevealMap = pClassInfo->isNoRevealMap();

/********************************************************************************/
/**		REVDCM_OC								2/16/10				phungus420	*/
/**																				*/
/**		 																		*/
/********************************************************************************/
	if ( m_bInquisitor == bDefault )	m_bInquisitor = pClassInfo->isInquisitor();
/********************************************************************************/
/**		REVDCM_OC								END								*/
/********************************************************************************/
	//ls612: Can't enter non-Owned cities
	if ( m_bNoNonOwnedEntry == bDefault ) m_bNoNonOwnedEntry = pClassInfo->isNoNonOwnedEntry();

	for ( int i = 0; i < GC.getNumUnitClassInfos(); i++)
	{
		if ( getUpgradeUnitClass(i) == bDefault && pClassInfo->getUpgradeUnitClass(i) != bDefault)
		{
			if ( m_pbUpgradeUnitClass == NULL )
			{
				CvXMLLoadUtility::InitList(&m_pbUpgradeUnitClass,GC.getNumUnitClassInfos(),bDefault);
			}
			m_pbUpgradeUnitClass[i] = pClassInfo->getUpgradeUnitClass(i);
		}
		if ( getTargetUnitClass(i) == bDefault && pClassInfo->getTargetUnitClass(i) != bDefault)
		{
			if ( m_pbTargetUnitClass == NULL )
			{
				CvXMLLoadUtility::InitList(&m_pbTargetUnitClass,GC.getNumUnitClassInfos(),bDefault);
			}
			m_pbTargetUnitClass[i] = pClassInfo->getTargetUnitClass(i);
		}
		if ( getDefenderUnitClass(i) == bDefault && pClassInfo->getDefenderUnitClass(i) != bDefault)
		{
			if ( m_pbDefenderUnitClass == NULL )
			{
				CvXMLLoadUtility::InitList(&m_pbDefenderUnitClass,GC.getNumUnitClassInfos(),bDefault);
			}
			m_pbDefenderUnitClass[i] = pClassInfo->getDefenderUnitClass(i);
		}
		if ( getFlankingStrikeUnitClass(i) == -1 && pClassInfo->getFlankingStrikeUnitClass(i) != -1)
		{
			if ( m_piFlankingStrikeUnitClass == NULL )
			{
				CvXMLLoadUtility::InitList(&m_piFlankingStrikeUnitClass,GC.getNumUnitClassInfos(),-1);
			}
			m_piFlankingStrikeUnitClass[i] = pClassInfo->getFlankingStrikeUnitClass(i);
		}
		if ( getUnitClassAttackModifier(i) == iDefault && pClassInfo->getUnitClassAttackModifier(i) != iDefault)
		{
			if ( m_piUnitClassAttackModifier == NULL )
			{
				CvXMLLoadUtility::InitList(&m_piUnitClassAttackModifier,GC.getNumUnitClassInfos(),iDefault);
			}
			m_piUnitClassAttackModifier[i] = pClassInfo->getUnitClassAttackModifier(i);
		}
		if ( getUnitClassDefenseModifier(i) == iDefault && pClassInfo->getUnitClassDefenseModifier(i) != iDefault)
		{
			if ( m_piUnitClassDefenseModifier == NULL )
			{
				CvXMLLoadUtility::InitList(&m_piUnitClassDefenseModifier,GC.getNumUnitClassInfos(),iDefault);
			}
			m_piUnitClassDefenseModifier[i] = pClassInfo->getUnitClassDefenseModifier(i);
		}

	}

	for ( int i = 0; i < GC.getNumUnitCombatInfos(); i++)
	{
		if ( getTargetUnitCombat(i) == bDefault && pClassInfo->getTargetUnitCombat(i) != bDefault)
		{
			if ( NULL == m_pbTargetUnitCombat )
			{
				CvXMLLoadUtility::InitList(&m_pbTargetUnitCombat,GC.getNumUnitCombatInfos(),bDefault);
			}
			m_pbTargetUnitCombat[i] = pClassInfo->getTargetUnitCombat(i);
		}
		if ( getDefenderUnitCombat(i) == bDefault && pClassInfo->getDefenderUnitCombat(i) != bDefault)
		{
			if ( NULL == m_pbDefenderUnitCombat )
			{
				CvXMLLoadUtility::InitList(&m_pbDefenderUnitCombat,GC.getNumUnitCombatInfos(),bDefault);
			}
			m_pbDefenderUnitCombat[i] = pClassInfo->getDefenderUnitCombat(i);
		}
		if ( getUnitCombatModifier(i) == iDefault && pClassInfo->getUnitCombatModifier(i) != iDefault )
		{
			if ( NULL == m_piUnitCombatModifier )
			{
				CvXMLLoadUtility::InitList(&m_piUnitCombatModifier,GC.getNumUnitCombatInfos(),iDefault);
			}
			m_piUnitCombatModifier[i] = pClassInfo->getUnitCombatModifier(i);
		}
		if ( getUnitCombatCollateralImmune(i) == iDefault && pClassInfo->getUnitCombatCollateralImmune(i) != iDefault)
		{
			if ( NULL == m_piUnitCombatCollateralImmune )
			{
				CvXMLLoadUtility::InitList(&m_piUnitCombatCollateralImmune,GC.getNumUnitCombatInfos(),iDefault);
			}
			m_piUnitCombatCollateralImmune[i] = pClassInfo->getUnitCombatCollateralImmune(i);
		}
	}

	for ( int i = 0; i < NUM_UNITAI_TYPES; i++)
	{
		if ( getUnitAIType(i) == bDefault && pClassInfo->getUnitAIType(i) != bDefault)
		{
			if ( NULL == m_pbUnitAIType )
			{
				CvXMLLoadUtility::InitList(&m_pbUnitAIType,NUM_UNITAI_TYPES,bDefault);
			}
			m_pbUnitAIType[i] = pClassInfo->getUnitAIType(i);
		}
		if ( getNotUnitAIType(i) == bDefault && pClassInfo->getNotUnitAIType(i) != bDefault)
		{
			if ( NULL == m_pbNotUnitAIType )
			{
				CvXMLLoadUtility::InitList(&m_pbNotUnitAIType,NUM_UNITAI_TYPES,bDefault);
			}
			m_pbNotUnitAIType[i] = pClassInfo->getNotUnitAIType(i);
		}
	}

	for ( int i = 0; i < GC.getNumBuildInfos(); i++)
	{
		if ( getBuilds(i) == bDefault && pClassInfo->getBuilds(i) != bDefault)
		{
			if ( NULL == m_pbBuilds )
			{
				CvXMLLoadUtility::InitList(&m_pbBuilds,GC.getNumBuildInfos(),bDefault);
			}
			m_pbBuilds[i] = pClassInfo->getBuilds(i);
		}
	}

	for ( int i = 0; i < GC.getNumBuildingInfos(); i++)
	{
		if ( getBuildings(i) == bDefault && pClassInfo->getBuildings(i) != bDefault)
		{
			if ( NULL == m_pbBuildings )
			{
				CvXMLLoadUtility::InitList(&m_pbBuildings,GC.getNumBuildingInfos(),bDefault);
			}
			m_pbBuildings[i] = pClassInfo->getBuildings(i);
		}
		if ( getForceBuildings(i) == bDefault && pClassInfo->getForceBuildings(i) != bDefault)
		{
			if ( NULL == m_pbForceBuildings )
			{
				CvXMLLoadUtility::InitList(&m_pbForceBuildings,GC.getNumBuildingInfos(),bDefault);
			}
			m_pbForceBuildings[i] = pClassInfo->getForceBuildings(i);
		}
	}

	for ( int i = 0; i < GC.getNumReligionInfos(); i++)
	{
		if ( getReligionSpreads(i) == -1 && pClassInfo->getReligionSpreads(i) != -1)
		{
			if ( NULL == m_piReligionSpreads )
			{
				CvXMLLoadUtility::InitList(&m_piReligionSpreads,GC.getNumReligionInfos(),-1);
			}
			m_piReligionSpreads[i] = pClassInfo->getReligionSpreads(i);
		}
	}

	for ( int i = 0; i < GC.getNumCorporationInfos(); i++)
	{
		if ( getCorporationSpreads(i) == -1 && pClassInfo->getCorporationSpreads(i) != -1 )
		{
			if ( NULL == m_piReligionSpreads )
			{
				CvXMLLoadUtility::InitList(&m_piCorporationSpreads,GC.getNumCorporationInfos(),-1);
			}
			m_piCorporationSpreads[i] = pClassInfo->getCorporationSpreads(i);
		}
	}

	for ( int i = 0; i < GC.getNumTraitInfos(); i++)
	{
		if ( getProductionTraits(i) == iDefault && pClassInfo->getProductionTraits(i) != iDefault)
		{
			if ( NULL == m_piProductionTraits )
			{
				CvXMLLoadUtility::InitList(&m_piProductionTraits,GC.getNumTraitInfos(),iDefault);
			}
			m_piProductionTraits[i] = pClassInfo->getProductionTraits(i);
		}
	}

	for ( int i = 0; i < GC.getNumFlavorTypes(); i++)
	{
		if ( getFlavorValue(i) == iDefault && pClassInfo->getFlavorValue(i) != iDefault)
		{
			if ( NULL == m_piFlavorValue )
			{
				CvXMLLoadUtility::InitList(&m_piFlavorValue,GC.getNumFlavorTypes(),iDefault);
			}
			m_piFlavorValue[i] = pClassInfo->getFlavorValue(i);
		}
	}

	for ( int i = 0; i < GC.getNumSpecialistInfos(); i++)
	{
		if ( getGreatPeoples(i) == bDefault && pClassInfo->getGreatPeoples(i) != bDefault)
		{
			if ( NULL == m_pbGreatPeoples )
			{
				CvXMLLoadUtility::InitList(&m_pbGreatPeoples,GC.getNumSpecialistInfos(),bDefault);
			}
			m_pbGreatPeoples[i] = pClassInfo->getGreatPeoples(i);
		}
	}

	for ( int i = 0; i < GC.getNumTerrainInfos(); i++)
	{
		if ( getTerrainImpassable(i) == bDefault && pClassInfo->getTerrainImpassable(i) != bDefault)
		{
			if ( NULL == m_pbTerrainImpassable )
			{
				CvXMLLoadUtility::InitList(&m_pbTerrainImpassable,GC.getNumTerrainInfos(),bDefault);
			}
			m_pbTerrainImpassable[i] = pClassInfo->getTerrainImpassable(i);
		}		
		if ( getTerrainNative(i) == bDefault && pClassInfo->getTerrainNative(i) != bDefault)
		{
			if ( NULL == m_pbTerrainNative )
			{
				CvXMLLoadUtility::InitList(&m_pbTerrainNative,GC.getNumTerrainInfos(),bDefault);
			}
			m_pbTerrainNative[i] = pClassInfo->getTerrainNative(i);
		}		
		if ( getTerrainAttackModifier(i) == iDefault && pClassInfo->getTerrainAttackModifier(i) != iDefault)
		{
			if ( NULL == m_piTerrainAttackModifier )
			{
				CvXMLLoadUtility::InitList(&m_piTerrainAttackModifier,GC.getNumTerrainInfos(),iDefault);
			}
			m_piTerrainAttackModifier[i] = pClassInfo->getTerrainAttackModifier(i);
		}
		if ( getTerrainDefenseModifier(i) == iDefault && pClassInfo->getTerrainDefenseModifier(i) != iDefault)
		{
			if ( NULL == m_piTerrainDefenseModifier )
			{
				CvXMLLoadUtility::InitList(&m_piTerrainDefenseModifier,GC.getNumTerrainInfos(),iDefault);
			}
			m_piTerrainDefenseModifier[i] = pClassInfo->getTerrainDefenseModifier(i);
		}
		if ( getTerrainPassableTech(i) == NO_TECH && pClassInfo->getTerrainPassableTech(i) != NO_TECH )
		{
			if ( NULL == m_piTerrainPassableTech )
			{
				CvXMLLoadUtility::InitList(&m_piTerrainPassableTech,GC.getNumTerrainInfos(),(int)NO_TECH);
			}
			m_piTerrainPassableTech[i] = pClassInfo->getTerrainPassableTech(i);
		}
	}

	for ( int i = 0; i < GC.getNumFeatureInfos(); i++)
	{
		if ( getFeatureImpassable(i) == bDefault && pClassInfo->getFeatureImpassable(i) != bDefault)
		{
			if (m_pbFeatureImpassable == NULL)
			{
				CvXMLLoadUtility::InitList(&m_pbFeatureImpassable,GC.getNumFeatureInfos(),bDefault);
			}
			m_pbFeatureImpassable[i] = pClassInfo->getFeatureImpassable(i);
		}
		if ( getFeatureNative(i) == bDefault && pClassInfo->getFeatureNative(i) != bDefault)
		{
			if (m_pbFeatureNative == NULL)
			{
				CvXMLLoadUtility::InitList(&m_pbFeatureNative,GC.getNumFeatureInfos(),bDefault);
			}
			m_pbFeatureNative[i] = pClassInfo->getFeatureNative(i);
		}
		if ( getFeatureAttackModifier(i) == iDefault && pClassInfo->getFeatureAttackModifier(i) != iDefault)
		{
			if (m_piFeatureAttackModifier == NULL)
			{
				CvXMLLoadUtility::InitList(&m_piFeatureAttackModifier,GC.getNumFeatureInfos(),iDefault);
			}
			m_piFeatureAttackModifier[i] = pClassInfo->getFeatureAttackModifier(i);
		}
		if ( getFeatureDefenseModifier(i) == iDefault && pClassInfo->getFeatureDefenseModifier(i) != iDefault)
		{
			if (m_piFeatureDefenseModifier == NULL)
			{
				CvXMLLoadUtility::InitList(&m_piFeatureDefenseModifier,GC.getNumFeatureInfos(),iDefault);
			}
			m_piFeatureDefenseModifier[i] = pClassInfo->getFeatureDefenseModifier(i);
		}		
		if ( getFeaturePassableTech(i) == NO_TECH && pClassInfo->getFeaturePassableTech(i) != NO_TECH )
		{
			if ( NULL == m_piFeaturePassableTech )
			{
				CvXMLLoadUtility::InitList(&m_piFeaturePassableTech,GC.getNumFeatureInfos(),(int)NO_TECH);
			}
			m_piFeaturePassableTech[i] = pClassInfo->getFeaturePassableTech(i);
		}
	}

	for ( int i = 0; i < NUM_DOMAIN_TYPES; i++)
	{
		if ( getDomainModifier(i) == iDefault && pClassInfo->getDomainModifier(i) != iDefault)
		{
			if ( NULL == m_piDomainModifier )
			{
				CvXMLLoadUtility::InitList(&m_piDomainModifier,NUM_DOMAIN_TYPES,iDefault);
			}
			m_piDomainModifier[i] = pClassInfo->getDomainModifier(i);
		}
	}

	for ( int i = 0; i < GC.getNumBonusInfos(); i++)
	{
		if ( getBonusProductionModifier(i) == iDefault && pClassInfo->getBonusProductionModifier(i) != iDefault)
		{
			if ( NULL == m_piBonusProductionModifier )
			{
				CvXMLLoadUtility::InitList(&m_piBonusProductionModifier,GC.getNumBonusInfos(),iDefault);
			}
			m_piBonusProductionModifier[i] = pClassInfo->getBonusProductionModifier(i);
		}
	}

	for ( int i = 0; i < GC.getNumPromotionInfos(); i++)
	{
		if ( getFreePromotions(i) == bDefault && pClassInfo->getFreePromotions(i) != bDefault)
		{
			if ( NULL == m_pbFreePromotions )
			{
				CvXMLLoadUtility::InitList(&m_pbFreePromotions,GC.getNumPromotionInfos(),bDefault);
			}
			m_pbFreePromotions[i] = pClassInfo->getFreePromotions(i);
		}
	}

/********************************************************************************/
/**		REVDCM									2/16/10				phungus420	*/
/**																				*/
/**		CanTrain																*/
/********************************************************************************/
	for ( int i = 0; i < GC.getNumCivicInfos(); i++)
	{
		if ( isPrereqOrCivics(i) == bDefault && pClassInfo->isPrereqOrCivics(i) != bDefault)
		{
			if ( NULL == m_pbPrereqOrCivics )
			{
				CvXMLLoadUtility::InitList(&m_pbPrereqOrCivics,GC.getNumCivicInfos(),bDefault);
			}
			m_pbPrereqOrCivics[i] = pClassInfo->isPrereqOrCivics(i);
		}
	}

	for ( int i = 0; i < GC.getNumBuildingClassInfos(); i++)
	{
        if ( isPrereqBuildingClass(i) == bDefault && pClassInfo->isPrereqBuildingClass(i) != bDefault)
		{
			if ( NULL == m_pbPrereqBuildingClass )
			{
				CvXMLLoadUtility::InitList(&m_pbPrereqBuildingClass,GC.getNumBuildingClassInfos(),bDefault);
			}
			m_pbPrereqBuildingClass[i] = pClassInfo->isPrereqBuildingClass(i);
		}
	}

	for ( int i = 0; i < GC.getNumBuildingClassInfos(); i++)
	{
        if ( getPrereqBuildingClassOverrideTech(i) == -1 && pClassInfo->getPrereqBuildingClassOverrideTech(i) != -1 )
		{
			if ( NULL == m_piPrereqBuildingClassOverrideTech )
			{
				CvXMLLoadUtility::InitList(&m_piPrereqBuildingClassOverrideTech,GC.getNumBuildingClassInfos(),-1);
			}
			m_piPrereqBuildingClassOverrideTech[i] = pClassInfo->getPrereqBuildingClassOverrideTech(i);
		}
	}

	for ( int i = 0; i < GC.getNumBuildingClassInfos(); i++)
	{
        if ( getPrereqBuildingClassOverrideEra(i) == -1 && pClassInfo->getPrereqBuildingClassOverrideEra(i) != -1 )
		{
			if ( NULL == m_piPrereqBuildingClassOverrideEra )
			{
				CvXMLLoadUtility::InitList(&m_piPrereqBuildingClassOverrideEra,GC.getNumBuildingClassInfos(),-1);
			}
			m_piPrereqBuildingClassOverrideEra[i] = pClassInfo->getPrereqBuildingClassOverrideEra(i);
		}
	}

	for ( int i = 0; i < GC.getNumUnitClassInfos(); i++)
	{
        if ( getForceObsoleteUnitClass(i) == bDefault && pClassInfo->getForceObsoleteUnitClass(i) != bDefault)
		{
			if ( NULL == m_pbForceObsoleteUnitClass )
			{
				CvXMLLoadUtility::InitList(&m_pbForceObsoleteUnitClass,GC.getNumUnitClassInfos(),bDefault);
			}
			m_pbForceObsoleteUnitClass[i] = pClassInfo->getForceObsoleteUnitClass(i);
		}
	}
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

	for ( int i = 0; i < GC.getNUM_UNIT_AND_TECH_PREREQS(); i++)
	{
		if ( getPrereqAndTechs(i) == NO_TECH && pClassInfo->getPrereqAndTechs(i) != NO_TECH)
		{
			if ( NULL == m_piPrereqAndTechs )
			{
				CvXMLLoadUtility::InitList(&m_piPrereqAndTechs,GC.getNUM_UNIT_AND_TECH_PREREQS(),(int)NO_TECH);
			}
			m_piPrereqAndTechs[i] = pClassInfo->getPrereqAndTechs(i);
		}
	}

	for ( int i = 0; i < GC.getNUM_UNIT_PREREQ_OR_BONUSES(); i++)
	{
		if ( getPrereqOrBonuses(i) == NO_BONUS && pClassInfo->getPrereqOrBonuses(i) != NO_BONUS)
		{
			if ( NULL == m_piPrereqOrBonuses )
			{
				CvXMLLoadUtility::InitList(&m_piPrereqOrBonuses,GC.getNUM_UNIT_PREREQ_OR_BONUSES(),(int)NO_BONUS);
			}
			m_piPrereqOrBonuses[i] = pClassInfo->getPrereqOrBonuses(i);
		}
	}
	if ( m_iVolleyRange == iDefault ) m_iVolleyRange = pClassInfo->getVolleyRange();
	if ( m_iVolleyAccuracy == iDefault ) m_iVolleyAccuracy = pClassInfo->getVolleyAccuracy();
	if ( m_iVolleyAccuracyMin == iDefault ) m_iVolleyAccuracyMin = pClassInfo->getVolleyAccuracyMin();
	if ( m_iVolleyEfficiency == iDefault ) m_iVolleyEfficiency = pClassInfo->getVolleyEfficiency();
	if ( m_iVolleyRounds == 1 ) m_iVolleyRounds = pClassInfo->getVolleyRounds();
	if ( m_iVolleyRounds < 1 ) m_iVolleyRounds = 1;

	/********************************************************************************/
	/*		REVOLUTION_MOD							2/12/09				Afforess    */
	/*																				*/
	/*		 																		*/
	/********************************************************************************/
	if ( m_bDCMAirBomb1 == bDefault ) m_bDCMAirBomb1 = pClassInfo->getDCMAirBomb1();
	if ( m_bDCMAirBomb2 == bDefault ) m_bDCMAirBomb2 = pClassInfo->getDCMAirBomb2();
	if ( m_bDCMAirBomb3 == bDefault ) m_bDCMAirBomb3 = pClassInfo->getDCMAirBomb3();
	if ( m_bDCMAirBomb4 == bDefault ) m_bDCMAirBomb4 = pClassInfo->getDCMAirBomb4();
	if ( m_bDCMAirBomb5 == bDefault ) m_bDCMAirBomb5 = pClassInfo->getDCMAirBomb5();
	
	if ( m_bDCMFighterEngage == bDefault ) m_bDCMFighterEngage = pClassInfo->getDCMFighterEngage();
	/********************************************************************************/
	/*		REVOLUTION_MOD							END								*/
	/********************************************************************************/

/********************************************************************************/
/**		REVDCM									2/16/10				phungus420	*/
/**																				*/
/**		CanTrain																*/
/********************************************************************************/
	if ( m_iMaxStartEra == iTextDefault) m_iMaxStartEra = pClassInfo->getMaxStartEra();
	if ( m_iForceObsoleteTech == iTextDefault ) m_iForceObsoleteTech = pClassInfo->getForceObsoleteTech();
	if ( m_bStateReligion == bDefault )	m_bStateReligion = pClassInfo->isStateReligion();
	if ( m_iPrereqGameOption == iTextDefault ) m_iPrereqGameOption = pClassInfo->getPrereqGameOption();
	if ( m_iNotGameOption == iTextDefault ) m_iNotGameOption = pClassInfo->getNotGameOption();
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

	if ( m_iHolyCity == iTextDefault ) m_iHolyCity = pClassInfo->getHolyCity();
	if ( m_iReligionType == iTextDefault ) m_iReligionType = pClassInfo->getReligionType();
	if ( m_iStateReligion == iTextDefault ) m_iStateReligion = pClassInfo->getStateReligion();
	if ( m_iPrereqReligion == iTextDefault ) m_iPrereqReligion = pClassInfo->getPrereqReligion();
	if ( m_iPrereqCorporation == iTextDefault ) m_iPrereqCorporation = pClassInfo->getPrereqCorporation();
	if ( m_iPrereqBuilding == iTextDefault ) m_iPrereqBuilding = pClassInfo->getPrereqBuilding();
	if ( m_aePrereqOrBuildings.empty() )
	{
		int iNum = pClassInfo->getPrereqOrBuildingsNum();
		for (int i = 0; i < iNum; i++)
			m_aePrereqOrBuildings.push_back(pClassInfo->getPrereqOrBuilding(i));
	}
	if ( m_iOverrideResourceBuilding == iTextDefault ) m_iOverrideResourceBuilding = pClassInfo->getOverrideResourceBuilding();	
	if ( m_iPrereqAndTech == iTextDefault ) m_iPrereqAndTech = pClassInfo->getPrereqAndTech();
	if ( m_iPrereqAndBonus == iTextDefault ) m_iPrereqAndBonus = pClassInfo->getPrereqAndBonus();

	if ( m_iAIWeight == iDefault ) m_iAIWeight = pClassInfo->getAIWeight();
	if ( m_iProductionCost == iDefault ) m_iProductionCost = pClassInfo->getProductionCost();
	if ( m_iHurryCostModifier == iDefault ) m_iHurryCostModifier = pClassInfo->getHurryCostModifier();
	if ( m_iAdvancedStartCost == iDefault ) m_iAdvancedStartCost = pClassInfo->getAdvancedStartCost();
	if ( m_iAdvancedStartCostIncrease == iDefault ) m_iAdvancedStartCostIncrease = pClassInfo->getAdvancedStartCostIncrease();
	if ( m_iMinAreaSize == iDefault ) m_iMinAreaSize = pClassInfo->getMinAreaSize();
	if ( m_iMoves == iDefault ) m_iMoves = pClassInfo->getMoves();
	if ( m_iAirRange == iDefault ) m_iAirRange = pClassInfo->getAirRange();
	if ( m_iAirUnitCap == iDefault ) m_iAirUnitCap = pClassInfo->getAirUnitCap();
	if ( m_iDropRange == iDefault ) m_iDropRange = pClassInfo->getDropRange();
	if ( m_iNukeRange == iDefault ) m_iNukeRange = pClassInfo->getNukeRange();
	if ( m_iWorkRate == iDefault ) m_iWorkRate = pClassInfo->getWorkRate();
	if ( m_iBaseDiscover == iDefault ) m_iBaseDiscover = pClassInfo->getBaseDiscover();
	if ( m_iDiscoverMultiplier == iDefault ) m_iDiscoverMultiplier = pClassInfo->getDiscoverMultiplier();
	if ( m_iBaseHurry == iDefault ) m_iBaseHurry = pClassInfo->getBaseHurry();
	if ( m_iHurryMultiplier == iDefault ) m_iHurryMultiplier = pClassInfo->getHurryMultiplier();
	if ( m_iBaseTrade == iDefault ) m_iBaseTrade = pClassInfo->getBaseTrade();
	if ( m_iTradeMultiplier == iDefault ) m_iTradeMultiplier = pClassInfo->getTradeMultiplier();
	if ( m_iGreatWorkCulture == iDefault ) m_iGreatWorkCulture = pClassInfo->getGreatWorkCulture();
	if ( m_iEspionagePoints == iDefault ) m_iEspionagePoints = pClassInfo->getEspionagePoints();	
	if ( m_iCombat == iDefault ) m_iCombat = pClassInfo->getCombat();
	if ( m_iCombatLimit == iDefault ) m_iCombatLimit = pClassInfo->getCombatLimit();
	if ( m_iAirCombat == iDefault ) m_iAirCombat = pClassInfo->getAirCombat();
	if ( m_iAirCombatLimit == iDefault ) m_iAirCombatLimit = pClassInfo->getAirCombatLimit();
	if ( m_iXPValueAttack == iDefault ) m_iXPValueAttack = pClassInfo->getXPValueAttack();
	if ( m_iXPValueDefense == iDefault ) m_iXPValueDefense = pClassInfo->getXPValueDefense();
	if ( m_iFirstStrikes == iDefault ) m_iFirstStrikes = pClassInfo->getFirstStrikes();
	if ( m_iChanceFirstStrikes == iDefault ) m_iChanceFirstStrikes = pClassInfo->getChanceFirstStrikes();
	if ( m_iInterceptionProbability == iDefault ) m_iInterceptionProbability = pClassInfo->getInterceptionProbability();
	//ls612: Advanced Nuke Interception
	//if ( m_iNukeInterceptionProbability == iDefault ) m_iNukeInterceptionProbability = pClassInfo->getNukeInterceptionProbability();
	//if ( m_iNukeInterceptionRange == iDefault ) m_iNukeInterceptionRange = pClassInfo->getNukeInterceptionRange();
	if ( m_iEvasionProbability == iDefault ) m_iEvasionProbability = pClassInfo->getEvasionProbability();
	if ( m_iWithdrawalProbability == iDefault ) m_iWithdrawalProbability = pClassInfo->getWithdrawalProbability();
	if ( m_iCollateralDamage == iDefault ) m_iCollateralDamage = pClassInfo->getCollateralDamage();
	if ( m_iCollateralDamageLimit == iDefault ) m_iCollateralDamageLimit = pClassInfo->getCollateralDamageLimit();
	if ( m_iCollateralDamageMaxUnits == iDefault ) m_iCollateralDamageMaxUnits = pClassInfo->getCollateralDamageMaxUnits();
	if ( m_iCityAttackModifier == iDefault ) m_iCityAttackModifier = pClassInfo->getCityAttackModifier();
	if ( m_iCityDefenseModifier == iDefault ) m_iCityDefenseModifier = pClassInfo->getCityDefenseModifier();
	if ( m_iAnimalCombatModifier == iDefault ) m_iAnimalCombatModifier = pClassInfo->getAnimalCombatModifier();
	if ( m_iHillsAttackModifier == iDefault ) m_iHillsAttackModifier = pClassInfo->getHillsAttackModifier();
	if ( m_iHillsDefenseModifier == iDefault ) m_iHillsDefenseModifier = pClassInfo->getHillsDefenseModifier();

	if ( m_iBombRate == iDefault ) m_iBombRate = pClassInfo->getBombRate();
	if ( m_iBombardRate == iDefault ) m_iBombardRate = pClassInfo->getBombardRate();

	//if ( m_iSpecialCargo == iTextDefault ) m_iSpecialCargo = pClassInfo->getSpecialCargo(); removed by 45deg for advanced cargo mod
	if ( m_iDomainCargo == iTextDefault ) m_iDomainCargo = pClassInfo->getDomainCargo();
	
	if ( m_iCargoSpace == iDefault ) m_iCargoSpace = pClassInfo->getCargoSpace();
	if ( m_iConscriptionValue == iDefault ) m_iConscriptionValue = pClassInfo->getConscriptionValue();
	if ( m_iCultureGarrisonValue == iDefault ) m_iCultureGarrisonValue = pClassInfo->getCultureGarrisonValue();
	if ( m_iExtraCost == iDefault ) m_iExtraCost = pClassInfo->getExtraCost();
	if ( m_iAssetValue == iDefault ) m_iAssetValue = pClassInfo->getAssetValue();
	if ( m_iPowerValue == iDefault ) m_iPowerValue = pClassInfo->getPowerValue();

	if ( m_iLeaderPromotion == iTextDefault ) m_iLeaderPromotion = pClassInfo->getLeaderPromotion();
	if ( m_iLeaderExperience == iDefault ) m_iLeaderExperience = pClassInfo->getLeaderExperience();
	
/************************************************************************************************/
/* Afforess	                  Start		 02/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (getPrereqVicinityBonus() == iTextDefault) m_iPrereqVicinityBonus = pClassInfo->getPrereqVicinityBonus();
	if ( m_bRequiresStateReligionInCity == bDefault )	m_bRequiresStateReligionInCity = pClassInfo->isRequiresStateReligionInCity();
	if ( m_bWorkerTrade == bDefault )	m_bWorkerTrade = pClassInfo->isWorkerTrade();
	if ( m_bMilitaryTrade == bDefault )	m_bMilitaryTrade = pClassInfo->isMilitaryTrade();
	if ( m_bForceUpgrade == bDefault )	m_bForceUpgrade = pClassInfo->isForceUpgrade();
	if ( m_bGreatGeneral == bDefault )	m_bGreatGeneral = pClassInfo->isGreatGeneral();
	if ( m_bSlave == bDefault )	m_bSlave = pClassInfo->isSlave();
	if ( m_iBaseFoodChange == iDefault )	m_iBaseFoodChange = pClassInfo->getBaseFoodChange();
	if ( m_iControlPoints == iDefault )	m_iControlPoints = pClassInfo->getControlPoints();
	if ( m_iCommandRange == iDefault )	m_iCommandRange = pClassInfo->getCommandRange();
	
	for ( int i = 0; i < GC.getNUM_UNIT_PREREQ_OR_BONUSES(); i++)
	{
		if ( getPrereqOrVicinityBonuses(i) == NO_BONUS && pClassInfo->getPrereqOrVicinityBonuses(i) != NO_BONUS)
		{
			if ( NULL == m_piPrereqOrVicinityBonuses )
			{
				CvXMLLoadUtility::InitList(&m_piPrereqOrVicinityBonuses,GC.getNUM_UNIT_PREREQ_OR_BONUSES(),(int)NO_BONUS);
			}
			m_piPrereqOrVicinityBonuses[i] = pClassInfo->getPrereqOrVicinityBonuses(i);
		}
	}
	
	for (int i = 0; i < GC.getNumRouteInfos(); i++)
	{
		if (!getPassableRouteNeeded(i) && pClassInfo->getPassableRouteNeeded(i))
		{
			if ( NULL == m_pbPassableRouteNeeded )
			{
				CvXMLLoadUtility::InitList(&m_pbPassableRouteNeeded,GC.getNumRouteInfos(),false);
			}
			m_pbPassableRouteNeeded[i] = pClassInfo->getPassableRouteNeeded(i);
		}
	}

	m_PropertyManipulators.copyNonDefaults(pClassInfo->getPropertyManipulators(),pXML);

	if (!m_pExprTrainCondition)
	{
		m_pExprTrainCondition = pClassInfo->m_pExprTrainCondition;
		pClassInfo->m_pExprTrainCondition = NULL;
	}
	
	// For the Meshgroups I assume the XML holding the largest GroupSize is the most completed(most fancy)
	// and we want to keep that one
	if ( m_iGroupSize < pClassInfo->getGroupSize() )
	{
		m_iGroupSize = pClassInfo->getGroupSize();
		m_iGroupDefinitions = pClassInfo->getGroupDefinitions();

		m_iUnitMeleeWaveSize = pClassInfo->getMeleeWaveSize();
		m_iUnitRangedWaveSize = pClassInfo->getRangedWaveSize();
		m_fUnitMaxSpeed = pClassInfo->getUnitMaxSpeed();
		m_fUnitPadTime = pClassInfo->getUnitPadTime();

		//Delete old Arrays for initializing a new one
		SAFE_DELETE_ARRAY(m_piUnitGroupRequired)
		SAFE_DELETE_ARRAY(m_paszEarlyArtDefineTags)
		SAFE_DELETE_ARRAY(m_paszLateArtDefineTags)
		SAFE_DELETE_ARRAY(m_paszMiddleArtDefineTags)
		SAFE_DELETE_ARRAY(m_paszClassicalArtDefineTags)
		SAFE_DELETE_ARRAY(m_paszRennArtDefineTags)
		SAFE_DELETE_ARRAY(m_paszIndustrialArtDefineTags)
		SAFE_DELETE_ARRAY(m_paszFutureArtDefineTags)
	
		m_piUnitGroupRequired = new int[ m_iGroupDefinitions ];
		m_paszEarlyArtDefineTags = new CvString[ m_iGroupDefinitions ];
		m_paszLateArtDefineTags = new CvString[ m_iGroupDefinitions ];
		m_paszMiddleArtDefineTags = new CvString[ m_iGroupDefinitions ];
		m_paszClassicalArtDefineTags = new CvString[ m_iGroupDefinitions ];
		m_paszRennArtDefineTags = new CvString[ m_iGroupDefinitions ];
		m_paszIndustrialArtDefineTags = new CvString[ m_iGroupDefinitions ];
		m_paszFutureArtDefineTags = new CvString[ m_iGroupDefinitions ];
		
		for ( int i = 0; i < m_iGroupDefinitions; i++ )
		{
			m_piUnitGroupRequired[i] = pClassInfo->getUnitGroupRequired(i);
			setEarlyArtDefineTag(i, pClassInfo->getEarlyArtDefineTag(i, NO_UNIT_ARTSTYLE));
			setLateArtDefineTag(i, pClassInfo->getLateArtDefineTag(i, NO_UNIT_ARTSTYLE));
			setClassicalArtDefineTag(i, pClassInfo->getClassicalArtDefineTag(i, NO_UNIT_ARTSTYLE));
			setMiddleArtDefineTag(i, pClassInfo->getMiddleArtDefineTag(i, NO_UNIT_ARTSTYLE));
			setRennArtDefineTag(i, pClassInfo->getRennArtDefineTag(i, NO_UNIT_ARTSTYLE));
			setIndustrialArtDefineTag(i, pClassInfo->getIndustrialArtDefineTag(i, NO_UNIT_ARTSTYLE));
			setFutureArtDefineTag(i, pClassInfo->getFutureArtDefineTag(i, NO_UNIT_ARTSTYLE));
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	
	if ( m_szFormationType == "FORMATION_TYPE_DEFAULT" ) m_szFormationType = pClassInfo->getFormationType();

	// First we check if there are different Unique Unitnames in the Modules(we want to keep all of them)
	// So we have to set the Arraysize properly, knowing the amount of Unique Unitnames
	if ( pClassInfo->getNumUnitNames() != 0 )
	{
		CvString* m_paszNewNames = new CvString[pClassInfo->getNumUnitNames()];
		for ( int i = 0; i < pClassInfo->getNumUnitNames(); i++)
		{
			m_paszNewNames[i] = pClassInfo->getUnitNames(i);
		}
		
		CvXMLLoadUtilityModTools* pCurrentUnitClass = new CvXMLLoadUtilityModTools;
		pCurrentUnitClass->StringArrayExtend(&m_paszUnitNames, &m_iNumUnitNames,
										 &m_paszNewNames, pClassInfo->getNumUnitNames());
		delete pCurrentUnitClass;
		SAFE_DELETE_ARRAY(m_paszNewNames)
	}

	if (m_iGroupSize != 0)
	{
		updateArtDefineButton();
	}
}

bool CvUnitInfo::readPass2(CvXMLLoadUtility *pXML)
{
	if (!CvInfoBase::read(pXML))
	{
		return false;
	}

	getKillOutcomeList()->read(pXML, "KillOutcomes");

	if(GETXML->SetToChildByTagName( pXML->GetXML(), "Actions"))
	{
		if(GETXML->SetToChild( pXML->GetXML() ))
		{

			if (GETXML->LocateFirstSiblingNodeByTagName(pXML->GetXML(), "Action"))
			{
				int i = 0;
				do
				{
					m_aOutcomeMissions.push_back(new CvOutcomeMission());
					m_aOutcomeMissions[i]->read(pXML);
					i++;
				} while(GETXML->NextSibling(pXML->GetXML()));
			}
			GETXML->SetToParent( pXML->GetXML() );
		}
		GETXML->SetToParent( pXML->GetXML() );
	}

	return true;
}

void CvUnitInfo::copyNonDefaultsReadPass2(CvUnitInfo *pClassInfo, CvXMLLoadUtility *pXML)
{
	getKillOutcomeList()->copyNonDefaults(pClassInfo->getKillOutcomeList(), pXML);

	if (m_aOutcomeMissions.empty())
	{
		int num = (int) pClassInfo->getNumActionOutcomes();
		for (int index = 0; index < num; index++)
		{
			m_aOutcomeMissions.push_back(pClassInfo->m_aOutcomeMissions[index]);
			pClassInfo->m_aOutcomeMissions[index] = NULL;
		}
	}
}

bool CvUnitInfo::readPass3()
{
	m_paszCivilizationNames = new CvWString[GC.getNumCivilizationInfos()];
	for (int iI = 0; iI < GC.getNumCivilizationInfos(); iI++)
	{
		m_paszCivilizationNames[iI] = "";
	}

	if (!m_aszCivilizationNamesValueforPass3.empty() && !m_aszCivilizationNamesforPass3.empty())
	{
		int iNumLoad = m_aszCivilizationNamesValueforPass3.size();
		for(int iI = 0; iI < iNumLoad; iI++)
		{
			int iTempIndex = GC.getInfoTypeForString(m_aszCivilizationNamesforPass3[iI]);
			if (iTempIndex >= 0 && iTempIndex < GC.getNumCivilizationInfos())
				m_paszCivilizationNames[iTempIndex] = m_aszCivilizationNamesValueforPass3[iI];
		}
		m_aszCivilizationNamesforPass3.clear();
		m_aszCivilizationNamesValueforPass3.clear();
	}

	return true;
}
/************************************************************************************************/
/* XMLCOPY                                 END                                                  */
/************************************************************************************************/

bool CvUnitInfo::hasUnitCombat(UnitCombatTypes eUnitCombat) const
{
	FAssert(0 <= eUnitCombat && eUnitCombat < GC.getNumUnitCombatInfos());
	
	if ( m_abHasCombatType == NULL )
	{
		m_abHasCombatType = new bool[GC.getNumUnitCombatInfos()];
		memset(m_abHasCombatType, 0, GC.getNumUnitCombatInfos());

		for(int iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
		{
			if (getUnitCombatType() == iI)
			{
				m_abHasCombatType[iI] = true;
				continue;
			}
			//TB SubCombat Mod Begin
			for (int iJ = 0; iJ < getNumSubCombatTypes(); iJ++)
			{
				if (getSubCombatType(iJ) == iI)
				{
					m_abHasCombatType[iI] = true;
					break;
				}
			}
		}
	}

	return m_abHasCombatType[eUnitCombat];
}
