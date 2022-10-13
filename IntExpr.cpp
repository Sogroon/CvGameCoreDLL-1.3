//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:	BoolExpr.cpp
//
//  PURPOSE: Boolean Expressions for Civ4 classes
//
//------------------------------------------------------------------------------------------------
#include "CvGameCoreDLL.h"
#include "IntExpr.h"
#include "CvDLLXMLIFaceBase.h"
#include <boost/bind.hpp>

#include "CvGameAI.h"
#include "CvDLLPythonIFaceBase.h"

IntExpr::~IntExpr()
{
}

IntExpr* IntExpr::read(CvXMLLoadUtility *pXML)
{
	// In general we assume no comments to simplify reading code

	TCHAR szTag[1024];
	if (!GETXML->GetLastLocatedNodeTagName(pXML->GetXML(), szTag))
	{
		// No located node
		return NULL;
	}

	if (strcmp(szTag, "Random") == 0)
	{
		// this is a Random node, check number of children
		if (GETXML->GetNumChildren(pXML->GetXML()) < 1)
		{
			// Random node with constant
			int iConstant = 0;
			pXML->GetXmlVal(&iConstant);
			return new IntExprRandom(new IntExprConstant(iConstant));
		}
		else
		{
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				// read the subnode
				IntExpr* pExpr = read(pXML);
				
				GETXML->SetToParent(pXML->GetXML());
				return new IntExprRandom(pExpr);
			}
		}
	}

	if (strcmp(szTag, "Adapt") == 0)
	{
		// this is an Adapt node, check number of children
		int iNumChildren = GETXML->GetNumChildren(pXML->GetXML());
		if (iNumChildren < 1)
		{
			// Adapt node with constant and default adaption
			int iConstant = 0;
			pXML->GetXmlVal(&iConstant);
			return new IntExprAdapt(new IntExprConstant(iConstant));
		}
		else if (iNumChildren < 2)
		{
			// one child, so adapt node with subexpression
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				// read the subnode
				IntExpr* pExpr = read(pXML);
				
				GETXML->SetToParent(pXML->GetXML());
				return new IntExprAdapt(pExpr);
			}
		}
		else
		{
			// first child is ID, second is subexpression
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				CvString szTextVal;
				pXML->GetXmlVal(szTextVal);

				GETXML->NextSibling(pXML->GetXML());
				// read the subnode
				IntExpr* pExpr = read(pXML);
				
				GETXML->SetToParent(pXML->GetXML());
				return new IntExprAdapt(pExpr, GC.getOrCreateInfoTypeForString(szTextVal));
			}
		}
	}

	if (strcmp(szTag, "Plus") == 0)
	{
		// this is a Plus node, check number of children
		if (GETXML->GetNumChildren(pXML->GetXML()) < 2)
		{
			// no real Plus node
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				// there is a subexpression, so no simple constant
				IntExpr* pExpr = read(pXML);
				GETXML->SetToParent(pXML->GetXML());
				return pExpr;
			}
			else
			{
				// constant
				int iConstant = 0;
				pXML->GetXmlVal(&iConstant);
				return new IntExprConstant(iConstant);
			}
		}
		else
		{
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				// read the first node
				IntExpr* pExpr = read(pXML);
				
				// read nodes until there are no more siblings
				while (GETXML->NextSibling(pXML->GetXML()))
				{
					pExpr = new IntExprPlus(pExpr, read(pXML));
				}

				GETXML->SetToParent(pXML->GetXML());
				return pExpr;
			}
		}
	}

	if (strcmp(szTag, "Mult") == 0)
	{
		// this is a Mult node, check number of children
		if (GETXML->GetNumChildren(pXML->GetXML()) < 2)
		{
			// no real Mult node
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				// there is a subexpression, so no simple constant
				IntExpr* pExpr = read(pXML);
				GETXML->SetToParent(pXML->GetXML());
				return pExpr;
			}
			else
			{
				// constant
				int iConstant = 0;
				pXML->GetXmlVal(&iConstant);
				return new IntExprConstant(iConstant);
			}
		}
		else
		{
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				// read the first node
				IntExpr* pExpr = read(pXML);
				
				// read nodes until there are no more siblings
				while (GETXML->NextSibling(pXML->GetXML()))
				{
					pExpr = new IntExprMult(pExpr, read(pXML));
				}

				GETXML->SetToParent(pXML->GetXML());
				return pExpr;
			}
		}
	}

	if (strcmp(szTag, "Minus") == 0)
	{
		// this is a Minus node, check number of children
		if (GETXML->GetNumChildren(pXML->GetXML()) != 2)
		{
			// Minus nodes must have two subexpressions, make it a constant 0 node
			return new IntExprConstant(0);
		}
		else
		{
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				// read the first node
				IntExpr* pExpr = read(pXML);
				
				// read the second node
				if (GETXML->NextSibling(pXML->GetXML()))
				{
					pExpr = new IntExprMinus(pExpr, read(pXML));
				}

				GETXML->SetToParent(pXML->GetXML());
				return pExpr;
			}
		}
	}

	if (strcmp(szTag, "Div") == 0)
	{
		// this is a Div node, check number of children
		if (GETXML->GetNumChildren(pXML->GetXML()) != 2)
		{
			// Div nodes must have two subexpressions, make it a constant 0 node
			return new IntExprConstant(0);
		}
		else
		{
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				// read the first node
				IntExpr* pExpr = read(pXML);
				
				// read the second node
				if (GETXML->NextSibling(pXML->GetXML()))
				{
					pExpr = new IntExprDiv(pExpr, read(pXML));
				}

				GETXML->SetToParent(pXML->GetXML());
				return pExpr;
			}
		}
	}

	if (strcmp(szTag, "If") == 0)
	{
		// this is an if/then/else node, check number of children
		if (GETXML->GetNumChildren(pXML->GetXML()) != 3)
		{
			// if/then/else nodes must have three subexpressions, make it a constant 0 node
			return new IntExprConstant(0);
		}
		else
		{
			if (GETXML->SetToChild(pXML->GetXML()))
			{
				// read the if node
				BoolExpr* pIfExpr = BoolExpr::read(pXML);
				IntExpr* pThenExpr = NULL;
				IntExpr* pElseExpr = NULL;
				
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
				return new IntExprIf(pIfExpr, pThenExpr, pElseExpr);
			}
		}
	}

	// Check for the integrators
	if ((strcmp(szTag, "IntegrateSum") == 0) || (strcmp(szTag, "IntegrateAvg") == 0) || (strcmp(szTag, "IntegrateCount") == 0))
	{
		CvString szTextVal;
		pXML->GetChildXmlValByName(szTextVal, "RelationType");
		RelationTypes eRelation = (RelationTypes) pXML->FindInInfoClass(szTextVal);
		int iData = -1;
		pXML->GetChildXmlValByName(&iData, "iDistance");
		pXML->GetChildXmlValByName(szTextVal, "GameObjectType");
		GameObjectTypes eType = (GameObjectTypes) pXML->FindInInfoClass(szTextVal);
		
		IntExpr* pExpr = NULL;
		BoolExpr* pBExpr = NULL;
		// Find the expression and read it
		if (GETXML->SetToChild(pXML->GetXML()))
		{
			bool bFound = false;
			TCHAR szInnerTag[1024];
			while (GETXML->NextSibling(pXML->GetXML()))
			{
				if (!GETXML->IsLastLocatedNodeCommentNode(pXML->GetXML()))
				{
					if (GETXML->GetLastLocatedNodeTagName(pXML->GetXML(), szInnerTag))
					{
						if (!((strcmp(szInnerTag, "RelationType") == 0) || (strcmp(szInnerTag, "iDistance") == 0) || (strcmp(szInnerTag, "GameObjectType") == 0)))
						{
							bFound = true;
							if (strcmp(szTag, "IntegrateCount") == 0)
							{
								pBExpr = BoolExpr::read(pXML);
							}
							else
							{
								pExpr = IntExpr::read(pXML);
							}
							break;
						}
					}
				}
			}

			GETXML->SetToParent(pXML->GetXML());
			if (strcmp(szTag, "IntegrateSum") == 0)
			{
				return new IntExprIntegrateSum(pExpr, eRelation, iData, eType);
			}
			else if (strcmp(szTag, "IntegrateAvg") == 0)
			{
				return new IntExprIntegrateAvg(pExpr, eRelation, iData, eType);
			}
			else
			{
				return new IntExprIntegrateCount(pBExpr, eRelation, iData, eType);
			}
		}
	}

	// Check for the different literals
	if (strcmp(szTag, "PropertyType") == 0)
	{
		// this is a Property node, querying a property of the game object
		CvString szTextVal;
		pXML->GetXmlVal(szTextVal);
		return new IntExprProperty((PropertyTypes)pXML->FindInInfoClass(szTextVal));
	}

	if (strcmp(szTag, "AttributeType") == 0)
	{
		// this is an Attribute node, querying an attribute of the game object
		CvString szTextVal;
		pXML->GetXmlVal(szTextVal);
		return new IntExprAttribute((AttributeTypes)pXML->FindInInfoClass(szTextVal));
	}

	if (strcmp(szTag, "Python") == 0)
	{
		// this is a Python node
		CvString szTextVal;
		pXML->GetXmlVal(szTextVal);
		return new IntExprPython(szTextVal);
	}

	// Check if there is a subexpression and use that
	if (GETXML->SetToChild(pXML->GetXML()))
	{
		// there is a subexpression, so no simple constant
		IntExpr* pExpr = read(pXML);
		GETXML->SetToParent(pXML->GetXML());
		return pExpr;
	}
	else
	{
		// constant
		int iConstant = 0;
		pXML->GetXmlVal(&iConstant);
		return new IntExprConstant(iConstant);
	}
}

