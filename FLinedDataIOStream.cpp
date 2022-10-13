
//	AIAndy: An implementation of FDataStreamBase for manual caching, based on fstream

#include "CvGameCoreDLL.h"
#include "FLinedDataIOStream.h"

void FLinedDataIOStream::OpenRead(CvString szFile)
{
	stream.open(szFile.c_str(),std::ios::binary | std::ios::in);
}

void FLinedDataIOStream::OpenWrite(CvString szFile)
{
	stream.open(szFile.c_str(),std::ios::binary | std::ios::out | std::ios::trunc);
	FAssert(stream.good());
}

void FLinedDataIOStream::Close()
{
	stream.close();
}

bool FLinedDataIOStream::IsOpen()
{
	return stream.is_open();
}

void FLinedDataIOStream::Rewind()
{

}

bool FLinedDataIOStream::AtEnd()
{
	return true;
}

void FLinedDataIOStream::FastFwd()
{

}

unsigned int FLinedDataIOStream::GetPosition() const
{
	return 0;
}

void FLinedDataIOStream::SetPosition(unsigned int position)
{

}

void FLinedDataIOStream::Truncate()
{

}

void FLinedDataIOStream::Flush()
{

}

unsigned int FLinedDataIOStream::GetEOF() const
{
	return 0;
}

unsigned int FLinedDataIOStream::GetSizeLeft() const
{
	return 0;
}

void FLinedDataIOStream::CopyToMem(void *mem)
{

}

unsigned int FLinedDataIOStream::WriteString(const wchar *szName)
{
	int len = wcstombs(NULL, szName, 0xffffff);
	char* tmp = new char[len+1];
	wcstombs(tmp, szName, 0xfffffe);
	WriteString(tmp);
	delete[] tmp; // f1rpo: was non-array delete
	return 0;
}

unsigned int FLinedDataIOStream::WriteString(const char *szName)
{
	stream.write(szName, std::strlen(szName) + 1);
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
	return 0;
}

unsigned int FLinedDataIOStream::WriteString(const std::string& szName)
{
	stream.write(szName.c_str(), szName.length() + 1);
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
	return 0;
}

unsigned int FLinedDataIOStream::WriteString(const std::wstring& szName)
{
	WriteString(szName.c_str());
	return 0;
}

unsigned int FLinedDataIOStream::WriteString(int count, std::string values[])
{
	for (int i=0; i<count; i++)
	{
		WriteString(values[i]);
	}
	return 0;
}

unsigned int FLinedDataIOStream::WriteString(int count, std::wstring values[])
{
	for (int i=0; i<count; i++)
	{
		WriteString(values[i]);
	}
	return 0;
}
	
unsigned int FLinedDataIOStream::ReadString(char *szName)
{
	char c;
	do
	{
		stream.get(c);
		*szName = c;
		szName++;
	}
	while (c != '\0');
	FAssert(stream.good());
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
	return 0;
}
unsigned int FLinedDataIOStream::ReadString(wchar *szName)
{
	std::string tmp;
	ReadString(tmp);
	mbstowcs(szName, tmp.c_str(), 0xffffd);
	return 0;
}
unsigned int FLinedDataIOStream::ReadString(std::string& szName)
{
	char c;
	stream.get(c);
	szName.clear();

	while (c != '\0')
	{
		szName.push_back(c);
		stream.get(c);
	}
	FAssert(stream.good());
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
	return 0;
}
unsigned int FLinedDataIOStream::ReadString(std::wstring& szName)
{
	std::string tmp;
	ReadString(tmp);
	int len = mbstowcs(NULL, tmp.c_str(), 0xffffff);
	wchar_t* tmp2 = new wchar_t[len+1];
	mbstowcs(tmp2, tmp.c_str(), 0xfffffe);
	szName = tmp2;
	delete tmp2;
	return 0;
}
unsigned int FLinedDataIOStream::ReadString(int count, std::string values[])
{
	for (int i=0; i<count; i++)
	{
		ReadString(values[i]);
	}
	return 0;
}
unsigned int FLinedDataIOStream::ReadString(int count, std::wstring values[])
{
	for (int i=0; i<count; i++)
	{
		ReadString(values[i]);
	}
	return 0;
}
	
char * FLinedDataIOStream::ReadString()		// allocates memory 
{
	CvString s;
	ReadString(s);
	char* szName = new char[s.length()+1];
	memcpy((void*)szName, (void*)s.c_str(), s.length()+1);
	return szName;
}

wchar * FLinedDataIOStream::ReadWideString()	// allocates memory 
{
	CvWString s;
	ReadString(s);
	wchar* szName = new wchar[s.length()+1];
	memcpy((void*)szName, (void*)s.c_str(), (s.length()+1)*sizeof(wchar));
	return szName;
}

void FLinedDataIOStream::Read(char * pC)
{
	stream.get(*pC);
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}

void FLinedDataIOStream::Read(byte *pC)
{
	stream.get(*((char*)pC));
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}

void FLinedDataIOStream::Read(int count, char values[])
{
	stream.read(values, count);
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}

void FLinedDataIOStream::Read(int count, byte values[])
{
	stream.read((char*)values, count);
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}

