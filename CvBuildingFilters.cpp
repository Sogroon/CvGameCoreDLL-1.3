//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:	CvBuildingFilters.cpp
//
//  PURPOSE: Classes to filter buildings
//
//------------------------------------------------------------------------------------------------
#include "CvGameCoreDLL.h"
#include "CvBuildingFilters.h"
#include "CvGameCoreDLLUnDefNew.h"
#include <boost/python/enum.hpp>
#include "CvGameCoreDLLDefNew.h"
#include "CvBugOptions.h"
#include "CvCity.h"
#include "CvPlayerAI.h"
#include "CvGameCoreUtils.h"

void CyEnumsBuildingFiltersPythonInterface()
{
	python::enum_<BuildingFilterTypes>("BuildingFilterTypes")
		.value("NO_BUILDING_FILTER", NO_BUILDING_FILTER)
		.value("BUILDING_FILTER_HIDE_BASIC_INVISIBLE", BUILDING_FILTER_HIDE_BASIC_INVISIBLE)
		.value("BUILDING_FILTER_HIDE_BUILDABLE", BUILDING_FILTER_HIDE_BUILDABLE)
		.value("BUILDING_FILTER_HIDE_UNBUILDABLE", BUILDING_FILTER_HIDE_UNBUILDABLE)
		.value("BUILDING_FILTER_HIDE_GREAT_WONDER", BUILDING_FILTER_HIDE_GREAT_WONDER)
		.value("BUILDING_FILTER_HIDE_NATIONAL_WONDER", BUILDING_FILTER_HIDE_NATIONAL_WONDER)
		.value("BUILDING_FILTER_HIDE_NORMAL", BUILDING_FILTER_HIDE_NORMAL)
		.value("BUILDING_FILTER_SHOW_SCIENCE", BUILDING_FILTER_SHOW_SCIENCE)
		.value("BUILDING_FILTER_SHOW_FOOD", BUILDING_FILTER_SHOW_FOOD)
		.value("BUILDING_FILTER_SHOW_CULTURE", BUILDING_FILTER_SHOW_CULTURE)
		.value("BUILDING_FILTER_SHOW_ESPIONAGE", BUILDING_FILTER_SHOW_ESPIONAGE)
		.value("BUILDING_FILTER_SHOW_GOLD", BUILDING_FILTER_SHOW_GOLD)
		.value("BUILDING_FILTER_SHOW_PRODUCTION", BUILDING_FILTER_SHOW_PRODUCTION)
		.value("BUILDING_FILTER_SHOW_HAPPINESS", BUILDING_FILTER_SHOW_HAPPINESS)
		.value("BUILDING_FILTER_SHOW_HEALTH", BUILDING_FILTER_SHOW_HEALTH)
		.value("BUILDING_FILTER_SHOW_MILITARY", BUILDING_FILTER_SHOW_MILITARY)
		.value("BUILDING_FILTER_SHOW_CITY_DEFENSE", BUILDING_FILTER_SHOW_CITY_DEFENSE)
		.value("BUILDING_FILTER_HIDE_UNHAPPINESS", BUILDING_FILTER_HIDE_UNHAPPINESS)
		.value("BUILDING_FILTER_HIDE_UNHEALTHINESS", BUILDING_FILTER_HIDE_UNHEALTHINESS)
		.value("BUILDING_FILTER_SHOW_CRIME", BUILDING_FILTER_SHOW_CRIME)
		.value("BUILDING_FILTER_SHOW_FLAMMABILITY", BUILDING_FILTER_SHOW_FLAMMABILITY)
		;
}

BuildingFilterBase::BuildingFilterBase(bool bInvert)
{
	m_bInvert = bInvert;
	m_bActive = false;
}

void BuildingFilterBase::Activate()
{
	setActive(true);
}

void BuildingFilterBase::Deactivate()
{
	setActive(false);
}

bool BuildingFilterBase::isActive()
{
	return m_bActive;
}

bool BuildingFilterBase::setActive(bool bActive)
{
	bool bChanged = m_bActive ^ bActive;
	m_bActive = bActive;
	return bChanged;
}