IntExpr* IntExpr::readExpression(FDataStreamBase *pStream)
{
	IntExpr* pExpr = NULL;
	IntExprTypes eExprType = NO_INTEXPR;
	pStream->Read((int*)&eExprType);

	switch (eExprType)
	{
	case INTEXPR_CONSTANT:
		pExpr = new IntExprConstant();
		break;

	case INTEXPR_PROPERTY:
		pExpr = new IntExprProperty();
		break;

	case INTEXPR_ATTRIBUTE:
		pExpr = new IntExprAttribute();
		break;

	case INTEXPR_PYTHON:
		pExpr = new IntExprPython("");
		break;

	case INTEXPR_RANDOM:
		pExpr = new IntExprRandom();
		break;

	case INTEXPR_ADAPT:
		pExpr = new IntExprAdapt();
		break;

	case INTEXPR_PLUS:
		pExpr = new IntExprPlus();
		break;

	case INTEXPR_MINUS:
		pExpr = new IntExprMinus();
		break;

	case INTEXPR_MULT:
		pExpr = new IntExprMult();
		break;

	case INTEXPR_DIV:
		pExpr = new IntExprDiv();
		break;

	case INTEXPR_IF:
		pExpr = new IntExprIf();
		break;

	case INTEXPR_INTEGRATE_SUM:
		pExpr = new IntExprIntegrateSum();
		break;

	case INTEXPR_INTEGRATE_AVG:
		pExpr = new IntExprIntegrateAvg();
		break;

	case INTEXPR_INTEGRATE_COUNT:
		pExpr = new IntExprIntegrateCount();
	}

	if (pExpr)
		pExpr->read(pStream);

	return pExpr;
}

