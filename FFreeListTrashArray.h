#pragma once

//	$Revision: #2 $		$Author: mbreitkreutz $ 	$DateTime: 2005/06/13 13:35:55 $
//------------------------------------------------------------------------------------------------
//
//  *****************   FIRAXIS GAME ENGINE   ********************
//
//  FILE:    FFreeListTrashArray.h
//
//  AUTHOR:  Soren Johnson
//
//  PURPOSE: A dynamic array with a free list that keeps track of its own memory...
//
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2004 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------

#ifndef		FFREELISTTRASHARRAY_H
#define		FFREELISTTRASHARRAY_H
#pragma		once

#include	"FFreeListArrayBase.h"
#include	"FDataStreamBase.h"
#include	"CvTaggedSaveFormatWrapper.h"
#include	"ByteBuffer.h"


#define FLTA_ID_SHIFT				(13)
#define FLTA_MAX_BUCKETS		(1 << FLTA_ID_SHIFT)
#define FLTA_INDEX_MASK			(FLTA_MAX_BUCKETS - 1)
#define FLTA_ID_MASK				(~(FLTA_INDEX_MASK))
#define FLTA_GROWTH_FACTOR	(2)

template <class T>
class FFreeListTrashArray : public FFreeListArrayBase<T>
{
public:

	FFreeListTrashArray();
	virtual ~FFreeListTrashArray();

	virtual void init(int iNumSlots = 8);
	virtual void uninit();
	virtual T* getAt(int iID) const;

	T* add();
	bool remove(T* pData);
	bool removeAt(int iID);
	virtual void removeAll();

	void load(T* pData);

	int getNumSlots() const
	{
		return m_iNumSlots;
	}

	int getLastIndex() const
	{
		return m_iLastIndex;
	}
	void setLastIndex(int iNewValue)
	{
		m_iLastIndex = iNewValue;
	}

	int getFreeListHead() const
	{
		return m_iFreeListHead;
	}
	void setFreeListHead(int iNewValue)
	{
		m_iFreeListHead = iNewValue;
	}

	int getFreeListCount() const
	{
		return m_iFreeListCount;
	}
	void setFreeListCount(int iNewValue)
	{
		m_iFreeListCount = iNewValue;
	}

	int getCurrentID()
	{
		return m_iCurrentID;
	}

	void setCurrentID(int iNewValue)
	{
		assert((iNewValue & FLTA_INDEX_MASK) == 0);
		assert((iNewValue & FLTA_ID_MASK) != 0);
		m_iCurrentID = iNewValue;
	}

	int getNextFreeIndex(int iIndex)
	{
		if ((iIndex >= getNumSlots()) || (m_pArray == NULL))
		{
			assert(false);
			return FFreeList::INVALID_INDEX;
		}
		return m_pArray[iIndex].iNextFreeIndex;
	}
	void setNextFreeIndex(int iIndex, int iNewValue)
	{
		if ((iIndex >= getNumSlots()) || (m_pArray == NULL))
		{
			assert(false);
			return;
		}
		m_pArray[iIndex].iNextFreeIndex = iNewValue;
		//Afforess: not sure why this was added, it is not in the base BTS code, and causes OOS in pitboss due to ids mismatching!
		/*
		m_pArray[iIndex].iLastUsed = m_iCurrentID;
		*/
	}

	void Read( FDataStreamBase* pStream );
	void Write( FDataStreamBase* pStream );

	void cancelDeinitialize()
	{
		m_bCancelDeinitialize = true;
	}

protected:

	struct FFreeListTrashArrayNode
	{
		int iNextFreeIndex;
		int iLastUsed;
		T* pData;
	};

	bool m_bCancelDeinitialize;
	int m_iCurrentID;
	FFreeListTrashArrayNode* m_pArray;
	mutable CRITICAL_SECTION m_cModifySection;

	virtual void growArray();
};



// Public functions...

template <class T>
FFreeListTrashArray<T>::FFreeListTrashArray()
{
	m_iCurrentID = FLTA_MAX_BUCKETS;
	m_pArray = NULL;
	m_bCancelDeinitialize = false;

	InitializeCriticalSectionAndSpinCount(&m_cModifySection, 4000);
}


template <class T>
FFreeListTrashArray<T>::~FFreeListTrashArray()
{
	uninit();

	DeleteCriticalSection(&m_cModifySection);
}


