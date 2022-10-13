#pragma once

#include "CvUnit.h"

//	Define this to have cities advertise units bneeds and tender for the business of building them
//#define	USE_UNIT_TENDERING

//	Capability flags - may be ORd together
typedef enum	unitCapabilities
{
	NO_UNITCAPABILITIES			= 0,
	DEFENSIVE_UNITCAPABILITIES	= 1,
	OFFENSIVE_UNITCAPABILITIES	= 2,
	WORKER_UNITCAPABILITIES		= 4
} unitCapabilities;


//	Structures used to hold work requests and looking-for-work unit info
typedef struct
{
	int						iPriority;
	unitCapabilities		eUnitFlags;
	UnitAITypes				eAIType;
	int						iAtX;
	int						iAtY;
	int						iMaxPath;
	int						iUnitId;
	int						iWorkRequestId;
	int						iRequiredStrengthTimes100;
	bool					bFulfilled;
	CvUnitSelectionCriteria	criteria;
} workRequest;

typedef struct
{
	int					iUnitId;
	int					iDefensiveValue;
	int					iOffensiveValue;
	bool				bIsWorker;
	int					iAtX;
	int					iAtY;
	int					iContractedWorkRequest;
	int					iMatchedToRequestSeqThisPlot;
	int					iMatchedToRequestSeqAnyPlot;
	int					iMinPriority;
} advertisingUnit;

typedef struct
{
	int					iCityId;
	int					iMinPriority;
} cityTender;

//	Highest priority city builds have this priority and work down from there
#define	CITY_BUILD_PRIORITY_CEILING				900
#define HIGHEST_PRIORITY_ESCORT_PRIORITY		301
#define HIGH_PRIORITY_ESCORT_PRIORITY			300
#define	CITY_NO_WORKERS_WORKER_PRIORITY			299
#define FLOATING_DEFENDER_REQUEST_PRIORITY_MAX	200
#define LOW_PRIORITY_ESCORT_PRIORITY			100

//
//	The CvContractBroker abstracts a communication that allows 'idle' units to advertise themselves
//	as 'looking for work', and for requests for units to be placed with priority and criteria.  This
//	allows for coopertaive behaviour without both parties needing to be fully aware of the logic, and
//	reduces the need for searching whe a unit looks for the best activity to undertake, which helps
//	performance
//
//	Intended usage is in a 2-pass process (from CvUnitAI::AI_update()).  In the unit's activity choice AI
//	it should:
//		1)	If it has high priority work determined directly by its own unit AI push that mission
//		2)	If no high priority work advertise itself with this broker as available for work and return
//			from AI_update() with no mission pushed (which measn it'll get called again once all the units
//			have been cycled through)
//		3)	On the second call ask for the best work from this broker and if provided push a mission
//			to do what is asked of it (move to a location, join a group being current options)
//		4)	If no suitable work is available continue with its own unit AI for low priority actions
//
class CvContractBroker
{
public:
	CvContractBroker();
	virtual ~CvContractBroker(void);

	//	Initialize
	void	init(PlayerTypes eOwner);

	//	Delete all work requests and looking for work records
	void	reset();

	//	Note a unit looking for work
	void	lookingForWork(CvUnit* pUnit, int iMinPriority = 0);
	//	Unit fulfilled its work and is no longer advertising as available
	void	removeUnit(CvUnit* pUnit);
	//	Make a work request
	//		iPriority should be in the range 0-1000 ideally
	//		eUnitFlags indicate the type(s) of unit sought
	//		(iAtX,iAtY) is (roughly) where the work will be
	//		pJoinUnit may be NULL but if not it is a request to join that unit's group
	void	advertiseWork(int iPriority, unitCapabilities eUnitFlags, int iAtX, int iAtY, CvUnit* pJoinUnit, UnitAITypes eAIType = NO_UNITAI, int iUnitStrength = -1, CvUnitSelectionCriteria* criteria = NULL, int iMaxPath = MAX_INT);
	//	Advertise a tender to build units
	//		iMinPriority indicaes the lowest priority request this tender is appropriate for
	void	advertiseTender(CvCity* pCity, int iMinPriority);
	//	Find out how many requests have already been made for units of a specified AI type
	//	This is used by cities requesting globally needed units like settlers to avoid multiple
	//	tenders all ocurring at once
	int		numRequestsOutstanding(UnitAITypes eUnitAI, bool bAtCityOnly = true) const;
	//	Make a contract
	//	This will attempt to make the best contracts between currently
	//	advertising units and work, then search the resuilting set for the work 
	//	of the requested unit
	//	returns true if a contract is made along with the details of what to do
	bool	makeContract(CvUnit* pUnit, int& iAtX, int& iAtY, CvUnit*& pJoinUnit, bool bThisPlotOnly);
	void	finalizeTenderContracts(void);

private:
	const workRequest*	findWorkRequest(int iWorkRequestId) const;
	advertisingUnit*	findBestUnit(workRequest& request, bool bThisPlotOnly);
	CvUnit*				findUnit(int iUnitId) const;
	int					lowerPartiallyFulfilledRequestPriority(int iPreviousPriority, int iPreviousRequestStrength, int iStrengthProvided);
	UnitValueFlags		unitCapabilities2UnitValueFlags(unitCapabilities eCapabilities);

private:
	mutable CRITICAL_SECTION		m_cWorkRequestsSection;
	std::vector<workRequest>		m_workRequests;
	std::vector<advertisingUnit>	m_advertisingUnits;
	std::vector<cityTender>			m_advertisingTenders;
	mutable CRITICAL_SECTION		m_cContractedUnitsSection;
	std::map<int,bool>				m_contractedUnits;
	int								m_iNextWorkRequestId;
	PlayerTypes						m_eOwner;
};
