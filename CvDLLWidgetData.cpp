#include "CvGameCoreDLL.h"
#include "CvGameCoreUtils.h"
#include "CvInitCore.h"
#include "CvCity.h"
#include "CvUnit.h"
#include "CvGlobals.h"
#include "CvGameAI.h"
#include "CvMap.h"
#include "CvViewport.h"
#include "CvPlot.h"
#include "CvTeamAI.h"
#include "CvPlayerAI.h"
#include "CvGameCoreUtils.h"
#include "CyArgsList.h"
#include "CvGameTextMgr.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLPythonIFaceBase.h"
#include "CvDLLEngineIFaceBase.h"
#include "CvEventReporter.h"
#include "CvDLLWidgetData.h"
#include "CvPopupInfo.h"
#include "FProfiler.h"
#include "CvMessageControl.h"

// BUG - start
// RevolutionDCM - BugMod included in CvGlobals.h
//#include "BugMod.h"
#include "CvBugOptions.h"
// BUG - end

// BUFFY - start
#ifdef _BUFFY
#include "Buffy.h"
#endif
// BUFFY - end

CvDLLWidgetData* CvDLLWidgetData::m_pInst = NULL;

CvDLLWidgetData& CvDLLWidgetData::getInstance()
{
	if (m_pInst == NULL)
	{
		m_pInst = new CvDLLWidgetData;
	}
	return *m_pInst;
}

void CvDLLWidgetData::freeInstance()
{
	delete m_pInst;
	m_pInst = NULL;
}

