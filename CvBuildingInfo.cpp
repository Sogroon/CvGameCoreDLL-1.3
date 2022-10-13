//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:	CvBuildingInfos.cpp
//
//  PURPOSE: Info class for buildings
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
//					CvBuildingInfo
//======================================================================================================

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   CvBuildingInfo()
//
//  PURPOSE :   Default constructor
//
//------------------------------------------------------------------------------------------------------
CvBuildingInfo::CvBuildingInfo() :
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
// Dale - AB: Bombing START
m_iDCMAirbombMission(0),
// Dale - AB: Bombing END
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/

m_iBuildingClassType(NO_BUILDINGCLASS),
m_iVictoryPrereq(NO_VICTORY),
m_iFreeStartEra(NO_ERA),
m_iMaxStartEra(NO_ERA),
m_iObsoleteTech(NO_TECH),
m_iPrereqAndTech(NO_TECH),
m_iNoBonus(NO_BONUS),
m_iPowerBonus(NO_BONUS),
m_iFreeBonus(NO_BONUS),
m_iNumFreeBonuses(0),
//m_iFreeBuildingClass(NO_BUILDINGCLASS),   DarkLunaPhantom - FreeBuilding accepts lists.
m_iFreeAreaBuildingClass(NO_BUILDINGCLASS),
m_iFreeTradeRegionBuildingClass(NO_BUILDINGCLASS),
m_iFreePromotion(NO_PROMOTION),
m_iCivicOption(NO_CIVICOPTION),
m_iAIWeight(0),
m_iProductionCost(0),
m_iHurryCostModifier(0),
m_iHurryAngerModifier(0),
m_iAdvancedStartCost(0),
m_iAdvancedStartCostIncrease(0),
m_iMinAreaSize(0),
m_iNumCitiesPrereq(0),
m_iNumTeamsPrereq(0),
m_iUnitLevelPrereq(0),
m_iMinLatitude(0),
m_iMaxLatitude(90),
m_iGreatPeopleRateModifier(0),
m_iGreatGeneralRateModifier(0),
m_iDomesticGreatGeneralRateModifier(0),
m_iGlobalGreatPeopleRateModifier(0),
m_iAnarchyModifier(0),
m_iGoldenAgeModifier(0),
m_iGlobalHurryModifier(0),
m_iFreeExperience(0),
m_iGlobalFreeExperience(0),
m_iFoodKept(0),
//doto culture safe zone
m_iNoForeignCityCultureGrowth(0),
//doto culture safe zone
m_iAirlift(0),
m_iAirModifier(0),
m_iAirUnitCapacity(0),
m_iWonderCapacityIncrement(0),
m_iNukeModifier(0),
m_iNukeExplosionRand(0),
m_iFreeSpecialist(0),
m_iAreaFreeSpecialist(0),
m_iGlobalFreeSpecialist(0),
m_iHappiness(0),
m_iAreaHappiness(0),
m_iGlobalHappiness(0),
m_iStateReligionHappiness(0),
m_iStateReligionHealth(0),
m_iWorkerSpeedModifier(0),
m_iMilitaryProductionModifier(0),
m_iSpaceProductionModifier(0),
m_iGlobalSpaceProductionModifier(0),
m_iTradeRoutes(0),
m_iInterestPercent(0),
m_iInterestMax(0),
m_iCoastalTradeRoutes(0),
m_iGlobalTradeRoutes(0),
m_iTradeRouteModifier(0),
m_iForeignTradeRouteModifier(0),
m_iAssetValue(0),
m_iPowerValue(0),
m_iSpecialBuildingType(NO_SPECIALBUILDING),
m_iAdvisorType(NO_ADVISOR),

/********************************************************************************/
/**		REVDCM									2/16/10				phungus420	*/
/**																				*/
/**		CanConstruct															*/
/********************************************************************************/
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
m_iFoundsCorporation(NO_CORPORATION),
m_iGlobalReligionCommerce(0),
m_iGlobalCorporationCommerce(0),
m_iPrereqAndBonus(NO_BONUS),
m_iGreatPeopleUnitClass(NO_UNITCLASS),
m_iGreatPeopleRateChange(0),
m_iConquestProbability(0),
m_iMaintenanceModifier(0),
//DPII < Maintenance Modifier >
m_iGlobalMaintenanceModifier(0),
m_iAreaMaintenanceModifier(0),
m_iOtherAreaMaintenanceModifier(0),
m_iDistanceMaintenanceModifier(0),
m_iNumCitiesMaintenanceModifier(0),
m_iCoastalDistanceMaintenanceModifier(0),
m_iConnectedCityMaintenanceModifier(0),
//DPII < Maintenance Modifier >
m_iWarWearinessModifier(0),
m_iGlobalWarWearinessModifier(0),
m_iEnemyWarWearinessModifier(0),
m_iHealRateChange(0),
m_iHealth(0),
m_iAreaHealth(0),
m_iGlobalHealth(0),
m_iGlobalPopulationChange(0),
m_iFreeTechs(0),
m_eFreeSpecialTech(NO_TECH),
m_iDefenseModifier(0),
m_iBombardDefenseModifier(0),
m_iAllCityDefenseModifier(0),
m_iEspionageDefenseModifier(0),

/********************************************************************************/
/**		REVDCM									4/09/10				phungus420	*/
/**																				*/
/**		Building Effects														*/
/********************************************************************************/
m_iUnitUpgradePriceModifier(0),
m_iRevIdxLocal(0),
m_iRevIdxNational(0),
m_iRevIdxDistanceModifier(0),
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

m_iMissionType(NO_MISSION),
m_iVoteSourceType(NO_VOTESOURCE),
m_fVisibilityPriority(0.0f),
m_bTeamShare(false),
m_bWater(false),
m_bRiver(false),
m_bFreshWater(false),
m_bPower(false),
m_bNoPower(false),
m_bDirtyPower(false),
m_bAreaCleanPower(false),
m_bNoHolyCity(false),
m_bAreaBorderObstacle(false),
m_bForceTeamVoteEligible(false),
m_bCapital(false),
m_bGovernmentCenter(false),
m_bGoldenAge(false),
m_bMapCentering(false),
m_bNoUnhappiness(false),
m_bNoUnhealthyPopulation(false),
m_bBuildingOnlyHealthy(false),
m_bNeverCapture(false),
m_bNukeImmune(false),
m_bPrereqReligion(false),
m_bFreeFromStateReligion(false),
m_bCenterInCity(false),
m_bStateReligion(false),
m_bAllowsNukes(false),
// <f1rpo> Better initialize these
m_bEnablesOtherBuildingsValue(false),
m_bEnablesUnits(false), // </f1rpo>
m_piPrereqAndTechs(NULL),
m_piPrereqOrBonuses(NULL),
m_piProductionTraits(NULL),
m_piHappinessTraits(NULL),
m_piSeaPlotYieldChange(NULL),
m_piRiverPlotYieldChange(NULL),
m_piGlobalSeaPlotYieldChange(NULL),
m_piYieldChange(NULL),
m_piYieldModifier(NULL),
m_piPowerYieldModifier(NULL),
m_piAreaYieldModifier(NULL),
m_piGlobalYieldModifier(NULL),
m_piCommerceChange(NULL),
m_piObsoleteSafeCommerceChange(NULL),
m_piCommerceChangeDoubleTime(NULL),
m_piCommerceModifier(NULL),
m_piGlobalCommerceModifier(NULL),
m_piSpecialistExtraCommerce(NULL),
m_piStateReligionCommerce(NULL),
m_piCommerceHappiness(NULL),
m_piReligionChange(NULL),
m_piSpecialistCount(NULL),
m_piFreeSpecialistCount(NULL),
m_piBonusHealthChanges(NULL),
m_piBonusHappinessChanges(NULL),
m_piBonusProductionModifier(NULL),
m_piUnitCombatFreeExperience(NULL),
m_piDomainFreeExperience(NULL),
m_piDomainProductionModifier(NULL),
m_piBuildingHappinessChanges(NULL),
m_piPrereqNumOfBuildingClass(NULL),
m_piFlavorValue(NULL),
m_piImprovementFreeSpecialist(NULL),
m_pbCommerceFlexible(NULL),
m_pbCommerceChangeOriginalOwner(NULL),
m_pbBuildingClassNeededInCity(NULL),
m_ppaiSpecialistYieldChange(NULL),
m_ppaiBonusYieldModifier(NULL)
/************************************************************************************************/
/* Afforess	                  Start		 Last Update: 6/13/10                                    */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
//New Integers
,m_iGlobalPopulationgrowthratepercentage(0)
,m_iPopulationgrowthratepercentage(0)
,m_iPrereqVicinityBonus(NO_BONUS)
,m_iPillageGoldModifier(0)
,m_iFreePromotion_2(NO_PROMOTION)
,m_iFreePromotion_3(NO_PROMOTION)
,m_iWorldTradeRoutes(0)
,m_iLineOfSight(0)
,m_iInflationModifier(0)
,m_iInvasionChance(0)
,m_iAdjacentDamagePercent(0)
,m_iProductionContinueBuildingClass(NO_BUILDINGCLASS)
,m_iPrereqPopulation(0)
,m_iPrereqCultureLevel(NO_CULTURELEVEL)
,m_iWorkableRadius(0)
,m_iPrereqAnyoneBuildingClass(NO_BUILDINGCLASS)
,m_iOccupationTimeModifier(0)
,m_iNoEntryDefenseLevel(0)
,m_iNumUnitFullHeal(0)
,m_iExtendsBuildingClass(NO_BUILDINGCLASS)
,m_iNumPopulationEmployed(0)
,m_iHealthPercentPerPopulation(0)
,m_iHappinessPercentPerPopulation(0)
,m_iCapitalExtraCommerce(0)
,m_iForeignCapitalExtraCommerce(0)
,m_iDomesticConnectedCommerce(0)
,m_iCitiesRequiredPerDomesticConnectedCity(1)
,m_iMaxDomesticConnectedCommerce(0)
,m_iForeignConnectedCommerce(0)
,m_iCitiesRequiredPerForeignConnectedCity(1)
,m_iMaxForeignConnectedCommerce(0)
,m_iDiplomacyChange(0)

,m_iCrime(0)
,m_iCrimePerPop(0)

//New Booleans
,m_bAnySpecialistYieldChanges(false)
,m_bAnyBonusYieldModifiers(false)
,m_bApplyFreePromotionOnMove(false)
,m_bBuildOnlyOnPeaks(false)
,m_bPrereqPower(false)
,m_bNoEnemyPillagingIncome(false)
,m_bProvidesFreshWater(false)
,m_bForceAllTradeRoutes(false)
,m_bForceNoPrereqScaling(false)
,m_bPrereqWar(false)
,m_bRequiresActiveCivics(false)
,m_bZoneOfControl(false)
,m_bProtectedCulture(false)
,m_bAllowsAmbassadors(false)
//New Boolean Arrays
,m_pbPrereqOrCivics(NULL)
,m_pbPrereqAndCivics(NULL)
,m_pbPrereqNotBuildingClass(NULL)
,m_pbPrereqOrTerrain(NULL)
,m_pbPrereqAndTerrain(NULL)
,m_pbPrereqOrImprovement(NULL)
,m_pbPrereqOrFeature(NULL)
,m_pbReplaceBuildingClass(NULL)
,m_pbPrereqOrBuildingClass(NULL)
,m_pbPrereqOrGameSpeed(NULL)
//New Integer Arrays
,m_piMemoryTypeModifier(NULL)
,m_piBuildingClassProductionModifier(NULL)
,m_piGlobalBuildingClassProductionModifier(NULL)
,m_piTechHappinessChanges(NULL)
,m_piTechHealthChanges(NULL)
,m_piUnitClassProductionModifier(NULL)
,m_piPrereqOrVicinityBonuses(NULL)
,m_piBonusDefenseChanges(NULL)
,m_piUnitCombatExtraStrength(NULL)
,m_piCommerceAttacks(NULL)
//New Multidimensional Integer Arrays
,m_ppaiSpecialistCommercePercentChanges(NULL)
, m_bAnySpecialistCommerceChanges(false)

,m_ppaiTechCommerceChange(NULL)
,m_bAnyTechCommerceChanges(false)

,m_ppaiTechYieldChange(NULL)
,m_bAnyTechYieldChanges(false)

,m_ppaiTechSpecialistChange(NULL)
,m_bAnyTechSpecialistChanges(false)

,m_ppaiTechCommerceModifier(NULL)
,m_bAnyTechCommerceModifiers(false)

,m_ppaiTechYieldModifier(NULL)
,m_bAnyTechYieldModifiers(false)

,m_ppaiBonusCommerceModifier(NULL)
,m_bAnyBonusCommerceModifiers(false)

,m_ppaiBonusYieldChanges(NULL)
,m_bAnyBonusYieldChanges(false)

,m_ppaiVicinityBonusYieldChanges(NULL)
,m_bAnyVicinityBonusYieldChanges(false)

,m_ppaiBonusCommercePercentChanges(NULL)
,m_bAnyBonusCommercePercentChanges(false)