void FLinedDataIOStream::Read(bool *b)
{
	stream.read((char*)b, sizeof(bool));
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}

void FLinedDataIOStream::Read(int count, bool values[])
{
	stream.read((char*)values, sizeof(bool)*count);
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}

void FLinedDataIOStream::Read(short	*s)
{
	stream.read((char*)s, sizeof(short));
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}

void FLinedDataIOStream::Read(unsigned short	*s)
{
	stream.read((char*)s, sizeof(unsigned short));
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}

void FLinedDataIOStream::Read(int count, short values[])
{
	stream.read((char*)values, sizeof(short)*count);
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}

void FLinedDataIOStream::Read(int count, unsigned short values[])
{
	stream.read((char*)values, sizeof(unsigned short)*count);
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}

void FLinedDataIOStream::Read(int* i)
{
	stream.read((char*)i, sizeof(int));
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}

void FLinedDataIOStream::Read(unsigned int* i)
{
	stream.read((char*)i, sizeof(unsigned int));
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}

void FLinedDataIOStream::Read(int count, int values[])
{
	stream.read((char*)values, sizeof(int)*count);
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}

void FLinedDataIOStream::Read(int count, unsigned int values[])
{
	stream.read((char*)values, sizeof(unsigned int)*count);
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}


void FLinedDataIOStream::Read(long* l)
{
	stream.read((char*)l, sizeof(long));
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}

void FLinedDataIOStream::Read(unsigned long* l)
{
	stream.read((char*)l, sizeof(unsigned long));
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}

void FLinedDataIOStream::Read(int count, long values[])
{
	stream.read((char*)values, sizeof(long)*count);
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}

void FLinedDataIOStream::Read(int count, unsigned long values[])
{
	stream.read((char*)values, sizeof(unsigned long)*count);
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}


void FLinedDataIOStream::Read(float* value)
{
	stream.read((char*)value, sizeof(float));
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}

void FLinedDataIOStream::Read(int count, float values[])
{
	stream.read((char*)values, sizeof(float)*count);
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}


void FLinedDataIOStream::Read(double* value)
{
	stream.read((char*)value, sizeof(double));
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}

void FLinedDataIOStream::Read(int count, double values[])
{
	stream.read((char*)values, sizeof(double)*count);
	FAssert(stream.good());
	char c;
	stream.get(c);
	FAssert(stream.good());
	FAssert(c == '\n');
}


void FLinedDataIOStream::Write( char value)
{
	stream.put(value);
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}

void FLinedDataIOStream::Write(byte value)
{
	stream.put((char)value);
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}

void FLinedDataIOStream::Write(int count, const  char values[])
{
	stream.write(values, count);
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}

void FLinedDataIOStream::Write(int count, const  byte values[])
{
	stream.write((char*)values, count);
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}


void FLinedDataIOStream::Write(bool value)
{
	stream.write((char*)&value, sizeof(bool));
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}

void FLinedDataIOStream::Write(int count, const bool values[])
{
	stream.write((char*)values, sizeof(bool)*count);
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}


void FLinedDataIOStream::Write(short value)
{
	stream.write((char*)&value, sizeof(short));
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}

void FLinedDataIOStream::Write(unsigned short value)
{
	stream.write((char*)&value, sizeof(unsigned short));
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}

void FLinedDataIOStream::Write(int count, const short values[])
{
	stream.write((char*)values, sizeof(short)*count);
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}

void FLinedDataIOStream::Write(int count, const unsigned short values[])
{
	stream.write((char*)values, sizeof(unsigned short)*count);
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}


void FLinedDataIOStream::Write(int value)
{
	stream.write((char*)&value, sizeof(int));
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}

void FLinedDataIOStream::Write(unsigned int value)
{
	stream.write((char*)&value, sizeof(unsigned int));
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}

void FLinedDataIOStream::Write(int count, const int values[])
{
	stream.write((char*)values, sizeof(int)*count);
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}

void FLinedDataIOStream::Write(int count, const unsigned int values[])
{
	stream.write((char*)values, sizeof(unsigned int)*count);
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}


void FLinedDataIOStream::Write(long value)
{
	stream.write((char*)&value, sizeof(long));
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}

void FLinedDataIOStream::Write(unsigned long  value)
{
	stream.write((char*)&value, sizeof(unsigned long));
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}

void FLinedDataIOStream::Write(int count, const long values[])
{
	stream.write((char*)values, sizeof(long)*count);
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}

void FLinedDataIOStream::Write(int count, const unsigned long values[])
{
	stream.write((char*)values, sizeof(unsigned long)*count);
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}


void FLinedDataIOStream::Write(float value)
{
	stream.write((char*)&value, sizeof(float));
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}

void FLinedDataIOStream::Write(int count, const float values[])
{
	stream.write((char*)values, sizeof(float)*count);
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}


void FLinedDataIOStream::Write(double value)
{
	stream.write((char*)&value, sizeof(double));
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}

void FLinedDataIOStream::Write(int count, const double values[])
{
	stream.write((char*)values, sizeof(double)*count);
	FAssert(stream.good());
	stream.put('\n');
	FAssert(stream.good());
}