bool IntExpr::isConstantZero()
{
	return false;
}


int IntExprConstant::evaluate(CvGameObject *pObject)
{
	return m_iValue;
}

void IntExprConstant::read(FDataStreamBase *pStream)
{
	pStream->Read(&m_iValue);
}

void IntExprConstant::write(FDataStreamBase *pStream)
{
	pStream->Write((int)INTEXPR_CONSTANT);
	pStream->Write(m_iValue);
}

void IntExprConstant::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	CvWString s;
	s.Format(L"%d", m_iValue);
	szBuffer.append(s);
}

int IntExprConstant::getBindingStrength() const
{
	return 100;
}

bool IntExprConstant::isConstantZero()
{
	return m_iValue == 0;
}

void IntExprConstant::getCheckSum(unsigned int &iSum)
{
	CheckSum(iSum, m_iValue);
}


int IntExprAttribute::evaluate(CvGameObject *pObject)
{
	return pObject->getAttribute(m_eAttribute);
}

void IntExprAttribute::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eAttribute);
}

void IntExprAttribute::write(FDataStreamBase* pStream)
{
	pStream->Write((int)INTEXPR_ATTRIBUTE);
	pStream->Write((int)m_eAttribute);
}

void IntExprAttribute::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	CvWString s;
	switch (m_eAttribute)
	{
		case ATTRIBUTE_POPULATION:
			s.Format(L"%c", gDLL->getSymbolID(CITIZEN_CHAR));
			szBuffer.append(s);
			break;
		case ATTRIBUTE_HEALTH:
			s.Format(L"%c", gDLL->getSymbolID(HEALTHY_CHAR));
			szBuffer.append(s);
			break;
		case ATTRIBUTE_HAPPINESS:
			s.Format(L"%c", gDLL->getSymbolID(HAPPY_CHAR));
			szBuffer.append(s);
			break;
	}
}

