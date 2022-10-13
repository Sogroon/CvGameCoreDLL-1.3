//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:	BoolExpr.cpp
//
//  PURPOSE: Boolean Expressions for Civ4 classes
//
//------------------------------------------------------------------------------------------------
#include "CvGameCoreDLL.h"
#include "BoolExpr.h"
#include "CvDLLXMLIFaceBase.h"
#include <boost/bind.hpp>

BoolExpr::~BoolExpr()
{
}

BoolExpr* BoolExpr::read(CvXMLLoadUtility *pXML)
{
	// In general we assume no comments to simplify reading code

	TCHAR szTag[1024];
	if (!GETXML->GetLastLocatedNodeTagName(pXML->GetXML(), szTag))
	{
		// No located node
		return NULL;
	}
	
	if (strcmp(szTag, "Not") == 0)
	{
		// this is a Not node, read the first subexpression and generate a Not expression
		if (GETXML->SetToChild(pXML->GetXML()))
		{
			// there is a subexpression, so no simple constant
			BoolExpr* pExpr = read(pXML);
			GETXML->SetToParent(pXML->GetXML());
			return new BoolExprNot(pExpr);
		}
		else
		{
			// constant, no need to generate a real Not
			bool bConstant = false;
			pXML->GetXmlVal(&bConstant);
			return new BoolExprConstant(!bConstant);
		}
	}
	
	if (strcmp(szTag, "And") == 0)
	{
		// this is an And node, check number of children
		if (GETXML->GetNumChildren(pXML->GetXML()) < 2)
		{
			// no real And node
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				// there is a subexpression, so no simple constant
				BoolExpr* pExpr = read(pXML);
				GETXML->SetToParent(pXML->GetXML());
				return pExpr;
			}
			else
			{
				// constant
				bool bConstant = false;
				pXML->GetXmlVal(&bConstant);
				return new BoolExprConstant(!bConstant);
			}
		}
		else
		{
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				// read the first node
				BoolExpr* pExpr = read(pXML);
				
				// read nodes until there are no more siblings
				while (GETXML->NextSibling(pXML->GetXML()))
				{
					pExpr = new BoolExprAnd(pExpr, read(pXML));
				}

				GETXML->SetToParent(pXML->GetXML());
				return pExpr;
			}
		}
	}

	if (strcmp(szTag, "Or") == 0)
	{
		// this is an Or node, check number of children
		if (GETXML->GetNumChildren(pXML->GetXML()) < 2)
		{
			// no real Or node
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				// there is a subexpression, so no simple constant
				BoolExpr* pExpr = read(pXML);
				GETXML->SetToParent(pXML->GetXML());
				return pExpr;
			}
			else
			{
				// constant
				bool bConstant = false;
				pXML->GetXmlVal(&bConstant);
				return new BoolExprConstant(!bConstant);
			}
		}
		else
		{
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				// read the first node
				BoolExpr* pExpr = read(pXML);
				
				// read nodes until there are no more siblings
				while (GETXML->NextSibling(pXML->GetXML()))
				{
					pExpr = new BoolExprOr(pExpr, read(pXML));
				}

				GETXML->SetToParent(pXML->GetXML());
				return pExpr;
			}
		}
	}

	if (strcmp(szTag, "BEqual") == 0)
	{
		// this is a Boolean Expression comparison node, check number of children
		if (GETXML->GetNumChildren(pXML->GetXML()) != 2)
		{
			// BEqual nodes must have two boolean expressions, make it a constant false node
			return new BoolExprConstant(false);
		}
		else
		{
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				// read the first node
				BoolExpr* pExpr = read(pXML);
				
				// read the second node
				if (GETXML->NextSibling(pXML->GetXML()))
				{
					pExpr = new BoolExprBEqual(pExpr, read(pXML));
				}

				GETXML->SetToParent(pXML->GetXML());
				return pExpr;
			}
		}
	}

	if (strcmp(szTag, "Greater") == 0)
	{
		// this is a comparison node, check number of children
		if (GETXML->GetNumChildren(pXML->GetXML()) != 2)
		{
			// Comparison nodes must have two boolean expressions, make it a constant false node
			return new BoolExprConstant(false);
		}
		else
		{
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				// read the first node
				IntExpr* pExpr = IntExpr::read(pXML);
				BoolExpr* pBExpr = NULL;
				
				// read the second node
				if (GETXML->NextSibling(pXML->GetXML()))
				{
					pBExpr = new BoolExprGreater(pExpr, IntExpr::read(pXML));
				}

				GETXML->SetToParent(pXML->GetXML());
				return pBExpr;
			}
		}
	}

	if (strcmp(szTag, "GreaterEqual") == 0)
	{
		// this is a comparison node, check number of children
		if (GETXML->GetNumChildren(pXML->GetXML()) != 2)
		{
			// Comparison nodes must have two boolean expressions, make it a constant false node
			return new BoolExprConstant(false);
		}
		else
		{
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				// read the first node
				IntExpr* pExpr = IntExpr::read(pXML);
				BoolExpr* pBExpr = NULL;
				
				// read the second node
				if (GETXML->NextSibling(pXML->GetXML()))
				{
					pBExpr = new BoolExprGreaterEqual(pExpr, IntExpr::read(pXML));
				}

				GETXML->SetToParent(pXML->GetXML());
				return pBExpr;
			}
		}
	}

	if (strcmp(szTag, "Equal") == 0)
	{
		// this is a comparison node, check number of children
		if (GETXML->GetNumChildren(pXML->GetXML()) != 2)
		{
			// Comparison nodes must have two boolean expressions, make it a constant false node
			return new BoolExprConstant(false);
		}
		else
		{
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				// read the first node
				IntExpr* pExpr = IntExpr::read(pXML);
				BoolExpr* pBExpr = NULL;
				
				// read the second node
				if (GETXML->NextSibling(pXML->GetXML()))
				{
					pBExpr = new BoolExprEqual(pExpr, IntExpr::read(pXML));
				}

				GETXML->SetToParent(pXML->GetXML());
				return pBExpr;
			}
		}
	}

	if (strcmp(szTag, "If") == 0)
	{
		// this is an if/then/else node, check number of children
		if (GETXML->GetNumChildren(pXML->GetXML()) != 3)
		{
			// if/then/else nodes must have three boolean expressions, make it a constant false node
			return new BoolExprConstant(false);
		}
		else
		{
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				// read the if node
				BoolExpr* pIfExpr = read(pXML);
				BoolExpr* pThenExpr = NULL;
				BoolExpr* pElseExpr = NULL;
				
				// read the then node
				if (GETXML->NextSibling(pXML->GetXML()))
				{
					pThenExpr = read(pXML);
				}
				// read the else node
				if (GETXML->NextSibling(pXML->GetXML()))
				{
					pElseExpr = read(pXML);
				}

				GETXML->SetToParent(pXML->GetXML());
				return new BoolExprIf(pIfExpr, pThenExpr, pElseExpr);
			}
		}
	}

	// Check for the integrators
	if (strcmp(szTag, "IntegrateOr") == 0)
	{
		CvString szTextVal;
		pXML->GetChildXmlValByName(szTextVal, "RelationType");
		RelationTypes eRelation = (RelationTypes) pXML->FindInInfoClass(szTextVal);
		int iData = -1;
		pXML->GetChildXmlValByName(&iData, "iDistance");
		pXML->GetChildXmlValByName(szTextVal, "GameObjectType");
		GameObjectTypes eType = (GameObjectTypes) pXML->FindInInfoClass(szTextVal);
		
		// Find the expression and read it
		if (GETXML->SetToChild(pXML->GetXML()))
		{
			BoolExpr* pExpr = NULL;
			TCHAR szInnerTag[1024];
			while (GETXML->NextSibling(pXML->GetXML()))
			{
				if (!GETXML->IsLastLocatedNodeCommentNode(pXML->GetXML()))
				{
					if (GETXML->GetLastLocatedNodeTagName(pXML->GetXML(), szInnerTag))
					{
						if (!((strcmp(szInnerTag, "RelationType") == 0) || (strcmp(szInnerTag, "iDistance") == 0) || (strcmp(szInnerTag, "GameObjectType") == 0)))
						{
							pExpr = BoolExpr::read(pXML);
							break;
						}
					}
				}
			}

			GETXML->SetToParent(pXML->GetXML());

			// If no subexpression is defined, use constant true (so it will be true if any related object is found)
			if (!pExpr)
				pExpr = new BoolExprConstant(true);

			return new BoolExprIntegrateOr(pExpr, eRelation, iData, eType);
		}
	}

	// Check for the different literals
	if (strcmp(szTag, "Has") == 0)
	{
		// this is a Has GOM node
		BoolExprHas* pExpr = new BoolExprHas();
		pExpr->readContent(pXML);
		return pExpr;
	}

	if (strcmp(szTag, "Is") == 0)
	{
		// this is an Is node, querying a tag of the game object
		CvString szTextVal;
		pXML->GetXmlVal(szTextVal);
		return new BoolExprIs((TagTypes)pXML->FindInInfoClass(szTextVal));
	}

	// Check if there is a subexpression and use that
	if (GETXML->SetToChild(pXML->GetXML()))
	{
		// there is a subexpression, so no simple constant
		BoolExpr* pExpr = read(pXML);
		GETXML->SetToParent(pXML->GetXML());
		return pExpr;
	}
	else
	{
		// constant
		bool bConstant = false;
		pXML->GetXmlVal(&bConstant);
		return new BoolExprConstant(bConstant);
	}
}