void CvDLLWidgetData::parseHelp(CvWStringBuffer &szBuffer, CvWidgetDataStruct &widgetDataStruct)
{
	switch (widgetDataStruct.m_eWidgetType)
	{
	case WIDGET_PLOT_LIST:
		parsePlotListHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PLOT_LIST_SHIFT:
		szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CTRL_SHIFT", (GC.getDefineINT("MAX_PLOT_LIST_SIZE") - 1)));
		break;

	case WIDGET_CITY_SCROLL:
		break;

	case WIDGET_LIBERATE_CITY:
		parseLiberateCityHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CITY_NAME:
		parseCityNameHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_UNIT_NAME:
		szBuffer.append(gDLL->getText("TXT_KEY_CHANGE_NAME"));
		break;

	case WIDGET_CREATE_GROUP:
		szBuffer.append(gDLL->getText("TXT_KEY_WIDGET_CREATE_GROUP"));
		break;

	case WIDGET_DELETE_GROUP:
		szBuffer.append(gDLL->getText("TXT_KEY_WIDGET_DELETE_GROUP"));
		break;

	case WIDGET_TRAIN:
		parseTrainHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CONSTRUCT:
		parseConstructHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CREATE:
		parseCreateHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_MAINTAIN:
		parseMaintainHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_BUILD_LIST_TRAIN:
		GAMETEXT.setUnitHelp(szBuffer, (UnitTypes)widgetDataStruct.m_iData1);
		break;

	case WIDGET_BUILD_LIST_CONSTRUCT:
		GAMETEXT.setBuildingHelp(szBuffer, (BuildingTypes)widgetDataStruct.m_iData1);
		break;

	case WIDGET_BUILD_LIST_CREATE:
		GAMETEXT.setProjectHelp(szBuffer, (ProjectTypes)widgetDataStruct.m_iData1);
		break;

	case WIDGET_BUILD_LIST_QUEUE:
		parseBuildListQueueHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_BUILD_LIST:
	case WIDGET_BUILD_LIST_LIST:
		parseBuildListHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HURRY:
		parseHurryHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_MENU_ICON:
		szBuffer.append(gDLL->getText("TXT_KEY_MAIN_MENU"));

	case WIDGET_CONSCRIPT:
		parseConscriptHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_ACTION:
		parseActionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CITIZEN:
		parseCitizenHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_FREE_CITIZEN:
		parseFreeCitizenHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_DISABLED_CITIZEN:
		parseDisabledCitizenHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_ANGRY_CITIZEN:
		parseAngryCitizenHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CHANGE_SPECIALIST:
		parseChangeSpecialistHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_RESEARCH:
		parseResearchHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_TECH_TREE:
		parseTechTreeHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CHANGE_PERCENT:
		parseChangePercentHelp(widgetDataStruct, szBuffer);
		break;

// BUG - Min/Max Commerce Rate - start
	case WIDGET_SET_PERCENT:
		parseSetPercentHelp(widgetDataStruct, szBuffer);
		break;
// BUG - Min/Max Commerce Rate - end

	case WIDGET_CITY_TAB:
		{
			CvWString szTemp;
            szTemp.Format(L"%s", GC.getCityTabInfo((CityTabTypes)widgetDataStruct.m_iData1).getDescription());
			szBuffer.assign(szTemp);
		}
		break;

	case WIDGET_CONTACT_CIV:
		parseContactCivHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_SCORE_BREAKDOWN:
		parseScoreHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_ZOOM_CITY:
		szBuffer.append(gDLL->getText("TXT_KEY_ZOOM_CITY_HELP"));
// BUG - Zoom City Details - start
		if (getBugOptionBOOL("MiscHover__CDAZoomCityDetails", true, "BUG_CDA_ZOOM_CITY_DETAILS"))
		{
			// only if the active player owns the city
			if (GC.getGame().getActivePlayer() == widgetDataStruct.m_iData1) {
				szBuffer.append(NEWLINE);
				GAMETEXT.setCityBarHelp(szBuffer, GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getCity(widgetDataStruct.m_iData2));
			}
		}
// BUG - Zoom City Details - end
		break;

	case WIDGET_END_TURN:
		szBuffer.append(gDLL->getText("TXT_KEY_WIDGET_END_TURN"));
		break;
		
	case WIDGET_LAUNCH_VICTORY:
		szBuffer.append(gDLL->getText("TXT_KEY_WIDGET_LAUNCH_VICTORY"));
		break;

	case WIDGET_AUTOMATE_CITIZENS:
		parseAutomateCitizensHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_AUTOMATE_PRODUCTION:
		parseAutomateProductionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_EMPHASIZE:
		parseEmphasizeHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_BUILDING_FILTER:
		parseBuildingFilterHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_BUILDING_GROUPING:
		parseBuildingGroupingHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_BUILDING_SORT:
		parseBuildingSortHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_UNIT_FILTER:
		parseUnitFilterHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_UNIT_GROUPING:
		parseUnitGroupingHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_UNIT_SORT:
		parseUnitSortHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_TRADE_ITEM:
		parseTradeItem(widgetDataStruct, szBuffer);
		break;

	case WIDGET_UNIT_MODEL:
		parseUnitModelHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_FLAG:
		parseFlagHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_MAINTENANCE:
		parseMaintenanceHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_RELIGION:
		parseReligionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_RELIGION_CITY:
		parseReligionHelpCity(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_CORPORATION_CITY:
		parseCorporationHelpCity(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_NATIONALITY:
		parseNationalityHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_DEFENSE:
/************************************************************************************************/
/* Afforess	                  Start		 01/13/10                                               */
/*                                                                                              */
/*    Defense Hover                                                                             */
/************************************************************************************************/
		parseDefenseModHelp(widgetDataStruct, szBuffer);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		break;

	case WIDGET_HELP_HEALTH:
		parseHealthHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_HAPPINESS:
		parseHappinessHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_POPULATION:
		parsePopulationHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_PRODUCTION:
		parseProductionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_CULTURE:
		parseCultureHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_GREAT_PEOPLE:
		parseGreatPeopleHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_GREAT_GENERAL:
		parseGreatGeneralHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_SELECTED:
		parseSelectedHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_BUILDING:
		parseBuildingHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_TRADE_ROUTE_CITY:
		parseTradeRouteCityHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_ESPIONAGE_COST:
		parseEspionageCostHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_TECH_ENTRY:
		parseTechEntryHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_TECH_PREPREQ:
		parseTechPrereqHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_OBSOLETE:
		parseObsoleteHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_OBSOLETE_BONUS:
		parseObsoleteBonusString(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_OBSOLETE_SPECIAL:
		parseObsoleteSpecialHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_MOVE_BONUS:
		parseMoveHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FREE_UNIT:
		parseFreeUnitHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FEATURE_PRODUCTION:
		parseFeatureProductionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_WORKER_RATE:
		parseWorkerRateHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_TRADE_ROUTES:
		parseTradeRouteHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_HEALTH_RATE:
		parseHealthRateHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_HAPPINESS_RATE:
		parseHappinessRateHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FREE_TECH:
		parseFreeTechHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_LOS_BONUS:
		parseLOSHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_MAP_CENTER:
		parseMapCenterHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_MAP_REVEAL:
		parseMapRevealHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_MAP_TRADE:
		parseMapTradeHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_TECH_TRADE:
		parseTechTradeHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_GOLD_TRADE:
		parseGoldTradeHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_OPEN_BORDERS:
		parseOpenBordersHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_DEFENSIVE_PACT:
		parseDefensivePactHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_PERMANENT_ALLIANCE:
		parsePermanentAllianceHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_VASSAL_STATE:
		parseVassalStateHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_BUILD_BRIDGE:
		parseBuildBridgeHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_IRRIGATION:
		parseIrrigationHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_IGNORE_IRRIGATION:
		parseIgnoreIrrigationHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_WATER_WORK:
		parseWaterWorkHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_IMPROVEMENT:
		parseBuildHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_DOMAIN_EXTRA_MOVES:
		parseDomainExtraMovesHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_ADJUST:
		parseAdjustHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_TERRAIN_TRADE:
		parseTerrainTradeHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_SPECIAL_BUILDING:
		parseSpecialBuildingHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_YIELD_CHANGE:
		parseYieldChangeHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_BONUS_REVEAL:
		parseBonusRevealHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_CIVIC_REVEAL:
		parseCivicRevealHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_PROCESS_INFO:
		parseProcessInfoHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FOUND_RELIGION:
		parseFoundReligionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FOUND_CORPORATION:
		parseFoundCorporationHelp(widgetDataStruct, szBuffer);
		break;

// BUG - Finance Advisor - start
	case WIDGET_HELP_FINANCE_DOMESTIC_TRADE:
		parseFinanceDomesticTrade(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_FOREIGN_TRADE:
		parseFinanceForeignTrade(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_SPECIALISTS:
		parseFinanceSpecialistGold(widgetDataStruct, szBuffer);
		break;
// BUG - Finance Advisor - end

	case WIDGET_HELP_FINANCE_NUM_UNITS:
		parseFinanceNumUnits(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_UNIT_COST:
		parseFinanceUnitCost(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_AWAY_SUPPLY:
		parseFinanceAwaySupply(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_CITY_MAINT:
		parseFinanceCityMaint(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_CIVIC_UPKEEP:
		parseFinanceCivicUpkeep(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_FOREIGN_INCOME:
		parseFinanceForeignIncome(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_INFLATED_COSTS:
		parseFinanceInflatedCosts(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_GROSS_INCOME:
		parseFinanceGrossIncome(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_NET_GOLD:
		parseFinanceNetGold(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_GOLD_RESERVE:
		parseFinanceGoldReserve(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_TECH:
		parseTechEntryHelp(widgetDataStruct, szBuffer);
		break;

// BUG - Trade Denial - start
	case WIDGET_PEDIA_JUMP_TO_TECH_TRADE:
		parseTechTradeEntryHelp(widgetDataStruct, szBuffer);
		break;
// BUG - Trade Denial - end

	case WIDGET_PEDIA_JUMP_TO_REQUIRED_TECH:
		parseTechTreePrereq(widgetDataStruct, szBuffer, false);
		break;

	case WIDGET_PEDIA_JUMP_TO_DERIVED_TECH:
		parseTechTreePrereq(widgetDataStruct, szBuffer, true);
		break;

	case WIDGET_PEDIA_JUMP_TO_UNIT:
		parseUnitHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_BUILDING:
		parseBuildingHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_BACK:
		// parsePediaBack(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_FORWARD:
		// parsePediaForward(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_BONUS:
		parseBonusHelp(widgetDataStruct, szBuffer);
		break;

// BUG - Trade Denial - start
	case WIDGET_PEDIA_JUMP_TO_BONUS_TRADE:
		parseBonusTradeHelp(widgetDataStruct, szBuffer);
		break;
// BUG - Trade Denial - end

	case WIDGET_PEDIA_MAIN:
		break;

	case WIDGET_PEDIA_JUMP_TO_PROMOTION:
	case WIDGET_HELP_PROMOTION:
		parsePromotionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CHOOSE_EVENT:
		parseEventHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_UNIT_COMBAT:
		parseUnitCombatHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_IMPROVEMENT:
		parseImprovementHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_ROUTE:
		parseRouteHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_CIVIC:
		parseCivicHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_CIV:
		parseCivilizationHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_LEADER:
		parseLeaderHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_SPECIALIST:
		if (widgetDataStruct.m_iData1 != NO_SPECIALIST && widgetDataStruct.m_iData2 != 0)
		{
			CvWString szTemp;
			szTemp.Format(L"%s", GC.getSpecialistInfo((SpecialistTypes)widgetDataStruct.m_iData1).getDescription());
			szBuffer.assign(szTemp);
		}
		break;

	case WIDGET_PEDIA_JUMP_TO_PROJECT:
		parseProjectHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_RELIGION:
		parseReligionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_CORPORATION:
		parseCorporationHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_TERRAIN:
		parseTerrainHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_FEATURE:
		parseFeatureHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_DESCRIPTION:
		parseDescriptionHelp(widgetDataStruct, szBuffer, false);
		break;

	case WIDGET_CLOSE_SCREEN:
		//parseCloseScreenHelp(szBuffer);
		break;

	case WIDGET_DEAL_KILL:
		parseKillDealHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_DESCRIPTION_NO_HELP:
		//parseDescriptionHelp(widgetDataStruct, szBuffer, true);
		break;

	case WIDGET_MINIMAP_HIGHLIGHT:
		break;

// BUG - Food Rate Hover - start
	case WIDGET_FOOD_MOD_HELP:
		parseFoodModHelp(widgetDataStruct, szBuffer);
		break;
// BUG - Food Rate Hover - end

	case WIDGET_PRODUCTION_MOD_HELP:
		parseProductionModHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_LEADERHEAD:
		parseLeaderheadHelp(widgetDataStruct, szBuffer);
		break;

// BUG - Leaderhead Relations - start
	case WIDGET_LEADERHEAD_RELATIONS:
		parseLeaderheadRelationsHelp(widgetDataStruct, szBuffer);
		break;
// BUG - Leaderhead Relations - end

	case WIDGET_LEADER_LINE:
		parseLeaderLineHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_COMMERCE_MOD_HELP:
		parseCommerceModHelp(widgetDataStruct, szBuffer);
		break;

// BUG - Trade Hover - start
	case WIDGET_TRADE_ROUTES:
		parseTradeRoutes(widgetDataStruct, szBuffer);
		break;
// BUG - Trade Hover - end

/************************************************************************************************/
/* Afforess	                  Start		 6/19/11                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	case WIDGET_OBSOLETE_PROMOTION:
		parseObsoletePromotion(widgetDataStruct, szBuffer);
		break;
	case WIDGET_OBSOLETE_CORPORATION:
		parseObsoleteCorporation(widgetDataStruct, szBuffer);
		break;
	case WIDGET_HELP_EMPLOYMENT:
		{
			CvCity* pCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
			if (NULL != pCity)
			{
				GAMETEXT.setEmploymentHelp(szBuffer, *pCity);
			}
			break;
		}
	case WIDGET_PROPERTY:
		if ((PropertyTypes)widgetDataStruct.m_iData1 != NO_PROPERTY)
		{
			szBuffer.append(GC.getPropertyInfo((PropertyTypes)widgetDataStruct.m_iData1).getText());
			CvCity* pCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
			if (NULL != pCity)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_PROPERTY_SOURCED_CONSTANT", pCity->getTotalBuildingSourcedProperty((PropertyTypes)widgetDataStruct.m_iData1)));
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_PROPERTY_GLOBAL_SOURCED", pCity->getGlobalSourcedProperty((PropertyTypes)widgetDataStruct.m_iData1)));
			}
		}
		break;
	case WIDGET_HELP_IMPROVEMENT_CAN_UPGRADE:
		if (widgetDataStruct.m_iData1 != NO_IMPROVEMENT)
		{
			ImprovementTypes eNewImprovement = (ImprovementTypes)widgetDataStruct.m_iData1;
			ImprovementTypes eOldImprovement = (ImprovementTypes)GC.getImprovementInfo(eNewImprovement).getImprovementPillage();
			
			if (eOldImprovement != NO_IMPROVEMENT)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_TECH_ALLOWS_IMPROVEMENT_UPGRADE", GC.getImprovementInfo(eOldImprovement).getDescription(), GC.getImprovementInfo(eNewImprovement).getDescription() ));
			}
		}
		break;
	case WIDGET_EXTRA_TECH_STRENGTH:
		if (widgetDataStruct.m_iData1 != NO_UNIT && widgetDataStruct.m_iData2 != NO_TECH)
		{
			UnitTypes eUnit = (UnitTypes)widgetDataStruct.m_iData1;
			TechTypes eTech = (TechTypes)widgetDataStruct.m_iData2;
			bool bFound = false;
			CvWString szUnitName;
			if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
			{
				if (GC.getUnitInfo(eUnit).getCivilizationName(GC.getGameINLINE().getActiveCivilizationType()) != NULL)
				{
					if (!CvWString(GC.getUnitInfo(eUnit).getCivilizationName(GC.getGameINLINE().getActiveCivilizationType())).empty())
					{	
						bFound = true;
						szUnitName = gDLL->getText(GC.getUnitInfo(eUnit).getCivilizationName(GC.getGameINLINE().getActiveCivilizationType()));
					}
				}
			}
			if (!bFound)
			{
				szUnitName = GC.getUnitInfo(eUnit).getDescription();
			}
			if (GC.getTechInfo(eTech).getUnitClassStrengthChange(GC.getUnitInfo(eUnit).getUnitClassType()) != 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_TECH_UNIT_STRENGTH_CHANGE", GC.getTechInfo(eTech).getUnitClassStrengthChange(GC.getUnitInfo(eUnit).getUnitClassType()), szUnitName.c_str()));
			}
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

		case WIDGET_HELP_CITY_CRIME:
		{
			bool bFirst = true;
			const CvCity* city = gDLL->getInterfaceIFace()->getHeadSelectedCity();
			const int iCrimeFinal = city->getCrimeFinal();
			int iCrimeOther = iCrimeFinal;
			{
				const int iBaseCrime = GC.getDefineINT("CRIME_PER_100_POPULATION", 99) * city->getPopulation();
				CvWString szValue;
				if (iBaseCrime < 0)
				{
					if (iBaseCrime / 100 == 0)
					{
						szValue.Format(L"-0.%02d", abs(iBaseCrime) % 100);
					}
					else szValue.Format(L"%d.%02d", iBaseCrime / 100, abs(iBaseCrime) % 100);
				}
				else szValue.Format(L"%d.%02d", iBaseCrime / 100, iBaseCrime % 100);

				szBuffer.append(gDLL->getText("TXT_KEY_HELP_CITY_CRIME_BASE", szValue.GetCString()));

				const int iCrimePerPop = city->getCrimePerPop();
				if (iCrimePerPop != 0)
				{
					szBuffer.append(SEPARATOR);
					szBuffer.append(NEWLINE);
					if (iCrimePerPop < 0)
					{
						if (iCrimePerPop / 100 == 0)
						{
							szValue.Format(L"-0.%02d", abs(iCrimePerPop) % 100);
						}
						else szValue.Format(L"%d.%02d", iCrimePerPop / 100, abs(iCrimePerPop) % 100);
					}
					else szValue.Format(L"%d.%02d", iCrimePerPop / 100, iCrimePerPop % 100);

					CvWString szValue2;
					const int iCrimeFromPop = iCrimePerPop * city->getPopulation();
					if (iCrimeFromPop < 0)
					{
						if (iCrimeFromPop / 100 == 0)
						{
							szValue2.Format(L"-0.%02d", abs(iCrimeFromPop) % 100);
						}
						else szValue2.Format(L"%d.%02d", iCrimeFromPop / 100, abs(iCrimeFromPop) % 100);
					}
					else szValue2.Format(L"%d.%02d", iCrimeFromPop / 100, iCrimeFromPop % 100);

					szBuffer.append(gDLL->getText("TXT_KEY_HELP_CITY_CRIME_BUILDINGS_PER_POP", szValue.GetCString(), szValue2.GetCString()));
				}
				iCrimeOther -= (iBaseCrime + iCrimePerPop * city->getPopulation()) / 100;
			}
			{
				const int iCrime = city->getCrime();
				if (iCrime != 0)
				{
					if (bFirst)
					{
						szBuffer.append(SEPARATOR);
						bFirst = false;
					}
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_HELP_CITY_CRIME_BUILDINGS", iCrime));
					iCrimeOther -= iCrime;
				}
			}
			if (iCrimeOther != 0)
			{
				if (bFirst)
				{
					szBuffer.append(SEPARATOR);
					bFirst = false;
				}
				szBuffer.append(NEWLINE);
				FAssertMsg(false, "Something is not listed in tooltip, iCrimeOther != 0");
				szBuffer.append(gDLL->getText("TXT_KEY_HELP_CITY_CRIME_OTHER", iCrimeOther));
			}
			szBuffer.append(DOUBLE_SEPARATOR);
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_HELP_CITY_CRIME_SUM", iCrimeFinal));
			bFirst = true;
			{
				const int iCommercePenalty = city->getCrimePenaltyValue(city->getYieldRate100(YIELD_COMMERCE, false));
				if (iCommercePenalty != 0)
				{
					CvWString szYield = CvWString::format(L"%d.%02d", iCommercePenalty / 100, std::abs(iCommercePenalty % 100));
					CvYieldInfo& info = GC.getYieldInfo(YIELD_COMMERCE);
					szBuffer.append(DOUBLE_SEPARATOR);
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_HELP_CITY_CRIME_GENERIC_PENALTY", info.getTextKeyWide(), szYield.GetCString(), info.getChar()));
					bFirst = false;
				}
			}
		}
	}
}

// Protected Functions...
bool CvDLLWidgetData::executeAction( CvWidgetDataStruct &widgetDataStruct )
{
	bool bHandled = false;			//	Right now general bHandled = false;  We can specific case this to true later.  Game will run with this = false;

	switch (widgetDataStruct.m_eWidgetType)
	{

	case WIDGET_PLOT_LIST:
		doPlotList(widgetDataStruct);
		break;

	case WIDGET_PLOT_LIST_SHIFT:
		gDLL->getInterfaceIFace()->changePlotListColumn(widgetDataStruct.m_iData1 * ((gDLL->ctrlKey()) ? (GC.getDefineINT("MAX_PLOT_LIST_SIZE") - 1) : 1));
		break;

	case WIDGET_CITY_SCROLL:
		if ( widgetDataStruct.m_iData1 > 0 )
		{
			GC.getGameINLINE().doControl(CONTROL_NEXTCITY);
		}
		else
		{
			GC.getGameINLINE().doControl(CONTROL_PREVCITY);
		}
		break;

	case WIDGET_LIBERATE_CITY:
		doLiberateCity();
		break;

	case WIDGET_CITY_NAME:
		doRenameCity();
		break;

	case WIDGET_UNIT_NAME:
		doRenameUnit();
		break;

	case WIDGET_CREATE_GROUP:
		doCreateGroup();
		break;

	case WIDGET_DELETE_GROUP:
		doDeleteGroup();
		break;

	case WIDGET_TRAIN:
		doTrain(widgetDataStruct);
		break;

	case WIDGET_CONSTRUCT:
		doConstruct(widgetDataStruct);
		break;

	case WIDGET_CREATE:
		doCreate(widgetDataStruct);
		break;

	case WIDGET_MAINTAIN:
		doMaintain(widgetDataStruct);
		break;

	case WIDGET_BUILD_LIST_LIST:
		doBuildListList(widgetDataStruct);
		break;

	case WIDGET_BUILD_LIST_TRAIN:
		doBuildListOrder(widgetDataStruct, ORDER_TRAIN);
		break;

	case WIDGET_BUILD_LIST_CONSTRUCT:
		doBuildListOrder(widgetDataStruct, ORDER_CONSTRUCT);
		break;

	case WIDGET_BUILD_LIST_CREATE:
		doBuildListOrder(widgetDataStruct, ORDER_CREATE);
		break;

	case WIDGET_BUILD_LIST_QUEUE:
		doBuildListQueue(widgetDataStruct);
		break;

	case WIDGET_BUILD_LIST:
		doBuildList(widgetDataStruct);
		break;

	case WIDGET_HURRY:
		doHurry(widgetDataStruct);
		break;

	case WIDGET_MENU_ICON:
		doMenu();

	case WIDGET_CONSCRIPT:
		doConscript();
		break;

	case WIDGET_ACTION:
		doAction(widgetDataStruct);
		break;

	case WIDGET_CITIZEN:
	case WIDGET_DISABLED_CITIZEN:
		doEmphasizeSpecialist(widgetDataStruct);
		break;

	case WIDGET_FREE_CITIZEN:
		break;

	case WIDGET_ANGRY_CITIZEN:
		break;

	case WIDGET_CHANGE_SPECIALIST:
		doChangeSpecialist(widgetDataStruct);
		break;

	case WIDGET_RESEARCH:
	case WIDGET_TECH_TREE:
		doResearch(widgetDataStruct);
		break;

	case WIDGET_CHANGE_PERCENT:
		doChangePercent(widgetDataStruct);
		break;

// BUG - Min/Max Commerce Rate - start
	case WIDGET_SET_PERCENT:
		doSetPercent(widgetDataStruct);
		break;
// BUG - Min/Max Commerce Rate - end

	case WIDGET_CITY_TAB:
		doCityTab(widgetDataStruct);
		break;

	case WIDGET_CONTACT_CIV:
		doContactCiv(widgetDataStruct);
		break;

	case WIDGET_END_TURN:
		GC.getGameINLINE().doControl(CONTROL_FORCEENDTURN);
		break;

	case WIDGET_LAUNCH_VICTORY:
		doLaunch(widgetDataStruct);
		break;

	case WIDGET_CONVERT:
		doConvert(widgetDataStruct);
		break;

	case WIDGET_REVOLUTION:
		// handled in Python
		break;

	case WIDGET_AUTOMATE_CITIZENS:
		doAutomateCitizens();
		break;

	case WIDGET_AUTOMATE_PRODUCTION:
		doAutomateProduction();
		break;

	case WIDGET_EMPHASIZE:
		doEmphasize(widgetDataStruct);
		break;

	case WIDGET_BUILDING_FILTER:
		doBuildingFilter(widgetDataStruct);
		break;

	case WIDGET_BUILDING_GROUPING:
		doBuildingGrouping(widgetDataStruct);
		break;

	case WIDGET_BUILDING_SORT:
		doBuildingSort(widgetDataStruct);
		break;

	case WIDGET_UNIT_FILTER:
		doUnitFilter(widgetDataStruct);
		break;

	case WIDGET_UNIT_GROUPING:
		doUnitGrouping(widgetDataStruct);
		break;

	case WIDGET_UNIT_SORT:
		doUnitSort(widgetDataStruct);
		break;

	case WIDGET_DIPLOMACY_RESPONSE:
		// CLEANUP -- PD
//		GC.getDiplomacyScreen().handleClick(m_pWidget);
		break;

	case WIDGET_TRADE_ITEM:
		break;

	case WIDGET_UNIT_MODEL:
		doUnitModel();
		break;

	case WIDGET_FLAG:
		doFlag();
		break;

	case WIDGET_HELP_SELECTED:
		doSelected(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_UNIT:
		doPediaUnitJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_BUILDING:
		doPediaBuildingJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_TECH:
	case WIDGET_PEDIA_JUMP_TO_REQUIRED_TECH:
	case WIDGET_PEDIA_JUMP_TO_DERIVED_TECH:
		doPediaTechJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_BACK:
		doPediaBack();
		break;
	case WIDGET_PEDIA_FORWARD:
		doPediaForward();
		break;

	case WIDGET_PEDIA_JUMP_TO_BONUS:
		doPediaBonusJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_MAIN:
		doPediaMain(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_PROMOTION:
		doPediaPromotionJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_UNIT_COMBAT:
		doPediaUnitCombatJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_IMPROVEMENT:
		doPediaImprovementJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_ROUTE:
		doPediaRouteJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_CIVIC:
		doPediaCivicJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_CIV:
		doPediaCivilizationJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_LEADER:
		doPediaLeaderJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_SPECIALIST:
		doPediaSpecialistJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_PROJECT:
		doPediaProjectJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_RELIGION:
		doPediaReligionJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_CORPORATION:
		doPediaCorporationJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_TERRAIN:
		doPediaTerrainJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_FEATURE:
		doPediaFeatureJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_DESCRIPTION:
	case WIDGET_PEDIA_DESCRIPTION_NO_HELP:
		doPediaDescription(widgetDataStruct);
		break;

	case WIDGET_TURN_EVENT:
		doGotoTurnEvent(widgetDataStruct);
		break;

	case WIDGET_FOREIGN_ADVISOR:
		doForeignAdvisor(widgetDataStruct);
		break;

	case WIDGET_DEAL_KILL:
		doDealKill(widgetDataStruct);
		break;

	case WIDGET_MINIMAP_HIGHLIGHT:
		doRefreshMilitaryAdvisor(widgetDataStruct);
		break;

	case WIDGET_CHOOSE_EVENT:
		break;

	case WIDGET_ZOOM_CITY:
		break;

	case WIDGET_HELP_TECH_PREPREQ:
	case WIDGET_HELP_OBSOLETE:
	case WIDGET_HELP_OBSOLETE_BONUS:
	case WIDGET_HELP_OBSOLETE_SPECIAL:
	case WIDGET_HELP_MOVE_BONUS:
	case WIDGET_HELP_FREE_UNIT:
	case WIDGET_HELP_FEATURE_PRODUCTION:
	case WIDGET_HELP_WORKER_RATE:
	case WIDGET_HELP_TRADE_ROUTES:
	case WIDGET_HELP_HEALTH_RATE:
	case WIDGET_HELP_HAPPINESS_RATE:
	case WIDGET_HELP_FREE_TECH:
	case WIDGET_HELP_LOS_BONUS:
	case WIDGET_HELP_MAP_CENTER:
	case WIDGET_HELP_MAP_REVEAL:
	case WIDGET_HELP_MAP_TRADE:
	case WIDGET_HELP_TECH_TRADE:
	case WIDGET_HELP_GOLD_TRADE:
	case WIDGET_HELP_OPEN_BORDERS:
	case WIDGET_HELP_DEFENSIVE_PACT:
	case WIDGET_HELP_PERMANENT_ALLIANCE:
	case WIDGET_HELP_VASSAL_STATE:
	case WIDGET_HELP_BUILD_BRIDGE:
	case WIDGET_HELP_IRRIGATION:
	case WIDGET_HELP_IGNORE_IRRIGATION:
	case WIDGET_HELP_WATER_WORK:
	case WIDGET_HELP_IMPROVEMENT:
	case WIDGET_HELP_DOMAIN_EXTRA_MOVES:
	case WIDGET_HELP_ADJUST:
	case WIDGET_HELP_TERRAIN_TRADE:
	case WIDGET_HELP_SPECIAL_BUILDING:
	case WIDGET_HELP_YIELD_CHANGE:
	case WIDGET_HELP_BONUS_REVEAL:
	case WIDGET_HELP_CIVIC_REVEAL:
	case WIDGET_HELP_PROCESS_INFO:
	case WIDGET_HELP_FINANCE_NUM_UNITS:
	case WIDGET_HELP_FINANCE_UNIT_COST:
	case WIDGET_HELP_FINANCE_AWAY_SUPPLY:
	case WIDGET_HELP_FINANCE_CITY_MAINT:
	case WIDGET_HELP_FINANCE_CIVIC_UPKEEP:
	case WIDGET_HELP_FINANCE_FOREIGN_INCOME:
	case WIDGET_HELP_FINANCE_INFLATED_COSTS:
	case WIDGET_HELP_FINANCE_GROSS_INCOME:
	case WIDGET_HELP_FINANCE_NET_GOLD:
	case WIDGET_HELP_FINANCE_GOLD_RESERVE:
	case WIDGET_HELP_RELIGION_CITY:
	case WIDGET_HELP_CORPORATION_CITY:
	case WIDGET_HELP_PROMOTION:
	case WIDGET_LEADERHEAD:
	case WIDGET_LEADER_LINE:
	case WIDGET_SCORE_BREAKDOWN:
	case WIDGET_HELP_CITY_CRIME:
/************************************************************************************************/
/* Afforess	                  Start		 6/19/11                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	case WIDGET_OBSOLETE_PROMOTION:
	case WIDGET_OBSOLETE_CORPORATION:
	case WIDGET_HELP_EMPLOYMENT:
	case WIDGET_HELP_IMPROVEMENT_CAN_UPGRADE:
	case WIDGET_EXTRA_TECH_STRENGTH:
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		//	Nothing on clicked
		break;
	case WIDGET_CLOSE_SCREEN:
		if ( GC.getCurrentViewport()->getTransformType() == VIEWPORT_TRANSFORM_TYPE_SCALE )
		{
			GC.getCurrentViewport()->closeAdvisor(-1,-1,-1,-1,-1);
		}
		GC.setIsInPedia(false);
		break;

	//ls612: City Goto Helper
	case WIDGET_CITY_GOTO:
		doGoToCity(widgetDataStruct);
		break;
	}

	return bHandled;
}

//	right clicking action
bool CvDLLWidgetData::executeAltAction( CvWidgetDataStruct &widgetDataStruct )
{
	CvWidgetDataStruct widgetData = widgetDataStruct;

	bool bHandled = true;
	switch (widgetDataStruct.m_eWidgetType)
	{
	case WIDGET_HELP_TECH_ENTRY:
	case WIDGET_HELP_TECH_PREPREQ:
	case WIDGET_RESEARCH:
	case WIDGET_TECH_TREE:
		doPediaTechJump(widgetDataStruct);
		break;
	case WIDGET_TRAIN:
		doPediaTrainJump(widgetDataStruct);
		break;
	case WIDGET_CONSTRUCT:
		doPediaConstructJump(widgetDataStruct);
		break;
	case WIDGET_CREATE:
		doPediaProjectJump(widgetDataStruct);
		break;
	case WIDGET_PEDIA_JUMP_TO_UNIT:
	case WIDGET_HELP_FREE_UNIT:
	case WIDGET_BUILD_LIST_TRAIN:
		doPediaUnitJump(widgetDataStruct);
		break;
	case WIDGET_HELP_FOUND_RELIGION:
		widgetData.m_iData1 = widgetData.m_iData2;
		//	Intentional fallthrough...
	case WIDGET_PEDIA_JUMP_TO_RELIGION:
		doPediaReligionJump(widgetData);
		break;
	case WIDGET_HELP_FOUND_CORPORATION:
		widgetData.m_iData1 = widgetData.m_iData2;
		//	Intentional fallthrough...
	case WIDGET_PEDIA_JUMP_TO_CORPORATION:
/************************************************************************************************/
/* Afforess	                  Start		 01/18/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	case WIDGET_OBSOLETE_CORPORATION:
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		doPediaCorporationJump(widgetData);
		break;
	case WIDGET_PEDIA_JUMP_TO_BUILDING:
	case WIDGET_BUILD_LIST_CONSTRUCT:
		doPediaBuildingJump(widgetDataStruct);
		break;
	case WIDGET_PEDIA_JUMP_TO_PROMOTION:
/************************************************************************************************/
/* Afforess	                  Start		 01/18/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	case WIDGET_OBSOLETE_PROMOTION:
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		doPediaPromotionJump(widgetDataStruct);
		break;
	case WIDGET_HELP_OBSOLETE:
		doPediaBuildingJump(widgetDataStruct);
		break;
	case WIDGET_HELP_IMPROVEMENT:
		doPediaBuildJump(widgetDataStruct);
		break;
	case WIDGET_HELP_YIELD_CHANGE:
		doPediaImprovementJump(widgetDataStruct, true);
		break;
	case WIDGET_HELP_BONUS_REVEAL:
	case WIDGET_HELP_OBSOLETE_BONUS:
		doPediaBonusJump(widgetDataStruct, true);
		break;
	case WIDGET_CITIZEN:
	case WIDGET_FREE_CITIZEN:
	case WIDGET_DISABLED_CITIZEN:
		doPediaSpecialistJump(widgetDataStruct);
		break;
	case WIDGET_PEDIA_JUMP_TO_PROJECT:
	case WIDGET_BUILD_LIST_CREATE:
		doPediaProjectJump(widgetDataStruct);
		break;
	case WIDGET_HELP_CIVIC_REVEAL:
		widgetData.m_iData1 = widgetData.m_iData2;
		doPediaCivicJump(widgetData);
		break;
	case WIDGET_LEADERHEAD:
		doContactCiv(widgetDataStruct);
		break;

// BUG - Leaderhead Relations - start
	case WIDGET_LEADERHEAD_RELATIONS:
		doContactCiv(widgetDataStruct);
		break;
// BUG - Leaderhead Relations - end

	default:
		bHandled = false;
		break;
	}

	return (bHandled);
}

bool CvDLLWidgetData::isLink(const CvWidgetDataStruct &widgetDataStruct) const
{
	bool bLink = false;
	switch (widgetDataStruct.m_eWidgetType)
	{
	case WIDGET_PEDIA_JUMP_TO_TECH:
	case WIDGET_PEDIA_JUMP_TO_REQUIRED_TECH:
	case WIDGET_PEDIA_JUMP_TO_DERIVED_TECH:
	case WIDGET_PEDIA_JUMP_TO_BUILDING:
	case WIDGET_PEDIA_JUMP_TO_UNIT:
	case WIDGET_PEDIA_JUMP_TO_UNIT_COMBAT:
	case WIDGET_PEDIA_JUMP_TO_PROMOTION:
	case WIDGET_PEDIA_JUMP_TO_BONUS:
	case WIDGET_PEDIA_JUMP_TO_IMPROVEMENT:
	case WIDGET_PEDIA_JUMP_TO_ROUTE:
	case WIDGET_PEDIA_JUMP_TO_CIVIC:
	case WIDGET_PEDIA_JUMP_TO_CIV:
	case WIDGET_PEDIA_JUMP_TO_LEADER:
	case WIDGET_PEDIA_JUMP_TO_SPECIALIST:
	case WIDGET_PEDIA_JUMP_TO_PROJECT:
	case WIDGET_PEDIA_JUMP_TO_RELIGION:
	case WIDGET_PEDIA_JUMP_TO_CORPORATION:
	case WIDGET_PEDIA_JUMP_TO_TERRAIN:
	case WIDGET_PEDIA_JUMP_TO_FEATURE:
	case WIDGET_PEDIA_FORWARD:
	case WIDGET_PEDIA_BACK:
	case WIDGET_PEDIA_MAIN:
	case WIDGET_TURN_EVENT:
	case WIDGET_FOREIGN_ADVISOR:
	case WIDGET_PEDIA_DESCRIPTION:
	case WIDGET_PEDIA_DESCRIPTION_NO_HELP:
	case WIDGET_MINIMAP_HIGHLIGHT:
		bLink = (widgetDataStruct.m_iData1 >= 0);
		break;
	case WIDGET_DEAL_KILL:
		{
			CvDeal* pDeal = GC.getGameINLINE().getDeal(widgetDataStruct.m_iData1);
			bLink = (NULL != pDeal && pDeal->isCancelable(GC.getGameINLINE().getActivePlayer()));
		}
		break;
	case WIDGET_CONVERT:
		bLink = (0 != widgetDataStruct.m_iData2);
		break;
	case WIDGET_GENERAL:
	case WIDGET_REVOLUTION:
		bLink = (1 == widgetDataStruct.m_iData1);
		break;
	}
	return (bLink);
}


void CvDLLWidgetData::doPlotList(CvWidgetDataStruct &widgetDataStruct)
{
	PROFILE_FUNC();

	CvUnit* pUnit;
	bool bWasCityScreenUp;

	int iUnitIndex = widgetDataStruct.m_iData1 + gDLL->getInterfaceIFace()->getPlotListColumn() - gDLL->getInterfaceIFace()->getPlotListOffset();

	CvPlot *selectionPlot = gDLL->getInterfaceIFace()->getSelectionPlot();
	pUnit = gDLL->getInterfaceIFace()->getInterfacePlotUnit(selectionPlot, iUnitIndex);

	if (pUnit != NULL)
	{
		if (pUnit->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
		{
			bWasCityScreenUp = gDLL->getInterfaceIFace()->isCityScreenUp();

			gDLL->getInterfaceIFace()->selectGroup(pUnit, gDLL->shiftKey(), gDLL->ctrlKey(), gDLL->altKey());

			if (bWasCityScreenUp)
			{
				gDLL->getInterfaceIFace()->lookAtSelectionPlot();
			}
		}
	}
}


void CvDLLWidgetData::doLiberateCity()
{
	GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_LIBERATE, 0);

	gDLL->getInterfaceIFace()->clearSelectedCities();
}


void CvDLLWidgetData::doRenameCity()
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (pHeadSelectedCity->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
		{
			CvEventReporter::getInstance().cityRename(pHeadSelectedCity);
		}
	}
}


void CvDLLWidgetData::doRenameUnit()
{
	CvUnit* pHeadSelectedUnit;

	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (pHeadSelectedUnit != NULL)
	{
		if (pHeadSelectedUnit->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
		{
			CvEventReporter::getInstance().unitRename(pHeadSelectedUnit);
		}
	}
}


void CvDLLWidgetData::doCreateGroup()
{
	GC.getGameINLINE().selectionListGameNetMessage(GAMEMESSAGE_JOIN_GROUP);
}


void CvDLLWidgetData::doDeleteGroup()
{
	GC.getGameINLINE().selectionListGameNetMessage(GAMEMESSAGE_JOIN_GROUP, -1, -1, -1, 0, false, true);
}


void CvDLLWidgetData::doTrain(CvWidgetDataStruct &widgetDataStruct)
{
	UnitTypes eUnit;
	CvCity* pCity;
/************************************************************************************************/
/* Afforess	                  Start		 01/12/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/

	//Ass: Modified by Kael 07/02/2008
//	eUnit = ((UnitTypes)(GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationUnits(widgetDataStruct.m_iData1)));
//	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
//	{
//		gDLL->sendPushOrder(widgetDataStruct.m_iData2, ORDER_TRAIN, eUnit, false, false, false);
//	}
//	else
//	{
//		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_PUSH_ORDER, ORDER_TRAIN, eUnit, -1, false, gDLL->altKey(), gDLL->shiftKey(), gDLL->ctrlKey());
//	}

	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		pCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(widgetDataStruct.m_iData2);
		eUnit = ((UnitTypes)(GC.getCivilizationInfo(pCity->getCivilizationType()).getCivilizationUnits(widgetDataStruct.m_iData1)));
		// Train Units Forever - start
		if ( eUnit != NO_UNIT )
		{
			bool bAlt;
			CvUnitInfo& kUnit = GC.getUnitInfo(eUnit);

			if (kUnit.getCombat() || kUnit.getAirCombat())
			{
				bAlt = getBugOptionBOOL("CityScreen__ProductionPopupTrainMilitaryUnitsForever", true, "BUG_PRODUCTION_POPUP_TRAIN_MILITARY_UNITS_FOREVER");
			}
			else
			{
				bAlt = getBugOptionBOOL("CityScreen__ProductionPopupTrainCivilianUnitsForever", false, "BUG_PRODUCTION_POPUP_TRAIN_CIVILIAN_UNITS_FOREVER");
			}
			CvMessageControl::getInstance().sendPushOrder(widgetDataStruct.m_iData2, ORDER_TRAIN, eUnit, bAlt, false, false);
		}
// Train Units Forever - end
	}
	else
	{
		bool bShift = gDLL->shiftKey();
		if (getBugOptionBOOL("MainInterface__InverseShiftForQueueing", false) )
		{
			bShift = !bShift;
		}
		pCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(gDLL->getInterfaceIFace()->headSelectedCitiesNode()->m_data.iID);
		eUnit = ((UnitTypes)(GC.getCivilizationInfo(pCity->getCivilizationType()).getCivilizationUnits(widgetDataStruct.m_iData1)));
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_PUSH_ORDER, ORDER_TRAIN, eUnit, -1, false, gDLL->altKey(), bShift, gDLL->ctrlKey());
	}
	gDLL->getInterfaceIFace()->setCityTabSelectionRow(CITYTAB_UNITS);
	pCity->setUnitListSelected(eUnit);

/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/


}


void CvDLLWidgetData::doConstruct(CvWidgetDataStruct &widgetDataStruct)
{
	BuildingTypes eBuilding;

/************************************************************************************************/
/* Afforess	                  Start		 01/12/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
//Ass: Modified by Kael 07/02/2008
//	eBuilding = ((BuildingTypes)(GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationBuildings(widgetDataStruct.m_iData1)));
//	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
//	{
//		gDLL->sendPushOrder(widgetDataStruct.m_iData2, ORDER_CONSTRUCT, eBuilding, false, false, false);
//	}
//	else
//	{
//		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_PUSH_ORDER, ORDER_CONSTRUCT, eBuilding, -1, false, gDLL->altKey(), gDLL->shiftKey(), gDLL->ctrlKey());
//	}
	CvCity* pCity;
	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		pCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(widgetDataStruct.m_iData2);
		eBuilding = ((BuildingTypes)(GC.getCivilizationInfo(pCity->getCivilizationType()).getCivilizationBuildings(widgetDataStruct.m_iData1)));
		CvMessageControl::getInstance().sendPushOrder(widgetDataStruct.m_iData2, ORDER_CONSTRUCT, eBuilding, false, false, false);
	}
	else
	{
		bool bShift = gDLL->shiftKey();
		if (getBugOptionBOOL("MainInterface__InverseShiftForQueueing", false) )
		{
			bShift = !bShift;
		}
		pCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(gDLL->getInterfaceIFace()->headSelectedCitiesNode()->m_data.iID);
		eBuilding = ((BuildingTypes)(GC.getCivilizationInfo(pCity->getCivilizationType()).getCivilizationBuildings(widgetDataStruct.m_iData1)));
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_PUSH_ORDER, ORDER_CONSTRUCT, eBuilding, -1, false, gDLL->altKey(), bShift, gDLL->ctrlKey());
	}
	
	if (isLimitedWonderClass((BuildingClassTypes)(widgetDataStruct.m_iData1)))
	{
		gDLL->getInterfaceIFace()->setCityTabSelectionRow(CITYTAB_WONDERS);
		pCity->setBuildingListSelectedWonder(eBuilding);
	}
	else
	{
		gDLL->getInterfaceIFace()->setCityTabSelectionRow(CITYTAB_BUILDINGS);
		pCity->setBuildingListSelectedBuilding(eBuilding);
	}
	
//Ass: End Modify
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
}


void CvDLLWidgetData::doCreate(CvWidgetDataStruct &widgetDataStruct)
{
	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		CvMessageControl::getInstance().sendPushOrder(widgetDataStruct.m_iData2, ORDER_CREATE, widgetDataStruct.m_iData1, false, false, false);
	}
	else
	{
		bool bShift = gDLL->shiftKey();
		if (getBugOptionBOOL("MainInterface__InverseShiftForQueueing", false) )
		{
			bShift = !bShift;
		}
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_PUSH_ORDER, ORDER_CREATE, widgetDataStruct.m_iData1, -1, false, gDLL->altKey(), bShift, gDLL->ctrlKey());
	}

	gDLL->getInterfaceIFace()->setCityTabSelectionRow(CITYTAB_WONDERS);
}


void CvDLLWidgetData::doMaintain(CvWidgetDataStruct &widgetDataStruct)
{
	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		CvMessageControl::getInstance().sendPushOrder(widgetDataStruct.m_iData2, ORDER_MAINTAIN, widgetDataStruct.m_iData1, false, false, false);
	}
	else
	{
		bool bShift = gDLL->shiftKey();
		if (getBugOptionBOOL("MainInterface__InverseShiftForQueueing", false) )
		{
			bShift = !bShift;
		}
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_PUSH_ORDER, ORDER_MAINTAIN, widgetDataStruct.m_iData1, -1, false, gDLL->altKey(), bShift, gDLL->ctrlKey());
	}

	gDLL->getInterfaceIFace()->setCityTabSelectionRow(CITYTAB_WONDERS);
}


void CvDLLWidgetData::doBuildListList(CvWidgetDataStruct &widgetDataStruct)
{
	bool bShift = gDLL->shiftKey();
	if (getBugOptionBOOL("MainInterface__InverseShiftForQueueing", false) )
	{
		bShift = !bShift;
	}
	GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_PUSH_ORDER, ORDER_LIST, widgetDataStruct.m_iData1, -1, false, gDLL->altKey(), bShift, gDLL->ctrlKey());
}


void CvDLLWidgetData::doBuildListOrder(CvWidgetDataStruct &widgetDataStruct, OrderTypes eOrder)
{
	bool bShift = gDLL->shiftKey();
	if (getBugOptionBOOL("MainInterface__InverseShiftForQueueing", false) )
	{
		bShift = !bShift;
	}
	CvMessageControl::getInstance().sendBuildListPushOrder(widgetDataStruct.m_iData2, eOrder, widgetDataStruct.m_iData1, gDLL->altKey(), bShift, gDLL->ctrlKey());
}


void CvDLLWidgetData::doHurry(CvWidgetDataStruct &widgetDataStruct)
{
	GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_HURRY, widgetDataStruct.m_iData1);
}


void CvDLLWidgetData::doConscript()
{
	GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_CONSCRIPT);
}


void CvDLLWidgetData::doAction(CvWidgetDataStruct &widgetDataStruct)
{
	GC.getGameINLINE().handleAction(widgetDataStruct.m_iData1);
}


void CvDLLWidgetData::doChangeSpecialist(CvWidgetDataStruct &widgetDataStruct)
{
	GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_CHANGE_SPECIALIST, widgetDataStruct.m_iData1, widgetDataStruct.m_iData2);
}


void CvDLLWidgetData::doResearch(CvWidgetDataStruct &widgetDataStruct)
{
	bool bShift;

	bShift = gDLL->shiftKey();

	if (!bShift)
	{
		if ((GetKeyState(VK_LSHIFT) & 0x8000) || (GetKeyState(VK_RSHIFT) & 0x8000))
		{
			bShift = true;
		}
	}

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       12/07/09                             EmperorFool      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
	// Free Tech Popup Fix
	if (widgetDataStruct.m_iData2 > 0)
	{
		CvPlayer& kPlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());

		if (!kPlayer.isChoosingFreeTech())
		{
			MEMORY_TRACK_EXEMPT();

			AddDLLMessage(GC.getGameINLINE().getActivePlayer(), true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_CHEATERS_NEVER_PROSPER"), NULL, MESSAGE_TYPE_MAJOR_EVENT);
			return;
		}
		else
		{
			kPlayer.setChoosingFreeTech(false);
		}
	}
	if (widgetDataStruct.m_iData1 > 0)
	{
	if (!GET_PLAYER(GC.getGameINLINE().getActivePlayer()).hasValidBuildings((TechTypes)widgetDataStruct.m_iData1))
		return;
	}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/

	CvMessageControl::getInstance().sendResearch(((TechTypes)widgetDataStruct.m_iData1), widgetDataStruct.m_iData2, bShift);
}


void CvDLLWidgetData::doChangePercent(CvWidgetDataStruct &widgetDataStruct)
{
	CvMessageControl::getInstance().sendPercentChange(((CommerceTypes)widgetDataStruct.m_iData1), widgetDataStruct.m_iData2);
}

// BUG - Min/Max Commerce Rate - start
void CvDLLWidgetData::doSetPercent(CvWidgetDataStruct &widgetDataStruct)
{
	CvMessageControl::getInstance().sendPercentChange(((CommerceTypes)widgetDataStruct.m_iData1), widgetDataStruct.m_iData2 - GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCommercePercent((CommerceTypes)widgetDataStruct.m_iData1));
}
// BUG - Min/Max Commerce Rate - end

void CvDLLWidgetData::doCityTab(CvWidgetDataStruct &widgetDataStruct)
{
	gDLL->getInterfaceIFace()->setCityTabSelectionRow((CityTabTypes)widgetDataStruct.m_iData1);
}

void CvDLLWidgetData::doContactCiv(CvWidgetDataStruct &widgetDataStruct)
{
	if (gDLL->isDiplomacy() || gDLL->isMPDiplomacyScreenUp())
	{
		return;
	}

	//	Do not execute this if we are trying to contact ourselves...
	if (GC.getGameINLINE().getActivePlayer() == widgetDataStruct.m_iData1)
	{
		if (!gDLL->getInterfaceIFace()->isFocusedWidget())
		{
			gDLL->getInterfaceIFace()->toggleScoresMinimized();
		}

		return;
	}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      01/11/09                                jdog5000      */
/*                                                                                              */
/* Player Interface                                                                             */
/************************************************************************************************/
	if (gDLL->shiftKey() && !gDLL->altKey())
	{
		if (GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).isHuman())
		{
			if (widgetDataStruct.m_iData1 != GC.getGameINLINE().getActivePlayer())
			{
				gDLL->getInterfaceIFace()->showTurnLog((ChatTargetTypes)widgetDataStruct.m_iData1);
			}
		}
		return;
	}

	if (gDLL->altKey())
	{
		if( gDLL->shiftKey() )
		{
			// Warning: use of this is not multiplayer compatible
			if (GET_TEAM(GC.getGameINLINE().getActiveTeam()).canDeclareWar(GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getTeam()))
			{
				if( GET_TEAM(GC.getGameINLINE().getActiveTeam()).AI_getWarPlan(GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getTeam()) == WARPLAN_PREPARING_TOTAL) 
				{
					GET_TEAM(GC.getGameINLINE().getActiveTeam()).AI_setWarPlan(GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getTeam(), NO_WARPLAN);
				}
				else
				{
					GET_TEAM(GC.getGameINLINE().getActiveTeam()).AI_setWarPlan(GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getTeam(), WARPLAN_PREPARING_TOTAL);
				}
				gDLL->getInterfaceIFace()->setDirty(Score_DIRTY_BIT, true);
			}
		}
		else
		{
			if (GET_TEAM(GC.getGameINLINE().getActiveTeam()).canDeclareWar(GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getTeam()))
			{
				CvMessageControl::getInstance().sendChangeWar(GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getTeam(), true);
			}
			else if (GET_TEAM(GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getTeam()).isVassal(GC.getGameINLINE().getActiveTeam()))
			{
				CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_VASSAL_DEMAND_TRIBUTE, widgetDataStruct.m_iData1);
				if (pInfo)
				{
					gDLL->getInterfaceIFace()->addPopup(pInfo, GC.getGameINLINE().getActivePlayer(), true);
				}
			}
		}
		return;
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	GET_PLAYER(GC.getGameINLINE().getActivePlayer()).contact((PlayerTypes)widgetDataStruct.m_iData1);
}

void CvDLLWidgetData::doConvert(CvWidgetDataStruct &widgetDataStruct)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		CvMessageControl::getInstance().sendConvert((ReligionTypes)(widgetDataStruct.m_iData1));
	}
}

void CvDLLWidgetData::doAutomateCitizens()
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_SET_AUTOMATED_CITIZENS, -1, -1, !(pHeadSelectedCity->isCitizensAutomated()));
	}
}

void CvDLLWidgetData::doAutomateProduction()
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_SET_AUTOMATED_PRODUCTION, -1, -1, !pHeadSelectedCity->isProductionAutomated(), gDLL->altKey(), gDLL->shiftKey(), gDLL->ctrlKey());
	}
}

void CvDLLWidgetData::doEmphasize(CvWidgetDataStruct &widgetDataStruct)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_SET_EMPHASIZE, widgetDataStruct.m_iData1, -1, !(pHeadSelectedCity->AI_isEmphasize((EmphasizeTypes)(widgetDataStruct.m_iData1))));
	}
}

void CvDLLWidgetData::doEmphasizeSpecialist(CvWidgetDataStruct &widgetDataStruct)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_EMPHASIZE_SPECIALIST, widgetDataStruct.m_iData1, -1, !(pHeadSelectedCity->AI_isEmphasizeSpecialist((SpecialistTypes)(widgetDataStruct.m_iData1))));
	}
}

void CvDLLWidgetData::doBuildingFilter(CvWidgetDataStruct &widgetDataStruct)
{
	BuildingFilterTypes eFilter = (BuildingFilterTypes)(widgetDataStruct.m_iData1);
	if (widgetDataStruct.m_iData2 == -1)
	{
		CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

		if (pHeadSelectedCity != NULL)
		{
			bool bActive = pHeadSelectedCity->getBuildingListFilterActive(eFilter);
			pHeadSelectedCity->setBuildingListFilterActive(eFilter, !bActive);
			//gDLL->getInterfaceIFace()->setCityTabSelectionRow(CITYTAB_BUILDINGS);
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		}
	}
	else
	{
		bool bActive = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getBuildingListFilterActive(eFilter);
		GET_PLAYER(GC.getGameINLINE().getActivePlayer()).setBuildingListFilterActive(eFilter, !bActive);
		PYTHON_CALL_FUNCTION2(__FUNCTION__, "CvScreensInterface", "showBuildListScreen");
	}
}

void CvDLLWidgetData::doGoToCity(CvWidgetDataStruct &widgetDataStruct)
{
	CvCity* pCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(widgetDataStruct.m_iData1);
	CvPlot* pPlot = pCity->plot();
	
	GC.getGameINLINE().selectionListMove(pPlot, false, false, false);
}	

// This is not triggered
void CvDLLWidgetData::doBuildingGrouping(CvWidgetDataStruct &widgetDataStruct)
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		BuildingGroupingTypes eGrouping = (BuildingGroupingTypes)(widgetDataStruct.m_iData1);
		pHeadSelectedCity->setBuildingListGrouping(eGrouping);
		gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
	}
}

// This is not triggered
void CvDLLWidgetData::doBuildingSort(CvWidgetDataStruct &widgetDataStruct)
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		BuildingSortTypes eSort = (BuildingSortTypes)(widgetDataStruct.m_iData1);
		pHeadSelectedCity->setBuildingListSorting(eSort);
		gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
	}
}

