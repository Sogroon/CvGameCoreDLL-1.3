
#include "CvGameCoreDLL.h"
#include "FDataStreamBuffer.h"
#include "ByteBuffer.h"

FDataStreamBuffer::FDataStreamBuffer() : m_pByteBuffer(NULL)
{
	m_pByteBuffer = new ByteBuffer();
}

FDataStreamBuffer::~FDataStreamBuffer()
{
	SAFE_DELETE(m_pByteBuffer);
}

void FDataStreamBuffer::Rewind()
{

}

bool FDataStreamBuffer::AtEnd()
{
	return true;
}

void FDataStreamBuffer::FastFwd()
{

}

unsigned int FDataStreamBuffer::GetPosition() const
{
	return 0;
}

void FDataStreamBuffer::SetPosition(unsigned int position)
{

}

void FDataStreamBuffer::Truncate()
{

}

void FDataStreamBuffer::Flush()
{

}

unsigned int FDataStreamBuffer::GetEOF() const
{
	return 0;
}

unsigned int FDataStreamBuffer::GetSizeLeft() const
{
	return 0;
}

void FDataStreamBuffer::CopyToMem(void *mem)
{

}

unsigned int FDataStreamBuffer::WriteString(const wchar *szName)
{
	int iLen = szName == NULL ? 0 : std::wcslen(szName);
	m_pByteBuffer->putInt(iLen);
	for (int i = 0; i < iLen; i++)
	{
		m_pByteBuffer->putChar((char)szName[i]);
	}
	return 0;
}

unsigned int FDataStreamBuffer::WriteString(const char *szName)
{
	int iLen = szName == NULL ? 0 : std::strlen(szName);
	m_pByteBuffer->putInt(iLen);
	for (int i = 0; i < iLen; i++)
	{
		m_pByteBuffer->putChar(szName[i]);
	}
	return 0;
}

unsigned int FDataStreamBuffer::WriteString(const std::string& szName)
{
	return WriteString(szName.c_str());
}

unsigned int FDataStreamBuffer::WriteString(const std::wstring& szName)
{
	return WriteString(szName.c_str());
}

unsigned int FDataStreamBuffer::WriteString(int count, std::string values[])
{
	for (int i = 0; i<count; i++)
	{
		WriteString(values[i]);
	}
	return 0;
}

unsigned int FDataStreamBuffer::WriteString(int count, std::wstring values[])
{
	for (int i = 0; i<count; i++)
	{
		WriteString(values[i]);
	}
	return 0;
}

unsigned int FDataStreamBuffer::ReadString(char *szName)
{
	int iLen = m_pByteBuffer->getInt();
	if (iLen == 0)
	{
		*szName = NULL;
	}
	else
	{
		for (int i = 0; i < iLen; i++)
		{
			char c = m_pByteBuffer->getChar();
			*szName = c;
			szName++;
		}
	}
	return 0;
}
unsigned int FDataStreamBuffer::ReadString(wchar *szName)
{
	int iLen = m_pByteBuffer->getInt();
	if (iLen == 0)
	{
		*szName = NULL;
	}
	else
	{
		for (int i = 0; i < iLen; i++)
		{
			wchar c = m_pByteBuffer->getChar();
			*szName = c;
			szName++;
		}
	}
	return 0;
}
unsigned int FDataStreamBuffer::ReadString(std::string& szName)
{
	szName.clear();
	int iLen = m_pByteBuffer->getInt();
	for (int i = 0; i < iLen; i++)
	{
		char c = m_pByteBuffer->getChar();
		szName += c;
	}
	
	return 0;
}
unsigned int FDataStreamBuffer::ReadString(std::wstring& szName)
{
	szName.clear();
	int iLen = m_pByteBuffer->getInt();
	for (int i = 0; i < iLen; i++)
	{
		wchar c = m_pByteBuffer->getChar();
		szName += c;
	}

	return 0;
}
unsigned int FDataStreamBuffer::ReadString(int count, std::string values[])
{
	for (int i = 0; i < count; i++)
	{
		ReadString(values[i]);
	}
	return 0;
}
unsigned int FDataStreamBuffer::ReadString(int count, std::wstring values[])
{
	for (int i = 0; i<count; i++)
	{
		ReadString(values[i]);
	}
	return 0;
}

char * FDataStreamBuffer::ReadString()		// allocates memory 
{
	CvString s;
	ReadString(s);
	char* szName = new char[s.length() + 1];
	memcpy((void*)szName, (void*)s.c_str(), s.length() + 1);
	return szName;
}

wchar * FDataStreamBuffer::ReadWideString()	// allocates memory 
{
	CvWString s;
	ReadString(s);
	wchar* szName = new wchar[s.length() + 1];
	memcpy((void*)szName, (void*)s.c_str(), (s.length() + 1)*sizeof(wchar));
	return szName;
}

void FDataStreamBuffer::Read(char * pC)
{
	*pC = m_pByteBuffer->getChar();
}

void FDataStreamBuffer::Read(byte *pC)
{
	*pC = m_pByteBuffer->get();
}

void FDataStreamBuffer::Read(int count, char values[])
{
	for (int i = 0; i < count; i++)
	{
		values[i] = m_pByteBuffer->getChar();
	}
}

void FDataStreamBuffer::Read(int count, byte values[])
{
	for (int i = 0; i < count; i++)
	{
		values[i] = m_pByteBuffer->get();
	}
}

void FDataStreamBuffer::Read(bool *b)
{
	*b = (m_pByteBuffer->get() != 0);
}

void FDataStreamBuffer::Read(int count, bool values[])
{
	for (int i = 0; i < count; i++)
	{
		values[i] = (m_pByteBuffer->get() != 0);
	}
}