template <class T>
void FFreeListTrashArray<T>::init(int iNumSlots)
{
	int iCount;
	int iI;

	assert(iNumSlots >= 0);

	// make sure it's binary...
	if ((iNumSlots > 0) && ((iNumSlots - 1) & iNumSlots) != 0)
	{
		// find high bit
		iCount = 0;
		while (iNumSlots != 1)
		{
			iNumSlots >>= 1;
			iCount++;
		}
		iNumSlots = (1 << (iCount + 1));
	}

	assert(((iNumSlots - 1) & iNumSlots) == 0);
	assert((m_iNumSlots <= FLTA_MAX_BUCKETS) && "FFreeListTrashArray<T>::init() size too large");

	uninit();

	m_iNumSlots = iNumSlots;
	m_iLastIndex = FFreeList::INVALID_INDEX;
	m_iFreeListHead = FFreeList::INVALID_INDEX;
	m_iFreeListCount = 0;
	m_iCurrentID = FLTA_MAX_BUCKETS;

	if (m_iNumSlots > 0)
	{
		m_pArray = new FFreeListTrashArrayNode[m_iNumSlots];

		for (iI = 0; iI < m_iNumSlots; iI++)
		{
			m_pArray[iI].iNextFreeIndex = FFreeList::INVALID_INDEX;
			m_pArray[iI].iLastUsed = FFreeList::INVALID_INDEX;
			m_pArray[iI].pData = NULL;
		}
	}
}


template <class T>
void FFreeListTrashArray<T>::uninit()
{
	if (m_pArray != NULL)
	{
		if (!m_bCancelDeinitialize)
		{
			removeAll();
		}

		SAFE_DELETE_ARRAY(m_pArray);
	}
}


template <class T>
T* FFreeListTrashArray<T>::add()
{
	int iIndex;
	T* result = NULL;

	EnterCriticalSection(&m_cModifySection);

	if (m_pArray == NULL) 
	{
		init();
	}

	if ((m_iLastIndex == m_iNumSlots - 1) &&
		(m_iFreeListCount == 0))
	{
		if ((m_iNumSlots * FLTA_GROWTH_FACTOR) > FLTA_MAX_BUCKETS)
		{
			LeaveCriticalSection(&m_cModifySection);
			return NULL;
		}

		growArray();
	}

	if (m_iFreeListCount > 0)
	{
		m_iFreeListCount--;

		int iLast = FFreeList::INVALID_INDEX;
		iIndex = m_iFreeListHead;

		//	Ttry to choose a free node that is not from the current major id space so as
		//	to minimize the rate we move through the global id space
		while( iIndex != FFreeList::INVALID_INDEX && m_pArray[iIndex].iLastUsed == m_iCurrentID )
		{
			iLast = iIndex;
			iIndex = m_pArray[iIndex].iNextFreeIndex;
		}

		if ( iIndex == FFreeList::INVALID_INDEX )
		{
			//	Didn't find one - just take the free list head node
			iIndex = m_iFreeListHead;
			m_iFreeListHead = m_pArray[iIndex].iNextFreeIndex;
		}
		else
		{
			//	Foudn one - take it out of the middle of the free list
			if ( iLast == FFreeList::INVALID_INDEX )
			{
				m_iFreeListHead = m_pArray[iIndex].iNextFreeIndex;
			}
			else
			{
				m_pArray[iLast].iNextFreeIndex = m_pArray[iIndex].iNextFreeIndex;
			}
		}
	}
	else
	{
		m_iLastIndex++;
		iIndex = m_iLastIndex;
	}

	MEMORY_TRACK_EXEMPT();

	m_pArray[iIndex].pData = new T;
	m_pArray[iIndex].iNextFreeIndex = FFreeList::INVALID_INDEX;

	if ( m_pArray[iIndex].iLastUsed == m_iCurrentID )
	{
		m_iCurrentID += FLTA_MAX_BUCKETS;
	}
	m_pArray[iIndex].pData->setID(m_iCurrentID + iIndex);
	m_pArray[iIndex].iLastUsed = m_iCurrentID;

	result = m_pArray[iIndex].pData;

	LeaveCriticalSection(&m_cModifySection);

	return result;
}


template <class T>
T* FFreeListTrashArray<T>::getAt(int iID) const
{
	int iIndex;
	T*	result = NULL;

	if ((iID == FFreeList::INVALID_INDEX) || (m_pArray == NULL))
	{
		return NULL;
	}
	
	EnterCriticalSection(&m_cModifySection);

	iIndex = (iID & FLTA_INDEX_MASK);

	assert(iIndex >= 0);

	if ((iIndex <= m_iLastIndex) && 
		(m_pArray[iIndex].pData != NULL))
	{
		if (((iID & FLTA_ID_MASK) == 0) || (m_pArray[iIndex].pData->getID() == iID))
		{
			result = m_pArray[iIndex].pData;
		}
	}
	
	LeaveCriticalSection(&m_cModifySection);

	return result;
}