void CvDLLWidgetData::doUnitFilter(CvWidgetDataStruct &widgetDataStruct)
{
	UnitFilterTypes eFilter = (UnitFilterTypes)(widgetDataStruct.m_iData1);

	if (widgetDataStruct.m_iData2 == -1)
	{
		CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

		if (pHeadSelectedCity != NULL)
		{
			bool bActive = pHeadSelectedCity->getUnitListFilterActive(eFilter);
			pHeadSelectedCity->setUnitListFilterActive(eFilter, !bActive);
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		}
	}
	else
	{
		bool bActive = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getUnitListFilterActive(eFilter);
		GET_PLAYER(GC.getGameINLINE().getActivePlayer()).setUnitListFilterActive(eFilter, !bActive);
		PYTHON_CALL_FUNCTION2(__FUNCTION__, "CvScreensInterface", "showBuildListScreen");
	}
}

// This is not triggered
void CvDLLWidgetData::doUnitGrouping(CvWidgetDataStruct &widgetDataStruct)
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		UnitGroupingTypes eGrouping = (UnitGroupingTypes)(widgetDataStruct.m_iData1);
		pHeadSelectedCity->setUnitListGrouping(eGrouping);
		gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
	}
}

// This is not triggered
void CvDLLWidgetData::doUnitSort(CvWidgetDataStruct &widgetDataStruct)
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		UnitSortTypes eSort = (UnitSortTypes)(widgetDataStruct.m_iData1);
		pHeadSelectedCity->setUnitListSorting(eSort);
		gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
	}
}

void CvDLLWidgetData::doUnitModel()
{
	if (gDLL->getInterfaceIFace()->isFocused())
	{
		//	Do NOT execute if a screen is up...
		return;
	}

	gDLL->getInterfaceIFace()->lookAtSelectionPlot();
}


void CvDLLWidgetData::doFlag()
{
	GC.getGameINLINE().doControl(CONTROL_SELECTCAPITAL);
}

void CvDLLWidgetData::doSelected(CvWidgetDataStruct &widgetDataStruct)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_POP_ORDER, widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::doBuildListQueue(CvWidgetDataStruct &widgetDataStruct)
{
	CvMessageControl::getInstance().sendBuildListPopOrder(widgetDataStruct.m_iData1, widgetDataStruct.m_iData2);
}


void CvDLLWidgetData::doPediaTechJump(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToTech", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaUnitJump(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;

	argsList.add(widgetDataStruct.m_iData1);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToUnit", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaBuildingJump(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToBuilding", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaProjectJump(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToProject", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaReligionJump(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToReligion", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaCorporationJump(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToCorporation", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaTerrainJump(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToTerrain", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaFeatureJump(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToFeature", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaTrainJump(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationUnits(widgetDataStruct.m_iData1));
	
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToUnit", argsList.makeFunctionArgs());
}


void CvDLLWidgetData::doPediaConstructJump(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationBuildings(widgetDataStruct.m_iData1));

	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToBuilding", argsList.makeFunctionArgs());
}


void CvDLLWidgetData::doPediaBack()
{
	PYTHON_CALL_FUNCTION2(__FUNCTION__, PYScreensModule, "pediaBack");	
}

void CvDLLWidgetData::doPediaForward()
{
	PYTHON_CALL_FUNCTION2(__FUNCTION__, PYScreensModule, "pediaForward");	
}

void CvDLLWidgetData::doPediaBonusJump(CvWidgetDataStruct &widgetDataStruct, bool bData2)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	if (bData2)
	{
		argsList.add(widgetDataStruct.m_iData2);
	}
	else
	{
		argsList.add(widgetDataStruct.m_iData1);
	}
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToBonus", argsList.makeFunctionArgs());	
}

void CvDLLWidgetData::doPediaSpecialistJump(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToSpecialist", argsList.makeFunctionArgs());	
}

void CvDLLWidgetData::doPediaMain(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1 < 0 ? 0 : widgetDataStruct.m_iData1);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaMain", argsList.makeFunctionArgs());	
}

void CvDLLWidgetData::doPediaPromotionJump(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToPromotion", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaUnitCombatJump(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToUnitChart", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaImprovementJump(CvWidgetDataStruct &widgetDataStruct, bool bData2)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	if (bData2)
	{
		argsList.add(widgetDataStruct.m_iData2);
	}
	else
	{
		argsList.add(widgetDataStruct.m_iData1);
	}
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToImprovement", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaRouteJump(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToRoute", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaCivicJump(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToCivic", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaCivilizationJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToCiv", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaLeaderJump(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToLeader", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaDescription(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	argsList.add(widgetDataStruct.m_iData2);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaShowHistorical", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaBuildJump(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;

	ImprovementTypes eImprovement = NO_IMPROVEMENT;
	BuildTypes eBuild = (BuildTypes)widgetDataStruct.m_iData2;
	if (NO_BUILD != eBuild)
	{
		eImprovement = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();
	}

	if (NO_IMPROVEMENT != eImprovement)
	{
		argsList.add(eImprovement);
		PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "pediaJumpToImprovement", argsList.makeFunctionArgs());
	}
}

void CvDLLWidgetData::doGotoTurnEvent(CvWidgetDataStruct &widgetDataStruct)
{
	CvPlot* pPlot = GC.getMapINLINE().plotINLINE(widgetDataStruct.m_iData1, widgetDataStruct.m_iData2);

	if (NULL != pPlot && !gDLL->getEngineIFace()->isCameraLocked())
	{
		if (pPlot->isRevealed(GC.getGameINLINE().getActiveTeam(), false))
		{
			gDLL->getEngineIFace()->cameraLookAt(pPlot->getPoint());
		}
	}
}

void CvDLLWidgetData::doMenu( void )
{
	if (!gDLL->isGameInitializing())
	{
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_MAIN_MENU,gDLL->ctrlKey());
		if (NULL != pInfo)
		{
			gDLL->getInterfaceIFace()->addPopup(pInfo, NO_PLAYER, true);
		}
	}
}

void CvDLLWidgetData::doLaunch(CvWidgetDataStruct &widgetDataStruct)
{
	if (GET_TEAM(GC.getGameINLINE().getActiveTeam()).canLaunch((VictoryTypes)widgetDataStruct.m_iData1) && GC.getGameINLINE().testVictory((VictoryTypes)widgetDataStruct.m_iData1, GC.getGameINLINE().getActiveTeam()))
	{
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_LAUNCH, widgetDataStruct.m_iData1);
		if (NULL != pInfo)
		{
			gDLL->getInterfaceIFace()->addPopup(pInfo);
		}
	}
}

void CvDLLWidgetData::doForeignAdvisor(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "showForeignAdvisorScreen", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doBuildList(CvWidgetDataStruct &widgetDataStruct)
{
	GET_PLAYER(GC.getGameINLINE().getActivePlayer()).m_pBuildLists->setCurrentList(widgetDataStruct.m_iData1);
	PYTHON_CALL_FUNCTION2(__FUNCTION__, "CvScreensInterface", "showBuildListScreen");
}

//
//	HELP PARSING FUNCTIONS
//

void CvDLLWidgetData::parsePlotListHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	PROFILE_FUNC();

	CvUnit* pUnit;

	int iUnitIndex = widgetDataStruct.m_iData1 + gDLL->getInterfaceIFace()->getPlotListColumn() - gDLL->getInterfaceIFace()->getPlotListOffset();

	CvPlot *selectionPlot = gDLL->getInterfaceIFace()->getSelectionPlot();
	pUnit = gDLL->getInterfaceIFace()->getInterfacePlotUnit(selectionPlot, iUnitIndex);

	if (pUnit != NULL)
	{
		GAMETEXT.setUnitHelp(szBuffer, pUnit);

		if (pUnit->plot()->plotCount(PUF_isUnitType, pUnit->getUnitType(), -1, pUnit->getOwnerINLINE()) > 1)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_CTRL_SELECT", GC.getUnitInfo(pUnit->getUnitType()).getTextKeyWide()));
		}

		if (pUnit->plot()->plotCount(NULL, -1, -1, pUnit->getOwnerINLINE()) > 1)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_ALT_SELECT"));
		}
	}
}


void CvDLLWidgetData::parseLiberateCityHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	CvWString szTempBuffer;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		PlayerTypes ePlayer = pHeadSelectedCity->getLiberationPlayer(false);
		if (NO_PLAYER != ePlayer)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_LIBERATE_CITY_HELP", pHeadSelectedCity->getNameKey(), GET_PLAYER(ePlayer).getNameKey()));
		}
	}
}

void CvDLLWidgetData::parseCityNameHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	CvWString szTempBuffer;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		szBuffer.append(pHeadSelectedCity->getName());

		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_CITY_POPULATION", pHeadSelectedCity->getRealPopulation()));

		GAMETEXT.setTimeStr(szTempBuffer, pHeadSelectedCity->getGameTurnFounded(), false);
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_CITY_FOUNDED", szTempBuffer.GetCString()));

		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_CHANGE_NAME"));
	}
}



void CvDLLWidgetData::parseTrainHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	UnitTypes eUnit;

	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		pHeadSelectedCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(widgetDataStruct.m_iData2);
	}
	else
	{
		pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	}

	if (pHeadSelectedCity != NULL)
	{
		eUnit = (UnitTypes)GC.getCivilizationInfo(pHeadSelectedCity->getCivilizationType()).getCivilizationUnits(widgetDataStruct.m_iData1);

		GAMETEXT.setUnitHelp(szBuffer, eUnit, false, widgetDataStruct.m_bOption, false, pHeadSelectedCity);
	}
}


void CvDLLWidgetData::parseConstructHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	BuildingTypes eBuilding;

	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		pHeadSelectedCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(widgetDataStruct.m_iData2);
	}
	else
	{
		pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	}

	if (pHeadSelectedCity != NULL)
	{
		eBuilding = (BuildingTypes)GC.getCivilizationInfo(pHeadSelectedCity->getCivilizationType()).getCivilizationBuildings(widgetDataStruct.m_iData1);

// BUG - Building Actual Effects - start
		GAMETEXT.setBuildingHelpActual(szBuffer, eBuilding, false, widgetDataStruct.m_bOption, false, pHeadSelectedCity);
// BUG - Building Actual Effects - end
	}
}


void CvDLLWidgetData::parseCreateHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		pHeadSelectedCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(widgetDataStruct.m_iData2);
	}
	else
	{
		pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	}

	GAMETEXT.setProjectHelp(szBuffer, ((ProjectTypes)widgetDataStruct.m_iData1), false, pHeadSelectedCity);
}


void CvDLLWidgetData::parseMaintainHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.setProcessHelp(szBuffer, ((ProcessTypes)(widgetDataStruct.m_iData1)));
}


void CvDLLWidgetData::parseHurryHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	CvWString szTempBuffer;
	bool bFirst;
	int iHurryGold;
	int iHurryPopulation;
	int iHurryAngerLength;
	int iI;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		szBuffer.assign(gDLL->getText("TXT_KEY_MISC_HURRY_PROD", pHeadSelectedCity->getProductionNameKey()));

		iHurryGold = pHeadSelectedCity->hurryGold((HurryTypes)(widgetDataStruct.m_iData1));

		if (iHurryGold > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HURRY_GOLD", iHurryGold));
		}

		iHurryPopulation = pHeadSelectedCity->hurryPopulation((HurryTypes)(widgetDataStruct.m_iData1));

		if (iHurryPopulation > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HURRY_POP", iHurryPopulation));

			if (iHurryPopulation > pHeadSelectedCity->maxHurryPopulation())
			{
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_MAX_POP_HURRY", pHeadSelectedCity->maxHurryPopulation()));
			}
		}

// BUG - Hurry Overflow - start
		if (getBugOptionBOOL("MiscHover__HurryOverflow", true, "BUG_HURRY_OVERFLOW_HOVER"))
		{
			int iOverflowProduction = 0;
			int iOverflowGold = 0;
			if (pHeadSelectedCity->hurryOverflow((HurryTypes)(widgetDataStruct.m_iData1), &iOverflowProduction, &iOverflowGold, getBugOptionBOOL("MiscHover__HurryOverflowIncludeCurrent", false, "BUG_HURRY_OVERFLOW_HOVER_INCLUDE_CURRENT")))
			{
				if (iOverflowProduction > 0 || iOverflowGold > 0)
				{
					bFirst = true;
					CvWStringBuffer szOverflowBuffer;
					if (iOverflowProduction > 0)
					{
						szTempBuffer.Format(L"%d%c", iOverflowProduction, GC.getYieldInfo(YIELD_PRODUCTION).getChar());
						setListHelp(szOverflowBuffer, NULL, szTempBuffer, L", ", bFirst);
						bFirst = false;
					}
					if (iOverflowGold > 0)
					{
						szTempBuffer.Format(L"%d%c", iOverflowGold, GC.getCommerceInfo(COMMERCE_GOLD).getChar());
						setListHelp(szOverflowBuffer, NULL, szTempBuffer, L", ", bFirst);
						bFirst = false;
					}
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HURRY_OVERFLOW", szOverflowBuffer.getCString));
				}
			}
		}
// BUG - Hurry Overflow - end

		iHurryAngerLength = pHeadSelectedCity->hurryAngerLength((HurryTypes)(widgetDataStruct.m_iData1));

		if (iHurryAngerLength > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_ANGER_TURNS", GC.getDefineINT("HURRY_POP_ANGER"), (iHurryAngerLength + pHeadSelectedCity->getHurryAngerTimer())));
		}

		if (!(pHeadSelectedCity->isProductionUnit()) && !(pHeadSelectedCity->isProductionBuilding()))
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_UNIT_BUILDING_HURRY"));
		}

		if (!(GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).canHurry((HurryTypes)(widgetDataStruct.m_iData1))))
		{
			bFirst = true;

			for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
			{
				if (GC.getCivicInfo((CivicTypes)iI).isHurry(widgetDataStruct.m_iData1))
				{
					szTempBuffer = NEWLINE + gDLL->getText("TXT_KEY_REQUIRES");
					setListHelp(szBuffer, szTempBuffer, GC.getCivicInfo((CivicTypes)iI).getDescription(), gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
					bFirst = false;
				}
			}		

			for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
			{
				if (GC.getBuildingInfo((BuildingTypes)iI).isHurry(widgetDataStruct.m_iData1))
				{
					szTempBuffer = NEWLINE + gDLL->getText("TXT_KEY_REQUIRES");
					setListHelp(szBuffer, szTempBuffer, GC.getBuildingInfo((BuildingTypes)iI).getDescription(), gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
					bFirst = false;
				}
			}	

			if (!bFirst)
			{
				szBuffer.append(ENDCOLR);
			}
		}
	}
}


void CvDLLWidgetData::parseConscriptHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	CvWString szTempBuffer;
	int iConscriptPopulation;
	int iConscriptAngerLength;
	int iMinCityPopulation;
	int iMinCulturePercent;
	int iI;
	bool bFirst;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (pHeadSelectedCity->getConscriptUnit() != NO_UNIT)
		{
			CvWString szTemp;
			szTemp.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT"), GC.getUnitInfo(pHeadSelectedCity->getConscriptUnit()).getDescription());
			szBuffer.assign(szTemp);
		
// BUG - Starting Experience - start
			if (getBugOptionBOOL("MiscHover__ConscriptUnit", true, "BUG_CONSCRIPT_UNIT_HOVER"))
			{
				GAMETEXT.setBasicUnitHelpWithCity(szBuffer, pHeadSelectedCity->getConscriptUnit(), false, pHeadSelectedCity, true);
			}
// BUG - Starting Experience - end

			iConscriptPopulation = pHeadSelectedCity->getConscriptPopulation();

			if (iConscriptPopulation > 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_HURRY_POP", iConscriptPopulation));
			}

			iConscriptAngerLength = pHeadSelectedCity->flatConscriptAngerLength();

			if (iConscriptAngerLength > 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_ANGER_TURNS", GC.getDefineINT("CONSCRIPT_POP_ANGER"), (iConscriptAngerLength + pHeadSelectedCity->getConscriptAngerTimer())));
			}

			iMinCityPopulation = pHeadSelectedCity->conscriptMinCityPopulation();

			if (pHeadSelectedCity->getPopulation() < iMinCityPopulation)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_MIN_CITY_POP", iMinCityPopulation));
			}

			iMinCulturePercent = GC.getDefineINT("CONSCRIPT_MIN_CULTURE_PERCENT");

			if (pHeadSelectedCity->plot()->calculateTeamCulturePercent(pHeadSelectedCity->getTeam()) < iMinCulturePercent)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_MIN_CULTURE_PERCENT", iMinCulturePercent));
			}

			if (GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).getMaxConscript() == 0)
			{
				bFirst = true;

				for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
				{
					if (getWorldSizeMaxConscript((CivicTypes)iI) > 0)
					{
						szTempBuffer = NEWLINE + gDLL->getText("TXT_KEY_REQUIRES");
						setListHelp(szBuffer, szTempBuffer, GC.getCivicInfo((CivicTypes)iI).getDescription(), gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
						bFirst = false;
					}
				}		

				if (!bFirst)
				{
					szBuffer.append(ENDCOLR);
				}
			}
// BUG - Conscript Limit - start
			else if (getBugOptionBOOL("MiscHover__ConscriptLimit", true, "BUG_CONSCRIPT_LIMIT_HOVER"))
			{
				if (pHeadSelectedCity->isDrafted())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_CITY_HAS_CONSCRIPTED"));
				}
				else
				{
					int iDraftUnits = GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).getConscriptCount();
					int iMaxDraftUnits = GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).getMaxConscript();

					if (iDraftUnits >= iMaxDraftUnits)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_MISC_NO_CONSCRIPT_UNITS_LEFT"));
					}
					else
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_MISC_CONSCRIPT_UNITS_LEFT", iMaxDraftUnits - iDraftUnits));
					}
				}
			}
// BUG - Conscript Limit - end
		}
	}
}


void CvDLLWidgetData::parseActionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CLLNode<IDInfo>* pSelectedUnitNode;
	CvCity* pMissionCity;
	CvCity* pCity = NULL;
	CvUnit* pHeadSelectedUnit;
	CvUnit* pSelectedUnit;
	CvPlot* pMissionPlot;
	CvPlot* pLoopPlot;
	CvWString szTempBuffer;
	CvWString szFirstBuffer;
	ImprovementTypes eImprovement;
	ImprovementTypes eFinalImprovement;
	ReligionTypes eReligion;
	BuildingTypes eBuilding;
	PlayerTypes eGiftPlayer;
	BuildTypes eBuild;
	RouteTypes eRoute;
	BonusTypes eBonus;
	TechTypes eTech;
	bool bAlt;
	bool bShift;
	bool bValid;
	int iYield;
	int iUnitConsume;
	int iUnitDiff;
	int iProduction;
	int iMovementCost;
	int iFlatMovementCost;
	int iMoves;
	int iFlatMoves;
	int iNowWorkRate;
	int iThenWorkRate;
	int iTurns;
	int iPrice;
	int iLow;
	int iHigh;
	int iLast;
	int iRange;
	int iDX, iDY;
	int iI;

	bAlt = gDLL->altKey();
	bShift = gDLL->shiftKey();

	CvWString szTemp;
	szTemp.Format(SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getActionInfo(widgetDataStruct.m_iData1).getHotKeyDescription().c_str());
	szBuffer.assign(szTemp);

	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (pHeadSelectedUnit != NULL)
	{
		if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() != NO_MISSION)
		{
			if (bShift && gDLL->getInterfaceIFace()->mirrorsSelectionGroup())
			{
				pMissionPlot = pHeadSelectedUnit->getGroup()->lastMissionPlot();
			}
			else
			{
				pMissionPlot = pHeadSelectedUnit->plot();
			}

			pMissionCity = pMissionPlot->getPlotCity();
			
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
			if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_HEAL || GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_SENTRY_WHILE_HEAL)
#else
			if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_HEAL)
#endif
// BUG - Sentry Actions - end
			{
				iTurns = 0;

				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);
					iTurns = std::max(iTurns, pSelectedUnit->healTurns(pMissionPlot));

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}

				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_TURN_OR_TURNS", iTurns));
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_PILLAGE)
			{
				if (pMissionPlot->isCity())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_MISSION_PILLAGE_CITY"));
				}				
				else if (pMissionPlot->getImprovementType() != NO_IMPROVEMENT)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_DESTROY_IMP", GC.getImprovementInfo(pMissionPlot->getImprovementType()).getTextKeyWide()));
				}
				else if (pMissionPlot->getRouteType() != NO_ROUTE)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_DESTROY_IMP", GC.getRouteInfo(pMissionPlot->getRouteType()).getTextKeyWide()));
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_PLUNDER)
			{
				pMissionPlot = pHeadSelectedUnit->plot();

				if (pMissionPlot->getTeam() == pHeadSelectedUnit->getTeam())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_PLUNDER_IN_BORDERS"));
				}
			}

/************************************************************************************************/
/* Great Diplomat MOD               START                                      Stolenrays 		*/
/************************************************************************************************/
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_BRIBE_BARBARIAN)
			{
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					if (pSelectedUnit->canBribeBarbarian(pMissionPlot, true)) // XXX if queuing up this action, use the current plot along the goto...
					{
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_BRIBE_BARBARIAN"));
						}
						break;
					}

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}
			}

			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_GOODWILL)
			{
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					if (pSelectedUnit->canGoodwill(pMissionPlot, true)) // XXX if queuing up this action, use the current plot along the goto...
					{
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_GOODWILL"));
						}
						break;
					}

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}
			}
/************************************************************************************************/
/* Great Diplomat MOD               END                                                  		*/
/************************************************************************************************/
			
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_SABOTAGE)
			{
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					if (pSelectedUnit->canSabotage(pMissionPlot, true)) // XXX if queuing up this action, use the current plot along the goto...
					{
						iPrice = pSelectedUnit->sabotageCost(pMissionPlot);
						if (iPrice > 0)
						{
							szTempBuffer.Format(L"%d %c", iPrice, GC.getCommerceInfo(COMMERCE_GOLD).getChar());
							szBuffer.append(NEWLINE);
							szBuffer.append(szTempBuffer);
						}

						iLow = pSelectedUnit->sabotageProb(pMissionPlot, PROBABILITY_LOW);
						iHigh = pSelectedUnit->sabotageProb(pMissionPlot, PROBABILITY_HIGH);

						if (iLow == iHigh)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_PROBABILITY", iHigh));
						}
						else
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_PROBABILITY_RANGE", iLow, iHigh));
						}
						break;
					}

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_DESTROY)
			{
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					if (pSelectedUnit->canDestroy(pMissionPlot, true)) // XXX if queuing up this action, use the current plot along the goto...
					{
						iPrice = pSelectedUnit->destroyCost(pMissionPlot);
						if (iPrice > 0)
						{
							szTempBuffer.Format(L"%d %c", iPrice, GC.getCommerceInfo(COMMERCE_GOLD).getChar());
							szBuffer.append(NEWLINE);
							szBuffer.append(szTempBuffer);
						}

						iLow = pSelectedUnit->destroyProb(pMissionPlot, PROBABILITY_LOW);
						iHigh = pSelectedUnit->destroyProb(pMissionPlot, PROBABILITY_HIGH);

						if (iLow == iHigh)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_PROBABILITY", iHigh));
						}
						else
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_PROBABILITY_RANGE", iLow, iHigh));
						}
						break;
					}

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_STEAL_PLANS)
			{
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					if (pSelectedUnit->canStealPlans(pMissionPlot, true)) // XXX if queuing up this action, use the current plot along the goto...
					{
						iPrice = pSelectedUnit->stealPlansCost(pMissionPlot);
						if (iPrice > 0)
						{
							szTempBuffer.Format(L"%d %c", iPrice, GC.getCommerceInfo(COMMERCE_GOLD).getChar());
							szBuffer.append(NEWLINE);
							szBuffer.append(szTempBuffer);
						}

						iLow = pSelectedUnit->stealPlansProb(pMissionPlot, PROBABILITY_LOW);
						iHigh = pSelectedUnit->stealPlansProb(pMissionPlot, PROBABILITY_HIGH);

						if (iLow == iHigh)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_PROBABILITY", iHigh));
						}
						else
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_PROBABILITY_RANGE", iLow, iHigh));
						}
						break;
					}

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_FOUND)
			{
				if (!(GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).canFound(pMissionPlot->getX_INLINE(), pMissionPlot->getY_INLINE())))
				{
					bValid = true;

					iRange = GC.getMIN_CITY_RANGE();

					for (iDX = -(iRange); iDX <= iRange; iDX++)
					{
/************************************************************************************************/
/* Afforess	                  Start		 6/11/11                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
						if (!bValid) break;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
						for (iDY = -(iRange); iDY <= iRange; iDY++)
						{
							pLoopPlot	= plotXY(pMissionPlot->getX_INLINE(), pMissionPlot->getY_INLINE(), iDX, iDY);

							if (pLoopPlot != NULL)
							{
								if (pLoopPlot->isCity())
								{
									bValid = false;
/************************************************************************************************/
/* Afforess	                  Start		 6/11/11                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
									break;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
								}
							}
						}
					}

					if (!bValid)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CANNOT_FOUND", GC.getMIN_CITY_RANGE()));
					}
/************************************************************************************************/
/* Afforess	                  Start		 6/13/11                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
					if (GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getCityLimit() > 0 &&
						GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getCityOverLimitUnhappy() == 0 &&
						GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getNumCities() >= GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getCityLimit())
					{
						CvWString szCivics;
						for (iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
						{
							if (GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getCivics((CivicOptionTypes)iI) != NO_CIVIC)
							{
								if (GC.getCivicInfo((CivicTypes)GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getCivics((CivicOptionTypes)iI)).getCityLimit(pHeadSelectedUnit->getOwnerINLINE()) > 0)
								{
									szCivics.append(GC.getCivicInfo((CivicTypes)GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getCivics((CivicOptionTypes)iI)).getDescription());
								}
							}
						}
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CANNOT_FOUND_CITY_LIMIT", GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getCityLimit(), szCivics.c_str()));
					}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_SPREAD)
			{
				eReligion = ((ReligionTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionData()));

				if (pMissionCity != NULL)
				{
					if (pMissionCity->getTeam() != pHeadSelectedUnit->getTeam()) // XXX still true???
					{
						if (GET_PLAYER(pMissionCity->getOwnerINLINE()).isNoNonStateReligionSpread())
						{
							if (eReligion != GET_PLAYER(pMissionCity->getOwnerINLINE()).getStateReligion())
							{
								szBuffer.append(NEWLINE);
								szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CANNOT_SPREAD_NON_STATE_RELIGION"));
							}
						}
					}

					szBuffer.append(NEWLINE);
					GAMETEXT.setReligionHelpCity(szBuffer, eReligion, pMissionCity, false, true);
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_SPREAD_CORPORATION)
			{
				CorporationTypes eCorporation = ((CorporationTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionData()));

				if (pMissionCity != NULL)
				{
					szBuffer.append(NEWLINE);
					GAMETEXT.setCorporationHelpCity(szBuffer, eCorporation, pMissionCity, false, true);

					for (int iCorp = 0; iCorp < GC.getNumCorporationInfos(); ++iCorp)
					{
						if (eCorporation != iCorp)
						{
							if (pMissionCity->isHasCorporation((CorporationTypes)iCorp))
							{
								if (GC.getGameINLINE().isCompetingCorporation(eCorporation, (CorporationTypes)iCorp))
								{
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_ACTION_WILL_ELIMINATE_CORPORATION", GC.getCorporationInfo((CorporationTypes)iCorp).getTextKeyWide()));
								}
							}
						}
					}

					szTempBuffer.Format(L"%s%d %c", NEWLINE, pHeadSelectedUnit->spreadCorporationCost(eCorporation, pMissionCity), GC.getCommerceInfo(COMMERCE_GOLD).getChar());
					szBuffer.append(szTempBuffer);

					if (!pHeadSelectedUnit->canSpreadCorporation(pMissionPlot, eCorporation))
					{
						if (!GET_PLAYER(pMissionCity->getOwnerINLINE()).isActiveCorporation(eCorporation))
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CORPORATION_NOT_ACTIVE", GC.getCorporationInfo(eCorporation).getTextKeyWide(), GET_PLAYER(pMissionCity->getOwnerINLINE()).getCivilizationAdjective()));
						}

						CorporationTypes eCompetition = NO_CORPORATION;
						for (int iCorporation = 0; iCorporation < GC.getNumCorporationInfos(); ++iCorporation)
						{
							if (pMissionCity->isHeadquarters((CorporationTypes)iCorporation))
							{
								if (GC.getGameINLINE().isCompetingCorporation((CorporationTypes)iCorporation, eCorporation))
								{
									eCompetition = (CorporationTypes)iCorporation;
									break;
								}
							}
						}

						if (NO_CORPORATION != eCompetition)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CORPORATION_COMPETING_HEADQUARTERS", GC.getCorporationInfo(eCorporation).getTextKeyWide(), GC.getCorporationInfo(eCompetition).getTextKeyWide()));
						}

						CvWStringBuffer szBonusList;
						bool bValid = false;
						bool bFirst = true;
						for (int i = 0; i < GC.getNUM_CORPORATION_PREREQ_BONUSES(); ++i)
						{
							BonusTypes eBonus = (BonusTypes)GC.getCorporationInfo(eCorporation).getPrereqBonus(i);
							if (NO_BONUS != eBonus)
							{
								if (!bFirst)
								{
									szBonusList.append(L", ");
								}
								else
								{
									bFirst = false;
								}
								szBonusList.append(GC.getBonusInfo(eBonus).getDescription());

								if (pMissionCity->hasBonus(eBonus))
								{
									bValid = true;
									break;
								}
							}
						}

						if (!bValid)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CORPORATION_NO_RESOURCES", pMissionCity->getNameKey(), szBonusList.getCString()));
						}
						
						/************************************************************************************************/
						/* Afforess	                  Start		 02/05/10                                               */
						/*                                                                                              */
						/*                                                                                              */
						/************************************************************************************************/
						BuildingTypes ePrereqBuilding;
						CvCorporationInfo& kCorporation = GC.getCorporationInfo(eCorporation);
						for (int iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
						{
							if (kCorporation.getPrereqBuildingClass(iI) > 0)
							{
								ePrereqBuilding = ((BuildingTypes)(GC.getCivilizationInfo(GET_PLAYER(pMissionCity->getOwnerINLINE()).getCivilizationType()).getCivilizationBuildings((BuildingClassTypes)iI)));

								if (ePrereqBuilding != NO_BUILDING)
								{
									szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_REQUIRES_NUM_SPECIAL_BUILDINGS_NO_CITY", GC.getBuildingInfo(ePrereqBuilding).getTextKeyWide(), kCorporation.getPrereqBuildingClass(iI)).c_str());
									szBuffer.append(szTempBuffer);
								}
							}
						}
						/************************************************************************************************/
						/* Afforess	                     END                                                            */
						/************************************************************************************************/

					}
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_JOIN)
			{
// BUG - Specialist Actual Effects - start
				GAMETEXT.parseSpecialistHelpActual(szBuffer, ((SpecialistTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionData())), pMissionCity, true, 1);
// BUG - Specialist Actual Effects - end
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_CONSTRUCT)
			{
				eBuilding = ((BuildingTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionData()));

				if (pMissionCity != NULL)
				{
					if (!pHeadSelectedUnit->getUnitInfo().getForceBuildings(eBuilding)  && !pMissionCity->canConstruct(eBuilding, false, false, true))
					{
						if (!(GC.getGameINLINE().isBuildingClassMaxedOut((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType()))))
						{
							GAMETEXT.buildBuildingRequiresString(szBuffer, ((BuildingTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionData())), false, false, pMissionCity);
						}
					}
					else
					{
						szBuffer.append(NEWLINE);
// BUG - Building Actual Effects - start
						GAMETEXT.setBuildingHelpActual(szBuffer, ((BuildingTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionData())), false, false, false, pMissionCity);
// BUG - Building Actual Effects - end
					}
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_DISCOVER)
			{
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					if (pSelectedUnit->canDiscover(pMissionPlot))
					{
						eTech = pSelectedUnit->getDiscoveryTech();
	
						if (pSelectedUnit->getDiscoverResearch(eTech) >= GET_TEAM(pSelectedUnit->getTeam()).getResearchLeft(eTech))
						{
							szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_TECH_TEXT"), GC.getTechInfo(eTech).getDescription());
							szBuffer.append(NEWLINE);
							szBuffer.append(szTempBuffer);
						}
						else
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_EXTRA_RESEARCH", pSelectedUnit->getDiscoverResearch(eTech), GC.getTechInfo(eTech).getTextKeyWide()));
						}
						break;
					}

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_HURRY)
			{
				if (pMissionCity != NULL)
				{
					if (!(pMissionCity->isProductionBuilding()))
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_BUILDING_HURRY"));
					}
					else
					{
						pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

						while (pSelectedUnitNode != NULL)
						{
							pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

							if (pSelectedUnit->canHurry(pMissionPlot, true))
							{
								const wchar* pcKey = NULL;
								if (NO_PROJECT != pMissionCity->getProductionProject())
								{
									pcKey = GC.getProjectInfo(pMissionCity->getProductionProject()).getTextKeyWide();
								}
								else if (NO_BUILDING != pMissionCity->getProductionBuilding())
								{
									pcKey = GC.getBuildingInfo(pMissionCity->getProductionBuilding()).getTextKeyWide();
								}
								else if (NO_UNIT != pMissionCity->getProductionUnit())
								{
									pcKey = GC.getUnitInfo(pMissionCity->getProductionUnit()).getTextKeyWide();
								}
								if (NULL != pcKey && pSelectedUnit->getHurryProduction(pMissionPlot) >= pMissionCity->productionLeft())
								{
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_ACTION_FINISH_CONSTRUCTION", pcKey));
								}
								else
								{
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_ACTION_EXTRA_CONSTRUCTION", pSelectedUnit->getHurryProduction(pMissionPlot), pcKey));
								}
								break;
							}

							pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
						}
					}
				}
			}