bool BuildingFilterBase::isFiltered(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding)
{
	return !m_bActive || (m_bInvert ^ isFilteredBuilding(pPlayer, pCity, eBuilding));
}

bool BuildingFilterCanBuild::isFilteredBuilding(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding)
{
	if (pCity)
	{
		return pCity->canConstruct(eBuilding, false, m_bShowSomeUnconstructable);
	}
	return pPlayer->canConstruct(eBuilding, false, m_bShowSomeUnconstructable);
}

bool BuildingFilterIsWonder::isFilteredBuilding(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding)
{
	return isLimitedWonderClass((BuildingClassTypes)GC.getBuildingInfo(eBuilding).getBuildingClassType());
}

bool BuildingFilterIsGreatWonder::isFilteredBuilding(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding)
{
	return isWorldWonderClass((BuildingClassTypes)GC.getBuildingInfo(eBuilding).getBuildingClassType());
}

bool BuildingFilterIsNationalWonder::isFilteredBuilding(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding)
{
	return isNationalWonderClass((BuildingClassTypes)GC.getBuildingInfo(eBuilding).getBuildingClassType());
}

BuildingFilterIsCommerce::BuildingFilterIsCommerce(CommerceTypes eCommerce, bool bInvert) : BuildingFilterBase(bInvert)
{
	m_eCommerce = eCommerce;
}

bool BuildingFilterIsCommerce::isFilteredBuilding(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding)
{
	return (GC.getBuildingInfo(eBuilding).getCommerceChange(m_eCommerce) > 0) || (GC.getBuildingInfo(eBuilding).getCommerceModifier(m_eCommerce) > 0)
		|| (GC.getBuildingInfo(eBuilding).getSpecialistExtraCommerce(m_eCommerce) > 0) || (GC.getBuildingInfo(eBuilding).getGlobalCommerceModifier(m_eCommerce) > 0);
}

BuildingFilterIsYield::BuildingFilterIsYield(YieldTypes eYield, bool bInvert) : BuildingFilterBase(bInvert)
{
	m_eYield = eYield;
}

bool BuildingFilterIsYield::isFilteredBuilding(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding)
{
	return (GC.getBuildingInfo(eBuilding).getYieldChange(m_eYield) > 0) || (GC.getBuildingInfo(eBuilding).getYieldModifier(m_eYield) > 0)
		|| (GC.getBuildingInfo(eBuilding).getAreaYieldModifier(m_eYield) > 0) || (GC.getBuildingInfo(eBuilding).getGlobalYieldModifier(m_eYield) > 0)
		|| (GC.getBuildingInfo(eBuilding).getGlobalSeaPlotYieldChange(m_eYield) > 0) || (GC.getBuildingInfo(eBuilding).getSeaPlotYieldChange(m_eYield) > 0);
}

bool BuildingFilterIsHappiness::isFilteredBuilding(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding)
{
	return (GC.getBuildingInfo(eBuilding).getHappiness() > 0) || (GC.getBuildingInfo(eBuilding).getAreaHappiness() > 0) || (GC.getBuildingInfo(eBuilding).getGlobalHappiness() > 0);
}

bool BuildingFilterIsHealth::isFilteredBuilding(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding)
{
	return (GC.getBuildingInfo(eBuilding).getHealth() > 0) || (GC.getBuildingInfo(eBuilding).getAreaHealth() > 0) || (GC.getBuildingInfo(eBuilding).getGlobalHealth() > 0);
}

bool BuildingFilterIsUnhappiness::isFilteredBuilding(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding)
{
	return (GC.getBuildingInfo(eBuilding).getHappiness() < 0) || (GC.getBuildingInfo(eBuilding).getAreaHappiness() < 0) || (GC.getBuildingInfo(eBuilding).getGlobalHappiness() < 0);
}

bool BuildingFilterIsUnhealthiness::isFilteredBuilding(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding)
{
	return (GC.getBuildingInfo(eBuilding).getHealth() < 0) || (GC.getBuildingInfo(eBuilding).getAreaHealth() < 0) || (GC.getBuildingInfo(eBuilding).getGlobalHealth() < 0);
}

