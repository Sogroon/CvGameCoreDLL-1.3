#pragma once

// selectionGroupAI.h

#ifndef CIV4_SELECTION_GROUP_AI_H
#define CIV4_SELECTION_GROUP_AI_H

#include "CvSelectionGroup.h"

class CvSelectionGroupAI : public CvSelectionGroup
{

public:

	DllExport CvSelectionGroupAI();
	DllExport virtual ~CvSelectionGroupAI();

	void AI_init();
	void AI_uninit();
	void AI_reset();

	void AI_separate();
	void AI_separateNonAI(UnitAITypes eUnitAI);
	void AI_separateAI(UnitAITypes eUnitAI);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      06/02/09                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
	void AI_separateImpassable();
	void AI_separateEmptyTransports();
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/


	bool AI_update();

	int AI_attackOdds(const CvPlot* pPlot, bool bPotentialEnemy, bool bForce = false, bool* bWin = NULL, int iTheshold = -1) const;
	CvUnit* AI_getBestGroupAttacker(const CvPlot* pPlot, bool bPotentialEnemy, int& iUnitOdds, bool bForce = false, bool bNoBlitz = false, CvUnit** pDefender = NULL) const;
	CvUnit* AI_getBestGroupSacrifice(const CvPlot* pPlot, bool bPotentialEnemy, bool bForce = false, bool bNoBlitz = false) const;
	int AI_compareStacks(const CvPlot* pPlot, bool bPotentialEnemy, bool bCheckCanAttack = false, bool bCheckCanMove = false, int iRange = 0) const;
	int AI_sumStrength(const CvPlot* pAttackedPlot = NULL, DomainTypes eDomainType = NO_DOMAIN, bool bCheckCanAttack = false, bool bCheckCanMove = false) const;
	void AI_queueGroupAttack(int iX, int iY);
	void AI_cancelGroupAttack();
	bool AI_isGroupAttack();

	bool AI_isControlled();
	bool AI_isDeclareWar(const CvPlot* pPlot = NULL);

	CvPlot* AI_getMissionAIPlot();

	bool AI_isForceSeparate();
	void AI_makeForceSeparate();
	
	bool AI_isCityGarrison(const CvCity* pCity) const;
	void AI_setAsGarrison(const CvCity* pCity);

	MissionAITypes AI_getMissionAIType();
	void AI_setMissionAI(MissionAITypes eNewMissionAI, CvPlot* pNewPlot, CvUnit* pNewUnit);
	void AI_noteSizeChange(int iChange);
	CvUnit* AI_findBestDefender(CvPlot* pTargetPlot, bool allowAllDefenders, bool bConsiderPropertyValues = false);
	CvUnit* AI_ejectBestDefender(CvPlot* pTargetPlot, bool allowAllDefenders);
	virtual bool AI_hasBeneficialPropertyEffectForCity(CvCity* pCity) const;
	virtual CvUnit* AI_ejectBestPropertyManipulator(CvCity* pTargetCity);
	virtual int AI_getGenericValueTimes100(UnitValueFlags eFlags) const;

	CvUnit* AI_getMissionAIUnit();
	
	bool AI_isFull();

	void read(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);

	//Afforess - MP Resync
	void resync(bool bWrite, ByteBuffer* pBuffer);

protected:
	bool	AI_isAwaitingContract(void) const;

	int m_iMissionAIX;
	int m_iMissionAIY;

	bool m_bForceSeparate;

	MissionAITypes m_eMissionAIType;

	IDInfo m_missionAIUnit;

	bool m_bGroupAttack;
	int m_iGroupAttackX;
	int m_iGroupAttackY;
};

/*  f1rpo: If this fails, then you've probably added a data member (directly)
	to CvSelectionGroupAI. See comment in CvSelectionGroup.h. */
BOOST_STATIC_ASSERT(sizeof(CvSelectionGroupAI) == 36 + sizeof(CvSelectionGroup));

#endif