/************************************************************************************************/
/* Afforess	                  Start		 12/31/09                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_HURRY_FOOD)
			{
				if (pMissionCity != NULL)
				{
					pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

					while (pSelectedUnitNode != NULL)
					{
						pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

						if (pSelectedUnit->canHurryFood(pMissionPlot))
						{
							const wchar* pcKey = (pMissionCity->getNameKey());
							if (pSelectedUnit->getHurryFood(pMissionPlot) >= (pMissionCity->growthThreshold() - pMissionCity->getFood()))
							{
								szBuffer.append(NEWLINE);
								szBuffer.append(gDLL->getText("TXT_KEY_ACTION_FINISH_FOOD", pcKey));
							}
							else
							{
								szBuffer.append(NEWLINE);
								szBuffer.append(gDLL->getText("TXT_KEY_ACTION_EXTRA_FOOD", pSelectedUnit->getHurryFood(pMissionPlot), pcKey));
							}
							break;
						}
						pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
					}
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_ESPIONAGE_SLEEP)
			{
				if (pMissionCity != NULL)
				{
					pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

					while (pSelectedUnitNode != NULL)
					{
						pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

						if (pSelectedUnit->canEspionage(pMissionPlot, false) || pSelectedUnit->canEspionage(pMissionPlot, true))
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_MISSION_ESPIONAGE_SLEEP_HELP"));
							break;
						}
						pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
					}
				}
			}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_TRADE)
			{
				if (pMissionCity != NULL)
				{
					if (pMissionCity->getOwnerINLINE() == pHeadSelectedUnit->getOwnerINLINE())
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_TRADE_MISSION_FOREIGN"));
					}
					else
					{
						pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

						while (pSelectedUnitNode != NULL)
						{
							pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

							if (pSelectedUnit->canTrade(pMissionPlot, true))
							{
								szTempBuffer.Format(L"%s+%d%c", NEWLINE, pSelectedUnit->getTradeGold(pMissionPlot), GC.getCommerceInfo(COMMERCE_GOLD).getChar());
								szBuffer.append(szTempBuffer);
								break;
							}

							pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
						}
					}
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_GREAT_WORK)
			{
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					if (pSelectedUnit->canGreatWork(pMissionPlot))
					{
						szTempBuffer.Format(L"%s+%d%c", NEWLINE, pSelectedUnit->getGreatWorkCulture(pMissionPlot), GC.getCommerceInfo(COMMERCE_CULTURE).getChar());
						szBuffer.append(szTempBuffer);
						break;
					}

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_INFILTRATE)
			{
				if (pMissionCity != NULL)
				{
					if (pMissionCity->getOwnerINLINE() == pHeadSelectedUnit->getOwnerINLINE())
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_INFILTRATE_MISSION_FOREIGN"));
					}
					else
					{
						pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

						while (pSelectedUnitNode != NULL)
						{
							pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

							if (pSelectedUnit->canEspionage(pMissionPlot))
							{
								szTempBuffer.Format(L"%s+%d%c", NEWLINE, pSelectedUnit->getEspionagePoints(pMissionPlot), GC.getCommerceInfo(COMMERCE_ESPIONAGE).getChar());
								szBuffer.append(szTempBuffer);
								break;
							}

							pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
						}
					}
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_GOLDEN_AGE)
			{
				iUnitConsume = GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).unitsRequiredForGoldenAge();
				iUnitDiff = (iUnitConsume - GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).unitsGoldenAgeReady());

				if (iUnitDiff > 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_MORE_GREAT_PEOPLE", iUnitDiff));
				}

				if (iUnitConsume > 1)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CONSUME_GREAT_PEOPLE", iUnitConsume));
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_LEAD)
			{
				if (pHeadSelectedUnit->getUnitInfo().getLeaderExperience() > 0)
				{
					int iNumUnits = pHeadSelectedUnit->canGiveExperience(pHeadSelectedUnit->plot());
					if (iNumUnits > 0)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_LEAD_TROOPS", pHeadSelectedUnit->getStackExperienceToGive(iNumUnits)));
					}
				}
				if (pHeadSelectedUnit->getUnitInfo().getLeaderPromotion() != NO_PROMOTION)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_WHEN_LEADING"));
					GAMETEXT.parsePromotionHelp(szBuffer, (PromotionTypes)pHeadSelectedUnit->getUnitInfo().getLeaderPromotion(), L"\n   ");
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_ESPIONAGE)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_ACTION_ESPIONAGE_MISSION"));

				GAMETEXT.setEspionageMissionHelp(szBuffer, pHeadSelectedUnit);
			}
			//TSHEEP Assassin Mission
			//RevolutionDCM start
			/*
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_ASSASSIN)
			{
				if(pMissionPlot->plotCount(PUF_isOtherTeam, pHeadSelectedUnit->getOwnerINLINE(), -1, NO_PLAYER, NO_TEAM, PUF_isVisible, pHeadSelectedUnit->getOwnerINLINE()) == 1)
				{
					CvUnit* pTargetUnit;
					int iEspionageCost = 0;
					int iMissionChance = 0;

					pTargetUnit = pMissionPlot->plotCheck(PUF_isOtherTeam, pHeadSelectedUnit->getOwnerINLINE(), -1, NO_PLAYER, NO_TEAM, PUF_isVisible, pHeadSelectedUnit->getOwnerINLINE());
					iEspionageCost = pHeadSelectedUnit->assassinCost(pTargetUnit);
					iMissionChance = pHeadSelectedUnit->assassinProb(pTargetUnit);
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_ASSASSIN_MISSION",pTargetUnit->getNameKey(),iEspionageCost,iMissionChance));
				}
			}
			*/
			//RevolutionDCM end
			//TSHEEP End
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_BUILD)
			{
				eBuild = ((BuildTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionData()));
				FAssert(eBuild != NO_BUILD);
				eImprovement = ((ImprovementTypes)(GC.getBuildInfo(eBuild).getImprovement()));
				eRoute = ((RouteTypes)(GC.getBuildInfo(eBuild).getRoute()));
				eBonus = pMissionPlot->getBonusType(pHeadSelectedUnit->getTeam());

				for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
				{
					iYield = 0;

					if (eImprovement != NO_IMPROVEMENT)
					{
						iYield += pMissionPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iI), pHeadSelectedUnit->getOwnerINLINE());
						if (pMissionPlot->getImprovementType() != NO_IMPROVEMENT)
						{
							iYield -= pMissionPlot->calculateImprovementYieldChange(pMissionPlot->getImprovementType(), ((YieldTypes)iI), pHeadSelectedUnit->getOwnerINLINE());
						}
					}

					if (NO_FEATURE != pMissionPlot->getFeatureType())
					{
						if (GC.getBuildInfo(eBuild).isFeatureRemove(pMissionPlot->getFeatureType()))
						{
							iYield -= GC.getFeatureInfo(pMissionPlot->getFeatureType()).getYieldChange(iI);
						}
					}

					if (iYield != 0)
					{
						szTempBuffer.Format(L", %s%d%c", ((iYield > 0) ? L"+" : L""), iYield, GC.getYieldInfo((YieldTypes) iI).getChar());
						szBuffer.append(szTempBuffer);
					}
				}

				if (NO_IMPROVEMENT != eImprovement)
				{
					int iHappy = GC.getImprovementInfo(eImprovement).getHappiness();

					if (iHappy != 0)
					{
						szTempBuffer.Format(L", +%d%c", abs(iHappy), (iHappy > 0 ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
						szBuffer.append(szTempBuffer);
					}
				}

				bValid = false;

				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					if (pSelectedUnit->canBuild(pMissionPlot, eBuild))
					{
						bValid = true;
						break;
					}

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}

				if (!bValid)
				{
					if (eImprovement != NO_IMPROVEMENT)
					{
						if (pMissionPlot->getTeam() != pHeadSelectedUnit->getTeam())
						{
							if (GC.getImprovementInfo(eImprovement).isOutsideBorders())
							{
								if (pMissionPlot->getTeam() != NO_TEAM)
								{
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_ACTION_NEEDS_OUT_RIVAL_CULTURE_BORDER"));
								}
							}
							else
							{
								szBuffer.append(NEWLINE);
								szBuffer.append(gDLL->getText("TXT_KEY_ACTION_NEEDS_CULTURE_BORDER"));
							}
						}

						if ((eBonus == NO_BONUS) || !(GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eBonus)))
						{
							if (!(GET_TEAM(pHeadSelectedUnit->getTeam()).isIrrigation()) && !(GET_TEAM(pHeadSelectedUnit->getTeam()).isIgnoreIrrigation()))
							{
								if (GC.getImprovementInfo(eImprovement).isRequiresIrrigation() && !(pMissionPlot->isIrrigationAvailable()))
								{
									for (iI = 0; iI < GC.getNumTechInfos(); iI++)
									{
										if (GC.getTechInfo((TechTypes)iI).isIrrigation())
										{
											szBuffer.append(NEWLINE);
											szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_STRING", GC.getTechInfo((TechTypes)iI).getTextKeyWide()));
											break;
										}
									}
								}
							}
						}
					}
					if (!(GET_TEAM(pHeadSelectedUnit->getTeam()).isHasTech((TechTypes)GC.getBuildInfo(eBuild).getTechPrereq())))
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_STRING", GC.getTechInfo((TechTypes) GC.getBuildInfo(eBuild).getTechPrereq()).getTextKeyWide()));
					}

					if (eRoute != NO_ROUTE)
					{
						if (GC.getRouteInfo(eRoute).getPrereqBonus() != NO_BONUS)
						{
							if (!(pMissionPlot->isAdjacentPlotGroupConnectedBonus(pHeadSelectedUnit->getOwnerINLINE(), ((BonusTypes)(GC.getRouteInfo(eRoute).getPrereqBonus())))))
							{
								szBuffer.append(NEWLINE);
								szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_STRING", GC.getBonusInfo((BonusTypes) GC.getRouteInfo(eRoute).getPrereqBonus()).getTextKeyWide()));
							}
						}

						bool bFoundValid = true;
						std::vector<BonusTypes> aeOrBonuses;
						for (int i = 0; i < GC.getNUM_ROUTE_PREREQ_OR_BONUSES(); ++i)
						{
							if (NO_BONUS != GC.getRouteInfo(eRoute).getPrereqOrBonus(i))
							{
								aeOrBonuses.push_back((BonusTypes)GC.getRouteInfo(eRoute).getPrereqOrBonus(i));
								bFoundValid = false;

								if (pMissionPlot->isAdjacentPlotGroupConnectedBonus(pHeadSelectedUnit->getOwnerINLINE(), ((BonusTypes)(GC.getRouteInfo(eRoute).getPrereqOrBonus(i)))))
								{
									bFoundValid = true;
									break;
								}
							}
						}

						if (!bFoundValid)
						{
							bool bFirst = true;
							for (std::vector<BonusTypes>::iterator it = aeOrBonuses.begin(); it != aeOrBonuses.end(); ++it)
							{
								szFirstBuffer = NEWLINE + gDLL->getText("TXT_KEY_BUILDING_REQUIRES_LIST");
								szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getBonusInfo(*it).getDescription());
								setListHelp(szBuffer, szFirstBuffer.GetCString(), szTempBuffer, gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
								bFirst = false;
							}
						}
					}

					if (pMissionPlot->getFeatureType() != NO_FEATURE)
					{
/************************************************************************************************/
/* Afforess	                  Start		 08/29/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
						if (((TechTypes)GC.getBuildInfo(eBuild).getTechPrereq() != (TechTypes)GC.getBuildInfo(eBuild).getFeatureTech(pMissionPlot->getFeatureType())) && !(GET_TEAM(pHeadSelectedUnit->getTeam()).isHasTech((TechTypes)GC.getBuildInfo(eBuild).getFeatureTech(pMissionPlot->getFeatureType()))))
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_STRING", GC.getTechInfo((TechTypes) GC.getBuildInfo(eBuild).getFeatureTech(pMissionPlot->getFeatureType())).getTextKeyWide()));
						}
					}
				}

				if (eImprovement != NO_IMPROVEMENT)
				{
					if (pMissionPlot->getImprovementType() != NO_IMPROVEMENT)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_WILL_DESTROY_IMP", GC.getImprovementInfo(pMissionPlot->getImprovementType()).getTextKeyWide()));
					}
				}

				if (GC.getBuildInfo(eBuild).isKill())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CONSUME_UNIT"));
				}

				if (pMissionPlot->getFeatureType() != NO_FEATURE)
				{
					if (GC.getBuildInfo(eBuild).isFeatureRemove(pMissionPlot->getFeatureType()))
					{
// BUG - Feature Health - start
						if (pMissionPlot->isCityRadius() && getBugOptionBOOL("MiscHover__RemoveFeatureHealthEffects", true, "BUG_REMOVE_FEATURE_HEALTH_EFFECTS"))
						{
							int iHealthPercent = GC.getFeatureInfo(pMissionPlot->getFeatureType()).getHealthPercent();

							if (iHealthPercent != 0)
							{
								bool bCountOtherTiles = getBugOptionBOOL("MiscHover__RemoveFeatureHealthEffectsCountOtherTiles", true, "BUG_REMOVE_FEATURE_HEALTH_EFFECTS_COUNT_OTHER_TILES");
								int iGoodPercentChange = 0;
								int iBadPercentChange = 0;

								if (iHealthPercent > 0)
								{
									iGoodPercentChange = - iHealthPercent;
								}
								else
								{
									iBadPercentChange = iHealthPercent;
								}
								for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
								{
									CvPlot* pLoopPlot = plotCity(pMissionPlot->getX_INLINE(), pMissionPlot->getY_INLINE(), iI);

									if (pLoopPlot != NULL)
									{
										CvCity* pLoopCity = pLoopPlot->getPlotCity();

										if (pLoopCity != NULL && pLoopCity->getTeam() == pHeadSelectedUnit->getTeam())
										{
											int iGood = 0, iBad = 0;
											int iFeatureHealthAdjust = 0;

											if (bCountOtherTiles)
											{
												int iCityGoodPercentChange = 0;
												int iCityBadPercentChange = 0;

												pLoopCity->calculateFeatureHealthPercentChange(iCityGoodPercentChange, iCityBadPercentChange, pMissionPlot);
												pLoopCity->getAdditionalHealth(iCityGoodPercentChange, iCityBadPercentChange, iGood, iBad);
												iGood = -iGood;
												iBad = -iBad;
												iFeatureHealthAdjust = iGood - iBad;
												iCityGoodPercentChange += iGoodPercentChange;
												iCityBadPercentChange += iBadPercentChange;
												pLoopCity->getAdditionalHealth(iCityGoodPercentChange, iCityBadPercentChange, iGood, iBad);
											}
											else
											{
												pLoopCity->getAdditionalHealth(iGoodPercentChange, iBadPercentChange, iGood, iBad);
											}
											if (iGood != 0 || iBad != 0)
											{
												bool bStarted = false;
												CvWStringBuffer szFeatureEffects;
												bStarted = GAMETEXT.setResumableGoodBadChangeHelp(szFeatureEffects, L"", L"", L"", iGood, gDLL->getSymbolID(HEALTHY_CHAR), iBad, gDLL->getSymbolID(UNHEALTHY_CHAR), false, false, bStarted);

												//Fuyu Negative Health Adjust
												//if both clearing the feature at hand and the building being contructed in the city cause health reduction, consider both effects
												int iBadHealthAdjust = 0;
												if (iBad > iGood && pLoopCity->getProductionBuilding() != NO_BUILDING)
												{
													iBadHealthAdjust = -std::min(0, pLoopCity->getAdditionalHealthByBuilding(pLoopCity->getProductionBuilding()));
												}

												int iSpoiledFood = pLoopCity->getAdditionalSpoiledFood(iGood, iBad, -iBadHealthAdjust);
												int iStarvation = 0;
												if (iSpoiledFood != 0)
												{
                                                    iStarvation = pLoopCity->getAdditionalStarvation(iSpoiledFood, /* iFoodAdjust: spoiled food from building */ ((iBadHealthAdjust != 0)? -pLoopCity->getAdditionalSpoiledFood(0, iBadHealthAdjust) : 0));
												}

												bStarted = GAMETEXT.setResumableValueChangeHelp(szFeatureEffects, L"", L"", L"", iSpoiledFood, gDLL->getSymbolID(EATEN_FOOD_CHAR), false, false, bStarted);
												bStarted = GAMETEXT.setResumableValueChangeHelp(szFeatureEffects, L"", L"", L"", iStarvation, gDLL->getSymbolID(BAD_FOOD_CHAR), false, false, bStarted);

												//Fuyu Health Level dropping below Happy Level?
												if (bStarted && bCountOtherTiles && iBad > iGood && iSpoiledFood == 0 && !pLoopCity->isNoUnhappiness())
												{
													//Disregard all temporary unhappiness, and unhealth from espionage
													int iTemporaryUnhappiness = 0;
													int iAngerPercent = 0;
													iAngerPercent += pLoopCity->getHurryPercentAnger();
													iAngerPercent += pLoopCity->getConscriptPercentAnger();
													iAngerPercent += pLoopCity->getDefyResolutionPercentAnger();
													iAngerPercent += pLoopCity->getWarWearinessPercentAnger();
													iTemporaryUnhappiness += ((iAngerPercent * (pLoopCity->getPopulation())) / GC.getPERCENT_ANGER_DIVISOR());
													iTemporaryUnhappiness += pLoopCity->getEspionageHappinessCounter();

													int iHappinessLevel = pLoopCity->happyLevel() - pLoopCity->unhappyLevel() + iTemporaryUnhappiness;
													int iHealthLevel = pLoopCity->goodHealth() - pLoopCity->badHealth() + pLoopCity->getEspionageHealthCounter();
													//Adjustments
													iHealthLevel += iFeatureHealthAdjust;
													//Adjustment for building 
													int iBuildingAdjust = 0;
													if (pLoopCity->getProductionBuilding() != NO_BUILDING)
													{
														iBuildingAdjust = std::max(0, pLoopCity->getAdditionalHappinessByBuilding(pLoopCity->getProductionBuilding()) - pLoopCity->getAdditionalHealthByBuilding(pLoopCity->getProductionBuilding()));
													}

													if (iHealthLevel < iHappinessLevel + iBuildingAdjust)
													{
														//Health level is already below happy
														CvWString szHealthLimitTempBuffer;
														szHealthLimitTempBuffer.Format(L", (%c&lt;%c)", gDLL->getSymbolID(HEALTHY_CHAR), gDLL->getSymbolID(HAPPY_CHAR));
														szFeatureEffects.append(szHealthLimitTempBuffer);
													}
													else if (iHealthLevel - iBad + iGood < iHappinessLevel + iBuildingAdjust)
													{
														CvWString szHealthLimitTempBuffer;
														szHealthLimitTempBuffer.Format(L", %c&lt;%c", gDLL->getSymbolID(HEALTHY_CHAR), gDLL->getSymbolID(HAPPY_CHAR));
														szFeatureEffects.append(szHealthLimitTempBuffer);
													}
												}
												//Fuyu END

												szBuffer.append(NEWLINE);
												szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CHANGE_IN_CITY", szFeatureEffects.getCString(), pLoopCity->getNameKey()));
											}
										}
									}
								}
							}
						}
// BUG - Feature Health - end

						iProduction = pMissionPlot->getFeatureProduction(eBuild, pHeadSelectedUnit->getTeam(), &pCity);

						if (iProduction > 0)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CHANGE_PRODUCTION", iProduction, pCity->getNameKey()));
						}
/************************************************************************************************/
/* Afforess	                  Start		 08/29/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
						else
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_NO_PRODUCTION"));
						}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_REMOVE_FEATURE", GC.getFeatureInfo(pMissionPlot->getFeatureType()).getTextKeyWide()));
					}

				}

				if (eImprovement != NO_IMPROVEMENT)
				{
					if (eBonus != NO_BONUS)
					{
						if (!GET_TEAM(pHeadSelectedUnit->getTeam()).isBonusObsolete(eBonus))
						{
							if (GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eBonus))
							{
								szBuffer.append(NEWLINE);
								szBuffer.append(gDLL->getText("TXT_KEY_ACTION_PROVIDES_BONUS", GC.getBonusInfo(eBonus).getTextKeyWide()));

								if (GC.getBonusInfo(eBonus).getHealth() != 0)
								{
									szTempBuffer.Format(L" (+%d%c)", abs(GC.getBonusInfo(eBonus).getHealth()), ((GC.getBonusInfo(eBonus).getHealth() > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR)));
									szBuffer.append(szTempBuffer);
								}

								if (GC.getBonusInfo(eBonus).getHappiness() != 0)
								{
									szTempBuffer.Format(L" (+%d%c)", abs(GC.getBonusInfo(eBonus).getHappiness()), ((GC.getBonusInfo(eBonus).getHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
									szBuffer.append(szTempBuffer);
								}
							}
						}
					}
					else
					{
						iLast = 0;

						FAssertMsg((0 < GC.getNumBonusInfos()), "GC.getNumBonusInfos() is not greater than zero but an array is being allocated in CvDLLWidgetData::parseActionHelp");
						for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
						{
							if (GET_TEAM(pHeadSelectedUnit->getTeam()).isHasTech((TechTypes)(GC.getBonusInfo((BonusTypes) iI).getTechReveal())))
							{
								if (GC.getImprovementInfo(eImprovement).getImprovementBonusDiscoverRand(iI) > 0)
								{
									szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_ACTION_CHANCE_DISCOVER").c_str());
									szTempBuffer.Format(L"%c", GC.getBonusInfo((BonusTypes) iI).getChar());
									setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", (GC.getImprovementInfo(eImprovement).getImprovementBonusDiscoverRand(iI) != iLast));
									iLast = GC.getImprovementInfo(eImprovement).getImprovementBonusDiscoverRand(iI);
								}
							}
						}
					}

					if (!(pMissionPlot->isIrrigationAvailable()))
					{
						GAMETEXT.setYieldChangeHelp(szBuffer, gDLL->getText("TXT_KEY_ACTION_IRRIGATED").c_str(), L": ", L"", GC.getImprovementInfo(eImprovement).getIrrigatedYieldChangeArray());
					}

					if (eRoute == NO_ROUTE)
					{
						for (iI = 0; iI < GC.getNumRouteInfos(); iI++)
						{
							if (pMissionPlot->getRouteType() != ((RouteTypes)iI))
							{
								GAMETEXT.setYieldChangeHelp(szBuffer, GC.getRouteInfo((RouteTypes)iI).getDescription(), L": ", L"", GC.getImprovementInfo(eImprovement).getRouteYieldChangesArray((RouteTypes)iI));
							}
						}
					}

					if (GC.getImprovementInfo(eImprovement).getDefenseModifier() != 0)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_DEFENSE_MODIFIER", GC.getImprovementInfo(eImprovement).getDefenseModifier()));
					}
/************************************************************************************************/
/* Afforess	                  Start		 05/23/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
					if (GC.getImprovementInfo(eImprovement).getImprovementUpgrade() != NO_IMPROVEMENT)
*/
					if (GET_TEAM(pHeadSelectedUnit->getTeam()).getImprovementUpgrade(eImprovement) != NO_IMPROVEMENT)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
					{
						iTurns = pMissionPlot->getUpgradeTimeLeft(eImprovement, pHeadSelectedUnit->getOwnerINLINE());

						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_BECOMES_IMP", GC.getImprovementInfo((ImprovementTypes) GC.getImprovementInfo(eImprovement).getImprovementUpgrade()).getTextKeyWide(), iTurns));
					}
				}

				if (eRoute != NO_ROUTE)
				{
					eFinalImprovement = eImprovement;

					if (eFinalImprovement == NO_IMPROVEMENT)
					{
						eFinalImprovement = pMissionPlot->getImprovementType();
					}

					if (eFinalImprovement != NO_IMPROVEMENT)
					{
						RouteTypes eExistingRoute = pMissionPlot->getRouteType();
						const int* pYieldChanges = GC.getImprovementInfo(eFinalImprovement).getRouteYieldChangesArray(eRoute);
						const int* pExistingYieldChanges = (eExistingRoute == NO_ROUTE ? NULL : GC.getImprovementInfo(eFinalImprovement).getRouteYieldChangesArray(eExistingRoute));

						int aiYields[NUM_YIELD_TYPES];
						for (int iI = 0; iI < NUM_YIELD_TYPES; ++iI)
						{
							aiYields[iI] = (pYieldChanges == NULL ? 0 : pYieldChanges[iI]);
							aiYields[iI] -= (pExistingYieldChanges == NULL ? 0 : pExistingYieldChanges[iI]);
						}

						GAMETEXT.setYieldChangeHelp(szBuffer, GC.getImprovementInfo(eFinalImprovement).getDescription(), L": ", L"", aiYields);
					}

					iMovementCost = GC.getRouteInfo(eRoute).getMovementCost() + GET_TEAM(pHeadSelectedUnit->getTeam()).getRouteChange(eRoute);
					iFlatMovementCost = GC.getRouteInfo(eRoute).getFlatMovementCost();

					if (iMovementCost > 0)
					{
						iMoves = (GC.getMOVE_DENOMINATOR() / iMovementCost);

						if ((iMoves * iMovementCost) < GC.getMOVE_DENOMINATOR())
						{
							iMoves++;
						}
					}
					else
					{
						iMoves = GC.getMOVE_DENOMINATOR();
					}

					if (iFlatMovementCost > 0)
					{
						iFlatMoves = (GC.getMOVE_DENOMINATOR() / iFlatMovementCost);

						if ((iFlatMoves * iFlatMovementCost) < GC.getMOVE_DENOMINATOR())
						{
							iFlatMoves++;
						}
					}
					else
					{
						iFlatMoves = GC.getMOVE_DENOMINATOR();
					}

					if ((iMoves > 1) || (iFlatMoves > 1))
					{
						if (iMoves >= iFlatMoves)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_MOVEMENT_COST", iMoves));
						}
						else
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_FLAT_MOVEMENT_COST", iFlatMoves));
						}
					}

					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CONNECTS_RESOURCES"));
				}

				iNowWorkRate = 0;
				iThenWorkRate = 0;

				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				if (NULL != pHeadSelectedUnit)
				{
					if (GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getBuildCost(pMissionPlot, eBuild) > 0)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_BUILD_COST", GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getBuildCost(pMissionPlot, eBuild)));
					}
				}

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					if (pSelectedUnit->getBuildType() != eBuild)
					{
						iNowWorkRate += pSelectedUnit->workRate(false);
						iThenWorkRate += pSelectedUnit->workRate(true);
					}

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}

				iTurns = pMissionPlot->getBuildTurnsLeft(eBuild, iNowWorkRate, iThenWorkRate);


				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_ACTION_NUM_TURNS", iTurns));

				if (!CvWString(GC.getBuildInfo(eBuild).getHelp()).empty())
				{
					szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getBuildInfo(eBuild).getHelp()).c_str());
				}
			}