bool BuildingFilterIsMilitary::isFilteredBuilding(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding) // not finished
{
	return (GC.getBuildingInfo(eBuilding).getMilitaryProductionModifier() > 0) || (GC.getBuildingInfo(eBuilding).getFreeExperience() > 0) ||
		(GC.getBuildingInfo(eBuilding).getFreePromotion() != NO_PROMOTION);
}

bool BuildingFilterIsCityDefense::isFilteredBuilding(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding)
{
	return (GC.getBuildingInfo(eBuilding).getDefenseModifier() > 0) || (GC.getBuildingInfo(eBuilding).getAllCityDefenseModifier() > 0) ||
		(GC.getBuildingInfo(eBuilding).getAdjacentDamagePercent() > 0) || (GC.getBuildingInfo(eBuilding).getBombardDefenseModifier() > 0);
}

bool BuildingFilterIsProperty::isFilteredBuilding(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding)
{
	CvBuildingInfo& kInfo = GC.getBuildingInfo(eBuilding);
	if ((kInfo.getProperties()->getValueByProperty((int)m_eProperty) != 0) || (kInfo.getPropertiesAllCities()->getValueByProperty((int)m_eProperty)))
		return true;

	CvPropertyManipulators* pMani = kInfo.getPropertyManipulators();
	int iNum = pMani->getNumSources();
	for (int i=0; i<iNum; i++)
	{
		if (pMani->getSource(i)->getProperty() == m_eProperty)
			return true;
	}

	iNum = pMani->getNumInteractions();
	for (int i=0; i<iNum; i++)
	{
		if (pMani->getInteraction(i)->getSourceProperty() == m_eProperty)
			return true;
		if (pMani->getInteraction(i)->getTargetProperty() == m_eProperty)
			return true;
	}

	iNum = pMani->getNumPropagators();
	for (int i=0; i<iNum; i++)
	{
		if (pMani->getPropagator(i)->getProperty() == m_eProperty)
			return true;
	}

	return false;
}

BuildingFilterList::BuildingFilterList(CvPlayer *pPlayer, CvCity *pCity)
{
	m_pPlayer = pPlayer;
	m_pCity = pCity;

	for (int i = 0; i < NUM_BUILDING_FILTERS; i++)
	{
		m_apBuildingFilters[i] = NULL;
	}
}