int IntExprAttribute::getBindingStrength() const
{
	return 100;
}

void IntExprAttribute::getCheckSum(unsigned int &iSum)
{
	CheckSum(iSum, (int)m_eAttribute);
}


int IntExprProperty::evaluate(CvGameObject *pObject)
{
	return pObject->getProperties()->getValueByProperty((int)m_eProperty);
}

void IntExprProperty::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eProperty);
}

void IntExprProperty::write(FDataStreamBase* pStream)
{
	pStream->Write((int)INTEXPR_PROPERTY);
	pStream->Write((int)m_eProperty);
}

void IntExprProperty::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	CvWString s;
	s.Format(L"%c", GC.getPropertyInfo(m_eProperty).getChar());
	szBuffer.append(s);
}

int IntExprProperty::getBindingStrength() const
{
	return 100;
}

void IntExprProperty::getCheckSum(unsigned int &iSum)
{
	CheckSum(iSum, (int)m_eProperty);
}


IntExprOp::~IntExprOp()
{
	SAFE_DELETE(m_pExpr1);
	SAFE_DELETE(m_pExpr2);
}

void IntExprOp::read(FDataStreamBase *pStream)
{
	m_pExpr1 = IntExpr::readExpression(pStream);
	m_pExpr2 = IntExpr::readExpression(pStream);
}

void IntExprOp::write(FDataStreamBase *pStream)
{
	pStream->Write((int)getType());
	m_pExpr1->write(pStream);
	m_pExpr2->write(pStream);
}

void IntExprOp::buildDisplayString(CvWStringBuffer &szBuffer) const
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

void IntExprOp::getCheckSum(unsigned int &iSum)
{
	CheckSum(iSum, (int)getType());
	m_pExpr1->getCheckSum(iSum);
	m_pExpr2->getCheckSum(iSum);
}


int IntExprPlus::evaluate(CvGameObject *pObject)
{
	return m_pExpr1->evaluate(pObject) + m_pExpr2->evaluate(pObject);
}

IntExprTypes IntExprPlus::getType() const
{
	return INTEXPR_PLUS;
}

void IntExprPlus::buildOpNameString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("+");
}

int IntExprPlus::getBindingStrength() const
{
	return 11;
}


int IntExprMinus::evaluate(CvGameObject *pObject)
{
	return m_pExpr1->evaluate(pObject) - m_pExpr2->evaluate(pObject);
}

IntExprTypes IntExprMinus::getType() const
{
	return INTEXPR_MINUS;
}

void IntExprMinus::buildOpNameString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("-");
}

int IntExprMinus::getBindingStrength() const
{
	return 11;
}


int IntExprMult::evaluate(CvGameObject *pObject)
{
	return m_pExpr1->evaluate(pObject) * m_pExpr2->evaluate(pObject);
}

IntExprTypes IntExprMult::getType() const
{
	return INTEXPR_MULT;
}

void IntExprMult::buildOpNameString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("*");
}

int IntExprMult::getBindingStrength() const
{
	return 21;
}


int IntExprDiv::evaluate(CvGameObject *pObject)
{
	int iDiv = m_pExpr2->evaluate(pObject);
	return iDiv ? m_pExpr1->evaluate(pObject) / iDiv : m_pExpr1->evaluate(pObject);
}

IntExprTypes IntExprDiv::getType() const
{
	return INTEXPR_DIV;
}

void IntExprDiv::buildOpNameString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("/");
}

int IntExprDiv::getBindingStrength() const
{
	return 21;
}


IntExprIf::~IntExprIf()
{
	SAFE_DELETE(m_pExprIf);
	SAFE_DELETE(m_pExprThen);
	SAFE_DELETE(m_pExprElse);
}

int IntExprIf::evaluate(CvGameObject *pObject)
{
	return m_pExprIf->evaluate(pObject) ? m_pExprThen->evaluate(pObject) : m_pExprElse->evaluate(pObject);
}

void IntExprIf::read(FDataStreamBase *pStream)
{
	m_pExprIf = BoolExpr::readExpression(pStream);
	m_pExprThen = IntExpr::readExpression(pStream);
	m_pExprElse = IntExpr::readExpression(pStream);
}

