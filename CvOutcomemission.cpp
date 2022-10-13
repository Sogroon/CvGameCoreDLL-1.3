//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:	CvOutcomeMission.cpp
//
//  PURPOSE: A mission that has a cost and a result depending on an outcome list
//
//------------------------------------------------------------------------------------------------
#include "CvGameCoreDLL.h"
#include "CvOutcomeMission.h"

#include "CvGameObject.h"

#include "CvXMLLoadUtility.h"
#include "CvDLLXMLIFaceBase.h"

#include <boost/bind.hpp>

#include "CvUnit.h"
#include "CvPlayerAI.h"
#include "CvPlot.h"

CvOutcomeMission::CvOutcomeMission() :
m_eMission(NO_MISSION),
m_iCost(NULL),
m_bKill(true),
m_ePayerType(NO_GAMEOBJECT),
m_pPlotCondition(NULL),
m_pUnitCondition(NULL)
{
}

CvOutcomeMission::~CvOutcomeMission()
{
	GC.removeDelayedResolution((int*)&m_eMission);
	SAFE_DELETE(m_iCost);
	SAFE_DELETE(m_pPlotCondition);
	SAFE_DELETE(m_pUnitCondition);
}

//IntExpr* CvOutcomeMission::getCost()
//{
//	return m_iCost;
//}

MissionTypes CvOutcomeMission::getMission()
{
	return m_eMission;
}

CvOutcomeList* CvOutcomeMission::getOutcomeList()
{
	return &m_OutcomeList;
}

CvProperties* CvOutcomeMission::getPropertyCost()
{
	return &m_PropertyCost;
}

bool CvOutcomeMission::isKill()
{
	return m_bKill;
}

GameObjectTypes CvOutcomeMission::getPayerType()
{
	return m_ePayerType;
}

void callSetPayer(CvGameObject* pObject, CvGameObject** ppPayer)
{
	*ppPayer = pObject;
}

bool CvOutcomeMission::isPossible(CvUnit* pUnit, bool bTestVisible)
{
	CvPlayer* pOwner = &GET_PLAYER(pUnit->getOwnerINLINE());

	if (!bTestVisible)
	{
		if (m_iCost)
		{
			if (pOwner->getGold() < m_iCost->evaluate(pUnit->getGameObject()))
			{
				return false;
			}
		}

		if (m_pPlotCondition)
			if (!m_pPlotCondition->evaluate(pUnit->plot()->getGameObject()))
				return false;
	}

	if (m_pUnitCondition)
		if (!m_pUnitCondition->evaluate(pUnit->getGameObject()))
			return false;

	if (!getOutcomeList()->isPossible(*pUnit))
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (!getPropertyCost()->isEmpty())
		{
			CvGameObject* pPayer = NULL;
			if ((m_ePayerType == NO_GAMEOBJECT) || (m_ePayerType == GAMEOBJECT_UNIT))
			{
				pPayer = pUnit->getGameObject();
			}
			else
			{
				pUnit->getGameObject()->foreach(m_ePayerType, boost::bind(callSetPayer, _1, &pPayer));
			}

			if (!pPayer)
			{
				return false;
			}

			if (! (*(pPayer->getProperties()) > m_PropertyCost ))
			{
				return false;
			}
		}
	}

	return true;
}

void CvOutcomeMission::buildDisplayString(CvWStringBuffer &szBuffer, CvUnit *pUnit)
{
	if (!m_PropertyCost.isEmpty())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(L"Cost: ");
		m_PropertyCost.buildCompactChangesString(szBuffer);
	}
	
	if (m_iCost)
	{
		if (!m_iCost->evaluate(pUnit->getGameObject()))
		{
			CvWString szTempBuffer;

			szBuffer.append(NEWLINE);
			szBuffer.append(L"Cost: ");
			m_iCost->buildDisplayString(szBuffer);
		}
	}

	if (m_pPlotCondition)
	{
		if (!m_pPlotCondition->evaluate(pUnit->plot()->getGameObject()))
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_REQUIRES"));
			m_pPlotCondition->buildDisplayString(szBuffer);
		}
	}

	if (m_pUnitCondition)
	{
		if (!m_pUnitCondition->evaluate(pUnit->getGameObject()))
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_REQUIRES"));
			m_pUnitCondition->buildDisplayString(szBuffer);
		}
	}

	if (m_bKill)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CONSUME_UNIT"));
	}

	m_OutcomeList.buildDisplayString(szBuffer, *pUnit);
}