BoolExpr* BoolExpr::readExpression(FDataStreamBase *pStream)
{
	BoolExpr* pExpr = NULL;
	BoolExprTypes eExprType = NO_BOOLEXPR;
	pStream->Read((int*)&eExprType);

	switch (eExprType)
	{
	case BOOLEXPR_CONSTANT:
		pExpr = new BoolExprConstant();
		break;

	case BOOLEXPR_HAS:
		pExpr = new BoolExprHas();
		break;

	case BOOLEXPR_IS:
		pExpr = new BoolExprIs();
		break;

	case BOOLEXPR_NOT:
		pExpr = new BoolExprNot();
		break;

	case BOOLEXPR_AND:
		pExpr = new BoolExprAnd();
		break;

	case BOOLEXPR_OR:
		pExpr = new BoolExprOr();
		break;

	case BOOLEXPR_BEQUAL:
		pExpr = new BoolExprBEqual();
		break;

	case BOOLEXPR_IF:
		pExpr = new BoolExprIf();
		break;

	case BOOLEXPR_INTEGRATE_OR:
		pExpr = new BoolExprIntegrateOr();
		break;

	case BOOLEXPR_GREATER:
		pExpr = new BoolExprGreater();
		break;

	case BOOLEXPR_GREATER_EQUAL:
		pExpr = new BoolExprGreaterEqual();
		break;

	case BOOLEXPR_EQUAL:
		pExpr = new BoolExprEqual();
		break;
	}

	if (pExpr)
		pExpr->read(pStream);

	return pExpr;
}