,m_ppiImprovementYieldChanges(NULL)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
// <f1rpo> Better initialize these
,m_ppaiTechHappinessChanges(NULL)
,m_ppaiTechHealthChanges(NULL) // </f1rpo>
,m_PropertyManipulators()
,m_pExprNewCityFree(NULL)
,m_pExprConstructCondition(NULL),
m_pabHurry(NULL),
m_bAutoBuild(false),
m_bEnablesOtherBuildingsCalculated(false),
m_bEnablesUnitsCalculated(false)
,m_bNotShowInCity(false)
{
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   ~CvBuildingInfo()
//
//  PURPOSE :   Default destructor
//
//------------------------------------------------------------------------------------------------------
CvBuildingInfo::~CvBuildingInfo()
{
	SAFE_DELETE_ARRAY(m_piPrereqAndTechs);
	SAFE_DELETE_ARRAY(m_piPrereqOrBonuses);
	SAFE_DELETE_ARRAY(m_piProductionTraits);
	SAFE_DELETE_ARRAY(m_piHappinessTraits);
	SAFE_DELETE_ARRAY(m_piSeaPlotYieldChange);
	SAFE_DELETE_ARRAY(m_piRiverPlotYieldChange);
	SAFE_DELETE_ARRAY(m_piGlobalSeaPlotYieldChange);
	SAFE_DELETE_ARRAY(m_piYieldChange);
	SAFE_DELETE_ARRAY(m_piYieldModifier);
	SAFE_DELETE_ARRAY(m_piPowerYieldModifier);
	SAFE_DELETE_ARRAY(m_piAreaYieldModifier);
	SAFE_DELETE_ARRAY(m_piGlobalYieldModifier);
	SAFE_DELETE_ARRAY(m_piCommerceChange);
	SAFE_DELETE_ARRAY(m_piObsoleteSafeCommerceChange);
	SAFE_DELETE_ARRAY(m_piCommerceChangeDoubleTime);
	SAFE_DELETE_ARRAY(m_piCommerceModifier);
	SAFE_DELETE_ARRAY(m_piGlobalCommerceModifier);
	SAFE_DELETE_ARRAY(m_piSpecialistExtraCommerce);
	SAFE_DELETE_ARRAY(m_piStateReligionCommerce);
	SAFE_DELETE_ARRAY(m_piCommerceHappiness);
	SAFE_DELETE_ARRAY(m_piReligionChange);
	SAFE_DELETE_ARRAY(m_piSpecialistCount);
	SAFE_DELETE_ARRAY(m_piFreeSpecialistCount);
	SAFE_DELETE_ARRAY(m_piBonusHealthChanges);
	SAFE_DELETE_ARRAY(m_piBonusHappinessChanges);
	SAFE_DELETE_ARRAY(m_piBonusProductionModifier);
	SAFE_DELETE_ARRAY(m_piUnitCombatFreeExperience);
	SAFE_DELETE_ARRAY(m_piDomainFreeExperience);
	SAFE_DELETE_ARRAY(m_piDomainProductionModifier);
	SAFE_DELETE_ARRAY(m_piBuildingHappinessChanges);
	SAFE_DELETE_ARRAY(m_piPrereqNumOfBuildingClass);
	SAFE_DELETE_ARRAY(m_piFlavorValue);
	SAFE_DELETE_ARRAY(m_piImprovementFreeSpecialist);
	SAFE_DELETE_ARRAY(m_pbCommerceFlexible);
	SAFE_DELETE_ARRAY(m_pbCommerceChangeOriginalOwner);
	SAFE_DELETE_ARRAY(m_pbBuildingClassNeededInCity);

	if (m_ppaiSpecialistYieldChange != NULL)
	{
		for(int i=0;i<GC.getNumSpecialistInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiSpecialistYieldChange[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiSpecialistYieldChange);
	}

	if (m_ppaiBonusYieldModifier != NULL)
	{
		for(int i=0;i<GC.getNumBonusInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiBonusYieldModifier[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiBonusYieldModifier);
	}
/************************************************************************************************/
/* Afforess	                  Start		 Last Update: 6/13/10                                    */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	SAFE_DELETE_ARRAY(m_pbReplaceBuildingClass);
	SAFE_DELETE_ARRAY(m_pbPrereqOrCivics);
	SAFE_DELETE_ARRAY(m_pbPrereqAndCivics);
	SAFE_DELETE_ARRAY(m_pbPrereqNotBuildingClass);
	SAFE_DELETE_ARRAY(m_pbPrereqOrTerrain);
	SAFE_DELETE_ARRAY(m_pbPrereqAndTerrain);
	SAFE_DELETE_ARRAY(m_pbPrereqOrImprovement);
	SAFE_DELETE_ARRAY(m_pbPrereqOrFeature);
	SAFE_DELETE_ARRAY(m_pbPrereqOrBuildingClass);
	SAFE_DELETE_ARRAY(m_pbPrereqOrGameSpeed);

	SAFE_DELETE_ARRAY(m_piMemoryTypeModifier);
	SAFE_DELETE_ARRAY(m_piUnitClassProductionModifier);
	SAFE_DELETE_ARRAY(m_piPrereqOrVicinityBonuses);
	SAFE_DELETE_ARRAY(m_piBonusDefenseChanges);
	SAFE_DELETE_ARRAY(m_piBuildingClassProductionModifier);
	SAFE_DELETE_ARRAY(m_piGlobalBuildingClassProductionModifier);
	SAFE_DELETE_ARRAY(m_piTechHappinessChanges);
	SAFE_DELETE_ARRAY(m_piTechHealthChanges);
	SAFE_DELETE_ARRAY(m_piUnitCombatExtraStrength);
	SAFE_DELETE_ARRAY(m_piCommerceAttacks);

	if (m_ppaiBonusCommerceModifier != NULL)
	{
		for(int i=0;i<GC.getNumBonusInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiBonusCommerceModifier[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiBonusCommerceModifier);
	}

	if (m_ppaiBonusYieldChanges != NULL)
	{
		for(int i=0;i<GC.getNumBonusInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiBonusYieldChanges[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiBonusYieldChanges);
	}

	if (m_ppaiBonusCommercePercentChanges != NULL)
	{
		for(int i=0;i<GC.getNumBonusInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiBonusCommercePercentChanges[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiBonusCommercePercentChanges);
	}

	if (m_ppaiVicinityBonusYieldChanges != NULL)
	{
		for(int i=0;i<GC.getNumBonusInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiVicinityBonusYieldChanges[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiVicinityBonusYieldChanges);
	}

	if (m_ppaiTechCommerceChange != NULL)
	{
		for (int i=0; i<GC.getNumTechInfos(); i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiTechCommerceChange[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiTechCommerceChange);
	}

	if (m_ppaiTechYieldChange != NULL)
	{
		for (int i=0; i<GC.getNumTechInfos(); i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiTechYieldChange[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiTechYieldChange);
	}

	if (m_ppaiTechSpecialistChange != NULL)
	{
		for (int i=0; i<GC.getNumTechInfos(); i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiTechSpecialistChange[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiTechSpecialistChange);
	}

	if (m_ppaiTechCommerceModifier != NULL)
	{
		for (int i=0; i<GC.getNumTechInfos(); i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiTechCommerceModifier[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiTechCommerceModifier);
	}

	if (m_ppaiTechYieldModifier != NULL)
	{
		for (int i=0; i<GC.getNumTechInfos(); i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiTechYieldModifier[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiTechYieldModifier);
	}

	if (m_ppiImprovementYieldChanges != NULL)
	{
		for (int i=0;i<GC.getNumImprovementInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppiImprovementYieldChanges[i]);
		}
		SAFE_DELETE_ARRAY(m_ppiImprovementYieldChanges);
	}
	if (m_ppaiSpecialistCommercePercentChanges != NULL)
	{
		for (int i = 0; i < GC.getNumSpecialistInfos(); i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiSpecialistCommercePercentChanges[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiSpecialistCommercePercentChanges);
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	SAFE_DELETE(m_pExprNewCityFree);
	SAFE_DELETE(m_pExprConstructCondition);
}

int CvBuildingInfo::getBuildingClassType() const
{
	return m_iBuildingClassType;
}

int CvBuildingInfo::getVictoryPrereq() const
{
	return m_iVictoryPrereq;
}

int CvBuildingInfo::getFreeStartEra() const
{
	return m_iFreeStartEra;
}

int CvBuildingInfo::getMaxStartEra() const
{
	return m_iMaxStartEra;
}

int CvBuildingInfo::getObsoleteTech() const
{
	return m_iObsoleteTech;
}

int CvBuildingInfo::getPrereqAndTech() const
{
	return m_iPrereqAndTech;
}

int CvBuildingInfo::getNoBonus() const
{
	return m_iNoBonus;
}

int CvBuildingInfo::getPowerBonus() const
{
	return m_iPowerBonus;
}

int CvBuildingInfo::getFreeBonus() const
{
	return m_iFreeBonus;
}

int CvBuildingInfo::getNumFreeBonuses() const
{
	return m_iNumFreeBonuses;
}

int CvBuildingInfo::getNumExtraFreeBonuses() const
{
	return m_aExtraFreeBonuses.size();
}

BonusTypes CvBuildingInfo::getExtraFreeBonus(int i) const
{
	FAssertMsg(i < (int)m_aExtraFreeBonuses.size(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_aExtraFreeBonuses[i].first;
}

int CvBuildingInfo::getExtraFreeBonusNum(int i) const
{
	FAssertMsg(i < (int)m_aExtraFreeBonuses.size(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_aExtraFreeBonuses[i].second;
}

bool CvBuildingInfo::hasExtraFreeBonus(BonusTypes eBonus) const
{
	for(unsigned int i=0; i<m_aExtraFreeBonuses.size(); i++)
		if (m_aExtraFreeBonuses[i].first == eBonus)
			return true;
	return false;
}

// DarkLunaPhantom - FreeBuilding accepts lists.
int CvBuildingInfo::getFreeBuildingClass(int i) const
{
	FAssert(i < (int)m_aiFreeBuildingClass.size());

	return m_aiFreeBuildingClass[i];
}

int CvBuildingInfo::getNumFreeBuildingClass() const
{
	return (int)m_aiFreeBuildingClass.size();
}

// DarkLunaPhantom begin - Extra FreeBuilding entries.
int CvBuildingInfo::getNumExtraFreeBuildingClass() const
{
    return (int)m_aiExtraFreeBuildingClass.size();
}

int CvBuildingInfo::getExtraFreeBuildingClass(int i) const
{
    FAssert(i < (int)m_aiExtraFreeBuildingClass.size());
    return m_aiExtraFreeBuildingClass[i];
}

bool CvBuildingInfo::getExtraFreeBuildingConnected(int i) const
{
    FAssert(i < (int)m_abExtraFreeBuildingConnected.size());
    return m_abExtraFreeBuildingConnected[i];
}

bool CvBuildingInfo::getExtraFreeBuildingContinuous(int i) const
{
    FAssert(i < (int)m_abExtraFreeBuildingContinuous.size());
    return m_abExtraFreeBuildingContinuous[i];
}
// DarkLunaPhantom end

int CvBuildingInfo::getFreeAreaBuildingClass() const			
{
	return m_iFreeAreaBuildingClass;
}

int CvBuildingInfo::getFreeTradeRegionBuildingClass() const
{
	return m_iFreeTradeRegionBuildingClass;
}

int CvBuildingInfo::getFreePromotion() const
{
	return m_iFreePromotion;
}

int CvBuildingInfo::getCivicOption() const
{
	return m_iCivicOption;
}

int CvBuildingInfo::getAIWeight() const
{
	return m_iAIWeight;
}

int CvBuildingInfo::getProductionCost() const
{
	return m_iProductionCost;
}

int CvBuildingInfo::getHurryCostModifier() const
{
	return m_iHurryCostModifier;
}

int CvBuildingInfo::getHurryAngerModifier() const
{
	return m_iHurryAngerModifier;
}

int CvBuildingInfo::getAdvancedStartCost() const
{
	return m_iAdvancedStartCost;
}

int CvBuildingInfo::getAdvancedStartCostIncrease() const
{
	return m_iAdvancedStartCostIncrease;
}

int CvBuildingInfo::getMinAreaSize() const
{
	return m_iMinAreaSize;
}

int CvBuildingInfo::getNumCitiesPrereq() const
{
	return m_iNumCitiesPrereq;
}

int CvBuildingInfo::getNumTeamsPrereq() const
{
	return m_iNumTeamsPrereq;
}

int CvBuildingInfo::getUnitLevelPrereq() const
{
	return m_iUnitLevelPrereq;
}

int CvBuildingInfo::getMinLatitude() const
{
	return m_iMinLatitude;
}

int CvBuildingInfo::getMaxLatitude() const
{
	return m_iMaxLatitude;
}

int CvBuildingInfo::getGreatPeopleRateModifier() const
{
	return m_iGreatPeopleRateModifier;
}

int CvBuildingInfo::getGreatGeneralRateModifier() const
{
	return m_iGreatGeneralRateModifier;
}

int CvBuildingInfo::getDomesticGreatGeneralRateModifier() const
{
	return m_iDomesticGreatGeneralRateModifier;
}

int CvBuildingInfo::getGlobalGreatPeopleRateModifier() const
{
	return m_iGlobalGreatPeopleRateModifier;
}

int CvBuildingInfo::getAnarchyModifier() const
{
	return m_iAnarchyModifier;
}

int CvBuildingInfo::getGoldenAgeModifier() const
{
	return m_iGoldenAgeModifier;
}

int CvBuildingInfo::getGlobalHurryModifier() const
{
	return m_iGlobalHurryModifier;
}

int CvBuildingInfo::getFreeExperience() const
{
	return m_iFreeExperience;
}

int CvBuildingInfo::getGlobalFreeExperience() const
{
	return m_iGlobalFreeExperience;
}

int CvBuildingInfo::getFoodKept() const
{
	return m_iFoodKept;
}
//doto culture safe zone
int CvBuildingInfo::getNoForeignCityCultureGrowth() const
{
	return m_iNoForeignCityCultureGrowth;
}
//doto culture safe zone
int CvBuildingInfo::getAirlift() const
{
	return m_iAirlift;
}

int CvBuildingInfo::getAirModifier() const
{
	return m_iAirModifier;
}

int CvBuildingInfo::getAirUnitCapacity() const
{
	return m_iAirUnitCapacity;
}

int CvBuildingInfo::getWonderCapacityIncrement() const
{
	return m_iWonderCapacityIncrement;
}

int CvBuildingInfo::getNukeModifier() const
{
	return m_iNukeModifier;
}

int CvBuildingInfo::getNukeExplosionRand() const
{
	return m_iNukeExplosionRand;
}

int CvBuildingInfo::getFreeSpecialist() const
{
	return m_iFreeSpecialist;
}

int CvBuildingInfo::getAreaFreeSpecialist() const
{
	return m_iAreaFreeSpecialist;
}

int CvBuildingInfo::getGlobalFreeSpecialist() const
{
	return m_iGlobalFreeSpecialist;
}

int CvBuildingInfo::getHappiness() const
{
	return m_iHappiness;
}

int CvBuildingInfo::getAreaHappiness() const
{
	return m_iAreaHappiness;
}

int CvBuildingInfo::getGlobalHappiness() const
{
	return m_iGlobalHappiness;
}

int CvBuildingInfo::getStateReligionHappiness() const
{
	return m_iStateReligionHappiness;
}

int CvBuildingInfo::getStateReligionHealth() const
{
	return m_iStateReligionHealth;
}

int CvBuildingInfo::getWorkerSpeedModifier() const
{
	return m_iWorkerSpeedModifier;
}

int CvBuildingInfo::getMilitaryProductionModifier() const
{
	return m_iMilitaryProductionModifier;
}

int CvBuildingInfo::getSpaceProductionModifier() const
{
	return m_iSpaceProductionModifier;
}

int CvBuildingInfo::getGlobalSpaceProductionModifier() const
{
	return m_iGlobalSpaceProductionModifier;
}

int CvBuildingInfo::getTradeRoutes() const
{
	return m_iTradeRoutes;
}

int CvBuildingInfo::getInterestPercent() const
{
	return m_iInterestPercent;
}

int CvBuildingInfo::getInterestMax() const
{
	return m_iInterestMax;
}

int CvBuildingInfo::getCoastalTradeRoutes() const
{
	return m_iCoastalTradeRoutes;
}

int CvBuildingInfo::getGlobalTradeRoutes() const
{
	return m_iGlobalTradeRoutes;
}

int CvBuildingInfo::getTradeRouteModifier() const
{
	return m_iTradeRouteModifier;
}

int CvBuildingInfo::getForeignTradeRouteModifier() const
{
	return m_iForeignTradeRouteModifier;
}

int CvBuildingInfo::getAssetValue() const
{
	return m_iAssetValue;
}

int CvBuildingInfo::getPowerValue() const
{
	return m_iPowerValue;
}

int CvBuildingInfo::getSpecialBuildingType() const
{
	return m_iSpecialBuildingType;
}

int CvBuildingInfo::getAdvisorType() const
{
	return m_iAdvisorType;
}

/********************************************************************************/
/**		REVDCM									2/16/10				phungus420	*/
/**																				*/
/**		CanConstruct															*/
/********************************************************************************/
int CvBuildingInfo::getPrereqGameOption() const
{
	return m_iPrereqGameOption;
}

int CvBuildingInfo::getNotGameOption() const
{
	return m_iNotGameOption;
}
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

int CvBuildingInfo::getHolyCity() const
{
	return m_iHolyCity;
}

int CvBuildingInfo::getReligionType() const
{
	return m_iReligionType;
}

int CvBuildingInfo::getStateReligion() const
{
	return m_iStateReligion;
}

int CvBuildingInfo::getPrereqReligion() const
{
	return m_iPrereqReligion;
}

int CvBuildingInfo::getPrereqCorporation() const
{
	return m_iPrereqCorporation;
}

int CvBuildingInfo::getFoundsCorporation() const
{
	return m_iFoundsCorporation;
}

int CvBuildingInfo::getGlobalReligionCommerce() const
{
	return m_iGlobalReligionCommerce;
}

int CvBuildingInfo::getGlobalCorporationCommerce() const
{
	return m_iGlobalCorporationCommerce;
}

int CvBuildingInfo::getPrereqAndBonus() const
{
	return m_iPrereqAndBonus;
}

int CvBuildingInfo::getGreatPeopleUnitClass() const
{
	return m_iGreatPeopleUnitClass;
}

int CvBuildingInfo::getGreatPeopleRateChange() const
{
	return m_iGreatPeopleRateChange;
}

int CvBuildingInfo::getConquestProbability() const
{
	return m_iConquestProbability;
}

int CvBuildingInfo::getMaintenanceModifier() const
{
	return m_iMaintenanceModifier;
}

//DPII < Maintenance Modifier >
int CvBuildingInfo::getGlobalMaintenanceModifier() const
{
    return m_iGlobalMaintenanceModifier;
}

int CvBuildingInfo::getAreaMaintenanceModifier() const
{
    return m_iAreaMaintenanceModifier;
}

int CvBuildingInfo::getOtherAreaMaintenanceModifier() const
{
    return m_iOtherAreaMaintenanceModifier;
}

int CvBuildingInfo::getDistanceMaintenanceModifier() const
{
    return m_iDistanceMaintenanceModifier;
}

int CvBuildingInfo::getNumCitiesMaintenanceModifier() const
{
    return m_iNumCitiesMaintenanceModifier;
}

int CvBuildingInfo::getCoastalDistanceMaintenanceModifier() const
{
    return m_iCoastalDistanceMaintenanceModifier;
}

int CvBuildingInfo::getConnectedCityMaintenanceModifier() const
{
    return m_iConnectedCityMaintenanceModifier;
}
//DPII < Maintenance Modifier >

int CvBuildingInfo::getWarWearinessModifier() const
{
	return m_iWarWearinessModifier;
}

int CvBuildingInfo::getGlobalWarWearinessModifier() const
{
	return m_iGlobalWarWearinessModifier;
}

int CvBuildingInfo::getEnemyWarWearinessModifier() const
{
	return m_iEnemyWarWearinessModifier;
}

int CvBuildingInfo::getHealRateChange() const
{
	return m_iHealRateChange;
}

int CvBuildingInfo::getHealth() const
{
	return m_iHealth;
}

int CvBuildingInfo::getAreaHealth() const
{
	return m_iAreaHealth;
}

int CvBuildingInfo::getGlobalHealth() const
{
	return m_iGlobalHealth;
}

int CvBuildingInfo::getGlobalPopulationChange() const
{
	return m_iGlobalPopulationChange;
}

int CvBuildingInfo::getFreeTechs() const
{
	return m_iFreeTechs;
}

TechTypes CvBuildingInfo::getFreeSpecialTech() const
{
	return m_eFreeSpecialTech;
}

int CvBuildingInfo::getDefenseModifier() const
{
	return m_iDefenseModifier;
}

int CvBuildingInfo::getBombardDefenseModifier() const
{
	return m_iBombardDefenseModifier;
}

int CvBuildingInfo::getAllCityDefenseModifier() const
{
	return m_iAllCityDefenseModifier;
}

int CvBuildingInfo::getEspionageDefenseModifier() const
{
	return m_iEspionageDefenseModifier;
}

/********************************************************************************/
/**		REVDCM									4/09/10				phungus420	*/
/**																				*/
/**		Building Effects														*/
/********************************************************************************/
int CvBuildingInfo::getUnitUpgradePriceModifier() const
{
	return m_iUnitUpgradePriceModifier;
}

int CvBuildingInfo::getRevIdxLocal() const
{
	return m_iRevIdxLocal;
}

int CvBuildingInfo::getRevIdxNational() const
{
	return m_iRevIdxNational;
}

int CvBuildingInfo::getRevIdxDistanceModifier() const
{
	return m_iRevIdxDistanceModifier;
}
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

int CvBuildingInfo::getMissionType() const
{
	return m_iMissionType;
}

void CvBuildingInfo::setMissionType(int iNewType)
{
	m_iMissionType = iNewType;
}

int CvBuildingInfo::getVoteSourceType() const
{
	return m_iVoteSourceType;
}

float CvBuildingInfo::getVisibilityPriority() const
{
	return m_fVisibilityPriority;
}

bool CvBuildingInfo::isTeamShare() const
{
	return m_bTeamShare;
}

bool CvBuildingInfo::isAutoBuild() const
{
	return m_bAutoBuild;
}

bool CvBuildingInfo::isWater() const
{
	return m_bWater;
}

bool CvBuildingInfo::isRiver() const
{
	return m_bRiver;
}

bool CvBuildingInfo::isFreshWater() const
{
	return m_bFreshWater;
}

bool CvBuildingInfo::isPower() const
{
	return m_bPower;
}

bool CvBuildingInfo::isNoPower() const
{
	return m_bNoPower;
}

bool CvBuildingInfo::isDirtyPower() const
{
	return m_bDirtyPower;
}

bool CvBuildingInfo::isAreaCleanPower() const
{
	return m_bAreaCleanPower;
}

bool CvBuildingInfo::isNoHolyCity() const
{
	return m_bNoHolyCity;
}

bool CvBuildingInfo::isAreaBorderObstacle() const
{
	return m_bAreaBorderObstacle;
}

bool CvBuildingInfo::isForceTeamVoteEligible() const
{
	return m_bForceTeamVoteEligible;
}

bool CvBuildingInfo::isCapital() const
{
	return m_bCapital;
}

/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
int CvBuildingInfo::getDCMAirbombMission() const
{
	return m_iDCMAirbombMission;
}

/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
bool CvBuildingInfo::isGovernmentCenter() const
{
	return m_bGovernmentCenter;
}

bool CvBuildingInfo::isGoldenAge() const
{
	return m_bGoldenAge;
}

bool CvBuildingInfo::isMapCentering() const
{
	return m_bMapCentering;
}

bool CvBuildingInfo::isNoUnhappiness() const
{
	return m_bNoUnhappiness;
}

bool CvBuildingInfo::isNoUnhealthyPopulation() const
{
	return m_bNoUnhealthyPopulation;
}

bool CvBuildingInfo::isBuildingOnlyHealthy() const
{
	return m_bBuildingOnlyHealthy;
}

bool CvBuildingInfo::isNeverCapture() const
{
	return m_bNeverCapture;
}

bool CvBuildingInfo::isNukeImmune() const
{
	return m_bNukeImmune;
}

bool CvBuildingInfo::isPrereqReligion() const
{
	return m_bPrereqReligion;
}

bool CvBuildingInfo::isFreeFromStateReligion() const
{
	return m_bFreeFromStateReligion;
}

bool CvBuildingInfo::isCenterInCity() const
{
	return m_bCenterInCity;
}

bool CvBuildingInfo::isStateReligion() const
{
	return m_bStateReligion;
}

bool CvBuildingInfo::isAllowsNukes() const
{
	return m_bAllowsNukes;
}

const TCHAR* CvBuildingInfo::getConstructSound() const
{
	return m_szConstructSound;
}

void CvBuildingInfo::setConstructSound(const TCHAR* szVal)
{
	m_szConstructSound = szVal;
}

const TCHAR* CvBuildingInfo::getArtDefineTag() const
{
	return m_szArtDefineTag;
}

void CvBuildingInfo::setArtDefineTag(const TCHAR* szVal)
{
	m_szArtDefineTag = szVal;
}

const TCHAR* CvBuildingInfo::getMovieDefineTag() const
{
	return m_szMovieDefineTag;
}

void CvBuildingInfo::setMovieDefineTag(const TCHAR* szVal)
{
	m_szMovieDefineTag = szVal;
}

CvProperties* CvBuildingInfo::getProperties()
{
	return &m_Properties;
}

CvProperties* CvBuildingInfo::getPropertiesAllCities()
{
	return &m_PropertiesAllCities;
}

CvProperties* CvBuildingInfo::getPrereqMinProperties()
{
	return &m_PrereqMinProperties;
}

CvProperties* CvBuildingInfo::getPrereqMaxProperties()
{
	return &m_PrereqMaxProperties;
}

CvProperties* CvBuildingInfo::getPrereqPlayerMinProperties()
{
	return &m_PrereqPlayerMinProperties;
}

CvProperties* CvBuildingInfo::getPrereqPlayerMaxProperties()
{
	return &m_PrereqPlayerMaxProperties;
}

// Arrays

int CvBuildingInfo::getYieldChange(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piYieldChange ? m_piYieldChange[i] : 0;
}

int* CvBuildingInfo::getYieldChangeArray() const
{
	return m_piYieldChange;
}

int CvBuildingInfo::getYieldModifier(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piYieldModifier ? m_piYieldModifier[i] : 0;
}

int* CvBuildingInfo::getYieldModifierArray() const
{
	return m_piYieldModifier;
}

int CvBuildingInfo::getPowerYieldModifier(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piPowerYieldModifier ? m_piPowerYieldModifier[i] : 0;
}

int* CvBuildingInfo::getPowerYieldModifierArray() const
{
	return m_piPowerYieldModifier;
}

int CvBuildingInfo::getAreaYieldModifier(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piAreaYieldModifier ? m_piAreaYieldModifier[i] : 0;
}

int* CvBuildingInfo::getAreaYieldModifierArray() const
{
	return m_piAreaYieldModifier;
}

int CvBuildingInfo::getGlobalYieldModifier(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piGlobalYieldModifier ? m_piGlobalYieldModifier[i] : 0;
}

int* CvBuildingInfo::getGlobalYieldModifierArray() const
{
	return m_piGlobalYieldModifier;
}

int CvBuildingInfo::getSeaPlotYieldChange(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piSeaPlotYieldChange ? m_piSeaPlotYieldChange[i] : 0;
}

int* CvBuildingInfo::getSeaPlotYieldChangeArray() const
{
	return m_piSeaPlotYieldChange;
}

int CvBuildingInfo::getRiverPlotYieldChange(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piRiverPlotYieldChange ? m_piRiverPlotYieldChange[i] : 0;
}

int* CvBuildingInfo::getRiverPlotYieldChangeArray() const
{
	return m_piRiverPlotYieldChange;
}

int CvBuildingInfo::getGlobalSeaPlotYieldChange(int i) const
{
	FAssertMsg(i < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piGlobalSeaPlotYieldChange ? m_piGlobalSeaPlotYieldChange[i] : 0;
}

int* CvBuildingInfo::getGlobalSeaPlotYieldChangeArray() const
{
	return m_piGlobalSeaPlotYieldChange;
}

int CvBuildingInfo::getCommerceChange(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piCommerceChange ? m_piCommerceChange[i] : 0;
}

int* CvBuildingInfo::getCommerceChangeArray() const
{
	return m_piCommerceChange;
}

int CvBuildingInfo::getObsoleteSafeCommerceChange(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piObsoleteSafeCommerceChange ? m_piObsoleteSafeCommerceChange[i] : 0;
}

int* CvBuildingInfo::getObsoleteSafeCommerceChangeArray() const
{
	return m_piObsoleteSafeCommerceChange;
}

int CvBuildingInfo::getCommerceChangeDoubleTime(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piCommerceChangeDoubleTime ? m_piCommerceChangeDoubleTime[i] : 0;
}

int CvBuildingInfo::getCommerceModifier(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piCommerceModifier ? m_piCommerceModifier[i] : 0;
}

int* CvBuildingInfo::getCommerceModifierArray() const
{
	return m_piCommerceModifier;
}

int CvBuildingInfo::getGlobalCommerceModifier(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piGlobalCommerceModifier ? m_piGlobalCommerceModifier[i] : 0;
}

int* CvBuildingInfo::getGlobalCommerceModifierArray() const
{
	return m_piGlobalCommerceModifier;
}

int CvBuildingInfo::getSpecialistExtraCommerce(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piSpecialistExtraCommerce ? m_piSpecialistExtraCommerce[i] : 0;
}

int* CvBuildingInfo::getSpecialistExtraCommerceArray() const
{
	return m_piSpecialistExtraCommerce;
}

int CvBuildingInfo::getStateReligionCommerce(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piStateReligionCommerce ? m_piStateReligionCommerce[i] : 0;
}

int* CvBuildingInfo::getStateReligionCommerceArray() const
{
	return m_piStateReligionCommerce;
}

int CvBuildingInfo::getCommerceHappiness(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return m_piCommerceHappiness ? 1 : 0;
	}
	else
	{
		return m_piCommerceHappiness ? m_piCommerceHappiness[i] : 0;
	}
}

int CvBuildingInfo::getReligionChange(int i) const
{
	FAssertMsg(i < GC.getNumReligionInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piReligionChange ? m_piReligionChange[i] : 0;
}

int CvBuildingInfo::getSpecialistCount(int i) const
{
	FAssertMsg(i < GC.getNumSpecialistInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return m_piSpecialistCount ? 1 : 0;
	}
	else
	{
		return m_piSpecialistCount ? m_piSpecialistCount[i] : 0;
	}
}

int CvBuildingInfo::getFreeSpecialistCount(int i) const
{
	FAssertMsg(i < GC.getNumSpecialistInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return m_piFreeSpecialistCount ? 1 : 0;
	}
	else
	{
		return m_piFreeSpecialistCount ? m_piFreeSpecialistCount[i] : 0;
	}
}

int CvBuildingInfo::getBonusHealthChanges(int i) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return m_piBonusHealthChanges ? 1 : 0;
	}
	else
	{
		return m_piBonusHealthChanges ? m_piBonusHealthChanges[i] : 0;
	}
}

int CvBuildingInfo::getBonusHappinessChanges(int i) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return m_piBonusHappinessChanges ? 1 : 0;
	}
	else
	{
		return m_piBonusHappinessChanges ? m_piBonusHappinessChanges[i] : 0;
	}
}

int CvBuildingInfo::getBonusProductionModifier(int i) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piBonusProductionModifier ? m_piBonusProductionModifier[i] : 0;
}

int CvBuildingInfo::getUnitCombatFreeExperience(int i) const
{
	FAssertMsg(i < GC.getNumUnitCombatInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return m_piUnitCombatFreeExperience ? 1 : 0;
	}
	else
	{
		return m_piUnitCombatFreeExperience ? m_piUnitCombatFreeExperience[i] : 0;
	}
}

int CvBuildingInfo::getDomainFreeExperience(int i) const
{
	FAssertMsg(i < NUM_DOMAIN_TYPES, "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return m_piDomainFreeExperience ? 1 : 0;
	}
	else
	{
		return m_piDomainFreeExperience ? m_piDomainFreeExperience[i] : 0;
	}
}

int CvBuildingInfo::getDomainProductionModifier(int i) const
{
	FAssertMsg(i < NUM_DOMAIN_TYPES, "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return m_piDomainProductionModifier ? 1 : 0;
	}
	else
	{
		return m_piDomainProductionModifier ? m_piDomainProductionModifier[i] : 0;
	}
}

int CvBuildingInfo::getPrereqAndTechs(int i) const
{
	FAssertMsg(i < GC.getNUM_BUILDING_AND_TECH_PREREQS(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piPrereqAndTechs ? m_piPrereqAndTechs[i] : -1;
}

int CvBuildingInfo::getPrereqOrBonuses(int i) const
{
	FAssertMsg(i < GC.getNUM_BUILDING_PREREQ_OR_BONUSES(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piPrereqOrBonuses ? m_piPrereqOrBonuses[i] : -1;
}

int CvBuildingInfo::getProductionTraits(int i) const
{
	FAssertMsg(i < GC.getNumTraitInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piProductionTraits ? m_piProductionTraits[i] : 0;
}

int CvBuildingInfo::getHappinessTraits(int i) const
{
	FAssertMsg(i < GC.getNumTraitInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piHappinessTraits ? m_piHappinessTraits[i] : 0;
}

int CvBuildingInfo::getBuildingHappinessChanges(int i) const
{
	FAssertMsg(i < GC.getNumBuildingClassInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return m_piBuildingHappinessChanges ? 1 : 0;
	}
	else
	{
		return m_piBuildingHappinessChanges ? m_piBuildingHappinessChanges[i] : 0;
	}
}

int CvBuildingInfo::getPrereqNumOfBuildingClass(int i) const
{
	FAssertMsg(i < GC.getNumBuildingClassInfos(), "Index out of bounds");

	if ( NO_BUILDINGCLASS == i )
	{
		return m_piPrereqNumOfBuildingClass ? 1 : 0;
	}
	else
	{
		return m_piPrereqNumOfBuildingClass ? m_piPrereqNumOfBuildingClass[i] : 0;
	}
}

int CvBuildingInfo::getFlavorValue(int i) const
{
	FAssertMsg(i < GC.getNumFlavorTypes(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piFlavorValue ? m_piFlavorValue[i] : 0;
}

int CvBuildingInfo::getImprovementFreeSpecialist(int i) const
{
	FAssertMsg(i < GC.getNumImprovementInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return m_piImprovementFreeSpecialist ? 1 : 0;
	}
	else
	{
		return m_piImprovementFreeSpecialist ? m_piImprovementFreeSpecialist[i] : 0;
	}
}

bool CvBuildingInfo::isCommerceFlexible(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbCommerceFlexible ? m_pbCommerceFlexible[i] : false;
}

bool CvBuildingInfo::isCommerceChangeOriginalOwner(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbCommerceChangeOriginalOwner ? m_pbCommerceChangeOriginalOwner[i] : false;
}

bool CvBuildingInfo::isBuildingClassNeededInCity(int i) const
{
	FAssertMsg(i < GC.getNumBuildingClassInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return (m_pbBuildingClassNeededInCity != NULL);
	}
	else
	{
		return m_pbBuildingClassNeededInCity ? m_pbBuildingClassNeededInCity[i] : false;
	}
}

int CvBuildingInfo::getSpecialistYieldChange(int i, int j) const
{
	FAssertMsg(i < GC.getNumSpecialistInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");

	return (m_ppaiSpecialistYieldChange && m_ppaiSpecialistYieldChange[i]) ? m_ppaiSpecialistYieldChange[i][j] : 0;
}

int* CvBuildingInfo::getSpecialistYieldChangeArray(int i) const
{
	FAssertMsg(i < GC.getNumSpecialistInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_ppaiSpecialistYieldChange[i];
}

int CvBuildingInfo::getBonusYieldModifier(int i, int j) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return m_ppaiBonusYieldModifier ? 1 : 0;
	}
	else
	{
		FAssertMsg(j < NUM_YIELD_TYPES, "Index out of bounds");
		FAssertMsg(j > -1, "Index out of bounds");
		return (m_ppaiBonusYieldModifier && m_ppaiBonusYieldModifier[i]) ? m_ppaiBonusYieldModifier[i][j] : 0;
	}
}

int* CvBuildingInfo::getBonusYieldModifierArray(int i) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_ppaiBonusYieldModifier == NULL ? NULL : m_ppaiBonusYieldModifier[i];
}

int CvBuildingInfo::getGlobalBuildingCommerceChange(int iBuildingClass, int iCommerce) const
{
	for (std::vector<BuildingCommerceChange>::const_iterator it = m_aGlobalBuildingCommerceChanges.begin(); it != m_aGlobalBuildingCommerceChanges.end(); ++it)
	{
		if ((*it).eBuildingClass == (BuildingClassTypes)iBuildingClass && (*it).eCommerce == (CommerceTypes)iCommerce)
		{
			return (*it).iChange;
		}
	}

	return 0;
}

int CvBuildingInfo::getNumGlobalBuildingCommerceChanges() const
{
	return m_aGlobalBuildingCommerceChanges.size();
}

const TCHAR* CvBuildingInfo::getButton() const
{
/************************************************************************************************/
/* XMLCOPY                                 10/25/07                                MRGENIE      */
/*                                                                                              */
/* Catch non-existing tag                                                                       */
/************************************************************************************************/
	CvString cDefault = CvString::format("").GetCString();
	if (getArtDefineTag() == cDefault)
	{
		return NULL;
	}
/************************************************************************************************/
/* XMLCOPY                                 END                                                  */
/************************************************************************************************/
	const CvArtInfoBuilding * pBuildingArtInfo;
	pBuildingArtInfo = getArtInfo();
	if (pBuildingArtInfo != NULL)
	{
		return pBuildingArtInfo->getButton();
	}
	else
	{
		return NULL;
	}
}

const CvArtInfoBuilding* CvBuildingInfo::getArtInfo() const
{
	return ARTFILEMGR.getBuildingArtInfo(getArtDefineTag());
}

const CvArtInfoMovie* CvBuildingInfo::getMovieInfo() const
{
	const TCHAR* pcTag = getMovieDefineTag();
	if (NULL != pcTag && 0 != _tcscmp(pcTag, "NONE"))
	{
		return ARTFILEMGR.getMovieArtInfo(pcTag);
	}
	else
	{
		return NULL;
	}
}

const TCHAR* CvBuildingInfo::getMovie() const
{
	const CvArtInfoMovie* pArt;
	pArt = getMovieInfo();
	if (pArt != NULL)
	{
		return pArt->getPath();
	}
	else
	{
		return NULL;
	}
}

/************************************************************************************************/
/* Afforess	                  Start		 Last Update: 6/13/10                                    */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
int CvBuildingInfo::getGlobalPopulationgrowthratepercentage() const
{
	return m_iGlobalPopulationgrowthratepercentage;
}
int CvBuildingInfo::getPopulationgrowthratepercentage() const
{
	return m_iPopulationgrowthratepercentage;
}

int CvBuildingInfo::getPrereqVicinityBonus() const
{
	return m_iPrereqVicinityBonus;
}

int CvBuildingInfo::getLineOfSight() const
{
	return m_iLineOfSight;
}

int CvBuildingInfo::getInflationModifier() const
{
	return m_iInflationModifier;
}

int CvBuildingInfo::getFreePromotion_2() const
{
	return m_iFreePromotion_2;
}

int CvBuildingInfo::getFreePromotion_3() const
{
	return m_iFreePromotion_3;
}

int CvBuildingInfo::getInvasionChance() const
{
	return m_iInvasionChance;
}

int CvBuildingInfo::getAdjacentDamagePercent() const
{
	return m_iAdjacentDamagePercent;
}

int CvBuildingInfo::getProductionContinueBuildingClass() const
{
	return m_iProductionContinueBuildingClass;
}

int CvBuildingInfo::getPrereqPopulation() const
{
	return m_iPrereqPopulation;
}

int CvBuildingInfo::getPrereqCultureLevel() const
{
	return m_iPrereqCultureLevel;
}

int CvBuildingInfo::getPrereqAnyoneBuildingClass() const
{
	return m_iPrereqAnyoneBuildingClass;
}

int CvBuildingInfo::getExtendsBuildingClass() const
{
	return m_iExtendsBuildingClass;
}

int CvBuildingInfo::getOccupationTimeModifier() const
{
	return m_iOccupationTimeModifier;
}

int CvBuildingInfo::getNoEntryDefenseLevel() const
{
	return m_iNoEntryDefenseLevel;
}

int CvBuildingInfo::getNumUnitFullHeal() const
{
	return m_iNumUnitFullHeal;
}

int CvBuildingInfo::getPillageGoldModifier() const
{
	return m_iPillageGoldModifier;
}

int CvBuildingInfo::getWorldTradeRoutes() const
{
	return m_iWorldTradeRoutes;
}

int CvBuildingInfo::getNumPopulationEmployed() const
{
	return m_iNumPopulationEmployed;
}

int CvBuildingInfo::getHealthPercentPerPopulation() const
{
	return m_iHealthPercentPerPopulation;
}

int CvBuildingInfo::getHappinessPercentPerPopulation() const
{
	return m_iHappinessPercentPerPopulation;
}

int CvBuildingInfo::getExtraCapitalCommerce() const
{
	return m_iCapitalExtraCommerce;
}

int CvBuildingInfo::getExtraForeignCapitalCommerce() const
{
	return m_iForeignCapitalExtraCommerce;
}

int CvBuildingInfo::getDomesticConnectedCommerce() const
{
	return m_iDomesticConnectedCommerce;
}

int CvBuildingInfo::getCitiesRequiredPerDomesticConnectedCommerce() const
{
	return m_iCitiesRequiredPerDomesticConnectedCity;
}

int CvBuildingInfo::getMaxDomesticConnectedCommerce() const
{
	return m_iMaxDomesticConnectedCommerce;
}

int CvBuildingInfo::getForeignConnectedCommerce() const
{
	return m_iForeignConnectedCommerce;
}

int CvBuildingInfo::getCitiesRequiredPerForeignConnectedCommerce() const
{
	return m_iCitiesRequiredPerForeignConnectedCity;
}

int CvBuildingInfo::getMaxForeignConnectedCommerce() const
{
	return m_iMaxForeignConnectedCommerce;
}

bool CvBuildingInfo::isProtectedCulture() const
{
	return m_bProtectedCulture;
}

bool CvBuildingInfo::isAllowsAmbassadors() const
{
	return m_bAllowsAmbassadors;
}

bool CvBuildingInfo::isApplyFreePromotionOnMove() const
{
	return m_bApplyFreePromotionOnMove;
}

bool CvBuildingInfo::isBuildOnlyOnPeaks() const
{
	return m_bBuildOnlyOnPeaks;
}

bool CvBuildingInfo::isPrereqPower() const
{
	return m_bPrereqPower;
}

bool CvBuildingInfo::isNoEnemyPillagingIncome() const
{
	return m_bNoEnemyPillagingIncome;
}

bool CvBuildingInfo::isProvidesFreshWater() const
{
	return m_bProvidesFreshWater;
}

bool CvBuildingInfo::isForceAllTradeRoutes() const
{
	return m_bForceAllTradeRoutes;
}

bool CvBuildingInfo::isForceNoPrereqScaling() const
{
	return m_bForceNoPrereqScaling;
}

bool CvBuildingInfo::isPrereqWar() const
{
	return m_bPrereqWar;
}

bool CvBuildingInfo::isRequiresActiveCivics() const
{
	return m_bRequiresActiveCivics;
}

bool CvBuildingInfo::isZoneOfControl() const
{
	return m_bZoneOfControl;
}

int CvBuildingInfo::getDiplomacyChange() const
{
	return m_iDiplomacyChange;
}

int CvBuildingInfo::getMemoryTypeModifier(int i) const
{
	FAssertMsg(i < NUM_MEMORY_TYPES, "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if (i == -1)
	{
		return m_piMemoryTypeModifier != NULL;
	}
	return m_piMemoryTypeModifier != NULL ? m_piMemoryTypeModifier[i] : 0;
}

bool CvBuildingInfo::isPrereqOrBuildingClass(int i) const
{
	FAssertMsg(i < GC.getNumBuildingClassInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return (m_pbPrereqOrBuildingClass != NULL);
	}
	return m_pbPrereqOrBuildingClass ? m_pbPrereqOrBuildingClass[i] : false;
}

bool CvBuildingInfo::isPrereqOrGameSpeed(int i) const
{
	FAssertMsg(i < GC.getNumGameSpeedInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbPrereqOrGameSpeed ? m_pbPrereqOrGameSpeed[i] : false;
}

bool CvBuildingInfo::isPrereqOrCivics(int i) const
{
	FAssertMsg(i < GC.getNumCivicInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbPrereqOrCivics ? m_pbPrereqOrCivics[i] : false;
}

bool CvBuildingInfo::isPrereqAndCivics(int i) const
{
	FAssertMsg(i < GC.getNumCivicInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbPrereqAndCivics ? m_pbPrereqAndCivics[i] : false;
}

//This is for the readpass3
int CvBuildingInfo::isPrereqOrCivicsVectorSize()					{return m_aszPrereqOrCivicsforPass3.size();}
CvString CvBuildingInfo::isPrereqOrCivicsNamesVectorElement(int i)	{return m_aszPrereqOrCivicsforPass3[i];}
int CvBuildingInfo::isPrereqOrCivicsValuesVectorElement(int i)		{return m_abPrereqOrCivicsforPass3[i];}
int CvBuildingInfo::isPrereqAndCivicsVectorSize()					{return m_aszPrereqAndCivicsforPass3.size();}
CvString CvBuildingInfo::isPrereqAndCivicsNamesVectorElement(int i)	{return m_aszPrereqAndCivicsforPass3[i];}
int CvBuildingInfo::isPrereqAndCivicsValuesVectorElement(int i)		{return m_abPrereqAndCivicsforPass3[i];}

bool CvBuildingInfo::isPrereqOrTerrain(int i) const
{
	FAssertMsg(i < GC.getNumTerrainInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbPrereqOrTerrain ? m_pbPrereqOrTerrain[i] : false;
}

bool CvBuildingInfo::isPrereqAndTerrain(int i) const
{
	FAssertMsg(i < GC.getNumTerrainInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbPrereqAndTerrain ? m_pbPrereqAndTerrain[i] : false;
}

bool CvBuildingInfo::isPrereqOrImprovement(int i) const
{
	FAssertMsg(i < GC.getNumImprovementInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbPrereqOrImprovement ? m_pbPrereqOrImprovement[i] : false;
}

bool CvBuildingInfo::isPrereqOrFeature(int i) const
{
	FAssertMsg(i < GC.getNumFeatureInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pbPrereqOrFeature ? m_pbPrereqOrFeature[i] : false;
}

bool CvBuildingInfo::isReplaceBuildingClass(int i) const
{
	FAssertMsg(i < GC.getNumBuildingClassInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return (m_pbReplaceBuildingClass != NULL);
	}
	else
	{
		return m_pbReplaceBuildingClass ? m_pbReplaceBuildingClass[i] : false;
	}
}

bool CvBuildingInfo::isPrereqNotBuildingClass(int i) const
{
	FAssertMsg(i < GC.getNumBuildingClassInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return (m_pbPrereqNotBuildingClass != NULL);
	}
	else
	{
		return m_pbPrereqNotBuildingClass ? m_pbPrereqNotBuildingClass[i] : false;
	}
}

int CvBuildingInfo::getPrereqOrVicinityBonuses(int i) const
{
	FAssertMsg(i < GC.getNUM_BUILDING_PREREQ_OR_BONUSES(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piPrereqOrVicinityBonuses ? m_piPrereqOrVicinityBonuses[i] : -1;
}

int CvBuildingInfo::getBonusDefenseChanges(int i) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piBonusDefenseChanges ? m_piBonusDefenseChanges[i] : 0;
}

int CvBuildingInfo::getUnitClassProductionModifier(int i) const
{
	FAssertMsg(i < GC.getNumUnitClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piUnitClassProductionModifier ? m_piUnitClassProductionModifier[i] : 0;
}

int CvBuildingInfo::getBuildingClassProductionModifier(int i) const
{
	FAssertMsg(i < GC.getNumBuildingClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piBuildingClassProductionModifier ? m_piBuildingClassProductionModifier[i] : 0;
}

int CvBuildingInfo::getGlobalBuildingClassProductionModifier(int i) const
{
	FAssertMsg(i < GC.getNumBuildingClassInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piGlobalBuildingClassProductionModifier ? m_piGlobalBuildingClassProductionModifier[i] : 0;
}

int CvBuildingInfo::getTechHappinessChanges(int i) const
{
	FAssertMsg(i < GC.getNumTechInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return (m_piTechHappinessChanges == NULL) ? 0 : 1;
	}
	else
	{
		return m_piTechHappinessChanges ? m_piTechHappinessChanges[i] : 0;
	}
}

int CvBuildingInfo::getTechHealthChanges(int i) const
{
	FAssertMsg(i < GC.getNumTechInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return m_piTechHealthChanges ? 1 : 0;
	}
	else
	{
		return m_piTechHealthChanges ? m_piTechHealthChanges[i] : 0;
	}
}

int CvBuildingInfo::getCommerceAttacks(int i) const
{
	FAssertMsg(i < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_piCommerceAttacks ? m_piCommerceAttacks[i] : 0;
}

int* CvBuildingInfo::getCommerceAttacksArray() const
{
	return m_piCommerceAttacks;
}

int CvBuildingInfo::getBonusCommerceModifier(int i, int j) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return (m_ppaiBonusCommerceModifier && m_ppaiBonusCommerceModifier[i]) ? m_ppaiBonusCommerceModifier[i][j] : 0;
}

int* CvBuildingInfo::getBonusCommerceModifierArray(int i) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_ppaiBonusCommerceModifier[i];
}

int CvBuildingInfo::getBonusYieldChanges(int i, int j) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return (m_ppaiBonusYieldChanges ? 1 : 0);
	}
	else
	{
		FAssertMsg(j < NUM_YIELD_TYPES, "Index out of bounds");
		FAssertMsg(j > -1, "Index out of bounds");
		return (m_ppaiBonusYieldChanges && m_ppaiBonusYieldChanges[i]) ? m_ppaiBonusYieldChanges[i][j] : 0;
	}
}

int* CvBuildingInfo::getBonusYieldChangesArray(int i) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_ppaiBonusYieldChanges[i];
}

int CvBuildingInfo::getBonusCommercePercentChanges(int i, int j) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < NUM_COMMERCE_TYPES, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return (m_ppaiBonusCommercePercentChanges && m_ppaiBonusCommercePercentChanges[i]) ? m_ppaiBonusCommercePercentChanges[i][j] : 0;
}

int* CvBuildingInfo::getBonusCommercePercentChangesArray(int i) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_ppaiBonusCommercePercentChanges[i];
}

int CvBuildingInfo::getVicinityBonusYieldChanges(int i, int j) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return m_ppaiVicinityBonusYieldChanges ? 1 : 0;
	}
	else
	{
		FAssertMsg(j < NUM_YIELD_TYPES, "Index out of bounds");
		FAssertMsg(j > -1, "Index out of bounds");
		return (m_ppaiVicinityBonusYieldChanges && m_ppaiVicinityBonusYieldChanges[i]) ? m_ppaiVicinityBonusYieldChanges[i][j] : 0;
	}
}

int* CvBuildingInfo::getVicinityBonusYieldChangesArray(int i) const
{
	FAssertMsg(i < GC.getNumBonusInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_ppaiVicinityBonusYieldChanges[i];
}

int CvBuildingInfo::getTechCommerceChange(int i, int j) const
{
	FAssertMsg(i < GC.getNumTechInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return m_ppaiTechCommerceChange ? 1 : 0;
	}
	else
	{
		FAssertMsg(j < NUM_COMMERCE_TYPES, "Index out of bounds");
		FAssertMsg(j > -1, "Index out of bounds");
		return (m_ppaiTechCommerceChange && m_ppaiTechCommerceChange[i]) ? m_ppaiTechCommerceChange[i][j] : 0;
	}
}

int* CvBuildingInfo::getTechCommerceChangeArray(int i)
{
	return m_ppaiTechCommerceChange[i];
}

int CvBuildingInfo::getTechYieldChange(int i, int j) const
{
	FAssertMsg(i < GC.getNumTechInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return (m_ppaiTechYieldChange ? 1 : 0);
	}
	else
	{
		FAssertMsg(j < NUM_YIELD_TYPES, "Index out of bounds");
		FAssertMsg(j > -1, "Index out of bounds");
		return (m_ppaiTechYieldChange && m_ppaiTechYieldChange[i]) ? m_ppaiTechYieldChange[i][j] : 0;
	}
}

int* CvBuildingInfo::getTechYieldChangeArray(int i)
{
	return (m_ppaiTechYieldChange ? m_ppaiTechYieldChange[i] : NULL);
}

int CvBuildingInfo::getTechSpecialistChange(int i, int j) const
{
	FAssertMsg(i < GC.getNumTechInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return (m_ppaiTechSpecialistChange ? 1 : 0);
	}
	else
	{
		FAssertMsg(j < GC.getNumSpecialistInfos(), "Index out of bounds");
		FAssertMsg(j > -1, "Index out of bounds");
		return (m_ppaiTechSpecialistChange && m_ppaiTechSpecialistChange[i]) ? m_ppaiTechSpecialistChange[i][j] : 0;
	}
}

int* CvBuildingInfo::getTechSpecialistChangeArray(int i)
{
	return m_ppaiTechSpecialistChange[i];
}

int CvBuildingInfo::getTechCommerceModifier(int i, int j) const
{
	FAssertMsg(i < GC.getNumTechInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return (m_ppaiTechCommerceModifier ? 1 : 0);
	}
	else
	{
		FAssertMsg(j < NUM_COMMERCE_TYPES, "Index out of bounds");
		FAssertMsg(j > -1, "Index out of bounds");
		return (m_ppaiTechCommerceModifier && m_ppaiTechCommerceModifier[i]) ? m_ppaiTechCommerceModifier[i][j] : 0;
	}
}

int* CvBuildingInfo::getTechCommerceModifierArray(int i)
{
	return m_ppaiTechCommerceModifier[i];
}

int CvBuildingInfo::getTechYieldModifier(int i, int j) const
{
	FAssertMsg(i < GC.getNumTechInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
//	FAssertMsg(j < NUM_YIELD_TYPES, "Index out of bounds");
//	FAssertMsg(j > -1, "Index out of bounds");
//	return (m_ppaiTechYieldModifier && m_ppaiTechYieldModifier[i]) ? m_ppaiTechYieldModifier[i][j] : 0;
	if ( i == -1 )
	{
		return (m_ppaiTechYieldModifier ? 1 : 0);
	}
	else
	{
		FAssertMsg(j < NUM_YIELD_TYPES, "Index out of bounds");
		FAssertMsg(j > -1, "Index out of bounds");
		return (m_ppaiTechYieldModifier && m_ppaiTechYieldModifier[i]) ? m_ppaiTechYieldModifier[i][j] : 0;
	}
}

int* CvBuildingInfo::getTechYieldModifierArray(int i)
{
	return m_ppaiTechYieldModifier[i];
}

int CvBuildingInfo::getImprovementYieldChanges(int i, int j) const
{
	FAssertMsg(i < GC.getNumImprovementInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < NUM_YIELD_TYPES, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return (m_ppiImprovementYieldChanges && m_ppiImprovementYieldChanges[i]) ? m_ppiImprovementYieldChanges[i][j] : 0;
}

int CvBuildingInfo::getUnitCombatExtraStrength(int i) const
{
	FAssertMsg(i < GC.getNumUnitCombatInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if ( i == -1 )
	{
		return m_piUnitCombatExtraStrength ? 1 : 0;
	}
	else
	{
		return m_piUnitCombatExtraStrength ? m_piUnitCombatExtraStrength[i] : 0;
	}
}

int CvBuildingInfo::getSpecialistCommercePercentChanges(int i, int j) const
{
	FAssertMsg(i < GC.getNumSpecialistInfos(), "Index out of bounds");
	FAssertMsg(i >= -1, "Index out of bounds");

	if (i == -1)
	{
		return m_ppaiSpecialistCommercePercentChanges ? 1 : 0;
	}
	else
	{
		return (m_ppaiSpecialistCommercePercentChanges && m_ppaiSpecialistCommercePercentChanges[i]) ? m_ppaiSpecialistCommercePercentChanges[i][j] : 0;
	}
}

/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
bool CvBuildingInfo::isHurry(int i) const
{
	FAssertMsg(i < GC.getNumHurryInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_pabHurry ? m_pabHurry[i] : false;
}
bool CvBuildingInfo::EnablesOtherBuildings() const
{
	if ( !m_bEnablesOtherBuildingsCalculated )
	{
		int iId = getBuildingClassType();

		m_bEnablesOtherBuildingsCalculated = true;
		m_bEnablesOtherBuildingsValue = false;

		for (int iJ = 0; iJ < GC.getNumBuildingInfos(); iJ++)
		{
			if (GC.getBuildingInfo((BuildingTypes)iJ).isBuildingClassNeededInCity(iId) ||
				GC.getBuildingInfo((BuildingTypes)iJ).isPrereqOrBuildingClass(iId))
			{
				m_bEnablesOtherBuildingsValue = true;
				break;
			}
		}

		if ( !m_bEnablesOtherBuildingsValue )
		{
			BonusTypes eFreeBonus = (BonusTypes)getFreeBonus();

			if ( eFreeBonus != NO_BONUS )
			{
				for (int iJ = 0; iJ < GC.getNumBuildingInfos() && !m_bEnablesOtherBuildingsValue; iJ++)
				{
					if (GC.getBuildingInfo((BuildingTypes)iJ).getPrereqAndBonus() == eFreeBonus)
					{
						m_bEnablesOtherBuildingsValue = true;
						break;
					}

					for(int iI = 0; iI < GC.getNUM_BUILDING_PREREQ_OR_BONUSES(); iI++)
					{
						if ( GC.getBuildingInfo((BuildingTypes)iJ).getPrereqOrBonuses(iI) == eFreeBonus )
						{
							m_bEnablesOtherBuildingsValue = true;
							break;
						}
					}
				}
			}
		}
	}

	return m_bEnablesOtherBuildingsValue;
}

bool CvBuildingInfo::isFreeBonusOfBuilding(BonusTypes eBonus) const
{
	if ( getFreeBonus() == eBonus )
	{
		return true;
	}

	for( int iJ = 0; iJ < getNumExtraFreeBonuses(); iJ++ )
	{
		if ( getExtraFreeBonus(iJ) == eBonus )
		{
			return true;
		}
	}

	return false;
}

bool CvBuildingInfo::EnablesUnits() const
{
	if ( !m_bEnablesUnitsCalculated )
	{
		int eBuilding = GC.getInfoTypeForString(m_szType);

		m_bEnablesUnitsCalculated = true;
		m_bEnablesUnits = false;

		for (int iI = 0; iI < GC.getNumUnitInfos(); iI++)
		{
			CvUnitInfo& kUnit = GC.getUnitInfo((UnitTypes)iI);

			if (kUnit.getPrereqBuilding() == eBuilding)
			{
				m_bEnablesUnits = true;
				break;
			}

			if ( kUnit.getPrereqAndBonus() != NO_BONUS )
			{
				if ( isFreeBonusOfBuilding( (BonusTypes)kUnit.getPrereqAndBonus()) )
				{
					m_bEnablesUnits = true;
					break;
				}
			}

			for (int iK = 0; iK < GC.getNUM_UNIT_PREREQ_OR_BONUSES(); iK++)
			{
				BonusTypes eXtraFreeBonus = (BonusTypes)kUnit.getPrereqOrBonuses(iK);

				if ( eXtraFreeBonus != NO_BONUS )
				{
					if ( isFreeBonusOfBuilding( eXtraFreeBonus ) )
					{
						m_bEnablesUnits = true;
						break;
					}
				}
			}

			if ( m_bEnablesUnits )
			{
				break;
			}
		}
	}

	return m_bEnablesUnits;
}

//
// serialization
//
void CvBuildingInfo::read(FDataStreamBase* stream)
{
	CvHotkeyInfo::read(stream);

	uint uiFlag=0;
	stream->Read(&uiFlag);	// flags for expansion

	stream->Read(&m_iBuildingClassType);
	stream->Read(&m_iVictoryPrereq);
	stream->Read(&m_iFreeStartEra);
	stream->Read(&m_iMaxStartEra);
	stream->Read(&m_iObsoleteTech);
	stream->Read(&m_iPrereqAndTech);
	stream->Read(&m_iNoBonus);
	stream->Read(&m_iPowerBonus);
	stream->Read(&m_iFreeBonus);
	stream->Read(&m_iNumFreeBonuses);
	int iSize = 0;
	stream->Read(&iSize);
	for (int iI=0; iI<iSize; iI++)
	{
		int iBonus = 0;
		int iNum = 0;
		stream->Read(&iBonus);
		stream->Read(&iNum);
		m_aExtraFreeBonuses.push_back(std::pair<BonusTypes,int>((BonusTypes)iBonus, iNum));
	}
    
    // DarkLunaPhantom - FreeBuilding accepts lists.
    int iNumBuildingClasses;
    m_aiFreeBuildingClass.clear();
	stream->Read(&iNumBuildingClasses);
	for (int iI = 0; iI < iNumBuildingClasses; ++iI)
	{
		int iFreeBuildingClass = 0;
		stream->Read(&iFreeBuildingClass);
		m_aiFreeBuildingClass.push_back(iFreeBuildingClass);
	}
    
    // DarkLunaPhantom - Extra FreeBuilding entries.
    stream->Read(&iNumBuildingClasses);
    m_aiExtraFreeBuildingClass.clear();
    m_abExtraFreeBuildingConnected.clear();
    m_abExtraFreeBuildingContinuous.clear();
    for (int iI = 0; iI < iNumBuildingClasses; ++iI)
	{
		int iExtraFreeBuildingClass;
		stream->Read(&iExtraFreeBuildingClass);
		m_aiExtraFreeBuildingClass.push_back(iExtraFreeBuildingClass);
        
        bool bBool;
        stream->Read(&bBool);
        m_abExtraFreeBuildingConnected.push_back(bBool);
        
        stream->Read(&bBool);
        m_abExtraFreeBuildingContinuous.push_back(bBool);
	}
    
	stream->Read(&m_iFreeAreaBuildingClass);
	stream->Read(&m_iFreeTradeRegionBuildingClass);
	stream->Read(&m_iFreePromotion);
	stream->Read(&m_iCivicOption);
	stream->Read(&m_iAIWeight);
	stream->Read(&m_iProductionCost);
	stream->Read(&m_iHurryCostModifier);
	stream->Read(&m_iHurryAngerModifier);
	stream->Read(&m_iAdvancedStartCost);
	stream->Read(&m_iAdvancedStartCostIncrease);
	stream->Read(&m_iMinAreaSize);
	stream->Read(&m_iNumCitiesPrereq);
	stream->Read(&m_iNumTeamsPrereq);
	stream->Read(&m_iUnitLevelPrereq);
	stream->Read(&m_iMinLatitude);
	stream->Read(&m_iMaxLatitude);
	stream->Read(&m_iGreatPeopleRateModifier);
	stream->Read(&m_iGreatGeneralRateModifier);
	stream->Read(&m_iDomesticGreatGeneralRateModifier);
	stream->Read(&m_iGlobalGreatPeopleRateModifier);
	stream->Read(&m_iAnarchyModifier);
	stream->Read(&m_iGoldenAgeModifier);
	stream->Read(&m_iGlobalHurryModifier);
	stream->Read(&m_iFreeExperience);
	stream->Read(&m_iGlobalFreeExperience);
	stream->Read(&m_iFoodKept);
	//doto culture safe zone
	stream->Read(&m_iNoForeignCityCultureGrowth);
	//doto culture safe zone
	stream->Read(&m_iAirlift);
	stream->Read(&m_iAirModifier);
	stream->Read(&m_iAirUnitCapacity);
	stream->Read(&m_iWonderCapacityIncrement);
	stream->Read(&m_iNukeModifier);
	stream->Read(&m_iNukeExplosionRand);
	stream->Read(&m_iFreeSpecialist);
	stream->Read(&m_iAreaFreeSpecialist);
	stream->Read(&m_iGlobalFreeSpecialist);
	stream->Read(&m_iHappiness);
	stream->Read(&m_iAreaHappiness);
	stream->Read(&m_iGlobalHappiness);
	stream->Read(&m_iStateReligionHappiness);
	stream->Read(&m_iStateReligionHealth);
	stream->Read(&m_iWorkerSpeedModifier);
	stream->Read(&m_iMilitaryProductionModifier);
	stream->Read(&m_iSpaceProductionModifier);
	stream->Read(&m_iGlobalSpaceProductionModifier);
	stream->Read(&m_iTradeRoutes);
	stream->Read(&m_iInterestPercent);
	stream->Read(&m_iInterestMax);
	stream->Read(&m_iCoastalTradeRoutes);
	stream->Read(&m_iGlobalTradeRoutes);
	stream->Read(&m_iTradeRouteModifier);
	stream->Read(&m_iForeignTradeRouteModifier);
	stream->Read(&m_iAssetValue);
	stream->Read(&m_iPowerValue);
	stream->Read(&m_iSpecialBuildingType);
	stream->Read(&m_iAdvisorType);

	/********************************************************************************/
	/**		REVDCM									2/16/10				phungus420	*/
	/**																				*/
	/**		CanConstruct															*/
	/********************************************************************************/
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
	stream->Read(&m_iFoundsCorporation);
	stream->Read(&m_iGlobalReligionCommerce);
	stream->Read(&m_iGlobalCorporationCommerce);
	stream->Read(&m_iPrereqAndBonus);
	stream->Read(&m_iGreatPeopleUnitClass);
	stream->Read(&m_iGreatPeopleRateChange);
	stream->Read(&m_iConquestProbability);
	stream->Read(&m_iMaintenanceModifier);
	//DPII < Maintenance Modifier >
	stream->Read(&m_iGlobalMaintenanceModifier);
	stream->Read(&m_iAreaMaintenanceModifier);
	stream->Read(&m_iAreaMaintenanceModifier);
	stream->Read(&m_iDistanceMaintenanceModifier);
	stream->Read(&m_iNumCitiesMaintenanceModifier);
	stream->Read(&m_iDistanceMaintenanceModifier);
	stream->Read(&m_iConnectedCityMaintenanceModifier);
	//DPII < Maintenance Modifier >
	stream->Read(&m_iWarWearinessModifier);
	stream->Read(&m_iGlobalWarWearinessModifier);
	stream->Read(&m_iEnemyWarWearinessModifier);
	stream->Read(&m_iHealRateChange);
	stream->Read(&m_iHealth);
	stream->Read(&m_iAreaHealth);
	stream->Read(&m_iGlobalHealth);
	stream->Read(&m_iGlobalPopulationChange);
	stream->Read(&m_iFreeTechs);
	stream->Read((int*)&m_eFreeSpecialTech);
	stream->Read(&m_iDefenseModifier);
	stream->Read(&m_iBombardDefenseModifier);
	stream->Read(&m_iAllCityDefenseModifier);
	stream->Read(&m_iEspionageDefenseModifier);

	/********************************************************************************/
	/**		REVDCM									4/09/10				phungus420	*/
	/**																				*/
	/**		Building Effects														*/
	/********************************************************************************/
	stream->Read(&m_iUnitUpgradePriceModifier);
	stream->Read(&m_iRevIdxLocal);
	stream->Read(&m_iRevIdxNational);
	stream->Read(&m_iRevIdxDistanceModifier);
	/********************************************************************************/
	/**		REVDCM									END								*/
	/********************************************************************************/

	stream->Read(&m_iMissionType);
	stream->Read(&m_iVoteSourceType);

	stream->Read(&m_fVisibilityPriority);

	stream->Read(&m_bTeamShare);
	stream->Read(&m_bAutoBuild);
	stream->Read(&m_bWater);
	stream->Read(&m_bRiver);
	stream->Read(&m_bFreshWater);
	stream->Read(&m_bPower);
	stream->Read(&m_bNoPower);
	stream->Read(&m_bDirtyPower);
	stream->Read(&m_bAreaCleanPower);
	stream->Read(&m_bNoHolyCity);
	stream->Read(&m_bAreaBorderObstacle);
	stream->Read(&m_bForceTeamVoteEligible);
	stream->Read(&m_bCapital);
	/************************************************************************************************/
	/* DCM                                     04/19/09                                Johny Smith  */
	/************************************************************************************************/
	// Dale - AB: Bombing START
	stream->Read(&m_iDCMAirbombMission);
	// Dale - AB: Bombing END
	/************************************************************************************************/
	/* DCM                                     END                                                  */
	/************************************************************************************************/
	stream->Read(&m_bGovernmentCenter);
	stream->Read(&m_bGoldenAge);
	stream->Read(&m_bMapCentering);
	stream->Read(&m_bNoUnhappiness);
	stream->Read(&m_bNoUnhealthyPopulation);
	stream->Read(&m_bBuildingOnlyHealthy);
	stream->Read(&m_bNeverCapture);
	stream->Read(&m_bNukeImmune);
	stream->Read(&m_bPrereqReligion);
	stream->Read(&m_bFreeFromStateReligion);
	stream->Read(&m_bCenterInCity);
	stream->Read(&m_bStateReligion);
	stream->Read(&m_bAllowsNukes);

	stream->ReadString(m_szConstructSound);
	stream->ReadString(m_szArtDefineTag);
	stream->ReadString(m_szMovieDefineTag);

	bool bPresent = false;

	SAFE_DELETE_ARRAY(m_piPrereqAndTechs);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piPrereqAndTechs = new int[GC.getNUM_BUILDING_AND_TECH_PREREQS()];
		stream->Read(GC.getNUM_BUILDING_AND_TECH_PREREQS(), m_piPrereqAndTechs);
	}

	SAFE_DELETE_ARRAY(m_piPrereqOrBonuses);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piPrereqOrBonuses = new int[GC.getNUM_BUILDING_PREREQ_OR_BONUSES()];
		stream->Read(GC.getNUM_BUILDING_PREREQ_OR_BONUSES(), m_piPrereqOrBonuses);
	}

	SAFE_DELETE_ARRAY(m_piProductionTraits);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piProductionTraits = new int[GC.getNumTraitInfos()];
		stream->Read(GC.getNumTraitInfos(), m_piProductionTraits);
	}

	SAFE_DELETE_ARRAY(m_piHappinessTraits);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piHappinessTraits = new int[GC.getNumTraitInfos()];
		stream->Read(GC.getNumTraitInfos(), m_piHappinessTraits);
	}

	SAFE_DELETE_ARRAY(m_piSeaPlotYieldChange);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piSeaPlotYieldChange = new int[NUM_YIELD_TYPES];
		stream->Read(NUM_YIELD_TYPES, m_piSeaPlotYieldChange);
	}

	SAFE_DELETE_ARRAY(m_piRiverPlotYieldChange);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piRiverPlotYieldChange = new int[NUM_YIELD_TYPES];
		stream->Read(NUM_YIELD_TYPES, m_piRiverPlotYieldChange);
	}

	SAFE_DELETE_ARRAY(m_piGlobalSeaPlotYieldChange);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piGlobalSeaPlotYieldChange = new int[NUM_YIELD_TYPES];
		stream->Read(NUM_YIELD_TYPES, m_piGlobalSeaPlotYieldChange);
	}

	SAFE_DELETE_ARRAY(m_piYieldChange);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piYieldChange = new int[NUM_YIELD_TYPES];
		stream->Read(NUM_YIELD_TYPES, m_piYieldChange);
	}

	SAFE_DELETE_ARRAY(m_piYieldModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piYieldModifier = new int[NUM_YIELD_TYPES];
		stream->Read(NUM_YIELD_TYPES, m_piYieldModifier);
	}

	SAFE_DELETE_ARRAY(m_piPowerYieldModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piPowerYieldModifier = new int[NUM_YIELD_TYPES];
		stream->Read(NUM_YIELD_TYPES, m_piPowerYieldModifier);
	}

	SAFE_DELETE_ARRAY(m_piAreaYieldModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piAreaYieldModifier = new int[NUM_YIELD_TYPES];
		stream->Read(NUM_YIELD_TYPES, m_piAreaYieldModifier);
	}

	SAFE_DELETE_ARRAY(m_piGlobalYieldModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piGlobalYieldModifier = new int[NUM_YIELD_TYPES];
		stream->Read(NUM_YIELD_TYPES, m_piGlobalYieldModifier);
	}

	SAFE_DELETE_ARRAY(m_piCommerceChange);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piCommerceChange = new int[NUM_COMMERCE_TYPES];
		stream->Read(NUM_COMMERCE_TYPES, m_piCommerceChange);
	}

	SAFE_DELETE_ARRAY(m_piObsoleteSafeCommerceChange);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piObsoleteSafeCommerceChange = new int[NUM_COMMERCE_TYPES];
		stream->Read(NUM_COMMERCE_TYPES, m_piObsoleteSafeCommerceChange);
	}

	SAFE_DELETE_ARRAY(m_piCommerceChangeDoubleTime);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piCommerceChangeDoubleTime = new int[NUM_COMMERCE_TYPES];
		stream->Read(NUM_COMMERCE_TYPES, m_piCommerceChangeDoubleTime);
	}

	SAFE_DELETE_ARRAY(m_piCommerceModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piCommerceModifier = new int[NUM_COMMERCE_TYPES];
		stream->Read(NUM_COMMERCE_TYPES, m_piCommerceModifier);
	}

	SAFE_DELETE_ARRAY(m_piGlobalCommerceModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piGlobalCommerceModifier = new int[NUM_COMMERCE_TYPES];
		stream->Read(NUM_COMMERCE_TYPES, m_piGlobalCommerceModifier);
	}

	SAFE_DELETE_ARRAY(m_piSpecialistExtraCommerce);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piSpecialistExtraCommerce = new int[NUM_COMMERCE_TYPES];
		stream->Read(NUM_COMMERCE_TYPES, m_piSpecialistExtraCommerce);
	}

	SAFE_DELETE_ARRAY(m_piStateReligionCommerce);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piStateReligionCommerce = new int[NUM_COMMERCE_TYPES];
		stream->Read(NUM_COMMERCE_TYPES, m_piStateReligionCommerce);
	}

	SAFE_DELETE_ARRAY(m_piCommerceHappiness);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piCommerceHappiness = new int[NUM_COMMERCE_TYPES];
		stream->Read(NUM_COMMERCE_TYPES, m_piCommerceHappiness);
	}

	SAFE_DELETE_ARRAY(m_piReligionChange);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piReligionChange = new int[GC.getNumReligionInfos()];
		stream->Read(GC.getNumReligionInfos(), m_piReligionChange);
	}

	SAFE_DELETE_ARRAY(m_piSpecialistCount);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piSpecialistCount = new int[GC.getNumSpecialistInfos()];
		stream->Read(GC.getNumSpecialistInfos(), m_piSpecialistCount);
	}

	SAFE_DELETE_ARRAY(m_piFreeSpecialistCount);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piFreeSpecialistCount = new int[GC.getNumSpecialistInfos()];
		stream->Read(GC.getNumSpecialistInfos(), m_piFreeSpecialistCount);
	}

	SAFE_DELETE_ARRAY(m_piBonusHealthChanges);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piBonusHealthChanges = new int[GC.getNumBonusInfos()];
		stream->Read(GC.getNumBonusInfos(), m_piBonusHealthChanges);
	}

	SAFE_DELETE_ARRAY(m_piBonusHappinessChanges);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piBonusHappinessChanges = new int[GC.getNumBonusInfos()];
		stream->Read(GC.getNumBonusInfos(), m_piBonusHappinessChanges);
	}

	SAFE_DELETE_ARRAY(m_piBonusProductionModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piBonusProductionModifier = new int[GC.getNumBonusInfos()];
		stream->Read(GC.getNumBonusInfos(), m_piBonusProductionModifier);
	}

	SAFE_DELETE_ARRAY(m_piUnitCombatFreeExperience);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piUnitCombatFreeExperience = new int[GC.getNumUnitCombatInfos()];
		stream->Read(GC.getNumUnitCombatInfos(), m_piUnitCombatFreeExperience);
	}

	SAFE_DELETE_ARRAY(m_piDomainFreeExperience);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piDomainFreeExperience = new int[NUM_DOMAIN_TYPES];
		stream->Read(NUM_DOMAIN_TYPES, m_piDomainFreeExperience);
	}

	SAFE_DELETE_ARRAY(m_piDomainProductionModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piDomainProductionModifier = new int[NUM_DOMAIN_TYPES];
		stream->Read(NUM_DOMAIN_TYPES, m_piDomainProductionModifier);
	}

	SAFE_DELETE_ARRAY(m_piBuildingHappinessChanges);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piBuildingHappinessChanges = new int[GC.getNumBuildingClassInfos()];
		stream->Read(GC.getNumBuildingClassInfos(), m_piBuildingHappinessChanges);
	}

	SAFE_DELETE_ARRAY(m_piPrereqNumOfBuildingClass);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piPrereqNumOfBuildingClass = new int[GC.getNumBuildingClassInfos()];
		stream->Read(GC.getNumBuildingClassInfos(), m_piPrereqNumOfBuildingClass);
	}

	SAFE_DELETE_ARRAY(m_piFlavorValue);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piFlavorValue = new int[GC.getNumFlavorTypes()];
		stream->Read(GC.getNumFlavorTypes(), m_piFlavorValue);
	}

	SAFE_DELETE_ARRAY(m_piImprovementFreeSpecialist);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piImprovementFreeSpecialist = new int[GC.getNumImprovementInfos()];
		stream->Read(GC.getNumImprovementInfos(), m_piImprovementFreeSpecialist);
	}

	SAFE_DELETE_ARRAY(m_pbCommerceFlexible);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbCommerceFlexible = new bool[NUM_COMMERCE_TYPES];
		stream->Read(NUM_COMMERCE_TYPES, m_pbCommerceFlexible);
	}

	SAFE_DELETE_ARRAY(m_pbCommerceChangeOriginalOwner);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbCommerceChangeOriginalOwner = new bool[NUM_COMMERCE_TYPES];
		stream->Read(NUM_COMMERCE_TYPES, m_pbCommerceChangeOriginalOwner);
	}

	SAFE_DELETE_ARRAY(m_pbBuildingClassNeededInCity);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbBuildingClassNeededInCity = new bool[GC.getNumBuildingClassInfos()];
		stream->Read(GC.getNumBuildingClassInfos(), m_pbBuildingClassNeededInCity);
	}

	int i;
	if (m_ppaiSpecialistYieldChange != NULL)
	{
		for(i=0;i<GC.getNumSpecialistInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiSpecialistYieldChange[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiSpecialistYieldChange);
	}

	stream->Read(&bPresent);
	if (bPresent)
	{
		m_ppaiSpecialistYieldChange = new int*[GC.getNumSpecialistInfos()];
		for(i=0;i<GC.getNumSpecialistInfos();i++)
		{
			stream->Read(&bPresent);
			if (bPresent)
			{
				m_ppaiSpecialistYieldChange[i]  = new int[NUM_YIELD_TYPES];
				stream->Read(NUM_YIELD_TYPES, m_ppaiSpecialistYieldChange[i]);
				m_bAnySpecialistYieldChanges = true;
			}
		}
	}

	if (m_ppaiBonusYieldModifier != NULL)
	{
		for(i=0;i<GC.getNumBonusInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiBonusYieldModifier[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiBonusYieldModifier);
	}

	stream->Read(&bPresent);
	if (bPresent)
	{
		m_ppaiBonusYieldModifier = new int*[GC.getNumBonusInfos()];
		for(i=0;i<GC.getNumBonusInfos();i++)
		{
			stream->Read(&bPresent);
			if (bPresent)
			{
				m_ppaiBonusYieldModifier[i]  = new int[NUM_YIELD_TYPES];
				stream->Read(NUM_YIELD_TYPES, m_ppaiBonusYieldModifier[i]);
				m_bAnyBonusYieldModifiers = true;
			}
		}
	}
	readOptionalArray(stream, m_piSpecialistExtraCommerce);
	/************************************************************************************************/
	/* Afforess	                  Start		 Last Update: 3/8/10                                    */
	/*                                                                                              */
	/*                                                                                              */
	/************************************************************************************************/
	stream->Read(&m_iGlobalPopulationgrowthratepercentage);
	stream->Read(&m_iPopulationgrowthratepercentage);
	stream->Read(&m_iPrereqVicinityBonus);
	stream->Read(&m_iPillageGoldModifier);
	stream->Read(&m_iFreePromotion_2);
	stream->Read(&m_iFreePromotion_3);
	stream->Read(&m_iWorldTradeRoutes);
	stream->Read(&m_iLineOfSight);
	stream->Read(&m_iInflationModifier);
	stream->Read(&m_iInvasionChance);
	stream->Read(&m_iAdjacentDamagePercent);
	stream->Read(&m_iProductionContinueBuildingClass);
	stream->Read(&m_iPrereqPopulation);
	stream->Read(&m_iPrereqCultureLevel);
	stream->Read(&m_iWorkableRadius);
	stream->Read(&m_iPrereqAnyoneBuildingClass);
	stream->Read(&m_iExtendsBuildingClass);
	stream->Read(&m_iOccupationTimeModifier);
	stream->Read(&m_iNoEntryDefenseLevel);
	stream->Read(&m_iNumUnitFullHeal);
	stream->Read(&m_iNumPopulationEmployed);
	stream->Read(&m_iHealthPercentPerPopulation);
	stream->Read(&m_iHappinessPercentPerPopulation);
	stream->Read(&m_iCapitalExtraCommerce);
	stream->Read(&m_iForeignCapitalExtraCommerce);
	stream->Read(&m_iDomesticConnectedCommerce);
	stream->Read(&m_iCitiesRequiredPerDomesticConnectedCity);
	stream->Read(&m_iMaxDomesticConnectedCommerce);
	stream->Read(&m_iForeignConnectedCommerce);
	stream->Read(&m_iCitiesRequiredPerForeignConnectedCity);
	stream->Read(&m_iMaxForeignConnectedCommerce);
	stream->Read(&m_iDiplomacyChange);

	stream->Read(&m_iCrime);
	stream->Read(&m_iCrimePerPop);

	stream->Read(&m_bApplyFreePromotionOnMove);
	stream->Read(&m_bProtectedCulture);
	stream->Read(&m_bAllowsAmbassadors);
	stream->Read(&m_bBuildOnlyOnPeaks);
	stream->Read(&m_bPrereqPower);
	stream->Read(&m_bNoEnemyPillagingIncome);
	stream->Read(&m_bProvidesFreshWater);
	stream->Read(&m_bForceAllTradeRoutes);
	stream->Read(&m_bForceNoPrereqScaling);
	stream->Read(&m_bPrereqWar);
	stream->Read(&m_bRequiresActiveCivics);
	stream->Read(&m_bZoneOfControl);

	SAFE_DELETE_ARRAY(m_piMemoryTypeModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piMemoryTypeModifier = new int[NUM_MEMORY_TYPES];
		stream->Read(NUM_MEMORY_TYPES, m_piMemoryTypeModifier);
	}

	SAFE_DELETE_ARRAY(m_pbPrereqOrBuildingClass);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbPrereqOrBuildingClass = new bool[GC.getNumBuildingClassInfos()];
		stream->Read(GC.getNumBuildingClassInfos(), m_pbPrereqOrBuildingClass);
	}

	SAFE_DELETE_ARRAY(m_pbPrereqOrGameSpeed);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbPrereqOrGameSpeed = new bool[GC.getNumGameSpeedInfos()];
		stream->Read(GC.getNumGameSpeedInfos(), m_pbPrereqOrGameSpeed);
	}

	int iNumCivic = 0;
	SAFE_DELETE_ARRAY(m_pbPrereqOrCivics);
	stream->Read(&iNumCivic);
	if (iNumCivic)
	{
		m_pbPrereqOrCivics = new bool[iNumCivic];
		stream->Read(iNumCivic, m_pbPrereqOrCivics);
	}

	SAFE_DELETE_ARRAY(m_pbPrereqAndCivics);
	stream->Read(&iNumCivic);
	if (iNumCivic)
	{
		m_pbPrereqAndCivics = new bool[iNumCivic];
		stream->Read(iNumCivic, m_pbPrereqAndCivics);
	}

	SAFE_DELETE_ARRAY(m_pbPrereqNotBuildingClass);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbPrereqNotBuildingClass = new bool[GC.getNumBuildingClassInfos()];
		stream->Read(GC.getNumBuildingClassInfos(), m_pbPrereqNotBuildingClass);
	}

	SAFE_DELETE_ARRAY(m_pbPrereqOrTerrain);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbPrereqOrTerrain = new bool[GC.getNumTerrainInfos()];
		stream->Read(GC.getNumTerrainInfos(), m_pbPrereqOrTerrain);
	}

	SAFE_DELETE_ARRAY(m_pbPrereqAndTerrain);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbPrereqAndTerrain = new bool[GC.getNumTerrainInfos()];
		stream->Read(GC.getNumTerrainInfos(), m_pbPrereqAndTerrain);
	}

	SAFE_DELETE_ARRAY(m_pbPrereqOrImprovement);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbPrereqOrImprovement = new bool[GC.getNumImprovementInfos()];
		stream->Read(GC.getNumImprovementInfos(), m_pbPrereqOrImprovement);
	}

	SAFE_DELETE_ARRAY(m_pbPrereqOrFeature);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbPrereqOrFeature = new bool[GC.getNumFeatureInfos()];
		stream->Read(GC.getNumFeatureInfos(), m_pbPrereqOrFeature);
	}

	SAFE_DELETE_ARRAY(m_pbReplaceBuildingClass);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_pbReplaceBuildingClass = new bool[GC.getNumBuildingClassInfos()];
		stream->Read(GC.getNumBuildingClassInfos(), m_pbReplaceBuildingClass);
	}

	SAFE_DELETE_ARRAY(m_piBuildingClassProductionModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piBuildingClassProductionModifier = new int[GC.getNumBuildingClassInfos()];
		stream->Read(GC.getNumBuildingClassInfos(), m_piBuildingClassProductionModifier);
	}

	SAFE_DELETE_ARRAY(m_piGlobalBuildingClassProductionModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piGlobalBuildingClassProductionModifier = new int[GC.getNumBuildingClassInfos()];
		stream->Read(GC.getNumBuildingClassInfos(), m_piGlobalBuildingClassProductionModifier);
	}

	SAFE_DELETE_ARRAY(m_piTechHappinessChanges);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piTechHappinessChanges = new int[GC.getNumTechInfos()];
		stream->Read(GC.getNumTechInfos(), m_piTechHappinessChanges);
	}

	SAFE_DELETE_ARRAY(m_piTechHealthChanges);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piTechHealthChanges = new int[GC.getNumTechInfos()];
		stream->Read(GC.getNumTechInfos(), m_piTechHealthChanges);
	}

	SAFE_DELETE_ARRAY(m_piUnitClassProductionModifier);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piUnitClassProductionModifier = new int[GC.getNumUnitClassInfos()];
		stream->Read(GC.getNumUnitClassInfos(), m_piUnitClassProductionModifier);
	}

	SAFE_DELETE_ARRAY(m_piPrereqOrVicinityBonuses);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piPrereqOrVicinityBonuses = new int[GC.getNUM_BUILDING_PREREQ_OR_BONUSES()];
		stream->Read(GC.getNUM_BUILDING_PREREQ_OR_BONUSES(), m_piPrereqOrVicinityBonuses);
	}

	SAFE_DELETE_ARRAY(m_piBonusDefenseChanges);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piBonusDefenseChanges = new int[GC.getNumBonusInfos()];
		stream->Read(GC.getNumBonusInfos(), m_piBonusDefenseChanges);
	}

	SAFE_DELETE_ARRAY(m_piUnitCombatExtraStrength);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piUnitCombatExtraStrength = new int[GC.getNumUnitCombatInfos()];
		stream->Read(GC.getNumUnitCombatInfos(), m_piUnitCombatExtraStrength);
	}

	SAFE_DELETE_ARRAY(m_piCommerceAttacks);
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_piCommerceAttacks = new int[NUM_COMMERCE_TYPES];
		stream->Read(NUM_COMMERCE_TYPES, m_piCommerceAttacks);
	}

	if (m_ppaiTechCommerceChange != NULL)
	{
		for(i=0;i<GC.getNumTechInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiTechCommerceChange[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiTechCommerceChange);
	}
	if (m_ppaiTechYieldChange != NULL)
	{
		for(i=0;i<GC.getNumTechInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiTechYieldChange[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiTechYieldChange);
	}
	if (m_ppaiTechSpecialistChange != NULL)
	{
		for(i=0;i<GC.getNumTechInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiTechSpecialistChange[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiTechSpecialistChange);
	}
	if (m_ppaiTechCommerceModifier != NULL)
	{
		for(i=0;i<GC.getNumTechInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiTechCommerceModifier[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiTechCommerceModifier);
	}
	if (m_ppaiTechYieldModifier != NULL)
	{
		for(i=0;i<GC.getNumTechInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiTechYieldModifier[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiTechYieldModifier);
	}

	stream->Read(&bPresent);
	if (bPresent)
	{
		m_ppaiTechCommerceChange = new int*[GC.getNumTechInfos()];
		for(i=0;i<GC.getNumTechInfos();i++)
		{
			stream->Read(&bPresent);
			if (bPresent)
			{
				m_ppaiTechCommerceChange[i]  = new int[NUM_COMMERCE_TYPES];
				stream->Read(NUM_COMMERCE_TYPES, m_ppaiTechCommerceChange[i]);
				m_bAnyTechCommerceChanges = true;
			}
		}
	}
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_ppaiTechYieldChange = new int*[GC.getNumTechInfos()];
		for(i=0;i<GC.getNumTechInfos();i++)
		{
			stream->Read(&bPresent);
			if (bPresent)
			{
				m_ppaiTechYieldChange[i]  = new int[NUM_YIELD_TYPES];
				stream->Read(NUM_YIELD_TYPES, m_ppaiTechYieldChange[i]);
				m_bAnyTechYieldChanges = true;
			}
		}
	}
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_ppaiTechSpecialistChange = new int*[GC.getNumTechInfos()];
		for(i=0;i<GC.getNumTechInfos();i++)
		{
			stream->Read(&bPresent);
			if (bPresent)
			{
				m_ppaiTechSpecialistChange[i]  = new int[GC.getNumSpecialistInfos()];
				stream->Read(GC.getNumSpecialistInfos(), m_ppaiTechSpecialistChange[i]);
				m_bAnyTechSpecialistChanges = true;
			}
		}
	}
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_ppaiTechCommerceModifier = new int*[GC.getNumTechInfos()];
		for(i=0;i<GC.getNumTechInfos();i++)
		{
			stream->Read(&bPresent);
			if (bPresent)
			{
				m_ppaiTechCommerceModifier[i]  = new int[NUM_COMMERCE_TYPES];
				stream->Read(NUM_COMMERCE_TYPES, m_ppaiTechCommerceModifier[i]);
				m_bAnyTechCommerceModifiers = true;
			}
		}
	}
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_ppaiTechYieldModifier = new int*[GC.getNumTechInfos()];
		for(i=0;i<GC.getNumTechInfos();i++)
		{
			stream->Read(&bPresent);
			if (bPresent)
			{
				m_ppaiTechYieldModifier[i]  = new int[NUM_YIELD_TYPES];
				stream->Read(NUM_YIELD_TYPES, m_ppaiTechYieldModifier[i]);
				m_bAnyTechYieldModifiers = true;
			}
		}
	}

	if (m_ppaiBonusCommerceModifier != NULL)
	{
		for(i=0;i<GC.getNumBonusInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiBonusCommerceModifier[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiBonusCommerceModifier);
	}
	if (m_ppaiBonusYieldChanges != NULL)
	{
		for(i=0;i<GC.getNumBonusInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiBonusYieldChanges[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiBonusYieldChanges);
	}
	if (m_ppaiVicinityBonusYieldChanges != NULL)
	{
		for(i=0;i<GC.getNumBonusInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiVicinityBonusYieldChanges[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiVicinityBonusYieldChanges);
	}
	if (m_ppaiBonusCommercePercentChanges != NULL)
	{
		for(i=0;i<GC.getNumBonusInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppaiBonusCommercePercentChanges[i]);
		}
		SAFE_DELETE_ARRAY(m_ppaiBonusCommercePercentChanges);
	}

	stream->Read(&bPresent);
	if (bPresent)
	{
		m_ppaiVicinityBonusYieldChanges = new int*[GC.getNumBonusInfos()];
		for(i=0;i<GC.getNumBonusInfos();i++)
		{
			stream->Read(&bPresent);
			if (bPresent)
			{
				m_ppaiVicinityBonusYieldChanges[i]  = new int[NUM_YIELD_TYPES];
				stream->Read(NUM_YIELD_TYPES, m_ppaiVicinityBonusYieldChanges[i]);
				m_bAnyVicinityBonusYieldChanges = true;
			}
		}
	}

	stream->Read(&bPresent);
	if (bPresent)
	{
		m_ppaiBonusYieldChanges = new int*[GC.getNumBonusInfos()];
		for(i=0;i<GC.getNumBonusInfos();i++)
		{
			stream->Read(&bPresent);
			if (bPresent)
			{
				m_ppaiBonusYieldChanges[i]  = new int[NUM_YIELD_TYPES];
				stream->Read(NUM_YIELD_TYPES, m_ppaiBonusYieldChanges[i]);
				m_bAnyBonusYieldChanges = true;
			}
		}
	}

	stream->Read(&bPresent);
	if (bPresent)
	{
		m_ppaiBonusCommerceModifier = new int*[GC.getNumBonusInfos()];
		for(i=0;i<GC.getNumBonusInfos();i++)
		{
			stream->Read(&bPresent);
			if (bPresent)
			{
				m_ppaiBonusCommerceModifier[i]  = new int[NUM_COMMERCE_TYPES];
				stream->Read(NUM_COMMERCE_TYPES, m_ppaiBonusCommerceModifier[i]);
				m_bAnyBonusCommerceModifiers = true;
			}
		}
	}

	stream->Read(&bPresent);
	if (bPresent)
	{
		m_ppaiBonusCommercePercentChanges = new int*[GC.getNumBonusInfos()];
		for(i=0;i<GC.getNumBonusInfos();i++)
		{
			stream->Read(&bPresent);
			if (bPresent)
			{
				m_ppaiBonusCommercePercentChanges[i]  = new int[NUM_COMMERCE_TYPES];
				stream->Read(NUM_COMMERCE_TYPES, m_ppaiBonusCommercePercentChanges[i]);
				m_bAnyBonusCommercePercentChanges = true;
			}
		}
	}


	if (m_ppiImprovementYieldChanges != NULL)
	{
		for(i=0;i<GC.getNumImprovementInfos();i++)
		{
			SAFE_DELETE_ARRAY(m_ppiImprovementYieldChanges[i]);
		}
		SAFE_DELETE_ARRAY(m_ppiImprovementYieldChanges);
	}
	stream->Read(&bPresent);
	if (bPresent)
	{
		m_ppiImprovementYieldChanges = new int*[GC.getNumImprovementInfos()];
		for(i=0;i<GC.getNumImprovementInfos();i++)
		{
			stream->Read(&bPresent);
			if (bPresent)
			{
				m_ppiImprovementYieldChanges[i]  = new int[NUM_YIELD_TYPES];
				stream->Read(NUM_YIELD_TYPES, m_ppiImprovementYieldChanges[i]);
			}
		}
	}

	stream->Read(&bPresent);
	if (bPresent)
	{
		m_ppaiSpecialistCommercePercentChanges = new int*[GC.getNumSpecialistInfos()];
		for (i = 0; i<GC.getNumSpecialistInfos(); i++)
		{
			stream->Read(&bPresent);
			if (bPresent)
			{
				m_ppaiSpecialistCommercePercentChanges[i] = new int[NUM_COMMERCE_TYPES];
				stream->Read(NUM_COMMERCE_TYPES, m_ppaiSpecialistCommercePercentChanges[i]);
				m_bAnySpecialistCommerceChanges = true;
			}
		}
	}

/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	m_Properties.read(stream);
	m_PropertiesAllCities.read(stream);
	m_PrereqMinProperties.read(stream);
	m_PrereqMaxProperties.read(stream);
	m_PrereqPlayerMinProperties.read(stream);
	m_PrereqPlayerMaxProperties.read(stream);

	m_PropertyManipulators.read(stream);

	bool bExpr = false;
	stream->Read(&bExpr);
	if (bExpr)
	{
		m_pExprNewCityFree = BoolExpr::readExpression(stream);
	}

	bExpr = false;
	stream->Read(&bExpr);
	if (bExpr)
	{
		m_pExprConstructCondition = BoolExpr::readExpression(stream);
	}

}

//
// serialization
//
void CvBuildingInfo::write(FDataStreamBase* stream)
{
	CvHotkeyInfo::write(stream);

	uint uiFlag=0;
	stream->Write(uiFlag);		// flag for expansion

	stream->Write(m_iBuildingClassType);
	stream->Write(m_iVictoryPrereq);
	stream->Write(m_iFreeStartEra);
	stream->Write(m_iMaxStartEra);
	stream->Write(m_iObsoleteTech);
	stream->Write(m_iPrereqAndTech);
	stream->Write(m_iNoBonus);
	stream->Write(m_iPowerBonus);
	stream->Write(m_iFreeBonus);
	stream->Write(m_iNumFreeBonuses);
	int iSize = m_aExtraFreeBonuses.size();
	stream->Write(iSize);
	for (int iI=0; iI<iSize; iI++)
	{
		stream->Write(m_aExtraFreeBonuses[iI].first);
		stream->Write(m_aExtraFreeBonuses[iI].second);
	}
    
    // DarkLunaPhantom - FreeBuilding accepts lists.
    stream->Write((int)m_aiFreeBuildingClass.size());
	for (int iI = 0; iI < (int)m_aiFreeBuildingClass.size(); ++iI)
	{
		stream->Write(m_aiFreeBuildingClass[iI]);
	}
    
    // DarkLunaPhantom - Extra FreeBuilding entries.
    stream->Write((int)m_aiExtraFreeBuildingClass.size());
    for (int iI = 0; iI < (int)m_aiExtraFreeBuildingClass.size(); ++iI)
	{
		stream->Write(m_aiExtraFreeBuildingClass[iI]);
        stream->Write(m_abExtraFreeBuildingConnected[iI]);
        stream->Write(m_abExtraFreeBuildingContinuous[iI]);
	}

	stream->Write(m_iFreeAreaBuildingClass);
	stream->Write(m_iFreeTradeRegionBuildingClass);
	stream->Write(m_iFreePromotion);
	stream->Write(m_iCivicOption);
	stream->Write(m_iAIWeight);
	stream->Write(m_iProductionCost);
	stream->Write(m_iHurryCostModifier);
	stream->Write(m_iHurryAngerModifier);
	stream->Write(m_iAdvancedStartCost);
	stream->Write(m_iAdvancedStartCostIncrease);
	stream->Write(m_iMinAreaSize);
	stream->Write(m_iNumCitiesPrereq);
	stream->Write(m_iNumTeamsPrereq);
	stream->Write(m_iUnitLevelPrereq);
	stream->Write(m_iMinLatitude);
	stream->Write(m_iMaxLatitude);
	stream->Write(m_iGreatPeopleRateModifier);
	stream->Write(m_iGreatGeneralRateModifier);
	stream->Write(m_iDomesticGreatGeneralRateModifier);
	stream->Write(m_iGlobalGreatPeopleRateModifier);
	stream->Write(m_iAnarchyModifier);
	stream->Write(m_iGoldenAgeModifier);
	stream->Write(m_iGlobalHurryModifier);
	stream->Write(m_iFreeExperience);
	stream->Write(m_iGlobalFreeExperience);
	stream->Write(m_iFoodKept);
	//doto culture safe zone
	stream->Write(m_iNoForeignCityCultureGrowth);
	//doto culture safe zone
	stream->Write(m_iAirlift);
	stream->Write(m_iAirModifier);
	stream->Write(m_iAirUnitCapacity);
	stream->Write(m_iWonderCapacityIncrement);
	stream->Write(m_iNukeModifier);
	stream->Write(m_iNukeExplosionRand);
	stream->Write(m_iFreeSpecialist);
	stream->Write(m_iAreaFreeSpecialist);
	stream->Write(m_iGlobalFreeSpecialist);
	stream->Write(m_iHappiness);
	stream->Write(m_iAreaHappiness);
	stream->Write(m_iGlobalHappiness);
	stream->Write(m_iStateReligionHappiness);
	stream->Write(m_iStateReligionHealth);
	stream->Write(m_iWorkerSpeedModifier);
	stream->Write(m_iMilitaryProductionModifier);
	stream->Write(m_iSpaceProductionModifier);
	stream->Write(m_iGlobalSpaceProductionModifier);
	stream->Write(m_iTradeRoutes);
	stream->Write(m_iInterestPercent);
	stream->Write(m_iInterestMax);
	stream->Write(m_iCoastalTradeRoutes);
	stream->Write(m_iGlobalTradeRoutes);
	stream->Write(m_iTradeRouteModifier);
	stream->Write(m_iForeignTradeRouteModifier);
	stream->Write(m_iAssetValue);
	stream->Write(m_iPowerValue);
	stream->Write(m_iSpecialBuildingType);
	stream->Write(m_iAdvisorType);

/********************************************************************************/
/**		REVDCM									2/16/10				phungus420	*/
/**																				*/
/**		CanConstruct															*/
/********************************************************************************/
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
	stream->Write(m_iFoundsCorporation);
	stream->Write(m_iGlobalReligionCommerce);
	stream->Write(m_iGlobalCorporationCommerce);
	stream->Write(m_iPrereqAndBonus);
	stream->Write(m_iGreatPeopleUnitClass);
	stream->Write(m_iGreatPeopleRateChange);
	stream->Write(m_iConquestProbability);
	stream->Write(m_iMaintenanceModifier);
	//DPII < Maintenance Modifier >
	stream->Write(m_iGlobalMaintenanceModifier);
	stream->Write(m_iAreaMaintenanceModifier);
	stream->Write(m_iOtherAreaMaintenanceModifier);
	stream->Write(m_iDistanceMaintenanceModifier);
	stream->Write(m_iNumCitiesMaintenanceModifier);
	stream->Write(m_iCoastalDistanceMaintenanceModifier);
	stream->Write(m_iConnectedCityMaintenanceModifier);
	//DPII < Maintenance Modifier >
	stream->Write(m_iWarWearinessModifier);
	stream->Write(m_iGlobalWarWearinessModifier);
	stream->Write(m_iEnemyWarWearinessModifier);
	stream->Write(m_iHealRateChange);
	stream->Write(m_iHealth);
	stream->Write(m_iAreaHealth);
	stream->Write(m_iGlobalHealth);
	stream->Write(m_iGlobalPopulationChange);
	stream->Write(m_iFreeTechs);
	stream->Write(m_eFreeSpecialTech);
	stream->Write(m_iDefenseModifier);
	stream->Write(m_iBombardDefenseModifier);
	stream->Write(m_iAllCityDefenseModifier);
	stream->Write(m_iEspionageDefenseModifier);

/********************************************************************************/
/**		REVDCM									4/09/10				phungus420	*/
/**																				*/
/**		Building Effects														*/
/********************************************************************************/
	stream->Write(m_iUnitUpgradePriceModifier);
	stream->Write(m_iRevIdxLocal);
	stream->Write(m_iRevIdxNational);
	stream->Write(m_iRevIdxDistanceModifier);
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

	stream->Write(m_iMissionType);
	stream->Write(m_iVoteSourceType);

	stream->Write(m_fVisibilityPriority);

	stream->Write(m_bTeamShare);
	stream->Write(m_bAutoBuild);
	stream->Write(m_bWater);
	stream->Write(m_bRiver);
	stream->Write(m_bFreshWater);
	stream->Write(m_bPower);
	stream->Write(m_bNoPower);
	stream->Write(m_bDirtyPower);
	stream->Write(m_bAreaCleanPower);
	stream->Write(m_bNoHolyCity);
	stream->Write(m_bAreaBorderObstacle);
	stream->Write(m_bForceTeamVoteEligible);
	stream->Write(m_bCapital);
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
	// Dale - AB: Bombing START
	stream->Write(m_iDCMAirbombMission);
	// Dale - AB: Bombing END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
	stream->Write(m_bGovernmentCenter);
	stream->Write(m_bGoldenAge);
	stream->Write(m_bMapCentering);
	stream->Write(m_bNoUnhappiness);
	stream->Write(m_bNoUnhealthyPopulation);
	stream->Write(m_bBuildingOnlyHealthy);
	stream->Write(m_bNeverCapture);
	stream->Write(m_bNukeImmune);
	stream->Write(m_bPrereqReligion);
	stream->Write(m_bFreeFromStateReligion);
	stream->Write(m_bCenterInCity);
	stream->Write(m_bStateReligion);
	stream->Write(m_bAllowsNukes);

	stream->WriteString(m_szConstructSound);
	stream->WriteString(m_szArtDefineTag);
	stream->WriteString(m_szMovieDefineTag);

	if (m_piPrereqAndTechs)
	{
		stream->Write(true);
		stream->Write(GC.getNUM_BUILDING_AND_TECH_PREREQS(), m_piPrereqAndTechs);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piPrereqOrBonuses)
	{
		stream->Write(true);
		stream->Write(GC.getNUM_BUILDING_PREREQ_OR_BONUSES(), m_piPrereqOrBonuses);
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

	if (m_piHappinessTraits)
	{
		stream->Write(true);
		stream->Write(GC.getNumTraitInfos(), m_piHappinessTraits);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piSeaPlotYieldChange)
	{
		stream->Write(true);
		stream->Write(NUM_YIELD_TYPES, m_piSeaPlotYieldChange);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piRiverPlotYieldChange)
	{
		stream->Write(true);
		stream->Write(NUM_YIELD_TYPES, m_piRiverPlotYieldChange);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piGlobalSeaPlotYieldChange)
	{
		stream->Write(true);
		stream->Write(NUM_YIELD_TYPES, m_piGlobalSeaPlotYieldChange);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piYieldChange)
	{
		stream->Write(true);
		stream->Write(NUM_YIELD_TYPES, m_piYieldChange);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piYieldModifier)
	{
		stream->Write(true);
		stream->Write(NUM_YIELD_TYPES, m_piYieldModifier);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piPowerYieldModifier)
	{
		stream->Write(true);
		stream->Write(NUM_YIELD_TYPES, m_piPowerYieldModifier);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piAreaYieldModifier)
	{
		stream->Write(true);
		stream->Write(NUM_YIELD_TYPES, m_piAreaYieldModifier);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piGlobalYieldModifier)
	{
		stream->Write(true);
		stream->Write(NUM_YIELD_TYPES, m_piGlobalYieldModifier);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piCommerceChange)
	{
		stream->Write(true);
		stream->Write(NUM_COMMERCE_TYPES, m_piCommerceChange);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piObsoleteSafeCommerceChange)
	{
		stream->Write(true);
		stream->Write(NUM_COMMERCE_TYPES, m_piObsoleteSafeCommerceChange);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piCommerceChangeDoubleTime)
	{
		stream->Write(true);
		stream->Write(NUM_COMMERCE_TYPES, m_piCommerceChangeDoubleTime);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piCommerceModifier)
	{
		stream->Write(true);
		stream->Write(NUM_COMMERCE_TYPES, m_piCommerceModifier);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piGlobalCommerceModifier)
	{
		stream->Write(true);
		stream->Write(NUM_COMMERCE_TYPES, m_piGlobalCommerceModifier);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piSpecialistExtraCommerce)
	{
		stream->Write(true);
		stream->Write(NUM_COMMERCE_TYPES, m_piSpecialistExtraCommerce);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piStateReligionCommerce)
	{
		stream->Write(true);
		stream->Write(NUM_COMMERCE_TYPES, m_piStateReligionCommerce);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piCommerceHappiness)
	{
		stream->Write(true);
		stream->Write(NUM_COMMERCE_TYPES, m_piCommerceHappiness);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piReligionChange)
	{
		stream->Write(true);
		stream->Write(GC.getNumReligionInfos(), m_piReligionChange);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piSpecialistCount)
	{
		stream->Write(true);
		stream->Write(GC.getNumSpecialistInfos(), m_piSpecialistCount);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piFreeSpecialistCount)
	{
		stream->Write(true);
		stream->Write(GC.getNumSpecialistInfos(), m_piFreeSpecialistCount);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piBonusHealthChanges)
	{
		stream->Write(true);
		stream->Write(GC.getNumBonusInfos(), m_piBonusHealthChanges);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piBonusHappinessChanges)
	{
		stream->Write(true);
		stream->Write(GC.getNumBonusInfos(), m_piBonusHappinessChanges);
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

	if (m_piUnitCombatFreeExperience)
	{
		stream->Write(true);
		stream->Write(GC.getNumUnitCombatInfos(), m_piUnitCombatFreeExperience);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piDomainFreeExperience)
	{
		stream->Write(true);
		stream->Write(NUM_DOMAIN_TYPES, m_piDomainFreeExperience);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piDomainProductionModifier)
	{
		stream->Write(true);
		stream->Write(NUM_DOMAIN_TYPES, m_piDomainProductionModifier);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piBuildingHappinessChanges)
	{
		stream->Write(true);
		stream->Write(GC.getNumBuildingClassInfos(), m_piBuildingHappinessChanges);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piPrereqNumOfBuildingClass)
	{
		stream->Write(true);
		stream->Write(GC.getNumBuildingClassInfos(), m_piPrereqNumOfBuildingClass);
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

	if (m_piImprovementFreeSpecialist)
	{
		stream->Write(true);
		stream->Write(GC.getNumImprovementInfos(), m_piImprovementFreeSpecialist);
	}
	else
	{
		stream->Write(false);
	}


	if (m_pbCommerceFlexible)
	{
		stream->Write(true);
		stream->Write(NUM_COMMERCE_TYPES, m_pbCommerceFlexible);
	}
	else
	{
		stream->Write(false);
	}

	if (m_pbCommerceChangeOriginalOwner)
	{
		stream->Write(true);
		stream->Write(NUM_COMMERCE_TYPES, m_pbCommerceChangeOriginalOwner);
	}
	else
	{
		stream->Write(false);
	}

	if (m_pbBuildingClassNeededInCity)
	{
		stream->Write(true);
		stream->Write(GC.getNumBuildingClassInfos(), m_pbBuildingClassNeededInCity);
	}
	else
	{
		stream->Write(false);
	}


	int i;
	if (m_ppaiSpecialistYieldChange)
	{
		stream->Write(true);
		for(i=0;i<GC.getNumSpecialistInfos();i++)
		{
			if (m_ppaiSpecialistYieldChange[i])
			{
				stream->Write(true);
				stream->Write(NUM_YIELD_TYPES, m_ppaiSpecialistYieldChange[i]);
			}
			else
			{
				stream->Write(false);
			}
		}
	}
	else
	{
		stream->Write(false);
	}

	if (m_ppaiBonusYieldModifier)
	{
		stream->Write(true);
		for(i=0;i<GC.getNumBonusInfos();i++)
		{
			if (m_ppaiBonusYieldModifier[i])
			{
				stream->Write(true);
				stream->Write(NUM_YIELD_TYPES, m_ppaiBonusYieldModifier[i]);
			}
			else
			{
				stream->Write(false);
			}
		}
	}
	else
	{
		stream->Write(false);
	}
	writeOptionalArray(stream, NUM_COMMERCE_TYPES, m_piSpecialistExtraCommerce);
/************************************************************************************************/
/* Afforess	                  Start		 Last Update: 3/8/10                                    */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	stream->Write(m_iGlobalPopulationgrowthratepercentage);
	stream->Write(m_iPopulationgrowthratepercentage);
	stream->Write(m_iPrereqVicinityBonus);
	stream->Write(m_iPillageGoldModifier);
	stream->Write(m_iFreePromotion_2);
	stream->Write(m_iFreePromotion_3);
	stream->Write(m_iWorldTradeRoutes);
	stream->Write(m_iLineOfSight);
	stream->Write(m_iInflationModifier);
	stream->Write(m_iInvasionChance);
	stream->Write(m_iAdjacentDamagePercent);
	stream->Write(m_iProductionContinueBuildingClass);
	stream->Write(m_iPrereqPopulation);
	stream->Write(m_iPrereqCultureLevel);
	stream->Write(m_iWorkableRadius);
	stream->Write(m_iPrereqAnyoneBuildingClass);
	stream->Write(m_iExtendsBuildingClass);
	stream->Write(m_iOccupationTimeModifier);
	stream->Write(m_iNoEntryDefenseLevel);
	stream->Write(m_iNumUnitFullHeal);
	stream->Write(m_iNumPopulationEmployed);
	stream->Write(m_iHealthPercentPerPopulation);
	stream->Write(m_iHappinessPercentPerPopulation);
	stream->Write(m_iCapitalExtraCommerce);
	stream->Write(m_iForeignCapitalExtraCommerce);
	stream->Write(m_iDomesticConnectedCommerce);
	stream->Write(m_iCitiesRequiredPerDomesticConnectedCity);
	stream->Write(m_iMaxDomesticConnectedCommerce);
	stream->Write(m_iForeignConnectedCommerce);
	stream->Write(m_iCitiesRequiredPerForeignConnectedCity);
	stream->Write(m_iMaxForeignConnectedCommerce);
	stream->Write(m_iDiplomacyChange);

	stream->Write(m_iCrime);
	stream->Write(m_iCrimePerPop);

	stream->Write(m_bApplyFreePromotionOnMove);
	stream->Write(m_bProtectedCulture);
	stream->Write(m_bAllowsAmbassadors);
	stream->Write(m_bBuildOnlyOnPeaks);
	stream->Write(m_bPrereqPower);
	stream->Write(m_bNoEnemyPillagingIncome);
	stream->Write(m_bProvidesFreshWater);
	stream->Write(m_bForceAllTradeRoutes);
	stream->Write(m_bForceNoPrereqScaling);
	stream->Write(m_bPrereqWar);
	stream->Write(m_bRequiresActiveCivics);
	stream->Write(m_bZoneOfControl);

	if (m_piMemoryTypeModifier)
	{
		stream->Write(true);
		stream->Write(NUM_MEMORY_TYPES, m_piMemoryTypeModifier);
	}
	else
	{
		stream->Write(false);
	}


	if (m_pbPrereqOrBuildingClass)
	{
		stream->Write(true);
		stream->Write(GC.getNumBuildingClassInfos(), m_pbPrereqOrBuildingClass);
	}
	else
	{
		stream->Write(false);
	}

	if (m_pbPrereqOrGameSpeed)
	{
		stream->Write(true);
		stream->Write(GC.getNumGameSpeedInfos(), m_pbPrereqOrGameSpeed);
	}
	else
	{
		stream->Write(false);
	}

	if (m_pbPrereqOrCivics)
	{
		stream->Write(GC.getNumCivicInfos());
		stream->Write(GC.getNumCivicInfos(), m_pbPrereqOrCivics);
	}
	else
	{
		stream->Write((int)0);
	}

	if (m_pbPrereqAndCivics)
	{
		stream->Write(GC.getNumCivicInfos());
		stream->Write(GC.getNumCivicInfos(), m_pbPrereqAndCivics);
	}
	else
	{
		stream->Write((int)0);
	}

	if (m_pbPrereqNotBuildingClass)
	{
		stream->Write(true);
		stream->Write(GC.getNumBuildingClassInfos(), m_pbPrereqNotBuildingClass);
	}
	else
	{
		stream->Write(false);
	}

	if (m_pbPrereqOrTerrain)
	{
		stream->Write(true);
		stream->Write(GC.getNumTerrainInfos(), m_pbPrereqOrTerrain);
	}
	else
	{
		stream->Write(false);
	}

	if (m_pbPrereqAndTerrain)
	{
		stream->Write(true);
		stream->Write(GC.getNumTerrainInfos(), m_pbPrereqAndTerrain);
	}
	else
	{
		stream->Write(false);
	}

	if (m_pbPrereqOrImprovement)
	{
		stream->Write(true);
		stream->Write(GC.getNumImprovementInfos(), m_pbPrereqOrImprovement);
	}
	else
	{
		stream->Write(false);
	}

	if (m_pbPrereqOrFeature)
	{
		stream->Write(true);
		stream->Write(GC.getNumFeatureInfos(), m_pbPrereqOrFeature);
	}
	else
	{
		stream->Write(false);
	}

	if (m_pbReplaceBuildingClass)
	{
		stream->Write(true);
		stream->Write(GC.getNumBuildingClassInfos(), m_pbReplaceBuildingClass);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piBuildingClassProductionModifier)
	{
		stream->Write(true);
		stream->Write(GC.getNumBuildingClassInfos(), m_piBuildingClassProductionModifier);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piGlobalBuildingClassProductionModifier)
	{
		stream->Write(true);
		stream->Write(GC.getNumBuildingClassInfos(), m_piGlobalBuildingClassProductionModifier);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piTechHappinessChanges)
	{
		stream->Write(true);
		stream->Write(GC.getNumTechInfos(), m_piTechHappinessChanges);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piTechHealthChanges)
	{
		stream->Write(true);
		stream->Write(GC.getNumTechInfos(), m_piTechHealthChanges);
	}
	else
	{
		stream->Write(false);
	}


	if (m_piUnitClassProductionModifier)
	{
		stream->Write(true);
		stream->Write(GC.getNumUnitClassInfos(), m_piUnitClassProductionModifier);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piPrereqOrVicinityBonuses)
	{
		stream->Write(true);
		stream->Write(GC.getNUM_BUILDING_PREREQ_OR_BONUSES(), m_piPrereqOrVicinityBonuses);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piBonusDefenseChanges)
	{
		stream->Write(true);
		stream->Write(GC.getNumBonusInfos(), m_piBonusDefenseChanges);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piUnitCombatExtraStrength)
	{
		stream->Write(true);
		stream->Write(GC.getNumUnitCombatInfos(), m_piUnitCombatExtraStrength);
	}
	else
	{
		stream->Write(false);
	}

	if (m_piCommerceAttacks)
	{
		stream->Write(true);
		stream->Write(NUM_COMMERCE_TYPES, m_piCommerceAttacks);
	}
	else
	{
		stream->Write(false);
	}

	if (m_ppaiTechCommerceChange)
	{
		stream->Write(true);
		for(i=0;i<GC.getNumTechInfos();i++)
		{
			if (m_ppaiTechCommerceChange[i])
			{
				stream->Write(true);
				stream->Write(NUM_COMMERCE_TYPES, m_ppaiTechCommerceChange[i]);
			}
			else
			{
				stream->Write(false);
			}
		}
	}
	else
	{
		stream->Write(false);
	}

	if (m_ppaiTechYieldChange)
	{
		stream->Write(true);
		for(i=0;i<GC.getNumTechInfos();i++)
		{
			if (m_ppaiTechYieldChange[i])
			{
				stream->Write(true);
				stream->Write(NUM_YIELD_TYPES, m_ppaiTechYieldChange[i]);
			}
			else
			{
				stream->Write(false);
			}

		}
	}
	else
	{
		stream->Write(false);
	}

	if (m_ppaiTechSpecialistChange)
	{
		stream->Write(true);
		for(i=0;i<GC.getNumTechInfos();i++)
		{
			if (m_ppaiTechSpecialistChange[i])
			{
				stream->Write(true);
				stream->Write(GC.getNumSpecialistInfos(), m_ppaiTechSpecialistChange[i]);
			}
			else
			{
				stream->Write(false);
			}

		}
	}
	else
	{
		stream->Write(false);
	}

	if (m_ppaiTechCommerceModifier)
	{
		stream->Write(true);
		for(i=0;i<GC.getNumTechInfos();i++)
		{
			if (m_ppaiTechCommerceModifier[i])
			{
				stream->Write(true);
				stream->Write(NUM_COMMERCE_TYPES, m_ppaiTechCommerceModifier[i]);
			}
			else
			{
				stream->Write(false);
			}

		}
	}
	else
	{
		stream->Write(false);
	}

	if (m_ppaiTechYieldModifier)
	{
		stream->Write(true);
		for(i=0;i<GC.getNumTechInfos();i++)
		{
			if (m_ppaiTechYieldModifier[i])
			{
				stream->Write(true);
				stream->Write(NUM_YIELD_TYPES, m_ppaiTechYieldModifier[i]);
			}
			else
			{
				stream->Write(false);
			}

		}
	}
	else
	{
		stream->Write(false);
	}

	if (m_ppaiVicinityBonusYieldChanges)
	{
		stream->Write(true);
		for(i=0;i<GC.getNumBonusInfos();i++)
		{
			if (m_ppaiVicinityBonusYieldChanges[i])
			{
				stream->Write(true);
				stream->Write(NUM_YIELD_TYPES, m_ppaiVicinityBonusYieldChanges[i]);
			}
			else
			{
				stream->Write(false);
			}

		}
	}
	else
	{
		stream->Write(false);
	}

	if (m_ppaiBonusYieldChanges)
	{
		stream->Write(true);
		for(i=0;i<GC.getNumBonusInfos();i++)
		{
			if (m_ppaiBonusYieldChanges[i])
			{
				stream->Write(true);
				stream->Write(NUM_YIELD_TYPES, m_ppaiBonusYieldChanges[i]);
			}
			else
			{
				stream->Write(false);
			}

		}
	}
	else
	{
		stream->Write(false);
	}

	if (m_ppaiBonusCommerceModifier)
	{
		stream->Write(true);
		for(i=0;i<GC.getNumBonusInfos();i++)
		{
			if (m_ppaiBonusCommerceModifier[i])
			{
				stream->Write(true);
				stream->Write(NUM_COMMERCE_TYPES, m_ppaiBonusCommerceModifier[i]);
			}
			else
			{
				stream->Write(false);
			}

		}
	}
	else
	{
		stream->Write(false);
	}

	if (m_ppaiBonusCommercePercentChanges)
	{
		stream->Write(true);
		for(i=0;i<GC.getNumBonusInfos();i++)
		{
			if (m_ppaiBonusCommercePercentChanges[i])
			{
				stream->Write(true);
				stream->Write(NUM_COMMERCE_TYPES, m_ppaiBonusCommercePercentChanges[i]);
			}
			else
			{
				stream->Write(false);
			}

		}
	}
	else
	{
		stream->Write(false);
	}


	if (m_ppiImprovementYieldChanges)
	{
		stream->Write(true);
		for(i=0;i<GC.getNumImprovementInfos();i++)
		{
			if (m_ppiImprovementYieldChanges[i])
			{
				stream->Write(true);
				stream->Write(NUM_YIELD_TYPES, m_ppiImprovementYieldChanges[i]);
			}
			else
			{
				stream->Write(false);
			}

		}
	}
	else
	{
		stream->Write(false);
	}

	if (m_ppaiSpecialistCommercePercentChanges)
	{
		stream->Write(true);
		for (i = 0; i<GC.getNumSpecialistInfos(); i++)
		{
			if (m_ppaiSpecialistCommercePercentChanges[i])
			{
				stream->Write(true);
				stream->Write(NUM_COMMERCE_TYPES, m_ppaiSpecialistCommercePercentChanges[i]);
			}
			else
			{
				stream->Write(false);
			}

		}
	}
	else
	{
		stream->Write(false);
	}

/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	m_Properties.write(stream);
	m_PropertiesAllCities.write(stream);
	m_PrereqMinProperties.write(stream);
	m_PrereqMaxProperties.write(stream);
	m_PrereqPlayerMinProperties.write(stream);
	m_PrereqPlayerMaxProperties.write(stream);

	m_PropertyManipulators.write(stream);

	if (m_pExprNewCityFree)
	{
		stream->Write(true);
		m_pExprNewCityFree->write(stream);
	}
	else
	{
		stream->Write(false);
	}

	if (m_pExprConstructCondition)
	{
		stream->Write(true);
		m_pExprConstructCondition->write(stream);
	}
	else
	{
		stream->Write(false);
	}
}


void CvBuildingInfo::getCheckSum(unsigned int& iSum)
{
	CheckSum(iSum, m_iBuildingClassType);
	CheckSum(iSum, m_iVictoryPrereq);
	CheckSum(iSum, m_iFreeStartEra);
	CheckSum(iSum, m_iMaxStartEra);
	CheckSum(iSum, m_iObsoleteTech);
	CheckSum(iSum, m_iPrereqAndTech);
	CheckSum(iSum, m_iNoBonus);
	CheckSum(iSum, m_iPowerBonus);
	CheckSum(iSum, m_iFreeBonus);
	CheckSum(iSum, m_iNumFreeBonuses);

	CheckSumC(iSum, m_aExtraFreeBonuses);

	CheckSumC(iSum, m_aiFreeBuildingClass); // DarkLunaPhantom - FreeBuilding accepts lists.
    // DarkLunaPhantom - Extra FreeBuilding entries.
    CheckSumC(iSum, m_aiExtraFreeBuildingClass);
    CheckSumC(iSum, m_abExtraFreeBuildingConnected);
    CheckSumC(iSum, m_abExtraFreeBuildingContinuous);
    
	CheckSum(iSum, m_iFreeAreaBuildingClass);
	CheckSum(iSum, m_iFreeTradeRegionBuildingClass);
	CheckSum(iSum, m_iFreePromotion);
	CheckSum(iSum, m_iCivicOption);
	CheckSum(iSum, m_iAIWeight);
	CheckSum(iSum, m_iProductionCost);
	CheckSum(iSum, m_iHurryCostModifier);
	CheckSum(iSum, m_iHurryAngerModifier);
	CheckSum(iSum, m_iAdvancedStartCost);
	CheckSum(iSum, m_iAdvancedStartCostIncrease);
	CheckSum(iSum, m_iMinAreaSize);
	CheckSum(iSum, m_iNumCitiesPrereq);
	CheckSum(iSum, m_iNumTeamsPrereq);
	CheckSum(iSum, m_iUnitLevelPrereq);
	CheckSum(iSum, m_iMinLatitude);
	CheckSum(iSum, m_iMaxLatitude);
	CheckSum(iSum, m_iGreatPeopleRateModifier);
	CheckSum(iSum, m_iGreatGeneralRateModifier);
	CheckSum(iSum, m_iDomesticGreatGeneralRateModifier);
	CheckSum(iSum, m_iGlobalGreatPeopleRateModifier);
	CheckSum(iSum, m_iAnarchyModifier);
	CheckSum(iSum, m_iGoldenAgeModifier);
	CheckSum(iSum, m_iGlobalHurryModifier);
	CheckSum(iSum, m_iFreeExperience);
	CheckSum(iSum, m_iGlobalFreeExperience);
	CheckSum(iSum, m_iFoodKept);
	//doto culture safe zone
	CheckSum(iSum, m_iNoForeignCityCultureGrowth);
	//doto culture safe zone
	CheckSum(iSum, m_iAirlift);
	CheckSum(iSum, m_iAirModifier);
	CheckSum(iSum, m_iAirUnitCapacity);
	CheckSum(iSum,	m_iWonderCapacityIncrement);
	CheckSum(iSum, m_iNukeModifier);
	CheckSum(iSum, m_iNukeExplosionRand);
	CheckSum(iSum, m_iFreeSpecialist);
	CheckSum(iSum, m_iAreaFreeSpecialist);
	CheckSum(iSum, m_iGlobalFreeSpecialist);
	CheckSum(iSum, m_iHappiness);
	CheckSum(iSum, m_iAreaHappiness);
	CheckSum(iSum, m_iGlobalHappiness);
	CheckSum(iSum, m_iStateReligionHappiness);
	CheckSum(iSum, m_iStateReligionHealth);
	CheckSum(iSum, m_iWorkerSpeedModifier);
	CheckSum(iSum, m_iMilitaryProductionModifier);
	CheckSum(iSum, m_iSpaceProductionModifier);
	CheckSum(iSum, m_iGlobalSpaceProductionModifier);
	CheckSum(iSum, m_iTradeRoutes);
	CheckSum(iSum, m_iInterestPercent);
	CheckSum(iSum, m_iInterestMax);
	CheckSum(iSum, m_iCoastalTradeRoutes);
	CheckSum(iSum, m_iGlobalTradeRoutes);
	CheckSum(iSum, m_iTradeRouteModifier);
	CheckSum(iSum, m_iForeignTradeRouteModifier);
	CheckSum(iSum, m_iAssetValue);
	CheckSum(iSum, m_iPowerValue);
	CheckSum(iSum, m_iSpecialBuildingType);
	CheckSum(iSum, m_iAdvisorType);

/********************************************************************************/
/**		REVDCM									2/16/10				phungus420	*/
/**																				*/
/**		CanConstruct															*/
/********************************************************************************/
	CheckSum(iSum, m_iPrereqGameOption);
	CheckSum(iSum, m_iNotGameOption);
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

	CheckSum(iSum, m_iHolyCity);
	CheckSum(iSum, m_iReligionType);
	CheckSum(iSum, m_iStateReligion);
	CheckSum(iSum, m_iPrereqReligion);
	CheckSum(iSum, m_iPrereqCorporation);
	CheckSum(iSum, m_iFoundsCorporation);
	CheckSum(iSum, m_iGlobalReligionCommerce);
	CheckSum(iSum, m_iGlobalCorporationCommerce);
	CheckSum(iSum, m_iPrereqAndBonus);
	CheckSum(iSum, m_iGreatPeopleUnitClass);
	CheckSum(iSum, m_iGreatPeopleRateChange);
	CheckSum(iSum, m_iConquestProbability);
	CheckSum(iSum, m_iMaintenanceModifier);
	//DPII < Maintenance Modifier >
	CheckSum(iSum, m_iGlobalMaintenanceModifier);
	CheckSum(iSum, m_iAreaMaintenanceModifier);
	CheckSum(iSum, m_iOtherAreaMaintenanceModifier);
	CheckSum(iSum, m_iDistanceMaintenanceModifier);
	CheckSum(iSum, m_iNumCitiesMaintenanceModifier);
	CheckSum(iSum, m_iCoastalDistanceMaintenanceModifier);
	CheckSum(iSum, m_iConnectedCityMaintenanceModifier);
	//DPII < Maintenance Modifier >
	CheckSum(iSum, m_iWarWearinessModifier);
	CheckSum(iSum, m_iGlobalWarWearinessModifier);
	CheckSum(iSum, m_iEnemyWarWearinessModifier);
	CheckSum(iSum, m_iHealRateChange);
	CheckSum(iSum, m_iHealth);
	CheckSum(iSum, m_iAreaHealth);
	CheckSum(iSum, m_iGlobalHealth);
	CheckSum(iSum, m_iGlobalPopulationChange);
	CheckSum(iSum, m_iFreeTechs);
	CheckSum(iSum, m_eFreeSpecialTech);
	CheckSum(iSum, m_iDefenseModifier);
	CheckSum(iSum, m_iBombardDefenseModifier);
	CheckSum(iSum, m_iAllCityDefenseModifier);
	CheckSum(iSum, m_iEspionageDefenseModifier);

/********************************************************************************/
/**		REVDCM									4/09/10				phungus420	*/
/**																				*/
/**		Building Effects														*/
/********************************************************************************/
	CheckSum(iSum, m_iUnitUpgradePriceModifier);
	CheckSum(iSum, m_iRevIdxLocal);
	CheckSum(iSum, m_iRevIdxNational);
	CheckSum(iSum, m_iRevIdxDistanceModifier);
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

	CheckSum(iSum, m_iMissionType);
	CheckSum(iSum, m_iVoteSourceType);

	CheckSum(iSum, m_bTeamShare);
	CheckSum(iSum, m_bAutoBuild);
	CheckSum(iSum, m_bWater);
	CheckSum(iSum, m_bRiver);
	CheckSum(iSum, m_bFreshWater);
	CheckSum(iSum, m_bPower);
	CheckSum(iSum, m_bNoPower);
	CheckSum(iSum, m_bDirtyPower);
	CheckSum(iSum, m_bAreaCleanPower);
	CheckSum(iSum, m_bNoHolyCity);
	CheckSum(iSum, m_bAreaBorderObstacle);
	CheckSum(iSum, m_bForceTeamVoteEligible);
	CheckSum(iSum, m_bCapital);
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
	// Dale - AB: Bombing START
	CheckSum(iSum, m_iDCMAirbombMission);
	// Dale - AB: Bombing END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
	CheckSum(iSum, m_bGovernmentCenter);
	CheckSum(iSum, m_bGoldenAge);
	CheckSum(iSum, m_bMapCentering);
	CheckSum(iSum, m_bNoUnhappiness);
	CheckSum(iSum, m_bNoUnhealthyPopulation);
	CheckSum(iSum, m_bBuildingOnlyHealthy);
	CheckSum(iSum, m_bNeverCapture);
	CheckSum(iSum, m_bNukeImmune);
	CheckSum(iSum, m_bPrereqReligion);
	CheckSum(iSum, m_bFreeFromStateReligion);
	CheckSum(iSum, m_bCenterInCity);
	CheckSum(iSum, m_bStateReligion);
	CheckSum(iSum, m_bAllowsNukes);

	CheckSumI(iSum, GC.getNUM_BUILDING_AND_TECH_PREREQS(), m_piPrereqAndTechs);
	CheckSumI(iSum, GC.getNUM_BUILDING_PREREQ_OR_BONUSES(), m_piPrereqOrBonuses);
	CheckSumI(iSum, GC.getNumTraitInfos(), m_piProductionTraits);
	CheckSumI(iSum, GC.getNumTraitInfos(), m_piHappinessTraits);
	CheckSumI(iSum, NUM_YIELD_TYPES, m_piSeaPlotYieldChange);
	CheckSumI(iSum, NUM_YIELD_TYPES, m_piRiverPlotYieldChange);
	CheckSumI(iSum, NUM_YIELD_TYPES, m_piGlobalSeaPlotYieldChange);
	CheckSumI(iSum, NUM_YIELD_TYPES, m_piYieldChange);
	CheckSumI(iSum, NUM_YIELD_TYPES, m_piYieldModifier);
	CheckSumI(iSum, NUM_YIELD_TYPES, m_piPowerYieldModifier);
	CheckSumI(iSum, NUM_YIELD_TYPES, m_piAreaYieldModifier);
	CheckSumI(iSum, NUM_YIELD_TYPES, m_piGlobalYieldModifier);
	CheckSumI(iSum, NUM_COMMERCE_TYPES, m_piCommerceChange);
	CheckSumI(iSum, NUM_COMMERCE_TYPES, m_piObsoleteSafeCommerceChange);
	CheckSumI(iSum, NUM_COMMERCE_TYPES, m_piCommerceChangeDoubleTime);
	CheckSumI(iSum, NUM_COMMERCE_TYPES, m_piCommerceModifier);
	CheckSumI(iSum, NUM_COMMERCE_TYPES, m_piGlobalCommerceModifier);
	CheckSumI(iSum, NUM_COMMERCE_TYPES, m_piSpecialistExtraCommerce);
	CheckSumI(iSum, NUM_COMMERCE_TYPES, m_piStateReligionCommerce);
	CheckSumI(iSum, NUM_COMMERCE_TYPES, m_piCommerceHappiness);
	CheckSumI(iSum, GC.getNumReligionInfos(), m_piReligionChange);
	CheckSumI(iSum, GC.getNumSpecialistInfos(), m_piSpecialistCount);
	CheckSumI(iSum, GC.getNumSpecialistInfos(), m_piFreeSpecialistCount);
	CheckSumI(iSum, GC.getNumBonusInfos(), m_piBonusHealthChanges);
	CheckSumI(iSum, GC.getNumBonusInfos(), m_piBonusHappinessChanges);
	CheckSumI(iSum, GC.getNumBonusInfos(), m_piBonusProductionModifier);
	CheckSumI(iSum, GC.getNumUnitCombatInfos(), m_piUnitCombatFreeExperience);
	CheckSumI(iSum, NUM_DOMAIN_TYPES, m_piDomainFreeExperience);
	CheckSumI(iSum, NUM_DOMAIN_TYPES, m_piDomainProductionModifier);
	CheckSumI(iSum, GC.getNumBuildingClassInfos(), m_piBuildingHappinessChanges);
	CheckSumI(iSum, GC.getNumBuildingClassInfos(), m_piPrereqNumOfBuildingClass);
	CheckSumI(iSum, GC.getNumFlavorTypes(), m_piFlavorValue);
	CheckSumI(iSum, GC.getNumImprovementInfos(), m_piImprovementFreeSpecialist);

	CheckSumI(iSum, NUM_COMMERCE_TYPES, m_pbCommerceFlexible);
	CheckSumI(iSum, NUM_COMMERCE_TYPES, m_pbCommerceChangeOriginalOwner);
	CheckSumI(iSum, GC.getNumBuildingClassInfos(), m_pbBuildingClassNeededInCity);

	int i;
	if (m_ppaiSpecialistYieldChange)
		for(i=0;i<GC.getNumSpecialistInfos();i++)
		{
			CheckSumI(iSum, NUM_YIELD_TYPES, m_ppaiSpecialistYieldChange[i]);
		}

	if (m_ppaiBonusYieldModifier)
		for(i=0;i<GC.getNumBonusInfos();i++)
		{
			CheckSumI(iSum, NUM_YIELD_TYPES, m_ppaiBonusYieldModifier[i]);
		}
/************************************************************************************************/
/* Afforess	                  Start		 Last Update: 3/8/10                                    */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	CheckSum(iSum, m_iGlobalPopulationgrowthratepercentage);
	CheckSum(iSum, m_iPopulationgrowthratepercentage);
	CheckSum(iSum, m_iPrereqVicinityBonus);
	CheckSum(iSum, m_iPillageGoldModifier);
	CheckSum(iSum, m_iFreePromotion_2);
	CheckSum(iSum, m_iFreePromotion_3);
	CheckSum(iSum, m_iWorldTradeRoutes);
	CheckSum(iSum, m_iLineOfSight);
	CheckSum(iSum, m_iInflationModifier);
	CheckSum(iSum, m_iInvasionChance);
	CheckSum(iSum, m_iAdjacentDamagePercent);
	CheckSum(iSum, m_iProductionContinueBuildingClass);
	CheckSum(iSum, m_iPrereqPopulation);
	CheckSum(iSum, m_iPrereqCultureLevel);
	CheckSum(iSum, m_iWorkableRadius);
	CheckSum(iSum, m_iPrereqAnyoneBuildingClass);
	CheckSum(iSum, m_iExtendsBuildingClass);
	CheckSum(iSum, m_iOccupationTimeModifier);
	CheckSum(iSum, m_iNoEntryDefenseLevel);
	CheckSum(iSum, m_iNumUnitFullHeal);
	CheckSum(iSum, m_iNumPopulationEmployed);
	CheckSum(iSum, m_iHealthPercentPerPopulation);
	CheckSum(iSum, m_iHappinessPercentPerPopulation);
	CheckSum(iSum, m_iCapitalExtraCommerce);
	CheckSum(iSum, m_iForeignCapitalExtraCommerce);
	CheckSum(iSum, m_iDomesticConnectedCommerce);
	CheckSum(iSum, m_iCitiesRequiredPerDomesticConnectedCity);
	CheckSum(iSum, m_iMaxDomesticConnectedCommerce);
	CheckSum(iSum, m_iForeignConnectedCommerce);
	CheckSum(iSum, m_iCitiesRequiredPerForeignConnectedCity);
	CheckSum(iSum, m_iMaxForeignConnectedCommerce);
	CheckSum(iSum, m_iDiplomacyChange);

	CheckSum(iSum, m_iCrime);
	CheckSum(iSum, m_iCrimePerPop);

	CheckSum(iSum, m_bApplyFreePromotionOnMove);
	CheckSum(iSum, m_bProtectedCulture);
	CheckSum(iSum, m_bAllowsAmbassadors);
	CheckSum(iSum, m_bBuildOnlyOnPeaks);
	CheckSum(iSum, m_bPrereqPower);
	CheckSum(iSum, m_bNoEnemyPillagingIncome);
	CheckSum(iSum, m_bProvidesFreshWater);
	CheckSum(iSum, m_bForceAllTradeRoutes);
	CheckSum(iSum, m_bForceNoPrereqScaling);
	CheckSum(iSum, m_bPrereqWar);
	CheckSum(iSum, m_bRequiresActiveCivics);
	CheckSum(iSum, m_bZoneOfControl);

	CheckSumI(iSum, NUM_MEMORY_TYPES, m_piMemoryTypeModifier);
	CheckSumI(iSum, GC.getNumBuildingClassInfos(), m_pbPrereqOrBuildingClass);
	CheckSumI(iSum, GC.getNumGameSpeedInfos(), m_pbPrereqOrGameSpeed);
	CheckSumI(iSum, GC.getNumCivicInfos(), m_pbPrereqOrCivics);
	CheckSumI(iSum, GC.getNumCivicInfos(), m_pbPrereqAndCivics);
	CheckSumI(iSum, GC.getNumBuildingClassInfos(), m_pbPrereqNotBuildingClass);
	CheckSumI(iSum, GC.getNumTerrainInfos(), m_pbPrereqOrTerrain);
	CheckSumI(iSum, GC.getNumTerrainInfos(), m_pbPrereqAndTerrain);
	CheckSumI(iSum, GC.getNumImprovementInfos(), m_pbPrereqOrImprovement);
	CheckSumI(iSum, GC.getNumFeatureInfos(), m_pbPrereqOrFeature);
	CheckSumI(iSum, GC.getNumBuildingClassInfos(), m_pbReplaceBuildingClass);
	CheckSumI(iSum, GC.getNumBuildingClassInfos(), m_piBuildingClassProductionModifier);
	CheckSumI(iSum, GC.getNumBuildingClassInfos(), m_piGlobalBuildingClassProductionModifier);
	CheckSumI(iSum, GC.getNumTechInfos(), m_piTechHappinessChanges);
	CheckSumI(iSum, GC.getNumTechInfos(), m_piTechHealthChanges);

	CheckSumI(iSum, GC.getNumUnitClassInfos(), m_piUnitClassProductionModifier);
	CheckSumI(iSum, GC.getNUM_BUILDING_PREREQ_OR_BONUSES(), m_piPrereqOrVicinityBonuses);
	CheckSumI(iSum, GC.getNumBonusInfos(), m_piBonusDefenseChanges);
	CheckSumI(iSum, GC.getNumUnitCombatInfos(), m_piUnitCombatExtraStrength);
	CheckSumI(iSum, NUM_COMMERCE_TYPES, m_piCommerceAttacks);

	for(i=0;i<GC.getNumTechInfos();i++)
	{
		if (m_ppaiTechCommerceChange)
			CheckSumI(iSum, NUM_COMMERCE_TYPES, m_ppaiTechCommerceChange[i]);
		if (m_ppaiTechYieldChange)
			CheckSumI(iSum, NUM_YIELD_TYPES, m_ppaiTechYieldChange[i]);
		if (m_ppaiTechSpecialistChange)
			CheckSumI(iSum, GC.getNumSpecialistInfos(), m_ppaiTechSpecialistChange[i]);
		if (m_ppaiTechCommerceModifier)
			CheckSumI(iSum, NUM_COMMERCE_TYPES, m_ppaiTechCommerceModifier[i]);
		if (m_ppaiTechYieldModifier)
			CheckSumI(iSum, NUM_YIELD_TYPES, m_ppaiTechYieldModifier[i]);
	}

	for(i=0;i<GC.getNumBonusInfos();i++)
	{
		if (m_ppaiBonusCommerceModifier)
			CheckSumI(iSum, NUM_COMMERCE_TYPES, m_ppaiBonusCommerceModifier[i]);
		if (m_ppaiBonusCommercePercentChanges)
			CheckSumI(iSum, NUM_COMMERCE_TYPES, m_ppaiBonusCommercePercentChanges[i]);
		if (m_ppaiBonusYieldChanges)
			CheckSumI(iSum, NUM_YIELD_TYPES, m_ppaiBonusYieldChanges[i]);
		if (m_ppaiVicinityBonusYieldChanges)
			CheckSumI(iSum, NUM_YIELD_TYPES, m_ppaiVicinityBonusYieldChanges[i]);
	}

	if (m_ppiImprovementYieldChanges)
	{
		for (i = 0; i < GC.getNumImprovementInfos(); i++)
		{
			CheckSumI(iSum, NUM_YIELD_TYPES, m_ppiImprovementYieldChanges[i]);
		}
	}

	if (m_ppaiSpecialistCommercePercentChanges)
	{
		for (i = 0; i < GC.getNumSpecialistInfos(); i++)
		{
			CheckSumI(iSum, NUM_COMMERCE_TYPES, m_ppaiSpecialistCommercePercentChanges[i]);
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	m_Properties.getCheckSum(iSum);
	m_PropertiesAllCities.getCheckSum(iSum);
	m_PrereqMinProperties.getCheckSum(iSum);
	m_PrereqMaxProperties.getCheckSum(iSum);
	m_PrereqPlayerMinProperties.getCheckSum(iSum);
	m_PrereqPlayerMaxProperties.getCheckSum(iSum);

	m_PropertyManipulators.getCheckSum(iSum);

	if (m_pExprNewCityFree)
		m_pExprNewCityFree->getCheckSum(iSum);

	if (m_pExprConstructCondition)
		m_pExprConstructCondition->getCheckSum(iSum);
}

//
// read from XML
//
bool CvBuildingInfo::read(CvXMLLoadUtility* pXML)
{
	MEMORY_TRACE_FUNCTION();

	CvString szTextVal;
/************************************************************************************************/
/* XMLCOPY                                 11/19/07                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	CvString szDebugBuffer;
/************************************************************************************************/
/* XMLCOPY                                 END                                                  */
/************************************************************************************************/
	if (!CvHotkeyInfo::read(pXML))
	{
		return false;
	}

	int j=0;						//loop counter
	int k=0;						//loop counter
	int iNumSibs=0;				// the number of siblings the current xml node has
	int iNumChildren;				// the number of children the current node has

	pXML->GetChildXmlValByName(szTextVal, "BuildingClass");
	m_iBuildingClassType = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "SpecialBuildingType");
	m_iSpecialBuildingType = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "Advisor");
	m_iAdvisorType = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "ArtDefineTag");
	setArtDefineTag(szTextVal);

/************************************************************************************************/
/* XMLCOPY                                 11/19/07                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
	pXML->GetChildXmlValByName(szTextVal, "MovieDefineTag");
	setMovieDefineTag(szTextVal);
*/
	pXML->GetChildXmlValByName(szTextVal, "MovieDefineTag", "NONE");
	setMovieDefineTag(szTextVal);
/************************************************************************************************/
/* XMLCOPY                                 END                                                  */
/************************************************************************************************/

/********************************************************************************/
/**		REVDCM									2/16/10				phungus420	*/
/**																				*/
/**		CanConstruct															*/
/********************************************************************************/
	pXML->GetChildXmlValByName(szTextVal, "PrereqGameOption");
	m_iPrereqGameOption = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "NotGameOption");
	m_iNotGameOption = pXML->FindInInfoClass(szTextVal);
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

	pXML->GetChildXmlValByName(szTextVal, "FoundsCorporation");
	m_iFoundsCorporation = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "GlobalReligionCommerce");
	m_iGlobalReligionCommerce = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "GlobalCorporationCommerce");
	m_iGlobalCorporationCommerce = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "VictoryPrereq");
	m_iVictoryPrereq = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "FreeStartEra");
	m_iFreeStartEra = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "MaxStartEra");
	m_iMaxStartEra = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "ObsoleteTech");
	m_iObsoleteTech = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "PrereqTech");
	m_iPrereqAndTech = pXML->FindInInfoClass(szTextVal);

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"TechTypes"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = GETXML->GetNumChildren(pXML->GetXML());

			if (0 < iNumSibs)
			{
				pXML->CvXMLLoadUtility::InitList(&m_piPrereqAndTechs, GC.getNUM_BUILDING_AND_TECH_PREREQS(), -1);
				if (pXML->GetChildXmlVal(szTextVal))
				{
					FAssertMsg((iNumSibs <= GC.getNUM_BUILDING_AND_TECH_PREREQS()),"For loop iterator is greater than array size");
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
/* XMLCOPY                                 10/10/07                                MRGENIE      */
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

	pXML->GetChildXmlValByName(szTextVal, "Bonus");
	m_iPrereqAndBonus = pXML->FindInInfoClass(szTextVal);

	// if we can set the current xml node to it's next sibling
	if (GETXML->SetToChildByTagName(pXML->GetXML(),"PrereqBonuses"))
	{
		// the next xml val in the buildinginfo tag set is the bonuses tag set
		// Skip any comments and stop at the next value we might want
		if (pXML->SkipToNextVal())
		{
			// get the total number of children the current xml node has
			iNumChildren = GETXML->GetNumChildren(pXML->GetXML());

			if (0 < iNumChildren)
			{
				pXML->CvXMLLoadUtility::InitList(&m_piPrereqOrBonuses, GC.getNUM_BUILDING_PREREQ_OR_BONUSES(), -1);
				// if the call to the function that sets the current xml node to it's first non-comment
				// child and sets the parameter with the new node's value succeeds
				if (pXML->GetChildXmlVal(szTextVal))
				{
					FAssertMsg((iNumChildren <= GC.getNUM_BUILDING_PREREQ_OR_BONUSES()),"For loop iterator is greater than array size");
					// loop through all the siblings
					for (j=0;j<iNumChildren;j++)
					{
						// call the find in list function to return either -1 if no value is found
						// or the index in the list the match is found at
						m_piPrereqOrBonuses[j] = pXML->FindInInfoClass(szTextVal);

						// if the call to the function that sets the current xml node to it's first non-comment
						// sibling and sets the parameter with the new node's value does not succeed
						// we will break out of this for loop
						if (!pXML->GetNextXmlVal(szTextVal))
						{
							break;
						}
					}

					// set the current xml node to it's parent node
					GETXML->SetToParent(pXML->GetXML());
				}
			}
			else
			{
				SAFE_DELETE_ARRAY(m_piPrereqOrBonuses);
			}
		}

		// set the current xml node to it's parent node
		GETXML->SetToParent(pXML->GetXML());
	}
/************************************************************************************************/
/* XMLCOPY                                 10/10/07                                MRGENIE      */
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

	pXML->SetVariableListTagPair(&m_piHappinessTraits, "HappinessTraits", sizeof(GC.getTraitInfo((TraitTypes)0)), GC.getNumTraitInfos());

	pXML->GetChildXmlValByName(szTextVal, "NoBonus");
	m_iNoBonus = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "PowerBonus");
	m_iPowerBonus = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "FreeBonus");
	m_iFreeBonus = pXML->FindInInfoClass(szTextVal);
	pXML->GetChildXmlValByName(&m_iNumFreeBonuses, "iNumFreeBonuses");

	if(GETXML->SetToChildByTagName( pXML->GetXML(), "ExtraFreeBonuses"))
	{
		if(GETXML->SetToChild( pXML->GetXML() ))
		{

			if (GETXML->LocateFirstSiblingNodeByTagName(pXML->GetXML(), "ExtraFreeBonus"))
			{
				do
				{
					pXML->GetChildXmlValByName(szTextVal, "FreeBonus");
					BonusTypes eBonus = (BonusTypes) pXML->FindInInfoClass(szTextVal);
					int iNum = 0;
					pXML->GetChildXmlValByName(&iNum, "iNumFreeBonuses");
					m_aExtraFreeBonuses.push_back(std::pair<BonusTypes,int>(eBonus, iNum));
				} while(GETXML->NextSibling(pXML->GetXML()));
			}
			GETXML->SetToParent( pXML->GetXML() );
		}
		GETXML->SetToParent( pXML->GetXML() );
	}

    // DarkLunaPhantom - FreeBuilding accepts lists.
	pXML->GetChildXmlValByName(szTextVal, "FreeBuilding");
	std::vector<CvString> tokens;
	szTextVal.getTokens(",", tokens);
    m_aiFreeBuildingClass.clear();
	for (int i = 0; i < (int)tokens.size(); ++i)
	{
		int iFreeBuildingClass = pXML->FindInInfoClass(tokens[i]);
		if (iFreeBuildingClass != NO_BUILDINGCLASS)
		{
			m_aiFreeBuildingClass.push_back(iFreeBuildingClass);
		}
	}
    
    // DarkLunaPhantom - Extra FreeBuilding entries. In separate function for code readability.
    pXML->setExtraFreeBuildings(m_aiExtraFreeBuildingClass, m_abExtraFreeBuildingConnected, m_abExtraFreeBuildingContinuous);

	pXML->GetChildXmlValByName(szTextVal, "FreeAreaBuilding");
	m_iFreeAreaBuildingClass = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "FreeTradeRegionBuilding");
	m_iFreeTradeRegionBuildingClass = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "FreePromotion");
	m_iFreePromotion = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "CivicOption");
	m_iCivicOption = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "GreatPeopleUnitClass");
	m_iGreatPeopleUnitClass = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "DiploVoteType");
	m_iVoteSourceType = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(&m_iGreatPeopleRateChange, "iGreatPeopleRateChange");
	pXML->GetChildXmlValByName(&m_bTeamShare, "bTeamShare");
	pXML->GetChildXmlValByName(&m_bAutoBuild, "bAutoBuild");
	pXML->GetChildXmlValByName(&m_bWater, "bWater");
	pXML->GetChildXmlValByName(&m_bRiver, "bRiver");
	pXML->GetChildXmlValByName(&m_bFreshWater, "bFreshWater");
	pXML->GetChildXmlValByName(&m_bPower, "bPower");
	pXML->GetChildXmlValByName(&m_bNoPower, "bNoPower");
	pXML->GetChildXmlValByName(&m_bDirtyPower, "bDirtyPower");
	pXML->GetChildXmlValByName(&m_bAreaCleanPower, "bAreaCleanPower");
	pXML->GetChildXmlValByName(&m_bNoHolyCity, "bNoHolyCity");
	pXML->GetChildXmlValByName(&m_bAreaBorderObstacle, "bBorderObstacle");
	pXML->GetChildXmlValByName(&m_bForceTeamVoteEligible, "bForceTeamVoteEligible");
	pXML->GetChildXmlValByName(&m_bCapital, "bCapital");
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
	// Dale - AB: Bombing START
	pXML->GetChildXmlValByName(&m_iDCMAirbombMission,"iDCMAirbombMission");
	// Dale - AB: Bombing END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
	pXML->GetChildXmlValByName(&m_bGovernmentCenter, "bGovernmentCenter");
	pXML->GetChildXmlValByName(&m_bGoldenAge, "bGoldenAge");
	pXML->GetChildXmlValByName(&m_bAllowsNukes, "bAllowsNukes");
	pXML->GetChildXmlValByName(&m_bMapCentering, "bMapCentering");
	pXML->GetChildXmlValByName(&m_bNoUnhappiness, "bNoUnhappiness");
	pXML->GetChildXmlValByName(&m_bNoUnhealthyPopulation, "bNoUnhealthyPopulation");
	pXML->GetChildXmlValByName(&m_bBuildingOnlyHealthy, "bBuildingOnlyHealthy");
	pXML->GetChildXmlValByName(&m_bNeverCapture, "bNeverCapture");
	pXML->GetChildXmlValByName(&m_bNukeImmune, "bNukeImmune");
	pXML->GetChildXmlValByName(&m_bPrereqReligion, "bPrereqReligion");
	pXML->GetChildXmlValByName(&m_bFreeFromStateReligion, "bFreeFromStateReligion");
	pXML->GetChildXmlValByName(&m_bCenterInCity, "bCenterInCity");
	pXML->GetChildXmlValByName(&m_bStateReligion, "bStateReligion");
	pXML->GetChildXmlValByName(&m_iAIWeight, "iAIWeight");
	pXML->GetChildXmlValByName(&m_iProductionCost, "iCost");
	pXML->GetChildXmlValByName(&m_iHurryCostModifier, "iHurryCostModifier");
	pXML->GetChildXmlValByName(&m_iHurryAngerModifier, "iHurryAngerModifier");
	pXML->GetChildXmlValByName(&m_iAdvancedStartCost, "iAdvancedStartCost");
	pXML->GetChildXmlValByName(&m_iAdvancedStartCostIncrease, "iAdvancedStartCostIncrease");
	pXML->GetChildXmlValByName(&m_iMinAreaSize, "iMinAreaSize");
	pXML->GetChildXmlValByName(&m_iConquestProbability, "iConquestProb");
	pXML->GetChildXmlValByName(&m_iNumCitiesPrereq, "iCitiesPrereq");
	pXML->GetChildXmlValByName(&m_iNumTeamsPrereq, "iTeamsPrereq");
	pXML->GetChildXmlValByName(&m_iUnitLevelPrereq, "iLevelPrereq");
	pXML->GetChildXmlValByName(&m_iMinLatitude, "iMinLatitude");
	pXML->GetChildXmlValByName(&m_iMaxLatitude, "iMaxLatitude", 90);
	pXML->GetChildXmlValByName(&m_iGreatPeopleRateModifier, "iGreatPeopleRateModifier");
	pXML->GetChildXmlValByName(&m_iGreatGeneralRateModifier, "iGreatGeneralRateModifier");
	pXML->GetChildXmlValByName(&m_iDomesticGreatGeneralRateModifier, "iDomesticGreatGeneralRateModifier");
	pXML->GetChildXmlValByName(&m_iGlobalGreatPeopleRateModifier, "iGlobalGreatPeopleRateModifier");
	pXML->GetChildXmlValByName(&m_iAnarchyModifier, "iAnarchyModifier");
	pXML->GetChildXmlValByName(&m_iGoldenAgeModifier, "iGoldenAgeModifier");
	pXML->GetChildXmlValByName(&m_iGlobalHurryModifier, "iGlobalHurryModifier");
	pXML->GetChildXmlValByName(&m_iFreeExperience, "iExperience");
	pXML->GetChildXmlValByName(&m_iGlobalFreeExperience, "iGlobalExperience");
	pXML->GetChildXmlValByName(&m_iFoodKept, "iFoodKept");
	//doto culture safe zone
	pXML->GetChildXmlValByName(&m_iNoForeignCityCultureGrowth, "iNoForeignCityCultureGrowth");
	//doto culture safe zone
	pXML->GetChildXmlValByName(&m_iAirlift, "iAirlift");
	pXML->GetChildXmlValByName(&m_iAirModifier, "iAirModifier");
	pXML->GetChildXmlValByName(&m_iAirUnitCapacity, "iAirUnitCapacity");
	pXML->GetChildXmlValByName(&m_iWonderCapacityIncrement, "iWonderCapacityIncrement");
	pXML->GetChildXmlValByName(&m_iNukeModifier, "iNukeModifier");
	pXML->GetChildXmlValByName(&m_iNukeExplosionRand, "iNukeExplosionRand");
	pXML->GetChildXmlValByName(&m_iFreeSpecialist, "iFreeSpecialist");
	pXML->GetChildXmlValByName(&m_iAreaFreeSpecialist, "iAreaFreeSpecialist");
	pXML->GetChildXmlValByName(&m_iGlobalFreeSpecialist, "iGlobalFreeSpecialist");
	pXML->GetChildXmlValByName(&m_iMaintenanceModifier, "iMaintenanceModifier");
	//DPII < Maintenance Modifier >
    pXML->GetChildXmlValByName(&m_iGlobalMaintenanceModifier, "iGlobalMaintenanceModifier");
    pXML->GetChildXmlValByName(&m_iAreaMaintenanceModifier, "iAreaMaintenanceModifier");
    pXML->GetChildXmlValByName(&m_iOtherAreaMaintenanceModifier, "iOtherAreaMaintenanceModifier");
	pXML->GetChildXmlValByName(&m_iDistanceMaintenanceModifier, "iDistanceMaintenanceModifier");
	pXML->GetChildXmlValByName(&m_iNumCitiesMaintenanceModifier, "iNumCitiesMaintenanceModifier");
	pXML->GetChildXmlValByName(&m_iCoastalDistanceMaintenanceModifier, "iCoastalDistanceMaintenanceModifier");
	pXML->GetChildXmlValByName(&m_iConnectedCityMaintenanceModifier, "iConnectedCityMaintenanceModifier");
	//DPII < Maintenance Modifier >
	pXML->GetChildXmlValByName(&m_iWarWearinessModifier, "iWarWearinessModifier");
	pXML->GetChildXmlValByName(&m_iGlobalWarWearinessModifier, "iGlobalWarWearinessModifier");
	pXML->GetChildXmlValByName(&m_iEnemyWarWearinessModifier, "iEnemyWarWearinessModifier");
	pXML->GetChildXmlValByName(&m_iHealRateChange, "iHealRateChange");
	pXML->GetChildXmlValByName(&m_iHealth, "iHealth");
	pXML->GetChildXmlValByName(&m_iAreaHealth, "iAreaHealth");
	pXML->GetChildXmlValByName(&m_iGlobalHealth, "iGlobalHealth");
	pXML->GetChildXmlValByName(&m_iHappiness, "iHappiness");
	pXML->GetChildXmlValByName(&m_iAreaHappiness, "iAreaHappiness");
	pXML->GetChildXmlValByName(&m_iGlobalHappiness, "iGlobalHappiness");
	pXML->GetChildXmlValByName(&m_iStateReligionHappiness, "iStateReligionHappiness");
	pXML->GetChildXmlValByName(&m_iStateReligionHealth, "iStateReligionHealth");
	pXML->GetChildXmlValByName(&m_iWorkerSpeedModifier, "iWorkerSpeedModifier");
	pXML->GetChildXmlValByName(&m_iMilitaryProductionModifier, "iMilitaryProductionModifier");
	pXML->GetChildXmlValByName(&m_iSpaceProductionModifier, "iSpaceProductionModifier");
	pXML->GetChildXmlValByName(&m_iGlobalSpaceProductionModifier, "iGlobalSpaceProductionModifier");
	pXML->GetChildXmlValByName(&m_iTradeRoutes, "iTradeRoutes");
	pXML->GetChildXmlValByName(&m_iInterestPercent, "iInterestPercent");
	pXML->GetChildXmlValByName(&m_iInterestMax, "iInterestMax");
	pXML->GetChildXmlValByName(&m_iCoastalTradeRoutes, "iCoastalTradeRoutes");
	pXML->GetChildXmlValByName(&m_iGlobalTradeRoutes, "iGlobalTradeRoutes");
	pXML->GetChildXmlValByName(&m_iTradeRouteModifier, "iTradeRouteModifier");
	pXML->GetChildXmlValByName(&m_iForeignTradeRouteModifier, "iForeignTradeRouteModifier");
	pXML->GetChildXmlValByName(&m_iGlobalPopulationChange, "iGlobalPopulationChange");
	pXML->GetChildXmlValByName(&m_iFreeTechs, "iFreeTechs");

	pXML->GetChildXmlValByName(szTextVal, "FreeSpecialTech");
	m_eFreeSpecialTech = (TechTypes) pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(&m_iDefenseModifier, "iDefense");
	pXML->GetChildXmlValByName(&m_iBombardDefenseModifier, "iBombardDefense");
	pXML->GetChildXmlValByName(&m_iAllCityDefenseModifier, "iAllCityDefense");
	pXML->GetChildXmlValByName(&m_iEspionageDefenseModifier, "iEspionageDefense");

