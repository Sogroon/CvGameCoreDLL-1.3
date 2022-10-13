#pragma once

// plotGroup.h

#ifndef CIV4_PLOT_GROUP_H
#define CIV4_PLOT_GROUP_H

//#include "CvStructs.h"
#include "LinkedList.h"
#include "CvGame.h"
#include "CvGameAI.h"
#include "ByteBuffer.h"

typedef struct
{
	int resourceNodesHash;
	int allNodesHash;
} plotGroupHashInfo;

typedef struct
{
	plotGroupHashInfo	hashInfo;
	int					groupGenerationNumber;
} plotGroupCheckInfo;

class CvPlot;
class CvPlotGroup
{
friend class CvPlot;

public:

	CvPlotGroup();
	virtual ~CvPlotGroup();

	void* operator new(size_t size) { return GC.getGameINLINE().getAllocator().allocate(size); }
	void operator delete(void* instance) { GC.getGameINLINE().getAllocator().deAllocate(instance); }
	void* operator new[](size_t size) { return GC.getGameINLINE().getAllocator().allocate(size); }
	void operator delete[](void* instance) { GC.getGameINLINE().getAllocator().deAllocate(instance); }

	void init(int iID, PlayerTypes eOwner, CvPlot* pPlot, bool bRecalculateBonuses = true);
	void uninit();
	void reset(int iID = 0, PlayerTypes eOwner = NO_PLAYER, bool bConstructorCall=false);

	void addPlot(CvPlot* pPlot, bool bRecalculateBonuses);
	void removePlot(CvPlot* pPlot, bool bRecalculateBonuses = true);
	void recalculatePlots();														

	int getID() const;
	void setID(int iID);

	PlayerTypes getOwner() const;
#ifdef _USRDLL
	inline PlayerTypes getOwnerINLINE() const
	{
		return m_eOwner;
	}
#endif
	int getNumBonuses(BonusTypes eBonus) const;
	int getNumFreeTradeRegionBuildings(BuildingTypes eBuilding) const;
	bool hasBonus(BonusTypes eBonus);										
	void changeNumBonuses(BonusTypes eBonus, int iChange);
	void changeNumFreeTradeRegionBuildings(BuildingTypes eBuilding, int iChange);

	int getNumCities(void);

	// for serialization
	void read(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);

	//Afforess - MP Resync
	void resync(bool bWrite, ByteBuffer* pBuffer);

	//	Calculate the hashes after a fresh game load
	void RecalculateHashes();
	void Validate(void);

	inline int getLengthPlots() { return m_numPlots; }

	void mergeIn(CvPlotGroup* from, bool bRecalculateBonuses);
	static void colorRegion(CvPlot* pStartPlot, PlayerTypes eOwner, bool bRecalculateBonuses);
	static void startBulkRecalculate(void);
	static void endBulkRecalculate(void);

private:
	CvPlot* getRepresentativePlot(void) const;
	void plotEnumerator(bool (*pfFunc)(CvPlotGroup* onBehalfOf, CvPlot*, void*), void* param);
	static CvPlotGroup* colorRegionInternal(CvPlot* pPlot, PlayerTypes eOwner, CvPlotGroup* pPlotGroup, bool bRecalculateBonuses);

	int m_sessionAllocSeq;
	int m_sessionRecalcSeq;
	static int m_allocationSeqForSession;
	static int m_recalcSeqForSession;
	static int m_bulkRecalcStartSeq;

protected:

	int m_iID;

	PlayerTypes m_eOwner;

	int* m_paiNumBonuses;
	CRITICAL_SECTION m_cFreeTradeRegionBuildingsSection;
	int* m_paiNumFreeTradeRegionBuildings;
	mutable int m_seedPlotX;
	mutable int m_seedPlotY;

	plotGroupHashInfo m_zobristHashes;
						//	XOR of the zobrist contributions from all
						//	constituent plots 
public:
	int m_numPlots;
	mutable int m_numCities;
	//CLinkList<XYCoords> m_plots;
};

#endif