bool BoolExprConstant::evaluate(CvGameObject *pObject)
{
	return m_bValue;
}

void BoolExprConstant::read(FDataStreamBase *pStream)
{
	pStream->Read(&m_bValue);
}

void BoolExprConstant::write(FDataStreamBase *pStream)
{
	pStream->Write((int)BOOLEXPR_CONSTANT);
	pStream->Write(m_bValue);
}

void BoolExprConstant::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append(m_bValue ? "True" : "False");
}

int BoolExprConstant::getBindingStrength() const
{
	return 100;
}

void BoolExprConstant::getCheckSum(unsigned int &iSum)
{
	CheckSum(iSum, m_bValue);
}



BoolExprHas::~BoolExprHas()
{
	GC.removeDelayedResolution((int*)&m_eGOM);
	GC.removeDelayedResolution(&m_iID);
}

bool BoolExprHas::evaluate(CvGameObject *pObject)
{
	return pObject->hasGOM(m_eGOM, m_iID);
}

void BoolExprHas::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eGOM);
	pStream->Read(&m_iID);
}

void BoolExprHas::write(FDataStreamBase* pStream)
{
	pStream->Write((int)BOOLEXPR_HAS);
	pStream->Write((int)m_eGOM);
	pStream->Write(m_iID);
}

void BoolExprHas::readContent(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	pXML->GetChildXmlValByName(szTextVal, "GOMType");
	GC.addDelayedResolution((int*)&m_eGOM, szTextVal);
	pXML->GetChildXmlValByName(szTextVal, "ID");
	GC.addDelayedResolution(&m_iID, szTextVal);
}