void IntExprIf::write(FDataStreamBase *pStream)
{
	pStream->Write((int)INTEXPR_IF);
	m_pExprIf->write(pStream);
	m_pExprThen->write(pStream);
	m_pExprElse->write(pStream);
}

void IntExprIf::buildDisplayString(CvWStringBuffer &szBuffer) const
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

int IntExprIf::getBindingStrength() const
{
	return 25;
}

void IntExprIf::getCheckSum(unsigned int &iSum)
{
	m_pExprIf->getCheckSum(iSum);
	m_pExprThen->getCheckSum(iSum);
	m_pExprElse->getCheckSum(iSum);
}


IntExprIntegrateOp::~IntExprIntegrateOp()
{
	SAFE_DELETE(m_pExpr);
}

int IntExprIntegrateOp::evaluate(CvGameObject *pObject)
{
	int iAcc = 0;
	pObject->foreachRelated(m_eType, m_eRelation, boost::bind(getOp(), _1, m_pExpr, &iAcc));
	return iAcc;
}

void IntExprIntegrateOp::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eRelation);
	pStream->Read(&m_iData);
	pStream->Read((int*)&m_eType);
	m_pExpr = IntExpr::readExpression(pStream);
}

void IntExprIntegrateOp::write(FDataStreamBase *pStream)
{
	pStream->Write((int)getType());
	pStream->Write((int)m_eRelation);
	pStream->Write(m_iData);
	pStream->Write((int)m_eType);
	m_pExpr->write(pStream);
}

void IntExprIntegrateOp::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	// TODO: Proper rendering of relations
	m_pExpr->buildDisplayString(szBuffer);
}

int IntExprIntegrateOp::getBindingStrength() const
{
	// TODO: Review when proper rendering of the build string is done
	return m_pExpr->getBindingStrength();
}

void IntExprIntegrateOp::getCheckSum(unsigned int &iSum)
{
	CheckSum(iSum, (int)getType());
	CheckSum(iSum, (int)m_eRelation);
	CheckSum(iSum, m_iData);
	CheckSum(iSum, (int)m_eType);
	m_pExpr->getCheckSum(iSum);
}


void evalExprIntegrateSum(CvGameObject* pObject, IntExpr* pExpr, int* iAcc)
{
	*iAcc = *iAcc + pExpr->evaluate(pObject);
}

IntExprTypes IntExprIntegrateSum::getType() const
{
	return INTEXPR_INTEGRATE_SUM;
}

IntegrateOpFunc IntExprIntegrateSum::getOp()
{
	return &evalExprIntegrateSum;
}


void evalExprIntegrateAvg(CvGameObject* pObject, IntExpr* pExpr, int* iAcc, int* iCount)
{
	*iAcc = *iAcc + pExpr->evaluate(pObject);
	++*iCount;
}

IntExprTypes IntExprIntegrateAvg::getType() const
{
	return INTEXPR_INTEGRATE_AVG;
}

int IntExprIntegrateAvg::evaluate(CvGameObject *pObject)
{
	int iAcc = 0;
	int iCount = 0;
	pObject->foreachRelated(m_eType, m_eRelation, boost::bind(evalExprIntegrateAvg, _1, m_pExpr, &iAcc, &iCount));
	return iCount ? iAcc/iCount : 0;
}

IntegrateOpFunc IntExprIntegrateAvg::getOp()
{
	return NULL;
}


void evalExprIntegrateCount(CvGameObject* pObject, BoolExpr* pExpr, int* iAcc)
{
	if (pExpr->evaluate(pObject))
	{
		++*iAcc;
	}
}

IntExprIntegrateCount::~IntExprIntegrateCount()
{
	SAFE_DELETE(m_pExpr);
}

int IntExprIntegrateCount::evaluate(CvGameObject *pObject)
{
	int iAcc = 0;
	pObject->foreachRelated(m_eType, m_eRelation, boost::bind(evalExprIntegrateCount, _1, m_pExpr, &iAcc));
	return iAcc;
}

void IntExprIntegrateCount::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eRelation);
	pStream->Read(&m_iData);
	pStream->Read((int*)&m_eType);
	m_pExpr = BoolExpr::readExpression(pStream);
}

