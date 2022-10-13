#pragma once
#include <queue>
#include "FProfiler.h"

#define BOOST_THREAD_NO_LIB
#define BOOST_THREAD_USE_LIB
#include <boost/thread/thread.hpp>

//#define	DYNAMIC_PATH_STRUCTURE_VALIDATION

//	Forward declarations of helper classes
class CvPathNode;
class CvPathGenerator;
class CvPathGeneratorMT;
class CvSelectionGroup;

class CvPathGeneratorBase
{
public:
	virtual bool useAIPathingAlways(void) const { return m_useAIPathingAlways; };
	virtual const CvPlot* getTerminalPlot(void) const = 0;
protected:
	bool								m_useAIPathingAlways;
};

// Function prototype for Cost and Validity functions
typedef int(*HeuristicCost)(CvSelectionGroup* pGroup, int iFromX, int iFromY, int iToX, int iToY, int& iLimitCost);
typedef int(*EdgeCost)(CvPathGeneratorBase* generator, CvSelectionGroup* pGroup, int iFromX, int iFromY, int iToX, int iToY, int iFlags, int& iMovementRemaining, int iPathTurns, int& iToNodeCost, bool bIsTerminalNode);
typedef bool(*EdgeValidity)(CvSelectionGroup* pGroup, int iFromX, int iFromY, int iToX, int iToY, int iFlags, bool isTerminus, bool bAssertTerminatesMove, int iPathTurns, bool& bToNodeInvalidity);
typedef bool(*TerminusValidity)(CvSelectionGroup* pGroup, int iToX, int iToY, int iFlags, bool& bRequiresWar);
typedef bool(*TurnEndValidityCheckRequired)(CvSelectionGroup* pGroup, int iFlags);

class CvPath
{
friend CvPathGeneratorMT;
friend CvPathGenerator;
public:
	class const_iterator
	{
	friend CvPath;
	protected:
		const_iterator(CvPathNode* cursorNode);

	public:
		const_iterator& operator++();

		bool operator==(const_iterator& other);

		bool operator!=(const_iterator& other);

		CvPlot*	plot(void);
		int		turn(void);

	private:
		CvPathNode*	m_cursorNode;
	};

protected:
	CvPath();
	
	void Set(CvPathNode* startNode);

public:
	const_iterator begin(void);
	const_iterator end(void);

	int	length(void) const;
	CvPlot*	lastPlot(void) const;
	bool	containsEdge(const CvPlot* pFromPlot, const CvPlot* pToPlot) const;
	bool	containsNode(const CvPlot* pPlot) const;
	void	trimBefore(const CvPlot* pPlot);
	int		movementRemaining(void) const;
	int		cost(void) const;

private:
	CvPathNode* m_startNode;
	CvPathNode* m_endNode;
};

//	Helper class to manage allocation of tree nodes efficiently
typedef std::vector<CvPathNode> PoolBucket;

template <class T>
class CvAllocationPool
{
private:
	class AllocationType
	{
	public:
		AllocationType() {}

#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
		int	m_seq;
#endif
		T	m_data;
	};

public:
	CvAllocationPool()
	{
		m_nextBucketToAllocate = 0;
#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
		m_iSeq = 0;
#endif
		reset();
	}
	~CvAllocationPool()
	{
		while(m_nextBucketToAllocate-- > 0)
		{
			SAFE_DELETE(m_pool[m_nextBucketToAllocate]);
		}
	}

	T*		allocateNode(void)
	{
		//PROFILE("CvAllocationPool.allocateNode");

		int iCurrentBucketCapacity;

		if ( m_nextBucketIndex == 0 )
		{
			iCurrentBucketCapacity = 0;
		}
		else
		{
			iCurrentBucketCapacity = m_pool[m_nextBucketIndex-1]->capacity();
		}

		if ( m_nextIndex >= iCurrentBucketCapacity )
		{
			if ( m_nextBucketToAllocate == m_nextBucketIndex )
			{
				MEMORY_TRACK_EXEMPT();

				std::vector<AllocationType>* newBucket = new std::vector<AllocationType>();

				newBucket->reserve(range(iCurrentBucketCapacity*2, 16, 1024));
				m_pool.push_back(newBucket);

				m_nextBucketToAllocate++;
			}

			m_nextIndex = 0;
			m_nextBucketIndex++;
		}

		AllocationType& allocated = (*m_pool[m_nextBucketIndex-1])[m_nextIndex++];

#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
		allocated.m_seq = m_iSeq;
#endif
		return &allocated.m_data;
	}