void BoolExprHas::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append(L"<link=literal>");
	switch (m_eGOM)
	{
		case GOM_BUILDING:
			szBuffer.append(GC.getBuildingInfo((BuildingTypes)m_iID).getDescription());
			break;
		case GOM_PROMOTION:
			szBuffer.append(GC.getPromotionInfo((PromotionTypes)m_iID).getDescription());
			break;
		case GOM_TRAIT:
			szBuffer.append(GC.getTraitInfo((TraitTypes)m_iID).getDescription());
			break;
		case GOM_FEATURE:
			szBuffer.append(GC.getFeatureInfo((FeatureTypes)m_iID).getDescription());
			break;
		case GOM_OPTION:
			szBuffer.append(GC.getGameOptionInfo((GameOptionTypes)m_iID).getDescription());
			break;
		case GOM_TERRAIN:
			szBuffer.append(GC.getTerrainInfo((TerrainTypes)m_iID).getDescription());
			break;
		case GOM_GAMESPEED:
			szBuffer.append(GC.getGameSpeedInfo((GameSpeedTypes)m_iID).getDescription());
			break;
		case GOM_ROUTE:
			szBuffer.append(GC.getRouteInfo((RouteTypes)m_iID).getDescription());
			break;
		case GOM_BONUS:
			szBuffer.append(GC.getBonusInfo((BonusTypes)m_iID).getDescription());
			break;
		case GOM_UNITTYPE:
			szBuffer.append(GC.getUnitInfo((UnitTypes)m_iID).getDescription());
			break;
		case GOM_TECH:
			szBuffer.append(GC.getTechInfo((TechTypes)m_iID).getDescription());
			break;
		case GOM_CIVIC:
			szBuffer.append(GC.getCivicInfo((CivicTypes)m_iID).getDescription());
			break;
		case GOM_RELIGION:
			szBuffer.append(GC.getReligionInfo((ReligionTypes)m_iID).getDescription());
			break;
		case GOM_CORPORATION:
			szBuffer.append(GC.getCorporationInfo((CorporationTypes)m_iID).getDescription());
			break;
		case GOM_IMPROVEMENT:
			szBuffer.append(GC.getImprovementInfo((ImprovementTypes)m_iID).getDescription());
			break;
		case GOM_UNITCOMBAT:
			szBuffer.append(GC.getUnitCombatInfo((UnitCombatTypes)m_iID).getDescription());
			break;
		case GOM_HANDICAP:
			szBuffer.append(GC.getHandicapInfo((HandicapTypes)m_iID).getDescription());
			break;
	}
	szBuffer.append(L"</link>");
}

int BoolExprHas::getBindingStrength() const
{
	return 100;
}

void BoolExprHas::getCheckSum(unsigned int &iSum)
{
	CheckSum(iSum, (int)m_eGOM);
	CheckSum(iSum, m_iID);
}


bool BoolExprIs::evaluate(CvGameObject *pObject)
{
	return pObject->isTag(m_eTag);
}

void BoolExprIs::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eTag);
}

void BoolExprIs::write(FDataStreamBase* pStream)
{
	pStream->Write((int)BOOLEXPR_IS);
	pStream->Write((int)m_eTag);
}

void BoolExprIs::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	// This is only the quick and dirty variant. Needs some proper text usage still.
	switch (m_eTag)
	{
		case TAG_WATER:
			szBuffer.append("Is on or near water");
			break;
		case TAG_FRESH_WATER:
			szBuffer.append("Has Fresh Water");
			break;
		case TAG_PEAK:
			szBuffer.append("Is peak");
			break;
		case TAG_HILL:
			szBuffer.append("Is hill");
			break;
		case TAG_FLATLAND:
			szBuffer.append("Is flat land");
			break;
		case TAG_OWNED:
			szBuffer.append("Is owned");
			break;
		case TAG_CITY:
			szBuffer.append("Is city");
			break;
		case TAG_ANARCHY:
			szBuffer.append("Is in anarchy");
			break;
	}
}

int BoolExprIs::getBindingStrength() const
{
	return 100;
}

void BoolExprIs::getCheckSum(unsigned int &iSum)
{
	CheckSum(iSum, (int)m_eTag);
}



BoolExprNot::~BoolExprNot()
{
	SAFE_DELETE(m_pExpr);
}