// BUG - Fortify/Sleep All Action - start
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_FORTIFY || GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_SLEEP)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_SAME_UNITS_TYPE"));
			}
// BUG - Fortify/Sleep All Action - end

			if (!CvWString(GC.getMissionInfo((MissionTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType())).getHelp()).empty())
			{
				szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getMissionInfo((MissionTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType())).getHelp()).c_str());
			}

			MissionTypes eMission = (MissionTypes) GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType();
			if ( eMission > MISSION_PRETARGET_NUKE) // Outcome missions
			{
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					CvOutcomeMission* pOutcomeMission = pSelectedUnit->getUnitInfo().getOutcomeMissionByMission(eMission);
					if (pOutcomeMission)
					{
						if (pOutcomeMission->isPossible(pSelectedUnit))
						{
							pOutcomeMission->buildDisplayString(szBuffer, pSelectedUnit);
							break;
						}
					}

					// check the unit combat types
					bool bFound = false;
					for (int iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
					{
						if (pSelectedUnit->hasCombatType((UnitCombatTypes)iI))
						{
							UnitCombatTypes eCombat = (UnitCombatTypes)iI;
							CvOutcomeMission* pOutcomeMission = GC.getUnitCombatInfo(eCombat).getOutcomeMissionByMission(eMission);
							if (pOutcomeMission)
							{
								if (pOutcomeMission->isPossible(pSelectedUnit))
								{
									pOutcomeMission->buildDisplayString(szBuffer, pSelectedUnit);
									bFound = true;
									break;
								}
							}
						}
					}
					if (bFound)
						break;

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}
			}
		}

		if (GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType() != NO_COMMAND)
		{
			if (GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType() == COMMAND_PROMOTION)
			{
				GAMETEXT.parsePromotionHelp(szBuffer, ((PromotionTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandData())));
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType() == COMMAND_UPGRADE)
			{
				GAMETEXT.setBasicUnitHelp(szBuffer, ((UnitTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandData())));

				if (bAlt && GC.getCommandInfo((CommandTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType())).getAll())
				{
					iPrice = GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).upgradeAllPrice(((UnitTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandData())), pHeadSelectedUnit->getUnitType());
				}
				else
				{
					iPrice = 0;

					pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

					while (pSelectedUnitNode != NULL)
					{
						pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

						if (pSelectedUnit->canUpgrade(((UnitTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandData())), true))
						{
							iPrice += pSelectedUnit->upgradePrice((UnitTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandData()));
						}

						pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
					}
				}

				szTempBuffer.Format(L"%s%d %c", NEWLINE, iPrice, GC.getCommerceInfo(COMMERCE_GOLD).getChar());
				szBuffer.append(szTempBuffer);
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType() == COMMAND_GIFT)
			{
				eGiftPlayer = pHeadSelectedUnit->plot()->getOwnerINLINE();

				if (eGiftPlayer != NO_PLAYER)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_GOES_TO_CIV"));

					szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, GET_PLAYER(eGiftPlayer).getPlayerTextColorR(), GET_PLAYER(eGiftPlayer).getPlayerTextColorG(), GET_PLAYER(eGiftPlayer).getPlayerTextColorB(), GET_PLAYER(eGiftPlayer).getPlayerTextColorA(), GET_PLAYER(eGiftPlayer).getCivilizationShortDescription());
					szBuffer.append(szTempBuffer);

					pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

					while (pSelectedUnitNode != NULL)
					{
						pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

						if (!(GET_PLAYER(eGiftPlayer).AI_acceptUnit(pSelectedUnit)))
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_REFUSE_GIFT", GET_PLAYER(eGiftPlayer).getNameKey()));
							break;
						}

						pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
					}
				}
			}
// BUG - Delete All Action - start
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType() == COMMAND_DELETE)
			{
/************************************************************************************************/
/* Afforess	                  Start		 02/12/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();
				pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

				if (pSelectedUnit->plot()->getOwner() == pSelectedUnit->getOwner())	//units have to be inside cultural borders
				{
					if (GC.getDefineINT("DISBANDING_EARNS_GOLD"))
					{
						int iGold = 0;
						int iTrainPercent = GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent();

						while (pSelectedUnitNode != NULL)
						{
							pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);
							iGold += pSelectedUnit->getUnitInfo().getProductionCost();

							pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
						}
					
						iGold = (iGold * iTrainPercent) / std::max(1, (GC.getDefineINT("UNIT_GOLD_DISBAND_DIVISOR") * 100));

						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_MISC_GOLD_FOR_DISBANDING", iGold));
					}
				}

/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			}
// BUG - Delete All Action - end

			if (GC.getCommandInfo((CommandTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType())).getAll())
			{
				szBuffer.append(gDLL->getText("TXT_KEY_ACTION_ALL_UNITS"));
			}

			if (!CvWString(GC.getCommandInfo((CommandTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType())).getHelp()).empty())
			{
/************************************************************************************************/
/* Great Diplomat MOD               START                                                  		*/
/************************************************************************************************/
				if ((CommandTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType()) == COMMAND_UPGRADE_IMPROVEMENTS)
				{
					CvWString szTempBuffer;
					CvWString szTempBuffer2;

					int numImprovementInfos = GC.getNumImprovementInfos();
					for (int iJ = 0; iJ < numImprovementInfos; ++iJ)
					{
						if (GC.getCommandInfo(COMMAND_UPGRADE_IMPROVEMENTS).getUpgradeImprovementRate(iJ) > 0)
						{
							ImprovementTypes eFromImprovement = (ImprovementTypes)iJ;
							ImprovementTypes eToImprovement = NO_IMPROVEMENT;

							for (int iK = 0; iK < GC.getCommandInfo(COMMAND_UPGRADE_IMPROVEMENTS).getUpgradeImprovementRate(iJ); ++iK)
							{
								eToImprovement = (ImprovementTypes)GC.getImprovementInfo(eFromImprovement).getImprovementUpgrade();
								if (eToImprovement != NO_IMPROVEMENT)
								{
									eFromImprovement = eToImprovement;
								}
								else
								{
									break;
								}
							}

							if (eToImprovement == NO_IMPROVEMENT)
							{
								eToImprovement = eFromImprovement;
							}

							if ((ImprovementTypes)iJ != eToImprovement)
							{
								if (szTempBuffer2.empty())
								{
									szTempBuffer.Format(SETCOLR L"%s" ENDCOLR L" -> " SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getImprovementInfo((ImprovementTypes)iJ).getDescription(), TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getImprovementInfo(eToImprovement).getDescription());
									szTempBuffer2.append(szTempBuffer);
								}
								else
								{
									szTempBuffer.Format(L", " SETCOLR L"%s" ENDCOLR L" -> " SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getImprovementInfo((ImprovementTypes)iJ).getDescription(), TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getImprovementInfo(eToImprovement).getDescription());
									szTempBuffer2.append(szTempBuffer);
								}
							}
						}
					}

					if (!szTempBuffer2.empty())
					{
						szBuffer.append(CvWString::format(L"%s%s%s", NEWLINE, GC.getCommandInfo((CommandTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType())).getHelp(), szTempBuffer2.GetCString()).c_str());
					}
				}
				else
				{
					szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getCommandInfo((CommandTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType())).getHelp()).c_str());
				}
/************************************************************************************************/
/* Great Diplomat MOD               END                                                  		*/
/************************************************************************************************/
			}
		}

		if (GC.getActionInfo(widgetDataStruct.m_iData1).getAutomateType() != NO_AUTOMATE)
		{
			if (!CvWString(GC.getAutomateInfo((ControlTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getAutomateType())).getHelp()).empty())
			{
				szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getAutomateInfo((ControlTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getAutomateType())).getHelp()).c_str());
			}
		}
	}

	if (GC.getActionInfo(widgetDataStruct.m_iData1).getControlType() != NO_CONTROL)
	{
		if (!CvWString(GC.getControlInfo((ControlTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getControlType())).getHelp()).empty())
		{
			szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getControlInfo((ControlTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getControlType())).getHelp()).c_str());
		}
	}

	if (GC.getActionInfo(widgetDataStruct.m_iData1).getInterfaceModeType() != NO_INTERFACEMODE)
	{
		if (!CvWString(GC.getInterfaceModeInfo((InterfaceModeTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getInterfaceModeType())).getHelp()).empty())
		{
			szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getInterfaceModeInfo((InterfaceModeTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getInterfaceModeType())).getHelp()).c_str());
		}
	}
}


void CvDLLWidgetData::parseCitizenHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	int iCount;
	int iI;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (widgetDataStruct.m_iData1 != NO_SPECIALIST)
		{
			GAMETEXT.parseSpecialistHelp(szBuffer, ((SpecialistTypes)(widgetDataStruct.m_iData1)), pHeadSelectedCity);

			if (widgetDataStruct.m_iData2 != -1)
			{
				iCount = 0;

				for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
				{
					if (iI < widgetDataStruct.m_iData1)
					{
						iCount += pHeadSelectedCity->getSpecialistCount((SpecialistTypes)iI);
					}
					else if (iI == widgetDataStruct.m_iData1)
					{
						iCount += widgetDataStruct.m_iData2;
					}
				}

				if (iCount < pHeadSelectedCity->totalFreeSpecialists())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_FREE_SPECIALIST"));
				}
			}
		}
	}
}


void CvDLLWidgetData::parseFreeCitizenHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		SpecialistTypes eSpecialist = (SpecialistTypes)widgetDataStruct.m_iData1;
		if (NO_SPECIALIST != eSpecialist)
		{
			GAMETEXT.parseSpecialistHelp(szBuffer, eSpecialist, pHeadSelectedCity);
		}
		if (widgetDataStruct.m_iData2 != -1)
		{
			szBuffer.append(SEPARATOR);
			GAMETEXT.parseFreeSpecialistHelp(szBuffer, *pHeadSelectedCity);
		}
	}
}


void CvDLLWidgetData::parseDisabledCitizenHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	CvWString szTempBuffer;
	BuildingTypes eLoopBuilding;
	bool bFirst;
	int iI;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (widgetDataStruct.m_iData1 != NO_SPECIALIST)
		{
// BUG - Specialist Actual Effects - start
			GAMETEXT.parseSpecialistHelpActual(szBuffer, ((SpecialistTypes)(widgetDataStruct.m_iData1)), pHeadSelectedCity, false, 1);
// BUG - Specialist Actual Effects - end

			if (!(pHeadSelectedCity->isSpecialistValid(((SpecialistTypes)(widgetDataStruct.m_iData1)), 1)))
			{
				bFirst = true;

				for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
				{
					eLoopBuilding = (BuildingTypes)GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationBuildings(iI);

					if (eLoopBuilding != NO_BUILDING)
					{
						if (GC.getBuildingInfo(eLoopBuilding).getSpecialistCount(widgetDataStruct.m_iData1) > 0)
						{
							if ((pHeadSelectedCity->getNumBuilding(eLoopBuilding) <= 0) && !isLimitedWonderClass((BuildingClassTypes)iI))
							{
								if ((GC.getBuildingInfo(eLoopBuilding).getSpecialBuildingType() == NO_SPECIALBUILDING) || pHeadSelectedCity->canConstruct(eLoopBuilding))
								{
									szTempBuffer = NEWLINE + gDLL->getText("TXT_KEY_REQUIRES");
									setListHelp(szBuffer, szTempBuffer, GC.getBuildingInfo(eLoopBuilding).getDescription(), gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
									bFirst = false;
								}
							}
						}
					}
				}

				if (!bFirst)
				{
					szBuffer.append(ENDCOLR);
				}
			}
		}
	}
}


void CvDLLWidgetData::parseAngryCitizenHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		szBuffer.assign(gDLL->getText("TXT_KEY_MISC_ANGRY_CITIZEN"));
		szBuffer.append(NEWLINE);

		GAMETEXT.setAngerHelp(szBuffer, *pHeadSelectedCity);
	}
}


void CvDLLWidgetData::parseChangeSpecialistHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (widgetDataStruct.m_iData2 > 0)
		{
// BUG - Specialist Actual Effects - start
			GAMETEXT.parseSpecialistHelpActual(szBuffer, ((SpecialistTypes)(widgetDataStruct.m_iData1)), pHeadSelectedCity, false, widgetDataStruct.m_iData2);
// BUG - Specialist Actual Effects - end

			if (widgetDataStruct.m_iData1 != GC.getDefineINT("DEFAULT_SPECIALIST"))
			{
				if (!(GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).isSpecialistValid((SpecialistTypes)(widgetDataStruct.m_iData1))))
				{
					if (pHeadSelectedCity->getMaxSpecialistCount((SpecialistTypes)(widgetDataStruct.m_iData1)) > 0)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_MISC_MAX_SPECIALISTS", pHeadSelectedCity->getMaxSpecialistCount((SpecialistTypes)(widgetDataStruct.m_iData1))));
					}
				}
			}
		}
		else
		{
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_REMOVE_SPECIALIST", GC.getSpecialistInfo((SpecialistTypes) widgetDataStruct.m_iData1).getTextKeyWide()));

// BUG - Remove Specialist Hover - start
			if (getBugOptionBOOL("MiscHover__RemoveSpecialist", true, "BUG_CITY_SCREEN_REMOVE_SPECIALIST_HOVER"))
			{
// BUG - Specialist Actual Effects - start
				szBuffer.append(NEWLINE);
				GAMETEXT.parseSpecialistHelpActual(szBuffer, ((SpecialistTypes)(widgetDataStruct.m_iData1)), pHeadSelectedCity, false, widgetDataStruct.m_iData2);
// BUG - Specialist Actual Effects - end
			}
// BUG - Remove Specialist Hover - end

			if (pHeadSelectedCity->getForceSpecialistCount((SpecialistTypes)(widgetDataStruct.m_iData1)) > 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_FORCED_SPECIALIST", pHeadSelectedCity->getForceSpecialistCount((SpecialistTypes)(widgetDataStruct.m_iData1))));
			}
		}
	}
}


void CvDLLWidgetData::parseResearchHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	TechTypes eTech;

	eTech = ((TechTypes)(widgetDataStruct.m_iData1));

	if (eTech == NO_TECH)
	{
		//	No Technology
		if (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCurrentResearch() != NO_TECH)
		{
			CvGameAI& game = GC.getGameINLINE();
			CvPlayer& activePlayer = GET_PLAYER(game.getActivePlayer());
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CHANGE_RESEARCH"));
			szBuffer.append(NEWLINE);
			GAMETEXT.setTechHelp(szBuffer, activePlayer.getCurrentResearch(), false, true);
		}
	}
	else
	{
		GAMETEXT.setTechHelp(szBuffer, eTech, false, true, widgetDataStruct.m_bOption);
	}
}


void CvDLLWidgetData::parseTechTreeHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.setTechHelp(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), false, false, false, false);
}


void CvDLLWidgetData::parseChangePercentHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 > 0)
	{
		szBuffer.assign(gDLL->getText("TXT_KEY_MISC_INCREASE_RATE", GC.getCommerceInfo((CommerceTypes) widgetDataStruct.m_iData1).getTextKeyWide(), widgetDataStruct.m_iData2));
	}
	else
	{
		szBuffer.assign(gDLL->getText("TXT_KEY_MISC_DECREASE_RATE", GC.getCommerceInfo((CommerceTypes) widgetDataStruct.m_iData1).getTextKeyWide(), -(widgetDataStruct.m_iData2)));
	}
}

// BUG - Min/Max Commerce Rate - start
void CvDLLWidgetData::parseSetPercentHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_SET_RATE", GC.getCommerceInfo((CommerceTypes) widgetDataStruct.m_iData1).getTextKeyWide(), widgetDataStruct.m_iData2));
}
// BUG - Min/Max Commerce Rate - end


void CvDLLWidgetData::parseContactCivHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      04/03/09                                jdog5000      */
/*                                                                                              */
/* DEBUG                                                                                        */
/************************************************************************************************/
	// do not execute if player is out of range
	PlayerTypes ePlayer = (PlayerTypes) widgetDataStruct.m_iData1;
	if (ePlayer >= MAX_PLAYERS)
	{
		return;
	}

	// do not execute if player is not a real civ
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	if (kPlayer.getCivilizationType() == NO_CIVILIZATION)
	{
		return;
	}

	// make sure its empty to start
	szBuffer.clear();

	TeamTypes eTeam = (TeamTypes) kPlayer.getTeam();
	CvTeamAI& kTeam = GET_TEAM(eTeam);

	PlayerTypes eActivePlayer = GC.getGameINLINE().getActivePlayer();
	TeamTypes eActiveTeam = (TeamTypes) GET_PLAYER(eActivePlayer).getTeam();
	CvTeamAI& kActiveTeam = GET_TEAM(eActiveTeam);
	