template <class T>
bool FFreeListTrashArray<T>::remove(T* pData)
{
	int iI;
	bool result = false;

	assert(m_pArray != NULL);

	EnterCriticalSection(&m_cModifySection);

	if (pData != NULL)
	{
		for (iI = 0; iI <= m_iLastIndex; iI++)
		{
			if (m_pArray[iI].pData == pData)
			{
				result = removeAt(iI);
			}
		}
	}

	LeaveCriticalSection(&m_cModifySection);

	return result;
}


template <class T>
bool FFreeListTrashArray<T>::removeAt(int iID)
{
	int iIndex;
	bool result = false;

	if ((iID == FFreeList::INVALID_INDEX) || (m_pArray == NULL))
	{
		return false;
	}

	EnterCriticalSection(&m_cModifySection);

	iIndex = (iID & FLTA_INDEX_MASK);

	assert(iIndex >= 0);

	if ((iIndex <= m_iLastIndex) && 
		(m_pArray[iIndex].pData != NULL))
	{
		if (((iID & FLTA_ID_MASK) == 0) || (m_pArray[iIndex].pData->getID() == iID))
		{
			delete m_pArray[iIndex].pData;
			m_pArray[iIndex].pData = NULL;

			m_pArray[iIndex].iNextFreeIndex = m_iFreeListHead;
			m_iFreeListHead = iIndex;
			m_iFreeListCount++;

			result = true;
		}
		else
		{
			assert(false);
		}
	}

	LeaveCriticalSection(&m_cModifySection);

	return result;
}


template <class T>
void FFreeListTrashArray<T>::removeAll()
{
	int iI;

	if (m_pArray == NULL)
	{
		return;
	}

	EnterCriticalSection(&m_cModifySection);

	m_iLastIndex = FFreeList::INVALID_INDEX;
	m_iFreeListHead = FFreeList::INVALID_INDEX;
	m_iFreeListCount = 0;

	for (iI = 0; iI < m_iNumSlots; iI++)
	{
		m_pArray[iI].iNextFreeIndex = FFreeList::INVALID_INDEX;
		if (m_pArray[iI].pData != NULL)
		{
			delete m_pArray[iI].pData;
		}
		m_pArray[iI].pData = NULL;
	}
	
	LeaveCriticalSection(&m_cModifySection);
}


template <class T>
void FFreeListTrashArray<T>::load(T* pData)
{
	int iIndex;

	assert(pData != NULL);
	//assert((pData->getID() & FLTA_ID_MASK) < m_iCurrentID);
	assert(m_pArray != NULL);

	iIndex = (pData->getID() & FLTA_INDEX_MASK);

	assert(iIndex < FLTA_MAX_BUCKETS);
	assert(iIndex <= m_iLastIndex);
	assert(m_pArray[iIndex].pData == NULL);
	assert(m_pArray[iIndex].iNextFreeIndex == FFreeList::INVALID_INDEX);

	m_pArray[iIndex].pData = pData;
	m_pArray[iIndex].iLastUsed = (pData->getID() & FLTA_ID_MASK);
}

// Protected functions...

template <class T>
void FFreeListTrashArray<T>::growArray()
{
	FFreeListTrashArrayNode* pOldArray;
	int iOldNumSlots;
	int iI;

	MEMORY_TRACK_EXEMPT();

	assert(m_pArray != NULL);

	pOldArray = m_pArray;
	iOldNumSlots = m_iNumSlots;

	m_iNumSlots *= FLTA_GROWTH_FACTOR;
	assert((m_iNumSlots <= FLTA_MAX_BUCKETS) && "FFreeListTrashArray<T>::growArray() size too large");
	m_pArray = new FFreeListTrashArrayNode[m_iNumSlots];

	for (iI = 0; iI < m_iNumSlots; iI++)
	{
		if (iI < iOldNumSlots)
		{
			m_pArray[iI] = pOldArray[iI];
		}
		else
		{
			m_pArray[iI].iNextFreeIndex = FFreeList::INVALID_INDEX;
			m_pArray[iI].iLastUsed = FFreeList::INVALID_INDEX;
			m_pArray[iI].pData = NULL;
		}
	}

	delete [] pOldArray;
}