bool BoolExprNot::evaluate(CvGameObject *pObject)
{
	return !m_pExpr->evaluate(pObject);
}

void BoolExprNot::read(FDataStreamBase *pStream)
{
	m_pExpr = BoolExpr::readExpression(pStream);
}

void BoolExprNot::write(FDataStreamBase *pStream)
{
	pStream->Write((int)BOOLEXPR_NOT);
	m_pExpr->write(pStream);
}

void BoolExprNot::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("Not (");
	m_pExpr->buildDisplayString(szBuffer);
	szBuffer.append(")");
}

int BoolExprNot::getBindingStrength() const
{
	return 40;
}

void BoolExprNot::getCheckSum(unsigned int &iSum)
{
	m_pExpr->getCheckSum(iSum);
}


BoolExprAnd::~BoolExprAnd()
{
	SAFE_DELETE(m_pExpr1);
	SAFE_DELETE(m_pExpr2);
}

bool BoolExprAnd::evaluate(CvGameObject *pObject)
{
	return m_pExpr1->evaluate(pObject) && m_pExpr2->evaluate(pObject);
}

void BoolExprAnd::read(FDataStreamBase *pStream)
{
	m_pExpr1 = BoolExpr::readExpression(pStream);
	m_pExpr2 = BoolExpr::readExpression(pStream);
}

void BoolExprAnd::write(FDataStreamBase *pStream)
{
	pStream->Write((int)BOOLEXPR_AND);
	m_pExpr1->write(pStream);
	m_pExpr2->write(pStream);
}

void BoolExprAnd::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	bool bBrackets1 = false;
	bool bBrackets2 = false;
	if (getBindingStrength() > m_pExpr1->getBindingStrength())
		bBrackets1 = true;
	if (getBindingStrength() > m_pExpr2->getBindingStrength())
		bBrackets2 = true;
	if (bBrackets1)
		szBuffer.append("(");
	m_pExpr1->buildDisplayString(szBuffer);
	if (bBrackets1)
		szBuffer.append(")");
	szBuffer.append(" And ");
	if (bBrackets2)
		szBuffer.append("(");
	m_pExpr2->buildDisplayString(szBuffer);
	if (bBrackets2)
		szBuffer.append(")");
}

int BoolExprAnd::getBindingStrength() const
{
	return 20;
}

void BoolExprAnd::getCheckSum(unsigned int &iSum)
{
	m_pExpr1->getCheckSum(iSum);
	m_pExpr2->getCheckSum(iSum);
}



BoolExprOr::~BoolExprOr()
{
	SAFE_DELETE(m_pExpr1);
	SAFE_DELETE(m_pExpr2);
}

bool BoolExprOr::evaluate(CvGameObject *pObject)
{
	return m_pExpr1->evaluate(pObject) || m_pExpr2->evaluate(pObject);
}

void BoolExprOr::read(FDataStreamBase *pStream)
{
	m_pExpr1 = BoolExpr::readExpression(pStream);
	m_pExpr2 = BoolExpr::readExpression(pStream);
}

void BoolExprOr::write(FDataStreamBase *pStream)
{
	pStream->Write((int)BOOLEXPR_OR);
	m_pExpr1->write(pStream);
	m_pExpr2->write(pStream);
}

void BoolExprOr::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	bool bBrackets1 = false;
	bool bBrackets2 = false;
	if (getBindingStrength() > m_pExpr1->getBindingStrength())
		bBrackets1 = true;
	if (getBindingStrength() > m_pExpr2->getBindingStrength())
		bBrackets2 = true;
	if (bBrackets1)
		szBuffer.append("(");
	m_pExpr1->buildDisplayString(szBuffer);
	if (bBrackets1)
		szBuffer.append(")");
	szBuffer.append(" Or ");
	if (bBrackets2)
		szBuffer.append("(");
	m_pExpr2->buildDisplayString(szBuffer);
	if (bBrackets2)
		szBuffer.append(")");
}

int BoolExprOr::getBindingStrength() const
{
	return 10;
}

void BoolExprOr::getCheckSum(unsigned int &iSum)
{
	m_pExpr1->getCheckSum(iSum);
	m_pExpr2->getCheckSum(iSum);
}