void FDataStreamBuffer::Read(short	*s)
{
	*s = m_pByteBuffer->getShort();
}

void FDataStreamBuffer::Read(unsigned short	*s)
{
	*s = m_pByteBuffer->getShort();
}

void FDataStreamBuffer::Read(int count, short values[])
{
	for (int i = 0; i < count; i++)
	{
		values[i] = m_pByteBuffer->getShort();
	}
}

void FDataStreamBuffer::Read(int count, unsigned short values[])
{
	for (int i = 0; i < count; i++)
	{
		values[i] = m_pByteBuffer->getShort();
	}
}

void FDataStreamBuffer::Read(int* i)
{
	*i = m_pByteBuffer->getInt();
}

void FDataStreamBuffer::Read(unsigned int* i)
{
	*i = m_pByteBuffer->getInt();
}

void FDataStreamBuffer::Read(int count, int values[])
{
	for (int i = 0; i < count; i++)
	{
		values[i] = m_pByteBuffer->getInt();
	}
}

void FDataStreamBuffer::Read(int count, unsigned int values[])
{
	for (int i = 0; i < count; i++)
	{
		values[i] = m_pByteBuffer->getInt();
	}
}

void FDataStreamBuffer::Read(long* l)
{
	*l = (long)m_pByteBuffer->getLong();
}

void FDataStreamBuffer::Read(unsigned long* l)
{
	*l = (unsigned long)m_pByteBuffer->getLong();
}

void FDataStreamBuffer::Read(int count, long values[])
{
	for (int i = 0; i < count; i++)
	{
		values[i] = (long)m_pByteBuffer->getLong();
	}
}

void FDataStreamBuffer::Read(int count, unsigned long values[])
{
	for (int i = 0; i < count; i++)
	{
		values[i] = (unsigned long)m_pByteBuffer->getLong();
	}
}


void FDataStreamBuffer::Read(float* value)
{
	*value = m_pByteBuffer->getFloat();
}

void FDataStreamBuffer::Read(int count, float values[])
{
	for (int i = 0; i < count; i++)
	{
		values[i] = m_pByteBuffer->getFloat();
	}
}


void FDataStreamBuffer::Read(double* value)
{
	*value = m_pByteBuffer->getDouble();
}

void FDataStreamBuffer::Read(int count, double values[])
{
	for (int i = 0; i < count; i++)
	{
		values[i] = m_pByteBuffer->getDouble();
	}
}


void FDataStreamBuffer::Write(char value)
{
	m_pByteBuffer->putChar(value);
}

void FDataStreamBuffer::Write(byte value)
{
	m_pByteBuffer->put(value);
}

void FDataStreamBuffer::Write(int count, const  char values[])
{
	for (int i = 0; i < count; i++)
	{
		m_pByteBuffer->putChar(values[i]);
	}
}

void FDataStreamBuffer::Write(int count, const  byte values[])
{
	for (int i = 0; i < count; i++)
	{
		m_pByteBuffer->put(values[i]);
	}
}


void FDataStreamBuffer::Write(bool value)
{
	m_pByteBuffer->put(value ? 1 : 0);
}

void FDataStreamBuffer::Write(int count, const bool values[])
{
	for (int i = 0; i < count; i++)
	{
		m_pByteBuffer->put(values[i] ? 1 : 0);
	}
}


void FDataStreamBuffer::Write(short value)
{
	m_pByteBuffer->putShort(value);
}

void FDataStreamBuffer::Write(unsigned short value)
{
	m_pByteBuffer->putShort(value);
}

void FDataStreamBuffer::Write(int count, const short values[])
{
	for (int i = 0; i < count; i++)
	{
		m_pByteBuffer->putShort(values[i]);
	}
}

void FDataStreamBuffer::Write(int count, const unsigned short values[])
{
	for (int i = 0; i < count; i++)
	{
		m_pByteBuffer->putShort(values[i]);
	}
}


void FDataStreamBuffer::Write(int value)
{
	m_pByteBuffer->putInt(value);

}

void FDataStreamBuffer::Write(unsigned int value)
{
	m_pByteBuffer->putInt(value);
}

void FDataStreamBuffer::Write(int count, const int values[])
{
	for (int i = 0; i < count; i++)
	{
		m_pByteBuffer->putInt(values[i]);
	}
}

void FDataStreamBuffer::Write(int count, const unsigned int values[])
{
	for (int i = 0; i < count; i++)
	{
		m_pByteBuffer->putInt(values[i]);
	}
}


void FDataStreamBuffer::Write(long value)
{
	m_pByteBuffer->putLong(value);
}

void FDataStreamBuffer::Write(unsigned long  value)
{
	m_pByteBuffer->putLong(value);
}

void FDataStreamBuffer::Write(int count, const long values[])
{
	for (int i = 0; i < count; i++)
	{
		m_pByteBuffer->putLong(values[i]);
	}
}

void FDataStreamBuffer::Write(int count, const unsigned long values[])
{
	for (int i = 0; i < count; i++)
	{
		m_pByteBuffer->putLong(values[i]);
	}
}

void FDataStreamBuffer::Write(float value)
{
	m_pByteBuffer->putFloat(value);
}

void FDataStreamBuffer::Write(int count, const float values[])
{
	for (int i = 0; i < count; i++)
	{
		m_pByteBuffer->putFloat(values[i]);
	}
}


void FDataStreamBuffer::Write(double value)
{
	m_pByteBuffer->putDouble(value);
}

void FDataStreamBuffer::Write(int count, const double values[])
{
	for (int i = 0; i < count; i++)
	{
		m_pByteBuffer->putDouble(values[i]);
	}
}