/************************************************************************************************/
/* REVOLUTION_MOD                         02/10/08                                jdog5000      */
/*                                                                                              */
/* Minor Civs                                                                                   */
/************************************************************************************************/
	if( GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).isMinorCiv() )
	{
		szBuffer.assign(gDLL->getText("TXT_KEY_REV_CONTACT_MINOR", GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getCivilizationDescription()));
		return;
	}
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/

	// if alt down and cheat on, show extra info
	if (gDLL->altKey() && gDLL->getChtLvl() > 0)
	{
		
		// Show tech percent adjust
		szBuffer.append(CvWString::format(SETCOLR L"TechPercent: %d%%, CurResMod: %d%%" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), kTeam.getBestKnownTechScorePercent(), kPlayer.calculateResearchModifier(kPlayer.getCurrentResearch()) ));
		szBuffer.append(NEWLINE);
		szBuffer.append(NEWLINE);

		szBuffer.append("Power");
		szBuffer.append(NEWLINE);
		// show everyones power for the active player
		if (eActivePlayer == ePlayer)
		{
			for (int iI = 0; iI < MAX_PLAYERS; iI++)
			{
				CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iI);
				if (kLoopPlayer.isAlive())
				{
					CvTeam& kLoopTeam = GET_TEAM((TeamTypes) kLoopPlayer.getTeam());

					szBuffer.append(CvWString::format(SETCOLR L"%s (team%d): %d (%d def)" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"),
						kLoopPlayer.getName(), kLoopPlayer.getTeam(), kLoopPlayer.getPower(), kLoopTeam.getDefensivePower()));
					
					// if in financial trouble, show that
					if (kLoopPlayer.AI_isFinancialTrouble())
					{
						szBuffer.append(CvWString::format(SETCOLR L" ($$$)" ENDCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT")));
					}

					szBuffer.append(NEWLINE);
				}
			}
		}
		// only should this one power if not active player
		else
		{
			szBuffer.append(CvWString::format(SETCOLR L"%d (%d) power" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), kPlayer.getPower(), kTeam.getPower(true)));
			
			// if in financial trouble, show that
			if (kPlayer.AI_isFinancialTrouble())
			{
				szBuffer.append(CvWString::format(SETCOLR L" ($$$)" ENDCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT")));
			}

			szBuffer.append(NEWLINE);
		}

		// Strategies
		CvWString szTempBuffer;
		szTempBuffer.Format(L"");

		//kPlayer.AI_getStrategyHash();
		if (kPlayer.AI_isDoStrategy(AI_STRATEGY_DAGGER))
        {
            szTempBuffer.Format(L"Dagger, ");
            szBuffer.append(szTempBuffer);
        }
		if (kPlayer.AI_isDoStrategy(AI_STRATEGY_CRUSH))
        {
            szTempBuffer.Format(L"Crush, ");
            szBuffer.append(szTempBuffer);
		}
		if (kPlayer.AI_isDoStrategy(AI_STRATEGY_ALERT1))
        {
            szTempBuffer.Format(L"Alert1, ");
            szBuffer.append(szTempBuffer);
		}
		if (kPlayer.AI_isDoStrategy(AI_STRATEGY_ALERT2))
        {
            szTempBuffer.Format(L"Alert2, ");
            szBuffer.append(szTempBuffer);
		}
		if (kPlayer.AI_isDoStrategy(AI_STRATEGY_TURTLE))
        {
            szTempBuffer.Format(L"Turtle, ");
            szBuffer.append(szTempBuffer);
		}
		if (kPlayer.AI_isDoStrategy(AI_STRATEGY_LAST_STAND))
        {
            szTempBuffer.Format(L"LastStand, ");
            szBuffer.append(szTempBuffer);
		}
		if (kPlayer.AI_isDoStrategy(AI_STRATEGY_FINAL_WAR))
        {
            szTempBuffer.Format(L"FinalWar, ");
            szBuffer.append(szTempBuffer);
        }
        if (kPlayer.AI_isDoStrategy(AI_STRATEGY_GET_BETTER_UNITS))
        {
            szTempBuffer.Format(L"GetBetterUnits, ");
            szBuffer.append(szTempBuffer);
        }
		if (kPlayer.AI_isDoStrategy(AI_STRATEGY_PRODUCTION))
        {
            szTempBuffer.Format(L"Production, ");
            szBuffer.append(szTempBuffer);
        }
		if (kPlayer.AI_isDoStrategy(AI_STRATEGY_MISSIONARY))
        {
            szTempBuffer.Format(L"Missionary, ");
            szBuffer.append(szTempBuffer);
        }
		if (kPlayer.AI_isDoStrategy(AI_STRATEGY_BIG_ESPIONAGE))
        {
            szTempBuffer.Format(L"BigEspionage, ");
            szBuffer.append(szTempBuffer);
        }

		szBuffer.append(NEWLINE);
		szTempBuffer.Format(L"Vic Strats: ");
        szBuffer.append(szTempBuffer);

		szTempBuffer.Format(L"");
		// Victory strategies

		kPlayer.AI_getVictoryStrategyHash();
		if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CULTURE1))
        {
            szTempBuffer.Format(L"Culture1, ");
            szBuffer.append(szTempBuffer);
        }
        if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2))
        {
            szTempBuffer.Format(L"Culture2, ");
            szBuffer.append(szTempBuffer);
        }
        if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3))
        {
            szTempBuffer.Format(L"Culture3, ");
            szBuffer.append(szTempBuffer);
        }
		if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CULTURE4))
        {
            szTempBuffer.Format(L"Culture4, ");
            szBuffer.append(szTempBuffer);
        }

		if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_SPACE1))
        {
            szTempBuffer.Format(L"Space1, ");
            szBuffer.append(szTempBuffer);
        }
        if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_SPACE2))
        {
            szTempBuffer.Format(L"Space2, ");
            szBuffer.append(szTempBuffer);
        }
        if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_SPACE3))
        {
            szTempBuffer.Format(L"Space3, ");
            szBuffer.append(szTempBuffer);
        }
		if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_SPACE4))
        {
            szTempBuffer.Format(L"Space4, ");
            szBuffer.append(szTempBuffer);
        }

		if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST1))
        {
            szTempBuffer.Format(L"Conq1, ");
            szBuffer.append(szTempBuffer);
        }
        if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST2))
        {
            szTempBuffer.Format(L"Conq2, ");
            szBuffer.append(szTempBuffer);
        }
		if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST3))
        {
            szTempBuffer.Format(L"Conq3, ");
            szBuffer.append(szTempBuffer);
        }
		if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST4))
        {
            szTempBuffer.Format(L"Conq4, ");
            szBuffer.append(szTempBuffer);
        }

		if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_DOMINATION1))
        {
            szTempBuffer.Format(L"Dom1, ");
            szBuffer.append(szTempBuffer);
        }
        if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_DOMINATION2))
        {
            szTempBuffer.Format(L"Dom2, ");
            szBuffer.append(szTempBuffer);
        }
		if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_DOMINATION3))
        {
            szTempBuffer.Format(L"Dom3, ");
            szBuffer.append(szTempBuffer);
        }
		if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_DOMINATION4))
        {
            szTempBuffer.Format(L"Dom4, ");
            szBuffer.append(szTempBuffer);
        }

		if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_DIPLOMACY1))
        {
            szTempBuffer.Format(L"Diplo1, ");
            szBuffer.append(szTempBuffer);
        }
        if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_DIPLOMACY2))
        {
            szTempBuffer.Format(L"Diplo2, ");
            szBuffer.append(szTempBuffer);
        }
		if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_DIPLOMACY3))
        {
            szTempBuffer.Format(L"Diplo3, ");
            szBuffer.append(szTempBuffer);
        }
		if (kPlayer.AI_isDoVictoryStrategy(AI_VICTORY_DIPLOMACY4))
        {
            szTempBuffer.Format(L"Diplo4, ");
            szBuffer.append(szTempBuffer);
        }
		
		// skip a line
		szBuffer.append(NEWLINE);
		szBuffer.append(NEWLINE);

		// show peace values
		bool bHadAny = false;
		bool bFirst = true;
		for (int iTeamIndex = 0; iTeamIndex < MAX_TEAMS; iTeamIndex++)
		{
			TeamTypes eLoopTeam = (TeamTypes) iTeamIndex;
			CvTeamAI& kLoopTeam = GET_TEAM(eLoopTeam);
			if (eLoopTeam != eTeam && kLoopTeam.isAlive() && !kLoopTeam.isBarbarian() && !kLoopTeam.isMinorCiv())
			{
				if (kTeam.isAtWar(eLoopTeam))
				{
					if (bFirst)
					{
						szBuffer.append(CvWString::format(SETCOLR L"Current War:\n" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT")));
						bFirst = false;
					}

					bHadAny = true;
					
					WarPlanTypes eWarPlan = kTeam.AI_getWarPlan(eLoopTeam);
					CvWStringBuffer szWarplan;
					GAMETEXT.getWarplanString(szWarplan, eWarPlan);

					int iOtherValue = kTeam.AI_endWarVal(eLoopTeam);
					int iTheirValue = kLoopTeam.AI_endWarVal(eTeam);
					
					szBuffer.append( CvWString::format(SETCOLR L" %s " ENDCOLR SETCOLR L"(%d, %d)" ENDCOLR SETCOLR L" with %s " ENDCOLR  SETCOLR L"(%d, %d)\n" ENDCOLR, 
						TEXT_COLOR((iOtherValue < iTheirValue) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"),
						szWarplan.getCString(),
						TEXT_COLOR((iOtherValue < iTheirValue) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"),
						iOtherValue, kTeam.AI_getWarSuccess(eLoopTeam),
						TEXT_COLOR((iOtherValue < iTheirValue) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"),
						kLoopTeam.getName().GetCString(),
						TEXT_COLOR((iTheirValue < iOtherValue) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"),
						iTheirValue, kLoopTeam.AI_getWarSuccess(eTeam)) );
				}
			}
		}
		
		// double space if had any war
		if( kTeam.getAnyWarPlanCount(true) > 0 )
		{
			int iEnemyPowerPercent = kTeam.AI_getEnemyPowerPercent();
			szBuffer.append(CvWString::format(SETCOLR L"\nEnemy Power Percent: %d" ENDCOLR, TEXT_COLOR((iEnemyPowerPercent < 100) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), iEnemyPowerPercent));
			
		}
		if (bHadAny)
		{
			int iWarSuccessRatio = kTeam.AI_getWarSuccessCapitulationRatio();
			szBuffer.append(CvWString::format(SETCOLR L"\nWar Success Ratio: %d" ENDCOLR, TEXT_COLOR((iWarSuccessRatio > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), iWarSuccessRatio));
		}
		if (bHadAny || kTeam.getAnyWarPlanCount(true) > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(NEWLINE);
		}
		
		// show warplan values
		bHadAny = false;
		bFirst = true;
		for (int iTeamIndex = 0; iTeamIndex < MAX_TEAMS; iTeamIndex++)
		{
			TeamTypes eLoopTeam = (TeamTypes) iTeamIndex;
			CvTeamAI& kLoopTeam = GET_TEAM(eLoopTeam);
			if (eLoopTeam != eTeam && kLoopTeam.isAlive() && !kLoopTeam.isBarbarian())
			{
				WarPlanTypes eWarPlan = kTeam.AI_getWarPlan(eLoopTeam);
				if (!kTeam.isAtWar(eLoopTeam) && eWarPlan != NO_WARPLAN)
				{
					if (bFirst)
					{
						szBuffer.append(CvWString::format(SETCOLR L"Imminent War:\n" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT")));
						bFirst = false;
					}

					bHadAny = true;
					
					CvWStringBuffer szWarplan;
					GAMETEXT.getWarplanString(szWarplan, eWarPlan);
					szBuffer.append(CvWString::format(SETCOLR L" %s (%d) with %s\n" ENDCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT"),
						szWarplan.getCString(), 
						kTeam.AI_startWarVal(eLoopTeam), 
						kLoopTeam.getName().GetCString()));

				}
			}
		}
		
		// double space if had any war plan
		if (bHadAny)
		{
			szBuffer.append(NEWLINE);
		}
		
		// calculate war percentages
		float fOverallWarPercentage = 0;
		bool bAggressive = GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI);
		
		bool bIsAnyCapitalAreaAlone = kTeam.AI_isAnyCapitalAreaAlone();

		int iI;
		int iFinancialTroubleCount = 0;
		int iDaggerCount = 0;
		int iGetBetterUnitsCount = 0;
		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam)
				{
					if ( GET_PLAYER((PlayerTypes)iI).AI_isDoStrategy(AI_STRATEGY_DAGGER)
						|| GET_PLAYER((PlayerTypes)iI).AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST3)
						|| GET_PLAYER((PlayerTypes)iI).AI_isDoVictoryStrategy(AI_VICTORY_DOMINATION4) )
					{
						iDaggerCount++;
						bAggressive = true;
					}
					if (GET_PLAYER((PlayerTypes)iI).AI_isDoStrategy(AI_STRATEGY_GET_BETTER_UNITS))
					{
						iGetBetterUnitsCount++;
					}
					
					if (GET_PLAYER((PlayerTypes)iI).AI_isFinancialTrouble())
					{
						iFinancialTroubleCount++;
					}
				}
			}
		}

		// calculate unit spending for use in iTotalWarRandThreshold
		int iNumMembers = kTeam.getNumMembers();

		// if random in this range is 0, we go to war of this type (so lower numbers are higher probablity)
		// average of everyone on our team
		int iTotalWarRand;
	    int iLimitedWarRand;
	    int iDogpileWarRand;
		kTeam.AI_getWarRands( iTotalWarRand, iLimitedWarRand, iDogpileWarRand );

		int iTotalWarThreshold;
		int iLimitedWarThreshold;
		int iDogpileWarThreshold;
		kTeam.AI_getWarThresholds( iTotalWarThreshold, iLimitedWarThreshold, iDogpileWarThreshold );

		// we oppose war if half the non-dagger teammates in financial trouble
		bool bFinancesOpposeWar = false;
		if ((iFinancialTroubleCount - iDaggerCount) >= std::max(1, kTeam.getNumMembers() / 2 ))
		{
			// this can be overridden by by the pro-war booleans
			bFinancesOpposeWar = true;
		}

		// if agressive, we may start a war to get money
		bool bFinancesProTotalWar = false;
		bool bFinancesProLimitedWar = false;
		bool bFinancesProDogpileWar = false;
		if (iFinancialTroubleCount > 0)
		{
			// do we like all out wars?
			if (iDaggerCount > 0 || iTotalWarRand < 100)
			{
				bFinancesProTotalWar = true;
			}

			// do we like limited wars?
			if (iLimitedWarRand < 100)
			{
				bFinancesProLimitedWar = true;
			}
			
			// do we like dogpile wars?
			if (iDogpileWarRand < 100)
			{
				bFinancesProDogpileWar = true;
			}
		}
		bool bFinancialProWar = (bFinancesProTotalWar || bFinancesProLimitedWar || bFinancesProDogpileWar);
		
		// overall war check (quite frequently true)
		if ((iGetBetterUnitsCount - iDaggerCount) * 3 < iNumMembers * 2)
		{
			if (bFinancialProWar || !bFinancesOpposeWar)
			{
				fOverallWarPercentage = (float)std::min(100, GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIDeclareWarProb());
			}
		}

		// team power (if agressive, we use higher value)
		int iTeamPower = kTeam.getPower(true);
		if (bAggressive && kTeam.getAnyWarPlanCount(true) == 0)
		{
			iTeamPower *= 4;
			iTeamPower /= 3;
		}

		iTeamPower *= (100 - kTeam.AI_getEnemyPowerPercent());
		iTeamPower /= 100;
		
		// we will put the values into an array, then sort it for display
		int iBestPossibleMaxWarPass = MAX_INT;
		struct CvStartWarInfo
		{
			int		iStartWarValue;
			int		iNoWarAttitudeProb;
			int		iPossibleMaxWarPass;
			bool	bPossibleLimitedWar;
			bool	bPossibleDogpileWar;
			bool    bEnoughDogpilePower;

			bool	bValid;
			bool	bIsLandTarget;
			bool	bIsVictory4;
			bool	bIsAnyCapitalAreaAlone;
			bool	bAdjacentCheckPassed;
			bool	bIsMaxWarNearbyPowerRatio;
			bool	bIsMaxWarDistantPowerRatio;
		} aStartWarInfo[MAX_TEAMS];
		
		// first calculate all the values and put into array
		for (int iTeamIndex = 0; iTeamIndex < MAX_TEAMS; iTeamIndex++)
		{
			aStartWarInfo[iTeamIndex].bValid = false;
			
			TeamTypes eLoopTeam = (TeamTypes) iTeamIndex;
			CvTeamAI& kLoopTeam = GET_TEAM(eLoopTeam);
			if (eLoopTeam != eTeam && kLoopTeam.isAlive() && !kLoopTeam.isBarbarian())
			{
				WarPlanTypes eWarPlan = kTeam.AI_getWarPlan(eLoopTeam);
				if (!kTeam.isAtWar(eLoopTeam) && (eWarPlan == NO_WARPLAN))
				{
					if (kTeam.canEventuallyDeclareWar(eLoopTeam) && kTeam.isHasMet(eLoopTeam))
					{
						if( GET_TEAM(eLoopTeam).isAVassal() && !kTeam.AI_isOkayVassalTarget(eLoopTeam) )
						{
							continue;
						}

						aStartWarInfo[iTeamIndex].bValid = true;
						
						int iLoopTeamPower = kLoopTeam.getDefensivePower();
						bool bIsLandTarget = kTeam.AI_isLandTarget(eLoopTeam);
						aStartWarInfo[iTeamIndex].bIsLandTarget = bIsLandTarget;

						bool bIsVictory4 = GET_TEAM(eLoopTeam).AI_isAnyMemberDoVictoryStrategyLevel4();
						aStartWarInfo[iTeamIndex].bIsVictory4 = bIsVictory4;

						int iNoWarAttitudeProb = kTeam.AI_noWarAttitudeProb(kTeam.AI_getAttitude(eLoopTeam));
						aStartWarInfo[iTeamIndex].iNoWarAttitudeProb = iNoWarAttitudeProb;

						// total war
						aStartWarInfo[iTeamIndex].iPossibleMaxWarPass = MAX_INT;
						if (iNoWarAttitudeProb < 100 && (bFinancesProTotalWar || !bFinancesOpposeWar))
						{
							int iNoWarChance = range(iNoWarAttitudeProb - (bAggressive ? 10 : 0) - (bFinancesProTotalWar ? 10 : 0) + (20*iGetBetterUnitsCount)/iNumMembers, 0, 100);
							if (iNoWarChance < 100)
							{
								bool bIsMaxWarNearbyPowerRatio = (iLoopTeamPower < ((iTeamPower * kTeam.AI_maxWarNearbyPowerRatio()) / 100));
								bool bIsMaxWarDistantPowerRatio = (iLoopTeamPower < ((iTeamPower * kTeam.AI_maxWarDistantPowerRatio()) / 100));
								aStartWarInfo[iTeamIndex].bIsMaxWarNearbyPowerRatio = bIsMaxWarNearbyPowerRatio;
								aStartWarInfo[iTeamIndex].bIsMaxWarDistantPowerRatio = bIsMaxWarDistantPowerRatio;

								bool bAdjacentCheckPassed = true;
								int iMaxWarMinAdjacentPercent = kTeam.AI_maxWarMinAdjacentLandPercent();
								if (iMaxWarMinAdjacentPercent > 0)
								{
									int iMinAdjacentPlots = ((kTeam.getTotalLand() * iMaxWarMinAdjacentPercent) / 100);
									if (iMinAdjacentPlots > 0)
									{
										bAdjacentCheckPassed = (kTeam.AI_calculateAdjacentLandPlots(eLoopTeam) >= iMinAdjacentPlots);
									}
								}
								aStartWarInfo[iTeamIndex].bAdjacentCheckPassed = bAdjacentCheckPassed;

								// check to see which max war pass, if any is valid for this loop team
								int iPossibleMaxWarPass = MAX_INT;
								if (bIsMaxWarNearbyPowerRatio && (bAdjacentCheckPassed || bIsVictory4))
								{
									iPossibleMaxWarPass = 0;
								}
								else if (bIsMaxWarNearbyPowerRatio && (bIsLandTarget || bIsAnyCapitalAreaAlone || bIsVictory4))
								{
									iPossibleMaxWarPass = 1;
								}

								else if (bIsMaxWarDistantPowerRatio)
								{
									iPossibleMaxWarPass = 2;
								}
								aStartWarInfo[iTeamIndex].iPossibleMaxWarPass = iPossibleMaxWarPass;
								
								// if this team is valid on a lower pass, then it is the best pass
								if (iPossibleMaxWarPass < iBestPossibleMaxWarPass)
								{
									iBestPossibleMaxWarPass = iPossibleMaxWarPass;
								}
							}
						}

						// limited war
						aStartWarInfo[iTeamIndex].bPossibleLimitedWar = false;
						if (iNoWarAttitudeProb < 100 && (bFinancesProLimitedWar || !bFinancesOpposeWar))
						{
							int iNoWarChance = std::max(0, iNoWarAttitudeProb + 10 - (bAggressive ? 10 : 0) - (bFinancesProLimitedWar ? 10 : 0));
							if (iNoWarChance < 100)
							{
								bool bIsLimitedPowerRatio = (iLoopTeamPower < ((iTeamPower * kTeam.AI_limitedWarPowerRatio()) / 100));
								bool bIsAnyLoopTeamCapitalAreaAlone = kLoopTeam.AI_isAnyCapitalAreaAlone();
								
								if (bIsLimitedPowerRatio && (bIsLandTarget || (bIsAnyCapitalAreaAlone && bIsAnyLoopTeamCapitalAreaAlone)))
								{
									aStartWarInfo[iTeamIndex].bPossibleLimitedWar = true;
								}
							}
						}

						// dogpile war
						aStartWarInfo[iTeamIndex].bPossibleDogpileWar = false;
						aStartWarInfo[iTeamIndex].bEnoughDogpilePower = false;
						if (iNoWarAttitudeProb < 100 && (bFinancesProDogpileWar || !bFinancesOpposeWar) && kTeam.canDeclareWar(eLoopTeam))
						{
							int iNoWarChance = std::max(0, iNoWarAttitudeProb + 20 - (bAggressive ? 10 : 0) - (bFinancesProDogpileWar ? 10 : 0));
							if (iNoWarChance < 100)
							{
								int iDogpilePower = iTeamPower;
								for (int iTeamIndex2 = 0; iTeamIndex2 < MAX_CIV_TEAMS; iTeamIndex2++)
								{
									TeamTypes eDogpileLoopTeam = (TeamTypes) iTeamIndex2;
									CvTeamAI& kDogpileLoopTeam = GET_TEAM(eDogpileLoopTeam);
									if (kDogpileLoopTeam.isAlive())
									{
										if (eDogpileLoopTeam != eLoopTeam)
										{
											if (atWar(eDogpileLoopTeam, eLoopTeam))
											{
												iDogpilePower += kDogpileLoopTeam.getPower(false);
											}
										}
									}
								}

								bool bIsDogpilePowerRatio = (((iLoopTeamPower * 3) / 2) < iDogpilePower);
								aStartWarInfo[iTeamIndex].bPossibleDogpileWar = true;

								if (bIsDogpilePowerRatio)
								{
									aStartWarInfo[iTeamIndex].bEnoughDogpilePower = true;
								}
							}
						}
						
						// if this team can have any war, calculate the start war value
						aStartWarInfo[iTeamIndex].iStartWarValue = 0;
						if (aStartWarInfo[iTeamIndex].iPossibleMaxWarPass < MAX_INT || aStartWarInfo[iTeamIndex].bPossibleLimitedWar || aStartWarInfo[iTeamIndex].bPossibleDogpileWar)
						{
							aStartWarInfo[iTeamIndex].iStartWarValue = kTeam.AI_startWarVal(eLoopTeam);
						}
					}
				}
			}
		}

		if( bFinancesOpposeWar )
		{
			szBuffer.append(CvWString::format(SETCOLR L"## Finances oppose war%s%s%s\n" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"),
				bFinancesProTotalWar ? L", pro Total" : L"",
				bFinancesProLimitedWar ? L", pro Limited" : L"",
				bFinancesProDogpileWar ? L", pro Dogpile" : L""));
			szBuffer.append(NEWLINE);
		}

		// display total war items, sorting the list
		bHadAny = false;
		bFirst = true;
		int iBestValue;
		int iLastValue = MAX_INT;
		do
		{
			// find the highest value item left to do
			iBestValue = 0;
			for (int iTeamIndex = 0; iTeamIndex < MAX_TEAMS; iTeamIndex++)
			{
				if (aStartWarInfo[iTeamIndex].bValid && aStartWarInfo[iTeamIndex].iPossibleMaxWarPass < MAX_INT)
				{
					if (aStartWarInfo[iTeamIndex].iStartWarValue > iBestValue && aStartWarInfo[iTeamIndex].iStartWarValue < iLastValue)
					{
						iBestValue = aStartWarInfo[iTeamIndex].iStartWarValue;
					}
				}
			}
			
			// did we find one?
			if (iBestValue > 0)
			{
				// setup for next loop
				iLastValue = iBestValue;

				// now display every team that has that value
				for (int iTeamIndex = 0; iTeamIndex < MAX_TEAMS; iTeamIndex++)
				{
					if (aStartWarInfo[iTeamIndex].bValid && aStartWarInfo[iTeamIndex].iStartWarValue == iBestValue)
					{
						CvTeamAI& kLoopTeam = GET_TEAM((TeamTypes) iTeamIndex);

						if (bFirst)
						{
							float fMaxWarPercentage = ((fOverallWarPercentage * (iTotalWarThreshold + 1)) / iTotalWarRand);
							szBuffer.append(CvWString::format(SETCOLR L"%.2f%% [%d/%d] Total War:\n" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT"), fMaxWarPercentage, (iTotalWarThreshold + 1), iTotalWarRand));
							bFirst = false;
						}

						bHadAny = true;
						
						int iNoWarChance = std::max(0, aStartWarInfo[iTeamIndex].iNoWarAttitudeProb - (bAggressive ? 10 : 0) - (bFinancesProTotalWar ? 10 : 0));
						int iTeamWarPercentage = (100 - iNoWarChance);
						
						if (aStartWarInfo[iTeamIndex].iPossibleMaxWarPass <= iBestPossibleMaxWarPass)
						{
							szBuffer.append(CvWString::format(SETCOLR L" %d%% %s%s war (%d) with %s\n" ENDCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"),
								iTeamWarPercentage,
								(aStartWarInfo[iTeamIndex].bIsVictory4) ? L"**" : L"",
								(aStartWarInfo[iTeamIndex].bIsLandTarget) ? L"land" : L"sea",
								aStartWarInfo[iTeamIndex].iStartWarValue, 
								kLoopTeam.getName().GetCString()));
						}
						else
						{
							szBuffer.append(CvWString::format(SETCOLR L" (%d%% %s%s war (%d) with %s [%s%s])\n" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"),
								iTeamWarPercentage,
								(aStartWarInfo[iTeamIndex].bIsVictory4) ? L"**" : L"",
								(aStartWarInfo[iTeamIndex].bIsLandTarget) ? L"land" : L"sea",
								aStartWarInfo[iTeamIndex].iStartWarValue, 
								kLoopTeam.getName().GetCString(),
								(iBestPossibleMaxWarPass == 0) ? ((aStartWarInfo[iTeamIndex].bIsMaxWarNearbyPowerRatio) ? L"not adjacent" : L"low power") : L"",
								(iBestPossibleMaxWarPass == 1) ? ((aStartWarInfo[iTeamIndex].bIsMaxWarNearbyPowerRatio) ? L"not land" : L"low power") : L""));
						}
					}
				}
			}
		}
		while (iBestValue > 0);

		// double space if had any war
		if (bHadAny)
		{
			szBuffer.append(NEWLINE);
		}
		
		// display limited war items, sorting the list
		bHadAny = false;
		bFirst = true;
		iLastValue = MAX_INT;
		do
		{
			// find the highest value item left to do
			iBestValue = 0;
			for (int iTeamIndex = 0; iTeamIndex < MAX_TEAMS; iTeamIndex++)
			{
				if (aStartWarInfo[iTeamIndex].bValid && aStartWarInfo[iTeamIndex].bPossibleLimitedWar)
				{
					if (aStartWarInfo[iTeamIndex].iStartWarValue > iBestValue && aStartWarInfo[iTeamIndex].iStartWarValue < iLastValue)
					{
						iBestValue = aStartWarInfo[iTeamIndex].iStartWarValue;
					}
				}
			}
			
			// did we find one?
			if (iBestValue > 0)
			{
				// setup for next loop
				iLastValue = iBestValue;

				// now display every team that has that value
				for (int iTeamIndex = 0; iTeamIndex < MAX_TEAMS; iTeamIndex++)
				{
					if (aStartWarInfo[iTeamIndex].bValid && aStartWarInfo[iTeamIndex].iStartWarValue == iBestValue)
					{
						if (bFirst)
						{
							float fLimitedWarPercentage = (fOverallWarPercentage * (iLimitedWarThreshold + 1)) / iLimitedWarRand;
							szBuffer.append(CvWString::format(SETCOLR L"%.2f%% Limited War:\n" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT"), fLimitedWarPercentage));
							bFirst = false;
						}

						bHadAny = true;
						
						int iNoWarChance = std::max(0, aStartWarInfo[iTeamIndex].iNoWarAttitudeProb + 10 - (bAggressive ? 10 : 0) - (bFinancesProLimitedWar ? 10 : 0));
						int iTeamWarPercentage = (100 - iNoWarChance);
						
						szBuffer.append(CvWString::format(SETCOLR L" %d%% %s%s war (%d) with %s\n" ENDCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"),
							iTeamWarPercentage,
							(aStartWarInfo[iTeamIndex].bIsVictory4) ? L"**" : L"",
							(aStartWarInfo[iTeamIndex].bIsLandTarget) ? L"land" : L"sea",
							aStartWarInfo[iTeamIndex].iStartWarValue, 
							GET_TEAM((TeamTypes) iTeamIndex).getName().GetCString()));
					}
				}
			}
		}
		while (iBestValue > 0);

		// double space if had any war
		if (bHadAny)
		{
			szBuffer.append(NEWLINE);
		}
		
		// display dogpile war items, sorting the list
		bHadAny = false;
		bFirst = true;
		iLastValue = MAX_INT;
		do
		{
			// find the highest value item left to do
			iBestValue = 0;
			for (int iTeamIndex = 0; iTeamIndex < MAX_TEAMS; iTeamIndex++)
			{
				if (aStartWarInfo[iTeamIndex].bValid && aStartWarInfo[iTeamIndex].bPossibleDogpileWar)
				{
					if (aStartWarInfo[iTeamIndex].iStartWarValue > iBestValue && aStartWarInfo[iTeamIndex].iStartWarValue < iLastValue)
					{
						iBestValue = aStartWarInfo[iTeamIndex].iStartWarValue;
					}
				}
			}
			
			// did we find one?
			if (iBestValue > 0)
			{
				// setup for next loop
				iLastValue = iBestValue;

				// now display every team that has that value
				for (int iTeamIndex = 0; iTeamIndex < MAX_TEAMS; iTeamIndex++)
				{
					if (aStartWarInfo[iTeamIndex].bValid && aStartWarInfo[iTeamIndex].iStartWarValue == iBestValue)
					{
						if (bFirst)
						{
							float fDogpileWarPercentage = (fOverallWarPercentage * (iDogpileWarThreshold + 1)) / iDogpileWarRand;
							szBuffer.append(CvWString::format(SETCOLR L"%.2f%% Dogpile War:\n" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT"), fDogpileWarPercentage));
							bFirst = false;
						}

						bHadAny = true;
						
						int iNoWarChance = std::max(0, aStartWarInfo[iTeamIndex].iNoWarAttitudeProb + 20 - (bAggressive ? 10 : 0) - (bFinancesProDogpileWar ? 10 : 0));
						int iTeamWarPercentage = (100 - iNoWarChance);
						
						if( aStartWarInfo[iTeamIndex].bEnoughDogpilePower )
						{
							if( (aStartWarInfo[iTeamIndex].bIsLandTarget) || (aStartWarInfo[iTeamIndex].bIsVictory4) )
							{
								szBuffer.append(CvWString::format(SETCOLR L" %d%% %s%s war (%d) with %s\n" ENDCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"),
									iTeamWarPercentage,
									(aStartWarInfo[iTeamIndex].bIsVictory4) ? L"**" : L"",
									L"land",
									aStartWarInfo[iTeamIndex].iStartWarValue, 
									GET_TEAM((TeamTypes) iTeamIndex).getName().GetCString()));
							}
							else
							{
								szBuffer.append(CvWString::format(SETCOLR L" %d%% %s%s war (%d) with %s\n" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"),
									iTeamWarPercentage,
									(aStartWarInfo[iTeamIndex].bIsVictory4) ? L"**" : L"",
									L"sea",
									aStartWarInfo[iTeamIndex].iStartWarValue, 
									GET_TEAM((TeamTypes) iTeamIndex).getName().GetCString()));
							}
						}
						else
						{
							szBuffer.append(CvWString::format(SETCOLR L" Lack power for %s%s war (%d) with %s\n" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"),
								(aStartWarInfo[iTeamIndex].bIsVictory4) ? L"**" : L"",
								(aStartWarInfo[iTeamIndex].bIsLandTarget) ? L"land" : L"sea",
								aStartWarInfo[iTeamIndex].iStartWarValue, 
								GET_TEAM((TeamTypes) iTeamIndex).getName().GetCString()));
						}
					}
				}
			}
		}
		while (iBestValue > 0);

		// double space if had any war
		if (bHadAny)
		{
			szBuffer.append(NEWLINE);
		}

		if (gDLL->shiftKey())
		{
			return;
		}
	}


	//	Show score info instead if we are trying to contact ourselves...
	if ( eActivePlayer == ePlayer || (gDLL->ctrlKey() && gDLL->getChtLvl() > 0) )
	{
		parseScoreHelp(widgetDataStruct, szBuffer);
		return;
	}

/************************************************************************************************/
/* REVOLUTION_MOD                         02/10/08                                jdog5000      */
/*                                                                                              */
/* Minor Civs                                                                                   */
/************************************************************************************************/
	if( GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).isMinorCiv() )
	{
		szBuffer.append(gDLL->getText("TXT_KEY_REV_CONTACT_MINOR", GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getCivilizationDescription()));
		return;
	}
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/

/************************************************************************************************/
/* REVOLUTION_MOD                         01/01/08                                jdog5000      */
/*                                                                                              */
/* Dynamic Civ Names                                                                            */
/************************************************************************************************/
	// Make civs full name show up in Score board
/* original BS
	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CONTACT_LEADER", GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getNameKey(), GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getCivilizationShortDescription()));
*/
	szBuffer.append(gDLL->getText("TXT_KEY_MISC_CONTACT_LEADER", GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getNameKey(), GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getCivilizationDescription()));
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/

	szBuffer.append(NEWLINE);
	GAMETEXT.parsePlayerTraits(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);




	if (!(kActiveTeam.isHasMet(eTeam)))
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HAVENT_MET_CIV"));
	}

/************************************************************************************************/
/* Afforess                                     12/7/09                                         */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	szBuffer.append(NEWLINE);
	szBuffer.append(gDLL->getText(kPlayer.hasFixedBorders() ? "TXT_KEY_PLAYER_HAS_FIXED_BORDERS" : "TXT_KEY_PLAYER_HAS_NOT_FIXED_BORDERS"));
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	
	if( (kActiveTeam.isHasMet(eTeam)) || GC.getGameINLINE().isDebugMode() )
	{
		if (!(kPlayer.isHuman()))
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_HANDICAP_LEVEL"));
			szBuffer.append(gDLL->getText(GC.getHandicapInfo((HandicapTypes)kPlayer.getHandicapType()).getDescription()));
			if (!(kPlayer.AI_isWillingToTalk(eActivePlayer)))
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_REFUSES_TO_TALK"));
			}

			if ( !((gDLL->altKey() || gDLL->ctrlKey()) && gDLL->getChtLvl() > 0) )
			{
// BUG - start
			// moved up here to match other leaderhead hovers
				szBuffer.append(NEWLINE);
				GAMETEXT.getEspionageString(szBuffer, ePlayer, eActivePlayer);

				//szBuffer.append(NEWLINE);
				GAMETEXT.getAttitudeString(szBuffer, ePlayer, eActivePlayer);

			// espionage moved above
			// CTRL instructions moved below
// BUG - end
			}
		}
// BUG - Espionage for Humans - start
		else
		{
			szBuffer.append(NEWLINE);
			GAMETEXT.getEspionageString(szBuffer, ePlayer, eActivePlayer);
		}
// BUG - Espionage for Humans - end

		if ( !((gDLL->altKey() || gDLL->ctrlKey()) && gDLL->getChtLvl() > 0) )
		{

// BUG - Deals in Scoreboard - start
			if (gDLL->ctrlKey() && ePlayer != eActivePlayer)
			{
				GAMETEXT.getActiveDealsString(szBuffer, ePlayer, eActivePlayer);
			}
// BUG - Deals in Scoreboard - end

			if (eTeam != eActiveTeam)
			{
// BUG - Relations in Scoreboard - start
				GAMETEXT.getAllRelationsString(szBuffer, eTeam);
// BUG - Relations in Scoreboard - end
			}


// BUG - start
		// moved from above to organize the hover text
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_CTRL_TRADE"));
// BUG - end
		}

		if (eTeam != eActiveTeam)
		{
			if( !(kActiveTeam.isAtWar(eTeam)))
			{
				if (kActiveTeam.canDeclareWar(eTeam))
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_ALT_DECLARE_WAR"));
				}
				else
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_CANNOT_DECLARE_WAR"));
				}

				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_SHIFT_ALT_PREPARE_WAR"));
			}
		}
	}

	if (kPlayer.isHuman())
	{
//		szBuffer += "\n(<SHIFT> to Send Chat Message)";
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_SHIFT_SEND_CHAT"));
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
}


void CvDLLWidgetData::parseConvertHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		if (widgetDataStruct.m_iData1 == NO_RELIGION)
		{
//			szBuffer.Format(L"No State %c", gDLL->getSymbolID(RELIGION_CHAR));
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_NO_STATE_REL"));
		}
		else
		{
//			szBuffer.Format(L"Convert to %s", GC.getReligionInfo((ReligionTypes) widgetDataStruct.m_iData1).getDescription());
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_CONVERT_TO_REL", GC.getReligionInfo((ReligionTypes) widgetDataStruct.m_iData1).getTextKeyWide()));
		}
	}
	else
	{
		GAMETEXT.setConvertHelp(szBuffer, GC.getGameINLINE().getActivePlayer(), (ReligionTypes)widgetDataStruct.m_iData1);
	}
}


void CvDLLWidgetData::parseRevolutionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData1 != 0)
	{
//		szBuffer = "Change Civics";
		szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CHANGE_CIVICS"));
	}
	else
	{
		GAMETEXT.setRevolutionHelp(szBuffer, GC.getGameINLINE().getActivePlayer());
	}
}