//
// use when list contains non-streamable types
//
template < class T >
inline void FFreeListTrashArray< T >::Read( FDataStreamBase* pStream )
{
	CvTaggedSaveFormatWrapper&	wrapper = CvTaggedSaveFormatWrapper::getSaveFormatWrapper();

	wrapper.AttachToStream(pStream);

	WRAPPER_READ_OBJECT_START(wrapper);

	int iTemp = 0;
	WRAPPER_READ_DECORATED(wrapper, "FFreeListTrashArray", &iTemp, "numSlots" );
	init( iTemp );
	WRAPPER_READ_DECORATED(wrapper, "FFreeListTrashArray", &iTemp, "lastIndex" );
	setLastIndex( iTemp );
	WRAPPER_READ_DECORATED(wrapper, "FFreeListTrashArray", &iTemp, "freeListHead" );
	setFreeListHead( iTemp );
	WRAPPER_READ_DECORATED(wrapper, "FFreeListTrashArray", &iTemp, "freeListCount" );
	setFreeListCount( iTemp );
	WRAPPER_READ_DECORATED(wrapper, "FFreeListTrashArray", &iTemp, "currentID" );
	setCurrentID( iTemp );

	int i;

	for ( i = 0; i < getNumSlots(); i++ )
	{
		WRAPPER_READ_DECORATED(wrapper, "FFreeListTrashArray", &iTemp, "nextFreeIndex" );
		setNextFreeIndex( i, iTemp );
	}

	int iCount = 0;
	WRAPPER_READ_DECORATED(wrapper, "FFreeListTrashArray", &iCount, "count" );

	for ( i = 0; i < iCount; i++ )
	{
		T* pData = new T;
		WRAPPER_READ_ARRAY_DECORATED(wrapper, "FFreeListTrashArray", sizeof ( T ), ( byte* )pData, "pData" );
		load( pData );
	}

	WRAPPER_READ_OBJECT_END(wrapper);
}

template < class T >
inline void FFreeListTrashArray< T >::Write( FDataStreamBase* pStream )
{
	CvTaggedSaveFormatWrapper&	wrapper = CvTaggedSaveFormatWrapper::getSaveFormatWrapper();

	wrapper.AttachToStream(pStream);

	WRAPPER_WRITE_OBJECT_START(wrapper);

	WRAPPER_WRITE_DECORATED(wrapper, "FFreeListTrashArray",  getNumSlots(), "numSlots" );
	WRAPPER_WRITE_DECORATED(wrapper, "FFreeListTrashArray",  getLastIndex(), "lastIndex" );
	WRAPPER_WRITE_DECORATED(wrapper, "FFreeListTrashArray",  getFreeListHead(), "freeListHead" );
	WRAPPER_WRITE_DECORATED(wrapper, "FFreeListTrashArray",  getFreeListCount(), "freeListCount" );
	WRAPPER_WRITE_DECORATED(wrapper, "FFreeListTrashArray",  getCurrentID(), "currentID" );

	int i;

	for ( i = 0; i < getNumSlots(); i++ )
	{
		WRAPPER_WRITE_DECORATED(wrapper, "FFreeListTrashArray",  getNextFreeIndex( i ), "nextFreeIndex" );
	}

	WRAPPER_WRITE_DECORATED(wrapper, "FFreeListTrashArray",  getCount(), "count" );

	for ( i = 0; i < getIndexAfterLast(); i++ )
	{
		if ( getAt( i ) )
		{
			WRAPPER_WRITE_ARRAY_DECORATED(wrapper, "FFreeListTrashArray",  sizeof ( T ), ( byte* )getAt( i ), "pData" );
		}
	}

	WRAPPER_WRITE_OBJECT_END(wrapper);
}

//-------------------------------
// Serialization helper templates:
//-------------------------------

