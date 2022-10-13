#include "CvGameCoreDLL.h"
#include "CvGameCoreUtils.h"
#include "CvGame.h"
#include "CvGameAI.h"
#include "CvPipeline.h"

#include <boost/bind.hpp>

CvPipelineWorkItem::CvPipelineWorkItem()
{
	m_state = WORKITEM_STATE_NONE;
}

CvPipelineWorkItem::~CvPipelineWorkItem()
{
	m_pipeline = NULL;
	m_currentStage = NULL;
	m_iQueueingKey = 0;
}

//	Get a queuiong key - thi should be queried before processing and passed to Complete() on completion
int		CvPipelineWorkItem::GetKey(void) const
{
	return m_iQueueingKey;
}

//	Requeue the workitem to the last non-synchronous stage not yet completed
//	This is intended for use with optimistic locking systems to handle a lock failure
//	down-stream of the pipeline stage that took the lock
void	CvPipelineWorkItem::Requeue(void)
{
	if ( m_pipeline != NULL && GetState() == WORKITEM_STATE_PROCESSING )
	{
		m_pipeline->RequeueItem(this);
	}
}

//	Abandon a work item
void	CvPipelineWorkItem::Abandon(void)
{
	if ( m_currentStage != NULL && GetState() == WORKITEM_STATE_PROCESSING )
	{
		m_currentStage->NoteProcessed(this);
	}
}

//	Note that processing has completed (for the current pipeline stage) on this work item
void	CvPipelineWorkItem::Complete(int iKey)
{
	//	m_iQueueingKey needs some locking...
	if ( m_pipeline != NULL && GetState() == WORKITEM_STATE_PROCESSING && m_iQueueingKey == iKey )
	{
		m_pipeline->CompleteItem(this);
	}
}

//	Initialize the seed to use in the item's sync rand
void	CvPipelineWorkItem::SetRandSeed(unsigned long ulSeed)
{
	m_sorenRand.reseed(ulSeed);
}

//	A synchronous random number, which is OOS safe even in multi-threaded pipeline usage.  Always
//	use this in pirpeline stage processing rather than the regulat synced random number generator
int		CvPipelineWorkItem::GetSyncRand(int iNum, const char* pszLog)
{
	int iScale = 0;
	while(iNum > MAX_UNSIGNED_SHORT)
	{
		iNum /= 2;
		iScale++;
	}

    int Result = m_sorenRand.get(iNum, pszLog);

	while(iScale-- > 0)
	{
		Result *= 2;
	}

	GC.getGameINLINE().logRandomResult(GetName().c_str(), pszLog, iNum, Result);

	return Result;
}

CvPipelineStage::CvPipelineStage(const char* name, bool bSynchronous)
{
	m_bSynchronous = bSynchronous;
	m_iNumItemsRemaining = 0;
	m_iNumItemsQueued = 0;
	m_prevStage = NULL;
	m_nextStage = NULL;
	m_bStalled = false;
	m_szName = name;

	InitializeCriticalSectionAndSpinCount(&m_csQueueAccessSection, 4000);
}

CvPipelineStage::~CvPipelineStage()
{
	DeleteCriticalSection(&m_csQueueAccessSection);
}

//	Queue a work item into the stage
void	CvPipelineStage::EnqueueWorkItem(CvPipelineWorkItem*	item)
{
	LONG iNum = InterlockedIncrement(&m_iNumItemsRemaining);
	OutputDebugString(CvString::format("[%d] Enqueue item %08lx on stage %s (now %d items)\n", GetCurrentThreadId(), item, m_szName, iNum).c_str());

	item->m_currentStage = this;
	item->SetState(WORKITEM_STATE_QUEUED);

	EnterCriticalSection(&m_csQueueAccessSection);
	m_queue.push(item);
	LeaveCriticalSection(&m_csQueueAccessSection);

	InterlockedIncrement(&m_iNumItemsQueued);
}

void	CvPipelineStage::NoteProcessed(CvPipelineWorkItem* item)
{
	if ( item->m_currentStage == this )
	{
		LONG iNum = InterlockedDecrement(&m_iNumItemsRemaining);
		OutputDebugString(CvString::format("[%d] Note processed item %08lx on stage %s (%d items remain)\n", GetCurrentThreadId(), item, m_szName, iNum).c_str());

		item->SetState(WORKITEM_STATE_NONE);
	}
	else
	{
		OutputDebugString(CvString::format("[%d] Ignoring note processed item %08lx on stage %s\n", GetCurrentThreadId(), item, m_szName).c_str());
	}
}

void	CvPipelineStage::Stall(void)
{
	//	Stalls are no-ops for async stages
	if ( m_bSynchronous )
	{
		m_bStalled = true;
	}
}

int		CvPipelineStage::GetNumUnprocessedItems(void) const
{
	return m_iNumItemsRemaining;
}

//	Commence procesing of queued work items
void	CvPipelineStage::Begin(void)
{
}

//	Complete processing - will synchronously wait for all work items to complete
bool	CvPipelineStage::End(void)
{
	if ( m_bSynchronous )
	{
		m_bStalled = false;
	}

	//	Await completion
	while( InterlockedCompareExchange(&m_iNumItemsRemaining, 0, 0) != 0 )
	{
		//	This thread may as well help out even in the async case
		DrainQueue();
	}

	return m_bStalled;
}