/*void CvDLLWidgetData::parsePopupQueue(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer)
{
	PopupEventTypes eEvent;

	if ( m_pPopup || m_pScreen )
	{
		if ( m_pPopup )
		{
			eEvent = m_pPopup->getPopupType();
		}
		else if ( m_pScreen )
		{
			eEvent = m_pScreen->getPopupType();
		}

		switch (eEvent)
		{
			case POPUPEVENT_NONE:
//				szBuffer = "***NO MOUSEOVER TEXT***.  Click to activate popup.";
				szBuffer = gDLL->getText("TXT_KEY_MISC_NO_MOUSEOVER_TEXT");
				break;

			case POPUPEVENT_PRODUCTION:
//				szBuffer = "Production Complete";
				szBuffer = gDLL->getText("TXT_KEY_MISC_PRODUCTION_COMPLETE");
				break;

			case POPUPEVENT_TECHNOLOGY:
//				szBuffer = "Technology Research Complete";
				szBuffer = gDLL->getText("TXT_KEY_MISC_TECH_RESEARCH_COMPLETE");
				break;

			case POPUPEVENT_RELIGION:
//				szBuffer = "New Religion Discovered";
				szBuffer = gDLL->getText("TXT_KEY_MISC_NEW_REL_DISCOVERED");
				break;

			case POPUPEVENT_WARNING:
//				szBuffer = "Warning!!!";
				szBuffer = gDLL->getText("TXT_KEY_MISC_WARNING");
				break;

			case POPUPEVENT_CIVIC:
//				szBuffer = "New Civic Accessible";
				szBuffer = gDLL->getText("TXT_KEY_MISC_NEW_CIVIC_ACCESSIBLE");
				break;
		}
	}
}*/

void CvDLLWidgetData::parseAutomateCitizensHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (pHeadSelectedCity->isCitizensAutomated())
		{
//			szBuffer = "Turn Off Citizen Automation";
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_OFF_CITIZEN_AUTO"));
		}
		else
		{
//			szBuffer = "Turn On Citizen Automation";
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_ON_CITIZEN_AUTO"));
		}
	}
}

void CvDLLWidgetData::parseAutomateProductionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (pHeadSelectedCity->isProductionAutomated())
		{
//			szBuffer = "Turn Off Production Automation";
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_OFF_PROD_AUTO"));
		}
		else
		{
//			szBuffer = "Turn On Production Automation";
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_ON_PROD_AUTO"));
		}
	}
}

void CvDLLWidgetData::parseEmphasizeHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	szBuffer.clear();

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (pHeadSelectedCity->AI_isEmphasize((EmphasizeTypes)widgetDataStruct.m_iData1))
		{
//			szBuffer += "Turn Off ";
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_TURN_OFF"));
		}
		else
		{
//			szBuffer += "Turn On ";
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_TURN_ON"));
		}
	}

	szBuffer.append(GC.getEmphasizeInfo((EmphasizeTypes)widgetDataStruct.m_iData1).getDescription());
}

void CvDLLWidgetData::parseBuildingFilterHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	BuildingFilterTypes eFilter = (BuildingFilterTypes) widgetDataStruct.m_iData1;
	bool bIsActive = false;

	if (widgetDataStruct.m_iData2 == - 1)
	{
		CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
		if (pHeadSelectedCity != NULL)
		{
			bIsActive = pHeadSelectedCity->getBuildingListFilterActive(eFilter);
		}
	}
	else
	{
		bIsActive = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getBuildingListFilterActive(eFilter);
	}

	szBuffer.clear();

	if (bIsActive)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_TURN_OFF"));
	}
	else
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_TURN_ON"));
	}
	szBuffer.append(NEWLINE);

	switch (eFilter)
	{
		case BUILDING_FILTER_HIDE_UNBUILDABLE:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_FILTER_SHOW_BUILDABLE"));
			break;
		case BUILDING_FILTER_SHOW_SCIENCE:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_FILTER_SHOW_SCIENCE"));
			break;
		case BUILDING_FILTER_SHOW_ESPIONAGE:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_FILTER_SHOW_ESPIONAGE"));
			break;
		case BUILDING_FILTER_SHOW_CULTURE:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_FILTER_SHOW_CULTURE"));
			break;
		case BUILDING_FILTER_SHOW_GOLD:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_FILTER_SHOW_GOLD"));
			break;
		case BUILDING_FILTER_SHOW_FOOD:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_FILTER_SHOW_FOOD"));
			break;
		case BUILDING_FILTER_SHOW_PRODUCTION:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_FILTER_SHOW_PRODUCTION"));
			break;
		case BUILDING_FILTER_SHOW_HAPPINESS:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_FILTER_SHOW_HAPPINESS"));
			break;
		case BUILDING_FILTER_SHOW_HEALTH:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_FILTER_SHOW_HEALTH"));
			break;
		case BUILDING_FILTER_SHOW_MILITARY:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_FILTER_SHOW_MILITARY"));
			break;
		case BUILDING_FILTER_SHOW_CITY_DEFENSE:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_FILTER_SHOW_CITY_DEFENSE"));
			break;
		case BUILDING_FILTER_HIDE_UNHAPPINESS:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_FILTER_HIDE_UNHAPPINESS"));
			break;
		case BUILDING_FILTER_HIDE_UNHEALTHINESS:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_FILTER_HIDE_UNHEALTHINESS"));
			break;
		case BUILDING_FILTER_HIDE_GREAT_WONDER:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_FILTER_HIDE_GREAT_WONDER"));
			break;
		case BUILDING_FILTER_HIDE_NATIONAL_WONDER:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_FILTER_HIDE_NATIONAL_WONDER"));
			break;
		case BUILDING_FILTER_HIDE_NORMAL:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_FILTER_HIDE_NORMAL"));
			break;
		case BUILDING_FILTER_SHOW_CRIME:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_FILTER_SHOW_CRIME"));
			break;
		case BUILDING_FILTER_SHOW_FLAMMABILITY:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_FILTER_SHOW_FLAMMABILITY"));
			break;
	}
}

void CvDLLWidgetData::parseBuildingGroupingHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	szBuffer.clear();
	szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_GROUPING"));
}

void CvDLLWidgetData::parseBuildingSortHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	szBuffer.clear();
	szBuffer.append(gDLL->getText("TXT_KEY_LIST_BUILDING_SORTING"));
}

void CvDLLWidgetData::parseUnitFilterHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	UnitFilterTypes eFilter = (UnitFilterTypes) widgetDataStruct.m_iData1;
	bool bIsActive = false;
	
	szBuffer.clear();

	if (widgetDataStruct.m_iData2 == - 1)
	{
		CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
		if (pHeadSelectedCity != NULL)
		{
			bIsActive = pHeadSelectedCity->getUnitListFilterActive(eFilter);
		}
	}
	else
	{
		bIsActive = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getUnitListFilterActive(eFilter);
	}

	if (bIsActive)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_TURN_OFF"));
	}
	else
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_TURN_ON"));
	}

	szBuffer.append(NEWLINE);

	switch (eFilter)
	{
		case UNIT_FILTER_HIDE_UNBUILDABLE:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_UNIT_FILTER_SHOW_BUILDABLE"));
			break;
		case UNIT_FILTER_HIDE_LIMITED:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_UNIT_FILTER_HIDE_LIMITED"));
			break;
		case UNIT_FILTER_SHOW_LAND:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_UNIT_FILTER_SHOW_LAND"));
			break;
		case UNIT_FILTER_SHOW_AIR:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_UNIT_FILTER_SHOW_AIR"));
			break;
		case UNIT_FILTER_SHOW_WATER:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_UNIT_FILTER_SHOW_WATER"));
			break;
		case UNIT_FILTER_SHOW_WORKERS:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_UNIT_FILTER_SHOW_WORKERS"));
			break;
		case UNIT_FILTER_SHOW_CIVILIAN:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_UNIT_FILTER_SHOW_CIVILIAN"));
			break;
		case UNIT_FILTER_SHOW_SIEGE:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_UNIT_FILTER_SHOW_SIEGE"));
			break;
		case UNIT_FILTER_SHOW_MOUNTED:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_UNIT_FILTER_SHOW_MOUNTED"));
			break;
		case UNIT_FILTER_SHOW_HEROES:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_UNIT_FILTER_SHOW_HEROES"));
			break;
		case UNIT_FILTER_SHOW_MILITARY:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_UNIT_FILTER_SHOW_MILITARY"));
			break;
		case UNIT_FILTER_SHOW_DEFENSE:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_UNIT_FILTER_SHOW_DEFENSE"));
			break;
		case UNIT_FILTER_SHOW_MISSIONARY:
			szBuffer.append(gDLL->getText("TXT_KEY_LIST_UNIT_FILTER_SHOW_MISSIONARY"));
			break;
	}
}

void CvDLLWidgetData::parseUnitGroupingHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	szBuffer.clear();
	szBuffer.append(gDLL->getText("TXT_KEY_LIST_UNIT_GROUPING"));
}

void CvDLLWidgetData::parseUnitSortHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	szBuffer.clear();
	szBuffer.append(gDLL->getText("TXT_KEY_LIST_UNIT_SORTING"));
}


void CvDLLWidgetData::parseTradeItem(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvWString szTempBuffer;
	TradeData item;
	PlayerTypes eWhoFrom = NO_PLAYER;
	PlayerTypes eWhoTo = NO_PLAYER;
	DenialTypes eDenial;
	PlayerTypes eWhoDenies;
/************************************************************************************************/
/* Afforess	                  Start		 06/16/10                                               */
/*                                                                                              */
/* Advanced Diplomacy                                                                           */
/************************************************************************************************/
	CvUnit* pUnit = NULL;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	szBuffer.clear();

	if (widgetDataStruct.m_bOption)
	{
		if ( gDLL->isDiplomacy())
		{
			eWhoFrom = (PlayerTypes) gDLL->getDiplomacyPlayer();
		}
		else if (gDLL->isMPDiplomacyScreenUp())
		{
			eWhoFrom = (PlayerTypes) gDLL->getMPDiplomacyPlayer();
		}
		eWhoTo = GC.getGameINLINE().getActivePlayer();
	}
	else
	{
		eWhoFrom = GC.getGameINLINE().getActivePlayer();
		if ( gDLL->isDiplomacy() )
		{
			eWhoTo = (PlayerTypes) gDLL->getDiplomacyPlayer();
		}
		else if (gDLL->isMPDiplomacyScreenUp())
		{
			eWhoTo = (PlayerTypes) gDLL->getMPDiplomacyPlayer();
		}
	}

	eWhoDenies = eWhoFrom;

	if ((eWhoFrom != NO_PLAYER) && (eWhoTo != NO_PLAYER))
	{
		//	Data1 is the heading
		switch (widgetDataStruct.m_iData1)
		{
		case TRADE_TECHNOLOGIES:
			GAMETEXT.setTechHelp(szBuffer, ((TechTypes)widgetDataStruct.m_iData2));
			eWhoDenies = (widgetDataStruct.m_bOption ? eWhoFrom : eWhoTo);
			break;
		case TRADE_RESOURCES:
			GAMETEXT.setBonusHelp(szBuffer, ((BonusTypes)widgetDataStruct.m_iData2));
			eWhoDenies = (widgetDataStruct.m_bOption ? eWhoFrom : eWhoTo);
			break;
		case TRADE_CITIES:
			szBuffer.assign(gDLL->getText("TXT_KEY_TRADE_CITIES"));
			eWhoDenies = (widgetDataStruct.m_bOption ? eWhoFrom : eWhoTo);
			break;
		case TRADE_PEACE:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_MAKE_PEACE", GET_TEAM(GET_PLAYER(eWhoFrom).getTeam()).getName().GetCString(), GET_TEAM((TeamTypes)widgetDataStruct.m_iData2).getName().GetCString()));
			break;
		case TRADE_WAR:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_MAKE_WAR", GET_TEAM(GET_PLAYER(eWhoFrom).getTeam()).getName().GetCString(), GET_TEAM((TeamTypes)widgetDataStruct.m_iData2).getName().GetCString()));
			break;
		case TRADE_EMBARGO:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_STOP_TRADING", GET_TEAM(GET_PLAYER(eWhoFrom).getTeam()).getName().GetCString(), GET_TEAM((TeamTypes)widgetDataStruct.m_iData2).getName().GetCString()));
			break;
		case TRADE_CIVIC:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_ADOPT_CIVIC", GC.getCivicInfo((CivicTypes)widgetDataStruct.m_iData2).getDescription()));
			break;
		case TRADE_RELIGION:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_CONVERT_RELIGION", GC.getReligionInfo((ReligionTypes)widgetDataStruct.m_iData2).getDescription()));
			break;
		case TRADE_GOLD:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_GOLD"));
			break;
		case TRADE_GOLD_PER_TURN:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_GOLD_PER_TURN"));
			break;
		case TRADE_MAPS:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_MAPS"));
			break;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/07/09                                jdog5000      */
/*                                                                                              */
/* Diplomacy                                                                                    */
/************************************************************************************************/
		case TRADE_SURRENDER:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_CAPITULATE"));
			eWhoDenies = (widgetDataStruct.m_bOption ? eWhoFrom : NO_PLAYER);
			break;
		case TRADE_VASSAL:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_VASSAL"));
			eWhoDenies = (widgetDataStruct.m_bOption ? eWhoFrom : NO_PLAYER);
			break;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
		case TRADE_OPEN_BORDERS:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_OPEN_BORDERS"));
			break;
		case TRADE_DEFENSIVE_PACT:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_DEFENSIVE_PACT"));
			break;
		case TRADE_PERMANENT_ALLIANCE:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_PERMANENT_ALLIANCE"));
			break;
		case TRADE_PEACE_TREATY:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_PEACE_TREATY", getTreatyLength()));
			break;
/************************************************************************************************/
/* Afforess	                  Start		 06/16/10                                               */
/*                                                                                              */
/* Advanced Diplomacy                                                                           */
/************************************************************************************************/
		case TRADE_RITE_OF_PASSAGE:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_LIMITED_BORDERS"));
			break;
		case TRADE_FREE_TRADE_ZONE:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_FREE_TRADE_ZONE"));
			break;
		case TRADE_WORKER:
			szBuffer.assign(gDLL->getText("TXT_KEY_TRADE_WORKER"));
			eWhoDenies = (widgetDataStruct.m_bOption ? eWhoFrom : eWhoTo);
			break;
		case TRADE_MILITARY_UNIT:
			pUnit = GET_PLAYER(eWhoFrom).getUnit(widgetDataStruct.m_iData2);
            GAMETEXT.setUnitHelp(szBuffer, pUnit, true);
			eWhoDenies = (widgetDataStruct.m_bOption ? eWhoFrom : eWhoTo);
			break;
		case TRADE_EMBASSY:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_EMBASSY", -25));
			break;
		case TRADE_CONTACT:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_CONTACT", GET_TEAM(GET_PLAYER(eWhoFrom).getTeam()).getName().GetCString(), GET_TEAM((TeamTypes)widgetDataStruct.m_iData2).getName().GetCString()));
			break;
		case TRADE_CORPORATION:
			GAMETEXT.setCorporationHelp(szBuffer, ((CorporationTypes)widgetDataStruct.m_iData2));
			eWhoDenies = (widgetDataStruct.m_bOption ? eWhoFrom : eWhoTo);
			break;
		case TRADE_SECRETARY_GENERAL_VOTE:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_SECRETARY_GENERAL", GET_PLAYER(eWhoFrom).getNameKey(), GET_PLAYER(eWhoTo).getNameKey(), GC.getVoteSourceInfo((VoteSourceTypes)widgetDataStruct.m_iData2).getDescription()));
			break;
		case TRADE_WAR_REPARATIONS:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_WAR_REPARATIONS"));
			break;
		case TRADE_CEASE_RELATIONS:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_CEASE_RELATIONS", GET_TEAM(GET_PLAYER(eWhoFrom).getTeam()).getName().GetCString(), GET_TEAM((TeamTypes)widgetDataStruct.m_iData2).getName().GetCString(), GC.getGameINLINE().getCeaseRelationsTurnAmt()));
			break;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		}

		setTradeItem(&item, ((TradeableItems)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2);

		eDenial = GET_PLAYER(eWhoFrom).getTradeDenial(eWhoTo, item);

		if (eDenial != NO_DENIAL)
		{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/07/09                                jdog5000      */
/*                                                                                              */
/* Diplomacy                                                                                    */
/************************************************************************************************/
			if( eWhoDenies == NO_PLAYER )
			{
				switch(eDenial)
				{
				case DENIAL_POWER_US:
					eDenial = DENIAL_POWER_YOU;
					break;
				case DENIAL_POWER_YOU:
					eDenial = DENIAL_POWER_US;
					break;
				case DENIAL_WAR_NOT_POSSIBLE_US:
					eDenial = DENIAL_WAR_NOT_POSSIBLE_YOU;
					break;
				case DENIAL_WAR_NOT_POSSIBLE_YOU:
					eDenial = DENIAL_WAR_NOT_POSSIBLE_US;
					break;
				case DENIAL_PEACE_NOT_POSSIBLE_US:
					eDenial = DENIAL_PEACE_NOT_POSSIBLE_YOU;
					break;
				case DENIAL_PEACE_NOT_POSSIBLE_YOU:
					eDenial = DENIAL_PEACE_NOT_POSSIBLE_US;
					break;
				default :
					break;
				}
				szTempBuffer.Format(L"%s: " SETCOLR L"%s" ENDCOLR, GET_PLAYER(eWhoTo).getName(), TEXT_COLOR("COLOR_WARNING_TEXT"), GC.getDenialInfo(eDenial).getDescription());
			}
			else
			{
				szTempBuffer.Format(L"%s: " SETCOLR L"%s" ENDCOLR, GET_PLAYER(eWhoDenies).getName(), TEXT_COLOR("COLOR_WARNING_TEXT"), GC.getDenialInfo(eDenial).getDescription());
			}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}
	}
}


void CvDLLWidgetData::parseUnitModelHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvUnit* pHeadSelectedUnit;

	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (pHeadSelectedUnit != NULL)
	{
		GAMETEXT.setUnitHelp(szBuffer, pHeadSelectedUnit);
	}
}


void CvDLLWidgetData::parseFlagHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvWString szTempBuffer;

/************************************************************************************************/
/* Afforess: Rise of Mankind                    6/15/10                               	         */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	// Show the active player's civ's current name when mousing over flag
	szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivilizationDescription());
	szBuffer.append(szTempBuffer);
	szBuffer.append(NEWLINE);

	// BTS Version
	float fVersion = GC.getDefineINT("CIV4_VERSION") / 100.0f;
	szTempBuffer.Format(SETCOLR L"Beyond the Sword %0.2f" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), fVersion);
	szBuffer.append(szTempBuffer);
	CvWString szBugNameAndVersion;
	PYTHON_CALL_FUNCTION4(__FUNCTION__, PYBugModule, "getModNameAndVersion", NULL, &szBugNameAndVersion);
	szTempBuffer.Format(NEWLINE SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_YELLOW"), szBugNameAndVersion.c_str());
	szBuffer.append(szTempBuffer);
	CvWString szBULLETICON;
	szBULLETICON.Format(NEWLINE L"%c", gDLL->getSymbolID(BULLET_CHAR));
	szBuffer.append(szBULLETICON);
	// BUG DLL version
	szTempBuffer.Format(SETCOLR L"%s %s [Build %s]" ENDCOLR, TEXT_COLOR("COLOR_WHITE"), BUG_DLL_NAME, BUG_DLL_VERSION, BUG_DLL_BUILD);
	szBuffer.append(szTempBuffer);

	szBuffer.append(szBULLETICON);
	if ( GC.getLoadedInitCore().getGameSaveSvnRev() == -1 )
	{
		szTempBuffer.Format(L"%S %d",
							"A New Dawn DLL build", GC.getLoadedInitCore().getSvnRev());
	}
	else
	{
		szTempBuffer.Format(L"%S %d (game saved from build %d)",
							"A New Dawn build", GC.getLoadedInitCore().getSvnRev(), GC.getLoadedInitCore().getGameSaveSvnRev());
	}
	szBuffer.append(szTempBuffer);

	szBuffer.append(szBULLETICON);
	szTempBuffer.Format(L"%S", "Rise of Mankind v2.92");
	szBuffer.append(szTempBuffer);
	
	szBuffer.append(szBULLETICON);
	szTempBuffer.Format(L"%S", "RevolutionDCM v2.80 pre-release");
	szBuffer.append(szTempBuffer);	
	
	szBuffer.append(szBULLETICON);
	szTempBuffer.Format(L"%S", "Better BTS AI v1.01b");
	szBuffer.append(szTempBuffer);
	
	szBuffer.append(szBULLETICON);
	szTempBuffer.Format(L"%S", "Unofficial 3.19 Patch v1.50+");
	szBuffer.append(szTempBuffer);
	
// BUG - Version Info - end
	if (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).isModderOption(MODDEROPTION_SHOW_TRAITS_FLAG))
	{
		// separator line
		szBuffer.append(NEWLINE L"==============================" NEWLINE);
		szBuffer.append(NEWLINE);
		GAMETEXT.parseLeaderTraits(szBuffer, GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getLeaderType(), GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivilizationType());
	}
	
	// Player properties
	szBuffer.append(NEWLINE L"==============================" NEWLINE);
	GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getProperties()->buildDisplayString(szBuffer);
	szBuffer.append(NEWLINE L"==============================" NEWLINE);
	GET_TEAM(GC.getGameINLINE().getActiveTeam()).getProperties()->buildDisplayString(szBuffer);
	szBuffer.append(NEWLINE L"==============================" NEWLINE);
	GC.getGameINLINE().getProperties()->buildDisplayString(szBuffer);
	
/************************************************************************************************/
/* Afforess                                    END                                              */
/************************************************************************************************/
}


void CvDLLWidgetData::parseMaintenanceHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	wchar szTempBuffer[1024];
	int iMaintenanceValue;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (pHeadSelectedCity->isWeLoveTheKingDay())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_WE_LOVE_KING_MAINT"));
		}
		else
		{
			//		szBuffer = "Maintenance represents the total cost of governing this city.\n";
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_MAINT_INFO"));
			szBuffer.append(NEWLINE);
/************************************************************************************************/
/* Afforess	                  Start		 09/12/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			iMaintenanceValue = GC.getEraInfo(GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).getCurrentEra()).getInitialCityMaintenancePercent();
			if (iMaintenanceValue != 0 && !pHeadSelectedCity->isDisorder() && !GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).isAnarchy())
			{
				CvWString szMaint = CvWString::format(L"%d.%02d", iMaintenanceValue/100, iMaintenanceValue%100);
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_BASE_MAINTENANCE", szMaint.GetCString()));
			}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			iMaintenanceValue = pHeadSelectedCity->calculateDistanceMaintenanceTimes100();
			if (iMaintenanceValue != 0)
			{
				//			swprintf(szTempBuffer, "\n%s%d%c: %s", ((iMaintenanceValue > 0) ?  "+" : ""), iMaintenanceValue, GC.getCommerceInfo(COMMERCE_GOLD).getChar(), ((GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).getNumGovernmentCenters() > 0) ? "Distance from Palace" : "No Palace Penalty"));
				CvWString szMaint = CvWString::format(L"%d.%02d", iMaintenanceValue/100, iMaintenanceValue%100);
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_NUM_MAINT_FLOAT", szMaint.GetCString()) + ((GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).getNumGovernmentCenters() > 0) ? gDLL->getText("TXT_KEY_MISC_DISTANCE_FROM_PALACE") : gDLL->getText("TXT_KEY_MISC_NO_PALACE_PENALTY")));
			}

			iMaintenanceValue = pHeadSelectedCity->calculateNumCitiesMaintenanceTimes100();
			if (iMaintenanceValue != 0)
			{
				//			swprintf(szTempBuffer, "\n%s%d%c: Number of Cities", ((iMaintenanceValue > 0) ? "+" : ""), iMaintenanceValue, GC.getCommerceInfo(COMMERCE_GOLD).getChar());
				CvWString szMaint = CvWString::format(L"%d.%02d", iMaintenanceValue/100, iMaintenanceValue%100);
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_NUM_CITIES_FLOAT", szMaint.GetCString()));
			}

			iMaintenanceValue = pHeadSelectedCity->calculateColonyMaintenanceTimes100();
			if (iMaintenanceValue != 0)
			{
				CvWString szMaint = CvWString::format(L"%d.%02d", iMaintenanceValue/100, iMaintenanceValue%100);
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_COLONY_MAINT_FLOAT", szMaint.GetCString()));
			}

			iMaintenanceValue = pHeadSelectedCity->calculateCorporationMaintenanceTimes100();
			if (iMaintenanceValue != 0)
			{
				CvWString szMaint = CvWString::format(L"%d.%02d", iMaintenanceValue/100, iMaintenanceValue%100);
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_CORPORATION_MAINT_FLOAT", szMaint.GetCString()));
			}

			iMaintenanceValue = pHeadSelectedCity->calculateBuildingMaintenanceTimes100();
			if (iMaintenanceValue != 0)
			{
				CvWString szMaint = CvWString::format(L"%d.%02d", iMaintenanceValue/100, iMaintenanceValue%100);
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_BUILDING_MAINT_FLOAT", szMaint.GetCString()));
			}

			szBuffer.append(SEPARATOR);

			//		swprintf(szTempBuffer, "\n%d%c Total Maintenance", pHeadSelectedCity->getMaintenance(), GC.getCommerceInfo(COMMERCE_GOLD).getChar());
			CvWString szMaint = CvWString::format(L"%d.%02d", pHeadSelectedCity->getMaintenanceTimes100()/100, pHeadSelectedCity->getMaintenanceTimes100()%100);
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_TOTAL_MAINT_FLOAT", szMaint.GetCString()));
/************************************************************************************************/
/* Afforess	                  Start		 09/12/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
			iMaintenanceValue = pHeadSelectedCity->getMaintenanceModifier();
*/
			iMaintenanceValue = pHeadSelectedCity->getMaintenanceModifier() + GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).getMaintenanceModifier() + pHeadSelectedCity->area()->getTotalAreaMaintenanceModifier(GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).getID());

			if (pHeadSelectedCity->isConnectedToCapital() && !(pHeadSelectedCity->isCapital()))
			{
				iMaintenanceValue += GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).getConnectedCityMaintenanceModifier();
			}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			if (iMaintenanceValue != 0)
			{
				swprintf(szTempBuffer, L" (%s%d%%)", ((iMaintenanceValue > 0) ? L"+" : L""), iMaintenanceValue);
				szBuffer.append(szTempBuffer);
			}

// BUG - Building Saved Maintenance - start
			if (pHeadSelectedCity->getOwnerINLINE() == GC.getGame().getActivePlayer() && getBugOptionBOOL("MiscHover__BuildingSavedMaintenance", true, "BUG_BUILDING_SAVED_MAINTENANCE_HOVER"))
			{
				GAMETEXT.setBuildingSavedMaintenanceHelp(szBuffer, *pHeadSelectedCity, DOUBLE_SEPARATOR);
			}
// BUG - Building Saved Maintenance - end
		}
	}
}


void CvDLLWidgetData::parseHealthHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (NULL != pHeadSelectedCity)
	{
		GAMETEXT.setBadHealthHelp(szBuffer, *pHeadSelectedCity);
		szBuffer.append(L"\n=======================\n");
		GAMETEXT.setGoodHealthHelp(szBuffer, *pHeadSelectedCity);

// BUG - Building Additional Health - start
		if (pHeadSelectedCity->getOwnerINLINE() == GC.getGame().getActivePlayer() && getBugOptionBOOL("MiscHover__BuildingAdditionalHealth", true, "BUG_BUILDING_ADDITIONAL_HEALTH_HOVER"))
		{
			GAMETEXT.setBuildingAdditionalHealthHelp(szBuffer, *pHeadSelectedCity, DOUBLE_SEPARATOR);
		}
// BUG - Building Additional Health - end
	}
}


void CvDLLWidgetData::parseNationalityHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	wchar szTempBuffer[1024];
	CvCity* pHeadSelectedCity;
	PlayerTypes eCulturalOwner;
	int iCulturePercent;
	int iCityStrength;
	int iGarrison;
	int iI;

	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CITY_NATIONALITY"));

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				iCulturePercent = pHeadSelectedCity->plot()->calculateCulturePercent((PlayerTypes)iI);

				if (iCulturePercent > 0)
				{
					swprintf(szTempBuffer, L"\n%d%% " SETCOLR L"%s" ENDCOLR, iCulturePercent, GET_PLAYER((PlayerTypes)iI).getPlayerTextColorR(), GET_PLAYER((PlayerTypes)iI).getPlayerTextColorG(), GET_PLAYER((PlayerTypes)iI).getPlayerTextColorB(), GET_PLAYER((PlayerTypes)iI).getPlayerTextColorA(), GET_PLAYER((PlayerTypes)iI).getCivilizationAdjective());
					szBuffer.append(szTempBuffer);
				}
			}
		}

		eCulturalOwner = pHeadSelectedCity->plot()->calculateCulturalOwner();

		if (eCulturalOwner != NO_PLAYER)
		{
			if (GET_PLAYER(eCulturalOwner).getTeam() != pHeadSelectedCity->getTeam())
			{
				iCityStrength = pHeadSelectedCity->cultureStrength(eCulturalOwner);
				iGarrison = pHeadSelectedCity->cultureGarrison(eCulturalOwner);

				if (iCityStrength > iGarrison)
				{
					swprintf(szTempBuffer, L"%.2f", std::max(0.0f, (1.0f - (((float)iGarrison) / ((float)iCityStrength)))) * ((float)(std::min(100.0f, ((float)pHeadSelectedCity->getRevoltTestProbability())))));
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_CHANCE_OF_REVOLT", szTempBuffer));
				}
			}
		}
	}
}


void CvDLLWidgetData::parseHappinessHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		GAMETEXT.setAngerHelp(szBuffer, *pHeadSelectedCity);
		szBuffer.append(L"\n=======================\n");
		GAMETEXT.setHappyHelp(szBuffer, *pHeadSelectedCity);

