#include "CvGameCoreDLL.h"
#include "CvMapInterfaceBase.h"
#include "CvMapExternal.h"

CvMapInterfaceBase::CvMapInterfaceBase()
{
	m_proxy = new CvMapExternal(this);
}

CvMapInterfaceBase::~CvMapInterfaceBase()
{
}

CvMapExternal* CvMapInterfaceBase::getProxy()
{
	return m_proxy;
}