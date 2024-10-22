#pragma once

#ifndef CIV4_MAP_EXTERNAL_H
#define CIV4_MAP_EXTERNAL_H

//
//	FILE:	 CvMapExternal.h
//	AUTHOR:  Steve Draper
//	PURPOSE: Game map externals class
//


#include "CvArea.h"
#include "CvPlot.h"
#include "CvMapInterfaceBase.h"


class FAStar;
class CvPlotGroup;


//
// CvMap
//
class CvSelectionGroup;
class CvMapExternal
{

	friend class CyMap;

public:

	CvMapExternal(CvMapInterfaceBase* proxiedMap);

	void mapCoordinates(bool bNewValue) { m_bMapCoordinates = bNewValue; }

	DllExport CvMapExternal();
	DllExport virtual ~CvMapExternal();

	DllExport void init(CvMapInitData* pInitData=NULL);
	DllExport void setupGraphical();
	DllExport void reset(CvMapInitData* pInitData);

	DllExport void erasePlots();																			// Exposed to Python
	DllExport void updateFlagSymbols();

	DllExport void updateFog();
	DllExport void updateVisibility();																// Exposed to Python
	DllExport void updateSymbolVisibility();
	DllExport void updateMinimapColor();															// Exposed to Python
	DllExport void updateCenterUnit();
	DllExport CvCity* findCity(int iX, int iY, PlayerTypes eOwner = NO_PLAYER, TeamTypes eTeam = NO_TEAM, bool bSameArea = true, bool bCoastalOnly = false, TeamTypes eTeamAtWarWith = NO_TEAM, DirectionTypes eDirection = NO_DIRECTION, CvCity* pSkipCity = NULL);	// Exposed to Python
	DllExport CvSelectionGroup* findSelectionGroup(int iX, int iY, PlayerTypes eOwner = NO_PLAYER, bool bReadyToSelect = false, bool bWorkers = false);				// Exposed to Python
	DllExport bool isPlot(int iX, int iY) const;																		// Exposed to Python
	DllExport int numPlots() const; 																								// Exposed to Python
	DllExport int plotNum(int iX, int iY) const;																		// Exposed to Python
	DllExport int pointXToPlotX(float fX);
	DllExport float plotXToPointX(int iX);
	DllExport int pointYToPlotY(float fY);
	DllExport float plotYToPointY(int iY);
	DllExport int getGridWidth() const;																		// Exposed to Python
	DllExport int getGridHeight() const;																	// Exposed to Python
	DllExport int getLandPlots();																					// Exposed to Python
	DllExport int getOwnedPlots();																				// Exposed to Python
	DllExport bool isWrapX();																							// Exposed to Python
	DllExport bool isWrapY();																							// Exposed to Python
	DllExport bool isWrap();
	DllExport WorldSizeTypes getWorldSize();															// Exposed to Python
	DllExport ClimateTypes getClimate();																	// Exposed to Python
	DllExport SeaLevelTypes getSeaLevel();																// Exposed to Python

	DllExport int getNumCustomMapOptions();
	DllExport CustomMapOptionTypes getCustomMapOption(int iOption);				// Exposed to Python
	DllExport CvPlot* plotByIndex(int iIndex) const;											// Exposed to Python
	DllExport CvPlot* plot(int iX, int iY) const;													// Exposed to Python
	DllExport CvPlot* pointToPlot(float fX, float fY);
	DllExport int getNumAreas();														// Exposed to Python
	DllExport int getNumLandAreas();
	// Serialization:
	DllExport virtual void read(FDataStreamBase* pStream);
	DllExport virtual void write(FDataStreamBase* pStream);

private:
	CvMapInterfaceBase*	m_proxiedMap;
	mutable std::map<int,CvPlot*>		m_psuedoPlots;
	bool m_bMapCoordinates;
};

#endif