void	CvPipelineStage::DrainQueue(void)
{
	CvPipelineWorkItem* item = NULL;

	//OutputDebugString(CvString::format("[%d] DrainQueue for stage %s (%d items)\n", GetCurrentThreadId(), m_szName, m_iNumItemsRemaining).c_str());

	do
	{
		int iCount = m_iNumItemsQueued;

		if ( iCount > 0 && InterlockedCompareExchange(&m_iNumItemsQueued, iCount-1, iCount) == iCount )
		{
			EnterCriticalSection(&m_csQueueAccessSection);

			item = m_queue.top();
			m_queue.pop();

			LeaveCriticalSection(&m_csQueueAccessSection);

			OutputDebugString(CvString::format("[%d] Process item %08lx\n", GetCurrentThreadId(), item).c_str());

			item->SetState(WORKITEM_STATE_PROCESSING);
			ProcessWorkItem(item);
		}
	} while(m_iNumItemsQueued > 0);// && !m_bStalled);
}

CvPipeline::CvPipeline(int iMaxThreads)
{
	m_firstStage = NULL;
	m_lastStage = NULL;
	m_bRunning = false;
	m_iMaxThreads = iMaxThreads;
}

CvPipeline::~CvPipeline()
{
	CvPipelineStage* stage = m_firstStage;
	
	while(stage != NULL)
	{
		CvPipelineStage*	next = stage->m_nextStage;

		delete stage;

		stage = next;
	}
}

extern LONG WINAPI CustomFilter(EXCEPTION_POINTERS *ExceptionInfo);

void ExecutePipelineThread(CvPipeline* stage)
{
	__try
	{
		stage->DoDrainThread();
	}
	__except(CustomFilter(GetExceptionInformation()))
	{
		OutputDebugString("Exception on drain thread\n");
		ExitProcess(1);
	}
}

void	CvPipeline::DoDrainThread(void)
{
	PROFILE_THREAD("PipelineThread");

	do
	{
		for(CvPipelineStage* stage = m_firstStage; stage != NULL; stage = stage->m_nextStage)
		{
			if ( !stage->IsSynchronous() )
			{
				stage->DrainQueue();
			}
		}

		//Sleep(0);
	} while(m_bRunning);

	OutputDebugString(CvString::format("Drain thread exiting cleanly: %d\n", GetCurrentThreadId()).c_str());
}

//	Add a stage to the pipeline.  Must occur before the pipeline's Begin() is called
void	CvPipeline::AddStage(CvPipelineStage*	stage)
{
	FAssert( !m_bRunning );

	if ( m_lastStage == NULL )
	{
		m_firstStage = stage;
		stage->m_prevStage = NULL;
	}
	else
	{
		m_lastStage->m_nextStage = stage;
		stage->m_prevStage = m_lastStage;
	}

	m_lastStage = stage;
}

//	Queue a work item to be processed by the pipeline
void	CvPipeline::EnqueueWorkItem(CvPipelineWorkItem*	item)
{
	if ( m_firstStage != NULL )
	{
		item->m_pipeline = this;
		m_firstStage->EnqueueWorkItem(item);
	}
	else
	{
		delete item;
	}
}

//	Commence procesing of queued work items
void	CvPipeline::Begin(void)
{
	m_bRunning = true;

	//	Start threads...
	for(int iI = 0; iI < m_iMaxThreads-1; iI++)
	{
		m_threads.push_back(new boost::thread(boost::bind(ExecutePipelineThread, this)));
	}

	for(CvPipelineStage* stage = m_firstStage; stage != NULL; stage = stage->m_nextStage)
	{
		stage->Begin();
	}
}

//	Complete processing - will synchronously wait for all work items to complete
void	CvPipeline::End(void)
{
	bool	bStalled;

	do
	{
		bStalled = false;

		for(CvPipelineStage* stage = m_firstStage; stage != NULL && !bStalled; stage = stage->m_nextStage)
		{
			bStalled = stage->End();
		}
	} while(bStalled);

	m_bRunning = false;

	for(int iI = 0; iI < m_iMaxThreads-1; iI++)
	{
		m_threads[iI]->join();
		delete m_threads[iI];
	}
}

void	CvPipeline::CompleteItem(CvPipelineWorkItem* item)
{
	if ( item->GetState() == WORKITEM_STATE_PROCESSING )
	{
		CvPipelineStage* stage = item->m_currentStage;

		if ( stage->m_nextStage != NULL )
		{
			//	Must make sure the stage we are moving to thinks it has work BEFORE
			//	completing on the current stage or else the CvPipeline::End() processing
			//	is exposed to a window in which it can falsely conclude that the
			//	entire pipeline is complete
			stage->m_nextStage->IncrementWorkCount();
		}

		stage->NoteProcessed(item);

		if ( stage->m_nextStage != NULL )
		{
			stage->m_nextStage->EnqueueWorkItem(item);
			stage->m_nextStage->DecrementWorkCount();
		}
		else
		{
			delete item;
		}
	}
}

void	CvPipeline::RequeueItem(CvPipelineWorkItem* item)
{
	CvPipelineStage* stage = item->m_currentStage;
	CvPipelineStage* requeueStage;

	if ( stage->IsSynchronous() )
	{
		stage->Stall();
	}

	do
	{
		requeueStage = stage;
		stage = stage->m_prevStage;
	}
	while( stage != NULL && !stage->IsSynchronous() );

	requeueStage->IncrementWorkCount();
	item->m_currentStage->NoteProcessed(item);

	requeueStage->EnqueueWorkItem(item);
	requeueStage->DecrementWorkCount();
}