void CvOutcomeMission::execute(CvUnit* pUnit)
{
	CvPlayer* pOwner = &GET_PLAYER(pUnit->getOwnerINLINE());
	if (m_iCost)
	{
		pOwner->changeGold(-(m_iCost->evaluate(pUnit->getGameObject())));
	}

	getOutcomeList()->execute(*pUnit);

	if (!getPropertyCost()->isEmpty())
	{
		CvGameObject* pPayer = NULL;
		if ((m_ePayerType == NO_GAMEOBJECT) || (m_ePayerType == GAMEOBJECT_UNIT))
		{
			pPayer = pUnit->getGameObject();
		}
		else
		{
			pUnit->getGameObject()->foreach(m_ePayerType, boost::bind(callSetPayer, _1, &pPayer));
		}

		if (pPayer)
		{
			pPayer->getProperties()->subtractProperties(&m_PropertyCost);
		}
	}

	pUnit->finishMoves();
}

void CvOutcomeMission::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eMission);
	pStream->Read(&m_bKill);
	pStream->Read((int*)&m_ePayerType);
	m_PropertyCost.read(pStream);
	m_OutcomeList.read(pStream);

	bool bPresent = false;
	pStream->Read(&bPresent);
	if (bPresent)
	{
		m_pPlotCondition = BoolExpr::readExpression(pStream);
	}
	pStream->Read(&bPresent);
	if (bPresent)
	{
		m_pUnitCondition = BoolExpr::readExpression(pStream);
	}
	pStream->Read(&bPresent);
	if (bPresent)
	{
		m_iCost = IntExpr::readExpression(pStream);
	}
}

void CvOutcomeMission::write(FDataStreamBase *pStream)
{
	pStream->Write((int)m_eMission);
	pStream->Write(m_bKill);
	pStream->Write((int)m_ePayerType);
	m_PropertyCost.write(pStream);
	m_OutcomeList.write(pStream);
	if (m_pPlotCondition)
	{
		pStream->Write(true);
		m_pPlotCondition->write(pStream);
	}
	else
	{
		pStream->Write(false);
	}
	if (m_pUnitCondition)
	{
		pStream->Write(true);
		m_pUnitCondition->write(pStream);
	}
	else
	{
		pStream->Write(false);
	}
	if (m_iCost)
	{
		pStream->Write(true);
		m_iCost->write(pStream);
	}
	else
	{
		pStream->Write(false);
	}
}

bool CvOutcomeMission::read(CvXMLLoadUtility *pXML)
{
	CvString szTextVal;
	pXML->GetChildXmlValByName(szTextVal, "MissionType");
	m_eMission = (MissionTypes) pXML->FindInInfoClass(szTextVal);
	pXML->GetChildXmlValByName(&m_bKill, "bKill", true);
	pXML->GetChildXmlValByName(szTextVal, "PayerType");
	m_ePayerType = (GameObjectTypes) pXML->FindInInfoClass(szTextVal);
	m_PropertyCost.read(pXML, "PropertyCost");
	m_OutcomeList.read(pXML, "ActionOutcomes");
	if (GETXML->SetToChildByTagName(pXML->GetXML(),"PlotCondition"))
	{
		m_pPlotCondition = BoolExpr::read(pXML);
		GETXML->SetToParent(pXML->GetXML());
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"UnitCondition"))
	{
		m_pUnitCondition = BoolExpr::read(pXML);
		GETXML->SetToParent(pXML->GetXML());
	}

	if (GETXML->SetToChildByTagName(pXML->GetXML(),"iCost"))
	{
		m_iCost = IntExpr::read(pXML);
		GETXML->SetToParent(pXML->GetXML());
	}

	return true;
}

void CvOutcomeMission::copyNonDefaults(CvOutcomeMission *pOutcomeMission, CvXMLLoadUtility *pXML)
{
	if (m_eMission == NO_MISSION)
	{
		m_eMission = pOutcomeMission->getMission();
	}

	if (m_bKill)
	{
		m_bKill = pOutcomeMission->isKill();
	}

	if (m_ePayerType == NO_GAMEOBJECT)
	{
		m_ePayerType = pOutcomeMission->getPayerType();
	}

	m_PropertyCost.copyNonDefaults(pOutcomeMission->getPropertyCost(), pXML);
	m_OutcomeList.copyNonDefaults(pOutcomeMission->getOutcomeList(), pXML);
	if (!m_pPlotCondition)
	{
		m_pPlotCondition = pOutcomeMission->m_pPlotCondition;
		pOutcomeMission->m_pPlotCondition = NULL;
	}

	if (!m_pUnitCondition)
	{
		m_pUnitCondition = pOutcomeMission->m_pUnitCondition;
		pOutcomeMission->m_pUnitCondition = NULL;
	}
	
	if (!m_iCost)
	{
		m_iCost = pOutcomeMission->m_iCost;
		pOutcomeMission->m_iCost = NULL;
	}
}

void CvOutcomeMission::getCheckSum(unsigned int &iSum)
{
	CheckSum(iSum, m_eMission);
	CheckSum(iSum, m_bKill);
	CheckSum(iSum, m_ePayerType);
	m_PropertyCost.getCheckSum(iSum);
	m_OutcomeList.getCheckSum(iSum);
	if (m_iCost)
	{
		m_iCost->getCheckSum(iSum);
	}
}