void IntExprIntegrateCount::write(FDataStreamBase *pStream)
{
	pStream->Write((int)INTEXPR_INTEGRATE_COUNT);
	pStream->Write((int)m_eRelation);
	pStream->Write(m_iData);
	pStream->Write((int)m_eType);
	m_pExpr->write(pStream);
}

void IntExprIntegrateCount::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	// TODO: Proper rendering of relations
	m_pExpr->buildDisplayString(szBuffer);
}

int IntExprIntegrateCount::getBindingStrength() const
{
	// TODO: Review when proper rendering of the build string is done
	return m_pExpr->getBindingStrength();
}

void IntExprIntegrateCount::getCheckSum(unsigned int &iSum)
{
	CheckSum(iSum, (int)m_eRelation);
	CheckSum(iSum, m_iData);
	CheckSum(iSum, (int)m_eType);
	m_pExpr->getCheckSum(iSum);
}


IntExprRandom::~IntExprRandom()
{
	SAFE_DELETE(m_pExpr);
}

int IntExprRandom::evaluate(CvGameObject *pObject)
{
	return GC.getGameINLINE().getSorenRandNum(m_pExpr->evaluate(pObject), "Random integer expression");
}

void IntExprRandom::read(FDataStreamBase *pStream)
{
	m_pExpr = IntExpr::readExpression(pStream);
}

void IntExprRandom::write(FDataStreamBase *pStream)
{
	pStream->Write((int)INTEXPR_RANDOM);
	m_pExpr->write(pStream);
}

void IntExprRandom::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append(CvWString::format(L"%c(", gDLL->getSymbolID(RANDOM_CHAR)));
	m_pExpr->buildDisplayString(szBuffer);
	szBuffer.append(L")");
}

int IntExprRandom::getBindingStrength() const
{
	return 100;
}

void IntExprRandom::getCheckSum(unsigned int &iSum)
{
	m_pExpr->getCheckSum(iSum);
}


IntExprAdapt::~IntExprAdapt()
{
	SAFE_DELETE(m_pExpr);
}

int IntExprAdapt::evaluate(CvGameObject *pObject)
{
	return pObject->adaptValueToGame(m_iID, m_pExpr->evaluate(pObject));
}

void IntExprAdapt::read(FDataStreamBase *pStream)
{
	m_pExpr = IntExpr::readExpression(pStream);
	pStream->Read(&m_iID);
}

void IntExprAdapt::write(FDataStreamBase *pStream)
{
	pStream->Write((int)INTEXPR_ADAPT);
	m_pExpr->write(pStream);
	pStream->Write(m_iID);
}

void IntExprAdapt::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	CvWString szTextVal;
	szTextVal.Format(L"%d", GC.getGameINLINE().getGameObject()->adaptValueToGame(m_iID, m_pExpr->evaluate(GC.getGameINLINE().getGameObject())));
	szBuffer.append(szTextVal);
}

int IntExprAdapt::getBindingStrength() const
{
	return 100;
}

void IntExprAdapt::getCheckSum(unsigned int &iSum)
{
	m_pExpr->getCheckSum(iSum);
	CheckSum(iSum, m_iID);
}


int IntExprPython::evaluate(CvGameObject *pObject)
{
	PYTHON_ACCESS_LOCK_SCOPE

	long lResult = 0;

	CyArgsList argsList;
	void* pArgument = pObject->addPythonArgument(&argsList);

	PYTHON_CALL_FUNCTION4(__FUNCTION__, PYRandomEventModule, m_szPythonCallback, argsList.makeFunctionArgs(), &lResult);

	pObject->disposePythonArgument(pArgument);

	return lResult;
}

void IntExprPython::read(FDataStreamBase *pStream)
{
	pStream->ReadString(m_szPythonCallback);
}

void IntExprPython::write(FDataStreamBase *pStream)
{
	pStream->Write((int)INTEXPR_PYTHON);
	pStream->WriteString(m_szPythonCallback);
}

void IntExprPython::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	PYTHON_ACCESS_LOCK_SCOPE

	CvWString szResult;

	CyArgsList argsList;
	argsList.add(false);

	PYTHON_CALL_FUNCTION4(__FUNCTION__, PYRandomEventModule, m_szPythonCallback, argsList.makeFunctionArgs(), &szResult);

	szBuffer.append(szResult);
}

int IntExprPython::getBindingStrength() const
{
	return 100;
}

void IntExprPython::getCheckSum(unsigned int &iSum)
{
	CheckSumC(iSum, m_szPythonCallback);
}