/********************************************************************************/
/**		REVDCM									4/09/10				phungus420	*/
/**																				*/
/**		Building Effects														*/
/********************************************************************************/
	pXML->GetChildXmlValByName(&m_iUnitUpgradePriceModifier, "iUnitUpgradePriceModifier");
	pXML->GetChildXmlValByName(&m_iRevIdxLocal, "iRevIdxLocal");
	pXML->GetChildXmlValByName(&m_iRevIdxNational, "iRevIdxNational");
	pXML->GetChildXmlValByName(&m_iRevIdxDistanceModifier, "iRevIdxDistanceModifier");
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

	pXML->GetChildXmlValByName(&m_iAssetValue, "iAsset");
	pXML->GetChildXmlValByName(&m_iPowerValue, "iPower");
	pXML->GetChildXmlValByName(&m_fVisibilityPriority, "fVisibilityPriority");
	setNotShowInCity();

	// if we can set the current xml node to it's next sibling
	if (GETXML->SetToChildByTagName(pXML->GetXML(),"SeaPlotYieldChanges"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piSeaPlotYieldChange);
		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_piSeaPlotYieldChange);
	}

	// if we can set the current xml node to it's next sibling
	if (GETXML->SetToChildByTagName(pXML->GetXML(),"RiverPlotYieldChanges"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piRiverPlotYieldChange);
		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_piRiverPlotYieldChange);
	}

	// if we can set the current xml node to it's next sibling
	if (GETXML->SetToChildByTagName(pXML->GetXML(),"GlobalSeaPlotYieldChanges"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piGlobalSeaPlotYieldChange);
		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_piGlobalSeaPlotYieldChange);
	}

	// if we can set the current xml node to it's next sibling
	if (GETXML->SetToChildByTagName(pXML->GetXML(),"YieldChanges"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piYieldChange);
		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_piYieldChange);
	}

	// if we can set the current xml node to it's next sibling
	if (GETXML->SetToChildByTagName(pXML->GetXML(),"YieldModifiers"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piYieldModifier);
		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_piYieldModifier);
	}

	// if we can set the current xml node to it's next sibling
	if (GETXML->SetToChildByTagName(pXML->GetXML(),"PowerYieldModifiers"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piPowerYieldModifier);
		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_piPowerYieldModifier);
	}

	// if we can set the current xml node to it's next sibling
	if (GETXML->SetToChildByTagName(pXML->GetXML(),"AreaYieldModifiers"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piAreaYieldModifier);
		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_piAreaYieldModifier);
	}

	// if we can set the current xml node to it's next sibling
	if (GETXML->SetToChildByTagName(pXML->GetXML(),"GlobalYieldModifiers"))
	{
		// call the function that sets the yield change variable
		pXML->SetYields(&m_piGlobalYieldModifier);
		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_piGlobalYieldModifier);
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"CommerceChanges"))
	{
		pXML->SetCommerce(&m_piCommerceChange);
		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_piCommerceChange);
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"ObsoleteSafeCommerceChanges"))
	{
		pXML->SetCommerce(&m_piObsoleteSafeCommerceChange);
		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_piObsoleteSafeCommerceChange);
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"CommerceChangeDoubleTimes"))
	{
		pXML->SetCommerce(&m_piCommerceChangeDoubleTime);
		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_piCommerceChangeDoubleTime);
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"CommerceModifiers"))
	{
		pXML->SetCommerce(&m_piCommerceModifier);
		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_piCommerceModifier);
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"GlobalCommerceModifiers"))
	{
		pXML->SetCommerce(&m_piGlobalCommerceModifier);
		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_piGlobalCommerceModifier);
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"SpecialistExtraCommerces"))
	{
		pXML->SetCommerce(&m_piSpecialistExtraCommerce);
		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_piSpecialistExtraCommerce);
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"StateReligionCommerces"))
	{
		pXML->SetCommerce(&m_piStateReligionCommerce);
		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_piStateReligionCommerce);
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"CommerceHappinesses"))
	{
		pXML->SetCommerce(&m_piCommerceHappiness);
		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_piCommerceHappiness);
	}

	pXML->SetVariableListTagPair(&m_piReligionChange, "ReligionChanges", sizeof(GC.getReligionInfo((ReligionTypes)0)), GC.getNumReligionInfos());

	pXML->SetVariableListTagPair(&m_piSpecialistCount, "SpecialistCounts", sizeof(GC.getSpecialistInfo((SpecialistTypes)0)), GC.getNumSpecialistInfos());
	pXML->SetVariableListTagPair(&m_piFreeSpecialistCount, "FreeSpecialistCounts", sizeof(GC.getSpecialistInfo((SpecialistTypes)0)), GC.getNumSpecialistInfos());

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"CommerceFlexibles"))
	{
		pXML->SetCommerce(&m_pbCommerceFlexible);
		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_pbCommerceFlexible);
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"CommerceChangeOriginalOwners"))
	{
		pXML->SetCommerce(&m_pbCommerceChangeOriginalOwner);
		GETXML->SetToParent(pXML->GetXML());
	}
	else
	{
		SAFE_DELETE_ARRAY(m_pbCommerceChangeOriginalOwner);
	}

	pXML->GetChildXmlValByName(szTextVal, "ConstructSound");
	setConstructSound(szTextVal);

	pXML->SetVariableListTagPair(&m_piBonusHealthChanges, "BonusHealthChanges", sizeof(GC.getBonusInfo((BonusTypes)0)), GC.getNumBonusInfos());
	pXML->SetVariableListTagPair(&m_piBonusHappinessChanges, "BonusHappinessChanges", sizeof(GC.getBonusInfo((BonusTypes)0)), GC.getNumBonusInfos());
	pXML->SetVariableListTagPair(&m_piBonusProductionModifier, "BonusProductionModifiers", sizeof(GC.getBonusInfo((BonusTypes)0)), GC.getNumBonusInfos());

	pXML->SetVariableListTagPair(&m_piUnitCombatFreeExperience, "UnitCombatFreeExperiences", sizeof(GC.getUnitCombatInfo((UnitCombatTypes)0)), GC.getNumUnitCombatInfos());

	pXML->SetVariableListTagPair(&m_piDomainFreeExperience, "DomainFreeExperiences", sizeof(GC.getDomainInfo((DomainTypes)0)), NUM_DOMAIN_TYPES);
	pXML->SetVariableListTagPair(&m_piDomainProductionModifier, "DomainProductionModifiers", sizeof(GC.getDomainInfo((DomainTypes)0)), NUM_DOMAIN_TYPES);

	pXML->SetVariableListTagPair(&m_piPrereqNumOfBuildingClass, "PrereqBuildingClasses", sizeof(GC.getBuildingClassInfo((BuildingClassTypes)0)), GC.getNumBuildingClassInfos());
	pXML->SetVariableListTagPair(&m_pbBuildingClassNeededInCity, "BuildingClassNeededs", sizeof(GC.getBuildingClassInfo((BuildingClassTypes)0)), GC.getNumBuildingClassInfos());

	m_aGlobalBuildingCommerceChanges.clear();
	if (GETXML->SetToChildByTagName(pXML->GetXML(),"GlobalBuildingExtraCommerces"))
	{
		if (pXML->SkipToNextVal())
		{
			int iNumSibs = GETXML->GetNumChildren(pXML->GetXML());

			if (0 < iNumSibs)
			{
				if (GETXML->SetToChild(pXML->GetXML()))
				{
					for (int j = 0; j < iNumSibs; ++j)
					{
						if (pXML->GetChildXmlVal(szTextVal))
						{
							BuildingCommerceChange kChange;
							kChange.eBuildingClass = (BuildingClassTypes)pXML->FindInInfoClass(szTextVal);
							pXML->GetNextXmlVal(szTextVal);
							kChange.eCommerce = (CommerceTypes)pXML->FindInInfoClass(szTextVal);
							pXML->GetNextXmlVal(&kChange.iChange);
							m_aGlobalBuildingCommerceChanges.push_back(kChange);

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

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"SpecialistYieldChanges"))
	{
		iNumChildren = GETXML->GetNumChildren(pXML->GetXML());

		if (GETXML->SetToChildByTagName(pXML->GetXML(),"SpecialistYieldChange"))
		{
			for(j=0;j<iNumChildren;j++)
			{
				pXML->GetChildXmlValByName(szTextVal, "SpecialistType");
				k = pXML->FindInInfoClass(szTextVal);
				if (k > -1)
				{
					if ( m_ppaiSpecialistYieldChange == NULL )
					{
						m_ppaiSpecialistYieldChange = new int*[GC.getNumSpecialistInfos()];

						for(int i = 0; i < GC.getNumSpecialistInfos(); i++)
						{
							m_ppaiSpecialistYieldChange[i] = NULL;
						}
					}
					if (GETXML->SetToChildByTagName(pXML->GetXML(),"YieldChanges"))
					{
						// call the function that sets the yield change variable
						pXML->SetYields(&m_ppaiSpecialistYieldChange[k]);
						GETXML->SetToParent(pXML->GetXML());
						m_bAnySpecialistYieldChanges = true;
					}
				}

				if (!GETXML->NextSibling(pXML->GetXML()))
				{
					break;
				}
			}

			// set the current xml node to it's parent node
			GETXML->SetToParent(pXML->GetXML());
		}

		// set the current xml node to it's parent node
		GETXML->SetToParent(pXML->GetXML());
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"BonusYieldModifiers"))
	{
		iNumChildren = GETXML->GetNumChildren(pXML->GetXML());

		if (GETXML->SetToChildByTagName(pXML->GetXML(),"BonusYieldModifier"))
		{
			for(j=0;j<iNumChildren;j++)
			{
				pXML->GetChildXmlValByName(szTextVal, "BonusType");
				k = pXML->FindInInfoClass(szTextVal);
				if (k > -1)
				{
					if ( m_ppaiBonusYieldModifier == NULL )
					{
						m_ppaiBonusYieldModifier = new int*[GC.getNumBonusInfos()];

						for(int i = 0; i < GC.getNumBonusInfos(); i++)
						{
							m_ppaiBonusYieldModifier[i] = 0;
						}
					}
					if (GETXML->SetToChildByTagName(pXML->GetXML(),"YieldModifiers"))
					{
						// call the function that sets the yield change variable
						pXML->SetYields(&m_ppaiBonusYieldModifier[k]);
						GETXML->SetToParent(pXML->GetXML());
						m_bAnyBonusYieldModifiers = true;
					}
				}

				if (!GETXML->NextSibling(pXML->GetXML()))
				{
					break;
				}
			}

			// set the current xml node to it's parent node
			GETXML->SetToParent(pXML->GetXML());
		}

		// set the current xml node to it's parent node
		GETXML->SetToParent(pXML->GetXML());
	}

	pXML->SetVariableListTagPair(&m_piFlavorValue, "Flavors", GC.getFlavorTypes(), GC.getNumFlavorTypes());
	pXML->SetVariableListTagPair(&m_piImprovementFreeSpecialist, "ImprovementFreeSpecialists", sizeof(GC.getImprovementInfo((ImprovementTypes)0)), GC.getNumImprovementInfos());

	pXML->SetVariableListTagPair(&m_piBuildingHappinessChanges, "BuildingHappinessChanges", sizeof(GC.getBuildingClassInfo((BuildingClassTypes)0)), GC.getNumBuildingClassInfos());


/************************************************************************************************/
/* Afforess	                  Start		 12/9/09                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	pXML->GetChildXmlValByName(&m_iRevIdxLocal, "iRevolutionIndexModifier");
	pXML->GetChildXmlValByName(&m_bApplyFreePromotionOnMove, "bApplyFreePromotionOnMove");
	pXML->GetChildXmlValByName(&m_bNoEnemyPillagingIncome, "bNoEnemyPillagingIncome");
	pXML->GetChildXmlValByName(&m_bProvidesFreshWater, "bProvidesFreshWater");
	pXML->GetChildXmlValByName(&m_bForceAllTradeRoutes, "bForceAllTradeRoutes");
	pXML->GetChildXmlValByName(&m_iPillageGoldModifier, "iPillageGoldModifier");
	pXML->GetChildXmlValByName(&m_iWorldTradeRoutes, "iWorldTradeRoutes");
	pXML->GetChildXmlValByName(&m_bBuildOnlyOnPeaks, "bBuildOnlyOnPeaks");
	pXML->GetChildXmlValByName(&m_bPrereqPower, "bPrereqPower");
	pXML->GetChildXmlValByName(&m_bForceNoPrereqScaling, "bForceNoPrereqScaling");
	pXML->GetChildXmlValByName(&m_bPrereqWar, "bPrereqWar");
	pXML->GetChildXmlValByName(&m_bRequiresActiveCivics, "bRequiresActiveCivics");
	pXML->GetChildXmlValByName(&m_bZoneOfControl, "bZoneOfControl");
	pXML->GetChildXmlValByName(&m_iGlobalPopulationgrowthratepercentage, "iGlobalPopulationgrowthratepercentage");
	pXML->GetChildXmlValByName(&m_iPopulationgrowthratepercentage, "iPopulationgrowthratepercentage");
	pXML->GetChildXmlValByName(&m_iLineOfSight, "iLineOfSight");
	pXML->GetChildXmlValByName(&m_iInflationModifier, "iInflationModifier");
	pXML->GetChildXmlValByName(&m_iInvasionChance, "iInvasionChance");
	pXML->GetChildXmlValByName(&m_iAdjacentDamagePercent, "iAdjacentDamagePercent");
	pXML->GetChildXmlValByName(&m_iPrereqPopulation, "iPrereqPopulation");
	pXML->GetChildXmlValByName(&m_iWorkableRadius, "iWorkableRadius");
	pXML->GetChildXmlValByName(&m_iOccupationTimeModifier, "iOccupationTimeModifier");
	pXML->GetChildXmlValByName(&m_iNoEntryDefenseLevel, "iNoEntryDefenseLevel");
	pXML->GetChildXmlValByName(&m_iNumUnitFullHeal, "iNumUnitFullHeal");
	pXML->GetChildXmlValByName(&m_iNumPopulationEmployed, "iNumPopulationEmployed");
	pXML->GetChildXmlValByName(&m_iHealthPercentPerPopulation, "iHealthPercentPerPopulation");
	pXML->GetChildXmlValByName(&m_iHappinessPercentPerPopulation, "iHappinessPercentPerPopulation");
	pXML->GetChildXmlValByName(&m_iCapitalExtraCommerce, "iCapitalExtraCommerce");
	pXML->GetChildXmlValByName(&m_iForeignCapitalExtraCommerce, "iForeignCapitalExtraCommerce");
	pXML->GetChildXmlValByName(&m_iDomesticConnectedCommerce, "iDomesticConnectedCommerce");
	pXML->GetChildXmlValByName(&m_iCitiesRequiredPerDomesticConnectedCity, "iCitiesRequiredPerDomesticConnectedCity");
	pXML->GetChildXmlValByName(&m_iMaxDomesticConnectedCommerce, "iMaxDomesticConnectedCommerce");
	pXML->GetChildXmlValByName(&m_iForeignConnectedCommerce, "iForeignConnectedCommerce");
	pXML->GetChildXmlValByName(&m_iCitiesRequiredPerForeignConnectedCity, "iCitiesRequiredPerForeignConnectedCity");
	pXML->GetChildXmlValByName(&m_iMaxForeignConnectedCommerce, "iMaxForeignConnectedCommerce");
	pXML->GetChildXmlValByName(&m_iDiplomacyChange, "iDiplomacyChange");

	pXML->GetChildXmlValByName(&m_iCrime, "iCrime");
	pXML->GetChildXmlValByName(&m_iCrimePerPop, "iCrimePerPop");

	pXML->GetChildXmlValByName(&m_bProtectedCulture, "bProtectedCulture");
	pXML->GetChildXmlValByName(&m_bAllowsAmbassadors, "bAllowsAmbassadors");

	pXML->GetChildXmlValByName(szTextVal, "ProductionContinueBuildingClass");
	m_iProductionContinueBuildingClass = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "PrereqAnyoneBuildingClass");
	m_iPrereqAnyoneBuildingClass = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "ExtendsBuildingClass");
	m_iExtendsBuildingClass = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "PrereqCultureLevel");
	m_iPrereqCultureLevel = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "FreePromotion_2");
	m_iFreePromotion_2 = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "FreePromotion_3");
	m_iFreePromotion_3 = pXML->FindInInfoClass(szTextVal);

	pXML->GetChildXmlValByName(szTextVal, "VicinityBonus");
	m_iPrereqVicinityBonus = pXML->FindInInfoClass(szTextVal);

	pXML->SetVariableListTagPair(&m_piMemoryTypeModifier, "MemoryTypeModifiers", sizeof(GC.getMemoryInfo((MemoryTypes)0)), NUM_MEMORY_TYPES);
	pXML->SetVariableListTagPair(&m_pbPrereqOrBuildingClass, "PrereqOrBuildingClasses", sizeof(GC.getBuildingClassInfo((BuildingClassTypes)0)), GC.getNumBuildingClassInfos());
	pXML->SetVariableListTagPair(&m_piTechHappinessChanges, "TechHappinessChanges", sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());
	pXML->SetVariableListTagPair(&m_piTechHealthChanges, "TechHealthChanges", sizeof(GC.getTechInfo((TechTypes)0)), GC.getNumTechInfos());
	pXML->SetVariableListTagPair(&m_pbReplaceBuildingClass, "ReplaceBuildings", sizeof(GC.getBuildingClassInfo((BuildingClassTypes)0)), GC.getNumBuildingClassInfos());
	pXML->SetVariableListTagPair(&m_piUnitClassProductionModifier, "UnitClassProductionModifiers", sizeof(GC.getUnitClassInfo((UnitClassTypes)0)), GC.getNumUnitClassInfos());
	pXML->SetVariableListTagPair(&m_pbPrereqOrTerrain, "PrereqOrTerrain", sizeof(GC.getTerrainInfo((TerrainTypes)0)), GC.getNumTerrainInfos());
	pXML->SetVariableListTagPair(&m_pbPrereqAndTerrain, "PrereqAndTerrain", sizeof(GC.getTerrainInfo((TerrainTypes)0)), GC.getNumTerrainInfos());
	pXML->SetVariableListTagPair(&m_pbPrereqOrImprovement, "PrereqOrImprovement", sizeof(GC.getImprovementInfo((ImprovementTypes)0)), GC.getNumImprovementInfos());
	pXML->SetVariableListTagPair(&m_pbPrereqOrFeature, "PrereqOrFeature", sizeof(GC.getFeatureInfo((FeatureTypes)0)), GC.getNumFeatureInfos());
	pXML->SetVariableListTagPair(&m_pbPrereqNotBuildingClass, "PrereqNotBuildingClasses", sizeof(GC.getBuildingClassInfo((BuildingClassTypes)0)), GC.getNumBuildingClassInfos());
	pXML->SetVariableListTagPair(&m_piBuildingClassProductionModifier, "BuildingClassProductionModifiers", sizeof(GC.getBuildingClassInfo((BuildingClassTypes)0)), GC.getNumBuildingClassInfos());
	pXML->SetVariableListTagPair(&m_piGlobalBuildingClassProductionModifier, "GlobalBuildingClassProductionModifiers", sizeof(GC.getBuildingClassInfo((BuildingClassTypes)0)), GC.getNumBuildingClassInfos());
	pXML->SetVariableListTagPair(&m_piBonusDefenseChanges, "BonusDefenseChanges", sizeof(GC.getBonusInfo((BonusTypes)0)), GC.getNumBonusInfos());
	pXML->SetVariableListTagPair(&m_piUnitCombatExtraStrength, "UnitCombatExtraStrengths", sizeof(GC.getUnitCombatInfo((UnitCombatTypes)0)), GC.getNumUnitCombatInfos());

	pXML->SetVariableListTagPair(&m_pbPrereqOrGameSpeed, "PrereqOrGameSpeeds", sizeof(GC.getGameSpeedInfo((GameSpeedTypes)0)), GC.getNumGameSpeedInfos());

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"CommerceAttacks"))
	{
		pXML->SetCommerce(&m_piCommerceAttacks);
		GETXML->SetToParent(pXML->GetXML());
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"BonusCommerceModifiers"))
	{
		iNumChildren = GETXML->GetNumChildren(pXML->GetXML());

		if (GETXML->SetToChildByTagName(pXML->GetXML(),"BonusCommerceModifier"))
		{
			for(j=0;j<iNumChildren;j++)
			{
				pXML->GetChildXmlValByName(szTextVal, "BonusType");
				k = pXML->FindInInfoClass(szTextVal);
				if (k > -1)
				{
					if ( m_ppaiBonusCommerceModifier == NULL )
					{
						m_ppaiBonusCommerceModifier = new int*[GC.getNumBonusInfos()];

						for(int i = 0; i < GC.getNumBonusInfos(); i++)
						{
							m_ppaiBonusCommerceModifier[i] = NULL;
						}
					}
					if (GETXML->SetToChildByTagName(pXML->GetXML(),"CommerceModifiers"))
					{
						// call the function that sets the commerce change variable
						pXML->SetCommerce(&m_ppaiBonusCommerceModifier[k]);
						GETXML->SetToParent(pXML->GetXML());
						m_bAnyBonusCommerceModifiers = true;
					}
				}

				if (!GETXML->NextSibling(pXML->GetXML()))
				{
					break;
				}
			}

			// set the current xml node to it's parent node
			GETXML->SetToParent(pXML->GetXML());
		}

		// set the current xml node to it's parent node
		GETXML->SetToParent(pXML->GetXML());
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"BonusCommercePercentChanges"))
	{
		iNumChildren = GETXML->GetNumChildren(pXML->GetXML());

		if (GETXML->SetToChildByTagName(pXML->GetXML(),"BonusCommercePercentChange"))
		{
			for(j=0;j<iNumChildren;j++)
			{
				pXML->GetChildXmlValByName(szTextVal, "BonusType");
				k = pXML->FindInInfoClass(szTextVal);
				if (k > -1)
				{
					if ( m_ppaiBonusCommercePercentChanges == NULL )
					{
						m_ppaiBonusCommercePercentChanges = new int*[GC.getNumBonusInfos()];

						for(int i = 0; i < GC.getNumBonusInfos(); i++)
						{
							m_ppaiBonusCommercePercentChanges[i] = NULL;
						}
					}
					if (GETXML->SetToChildByTagName(pXML->GetXML(),"CommercePercents"))
					{
						// call the function that sets the commerce change variable
						pXML->SetCommerce(&m_ppaiBonusCommercePercentChanges[k]);
						GETXML->SetToParent(pXML->GetXML());
						m_bAnyBonusCommercePercentChanges = true;
					}
				}

				if (!GETXML->NextSibling(pXML->GetXML()))
				{
					break;
				}
			}

			// set the current xml node to it's parent node
			GETXML->SetToParent(pXML->GetXML());
		}

		// set the current xml node to it's parent node
		GETXML->SetToParent(pXML->GetXML());
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"BonusYieldChanges"))
	{
		iNumChildren = GETXML->GetNumChildren(pXML->GetXML());

		if (GETXML->SetToChildByTagName(pXML->GetXML(),"BonusYieldChange"))
		{
			for(j=0;j<iNumChildren;j++)
			{
				pXML->GetChildXmlValByName(szTextVal, "BonusType");
				k = pXML->FindInInfoClass(szTextVal);
				if (k > -1)
				{
					if ( m_ppaiBonusYieldChanges == NULL )
					{
						m_ppaiBonusYieldChanges = new int*[GC.getNumBonusInfos()];

						for(int i = 0; i < GC.getNumBonusInfos(); i++)
						{
							m_ppaiBonusYieldChanges[i] = NULL;
						}
					}
					if (GETXML->SetToChildByTagName(pXML->GetXML(),"YieldChanges"))
					{
						// call the function that sets the commerce change variable
						pXML->SetCommerce(&m_ppaiBonusYieldChanges[k]);
						GETXML->SetToParent(pXML->GetXML());
						m_bAnyBonusYieldChanges = true;
					}
				}

				if (!GETXML->NextSibling(pXML->GetXML()))
				{
					break;
				}
			}

			// set the current xml node to it's parent node
			GETXML->SetToParent(pXML->GetXML());
		}

		// set the current xml node to it's parent node
		GETXML->SetToParent(pXML->GetXML());
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"VicinityBonusYieldChanges"))
	{
		iNumChildren = GETXML->GetNumChildren(pXML->GetXML());

		if (GETXML->SetToChildByTagName(pXML->GetXML(),"BonusYieldChange"))
		{
			for(j=0;j<iNumChildren;j++)
			{
				pXML->GetChildXmlValByName(szTextVal, "BonusType");
				k = pXML->FindInInfoClass(szTextVal);
				if (k > -1)
				{
					if ( m_ppaiVicinityBonusYieldChanges == NULL )
					{
						m_ppaiVicinityBonusYieldChanges = new int*[GC.getNumBonusInfos()];

						for(int i = 0; i < GC.getNumBonusInfos(); i++)
						{
							m_ppaiVicinityBonusYieldChanges[i] = 0;
						}
					}
					if (GETXML->SetToChildByTagName(pXML->GetXML(),"YieldChanges"))
					{
						// call the function that sets the commerce change variable
						pXML->SetCommerce(&m_ppaiVicinityBonusYieldChanges[k]);
						GETXML->SetToParent(pXML->GetXML());
						m_bAnyVicinityBonusYieldChanges = true;
					}
				}

				if (!GETXML->NextSibling(pXML->GetXML()))
				{
					break;
				}
			}

			// set the current xml node to it's parent node
			GETXML->SetToParent(pXML->GetXML());
		}

		// set the current xml node to it's parent node
		GETXML->SetToParent(pXML->GetXML());
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"PrereqOrCivics"))
	{
		if (pXML->SkipToNextVal())
		{
			int iNumSibs = GETXML->GetNumChildren(pXML->GetXML());
			bool bTemp = false;
			if (iNumSibs > 0)
			{
				if (GETXML->SetToChild(pXML->GetXML()))
				{
					for (int i=0;i<iNumSibs;i++)
					{
						if (pXML->GetChildXmlVal(szTextVal))
						{
                            m_aszPrereqOrCivicsforPass3.push_back(szTextVal);
                            pXML->GetNextXmlVal(&bTemp);
                            m_abPrereqOrCivicsforPass3.push_back(bTemp);
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

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"PrereqAndCivics"))
	{
		if (pXML->SkipToNextVal())
		{
			int iNumSibs = GETXML->GetNumChildren(pXML->GetXML());
			bool bTemp = false;
			if (iNumSibs > 0)
			{
				if (GETXML->SetToChild(pXML->GetXML()))
				{
					for (int i=0;i<iNumSibs;i++)
					{
						if (pXML->GetChildXmlVal(szTextVal))
						{
                            m_aszPrereqAndCivicsforPass3.push_back(szTextVal);
                            pXML->GetNextXmlVal(&bTemp);
                            m_abPrereqAndCivicsforPass3.push_back(bTemp);
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
			iNumChildren = GETXML->GetNumChildren(pXML->GetXML());

			if (0 < iNumChildren)
			{
				pXML->CvXMLLoadUtility::InitList(&m_piPrereqOrVicinityBonuses, GC.getNUM_BUILDING_PREREQ_OR_BONUSES(), -1);
				if (pXML->GetChildXmlVal(szTextVal))
				{
					FAssertMsg((iNumChildren <= GC.getNUM_BUILDING_PREREQ_OR_BONUSES()),"For loop iterator is greater than array size");
					for (j=0;j<iNumChildren;j++)
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

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"TechCommerceChanges"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = GETXML->GetNumChildren(pXML->GetXML());
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				if (0 < iNumSibs)
				{
					for (j=0;j<iNumSibs;j++)
					{
						pXML->GetChildXmlValByName(szTextVal, "PrereqTech");
						k = pXML->FindInInfoClass(szTextVal);

						if (k > -1)
						{
							if ( m_ppaiTechCommerceChange == NULL )
							{
								m_ppaiTechCommerceChange = new int*[GC.getNumTechInfos()];
								for(int i = 0; i < GC.getNumTechInfos(); i++)
								{
									m_ppaiTechCommerceChange[i] = NULL;
								}
							}
							// delete the array since it will be reallocated
							SAFE_DELETE_ARRAY(m_ppaiTechCommerceChange[k]);
							// if we can set the current xml node to it's next sibling
							if (GETXML->SetToChildByTagName(pXML->GetXML(),"TechCommerce"))
							{
								// call the function that sets the commerce change variable
								pXML->SetCommerce(&m_ppaiTechCommerceChange[k]);
								GETXML->SetToParent(pXML->GetXML());
								m_bAnyTechCommerceChanges = true;
							}
						}

						if (!GETXML->NextSibling(pXML->GetXML()))
						{
							break;
						}
					}
				}

				GETXML->SetToParent(pXML->GetXML());
			}
		}

		GETXML->SetToParent(pXML->GetXML());
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"TechYieldChanges"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = GETXML->GetNumChildren(pXML->GetXML());
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				if (0 < iNumSibs)
				{
					for (j=0;j<iNumSibs;j++)
					{
						pXML->GetChildXmlValByName(szTextVal, "PrereqTech");
						k = pXML->FindInInfoClass(szTextVal);

						if (k > -1)
						{
							if ( m_ppaiTechYieldChange == NULL )
							{
								m_ppaiTechYieldChange = new int*[GC.getNumTechInfos()];

								for(int i = 0; i < GC.getNumTechInfos(); i++)
								{
									m_ppaiTechYieldChange[i] = NULL;
								}
							}
							// delete the array since it will be reallocated
							SAFE_DELETE_ARRAY(m_ppaiTechYieldChange[k]);
							// if we can set the current xml node to it's next sibling
							if (GETXML->SetToChildByTagName(pXML->GetXML(),"TechYield"))
							{
								// call the function that sets the yield change variable
								pXML->SetYields(&m_ppaiTechYieldChange[k]);
								GETXML->SetToParent(pXML->GetXML());
								m_bAnyTechYieldChanges = true;
							}
						}

						if (!GETXML->NextSibling(pXML->GetXML()))
						{
							break;
						}
					}
				}

				GETXML->SetToParent(pXML->GetXML());
			}
		}

		GETXML->SetToParent(pXML->GetXML());
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"TechSpecialistChanges"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = GETXML->GetNumChildren(pXML->GetXML());
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				if (0 < iNumSibs)
				{
					for (j=0;j<iNumSibs;j++)
					{
						pXML->GetChildXmlValByName(szTextVal, "PrereqTech");
						k = pXML->FindInInfoClass(szTextVal);

						if (k > -1)
						{
							if ( m_ppaiTechSpecialistChange == NULL )
							{
								m_ppaiTechSpecialistChange = new int*[GC.getNumTechInfos()];

								for(int i = 0; i < GC.getNumTechInfos(); i++)
								{
									m_ppaiTechSpecialistChange[i] = NULL;
								}
							}
							pXML->SetVariableListTagPair(&m_ppaiTechSpecialistChange[k], "SpecialistCounts", sizeof(GC.getSpecialistInfo((SpecialistTypes)0)), GC.getNumSpecialistInfos());
							m_bAnyTechSpecialistChanges = true;
						}

						if (!GETXML->NextSibling(pXML->GetXML()))
						{
							break;
						}
					}
				}

				GETXML->SetToParent(pXML->GetXML());
			}
		}

		GETXML->SetToParent(pXML->GetXML());
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"TechCommerceModifiers"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = GETXML->GetNumChildren(pXML->GetXML());
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				if (0 < iNumSibs)
				{
					for (j=0;j<iNumSibs;j++)
					{
						pXML->GetChildXmlValByName(szTextVal, "PrereqTech");
						k = pXML->FindInInfoClass(szTextVal);

						if (k > -1)
						{
							if ( m_ppaiTechCommerceModifier == NULL )
							{
								m_ppaiTechCommerceModifier = new int*[GC.getNumTechInfos()];

								for(int i = 0; i < GC.getNumTechInfos(); i++ )
								{
									m_ppaiTechCommerceModifier[i] = NULL;
								}
							}
							if (GETXML->SetToChildByTagName(pXML->GetXML(),"TechCommerce"))
							{
								// call the function that sets the commerce change variable
								pXML->SetCommerce(&m_ppaiTechCommerceModifier[k]);
								GETXML->SetToParent(pXML->GetXML());
								m_bAnyTechCommerceModifiers = true;
							}
						}

						if (!GETXML->NextSibling(pXML->GetXML()))
						{
							break;
						}
					}
				}

				GETXML->SetToParent(pXML->GetXML());
			}
		}
		GETXML->SetToParent(pXML->GetXML());
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"TechYieldModifiers"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = GETXML->GetNumChildren(pXML->GetXML());
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				if (0 < iNumSibs)
				{
					for (j=0;j<iNumSibs;j++)
					{
						pXML->GetChildXmlValByName(szTextVal, "PrereqTech");
						k = pXML->FindInInfoClass(szTextVal);

						if (k > -1)
						{
							if ( m_ppaiTechYieldModifier == NULL )
							{
								m_ppaiTechYieldModifier = new int*[GC.getNumTechInfos()];

								for(int i = 0; i < GC.getNumTechInfos(); i++ )
								{
									m_ppaiTechYieldModifier[i] = NULL;
								}
							}
							// if we can set the current xml node to it's next sibling
							if (GETXML->SetToChildByTagName(pXML->GetXML(),"TechYield"))
							{
								// call the function that sets the yield change variable
								pXML->SetYields(&m_ppaiTechYieldModifier[k]);
								GETXML->SetToParent(pXML->GetXML());
								m_bAnyTechYieldModifiers = true;
							}
						}
						if (!GETXML->NextSibling(pXML->GetXML()))
						{
							break;
						}
					}
				}
				GETXML->SetToParent(pXML->GetXML());
			}
		}
		GETXML->SetToParent(pXML->GetXML());
	}
	int iIndex;

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"ImprovementYieldChanges"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = GETXML->GetNumChildren(pXML->GetXML());
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				if (0 < iNumSibs)
				{
					for (j=0;j<iNumSibs;j++)
					{
						pXML->GetChildXmlValByName(szTextVal, "ImprovementType");
						iIndex = pXML->FindInInfoClass(szTextVal);

						if (iIndex > -1)
						{
							if ( m_ppiImprovementYieldChanges == NULL )
							{
								m_ppiImprovementYieldChanges = new int*[GC.getNumImprovementInfos()];

								for(int i = 0; i < GC.getNumImprovementInfos(); i++ )
								{
									m_ppiImprovementYieldChanges[i] = NULL;
								}
							}
							// if we can set the current xml node to it's next sibling
							if (GETXML->SetToChildByTagName(pXML->GetXML(),"ImprovementYields"))
							{
								// call the function that sets the yield change variable
								pXML->SetYields(&m_ppiImprovementYieldChanges[iIndex]);
								GETXML->SetToParent(pXML->GetXML());
							}
						}

						if (!GETXML->NextSibling(pXML->GetXML()))
						{
							break;
						}
					}
				}

				GETXML->SetToParent(pXML->GetXML());
			}
		}

		GETXML->SetToParent(pXML->GetXML());
	}

	FAssertMsg((GC.getNumSpecialistInfos() > 0) && (NUM_COMMERCE_TYPES) > 0, "either the number of specialist infos is zero or less or the number of commerce types is zero or less");
	if (GETXML->SetToChildByTagName(pXML->GetXML(), "SpecialistCommercePercentChanges"))
	{
		if (pXML->SkipToNextVal())
		{
			iNumSibs = GETXML->GetNumChildren(pXML->GetXML());
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				if (0 < iNumSibs)
				{
					for (j = 0; j<iNumSibs; j++)
					{
						pXML->GetChildXmlValByName(szTextVal, "SpecialistType");
						iIndex = pXML->FindInInfoClass(szTextVal);

						if (iIndex > -1)
						{
							if (m_ppaiSpecialistCommercePercentChanges == NULL)
							{
								m_ppaiSpecialistCommercePercentChanges = new int*[GC.getNumSpecialistInfos()];

								for (int i = 0; i < GC.getNumSpecialistInfos(); i++)
								{
									m_ppaiSpecialistCommercePercentChanges[i] = NULL;
								}
							}
							// if we can set the current xml node to it's next sibling
							if (GETXML->SetToChildByTagName(pXML->GetXML(), "SpecialistCommercePercents"))
							{
								// call the function that sets the commerce change variable
								pXML->SetCommerce(&m_ppaiSpecialistCommercePercentChanges[iIndex]);
								GETXML->SetToParent(pXML->GetXML());
								m_bAnySpecialistCommerceChanges = true;
							}
						}

						if (!GETXML->NextSibling(pXML->GetXML()))
						{
							break;
						}
					}
				}

				GETXML->SetToParent(pXML->GetXML());
			}
		}

		GETXML->SetToParent(pXML->GetXML());
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	m_Properties.read(pXML);
	m_PropertiesAllCities.read(pXML, "PropertiesAllCities");
	m_PrereqMinProperties.read(pXML, "PrereqMinProperties");
	m_PrereqMaxProperties.read(pXML, "PrereqMaxProperties");
	m_PrereqPlayerMinProperties.read(pXML, "PrereqPlayerMinProperties");
	m_PrereqPlayerMaxProperties.read(pXML, "PrereqPlayerMaxProperties");

	m_PropertyManipulators.read(pXML);

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"NewCityFree"))
	{
		m_pExprNewCityFree = BoolExpr::read(pXML);
		GETXML->SetToParent(pXML->GetXML());
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"ConstructCondition"))
	{
		m_pExprConstructCondition = BoolExpr::read(pXML);
		GETXML->SetToParent(pXML->GetXML());
	}

	return true;
}
/************************************************************************************************/
/* Afforess	                  Start		 12/9/09                                                */
/*                                                                                              */
/*   ReadPass3 added to search for information that has not been loaded                         */
/************************************************************************************************/
bool CvBuildingInfo::readPass3()
{
	m_pbPrereqOrCivics = new bool[GC.getNumCivicInfos()];
	for (int iI = 0; iI < GC.getNumCivicInfos(); iI++)
	{
		m_pbPrereqOrCivics[iI] = false;
	}
	if (!m_abPrereqOrCivicsforPass3.empty() && !m_aszPrereqOrCivicsforPass3.empty())
	{
		int iNumLoad = m_abPrereqOrCivicsforPass3.size();
		for(int iI = 0; iI < iNumLoad; iI++)
		{
			//FAssertMsg(GC.getInfoTypeForString(m_aszPrereqOrCivicsforPass3[iI]) >= 0, "Warning, about to leak memory in CvBuildingInfo::readPass3");
			int iTempIndex = GC.getInfoTypeForString(m_aszPrereqOrCivicsforPass3[iI]);
			if (iTempIndex >= 0 && iTempIndex < GC.getNumCivicInfos())
				m_pbPrereqOrCivics[iTempIndex] = m_abPrereqOrCivicsforPass3[iI];
		}
		m_aszPrereqOrCivicsforPass3.clear();
		m_abPrereqOrCivicsforPass3.clear();
	}
	m_pbPrereqAndCivics = new bool[GC.getNumCivicInfos()];
    for (int iI = 0; iI < GC.getNumCivicInfos(); iI++)
	{
        m_pbPrereqAndCivics[iI] = false;
	}
	if (!m_abPrereqAndCivicsforPass3.empty() && !m_aszPrereqAndCivicsforPass3.empty())
	{
		int iNumLoad = m_abPrereqAndCivicsforPass3.size();
		for(int iI = 0; iI < iNumLoad; iI++)
		{
			//FAssertMsg(GC.getInfoTypeForString(m_aszPrereqAndCivicsforPass3[iI]) >= 0, "Warning, about to leak memory in CvBuildingInfo::readPass3");
			int iTempIndex = GC.getInfoTypeForString(m_aszPrereqAndCivicsforPass3[iI]);
			if (iTempIndex >= 0 && iTempIndex < GC.getNumCivicInfos())
				m_pbPrereqAndCivics[iTempIndex] = m_abPrereqAndCivicsforPass3[iI];
		}
		m_aszPrereqAndCivicsforPass3.clear();
		m_abPrereqAndCivicsforPass3.clear();
	}

	return true;
}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
/************************************************************************************************/
/* XMLCOPY                                 10/09/07                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
void CvBuildingInfo::copyNonDefaults(CvBuildingInfo* pClassInfo, CvXMLLoadUtility* pXML)
{

	bool bDefault = false;
	int iDefault = 0;
	int iTextDefault = -1;
	float fDefault = 0.0f;
	CvString cDefault = CvString::format("").GetCString();
	CvWString wDefault = CvWString::format(L"").GetCString();

	if ( getArtDefineTag() == cDefault ) // "ArtDefineTag"
	{
		setArtDefineTag(pClassInfo->getArtDefineTag());
	}
	if ( getMovieDefineTag() == cDefault ) // "ArtDefineTag"
	{
		setMovieDefineTag(pClassInfo->getMovieDefineTag());
	}

	CvHotkeyInfo::copyNonDefaults(pClassInfo, pXML);

	if (getBuildingClassType() == iTextDefault) m_iBuildingClassType = pClassInfo->getBuildingClassType();
	if (getSpecialBuildingType() == iTextDefault) m_iSpecialBuildingType = pClassInfo->getSpecialBuildingType();
	if (getAdvisorType() == iTextDefault) m_iAdvisorType = pClassInfo->getAdvisorType();

/********************************************************************************/
/**		REVDCM									2/16/10				phungus420	*/
/**																				*/
/**		CanConstruct															*/
/********************************************************************************/
	if (getPrereqGameOption() == iTextDefault) m_iPrereqGameOption = pClassInfo->getPrereqGameOption();
	if (getNotGameOption() == iTextDefault) m_iNotGameOption = pClassInfo->getNotGameOption();
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

	if (getHolyCity() == iTextDefault) m_iHolyCity = pClassInfo->getHolyCity();
	if (getReligionType() == iTextDefault) m_iReligionType = pClassInfo->getReligionType();
	if (getStateReligion() == iTextDefault) m_iStateReligion = pClassInfo->getStateReligion();
	if (getPrereqReligion() == iTextDefault) m_iPrereqReligion = pClassInfo->getPrereqReligion();
	if (getPrereqCorporation() == iTextDefault) m_iPrereqCorporation = pClassInfo->getPrereqCorporation();
	if (getFoundsCorporation() == iTextDefault) m_iFoundsCorporation = pClassInfo->getFoundsCorporation();
	if (getGlobalReligionCommerce() == iTextDefault) m_iGlobalReligionCommerce = pClassInfo->getGlobalReligionCommerce();
	if (getGlobalCorporationCommerce() == iTextDefault) m_iGlobalCorporationCommerce = pClassInfo->getGlobalCorporationCommerce();
	if (getVictoryPrereq() == iTextDefault) m_iVictoryPrereq = pClassInfo->getVictoryPrereq();
	if (getFreeStartEra() == iTextDefault) m_iFreeStartEra = pClassInfo->getFreeStartEra();
	if (getMaxStartEra() == iTextDefault) m_iMaxStartEra = pClassInfo->getMaxStartEra();
	if (getObsoleteTech() == iTextDefault) m_iObsoleteTech = pClassInfo->getObsoleteTech();
	if (getPrereqAndTech() == iTextDefault) m_iPrereqAndTech = pClassInfo->getPrereqAndTech();
	if (getPrereqVicinityBonus() == iTextDefault) m_iPrereqVicinityBonus = pClassInfo->getPrereqVicinityBonus();

	for ( int j = 0; j < GC.getNUM_BUILDING_AND_TECH_PREREQS(); j++)
	{
		if (getPrereqAndTechs(j) == iTextDefault && pClassInfo->getPrereqAndTechs(j) != iTextDefault)
		{
			if ( NULL == m_piPrereqAndTechs )
			{
				CvXMLLoadUtility::InitList(&m_piPrereqAndTechs,GC.getNUM_BUILDING_AND_TECH_PREREQS(),iTextDefault);
			}
			m_piPrereqAndTechs[j] = pClassInfo->getPrereqAndTechs(j);
		}
	}

	if (getPrereqAndBonus() == NO_BONUS) m_iPrereqAndBonus = pClassInfo->getPrereqAndBonus();
	for ( int j = 0; j < GC.getNUM_BUILDING_PREREQ_OR_BONUSES(); j++)
	{
		if (getPrereqOrBonuses(j) == iTextDefault && pClassInfo->getPrereqOrBonuses(j) != iTextDefault)
		{
			if ( NULL == m_piPrereqOrBonuses )
			{
				CvXMLLoadUtility::InitList(&m_piPrereqOrBonuses,GC.getNUM_BUILDING_PREREQ_OR_BONUSES(),iTextDefault);
			}
			m_piPrereqOrBonuses[j] = pClassInfo->getPrereqOrBonuses(j);
		}
	}

	for ( int j = 0; j < GC.getNumTraitInfos(); j++)
	{
		if (getProductionTraits(j) == iDefault && pClassInfo->getProductionTraits(j) != iDefault)
		{
			if ( NULL == m_piProductionTraits )
			{
				CvXMLLoadUtility::InitList(&m_piProductionTraits,GC.getNumTraitInfos(),iDefault);
			}
			m_piProductionTraits[j] = pClassInfo->getProductionTraits(j);
		}
	}
	for ( int j = 0; j < GC.getNumTraitInfos(); j++)
	{
		if (getHappinessTraits(j) == iDefault && pClassInfo->getHappinessTraits(j) != iDefault)
		{
			if ( NULL == m_piHappinessTraits )
			{
				CvXMLLoadUtility::InitList(&m_piHappinessTraits,GC.getNumTraitInfos(),iDefault);
			}
			m_piHappinessTraits[j] = pClassInfo->getHappinessTraits(j);
		}
	}
	if (getNoBonus() == iTextDefault) m_iNoBonus = pClassInfo->getNoBonus();
	if (getPowerBonus() == iTextDefault) m_iPowerBonus = pClassInfo->getPowerBonus();
	if (getFreeBonus() == iTextDefault) m_iFreeBonus = pClassInfo->getFreeBonus();
	if (getNumFreeBonuses() == iDefault) m_iNumFreeBonuses = pClassInfo->getNumFreeBonuses();

	if (m_aExtraFreeBonuses.empty())
	{
		int iNum = pClassInfo->getNumExtraFreeBonuses();
		for (int i = 0; i < iNum; i++)
		{
			m_aExtraFreeBonuses.push_back(std::pair<BonusTypes,int>(pClassInfo->getExtraFreeBonus(i),pClassInfo->getExtraFreeBonusNum(i)));
		}
	}

    // DarkLunaPhantom - FreeBuilding accepts lists.
    if (getNumFreeBuildingClass() < 1)
	{
		for (int i = 0; i < pClassInfo->getNumFreeBuildingClass(); ++i)
		{
			int iFreeBuildingClass = pClassInfo->getFreeBuildingClass(i);
			if (iFreeBuildingClass != NO_BUILDINGCLASS)
			{
				m_aiFreeBuildingClass.push_back(iFreeBuildingClass);
			}
		}
	}
    
    // DarkLunaPhantom - Extra FreeBuilding entries.
    if (getNumExtraFreeBuildingClass() < 1)
	{
		for (int i = 0; i < pClassInfo->getNumExtraFreeBuildingClass(); ++i)
		{
			int iExtraFreeBuildingClass = pClassInfo->getExtraFreeBuildingClass(i);
			if (iExtraFreeBuildingClass != NO_BUILDINGCLASS)
			{
				m_aiExtraFreeBuildingClass.push_back(iExtraFreeBuildingClass);
                m_abExtraFreeBuildingConnected.push_back(pClassInfo->getExtraFreeBuildingConnected(i));
                m_abExtraFreeBuildingContinuous.push_back(pClassInfo->getExtraFreeBuildingContinuous(i));
			}
		}
	}
    
	if (getFreeAreaBuildingClass() == iTextDefault) m_iFreeAreaBuildingClass = pClassInfo->getFreeAreaBuildingClass();
	if (getFreeTradeRegionBuildingClass() == iTextDefault) m_iFreeTradeRegionBuildingClass = pClassInfo->getFreeTradeRegionBuildingClass();
	if (getFreePromotion() == iTextDefault) m_iFreePromotion = pClassInfo->getFreePromotion();
	if (getCivicOption() == iTextDefault) m_iCivicOption = pClassInfo->getCivicOption();
	if (getGreatPeopleUnitClass() == iTextDefault) m_iGreatPeopleUnitClass = pClassInfo->getGreatPeopleUnitClass();
	if (getVoteSourceType() == iTextDefault) m_iVoteSourceType = pClassInfo->getVoteSourceType();

	if (getGreatPeopleRateChange() == iDefault) m_iGreatPeopleRateChange = pClassInfo->getGreatPeopleRateChange();
	if (isTeamShare() == bDefault) m_bTeamShare = pClassInfo->isTeamShare();
	if (isAutoBuild() == bDefault) m_bAutoBuild = pClassInfo->isAutoBuild();
	if (isWater() == bDefault) m_bWater = pClassInfo->isWater();
	if (isRiver() == bDefault) m_bRiver = pClassInfo->isRiver();
	if (isPower() == bDefault) m_bPower = pClassInfo->isPower();
	if (isNoPower() == bDefault) m_bNoPower = pClassInfo->isNoPower();
	if (isDirtyPower() == bDefault) m_bDirtyPower = pClassInfo->isDirtyPower();
	if (isAreaCleanPower() == bDefault) m_bAreaCleanPower = pClassInfo->isAreaCleanPower();
	if (isNoHolyCity() == bDefault) m_bNoHolyCity = pClassInfo->isNoHolyCity();
	if (isAreaBorderObstacle() == bDefault) m_bAreaBorderObstacle = pClassInfo->isAreaBorderObstacle();
	if (isForceTeamVoteEligible() == bDefault) m_bForceTeamVoteEligible = pClassInfo->isForceTeamVoteEligible();
	if (isCapital() == bDefault) m_bCapital = pClassInfo->isCapital();
	if (isGovernmentCenter() == bDefault) m_bGovernmentCenter = pClassInfo->isGovernmentCenter();
	if (isGoldenAge () == bDefault) m_bGoldenAge = pClassInfo->isGoldenAge ();
	if (isAllowsNukes() == bDefault) m_bAllowsNukes = pClassInfo->isAllowsNukes();
	if (isMapCentering() == bDefault) m_bMapCentering = pClassInfo->isMapCentering();
	if (isNoUnhappiness() == bDefault) m_bNoUnhappiness = pClassInfo->isNoUnhappiness();
	if (isNoUnhealthyPopulation() == bDefault) m_bNoUnhealthyPopulation = pClassInfo->isNoUnhealthyPopulation();
	if (isBuildingOnlyHealthy() == bDefault) m_bBuildingOnlyHealthy = pClassInfo->isBuildingOnlyHealthy();
	if (isNeverCapture() == bDefault) m_bNeverCapture = pClassInfo->isNeverCapture();
	if (isNukeImmune() == bDefault) m_bNukeImmune = pClassInfo->isNukeImmune();
	if (isPrereqReligion() == bDefault) m_bPrereqReligion = pClassInfo->isPrereqReligion();
	if (isFreeFromStateReligion() == bDefault) m_bFreeFromStateReligion = pClassInfo->isFreeFromStateReligion();
	if (isCenterInCity() == bDefault) m_bCenterInCity = pClassInfo->isCenterInCity();
	if (isStateReligion() == bDefault) m_bStateReligion = pClassInfo->isStateReligion();

	if (getAIWeight() == iDefault) m_iAIWeight = pClassInfo->getAIWeight();
	if (getProductionCost() == iDefault) m_iProductionCost = pClassInfo->getProductionCost();
	if (getHurryCostModifier() == iDefault) m_iHurryCostModifier = pClassInfo->getHurryCostModifier();
	if (getHurryAngerModifier() == iDefault) m_iHurryAngerModifier = pClassInfo->getHurryAngerModifier();
	if (getAdvancedStartCost() == iDefault) m_iAdvancedStartCost = pClassInfo->getAdvancedStartCost();
	if (getAdvancedStartCostIncrease() == iDefault) m_iAdvancedStartCostIncrease = pClassInfo->getAdvancedStartCostIncrease();
	if (getMinAreaSize() == iDefault) m_iMinAreaSize = pClassInfo->getMinAreaSize();
	if (getConquestProbability() == iDefault) m_iConquestProbability = pClassInfo->getConquestProbability();
	if (getNumCitiesPrereq() == iDefault) m_iNumCitiesPrereq = pClassInfo->getNumCitiesPrereq();
	if (getNumTeamsPrereq() == iDefault) m_iNumTeamsPrereq = pClassInfo->getNumTeamsPrereq();
	if (getUnitLevelPrereq() == iDefault) m_iUnitLevelPrereq = pClassInfo->getUnitLevelPrereq();
	if (getMinLatitude() == iDefault) m_iMinLatitude = pClassInfo->getMinLatitude();
	if (getMaxLatitude() == 90) m_iMaxLatitude = pClassInfo->getMaxLatitude();
	if (getGreatPeopleRateModifier() == iDefault) m_iGreatPeopleRateModifier = pClassInfo->getGreatPeopleRateModifier();
	if (getGreatGeneralRateModifier() == iDefault) m_iGreatGeneralRateModifier = pClassInfo->getGreatGeneralRateModifier();
	if (getDomesticGreatGeneralRateModifier() == iDefault) m_iDomesticGreatGeneralRateModifier = pClassInfo->getDomesticGreatGeneralRateModifier();
	if (getGlobalGreatPeopleRateModifier() == iDefault) m_iGlobalGreatPeopleRateModifier = pClassInfo->getGlobalGreatPeopleRateModifier();
	if (getAnarchyModifier() == iDefault) m_iAnarchyModifier = pClassInfo->getAnarchyModifier();
	if (getGoldenAgeModifier() == iDefault) m_iGoldenAgeModifier = pClassInfo->getGoldenAgeModifier();
	if (getGlobalHurryModifier() == iDefault) m_iGlobalHurryModifier = pClassInfo->getGlobalHurryModifier();
	if (getFreeExperience() == iDefault) m_iFreeExperience = pClassInfo->getFreeExperience();
	if (getGlobalFreeExperience() == iDefault) m_iGlobalFreeExperience = pClassInfo->getGlobalFreeExperience();
	if (getFoodKept() == iDefault) m_iFoodKept = pClassInfo->getFoodKept();
	//doto culture safe zone
	if (getNoForeignCityCultureGrowth() == iDefault) m_iNoForeignCityCultureGrowth = pClassInfo->getNoForeignCityCultureGrowth();
	//doto culture safe zone
	if (getAirlift() == iDefault) m_iAirlift = pClassInfo->getAirlift();
	if (getAirModifier() == iDefault) m_iAirModifier = pClassInfo->getAirModifier();
	if (getAirUnitCapacity() == iDefault) m_iAirUnitCapacity = pClassInfo->getAirUnitCapacity();
	if (getWonderCapacityIncrement() == iDefault) m_iWonderCapacityIncrement = pClassInfo->getWonderCapacityIncrement();
	if (getNukeModifier() == iDefault) m_iNukeModifier = pClassInfo->getNukeModifier();
	if (getNukeExplosionRand() == iDefault) m_iNukeExplosionRand = pClassInfo->getNukeExplosionRand();
	if (getFreeSpecialist() == iDefault) m_iFreeSpecialist = pClassInfo->getFreeSpecialist();
	if (getAreaFreeSpecialist() == iDefault) m_iAreaFreeSpecialist = pClassInfo->getAreaFreeSpecialist();
	if (getGlobalFreeSpecialist() == iDefault) m_iGlobalFreeSpecialist = pClassInfo->getGlobalFreeSpecialist();
	if (getMaintenanceModifier() == iDefault) m_iMaintenanceModifier = pClassInfo->getMaintenanceModifier();
	if (getWarWearinessModifier() == iDefault) m_iWarWearinessModifier = pClassInfo->getWarWearinessModifier();
	if (getGlobalWarWearinessModifier() == iDefault) m_iGlobalWarWearinessModifier = pClassInfo->getGlobalWarWearinessModifier();
	if (getEnemyWarWearinessModifier() == iDefault) m_iEnemyWarWearinessModifier = pClassInfo->getEnemyWarWearinessModifier();
	if (getHealRateChange() == iDefault) m_iHealRateChange = pClassInfo->getHealRateChange();
	if (getHealth() == iDefault) m_iHealth = pClassInfo->getHealth();
	if (getAreaHealth() == iDefault) m_iAreaHealth = pClassInfo->getAreaHealth();
	if (getGlobalHealth() == iDefault) m_iGlobalHealth = pClassInfo->getGlobalHealth();
	if (getHappiness() == iDefault) m_iHappiness = pClassInfo->getHappiness();
	if (getAreaHappiness() == iDefault) m_iAreaHappiness = pClassInfo->getAreaHappiness();
	if (getGlobalHappiness() == iDefault) m_iGlobalHappiness = pClassInfo->getGlobalHappiness();
	if (getStateReligionHappiness() == iDefault) m_iStateReligionHappiness = pClassInfo->getStateReligionHappiness();
	if (getStateReligionHealth() == iDefault) m_iStateReligionHealth = pClassInfo->getStateReligionHealth();
	if (getWorkerSpeedModifier() == iDefault) m_iWorkerSpeedModifier = pClassInfo->getWorkerSpeedModifier();
	if (getMilitaryProductionModifier() == iDefault) m_iMilitaryProductionModifier = pClassInfo->getMilitaryProductionModifier();
	if (getSpaceProductionModifier() == iDefault) m_iSpaceProductionModifier = pClassInfo->getSpaceProductionModifier();
	if (getGlobalSpaceProductionModifier() == iDefault) m_iGlobalSpaceProductionModifier = pClassInfo->getGlobalSpaceProductionModifier();
	if (getTradeRoutes() == iDefault) m_iTradeRoutes = pClassInfo->getTradeRoutes();
	if (getInterestPercent() == iDefault) m_iInterestPercent = pClassInfo->getInterestPercent();
	if (getInterestMax() == iDefault) m_iInterestMax = pClassInfo->getInterestMax();
	if (getCoastalTradeRoutes() == iDefault) m_iCoastalTradeRoutes = pClassInfo->getCoastalTradeRoutes();
	if (getGlobalTradeRoutes() == iDefault) m_iGlobalTradeRoutes = pClassInfo->getGlobalTradeRoutes();
	if (getTradeRouteModifier() == iDefault) m_iTradeRouteModifier = pClassInfo->getTradeRouteModifier();
	if (getForeignTradeRouteModifier() == iDefault) m_iForeignTradeRouteModifier = pClassInfo->getForeignTradeRouteModifier();
	if (getGlobalPopulationChange() == iDefault) m_iGlobalPopulationChange = pClassInfo->getGlobalPopulationChange();
	if (getFreeTechs() == iDefault) m_iFreeTechs = pClassInfo->getFreeTechs();
	if (getFreeSpecialTech() == NO_TECH) m_eFreeSpecialTech = pClassInfo->getFreeSpecialTech();
	if (getDefenseModifier() == iDefault) m_iDefenseModifier = pClassInfo->getDefenseModifier();
	if (getBombardDefenseModifier() == iDefault) m_iBombardDefenseModifier = pClassInfo->getBombardDefenseModifier();
	if (getAllCityDefenseModifier() == iDefault) m_iAllCityDefenseModifier = pClassInfo->getAllCityDefenseModifier();
	if (getEspionageDefenseModifier() == iDefault) m_iEspionageDefenseModifier = pClassInfo->getEspionageDefenseModifier();
	if (getAssetValue() == iDefault) m_iAssetValue = pClassInfo->getAssetValue();
	if (getPowerValue() == iDefault) m_iPowerValue = pClassInfo->getPowerValue();
/********************************************************************************/
/**		REVDCM									4/09/10				Afforess	*/
/**																				*/
/**		Building Effects														*/
/********************************************************************************/
	if (getUnitUpgradePriceModifier() == iDefault) m_iUnitUpgradePriceModifier = pClassInfo->getUnitUpgradePriceModifier();
	if (getRevIdxLocal() == iDefault) m_iRevIdxLocal = pClassInfo->getRevIdxLocal();
	if (getRevIdxNational() == iDefault) m_iRevIdxNational = pClassInfo->getRevIdxNational();
	if (getRevIdxDistanceModifier() == iDefault) m_iRevIdxDistanceModifier = pClassInfo->getRevIdxDistanceModifier();
/********************************************************************************/
/**		REVDCM									END								*/
/********************************************************************************/

	if (getVisibilityPriority() == fDefault) m_fVisibilityPriority = pClassInfo->getVisibilityPriority();

	setNotShowInCity();

	for ( int j = 0; j < NUM_YIELD_TYPES; j++)
	{
		if ( getSeaPlotYieldChange(j) == iDefault && pClassInfo->getSeaPlotYieldChange(j) != iDefault)
		{
			if ( NULL == m_piSeaPlotYieldChange )
			{
				CvXMLLoadUtility::InitList(&m_piSeaPlotYieldChange,NUM_YIELD_TYPES,iDefault);
			}
			m_piSeaPlotYieldChange[j] = pClassInfo->getSeaPlotYieldChange(j);
		}
	}
	for ( int j = 0; j < NUM_YIELD_TYPES; j++)
	{
		if ( getRiverPlotYieldChange(j) == iDefault && pClassInfo->getRiverPlotYieldChange(j) != iDefault)
		{
			if ( NULL == m_piRiverPlotYieldChange )
			{
				CvXMLLoadUtility::InitList(&m_piRiverPlotYieldChange,NUM_YIELD_TYPES,iDefault);
			}
			m_piRiverPlotYieldChange[j] = pClassInfo->getRiverPlotYieldChange(j);
		}
	}
	for ( int j = 0; j < NUM_YIELD_TYPES; j++)
	{
		if ( getGlobalSeaPlotYieldChange(j) == iDefault && pClassInfo->getGlobalSeaPlotYieldChange(j) != iDefault)
		{
			if ( NULL == m_piGlobalSeaPlotYieldChange )
			{
				CvXMLLoadUtility::InitList(&m_piGlobalSeaPlotYieldChange,NUM_YIELD_TYPES,iDefault);
			}
			m_piGlobalSeaPlotYieldChange[j] = pClassInfo->getGlobalSeaPlotYieldChange(j);
		}
	}
	for ( int j = 0; j < NUM_YIELD_TYPES; j++)
	{
		if ( getYieldChange(j) == iDefault && pClassInfo->getYieldChange(j) != iDefault)
		{
			if ( NULL == m_piYieldChange )
			{
				CvXMLLoadUtility::InitList(&m_piYieldChange,NUM_YIELD_TYPES,iDefault);
			}
			m_piYieldChange[j] = pClassInfo->getYieldChange(j);
		}
	}
	for ( int j = 0; j < NUM_YIELD_TYPES; j++)
	{
		if ( getYieldModifier(j) == iDefault && pClassInfo->getYieldModifier(j) != iDefault)
		{
			if ( NULL == m_piYieldModifier )
			{
				CvXMLLoadUtility::InitList(&m_piYieldModifier,NUM_YIELD_TYPES,iDefault);
			}
			m_piYieldModifier[j] = pClassInfo->getYieldModifier(j);
		}
	}
	for ( int j = 0; j < NUM_YIELD_TYPES; j++)
	{
		if ( getPowerYieldModifier(j) == iDefault && pClassInfo->getPowerYieldModifier(j) != iDefault)
		{
			if ( NULL == m_piPowerYieldModifier )
			{
				CvXMLLoadUtility::InitList(&m_piPowerYieldModifier,NUM_YIELD_TYPES,iDefault);
			}
			m_piPowerYieldModifier[j] = pClassInfo->getPowerYieldModifier(j);
		}
	}
	for ( int j = 0; j < NUM_YIELD_TYPES; j++)
	{
		if ( getAreaYieldModifier(j) == iDefault && pClassInfo->getAreaYieldModifier(j) != iDefault)
		{
			if ( NULL == m_piAreaYieldModifier )
			{
				CvXMLLoadUtility::InitList(&m_piAreaYieldModifier,NUM_YIELD_TYPES,iDefault);
			}
			m_piAreaYieldModifier[j] = pClassInfo->getAreaYieldModifier(j);
		}
	}
	for ( int j = 0; j < NUM_YIELD_TYPES; j++)
	{
		if ( getGlobalYieldModifier(j) == iDefault && pClassInfo->getGlobalYieldModifier(j) != iDefault)
		{
			if ( NULL == m_piGlobalYieldModifier )
			{
				CvXMLLoadUtility::InitList(&m_piGlobalYieldModifier,NUM_YIELD_TYPES,iDefault);
			}
			m_piGlobalYieldModifier[j] = pClassInfo->getGlobalYieldModifier(j);
		}
	}

	for ( int j = 0; j < NUM_COMMERCE_TYPES; j++)
	{
		if ( getCommerceChange(j) == iDefault && pClassInfo->getCommerceChange(j) != iDefault)
		{
			if ( NULL == m_piCommerceChange )
			{
				CvXMLLoadUtility::InitList(&m_piCommerceChange,NUM_COMMERCE_TYPES,iDefault);
			}
			m_piCommerceChange[j] = pClassInfo->getCommerceChange(j);
		}
	}
	for ( int j = 0; j < NUM_COMMERCE_TYPES; j++)
	{
		if ( getObsoleteSafeCommerceChange(j) == iDefault && pClassInfo->getObsoleteSafeCommerceChange(j) != iDefault)
		{
			if ( NULL == m_piObsoleteSafeCommerceChange )
			{
				CvXMLLoadUtility::InitList(&m_piObsoleteSafeCommerceChange,NUM_COMMERCE_TYPES,iDefault);
			}
			m_piObsoleteSafeCommerceChange[j] = pClassInfo->getObsoleteSafeCommerceChange(j);
		}
	}
	for ( int j = 0; j < NUM_COMMERCE_TYPES; j++)
	{
		if ( getCommerceChangeDoubleTime(j) == iDefault && pClassInfo->getCommerceChangeDoubleTime(j) != iDefault)
		{
			if ( NULL == m_piCommerceChangeDoubleTime )
			{
				CvXMLLoadUtility::InitList(&m_piCommerceChangeDoubleTime,NUM_COMMERCE_TYPES,iDefault);
			}
			m_piCommerceChangeDoubleTime[j] = pClassInfo->getCommerceChangeDoubleTime(j);
		}
	}
	for ( int j = 0; j < NUM_COMMERCE_TYPES; j++)
	{
		if ( getCommerceModifier(j) == iDefault && pClassInfo->getCommerceModifier(j) != iDefault)
		{
			if ( NULL == m_piCommerceModifier )
			{
				CvXMLLoadUtility::InitList(&m_piCommerceModifier,NUM_COMMERCE_TYPES,iDefault);
			}
			m_piCommerceModifier[j] = pClassInfo->getCommerceModifier(j);
		}
	}
	for ( int j = 0; j < NUM_COMMERCE_TYPES; j++)
	{
		if ( getGlobalCommerceModifier(j) == iDefault && pClassInfo->getGlobalCommerceModifier(j) != iDefault)
		{
			if ( NULL == m_piGlobalCommerceModifier )
			{
				CvXMLLoadUtility::InitList(&m_piGlobalCommerceModifier,NUM_COMMERCE_TYPES,iDefault);
			}
			m_piGlobalCommerceModifier[j] = pClassInfo->getGlobalCommerceModifier(j);
		}
	}
	for ( int j = 0; j < NUM_COMMERCE_TYPES; j++)
	{
		if ( getSpecialistExtraCommerce(j) == iDefault && pClassInfo->getSpecialistExtraCommerce(j) != iDefault)
		{
			if ( NULL == m_piSpecialistExtraCommerce )
			{
				CvXMLLoadUtility::InitList(&m_piSpecialistExtraCommerce,NUM_COMMERCE_TYPES,iDefault);
			}
			m_piSpecialistExtraCommerce[j] = pClassInfo->getSpecialistExtraCommerce(j);
		}
	}
	for ( int j = 0; j < NUM_COMMERCE_TYPES; j++)
	{
		if ( getStateReligionCommerce(j) == iDefault && pClassInfo->getStateReligionCommerce(j) != iDefault)
		{
			if ( NULL == m_piStateReligionCommerce )
			{
				CvXMLLoadUtility::InitList(&m_piStateReligionCommerce,NUM_COMMERCE_TYPES,iDefault);
			}
			m_piStateReligionCommerce[j] = pClassInfo->getStateReligionCommerce(j);
		}
	}
	for ( int j = 0; j < NUM_COMMERCE_TYPES; j++)
	{
		if ( getCommerceHappiness(j) == iDefault && pClassInfo->getCommerceHappiness(j) != iDefault)
		{
			if ( NULL == m_piCommerceHappiness )
			{
				CvXMLLoadUtility::InitList(&m_piCommerceHappiness,NUM_COMMERCE_TYPES,iDefault);
			}
			m_piCommerceHappiness[j] = pClassInfo->getCommerceHappiness(j);
		}
	}

	for ( int j = 0; j < GC.getNumReligionInfos(); j++)
	{
		if ( getReligionChange(j) == iDefault && pClassInfo->getReligionChange(j) != iDefault)
		{
			if ( NULL == m_piReligionChange )
			{
				CvXMLLoadUtility::InitList(&m_piReligionChange,GC.getNumReligionInfos(),iDefault);
			}
			m_piReligionChange[j] = pClassInfo->getReligionChange(j);
		}
	}

	for ( int j = 0; j < GC.getNumSpecialistInfos(); j++)
	{
		if ( getSpecialistCount(j) == iDefault && pClassInfo->getSpecialistCount(j) != iDefault)
		{
			if ( NULL == m_piSpecialistCount )
			{
				CvXMLLoadUtility::InitList(&m_piSpecialistCount,GC.getNumSpecialistInfos(),iDefault);
			}
			m_piSpecialistCount[j] = pClassInfo->getSpecialistCount(j);
		}
	}
	for ( int j = 0; j < GC.getNumSpecialistInfos(); j++)
	{
		if ( getFreeSpecialistCount(j) == iDefault && pClassInfo->getFreeSpecialistCount(j) != iDefault)
		{
			if ( NULL == m_piFreeSpecialistCount )
			{
				CvXMLLoadUtility::InitList(&m_piFreeSpecialistCount,GC.getNumSpecialistInfos(),iDefault);
			}
			m_piFreeSpecialistCount[j] = pClassInfo->getFreeSpecialistCount(j);
		}
	}

	for ( int j = 0; j < NUM_COMMERCE_TYPES; j++)
	{
		if ( isCommerceFlexible(j) == bDefault && pClassInfo->isCommerceFlexible(j) != bDefault)
		{
			if ( NULL == m_pbCommerceFlexible )
			{
				CvXMLLoadUtility::InitList(&m_pbCommerceFlexible,NUM_COMMERCE_TYPES,bDefault);
			}
			m_pbCommerceFlexible[j] = pClassInfo->isCommerceFlexible(j);
		}
	}
	for ( int j = 0; j < NUM_COMMERCE_TYPES; j++)
	{
		if ( isCommerceChangeOriginalOwner(j) == bDefault && pClassInfo->isCommerceChangeOriginalOwner(j) != bDefault)
		{
			if ( NULL == m_pbCommerceChangeOriginalOwner )
			{
				CvXMLLoadUtility::InitList(&m_pbCommerceChangeOriginalOwner,NUM_COMMERCE_TYPES,bDefault);
			}
			m_pbCommerceChangeOriginalOwner[j] = pClassInfo->isCommerceChangeOriginalOwner(j);
		}
	}

	if ( getConstructSound() == cDefault ) setConstructSound( pClassInfo->getConstructSound() );

	for ( int j = 0; j < GC.getNumBonusInfos(); j++)
	{
		if ( getBonusHealthChanges(j) == iDefault && pClassInfo->getBonusHealthChanges(j) != iDefault)
		{
			if ( NULL == m_piBonusHealthChanges )
			{
				CvXMLLoadUtility::InitList(&m_piBonusHealthChanges,GC.getNumBonusInfos(),iDefault);
			}
			m_piBonusHealthChanges[j] = pClassInfo->getBonusHealthChanges(j);
		}
		if ( getBonusHappinessChanges(j) == iDefault && pClassInfo->getBonusHappinessChanges(j) != iDefault)
		{
			if ( NULL == m_piBonusHappinessChanges )
			{
				CvXMLLoadUtility::InitList(&m_piBonusHappinessChanges,GC.getNumBonusInfos(),iDefault);
			}
			m_piBonusHappinessChanges[j] = pClassInfo->getBonusHappinessChanges(j);
		}
		if ( getBonusProductionModifier(j) == iDefault && pClassInfo->getBonusProductionModifier(j) != iDefault )
		{
			if ( NULL == m_piBonusProductionModifier )
			{
				CvXMLLoadUtility::InitList(&m_piBonusProductionModifier,GC.getNumBonusInfos(),iDefault);
			}
			m_piBonusProductionModifier[j] = pClassInfo->getBonusProductionModifier(j);
		}
	}
	for ( int j = 0; j < GC.getNumUnitCombatInfos(); j++)
	{
		if ( getUnitCombatFreeExperience(j) == iDefault && pClassInfo->getUnitCombatFreeExperience(j) != iDefault)
		{
			if ( NULL == m_piUnitCombatFreeExperience)
			{
				CvXMLLoadUtility::InitList(&m_piUnitCombatFreeExperience,GC.getNumUnitCombatInfos(),iDefault);
			}
			m_piUnitCombatFreeExperience[j] = pClassInfo->getUnitCombatFreeExperience(j);
		}
	}
	for ( int j = 0; j < NUM_DOMAIN_TYPES; j++)
	{
		if ( getDomainFreeExperience(j) == iDefault && pClassInfo->getDomainFreeExperience(j) != iDefault)
		{
			if ( NULL == m_piDomainFreeExperience )
			{
				CvXMLLoadUtility::InitList(&m_piDomainFreeExperience,NUM_DOMAIN_TYPES,iDefault);
			}
			m_piDomainFreeExperience[j] = pClassInfo->getDomainFreeExperience(j);
		}
		if ( getDomainProductionModifier(j) == iDefault && pClassInfo->getDomainProductionModifier(j) != iDefault)
		{
			if ( NULL == m_piDomainProductionModifier )
			{
				CvXMLLoadUtility::InitList(&m_piDomainProductionModifier,NUM_DOMAIN_TYPES,iDefault);
			}
			m_piDomainProductionModifier[j] = pClassInfo->getDomainProductionModifier(j);
		}
	}

	for ( int j = 0; j < GC.getNumBuildingClassInfos(); j++)
	{
		if ( getPrereqNumOfBuildingClass(j) == iDefault && pClassInfo->getPrereqNumOfBuildingClass(j) != iDefault)
		{
			if ( NULL == m_piPrereqNumOfBuildingClass )
			{
				CvXMLLoadUtility::InitList(&m_piPrereqNumOfBuildingClass,GC.getNumBuildingClassInfos(),iDefault);
			}
			m_piPrereqNumOfBuildingClass[j] = pClassInfo->getPrereqNumOfBuildingClass(j);
		}
		if ( isBuildingClassNeededInCity(j) == bDefault && pClassInfo->isBuildingClassNeededInCity(j) != bDefault)
		{
			if ( NULL == m_pbBuildingClassNeededInCity )
			{
				CvXMLLoadUtility::InitList(&m_pbBuildingClassNeededInCity,GC.getNumBuildingClassInfos(),bDefault);
			}
			m_pbBuildingClassNeededInCity[j] = pClassInfo->isBuildingClassNeededInCity(j);
		}
	}
	for ( int i = 0; i < GC.getNumSpecialistInfos(); i++)	// "Init2DIntList" verify method
	{
		for ( int j = 0; j < NUM_YIELD_TYPES; j++)
		{
			if ( getSpecialistYieldChange(i,j) == iDefault )
			{
				int iChange = pClassInfo->getSpecialistYieldChange(i, j);

				if ( iChange != iDefault )
				{
					if ( m_ppaiSpecialistYieldChange == NULL )
					{
						m_ppaiSpecialistYieldChange = new int*[GC.getNumSpecialistInfos()];
						for(int k = 0; k < GC.getNumSpecialistInfos(); k++)
						{
							m_ppaiSpecialistYieldChange[k] = NULL;
						}
					}
					if ( m_ppaiSpecialistYieldChange[i] == NULL )
					{
						m_ppaiSpecialistYieldChange[i] = new int[NUM_YIELD_TYPES];
					}

					m_ppaiSpecialistYieldChange[i][j] = iChange;
					m_bAnySpecialistYieldChanges = true;
				}
			}
		}
	}
	for ( int i = 0; i < GC.getNumBonusInfos(); i++)	// "Init2DIntList" verify method
	{
		for ( int j = 0; j < NUM_YIELD_TYPES; j++)
		{
			if ( getBonusYieldModifier(i,j) == iDefault )
			{
				int iChange = pClassInfo->getBonusYieldModifier(i, j);

				if ( iChange != iDefault )
				{
					if ( m_ppaiBonusYieldModifier == NULL )
					{
						m_ppaiBonusYieldModifier = new int*[GC.getNumBonusInfos()];
						for(int k = 0; k < GC.getNumBonusInfos(); k++)
						{
							m_ppaiBonusYieldModifier[k] = NULL;
						}
					}
					if ( m_ppaiBonusYieldModifier[i] == NULL )
					{
						m_ppaiBonusYieldModifier[i] = new int[NUM_YIELD_TYPES];
					}

					m_ppaiBonusYieldModifier[i][j] = iChange;
					m_bAnyBonusYieldModifiers = true;
				}
			}
		}
	}
	for ( int j = 0; j < GC.getNumFlavorTypes(); j++)
	{
		if ( getFlavorValue(j) == iDefault && pClassInfo->getFlavorValue(j) != iDefault)
		{
			if ( NULL == m_piFlavorValue )
			{
				CvXMLLoadUtility::InitList(&m_piFlavorValue,GC.getNumFlavorTypes(),iDefault);
			}
			m_piFlavorValue[j] = pClassInfo->getFlavorValue(j);
		}
	}
	for ( int j = 0; j < GC.getNumImprovementInfos(); j++)
	{
		if ( getImprovementFreeSpecialist(j) == iDefault && pClassInfo->getImprovementFreeSpecialist(j) != iDefault)
		{
			if ( NULL == m_piImprovementFreeSpecialist )
			{
				CvXMLLoadUtility::InitList(&m_piImprovementFreeSpecialist,GC.getNumImprovementInfos(),iDefault);
			}
			m_piImprovementFreeSpecialist[j] = pClassInfo->getImprovementFreeSpecialist(j);
		}
	}
	for ( int j = 0; j < GC.getNumBuildingClassInfos(); j++)
	{
		if ( getBuildingHappinessChanges(j) == iDefault && pClassInfo->getBuildingHappinessChanges(j) != iDefault)
		{
			if ( NULL == m_piBuildingHappinessChanges )
			{
				CvXMLLoadUtility::InitList(&m_piBuildingHappinessChanges,GC.getNumBuildingClassInfos(),iDefault);
			}
			m_piBuildingHappinessChanges[j] = pClassInfo->getBuildingHappinessChanges(j);
		}
	}

	bool bNoDuplicate = true;
	for ( int iBuildingClass = 0; iBuildingClass < GC.getNumBuildingClassInfos(); iBuildingClass++)
	{
		for (std::vector<BuildingCommerceChange>::const_iterator it = m_aGlobalBuildingCommerceChanges.begin(); it != m_aGlobalBuildingCommerceChanges.end(); ++it)
		{
			if ((*it).eBuildingClass == (BuildingClassTypes)iBuildingClass)
			{
				//obviously some modder already set this buildingclass to some value
				//we don't want to overwrite his settings with the older(assuming he added
				//his tag on purpose)
				bNoDuplicate = false;
				break;
			}
		}
		if ( bNoDuplicate )
		{
			for ( int iCommerce = 0; iCommerce < NUM_COMMERCE_TYPES; iCommerce++)
			{
				if ( pClassInfo->getGlobalBuildingCommerceChange(iBuildingClass, iCommerce) != 0)
				{
					BuildingCommerceChange kChange;
					kChange.eBuildingClass = (BuildingClassTypes)iBuildingClass;
					kChange.eCommerce = (CommerceTypes)iCommerce;
					kChange.iChange = pClassInfo->getGlobalBuildingCommerceChange(iBuildingClass, iCommerce);
					m_aGlobalBuildingCommerceChanges.push_back(kChange);
				}
			}
		}
	}
/************************************************************************************************/
/* Afforess	                  Start		 12/9/09                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (isApplyFreePromotionOnMove() == bDefault) m_bApplyFreePromotionOnMove = pClassInfo->isApplyFreePromotionOnMove();
	if (isNoEnemyPillagingIncome() == bDefault) m_bNoEnemyPillagingIncome = pClassInfo->isNoEnemyPillagingIncome();
	if (isProvidesFreshWater() == bDefault) m_bProvidesFreshWater = pClassInfo->isProvidesFreshWater();
	if (isForceAllTradeRoutes() == bDefault) m_bForceAllTradeRoutes = pClassInfo->isForceAllTradeRoutes();
	if (getPillageGoldModifier() == iDefault) m_iPillageGoldModifier = pClassInfo->getPillageGoldModifier();
	if (getWorldTradeRoutes() == iDefault) m_iWorldTradeRoutes = pClassInfo->getWorldTradeRoutes();
	if (isBuildOnlyOnPeaks() == bDefault) m_bBuildOnlyOnPeaks = pClassInfo->isBuildOnlyOnPeaks();
	if (isForceNoPrereqScaling() == bDefault) m_bForceNoPrereqScaling = pClassInfo->isForceNoPrereqScaling();
	if (isPrereqWar() == bDefault) m_bPrereqWar = pClassInfo->isPrereqWar();
	if (isRequiresActiveCivics() == bDefault) m_bRequiresActiveCivics = pClassInfo->isRequiresActiveCivics();
	if (isZoneOfControl() == bDefault) m_bZoneOfControl = pClassInfo->isZoneOfControl();
	if (isPrereqPower() == bDefault) m_bPrereqPower = pClassInfo->isPrereqPower();
	if (getGlobalPopulationgrowthratepercentage() == iDefault) m_iGlobalPopulationgrowthratepercentage = pClassInfo->getGlobalPopulationgrowthratepercentage();
	if (getPopulationgrowthratepercentage() == iDefault) m_iPopulationgrowthratepercentage = pClassInfo->getPopulationgrowthratepercentage();
	if (getFreePromotion_2() == iTextDefault) m_iFreePromotion_2 = pClassInfo->getFreePromotion_2();
	if (getFreePromotion_3() == iTextDefault) m_iFreePromotion_3 = pClassInfo->getFreePromotion_3();
	if (getLineOfSight() == iDefault) m_iLineOfSight = pClassInfo->getLineOfSight();
	if (getInflationModifier() == iDefault) m_iInflationModifier = pClassInfo->getInflationModifier();
	if (!m_bAnyTechCommerceChanges) m_bAnyTechCommerceChanges = pClassInfo->m_bAnyTechCommerceChanges;
	if (getInvasionChance() == iDefault) m_iInvasionChance = pClassInfo->getInvasionChance();
	if (getAdjacentDamagePercent() == iDefault) m_iAdjacentDamagePercent = pClassInfo->getAdjacentDamagePercent();
	if (getProductionContinueBuildingClass() == NO_BUILDINGCLASS) m_iProductionContinueBuildingClass = pClassInfo->getProductionContinueBuildingClass();
	if (getPrereqPopulation() == iDefault) m_iPrereqPopulation = pClassInfo->getPrereqPopulation();
	if (getPrereqCultureLevel() == NO_CULTURELEVEL) m_iPrereqCultureLevel = pClassInfo->getPrereqCultureLevel();
	if (getWorkableRadius() == iDefault) m_iWorkableRadius = pClassInfo->getWorkableRadius();
	if (getPrereqAnyoneBuildingClass() == NO_BUILDINGCLASS) m_iPrereqAnyoneBuildingClass = pClassInfo->getPrereqAnyoneBuildingClass();
	if (getOccupationTimeModifier() == iDefault) m_iOccupationTimeModifier = pClassInfo->getOccupationTimeModifier();
	if (getNoEntryDefenseLevel() == iDefault) m_iNoEntryDefenseLevel = pClassInfo->getNoEntryDefenseLevel();
	if (getNumUnitFullHeal() == iDefault) m_iNumUnitFullHeal = pClassInfo->getNumUnitFullHeal();
	if (getNumPopulationEmployed() == iDefault) m_iNumPopulationEmployed = pClassInfo->getNumPopulationEmployed();
	if (getHealthPercentPerPopulation() == iDefault) m_iHealthPercentPerPopulation = pClassInfo->getHealthPercentPerPopulation();
	if (getHappinessPercentPerPopulation() == iDefault) m_iHappinessPercentPerPopulation = pClassInfo->getHappinessPercentPerPopulation();

	if (getExtraCapitalCommerce() == iDefault) m_iCapitalExtraCommerce = pClassInfo->getExtraCapitalCommerce();
	if (getExtraForeignCapitalCommerce() == iDefault) m_iForeignCapitalExtraCommerce = pClassInfo->getExtraForeignCapitalCommerce();

	if (getDomesticConnectedCommerce() == iDefault) m_iDomesticConnectedCommerce = pClassInfo->getDomesticConnectedCommerce();
	if (getCitiesRequiredPerDomesticConnectedCommerce() == 1) m_iCitiesRequiredPerDomesticConnectedCity = pClassInfo->getCitiesRequiredPerDomesticConnectedCommerce();
	if (getMaxDomesticConnectedCommerce() == iDefault) m_iMaxDomesticConnectedCommerce = pClassInfo->getMaxDomesticConnectedCommerce();

	if (getForeignConnectedCommerce() == iDefault) m_iForeignConnectedCommerce = pClassInfo->getForeignConnectedCommerce();
	if (getCitiesRequiredPerForeignConnectedCommerce() == 1) m_iCitiesRequiredPerForeignConnectedCity = pClassInfo->getCitiesRequiredPerForeignConnectedCommerce();
	if (getMaxForeignConnectedCommerce() == iDefault) m_iMaxForeignConnectedCommerce = pClassInfo->getMaxForeignConnectedCommerce();
	if (getDiplomacyChange() == iDefault) m_iDiplomacyChange = pClassInfo->getDiplomacyChange();

	if (m_iCrime == iDefault) m_iCrime = pClassInfo->getCrime();
	if (m_iCrimePerPop == iDefault) m_iCrimePerPop = pClassInfo->getCrimePerPop();

	if (isProtectedCulture() == bDefault) m_bProtectedCulture = pClassInfo->isProtectedCulture();
	if (isAllowsAmbassadors() == bDefault) m_bAllowsAmbassadors = pClassInfo->isAllowsAmbassadors();
	if (getExtendsBuildingClass() == NO_BUILDINGCLASS) m_iExtendsBuildingClass = pClassInfo->getExtendsBuildingClass();

	if (!m_iGlobalMaintenanceModifier) m_iGlobalMaintenanceModifier = pClassInfo->getGlobalMaintenanceModifier();
	if (!m_iAreaMaintenanceModifier) m_iAreaMaintenanceModifier = pClassInfo->getAreaMaintenanceModifier();
	if (!m_iOtherAreaMaintenanceModifier) m_iOtherAreaMaintenanceModifier = pClassInfo->getOtherAreaMaintenanceModifier();
	if (!m_iDistanceMaintenanceModifier) m_iDistanceMaintenanceModifier = pClassInfo->getDistanceMaintenanceModifier();
	if (!m_iNumCitiesMaintenanceModifier) m_iNumCitiesMaintenanceModifier = pClassInfo->getNumCitiesMaintenanceModifier();
	if (!m_iCoastalDistanceMaintenanceModifier) m_iCoastalDistanceMaintenanceModifier = pClassInfo->getCoastalDistanceMaintenanceModifier();
	if (!m_iConnectedCityMaintenanceModifier) m_iConnectedCityMaintenanceModifier = pClassInfo->getConnectedCityMaintenanceModifier();

	for (int j = 0; j < NUM_MEMORY_TYPES; j++)
	{
		if (getMemoryTypeModifier(j) == iDefault && pClassInfo->getMemoryTypeModifier(j) != iDefault)
		{
			if (NULL == m_piMemoryTypeModifier)
			{
				CvXMLLoadUtility::InitList(&m_piMemoryTypeModifier, NUM_MEMORY_TYPES, iDefault);
			}
			m_piMemoryTypeModifier[j] = pClassInfo->getMemoryTypeModifier(j);
		}
	}

	for ( int j = 0; j < GC.getNumUnitCombatInfos(); j++)
	{
		if ( getUnitCombatExtraStrength(j) == iDefault && pClassInfo->getUnitCombatExtraStrength(j) != iDefault)
		{
			if ( NULL == m_piUnitCombatExtraStrength )
			{
				CvXMLLoadUtility::InitList(&m_piUnitCombatExtraStrength,GC.getNumUnitCombatInfos(),iDefault);
			}
			m_piUnitCombatExtraStrength[j] = pClassInfo->getUnitCombatExtraStrength(j);
		}
	}

	for ( int j = 0; j < NUM_COMMERCE_TYPES; j++)
	{
		if ( getCommerceAttacks(j) == iDefault && pClassInfo->getCommerceAttacks(j) != iDefault)
		{
			if ( NULL == m_piCommerceAttacks )
			{
				CvXMLLoadUtility::InitList(&m_piCommerceAttacks,NUM_COMMERCE_TYPES,iDefault);
			}
			m_piCommerceAttacks[j] = pClassInfo->getCommerceAttacks(j);
		}
	}

	for ( int j = 0; j < GC.getNumUnitClassInfos(); j++)
	{
		if ( getUnitClassProductionModifier(j) == iDefault && pClassInfo->getUnitClassProductionModifier(j) != iDefault)
		{
			if ( NULL == m_piUnitClassProductionModifier )
			{
				CvXMLLoadUtility::InitList(&m_piUnitClassProductionModifier,GC.getNumUnitClassInfos(),iDefault);
			}
			m_piUnitClassProductionModifier[j] = pClassInfo->getUnitClassProductionModifier(j);
		}
	}
	for ( int j = 0; j < GC.getNumTerrainInfos(); j++)
	{
		if ( isPrereqOrTerrain(j) == bDefault && pClassInfo->isPrereqOrTerrain(j) != bDefault)
		{
			if ( NULL == m_pbPrereqOrTerrain )
			{
				CvXMLLoadUtility::InitList(&m_pbPrereqOrTerrain,GC.getNumTerrainInfos(),bDefault);
			}
			m_pbPrereqOrTerrain[j] = pClassInfo->isPrereqOrTerrain(j);
		}
	}
	for ( int j = 0; j < GC.getNumTerrainInfos(); j++)
	{
		if ( isPrereqAndTerrain(j) == bDefault && pClassInfo->isPrereqAndTerrain(j) != bDefault)
		{
			if ( NULL == m_pbPrereqAndTerrain)
			{
				CvXMLLoadUtility::InitList(&m_pbPrereqAndTerrain,GC.getNumTerrainInfos(),bDefault);
			}
			m_pbPrereqAndTerrain[j] = pClassInfo->isPrereqAndTerrain(j);
		}
	}
	for ( int j = 0; j < GC.getNumImprovementInfos(); j++)
	{
		if ( isPrereqOrImprovement(j) == bDefault && pClassInfo->isPrereqOrImprovement(j) != bDefault)
		{
			if ( NULL == m_pbPrereqOrImprovement )
			{
				CvXMLLoadUtility::InitList(&m_pbPrereqOrImprovement,GC.getNumImprovementInfos(),bDefault);
			}
			m_pbPrereqOrImprovement[j] = pClassInfo->isPrereqOrImprovement(j);
		}
	}
	for ( int j = 0; j < GC.getNumFeatureInfos(); j++)
	{
		if ( isPrereqOrFeature(j) == bDefault && pClassInfo->isPrereqOrFeature(j) != bDefault)
		{
			if ( NULL == m_pbPrereqOrFeature )
			{
				CvXMLLoadUtility::InitList(&m_pbPrereqOrFeature,GC.getNumFeatureInfos(),bDefault);
			}
			m_pbPrereqOrFeature[j] = pClassInfo->isPrereqOrFeature(j);
		}
	}
	for ( int j = 0; j < GC.getNumBuildingClassInfos(); j++)
	{
		if ( isPrereqNotBuildingClass(j) == bDefault && pClassInfo->isPrereqNotBuildingClass(j) != bDefault)
		{
			if ( NULL == m_pbPrereqNotBuildingClass )
			{
				CvXMLLoadUtility::InitList(&m_pbPrereqNotBuildingClass,GC.getNumBuildingClassInfos(),bDefault);
			}
			m_pbPrereqNotBuildingClass[j] = pClassInfo->isPrereqNotBuildingClass(j);
		}
	}
	for ( int j = 0; j < GC.getNumBuildingClassInfos(); j++)
	{
		if ( isPrereqOrBuildingClass(j) == bDefault && pClassInfo->isPrereqOrBuildingClass(j) != bDefault)
		{
			if ( NULL == m_pbPrereqOrBuildingClass )
			{
				CvXMLLoadUtility::InitList(&m_pbPrereqOrBuildingClass,GC.getNumBuildingClassInfos(),bDefault);
			}
			m_pbPrereqOrBuildingClass[j] = pClassInfo->isPrereqOrBuildingClass(j);
		}
	}
	for ( int j = 0; j < GC.getNumGameSpeedInfos(); j++)
	{
		if ( isPrereqOrGameSpeed(j) == bDefault && pClassInfo->isPrereqOrGameSpeed(j) != bDefault)
		{
			if ( NULL == m_pbPrereqOrGameSpeed )
			{
				CvXMLLoadUtility::InitList(&m_pbPrereqOrGameSpeed,GC.getNumGameSpeedInfos(),bDefault);
			}
			m_pbPrereqOrGameSpeed[j] = pClassInfo->isPrereqOrGameSpeed(j);
		}
	}
	for ( int j = 0; j < GC.getNumBuildingClassInfos(); j++)
	{
		if ( getBuildingClassProductionModifier(j) == iDefault && pClassInfo->getBuildingClassProductionModifier(j) != iDefault)
		{
			if ( NULL == m_piBuildingClassProductionModifier )
			{
				CvXMLLoadUtility::InitList(&m_piBuildingClassProductionModifier, GC.getNumBuildingClassInfos(),iDefault);
			}
			m_piBuildingClassProductionModifier[j] = pClassInfo->getBuildingClassProductionModifier(j);
		}
	}
	for ( int j = 0; j < GC.getNumBuildingClassInfos(); j++)
	{
		if ( getGlobalBuildingClassProductionModifier(j) == iDefault && pClassInfo->getGlobalBuildingClassProductionModifier(j) != iDefault)
		{
			if ( NULL == m_piGlobalBuildingClassProductionModifier )
			{
				CvXMLLoadUtility::InitList(&m_piGlobalBuildingClassProductionModifier,GC.getNumBuildingClassInfos(),iDefault);
			}
			m_piGlobalBuildingClassProductionModifier[j] = pClassInfo->getGlobalBuildingClassProductionModifier(j);
		}
	}
	for ( int j = 0; j < GC.getNumBonusInfos(); j++)
	{
		if ( getBonusDefenseChanges(j) == iDefault && pClassInfo->getBonusDefenseChanges(j) != iDefault)
		{
			if ( NULL == m_piBonusDefenseChanges )
			{
				CvXMLLoadUtility::InitList(&m_piBonusDefenseChanges,GC.getNumBonusInfos(),iDefault);
			}
			m_piBonusDefenseChanges[j] = pClassInfo->getBonusDefenseChanges(j);
		}
	}
	for ( int j = 0; j < GC.getNumBuildingClassInfos(); j++)
	{
		if ( isReplaceBuildingClass(j) == bDefault && pClassInfo->isReplaceBuildingClass(j) != bDefault )
		{
			if ( NULL == m_pbReplaceBuildingClass )
			{
				CvXMLLoadUtility::InitList(&m_pbReplaceBuildingClass,GC.getNumBuildingClassInfos(),bDefault);
			}
			m_pbReplaceBuildingClass[j] = pClassInfo->isReplaceBuildingClass(j);
		}
	}
	for ( int i = 0; i < GC.getNumBonusInfos(); i++)	// "Init2DIntList" verify method
	{
		for ( int j = 0; j < NUM_COMMERCE_TYPES; j++)
		{
			if ( getBonusCommerceModifier(i,j) == iDefault )
			{
				int iChange = pClassInfo->getBonusCommerceModifier(i, j);

				if ( iChange != iDefault )
				{
					if ( m_ppaiBonusCommerceModifier == NULL )
					{
						m_ppaiBonusCommerceModifier = new int*[GC.getNumBonusInfos()];
						for(int k = 0; k < GC.getNumBonusInfos(); k++)
						{
							m_ppaiBonusCommerceModifier[k] = NULL;
						}
					}
					if ( m_ppaiBonusCommerceModifier[i] == NULL )
					{
						m_ppaiBonusCommerceModifier[i] = new int[NUM_COMMERCE_TYPES];
						for(int k = 0; k < NUM_COMMERCE_TYPES; k++)
						{
							m_ppaiBonusCommerceModifier[i][k] = NULL;
						}
					}

					m_ppaiBonusCommerceModifier[i][j] = iChange;
					m_bAnyBonusCommerceModifiers = true;
				}
			}
		}
	}

	for ( int i = 0; i < GC.getNumBonusInfos(); i++)	// "Init2DIntList" verify method
	{
		for ( int j = 0; j < NUM_YIELD_TYPES; j++)
		{
			if ( getBonusYieldChanges(i,j) == iDefault )
			{
				int iChange = pClassInfo->getBonusYieldChanges(i, j);

				if ( iChange != iDefault )
				{
					if ( m_ppaiBonusYieldChanges == NULL )
					{
						m_ppaiBonusYieldChanges = new int*[GC.getNumBonusInfos()];
						for(int k = 0; k < GC.getNumBonusInfos(); k++)
						{
							m_ppaiBonusYieldChanges[k] = NULL;
						}
					}
					if ( m_ppaiBonusYieldChanges[i] == NULL )
					{
						m_ppaiBonusYieldChanges[i] = new int[NUM_YIELD_TYPES];
						for(int k = 0; k < NUM_YIELD_TYPES; k++)
						{
							m_ppaiBonusYieldChanges[i][k] = NULL;
						}
					}

					m_ppaiBonusYieldChanges[i][j] = iChange;
					m_bAnyBonusYieldChanges = true;
				}
			}
		}
	}

	for ( int i = 0; i < GC.getNumBonusInfos(); i++)	// "Init2DIntList" verify method
	{
		for ( int j = 0; j < NUM_COMMERCE_TYPES; j++)
		{
			if ( getBonusCommercePercentChanges(i,j) == iDefault )
			{
				int iChange = pClassInfo->getBonusCommercePercentChanges(i, j);

				if ( iChange != iDefault )
				{
					if ( m_ppaiBonusCommercePercentChanges == NULL )
					{
						m_ppaiBonusCommercePercentChanges = new int*[GC.getNumBonusInfos()];
						for(int k = 0; k < GC.getNumBonusInfos(); k++)
						{
							m_ppaiBonusCommercePercentChanges[k] = NULL;
						}
					}
					if ( m_ppaiBonusCommercePercentChanges[i] == NULL )
					{
						m_ppaiBonusCommercePercentChanges[i] = new int[NUM_COMMERCE_TYPES];
						for(int k = 0; k < NUM_COMMERCE_TYPES; k++)
						{
							m_ppaiBonusCommercePercentChanges[i][k] = NULL;
						}
					}

					m_ppaiBonusCommercePercentChanges[i][j] = iChange;
					m_bAnyBonusCommercePercentChanges = true;
				}
			}
		}
	}

	for ( int i = 0; i < GC.getNumBonusInfos(); i++)	// "Init2DIntList" verify method
	{
		for ( int j = 0; j < NUM_YIELD_TYPES; j++)
		{
			if ( getVicinityBonusYieldChanges(i,j) == iDefault )
			{
				int iChange = pClassInfo->getVicinityBonusYieldChanges(i, j);

				if ( iChange != iDefault )
				{
					if ( m_ppaiVicinityBonusYieldChanges == NULL )
					{
						m_ppaiVicinityBonusYieldChanges = new int*[GC.getNumBonusInfos()];
						for(int k = 0; k < GC.getNumBonusInfos(); k++)
						{
							m_ppaiVicinityBonusYieldChanges[k] = NULL;
						}
					}
					if ( m_ppaiVicinityBonusYieldChanges[i] == NULL )
					{
						m_ppaiVicinityBonusYieldChanges[i] = new int[NUM_YIELD_TYPES];
						for(int k = 0; k < NUM_YIELD_TYPES; k++)
						{
							m_ppaiVicinityBonusYieldChanges[i][k] = NULL;
						}
					}

					m_ppaiVicinityBonusYieldChanges[i][j] = iChange;
					m_bAnyVicinityBonusYieldChanges = true;
				}
			}
		}
	}

	for ( int i = 0; i < GC.getNumImprovementInfos(); i++ )
	{
		for ( int j = 0; j < NUM_YIELD_TYPES; j++ )
		{
			if ( getImprovementYieldChanges(i,j) == iDefault )
			{
				int iChange = pClassInfo->getImprovementYieldChanges(i, j);

				if ( iChange != iDefault )
				{
					if ( m_ppiImprovementYieldChanges == NULL )
					{
						m_ppiImprovementYieldChanges = new int*[GC.getNumImprovementInfos()];
						for(int k = 0; k < GC.getNumImprovementInfos(); k++)
						{
							m_ppiImprovementYieldChanges[k] = NULL;
						}
					}
					if ( m_ppiImprovementYieldChanges[i] == NULL )
					{
						m_ppiImprovementYieldChanges[i] = new int[NUM_YIELD_TYPES];
						for(int k = 0; k < NUM_YIELD_TYPES; k++)
						{
							m_ppiImprovementYieldChanges[i][k] = NULL;
						}
					}

					m_ppiImprovementYieldChanges[i][j] = iChange;
				}
			}
		}
	}

	for ( int j = 0; j < GC.getNumTechInfos(); j++)
	{
		if ( getTechHappinessChanges(j) == iDefault && pClassInfo->getTechHappinessChanges(j) != iDefault)
		{
			if ( NULL == m_piTechHappinessChanges)
			{
				CvXMLLoadUtility::InitList(&m_piTechHappinessChanges,GC.getNumTechInfos(),iDefault);
			}
			m_piTechHappinessChanges[j] = pClassInfo->getTechHappinessChanges(j);
		}
	}

	for ( int j = 0; j < GC.getNumTechInfos(); j++)
	{
		if ( getTechHealthChanges(j) == iDefault && pClassInfo->getTechHealthChanges(j) != iDefault)
		{
			if ( NULL == m_piTechHealthChanges)
			{
				CvXMLLoadUtility::InitList(&m_piTechHealthChanges,GC.getNumTechInfos(),iDefault);
			}
			m_piTechHealthChanges[j] = pClassInfo->getTechHealthChanges(j);
		}
	}

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

	for ( int i = 0; i < GC.getNumTechInfos(); i++)	// "Init2DIntList" verify method
	{
		for ( int j = 0; j < NUM_COMMERCE_TYPES; j++)
		{
			if ( getTechCommerceChange(i,j) == iDefault )
			{
				int iChange = pClassInfo->getTechCommerceChange(i, j);

				if ( iChange != iDefault )
				{
					if ( m_ppaiTechCommerceChange == NULL )
					{
						m_ppaiTechCommerceChange = new int*[GC.getNumTechInfos()];
						for(int k = 0; k < GC.getNumTechInfos(); k++)
						{
							m_ppaiTechCommerceChange[k] = NULL;
						}
					}
					if ( m_ppaiTechCommerceChange[i] == NULL )
					{
						m_ppaiTechCommerceChange[i] = new int[NUM_COMMERCE_TYPES];
						for(int k = 0; k < NUM_COMMERCE_TYPES; k++)
						{
							m_ppaiTechCommerceChange[i][k] = NULL;
						}
					}

					m_ppaiTechCommerceChange[i][j] = iChange;
					m_bAnyTechCommerceChanges = true;
				}
			}
		}
	}

	for ( int i = 0; i < GC.getNumTechInfos(); i++)	// "Init2DIntList" verify method
	{
		for ( int j = 0; j < NUM_YIELD_TYPES; j++)
		{
			if ( getTechYieldChange(i,j) == iDefault )
			{
				int iChange = pClassInfo->getTechYieldChange(i, j);

				if ( iChange != iDefault )
				{
					if ( m_ppaiTechYieldChange == NULL )
					{
						m_ppaiTechYieldChange = new int*[GC.getNumTechInfos()];
						for(int k = 0; k < GC.getNumTechInfos(); k++)
						{
							m_ppaiTechYieldChange[k] = NULL;
						}
					}
					if ( m_ppaiTechYieldChange[i] == NULL )
					{
						m_ppaiTechYieldChange[i] = new int[NUM_YIELD_TYPES];
						for(int k = 0; k < NUM_YIELD_TYPES; k++)
						{
							m_ppaiTechYieldChange[i][k] = NULL;
						}
					}

					m_ppaiTechYieldChange[i][j] = iChange;
					m_bAnyTechYieldChanges = true;
				}
			}
		}
	}

	for ( int i = 0; i < GC.getNumTechInfos(); i++)	// "Init2DIntList" verify method
	{
		for ( int j = 0; j < GC.getNumSpecialistInfos(); j++)
		{
			if ( getTechSpecialistChange(i,j) == iDefault )
			{
				int iChange = pClassInfo->getTechSpecialistChange(i, j);

				if ( iChange != iDefault )
				{
					if ( m_ppaiTechSpecialistChange == NULL )
					{
						m_ppaiTechSpecialistChange = new int*[GC.getNumTechInfos()];
						for(int k = 0; k < GC.getNumTechInfos(); k++)
						{
							m_ppaiTechSpecialistChange[k] = NULL;
						}
					}
					if ( m_ppaiTechSpecialistChange[i] == NULL )
					{
						m_ppaiTechSpecialistChange[i] = new int[GC.getNumSpecialistInfos()];
						for(int k = 0; k < GC.getNumSpecialistInfos(); k++)
						{
							m_ppaiTechSpecialistChange[i][k] = NULL;
						}
					}

					m_ppaiTechSpecialistChange[i][j] = iChange;
					m_bAnyTechSpecialistChanges = true;
				}
			}
		}
	}

	for ( int i = 0; i < GC.getNumTechInfos(); i++)	// "Init2DIntList" verify method
	{
		for ( int j = 0; j < NUM_COMMERCE_TYPES; j++)
		{
			if ( getTechCommerceModifier(i,j) == iDefault )
			{
				int iChange = pClassInfo->getTechCommerceModifier(i, j);

				if ( iChange != iDefault )
				{
					if ( m_ppaiTechCommerceModifier == NULL )
					{
						m_ppaiTechCommerceModifier = new int*[GC.getNumTechInfos()];
						for(int k = 0; k < GC.getNumTechInfos(); k++)
						{
							m_ppaiTechCommerceModifier[k] = NULL;
						}
					}
					if ( m_ppaiTechCommerceModifier[i] == NULL )
					{
						m_ppaiTechCommerceModifier[i] = new int[NUM_COMMERCE_TYPES];
						for(int k = 0; k < NUM_COMMERCE_TYPES; k++)
						{
							m_ppaiTechCommerceModifier[i][k] = NULL;
						}
					}

					m_ppaiTechCommerceModifier[i][j] = iChange;
					m_bAnyTechCommerceModifiers = true;
				}
			}
		}
	}

	for ( int i = 0; i < GC.getNumTechInfos(); i++)	// "Init2DIntList" verify method
	{
		for ( int j = 0; j < NUM_YIELD_TYPES; j++)
		{
			if ( getTechYieldModifier(i,j) == iDefault )
			{
				int iChange = pClassInfo->getTechYieldModifier(i, j);

				if ( iChange != iDefault )
				{
					if ( m_ppaiTechYieldModifier == NULL )
					{
						m_ppaiTechYieldModifier = new int*[GC.getNumTechInfos()];
						for(int k = 0; k < GC.getNumTechInfos(); k++)
						{
							m_ppaiTechYieldModifier[k] = NULL;
						}
					}
					if ( m_ppaiTechYieldModifier[i] == NULL )
					{
						m_ppaiTechYieldModifier[i] = new int[NUM_YIELD_TYPES];
						for(int k = 0; k < NUM_YIELD_TYPES; k++)
						{
							m_ppaiTechYieldModifier[i][k] = NULL;
						}
					}

					m_ppaiTechYieldModifier[i][j] = iChange;
					m_bAnyTechYieldModifiers = true;
				}
			}
		}
	}

	for (int i = 0; i < GC.getNumSpecialistInfos(); i++)	// "Init2DIntList" verify method
	{
		for (int j = 0; j < NUM_COMMERCE_TYPES; j++)
		{
			if (getSpecialistCommercePercentChanges(i, j) == iDefault)
			{
				int iChange = pClassInfo->getSpecialistCommercePercentChanges(i, j);

				if (iChange != iDefault)
				{
					if (m_ppaiSpecialistCommercePercentChanges == NULL)
					{
						m_ppaiSpecialistCommercePercentChanges = new int*[GC.getNumSpecialistInfos()];
						for (int k = 0; k < GC.getNumSpecialistInfos(); k++)
						{
							m_ppaiSpecialistCommercePercentChanges[k] = NULL;
						}
					}
					if (m_ppaiSpecialistCommercePercentChanges[i] == NULL)
					{
						m_ppaiSpecialistCommercePercentChanges[i] = new int[NUM_COMMERCE_TYPES];
					}

					m_ppaiSpecialistCommercePercentChanges[i][j] = iChange;
					m_bAnySpecialistCommerceChanges = true;
				}
			}
		}
	}

	//These are done differently because of the ReadPass3
	for ( int i = 0; i < pClassInfo->isPrereqOrCivicsVectorSize(); i++ )
	{
		m_abPrereqOrCivicsforPass3.push_back(pClassInfo->isPrereqOrCivicsValuesVectorElement(i));
		m_aszPrereqOrCivicsforPass3.push_back(pClassInfo->isPrereqOrCivicsNamesVectorElement(i));
	}

	for ( int i = 0; i < pClassInfo->isPrereqAndCivicsVectorSize(); i++ )
	{
		m_abPrereqAndCivicsforPass3.push_back(pClassInfo->isPrereqAndCivicsValuesVectorElement(i));
		m_aszPrereqAndCivicsforPass3.push_back(pClassInfo->isPrereqAndCivicsNamesVectorElement(i));
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	m_Properties.copyNonDefaults(pClassInfo->getProperties(), pXML);
	m_PropertiesAllCities.copyNonDefaults(pClassInfo->getPropertiesAllCities(), pXML);
	m_PrereqMinProperties.copyNonDefaults(pClassInfo->getPrereqMinProperties(), pXML);
	m_PrereqMaxProperties.copyNonDefaults(pClassInfo->getPrereqMaxProperties(), pXML);
	m_PrereqPlayerMinProperties.copyNonDefaults(pClassInfo->getPrereqPlayerMinProperties(), pXML);
	m_PrereqPlayerMaxProperties.copyNonDefaults(pClassInfo->getPrereqPlayerMaxProperties(), pXML);

	m_PropertyManipulators.copyNonDefaults(pClassInfo->getPropertyManipulators(), pXML);

	if (!m_pExprNewCityFree)
	{
		m_pExprNewCityFree = pClassInfo->m_pExprNewCityFree;
		pClassInfo->m_pExprNewCityFree = NULL;
	}

	if (!m_pExprConstructCondition)
	{
		m_pExprConstructCondition = pClassInfo->m_pExprConstructCondition;
		pClassInfo->m_pExprConstructCondition = NULL;
	}
}
/************************************************************************************************/
/* XMLCOPY                                 END                                                  */
/************************************************************************************************/

CvPropertyManipulators* CvBuildingInfo::getPropertyManipulators()
{
	return &m_PropertyManipulators;
}

bool CvBuildingInfo::isNewCityFree(CvGameObject* pObject)
{
	if (m_pExprNewCityFree)
	{
		return m_pExprNewCityFree->evaluate(pObject);
	}
	return false;
}

BoolExpr* CvBuildingInfo::getConstructCondition()
{
	return m_pExprConstructCondition;
}

bool CvBuildingInfo::getNotShowInCity() const
{
	return m_bNotShowInCity;
}

void CvBuildingInfo::setNotShowInCity()
{
	m_bNotShowInCity = 
	(
			m_szArtDefineTag == ""
		||	getArtInfo()->getScale() == 0.0
		||	stricmp(getArtInfo()->getNIF(), "Art/empty.nif") == 0
		// Toffer - Requested by Inthegrave
		||	m_fVisibilityPriority < 0.0f
	);
}