	void	reset(void)
	{
		m_nextIndex = 0;
		m_nextBucketIndex = 0;
#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
		m_iSeq++;
#endif

		//	No point freeing the allocated pools - we'll just need them again
		//	for another use
	}

#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
	bool	isAllocated(T* data) const
	{
		AllocationType*	alloc = (AllocationType*)(((int*)data) - 1);

		return (m_iSeq == alloc->m_seq);
	}
#endif

private:
	int											m_nextBucketIndex;
	int											m_nextIndex;
	int											m_nextBucketToAllocate;
#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
	int											m_iSeq;
#endif
	std::vector<std::vector<AllocationType>*>	m_pool;
};

typedef struct
{
	CvPathNode*	node;
	int			iQueuedCost;
} priorityQueueEntry;

//#define	MULTI_THREADED_PATHING
//#ifdef MULTI_THREADED_PATHING
#define	MAX_BACKGROUND_PATHING_THREADS	7

//	Forward declarations of helper classes
class CvPathPlotInfoStore;
class CvPathGenerator;
class CvPathGeneratorPlotInfo;

class CvNodeCostInfo
{
public:
	CvPathNode*					m_parentNode;
	CvPlot*						m_pAdjacentPlot;
	CvPathGeneratorPlotInfo*	m_pAdjacentPlotInfo;
	int							m_iEdgeCost;
	int							m_iNodeCost;
	int							m_iMovementRemaining;
	int							m_iDirection;
	bool						m_bValid;
};

typedef struct
{
	CvPathNode* root;
	CvSelectionGroup* pGroup;
	int iMaxTurns;
	int iFlags;
	const CvPlot* pTo;
	bool bTurnEndValidCheckNeeded;
} CvPathContext;

class CvPathGeneratorMT : public CvPathGeneratorBase
{
public:
	CvPathGeneratorMT(CvMap* pMap);
	virtual ~CvPathGeneratorMT(void);

	void reset(void);

	void Initialize(HeuristicCost HeuristicFunc, EdgeCost CostFunc, EdgeValidity ValidFunc, TerminusValidity TerminusValidFunc, TurnEndValidityCheckRequired TurnEndValidCheckNeeded);
	bool generatePath(const CvPlot* pFrom, const CvPlot* pTo, CvSelectionGroup* pGroup, int iFlags, int iMaxTurns);
	bool generatePathForHypotheticalUnit(const CvPlot* pFrom, const CvPlot* pTo, PlayerTypes ePlayer, UnitTypes eUnit, int iFlags, int iMaxTurns);
	bool haveRouteLength(const CvPlot* pTo, CvSelectionGroup* pGroup, int iFlags, int& iRouteLen);
	
	virtual const CvPlot* getTerminalPlot(void) const;

	CvPath&	getLastPath(void);
	void SelfTest(void);
	static void EnableMaxPerformance(bool bEnable) {}
private:
	class CvPathNodeComparer
	{
	public:
		CvPathNodeComparer()
		{
		}
		bool operator() (const priorityQueueEntry& lhs, const priorityQueueEntry& rhs) const;
	};

	CvPathNode*	allocatePathNode(void);
	bool groupMatches(CvSelectionGroup* pGroup, int iFlags, unsigned int& iGroupMembershipChecksum);
	void AdjustChildTreeCosts(CvPathNode* node, int iAmount, bool bHasQueued);
	void OrphanChildTree(CvPathNode* node);
	void DeleteChildTree(CvPathNode* node, bool bIsDeletionRoot);
	void ValidatePlotInfo(CvPathGeneratorPlotInfo* pPlotInfo);
	bool isDescendantOfReplacedNode(CvPathNode* node) const;
	void LinkNode(CvPathNode* node, CvPathNode* parent);
	void RelinkNode(CvPathNode* node, CvPathNode* parent);
	bool isBetterPath(
		 CvPathNode*	existing,
		 CvPathNode*	from,
		 int			iEdgeCost,
		 int			iMovementRemaining) const;
	void CalculateVertexCost(void);
	void CalculateVertexCost(int iIndex);

#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
	bool ValidateTreeInternal(CvPathNode* root, int& iValidationSeq, CvPathNode* unreferencedNode, CvPathNode* referencedNode, int& iQueuedCount);
	void ValidateTree(CvPathNode* root, CvPathNode* unreferencedNode, CvPathNode* referencedNode);
#endif

private:
	CvNodeCostInfo						m_adjacentCosts[NUM_DIRECTION_TYPES];
	LONG								m_adjacentCostRequestsQueued;
	LONG								m_adjacentCostRequestsReadIndex;
	LONG								m_adjacentCostsToBeCalculated;
	CvMap*								m_map;
	std::priority_queue<priorityQueueEntry,std::vector<priorityQueueEntry>,CvPathNodeComparer> m_priorityQueue;
	CvPathPlotInfoStore*				m_plotInfo;
	CvAllocationPool<CvPathNode>*		m_nodeAllocationPool;
	bool								m_bFoundRoute;
	int									m_iTerminalNodeCost;
	CvPathNode*							m_pBestTerminalNode;
	CvPathNode*							m_pReplacedNonTerminalNode;
	CvPath								m_generatedPath;
	unsigned int						m_currentGroupMembershipChecksum;
	const CvPlot*						m_pFrom;
	int									m_iFlags;
	int									m_iTurn;
	int									m_iSeq;
	CvPathContext						m_context;
	//	Callbacks
	HeuristicCost						m_HeuristicFunc;
	EdgeCost							m_CostFunc;
	EdgeValidity						m_ValidFunc;
	TerminusValidity					m_TerminusValidFunc;
	TurnEndValidityCheckRequired		m_TurnEndValidCheckNeeded;

public:
	int									m_nodesProcessed;
	int									m_nodesCosted;
};

