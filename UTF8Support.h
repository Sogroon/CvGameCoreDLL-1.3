#ifndef FONT_CONVERSION_H
#define FONT_CONVERSION_H
#pragma once

void convertString(CvString szTextVal, CvWString& wszNewString);
CvWString convertWString(CvWString wszTextVal);
void setConversionTableUnicode();
CvWString getGameFontString(CvWString szString);
void logMessage(char* format, ...);

//Unicode test


/*void setupGameFontIDs();

class CvGameFontInfo : public CvInfoBase
{
public:
	bool read(CvXMLLoadUtility* pXML);
};*/

#endif