// BUG - Building Additional Happiness - start
		if (pHeadSelectedCity->getOwnerINLINE() == GC.getGame().getActivePlayer() && getBugOptionBOOL("MiscHover__BuildingAdditionalHappiness", true, "BUG_BUILDING_ADDITIONAL_HAPPINESS_HOVER"))
		{
			GAMETEXT.setBuildingAdditionalHappinessHelp(szBuffer, *pHeadSelectedCity, DOUBLE_SEPARATOR);
		}
// BUG - Building Additional Happiness - end
	}
}


void CvDLLWidgetData::parsePopulationHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		szBuffer.assign(gDLL->getText("TXT_KEY_MISC_FOOD_THRESHOLD", pHeadSelectedCity->getFood(), pHeadSelectedCity->growthThreshold()));
	}
}


void CvDLLWidgetData::parseProductionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (pHeadSelectedCity->getProductionNeeded() != MAX_INT)
		{
			CvWString szTemp;
			szTemp.Format(L"%s: %d/%d %c", pHeadSelectedCity->getProductionName(), pHeadSelectedCity->getProduction(), pHeadSelectedCity->getProductionNeeded(), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
			szBuffer.assign(szTemp);
		}
	}
}


void CvDLLWidgetData::parseCultureHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		int iCultureTimes100 = pHeadSelectedCity->getCultureTimes100(pHeadSelectedCity->getOwnerINLINE());
		if (iCultureTimes100%100 == 0)
		{
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CULTURE", iCultureTimes100/100, pHeadSelectedCity->getCultureThreshold()));
		}
		else
		{
			CvWString szCulture = CvWString::format(L"%d.%02d", iCultureTimes100/100, iCultureTimes100%100);
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CULTURE_FLOAT", szCulture.GetCString(), pHeadSelectedCity->getCultureThreshold()));
		}

		int iCultureRateTimes100 = pHeadSelectedCity->getCommerceRateTimes100(COMMERCE_CULTURE);
		if (iCultureRateTimes100 > 0)
		{
			int iCultureLeftTimes100 = 100 * pHeadSelectedCity->getCultureThreshold() - iCultureTimes100;

			if (iCultureLeftTimes100 > 0)
			{
				int iTurnsLeft = (iCultureLeftTimes100  + iCultureRateTimes100 - 1) / iCultureRateTimes100;

				szBuffer.append(L' ');
				szBuffer.append(gDLL->getText("INTERFACE_CITY_TURNS", std::max(1, iTurnsLeft)));
			}
		}


		szBuffer.append(L"\n=======================\n");
		GAMETEXT.setCommerceHelp(szBuffer, *pHeadSelectedCity, COMMERCE_CULTURE);
	}
}


void CvDLLWidgetData::parseGreatPeopleHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (NULL != pHeadSelectedCity)
	{
		GAMETEXT.parseGreatPeopleHelp(szBuffer, *pHeadSelectedCity);
	}
}


void CvDLLWidgetData::parseGreatGeneralHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (NO_PLAYER != GC.getGame().getActivePlayer())
	{
		GAMETEXT.parseGreatGeneralHelp(szBuffer, GET_PLAYER(GC.getGame().getActivePlayer()));
	}
}


void CvDLLWidgetData::parseSelectedHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	//CvUnit* pHeadSelectedUnit;
	OrderData* pOrder;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	//pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (pHeadSelectedCity != NULL)
	{
		pOrder = pHeadSelectedCity->getOrderFromQueue(widgetDataStruct.m_iData1);

		if (pOrder != NULL)
		{
			switch (pOrder->eOrderType)
			{
			case ORDER_TRAIN:
				GAMETEXT.setUnitHelp(szBuffer, ((UnitTypes)EXTERNAL_ORDER_IDATA(pOrder->iData1)), false, false, false, pHeadSelectedCity);
				break;

			case ORDER_CONSTRUCT:
// BUG - Building Actual Effects - start
				GAMETEXT.setBuildingHelpActual(szBuffer, ((BuildingTypes)(pOrder->iData1)), false, false, false, pHeadSelectedCity);
// BUG - Building Actual Effects - end
				break;

			case ORDER_CREATE:
				GAMETEXT.setProjectHelp(szBuffer, ((ProjectTypes)(pOrder->iData1)), false, pHeadSelectedCity);
				break;

			case ORDER_MAINTAIN:
				GAMETEXT.setProcessHelp(szBuffer, ((ProcessTypes)(pOrder->iData1)));
				break;

			case ORDER_LIST:
				{
					CvPlayerAI& kPlayer = GET_PLAYER(pHeadSelectedCity->getOwnerINLINE());
					int index = kPlayer.m_pBuildLists->getIndexByID(pOrder->iData1);
					if (index >= 0)
						szBuffer.append(CvWString(kPlayer.m_pBuildLists->getListName(index)));
					// Possibly display some building names on the list here
				}
				break;

			default:
				FAssertMsg(false, "eOrderType did not match valid options");
				break;
			}
		}
	}
}

void CvDLLWidgetData::parseBuildListQueueHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	OrderData* pOrder;

	CvPlayerAI& kPlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());
	int index = kPlayer.m_pBuildLists->getIndexByID(widgetDataStruct.m_iData1);
	if (index > -1)
	{
		pOrder = kPlayer.m_pBuildLists->getOrder(index, widgetDataStruct.m_iData2);

		if (pOrder != NULL)
		{
			switch (pOrder->eOrderType)
			{
			case ORDER_TRAIN:
				GAMETEXT.setUnitHelp(szBuffer, (UnitTypes)(pOrder->iData1));
				break;

			case ORDER_CONSTRUCT:
				GAMETEXT.setBuildingHelp(szBuffer, (BuildingTypes)(pOrder->iData1));
				break;

			case ORDER_CREATE:
				GAMETEXT.setProjectHelp(szBuffer, (ProjectTypes)(pOrder->iData1));
				break;

			default:
				FAssertMsg(false, "eOrderType did not match valid options");
				break;
			}
		}
	}
}


void CvDLLWidgetData::parseBuildListHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvPlayerAI& kPlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());
	int index = kPlayer.m_pBuildLists->getIndexByID(widgetDataStruct.m_iData1);
	if (index > -1)
	{
		szBuffer.append(CvWString(kPlayer.m_pBuildLists->getListName(index)));
	}
}

void CvDLLWidgetData::parseTradeRouteCityHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setTradeRouteHelp(szBuffer, widgetDataStruct.m_iData1, gDLL->getInterfaceIFace()->getHeadSelectedCity());
	}
}

void CvDLLWidgetData::parseEspionageCostHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvUnit* pUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();
	if (NULL != pUnit)
	{
		CvPlot* pPlot = pUnit->plot();
		if (NULL != pPlot)
		{
			GAMETEXT.setEspionageCostHelp(szBuffer, (EspionageMissionTypes)widgetDataStruct.m_iData1, pPlot->getOwnerINLINE(), pPlot, widgetDataStruct.m_iData2, pUnit);
		}
	}
}

void CvDLLWidgetData::parseBuildingHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setBuildingHelp(szBuffer, ((BuildingTypes)(widgetDataStruct.m_iData1)), false, false, widgetDataStruct.m_bOption, gDLL->getInterfaceIFace()->getHeadSelectedCity());
	}
}

void CvDLLWidgetData::parseProjectHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setProjectHelp(szBuffer, ((ProjectTypes)widgetDataStruct.m_iData1), false, gDLL->getInterfaceIFace()->getHeadSelectedCity());
	}
}


void CvDLLWidgetData::parseTerrainHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setTerrainHelp(szBuffer, (TerrainTypes)widgetDataStruct.m_iData1);
	}
}


void CvDLLWidgetData::parseFeatureHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setFeatureHelp(szBuffer, (FeatureTypes)widgetDataStruct.m_iData1);
	}
}


void CvDLLWidgetData::parseTechEntryHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setTechHelp(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
	}
}

// BUG - Trade Denial - start
void CvDLLWidgetData::parseTechTradeEntryHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 == -1)
	{
		parseTechEntryHelp(widgetDataStruct, szBuffer);
	}
	else
	{
		GAMETEXT.setTechTradeHelp(szBuffer, (TechTypes)widgetDataStruct.m_iData1, (PlayerTypes)widgetDataStruct.m_iData2, false, false, false, true, NO_TECH);
	}
}
// BUG - Trade Denial - end

void CvDLLWidgetData::parseTechPrereqHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer.Format(L"%cThis technology requires the knowledge of %s", gDLL->getSymbolID(BULLET_CHAR), GC.getTechInfo((TechTypes) widgetDataStruct.m_iData1).getDescription());
	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_TECH_REQUIRES_KNOWLEDGE_OF", GC.getTechInfo((TechTypes) widgetDataStruct.m_iData1).getTextKeyWide()));
}

void CvDLLWidgetData::parseTechTreePrereq(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer, bool bTreeInfo)
{
	GAMETEXT.setTechHelp(szBuffer, (TechTypes)widgetDataStruct.m_iData1, false, false, false, bTreeInfo, (TechTypes)widgetDataStruct.m_iData2);
}


void CvDLLWidgetData::parseObsoleteHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildObsoleteString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseObsoleteBonusString(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildObsoleteBonusString(szBuffer, ((BonusTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseObsoleteSpecialHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildObsoleteSpecialString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseMoveHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildMoveString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseFreeUnitHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildFreeUnitString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData2)));
}

void CvDLLWidgetData::parseFeatureProductionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildFeatureProductionString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseWorkerRateHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildWorkerRateString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseTradeRouteHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildTradeRouteString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseHealthRateHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildHealthRateString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseHappinessRateHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildHappinessRateString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseFreeTechHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildFreeTechString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseLOSHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildLOSString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseMapCenterHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildMapCenterString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseMapRevealHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildMapRevealString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseMapTradeHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildMapTradeString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseTechTradeHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildTechTradeString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseGoldTradeHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildGoldTradeString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseOpenBordersHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildOpenBordersString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseDefensivePactHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildDefensivePactString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parsePermanentAllianceHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildPermanentAllianceString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseVassalStateHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildVassalStateString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseBuildBridgeHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildBridgeString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseIrrigationHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildIrrigationString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseIgnoreIrrigationHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildIgnoreIrrigationString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseWaterWorkHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildWaterWorkString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseBuildHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildImprovementString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2);
}

void CvDLLWidgetData::parseDomainExtraMovesHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildDomainExtraMovesString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2);
}

void CvDLLWidgetData::parseAdjustHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildAdjustString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2);
}

void CvDLLWidgetData::parseTerrainTradeHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 < GC.getNumTerrainInfos())
	{
		GAMETEXT.buildTerrainTradeString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2);
	}
	else if (widgetDataStruct.m_iData2 == GC.getNumTerrainInfos())
	{
		GAMETEXT.buildRiverTradeString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
	}
}

void CvDLLWidgetData::parseSpecialBuildingHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildSpecialBuildingString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2);
}

void CvDLLWidgetData::parseYieldChangeHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildYieldChangeString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2, false);
}

void CvDLLWidgetData::parseBonusRevealHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildBonusRevealString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2, true);
}

void CvDLLWidgetData::parseCivicRevealHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildCivicRevealString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2, true);
}

void CvDLLWidgetData::parseProcessInfoHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildProcessInfoString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2, true);
}

void CvDLLWidgetData::parseFoundReligionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildFoundReligionString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2, true);
}

void CvDLLWidgetData::parseFoundCorporationHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildFoundCorporationString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2, true);
}

// BUG - Finance Advisor - start
void CvDLLWidgetData::parseFinanceDomesticTrade(CvWidgetDataStruct& widgetDataStruct, CvWStringBuffer& szBuffer)
{
//	szBuffer = "The Total Commerce from Domestic Trade Routes";
	szBuffer.assign(gDLL->getText("TXT_KEY_BUG_FINANCIAL_ADVISOR_DOMESTIC_TRADE"));
	szBuffer.append(NEWLINE);
	if (widgetDataStruct.m_iData2 > 0)
	{
		GAMETEXT.buildDomesticTradeString(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseFinanceForeignTrade(CvWidgetDataStruct& widgetDataStruct, CvWStringBuffer& szBuffer)
{
//	szBuffer = "The Total Commerce from Foreign Trade Routes";
	szBuffer.assign(gDLL->getText("TXT_KEY_BUG_FINANCIAL_ADVISOR_FOREIGN_TRADE"));
	szBuffer.append(NEWLINE);
	if (widgetDataStruct.m_iData2 > 0)
	{
		GAMETEXT.buildForeignTradeString(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseFinanceSpecialistGold(CvWidgetDataStruct& widgetDataStruct, CvWStringBuffer& szBuffer)
{
//	szBuffer = "The Total Commerce from Foreign Trade Routes";
	szBuffer.assign(gDLL->getText("TXT_KEY_BUG_FINANCIAL_ADVISOR_SPECIALISTS"));
	if (widgetDataStruct.m_iData2 > 0)
	{
		GAMETEXT.buildFinanceSpecialistGoldString(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);
	}
}
// BUG - Finance Advisor - end

void CvDLLWidgetData::parseFinanceNumUnits(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "Number of units you are currently supporting";
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_NUM_UNITS_SUPPORTING"));
}

void CvDLLWidgetData::parseFinanceUnitCost(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "The amount of money spent on unit upkeep";
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_MONEY_SPENT_UPKEEP"));
	if (widgetDataStruct.m_iData2 > 0)
	{
		GAMETEXT.buildFinanceUnitCostString(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseFinanceAwaySupply(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "The amount of money spent on units in enemy territory";
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_AMOUNT_MONEY_UNITS_ENEMY_TERRITORY"));
	if (widgetDataStruct.m_iData2 > 0)
	{
		GAMETEXT.buildFinanceAwaySupplyString(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseFinanceCityMaint(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "The amount of money spent doing city maintenance";
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_AMOUNT_MONEY_CITY_MAINT"));
	if (widgetDataStruct.m_iData2 > 0)
	{
		GAMETEXT.buildFinanceCityMaintString(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseFinanceCivicUpkeep(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "The amount of money spent on Civics";
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_AMOUNT_MONEY_CIVIC_UPKEEP"));
	if (widgetDataStruct.m_iData2 > 0)
	{
		GAMETEXT.buildFinanceCivicUpkeepString(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseFinanceForeignIncome(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_AMOUNT_MONEY_FOREIGN"));
	if (widgetDataStruct.m_iData2 > 0)
	{
		GAMETEXT.buildFinanceForeignIncomeString(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseFinanceInflatedCosts(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_AMOUNT_MONEY_AFTER_INFLATION"));
	if (widgetDataStruct.m_iData2 > 0)
	{
		GAMETEXT.buildFinanceInflationString(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseFinanceGrossIncome(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "Your gross income";
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_GROSS_INCOME"));
}

void CvDLLWidgetData::parseFinanceNetGold(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "Net Gold per turn";
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_NET_GOLD"));
}

void CvDLLWidgetData::parseFinanceGoldReserve(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "Amount of money in your gold reserves";
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_GOLD_RESERVE"));
}

void CvDLLWidgetData::parseUnitHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setUnitHelp(szBuffer, (UnitTypes)widgetDataStruct.m_iData1, false, false, widgetDataStruct.m_bOption, gDLL->getInterfaceIFace()->getHeadSelectedCity());
	}
}

void CvDLLWidgetData::parsePediaBack(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "Back";
	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_PEDIA_BACK"));
}

void CvDLLWidgetData::parsePediaForward(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "Forward";
	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_PEDIA_FORWARD"));
}

void CvDLLWidgetData::parseBonusHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setBonusHelp(szBuffer, (BonusTypes)widgetDataStruct.m_iData1);
	}
}

// BUG - Trade Denial - start
void CvDLLWidgetData::parseBonusTradeHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 == -1)
	{
		parseBonusHelp(widgetDataStruct, szBuffer);
	}
	else
	{
		GAMETEXT.setBonusTradeHelp(szBuffer, (BonusTypes)widgetDataStruct.m_iData1, false, (PlayerTypes)widgetDataStruct.m_iData2);
	}
}
// BUG - Trade Denial - end

void CvDLLWidgetData::parseReligionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setReligionHelp(szBuffer, (ReligionTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseReligionHelpCity( CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer )
{
	GAMETEXT.setReligionHelpCity(szBuffer, (ReligionTypes)widgetDataStruct.m_iData1, gDLL->getInterfaceIFace()->getHeadSelectedCity(), true);
}

void CvDLLWidgetData::parseCorporationHelpCity( CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer )
{
	GAMETEXT.setCorporationHelpCity(szBuffer, (CorporationTypes)widgetDataStruct.m_iData1, gDLL->getInterfaceIFace()->getHeadSelectedCity(), true);
}

void CvDLLWidgetData::parseCorporationHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setCorporationHelp(szBuffer, (CorporationTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parsePromotionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setPromotionHelp(szBuffer, (PromotionTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseEventHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.setEventHelp(szBuffer, (EventTypes)widgetDataStruct.m_iData1, widgetDataStruct.m_iData2, GC.getGameINLINE().getActivePlayer());
}

void CvDLLWidgetData::parseUnitCombatHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setUnitCombatHelp(szBuffer, (UnitCombatTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseImprovementHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setImprovementHelp(szBuffer, (ImprovementTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseRouteHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setRouteHelp(szBuffer, (RouteTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseCivicHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.parseCivicInfo(szBuffer, (CivicTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseCivilizationHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.parseCivInfos(szBuffer, (CivilizationTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseLeaderHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != -1)
	{
		GAMETEXT.parseLeaderTraits(szBuffer, (LeaderHeadTypes)widgetDataStruct.m_iData1, (CivilizationTypes)widgetDataStruct.m_iData2);
	}
}

void CvDLLWidgetData::parseCloseScreenHelp(CvWStringBuffer& szBuffer)
{
	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CLOSE_SCREEN"));
}

void CvDLLWidgetData::parseDescriptionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer, bool bMinimal)
{
	CivilopediaPageTypes eType = (CivilopediaPageTypes)widgetDataStruct.m_iData1;
	switch (eType)
	{
	case CIVILOPEDIA_PAGE_TECH:
		{
			TechTypes eTech = (TechTypes)widgetDataStruct.m_iData2;
			if (NO_TECH != eTech)
			{
				szBuffer.assign(bMinimal ? GC.getTechInfo(eTech).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getTechInfo(eTech).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_UNIT:
		{
			UnitTypes eUnit = (UnitTypes)widgetDataStruct.m_iData2;
			if (NO_UNIT != eUnit)
			{
				szBuffer.assign(bMinimal ? GC.getUnitInfo(eUnit).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getUnitInfo(eUnit).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_BUILDING:
	case CIVILOPEDIA_PAGE_WONDER:
		{
			BuildingTypes eBuilding = (BuildingTypes)widgetDataStruct.m_iData2;
			if (NO_BUILDING != eBuilding)
			{
				szBuffer.assign(bMinimal ? GC.getBuildingInfo(eBuilding).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getBuildingInfo(eBuilding).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_BONUS:
		{
			BonusTypes eBonus = (BonusTypes)widgetDataStruct.m_iData2;
			if (NO_BONUS != eBonus)
			{
				szBuffer.assign(bMinimal ? GC.getBonusInfo(eBonus).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getBonusInfo(eBonus).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_IMPROVEMENT:
		{
			ImprovementTypes eImprovement = (ImprovementTypes)widgetDataStruct.m_iData2;
			if (NO_IMPROVEMENT != eImprovement)
			{
				szBuffer.assign(bMinimal ? GC.getImprovementInfo(eImprovement).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getImprovementInfo(eImprovement).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_UNIT_GROUP:
		{
			UnitCombatTypes eGroup = (UnitCombatTypes)widgetDataStruct.m_iData2;
			if (NO_UNITCOMBAT != eGroup)
			{
				szBuffer.assign(bMinimal ? GC.getUnitCombatInfo(eGroup).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getUnitCombatInfo(eGroup).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_PROMOTION:
		{
			PromotionTypes ePromo = (PromotionTypes)widgetDataStruct.m_iData2;
			if (NO_PROMOTION != ePromo)
			{
				szBuffer.assign(bMinimal ? GC.getPromotionInfo(ePromo).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getPromotionInfo(ePromo).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_CIV:
		{
			CivilizationTypes eCiv = (CivilizationTypes)widgetDataStruct.m_iData2;
			if (NO_CIVILIZATION != eCiv)
			{
				szBuffer.assign(bMinimal ? GC.getCivilizationInfo(eCiv).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getCivilizationInfo(eCiv).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_LEADER:
		{
			LeaderHeadTypes eLeader = (LeaderHeadTypes)widgetDataStruct.m_iData2;
			if (NO_LEADER != eLeader)
			{
				szBuffer.assign(bMinimal ? GC.getLeaderHeadInfo(eLeader).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getLeaderHeadInfo(eLeader).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_RELIGION:
		{
			ReligionTypes eReligion = (ReligionTypes)widgetDataStruct.m_iData2;
			if (NO_RELIGION != eReligion)
			{
				szBuffer.assign(bMinimal ? GC.getReligionInfo(eReligion).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getReligionInfo(eReligion).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_CORPORATION:
		{
			CorporationTypes eCorporation = (CorporationTypes)widgetDataStruct.m_iData2;
			if (NO_CORPORATION != eCorporation)
			{
				szBuffer.assign(bMinimal ? GC.getCorporationInfo(eCorporation).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getCorporationInfo(eCorporation).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_CIVIC:
		{
			CivicTypes eCivic = (CivicTypes)widgetDataStruct.m_iData2;
			if (NO_CIVIC != eCivic)
			{
				szBuffer.assign(bMinimal ? GC.getCivicInfo(eCivic).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getCivicInfo(eCivic).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_PROJECT:
		{
			ProjectTypes eProject = (ProjectTypes)widgetDataStruct.m_iData2;
			if (NO_PROJECT != eProject)
			{
				szBuffer.assign(bMinimal ? GC.getProjectInfo(eProject).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getProjectInfo(eProject).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_CONCEPT:
		{
			ConceptTypes eConcept = (ConceptTypes)widgetDataStruct.m_iData2;
			if (NO_CONCEPT != eConcept)
			{
				szBuffer.assign(GC.getConceptInfo(eConcept).getDescription());
			}
		}
		break;
	case CIVILOPEDIA_PAGE_CONCEPT_NEW:
		{
			NewConceptTypes eConcept = (NewConceptTypes)widgetDataStruct.m_iData2;
			if (NO_NEW_CONCEPT != eConcept)
			{
				szBuffer.assign(GC.getNewConceptInfo(eConcept).getDescription());
			}
		}
		break;
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
/*	case CIVILOPEDIA_PAGE_CONCEPT_DCM:
		{
			DCMConceptTypes eConcept = (DCMConceptTypes)widgetDataStruct.m_iData2;
			if (NO_DCM_CONCEPT != eConcept)
			{
				szBuffer.assign(GC.getDCMConceptInfo(eConcept).getDescription());
			}
		}
		break;*/
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/* Afforess                                     11/13/09                                        */
/************************************************************************************************/
	case CIVILOPEDIA_PAGE_CONCEPT_AND:
		{
			ANDConceptTypes eConcept = (ANDConceptTypes)widgetDataStruct.m_iData2;
			if (NO_AND_CONCEPT != eConcept)
			{
				szBuffer.assign(GC.getANDConceptInfo(eConcept).getDescription());
			}
		}
		break;
/************************************************************************************************/
/* Afforess                                    END                                              */
/************************************************************************************************/
	case CIVILOPEDIA_PAGE_SPECIALIST:
		{
			SpecialistTypes eSpecialist = (SpecialistTypes)widgetDataStruct.m_iData2;
			if (NO_SPECIALIST != eSpecialist)
			{
				szBuffer.assign(bMinimal ? GC.getSpecialistInfo(eSpecialist).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getSpecialistInfo(eSpecialist).getTextKeyWide()));
			}
		}
		break;
	default:
		break;
	}
}

void CvDLLWidgetData::parseKillDealHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "Click to cancel";
	CvWString szTemp;
	szTemp = szBuffer.getCString();
	CvDeal* pDeal = GC.getGameINLINE().getDeal(widgetDataStruct.m_iData1);
	if (NULL != pDeal)
	{
		if (pDeal->isCancelable(GC.getGameINLINE().getActivePlayer(), &szTemp))
		{
			szTemp = gDLL->getText("TXT_KEY_MISC_CLICK_TO_CANCEL");
		}
	}

	szBuffer.append(szTemp);
	
// BUG - Kill Deal Info - start
	if (pDeal != NULL)
	{
		szBuffer.append(NEWLINE);
		GAMETEXT.getDealString(szBuffer, *pDeal, GC.getGameINLINE().getActivePlayer());

		int iItem = widgetDataStruct.m_iData2;
		if (iItem != -1)
		{
			const CLinkList<TradeData>* listTradeData = NULL;

			if (iItem < pDeal->getLengthFirstTrades())
			{
				listTradeData = pDeal->getFirstTrades();
			}
			else
			{
				iItem -= pDeal->getLengthFirstTrades();
				if (iItem < pDeal->getLengthSecondTrades())
				{
					listTradeData = pDeal->getSecondTrades();
				}
			}

			if (listTradeData != NULL)
			{
				int iCount = 0;
				for (CLLNode<TradeData>* pNode = listTradeData->head(); NULL != pNode; pNode = listTradeData->next(pNode))
				{
					if (iCount++ == iItem)
					{
						TradeData& kTradeData = pNode->m_data;
						if (kTradeData.m_eItemType == TRADE_RESOURCES)
						{
							szBuffer.append(NEWLINE NEWLINE);
							GAMETEXT.setBonusHelp(szBuffer, (BonusTypes)kTradeData.m_iData);
						}
						break;
					}
				}
			}
		}
	}
// BUG - Kill Deal Info - end
}


void CvDLLWidgetData::doDealKill(CvWidgetDataStruct &widgetDataStruct)
{
	CvDeal* pDeal = GC.getGameINLINE().getDeal(widgetDataStruct.m_iData1);
	if (pDeal != NULL)
	{
		if (!pDeal->isCancelable(GC.getGameINLINE().getActivePlayer()))
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_TEXT);
			if (NULL != pInfo)
			{
				pInfo->setText(gDLL->getText("TXT_KEY_POPUP_CANNOT_CANCEL_DEAL"));
				gDLL->getInterfaceIFace()->addPopup(pInfo, GC.getGameINLINE().getActivePlayer(), true);
			}
		}
		else
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_DEAL_CANCELED);
			if (NULL != pInfo)
			{
				pInfo->setData1(pDeal->getID());
				pInfo->setOption1(false);
				gDLL->getInterfaceIFace()->addPopup(pInfo, GC.getGameINLINE().getActivePlayer(), true);
			}
		}
	}
}


void CvDLLWidgetData::doRefreshMilitaryAdvisor(CvWidgetDataStruct &widgetDataStruct)
{
	PYTHON_ACCESS_LOCK_SCOPE

	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	argsList.add(widgetDataStruct.m_iData2);
	PYTHON_CALL_FUNCTION(__FUNCTION__, PYScreensModule, "refreshMilitaryAdvisor", argsList.makeFunctionArgs());
}

// BUG - Food Rate Hover - start
void CvDLLWidgetData::parseFoodModHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (NULL != pCity)
	{
		GAMETEXT.setFoodHelp(szBuffer, *pCity);
	}
}
// BUG - Food Rate Hover - end

void CvDLLWidgetData::parseProductionModHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (NULL != pCity)
	{
		GAMETEXT.setProductionHelp(szBuffer, *pCity);
	}
}

void CvDLLWidgetData::parseLeaderheadHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.parseLeaderHeadHelp(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1, (PlayerTypes)widgetDataStruct.m_iData2);
}

void CvDLLWidgetData::parseLeaderLineHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.parseLeaderLineHelp(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1, (PlayerTypes)widgetDataStruct.m_iData2);
}

// BUG - Leaderhead Relations - start
void CvDLLWidgetData::parseLeaderheadRelationsHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.parseLeaderHeadRelationsHelp(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1, (PlayerTypes)widgetDataStruct.m_iData2);
}
// BUG - Leaderhead Relations - end

void CvDLLWidgetData::parseCommerceModHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (NULL != pCity)
	{
		GAMETEXT.setCommerceHelp(szBuffer, *pCity, (CommerceTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseScoreHelp(CvWidgetDataStruct& widgetDataStruct, CvWStringBuffer& szBuffer)
{
	GAMETEXT.setScoreHelp(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);
}

// BUG - Trade Hover - start
void CvDLLWidgetData::parseTradeRoutes(CvWidgetDataStruct& widgetDataStruct, CvWStringBuffer& szBuffer)
{
	GAMETEXT.buildTradeString(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1, (PlayerTypes)widgetDataStruct.m_iData2);
	GAMETEXT.getActiveDealsString(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1, (PlayerTypes)widgetDataStruct.m_iData2);
}
// BUG - Trade Hover - end

/************************************************************************************************/
/* Afforess	                  Start		 01/13/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
void CvDLLWidgetData::parseDefenseModHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (NULL != pCity)
	{
		GAMETEXT.setDefenseHelp(szBuffer, *pCity);
	}
}
void CvDLLWidgetData::parseObsoletePromotion(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildObsoletePromotionString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}
void CvDLLWidgetData::parseObsoleteCorporation(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildObsoleteCorporationString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/