//
// use when list contains streamable types
//
template < class T >
inline void ReadStreamableFFreeListTrashArray( FFreeListTrashArray< T >& flist, FDataStreamBase* pStream )
{
	CvTaggedSaveFormatWrapper&	wrapper = CvTaggedSaveFormatWrapper::getSaveFormatWrapper();

	wrapper.AttachToStream(pStream);

	WRAPPER_READ_OBJECT_START(wrapper);

	int iTemp = 0;
	WRAPPER_READ_DECORATED(wrapper, "WriteStreamableFFreeListTrashArray", &iTemp, "flistNumSlots" );
	flist.init( iTemp );
	WRAPPER_READ_DECORATED(wrapper, "WriteStreamableFFreeListTrashArray", &iTemp, "flistLastIndex" );
	flist.setLastIndex( iTemp );
	WRAPPER_READ_DECORATED(wrapper, "WriteStreamableFFreeListTrashArray", &iTemp, "flistFreeListHead" );
	flist.setFreeListHead( iTemp );
	WRAPPER_READ_DECORATED(wrapper, "WriteStreamableFFreeListTrashArray", &iTemp, "flistFreeListCount" );
	flist.setFreeListCount( iTemp );
	WRAPPER_READ_DECORATED(wrapper, "WriteStreamableFFreeListTrashArray", &iTemp, "flistCurrentId" );
	flist.setCurrentID( iTemp );
	int i;

	for ( i = 0; i < flist.getNumSlots(); i++ )
	{
		WRAPPER_READ_DECORATED(wrapper, "WriteStreamableFFreeListTrashArray", &iTemp, "flistNextFree" );
		flist.setNextFreeIndex( i, iTemp );
	}

	int iCount = 0;
	WRAPPER_READ_DECORATED(wrapper, "WriteStreamableFFreeListTrashArray", &iCount, "flistCount" );

	for ( i = 0; i < iCount; i++ )
	{
		T* pData = new T;
		pData->read( pStream );
		flist.load( pData );
	}

	WRAPPER_READ_OBJECT_END(wrapper);
}

template < class T >
inline void WriteStreamableFFreeListTrashArray( FFreeListTrashArray< T >& flist, FDataStreamBase* pStream )
{
	CvTaggedSaveFormatWrapper&	wrapper = CvTaggedSaveFormatWrapper::getSaveFormatWrapper();

	wrapper.AttachToStream(pStream);

	WRAPPER_WRITE_OBJECT_START(wrapper);

	WRAPPER_WRITE_DECORATED(wrapper, "WriteStreamableFFreeListTrashArray",  flist.getNumSlots(), "flistNumSlots" );
	WRAPPER_WRITE_DECORATED(wrapper, "WriteStreamableFFreeListTrashArray",  flist.getLastIndex(), "flistLastIndex" );
	WRAPPER_WRITE_DECORATED(wrapper, "WriteStreamableFFreeListTrashArray",  flist.getFreeListHead(), "flistFreeListHead" );
	WRAPPER_WRITE_DECORATED(wrapper, "WriteStreamableFFreeListTrashArray",  flist.getFreeListCount(), "flistFreeListCount" );
	WRAPPER_WRITE_DECORATED(wrapper, "WriteStreamableFFreeListTrashArray",  flist.getCurrentID(), "flistCurrentId" );

	int i;

	for ( i = 0; i < flist.getNumSlots(); i++ )
	{
		WRAPPER_WRITE_DECORATED(wrapper, "WriteStreamableFFreeListTrashArray",  flist.getNextFreeIndex( i ), "flistNextFree" );
	}

	WRAPPER_WRITE_DECORATED(wrapper, "WriteStreamableFFreeListTrashArray",  flist.getCount(), "flistCount" );

	for ( i = 0; i < flist.getIndexAfterLast(); i++ )
	{
		if ( flist[ i ] )
		{
			flist[ i ]->write( pStream );
		}
	}

	WRAPPER_WRITE_OBJECT_END(wrapper);
}

template < class T >
inline void ResyncStreamableFFreeListTrashArray(bool bWrite, ByteBuffer* pBuffer, FFreeListTrashArray< T >& flist)
{
	
	if (bWrite) pBuffer->putInt(flist.getNumSlots());				else flist.init(pBuffer->getInt());
	if (bWrite) pBuffer->putInt(flist.getLastIndex());				else flist.setLastIndex(pBuffer->getInt());
	if (bWrite) pBuffer->putInt(flist.getFreeListHead());			else flist.setFreeListHead(pBuffer->getInt());
	if (bWrite) pBuffer->putInt(flist.getFreeListCount());			else flist.setFreeListCount(pBuffer->getInt());
	if (bWrite) pBuffer->putInt(flist.getCurrentID());				else flist.setCurrentID(pBuffer->getInt());

	for (int i = 0; i < flist.getNumSlots(); i++)
	{
		if (bWrite) pBuffer->putInt(flist.getNextFreeIndex(i));		else flist.setNextFreeIndex(i, pBuffer->getInt());
	}

	if (bWrite)
	{
		pBuffer->putInt(flist.getCount());
		for (int i = 0; i < flist.getIndexAfterLast(); i++)
		{
			if (flist[i])
			{
				flist[i]->resync(bWrite, pBuffer);
			}
		}
	}
	else
	{
		int iCount = pBuffer->getInt();
		for (i = 0; i < iCount; i++)
		{
			T* pData = new T;
			pData->resync(bWrite, pBuffer);
			flist.load(pData);
		}
	}
}


#endif	//FFREELISTTRASHARRAY_H