//#else

//	Forward declarations of helper classes
class CvPathPlotInfoStore;
class CvPathGenerator;
class CvPathGeneratorPlotInfo;

class CvPathGenerator : public CvPathGeneratorBase
{
public:
	CvPathGenerator(CvMap* pMap);
	virtual ~CvPathGenerator(void);

	void reset(void);

	void Initialize(HeuristicCost HeuristicFunc, EdgeCost CostFunc, EdgeValidity ValidFunc, TerminusValidity TerminusValidFunc, TurnEndValidityCheckRequired TurnEndValidCheckNeeded);
	bool generatePath(const CvPlot* pFrom, const CvPlot* pTo, CvSelectionGroup* pGroup, int iFlags, int iMaxTurns, int iOptimizationLimit = -1);
	bool generatePathForHypotheticalUnit(const CvPlot* pFrom, const CvPlot* pTo, PlayerTypes ePlayer, UnitTypes eUnit, int iFlags, int iMaxTurns);
	bool haveRouteLength(const CvPlot* pTo, CvSelectionGroup* pGroup, int iFlags, int& iRouteLen);
	
	virtual const CvPlot* getTerminalPlot(void) const;

	CvPath&	getLastPath(void);
	void SelfTest(void);
	static void EnableMaxPerformance(bool bEnable) {}
private:
	class CvPathNodeComparer
	{
	public:
		CvPathNodeComparer()
		{
		}
		bool operator() (const priorityQueueEntry& lhs, const priorityQueueEntry& rhs) const;
	};

	CvPathNode*	allocatePathNode(void);
	bool groupMatches(CvSelectionGroup* pGroup, int iFlags, unsigned int& iGroupMembershipChecksum);
	void AdjustChildTreeCosts(CvPathNode* node, int iAmount, bool bHasQueued);
	void OrphanChildTree(CvPathNode* node);
	void DeleteChildTree(CvPathNode* node, bool bIsDeletionRoot);
	void ValidatePlotInfo(CvPathGeneratorPlotInfo* pPlotInfo);
	bool isDescendantOfReplacedNode(CvPathNode* node) const;
	void LinkNode(CvPathNode* node, CvPathNode* parent);
	void RelinkNode(CvPathNode* node, CvPathNode* parent);
	bool isBetterPath(
		 CvPathNode*	existing,
		 CvPathNode*	from,
		 int			iEdgeCost,
		 int			iMovementRemaining) const;

#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
	bool ValidateTreeInternal(CvPathNode* root, int& iValidationSeq, CvPathNode* unreferencedNode, CvPathNode* referencedNode, int& iQueuedCount);
	void ValidateTree(CvPathNode* root, CvPathNode* unreferencedNode, CvPathNode* referencedNode);
#endif

private:
	CvMap*								m_map;
	std::priority_queue<priorityQueueEntry,std::vector<priorityQueueEntry>,CvPathNodeComparer> m_priorityQueue;
	CvPathPlotInfoStore*				m_plotInfo;
	CvAllocationPool<CvPathNode>*		m_nodeAllocationPool;
	bool								m_bFoundRoute;
	int									m_iTerminalNodeCost;
	CvPathNode*							m_pBestTerminalNode;
	CvPathNode*							m_pReplacedNonTerminalNode;
	const CvPlot*						m_pTerminalPlot;
	CvPath								m_generatedPath;
	unsigned int						m_currentGroupMembershipChecksum;
	const CvPlot*						m_pFrom;
	int									m_iFlags;
	int									m_iTurn;
	int									m_iSeq;
	//	Callbacks
	HeuristicCost						m_HeuristicFunc;
	EdgeCost							m_CostFunc;
	EdgeValidity						m_ValidFunc;
	TerminusValidity					m_TerminusValidFunc;
	TurnEndValidityCheckRequired		m_TurnEndValidCheckNeeded;

public:
	int									m_nodesProcessed;
	int									m_nodesCosted;
};
//#endif