BoolExprBEqual::~BoolExprBEqual()
{
	SAFE_DELETE(m_pExpr1);
	SAFE_DELETE(m_pExpr2);
}

bool BoolExprBEqual::evaluate(CvGameObject *pObject)
{
	return m_pExpr1->evaluate(pObject) == m_pExpr2->evaluate(pObject);
}

void BoolExprBEqual::read(FDataStreamBase *pStream)
{
	m_pExpr1 = BoolExpr::readExpression(pStream);
	m_pExpr2 = BoolExpr::readExpression(pStream);
}

void BoolExprBEqual::write(FDataStreamBase *pStream)
{
	pStream->Write((int)BOOLEXPR_BEQUAL);
	m_pExpr1->write(pStream);
	m_pExpr2->write(pStream);
}

void BoolExprBEqual::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	bool bBrackets1 = false;
	bool bBrackets2 = false;
	if (getBindingStrength() > m_pExpr1->getBindingStrength())
		bBrackets1 = true;
	if (getBindingStrength() > m_pExpr2->getBindingStrength())
		bBrackets2 = true;
	if (bBrackets1)
		szBuffer.append("(");
	m_pExpr1->buildDisplayString(szBuffer);
	if (bBrackets1)
		szBuffer.append(")");
	szBuffer.append(" Equals ");
	if (bBrackets2)
		szBuffer.append("(");
	m_pExpr2->buildDisplayString(szBuffer);
	if (bBrackets2)
		szBuffer.append(")");
}

int BoolExprBEqual::getBindingStrength() const
{
	return 30;
}

void BoolExprBEqual::getCheckSum(unsigned int &iSum)
{
	m_pExpr1->getCheckSum(iSum);
	m_pExpr2->getCheckSum(iSum);
}


BoolExprIf::~BoolExprIf()
{
	SAFE_DELETE(m_pExprIf);
	SAFE_DELETE(m_pExprThen);
	SAFE_DELETE(m_pExprElse);
}

bool BoolExprIf::evaluate(CvGameObject *pObject)
{
	return m_pExprIf->evaluate(pObject) ? m_pExprThen->evaluate(pObject) : m_pExprElse->evaluate(pObject);
}

void BoolExprIf::read(FDataStreamBase *pStream)
{
	m_pExprIf = BoolExpr::readExpression(pStream);
	m_pExprThen = BoolExpr::readExpression(pStream);
	m_pExprElse = BoolExpr::readExpression(pStream);
}

void BoolExprIf::write(FDataStreamBase *pStream)
{
	pStream->Write((int)BOOLEXPR_IF);
	m_pExprIf->write(pStream);
	m_pExprThen->write(pStream);
	m_pExprElse->write(pStream);
}

void BoolExprIf::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	bool bBracketsIf = false;
	bool bBracketsThen = false;
	bool bBracketsElse = false;
	if (getBindingStrength() > m_pExprIf->getBindingStrength())
		bBracketsIf = true;
	if (getBindingStrength() > m_pExprThen->getBindingStrength())
		bBracketsThen = true;
	if (getBindingStrength() > m_pExprElse->getBindingStrength())
		bBracketsElse = true;
	szBuffer.append("If ");
	if (bBracketsIf)
		szBuffer.append("(");
	m_pExprIf->buildDisplayString(szBuffer);
	if (bBracketsIf)
		szBuffer.append(")");
	szBuffer.append(" Then ");
	if (bBracketsThen)
		szBuffer.append("(");
	m_pExprThen->buildDisplayString(szBuffer);
	if (bBracketsThen)
		szBuffer.append(")");
	szBuffer.append(" Else ");
	if (bBracketsElse)
		szBuffer.append("(");
	m_pExprElse->buildDisplayString(szBuffer);
	if (bBracketsElse)
		szBuffer.append(")");
}

int BoolExprIf::getBindingStrength() const
{
	return 25;
}

void BoolExprIf::getCheckSum(unsigned int &iSum)
{
	m_pExprIf->getCheckSum(iSum);
	m_pExprThen->getCheckSum(iSum);
	m_pExprElse->getCheckSum(iSum);
}


void evalExprIntegrateOr(CvGameObject* pObject, BoolExpr* pExpr, bool* bAcc)
{
	*bAcc = *bAcc || pExpr->evaluate(pObject);
}