void BuildingFilterList::init(void)
{
	for (int i = 0; i < NUM_BUILDING_FILTERS; i++)
	{
		SAFE_DELETE(m_apBuildingFilters[i]);
	}

	m_apBuildingFilters[BUILDING_FILTER_HIDE_BASIC_INVISIBLE] = new BuildingFilterCanBuild(true);
	if (m_pCity)
		m_apBuildingFilters[BUILDING_FILTER_HIDE_BASIC_INVISIBLE]->Activate();
	m_apBuildingFilters[BUILDING_FILTER_HIDE_BUILDABLE] = new BuildingFilterCanBuild(false, true);
	m_apBuildingFilters[BUILDING_FILTER_HIDE_UNBUILDABLE] = new BuildingFilterCanBuild();
	m_apBuildingFilters[BUILDING_FILTER_HIDE_GREAT_WONDER] = new BuildingFilterIsGreatWonder(true);
	m_apBuildingFilters[BUILDING_FILTER_HIDE_NATIONAL_WONDER] = new BuildingFilterIsNationalWonder(true);
	m_apBuildingFilters[BUILDING_FILTER_HIDE_NORMAL] = new BuildingFilterIsWonder();
	m_apBuildingFilters[BUILDING_FILTER_SHOW_SCIENCE] = new BuildingFilterIsCommerce(COMMERCE_RESEARCH);
	m_apBuildingFilters[BUILDING_FILTER_SHOW_FOOD] = new BuildingFilterIsYield(YIELD_FOOD);
	m_apBuildingFilters[BUILDING_FILTER_SHOW_CULTURE] = new BuildingFilterIsCommerce(COMMERCE_CULTURE);
	m_apBuildingFilters[BUILDING_FILTER_SHOW_ESPIONAGE] = new BuildingFilterIsCommerce(COMMERCE_ESPIONAGE);
	m_apBuildingFilters[BUILDING_FILTER_SHOW_GOLD] = new BuildingFilterIsCommerce(COMMERCE_GOLD);
	m_apBuildingFilters[BUILDING_FILTER_SHOW_PRODUCTION] = new BuildingFilterIsYield(YIELD_PRODUCTION);
	m_apBuildingFilters[BUILDING_FILTER_SHOW_HAPPINESS] = new BuildingFilterIsHappiness();
	m_apBuildingFilters[BUILDING_FILTER_SHOW_HEALTH] = new BuildingFilterIsHealth();
	m_apBuildingFilters[BUILDING_FILTER_SHOW_MILITARY] = new BuildingFilterIsMilitary();
	m_apBuildingFilters[BUILDING_FILTER_SHOW_CITY_DEFENSE] = new BuildingFilterIsCityDefense();
	m_apBuildingFilters[BUILDING_FILTER_HIDE_UNHAPPINESS] = new BuildingFilterIsUnhappiness(true);
	m_apBuildingFilters[BUILDING_FILTER_HIDE_UNHEALTHINESS] = new BuildingFilterIsUnhealthiness(true);
	// m_apBuildingFilters[BUILDING_FILTER_SHOW_CRIME] = new BuildingFilterIsProperty((PropertyTypes)GC.getInfoTypeForString("PROPERTY_CRIME"));
	m_apBuildingFilters[BUILDING_FILTER_SHOW_CRIME] = new BuildingFilterIsProperty((PropertyTypes)NO_PROPERTY);
	// m_apBuildingFilters[BUILDING_FILTER_SHOW_FLAMMABILITY] = new BuildingFilterIsProperty((PropertyTypes)GC.getInfoTypeForString("PROPERTY_FLAMMABILITY"));
	m_apBuildingFilters[BUILDING_FILTER_SHOW_FLAMMABILITY] = new BuildingFilterIsProperty((PropertyTypes)NO_PROPERTY);

	m_apBuildingFilters[BUILDING_FILTER_HIDE_UNBUILDABLE]->setActive(getBugOptionBOOL("RoMSettings__HideUnconstructableBuildings", false));
}

BuildingFilterList::~BuildingFilterList()
{
	for (int i = 0; i < NUM_BUILDING_FILTERS; i++)
	{
		SAFE_DELETE(m_apBuildingFilters[i]);
	}
}

int BuildingFilterList::getNumFilters()
{
	return NUM_BUILDING_FILTERS;
}

bool BuildingFilterList::isFilterActive(BuildingFilterTypes i)
{
	FAssertMsg(i < NUM_BUILDING_FILTERS, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_apBuildingFilters[i]->isActive();
}

void BuildingFilterList::setCity(CvCity *pCity)
{
	m_pCity = pCity;
}

void BuildingFilterList::setPlayer(CvPlayer *pPlayer)
{
	m_pPlayer = pPlayer;
}

bool BuildingFilterList::setFilterActive(BuildingFilterTypes i, bool bActive)
{
	FAssertMsg(i < NUM_BUILDING_FILTERS, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_apBuildingFilters[i]->setActive(bActive);
}

bool BuildingFilterList::isFiltered(BuildingTypes eBuilding)
{
	for (int i = 0; i < NUM_BUILDING_FILTERS; i++)
	{
		if (!m_apBuildingFilters[i]->isFiltered(m_pPlayer, m_pCity, eBuilding))
			return false;
	}
	return true;
}

void BuildingFilterList::setFilterActiveAll(BuildingFilterTypes eFilter, bool bActive)
{
	for (int iI = 0; iI < MAX_CIV_PLAYERS; ++iI)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iI);
		if (kLoopPlayer.isAlive())
		{
			kLoopPlayer.setBuildingListFilterActive(eFilter, bActive);
			int iIter = 0;
			for (CvCity* pCity = kLoopPlayer.firstCity(&iIter); NULL != pCity; pCity = kLoopPlayer.nextCity(&iIter))
			{
				pCity->setBuildingListFilterActive(eFilter, bActive);
			}
		}
	}
}