BoolExprIntegrateOr::~BoolExprIntegrateOr()
{
	SAFE_DELETE(m_pExpr);
}

bool BoolExprIntegrateOr::evaluate(CvGameObject *pObject)
{
	bool bAcc = false;
	pObject->foreachRelated(m_eType, m_eRelation, boost::bind(evalExprIntegrateOr, _1, m_pExpr, &bAcc));
	return bAcc;
}

void BoolExprIntegrateOr::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eRelation);
	pStream->Read(&m_iData);
	pStream->Read((int*)&m_eType);
	m_pExpr = BoolExpr::readExpression(pStream);
}

void BoolExprIntegrateOr::write(FDataStreamBase *pStream)
{
	pStream->Write((int)BOOLEXPR_INTEGRATE_OR);
	pStream->Write((int)m_eRelation);
	pStream->Write(m_iData);
	pStream->Write((int)m_eType);
	m_pExpr->write(pStream);
}

void BoolExprIntegrateOr::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	// TODO: Proper rendering of relations
	m_pExpr->buildDisplayString(szBuffer);
}

int BoolExprIntegrateOr::getBindingStrength() const
{
	// TODO: Update when proper buildDisplayString is implemented
	return m_pExpr->getBindingStrength();
}

void BoolExprIntegrateOr::getCheckSum(unsigned int &iSum)
{
	CheckSum(iSum, (int)m_eRelation);
	CheckSum(iSum, m_iData);
	CheckSum(iSum, (int)m_eType);
	m_pExpr->getCheckSum(iSum);
}


BoolExprComp::~BoolExprComp()
{
	SAFE_DELETE(m_pExpr1);
	SAFE_DELETE(m_pExpr2);
}

void BoolExprComp::read(FDataStreamBase *pStream)
{
	m_pExpr1 = IntExpr::readExpression(pStream);
	m_pExpr2 = IntExpr::readExpression(pStream);
}

void BoolExprComp::write(FDataStreamBase *pStream)
{
	pStream->Write((int)getType());
	m_pExpr1->write(pStream);
	m_pExpr2->write(pStream);
}

void BoolExprComp::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	bool bBrackets1 = false;
	bool bBrackets2 = false;
	if (getBindingStrength() > m_pExpr1->getBindingStrength())
		bBrackets1 = true;
	if (getBindingStrength() > m_pExpr2->getBindingStrength())
		bBrackets2 = true;
	if (bBrackets1)
		szBuffer.append("(");
	m_pExpr1->buildDisplayString(szBuffer);
	if (bBrackets1)
		szBuffer.append(")");
	szBuffer.append(" ");
	buildOpNameString(szBuffer);
	szBuffer.append(" ");
	if (bBrackets2)
		szBuffer.append("(");
	m_pExpr2->buildDisplayString(szBuffer);
	if (bBrackets2)
		szBuffer.append(")");
}

int BoolExprComp::getBindingStrength() const
{
	return 5;
}

void BoolExprComp::getCheckSum(unsigned int &iSum)
{
	CheckSum(iSum, (int)getType());
	m_pExpr1->getCheckSum(iSum);
	m_pExpr2->getCheckSum(iSum);
}


bool BoolExprGreater::evaluate(CvGameObject *pObject)
{
	return m_pExpr1->evaluate(pObject) > m_pExpr2->evaluate(pObject);
}

BoolExprTypes BoolExprGreater::getType() const
{
	return BOOLEXPR_GREATER;
}

void BoolExprGreater::buildOpNameString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append(">");
}


bool BoolExprGreaterEqual::evaluate(CvGameObject *pObject)
{
	return m_pExpr1->evaluate(pObject) >= m_pExpr2->evaluate(pObject);
}

BoolExprTypes BoolExprGreaterEqual::getType() const
{
	return BOOLEXPR_GREATER_EQUAL;
}

void BoolExprGreaterEqual::buildOpNameString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append(">=");
}


bool BoolExprEqual::evaluate(CvGameObject *pObject)
{
	return m_pExpr1->evaluate(pObject) == m_pExpr2->evaluate(pObject);
}

BoolExprTypes BoolExprEqual::getType() const
{
	return BOOLEXPR_EQUAL;
}

void BoolExprEqual::buildOpNameString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("=");
}
