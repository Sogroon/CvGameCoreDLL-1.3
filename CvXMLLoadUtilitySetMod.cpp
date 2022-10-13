//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvXMLLoadUtilitySetMod.h
//
//  AUTHOR:  Vincent Veldman  --  11/2007
//
//  PURPOSE: Group of functions for Vectorized Enumeration and MLF for Civilization 4 BtS
//
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2003 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------
#include "CvGameCoreDLL.h"
#include "CvDLLXMLIFaceBase.h"
#include "CvXMLLoadUtility.h"
#include "CvXMLLoadUtilitySetMod.h"
/************************************************************************************************/
/* Afforess	                  Start		 06/15/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
#include "CvInitCore.h"
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

#include "CvArtFileMgr.h"


#ifdef _DEBUG
#	include "FLinedDataIOStream.h"
#	define CACHESTREAM FLinedDataIOStream
#else
#	include "FDataIOStream.h"
#	define CACHESTREAM FDataIOStream
#endif



// In the following method we set the order of loading properly
void CvXMLLoadUtilitySetMod::setModLoadControlDirArray(bool bSetModControl)
{
	// basic variables
	GC.resetModLoadControlVector();
	if (!bSetModControl)
	{
		GC.setModLoadControlVector("Modules");
		return;
	}
	bool bContinue = true;
	int iDirDepthTemp = 0;		//we don't want to change the global value	

	// To check when bLoad = 1 if the module is valid
	CvXMLLoadUtilityModTools* p_szDirName = new CvXMLLoadUtilityModTools;
/************************************************************************************************/
/* Afforess	                  Start		 06/15/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	CvString szModDirectory;
	szModDirectory = GC.getInitCore().getDLLPath() + "\\";
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	bool bModuleExist = false;	// Valid Module?

	// logging to file
	CvXMLLoadUtility* pXMLLoadUtility = new CvXMLLoadUtility;
	pXMLLoadUtility->logMLF("\n\nThe game will now load the modules into the load vector in the order set by the MLF files:");

	// Initialization of a temp LoadArray 
	bool** aabLoaded = NULL;	// pointer to a pointer	
	aabLoaded = new bool*[GC.getNumModLoadControlInfos()];	//allocate the rows	
   	for ( int iInfosSet = 0; iInfosSet < GC.getNumModLoadControlInfos(); iInfosSet++)
	{
		aabLoaded[iInfosSet] = new bool[GC.getModLoadControlInfos(iInfosSet).getNumModules()];	// allocate the colums
		//loop through the modules of each MLF
		for ( int iSet = 0; iSet < GC.getModLoadControlInfos(iInfosSet).getNumModules(); iSet++ )
		{
			// set the values according to the InfoClass( CvInfos.cpp )
			if ( GC.getModLoadControlInfos(iInfosSet).isLoad(iSet))
			{
				aabLoaded[iInfosSet][iSet] = true;
			}
			else
			{
				aabLoaded[iInfosSet][iSet] = false;
			}
		}
	}

	// more basic variables
	int iInfosLoad;
	int iLoad;
	bool bParentDisable;

	while(bContinue)	//we start with iDirDepth at the highest value(set during the MLF loading)
	{		
		iDirDepthTemp = 0;
		iInfosLoad = 0;
		iLoad = 0;

		//Set the top level MLF to have a start to loop through
		for ( iLoad = 0; iLoad < GC.getModLoadControlInfos(0).getNumModules(); iLoad++ )
		{
			if (aabLoaded[0][iLoad])
			{
				break;  //we found next iLoad
			}
		}

		while(bContinue)
		{
			bContinue = false;
			bool bHitDeepestDepthLoop = false;
			//loop through all the MLF's
			for ( int iiInfos = 0; iiInfos < GC.getNumModLoadControlInfos(); iiInfos++)
			{
				// only loop through files that are actually 1 dir deeper
				if (GC.getModLoadControlInfos(iiInfos).getDirDepth() == iDirDepthTemp + 1)
				{
					// Check if the Loop Module is a Child of the actual Module which we will load if no Children will be found.
					if ( CvString::format("%s\\", GC.getModLoadControlInfos(iInfosLoad).getModuleFolder(iLoad).c_str()) == GC.getModLoadControlInfos(iiInfos).getParentFolder().c_str())
					{
						//loop through the modules of each MLF
						for ( int ii = 0; ii < GC.getModLoadControlInfos(iiInfos).getNumModules(); ii++ )
						{
							//Loaded already? Don't need an endless loop
							if (aabLoaded[iiInfos][ii])
							{
								//set the loading module to the current module
								iInfosLoad = iiInfos;
								iLoad = ii;										

								//The first valid we find on this level will be the first to load, 
								//so we abord further search inside this class, and Continue the loop to the next level
								//To see if this new Module itself has another MLF the next level
								bHitDeepestDepthLoop = true;
								iDirDepthTemp++;

								bContinue = true;                                      
							}
							if (bHitDeepestDepthLoop) break;
						}								
					}							
				}
				if (bHitDeepestDepthLoop) break;
			}
		}

		// means we are loading something which is not the deepest in directory structure that has been found valid to load...
		if ( GC.getModLoadControlInfos(iInfosLoad).getDirDepth() != iDirDepthTemp )
		{
			FAssertMsg(false, "Something ain't right with the parental MLF disabling function, put a bug report on http://www.worldofcivilization.net/bugtracker/bugtracker.htm, please supply your MLF configuration. All of them!");
			break;
		}

		//load the deepest file
		if (aabLoaded[iInfosLoad][iLoad])
		{
			// check for valid xml files			
			if ( isValidModule(bModuleExist, (szModDirectory + GC.getModLoadControlInfos(iInfosLoad).getModuleFolder(iLoad).c_str()).c_str(), GC.getModLoadControlInfos(iInfosLoad).getModuleFolder(iLoad).c_str(), CvString::format(".xml").c_str())/*(int)aszValidFilesVerification.size() > 0*/ )	// does ANY valid xml file exist?
			{
				// if valid, module XML file(s) exist
				// note: if dir isn't valid, of course xml's for that dir aren't valid either
				pXMLLoadUtility->logMLF("Load Priority: %d, \"%s\"", GC.getModLoadControlVectorSize(), GC.getModLoadControlInfos(iInfosLoad).getModuleFolder(iLoad).c_str());
				GC.setModLoadControlVector(GC.getModLoadControlInfos(iInfosLoad).getModuleFolder(iLoad).c_str());
			}
			else
			{
				// if not valid, module XML file(s) doesn't exist
				pXMLLoadUtility->logMLF("No valid module: \"%s\"", GC.getModLoadControlInfos(iInfosLoad).getModuleFolder(iLoad).c_str());
			}
			aabLoaded[iInfosLoad][iLoad] = false;
		}
		else
		{
			pXMLLoadUtility->logMLF("ERROR Vector element: %d, \"%s\", GC.getModLoadControlVectorSize(), You shouldn't have come here!", GC.getModLoadControlInfos(iInfosLoad).getModuleFolder(iLoad).c_str());
			FAssertMsg(aabLoaded[iInfosLoad][iLoad], "Something is wrong with the MLF Array");
		}

		// iInfosLoad is A child of the iInfosParentLoop, we have to check if all the children are loaded now
		// If there are no children to load anymore, we must disable the parent, so it's not looped through on the next run
		int iBreakEndless = 0;	// this is not needed for the code, just a security fix

		// If iDirDepth is 0 we only have 1 MLF anyway..no need to loop and spoil CPU power
		while( iDirDepthTemp > 0 )
		{
			// this code is just a security thing, not important for the actual code..
			iBreakEndless++;
			if ( iBreakEndless >= 20 )	//I assume noone will go 20 depths in dir structure anyway
			{
				FAssertMsg(false, "Something ain't right with the parental MLF disabling function, put a bug report on http://www.worldofcivilization.net/bugtracker/bugtracker.htm, please supply your MLF configuration. All of them!"); 
				break;
			}

			// actual code below
			iDirDepthTemp--;
			// loop through ALL MLF infos...
			for ( int IloopInfosDirDepth = 0; IloopInfosDirDepth < GC.getNumModLoadControlInfos(); IloopInfosDirDepth++)
			{
				// verify all MLF for their dir depth, only 1 higher(dir Depth lower) then the last loaded MLF needs to be checked
				if ( GC.getModLoadControlInfos(IloopInfosDirDepth).getDirDepth() == iDirDepthTemp) 
				{
					bParentDisable = true;  // by default we assume any parent we find must be disabled

					// loop throuhg the modules of every MLF to see if we can find the parent inside this one..
					for ( int IloopInfosModDirectory = 0; IloopInfosModDirectory < GC.getModLoadControlInfos(IloopInfosDirDepth).getNumModules(); IloopInfosModDirectory++)
					{
						// parent module of our latest loaded module?
						if ( CvString::format("%s\\", GC.getModLoadControlInfos(IloopInfosDirDepth).getModuleFolder(IloopInfosModDirectory).c_str()) == GC.getModLoadControlInfos(iInfosLoad).getParentFolder().c_str())
						{							
							//only disable if this is the actual parent

							// Loop through the loaded MLF if everything is now set to false?
							for ( iLoad = 0; iLoad < GC.getModLoadControlInfos(iInfosLoad).getNumModules(); iLoad++)
							{
								// if just ANY dir is still active, don't disable the parent
								if (aabLoaded[iInfosLoad][iLoad])
								{
									bParentDisable = false;
								}
							}
							
							// both conditions are met, we have a Parent, and it has no children left needed to be loaded
							if ( bParentDisable  )	
							{
								aabLoaded[IloopInfosDirDepth][IloopInfosModDirectory] = false;

								// we updated 1 entry, so we have to continue the loop with the new IInfosLoad and iLoad
								iInfosLoad = IloopInfosDirDepth;
								iLoad = IloopInfosModDirectory;
							}
						}
					}
				}
			}

			//break when we just finished checking the top level
			if ( iDirDepthTemp == 0 )
			{
				FAssert(iDirDepthTemp >= 0); //should never reach negative values!!
				break;
			}
		}

		// Check if we must continue
		bContinue = false;		
		//loop through all the MLF's
		for ( int iInfos = 0; iInfos < GC.getNumModLoadControlInfos(); iInfos++)
		{
			//loop through the modules of each MLF
			for ( int iIJ = 0; iIJ < GC.getModLoadControlInfos(iInfos).getNumModules(); iIJ++ )
			{
				// As long as Modules need to be loaded, we continue
				if ( aabLoaded[iInfos][iIJ])
				{
					bContinue = true;
					break;
				}				
			}
			if ( bContinue ) break;
		}
	}
	pXMLLoadUtility->logMLF("Finished the MLF, you will now continue loading regular XML files");		//logging

	SAFE_DELETE_ARRAY(aabLoaded);
	SAFE_DELETE(p_szDirName);
	SAFE_DELETE(pXMLLoadUtility);
}

void CvXMLLoadUtilitySetMod::MLFEnumerateFiles(
					std::vector<CvString>&			aszFiles,
                    const CvString&					refcstrRootDirectory,
					const CvString&					refcstrModularDirectory,
                    const CvString&					refcstrExtension,
                    bool							bSearchSubdirectories)
{
	CvString		strFilePath;		// Filepath
	CvString		strModPath;			// Modules path
	CvString		strPattern;			// Pattern
	CvString		strExtension;		// Extension
	CvString		compareCStrExtension; //Compare with refcstrExtension
	HANDLE          hFile;				// Handle to file
	WIN32_FIND_DATA FileInformation;	// File information

	CvString szDebugBuffer;

	strPattern = refcstrRootDirectory + "\\*.*";

#ifdef _DEBUG
	{
		szDebugBuffer.Format("=== MLFEnumerateFiles begin in root directory %s ===", refcstrRootDirectory.c_str());
		gDLL->logMsg("CvXMLLoadUtilitySetMod_MLFEnumerateFiles.log", szDebugBuffer.c_str());
	}
#endif

	hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(FileInformation.cFileName[0] != '.')
			{
				//strFilePath.erase();
				strFilePath = refcstrRootDirectory + "\\" + FileInformation.cFileName;
				strModPath = refcstrModularDirectory + "\\" + FileInformation.cFileName;
				
				if(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if(bSearchSubdirectories)
					{
#ifdef _DEBUG
						{
							szDebugBuffer.Format(" * Search subdirectory %s", strFilePath.c_str());
							gDLL->logMsg("CvXMLLoadUtilitySetMod_MLFEnumerateFiles.log", szDebugBuffer.c_str());
						}
#endif

						// Search subdirectory
						MLFEnumerateFiles(aszFiles, strFilePath, strModPath, refcstrExtension);
					}
				}
				else
				{
					// Check extension
					strExtension = FileInformation.cFileName;
					//force lowercase for comparison
					int length = strExtension.size();
					for (int i = 0; i < length; ++i)
					{
						strExtension[i] = tolower(strExtension[i]);
					}
					if ( strExtension.rfind("_civ4") != std::string::npos )
					{
						strExtension = strExtension.substr(strExtension.rfind("_civ4") + 1);
					}
/*	 - old buggy rules, just left here for history reminder
					if ( strExtension.rfind("civ4") != std::string::npos )
					{
						strExtension = strExtension.substr(strExtension.rfind("civ4") + 4);
					}
*/
					else if ( strExtension.rfind("globaldefines") != std::string::npos )
					{
						strExtension = strExtension.substr(strExtension.rfind("globaldefines"));
					}
					else if ( strExtension.rfind("pythoncallbackdefines") != std::string::npos )
					{
						strExtension = strExtension.substr(strExtension.rfind("pythoncallbackdefines"));
					}

					//force lowercase for comparison
					compareCStrExtension = refcstrExtension;
					length = compareCStrExtension.size();
					for (int i = 0; i < length; ++i)
					{
						compareCStrExtension[i] = tolower(compareCStrExtension[i]);
					}
					if ( compareCStrExtension.rfind("_civ4") != std::string::npos )
					{
						compareCStrExtension = compareCStrExtension.substr(compareCStrExtension.rfind("_civ4") + 1);
					}
/*	 - old buggy rules, just left here for history reminder
					if ( compareCStrExtension.rfind("civ4") != std::string::npos )
					{
						compareCStrExtension = compareCStrExtension.substr(compareCStrExtension.rfind("civ4") + 4);
					}
*/
					
					if (!strcmp(strExtension.c_str(),compareCStrExtension.c_str()))
					{
						aszFiles.push_back(strModPath.c_str());
					}
				}
			}
		} while(::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);

		//FAssertMsg(::GetLastError() != ERROR_NO_MORE_FILES, "something wrong");
		//DWORD dwError = ::GetLastError();
		//if(dwError != ERROR_NO_MORE_FILES)
		//{
		//	FAssertMsg(false, "something wrong");
		//	return;
		//}
	}
	return;
}

bool CvXMLLoadUtilitySetMod::isValidModule(
					bool&							bValid,
                    const CvString&					refcstrRootDirectory,
					const CvString&					refcstrModularDirectory,
                    const CvString&					refcstrExtension,					
                    bool							bSearchSubdirectories)
{
	CvString		strFilePath;		// Filepath
	CvString		strModPath;			// Modules path
	CvString		strPattern;			// Pattern
	CvString		strExtension;		// Extension
	CvString		compareCStrExtension; //Compare with refcstrExtension
	HANDLE          hFile;				// Handle to file
	WIN32_FIND_DATA FileInformation;	// File information

	strPattern = refcstrRootDirectory + "\\*.*";

	//Afforess Addition : Check that we want to load this module first
	for ( int iInfosSet = 0; iInfosSet < GC.getNumModLoadControlInfos(); iInfosSet++)
	{
		CvString szType = GC.getModLoadControlInfos(iInfosSet).getParentFolder();
		if (CvString::format("%s\\", refcstrModularDirectory.c_str()) == szType)
		{
			bool bLoadAny = false;
			for ( int iSet = 0; iSet < GC.getModLoadControlInfos(iInfosSet).getNumModules(); iSet++ )
			{
				if (GC.getModLoadControlInfos(iInfosSet).isLoad(iSet))
				{
					bLoadAny = true;
					break;
				}
			}
			if (!bLoadAny)
			{
				return false;
			}
		}
	}

	hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = refcstrRootDirectory + "\\" + FileInformation.cFileName;
				strModPath = refcstrModularDirectory + "\\" + FileInformation.cFileName;
				
				if(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if(bSearchSubdirectories)
					{
						// Search subdirectory
						bValid = isValidModule(bValid, strFilePath, strModPath, refcstrExtension);
						if (bValid) break;
					}
				}
				else
				{
					// Check extension
					strExtension = FileInformation.cFileName;
					//force lowercase for comparison
					int length = strExtension.size();
					for (int i = 0; i < length; ++i)
					{
						strExtension[i] = tolower(strExtension[i]);
					}
					//is xml file?
					if ( strExtension.rfind(".xml") != std::string::npos )
					{
						bValid = true;
						break;
					}					
				}
			}
		} while(::FindNextFile(hFile, &FileInformation) == TRUE);		

		// Close handle
		::FindClose(hFile);

		DWORD dwError = ::GetLastError();
		if(!bValid && dwError != ERROR_NO_MORE_FILES)
		{
			FAssertMsg(false, "something wrong");
			return false;
		}

		if ( bValid )
		{
			return true;
		}
	}
	return false;
}

void CvXMLLoadUtilitySetMod::loadModControlArray(std::vector<CvString>& aszFiles, const char* szFileRoot)
{	
	CvString szModDirectory;


/************************************************************************************************/
/* Afforess	                  Start		 06/15/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	/*
	CvXMLLoadUtilityModTools* p_szDirName = new CvXMLLoadUtilityModTools;
	szModDirectory = p_szDirName->GetProgramDir();		// Dir where the Civ4BeyondSword.exe is started from
	SAFE_DELETE(p_szDirName);
	szModDirectory += gDLL->getModName();		// "Mods\Modname\"
	szModDirectory += "Assets\\";		//Assets in the Moddirectory
	 */

	szModDirectory = GC.getInitCore().getDLLPath() + "\\";
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	


	for (int iI = 0; iI < GC.getModLoadControlVectorSize(); iI++)
	{
		MLFEnumerateFiles(aszFiles, (szModDirectory + GC.getModLoadControlVector(iI).c_str()).c_str(), GC.getModLoadControlVector(iI).c_str(), CvString::format("%s.xml", szFileRoot).c_str());
	}
}


void CvXMLLoadUtilitySetMod::checkCacheStateDir(
                    const CvString&					refcstrRootDirectory,
					CvChecksum&						ulCheckSum,
                    bool							bSearchSubdirectories)
{
	CvString		strFilePath;		// Filepath
	CvString		strPattern;			// Pattern
	CvString		strExtension;		// Extension
	HANDLE          hFile;				// Handle to file
	WIN32_FIND_DATA FileInformation;	// File information

	strPattern = refcstrRootDirectory + "\\*.*";

	hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(FileInformation.cFileName[0] != '.')
			{
				//strFilePath.erase();
				strFilePath = refcstrRootDirectory + "\\" + FileInformation.cFileName;
				
				if(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if(bSearchSubdirectories)
					{
						// Search subdirectory
						checkCacheStateDir(strFilePath, ulCheckSum);
					}
				}
				else
				{
					// Check extension
					strExtension = FileInformation.cFileName;
					//force lowercase for comparison
					int length = strExtension.size();
					for (int i = 0; i < length; ++i)
					{
						strExtension[i] = tolower(strExtension[i]);
					}
					//	Include any files ending '.xml' - this is to accoutn for things like GlobalDefines.xml
					//	which do not inlude the Civ4 substring.  This one in particular has to be included
					//	as it influences the allocated sizes of various arrays into which parsed data from other
					//	assets is placed
					//  AIAndy: This change also included the audio XML which is written regularly. That one needs to be excluded if the cache is supposed to have any value.
					if ( strExtension.rfind("civ4") != std::string::npos ||
						 (strExtension.length() > 4 && strExtension.rfind(".xml") == strExtension.length()-4 && strExtension.rfind("audio") == std::string::npos) )
					{
						ulCheckSum.add((int)FileInformation.ftLastWriteTime.dwHighDateTime);
						ulCheckSum.add((int)FileInformation.ftLastWriteTime.dwLowDateTime);
					}
				}
			}
		} while(::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);
	}
	return;
}

// Check the timestamp of all XML files and compare to the cache checksum
void CvXMLLoadUtilitySetMod::checkCacheState()
{
	CvChecksum ulCheckSum;

	CvString szModDirectory = GC.getInitCore().getDLLPath() + "\\";
	CvString szCacheFile = szModDirectory + "Cache\\Cache.dat";
	CvString szXMLBaseDir = szModDirectory + "XML";

	// Include mod base directories
 	checkCacheStateDir(szXMLBaseDir, ulCheckSum);

	// Check modular directories
	for (int iI = 0; iI < GC.getModLoadControlVectorSize(); iI++)
	{
		checkCacheStateDir((szModDirectory + GC.getModLoadControlVector(iI).c_str()).c_str(), ulCheckSum);
	}

	// Check if there is a cache file and read the checksum if there is
	CACHESTREAM cacheStream;
	cacheStream.OpenRead(szCacheFile);
	unsigned long ulCacheCheckSum = 0;
	if (cacheStream.IsOpen())
	{
		cacheStream.Read(&ulCacheCheckSum);
		cacheStream.Close();
	}

	if (ulCacheCheckSum == (unsigned long)ulCheckSum.get() &&
		readCacheHeader())
	{
		GC.setWriteCache(false);
		gDLL->logMsg("xml.log","Read XML from cache files");
	}
	else
	{
		GC.setWriteCache(true);
		gDLL->logMsg("xml.log","Read XML from XML files");

		// Create directory if not there
		CreateDirectory((szModDirectory + "Cache").c_str(), NULL);
		CACHESTREAM cacheOutStream;
		cacheOutStream.OpenWrite(szCacheFile);
		if (cacheOutStream.IsOpen())
		{
			cacheOutStream.Write((unsigned long)ulCheckSum.get());
			cacheOutStream.Close();
		}
	}
}

// Template function to write the cache of a single info class
template <class T>
void writeCache(std::vector<T*>& aInfos, const char* szFileRoot)
{
	// write info to cache
	CvString szModDirectory = GC.getInitCore().getDLLPath() + "\\";

	CACHESTREAM cacheStream;
	CvString szCacheFile = szModDirectory + "Cache\\" + szFileRoot + ".dat";;
	cacheStream.OpenWrite(szCacheFile);
	if (cacheStream.IsOpen())
	{
		// Write stream
		int iNum = (int)aInfos.size();
		cacheStream.Write(iNum);
		for (int i=0; i<iNum; i++)
		{
			CvInfoBase* pInfo = (CvInfoBase*)(aInfos[i]); // The compiler does not look into the base class by itself so the info classes without stream write cause a compile error otherwise
			pInfo->write(&cacheStream);
		}

		cacheStream.Close();
		//logMsg("Wrote %s to cache", szFileRoot);
	}
}

// Template function to write the cache of a single info replacement class
template <class T>
void writeReplacementCache(CvInfoReplacements<T>* pReplacements, const char* szFileRoot)
{
	// write info to cache
	CvString szModDirectory = GC.getInitCore().getDLLPath() + "\\";

	CACHESTREAM cacheStream;
	CvString szCacheFile = szModDirectory + "Cache\\" + szFileRoot + "Repl.dat";;
	cacheStream.OpenWrite(szCacheFile);
	if (cacheStream.IsOpen())
	{
		// Write stream
		pReplacements->write(&cacheStream);

		cacheStream.Close();
		//logMsg("Wrote %s to cache", szFileRoot);
	}
}

// Write the caches to disk
void CvXMLLoadUtilitySetMod::writeCaches()
{
	writeCacheHeader();

	writeCache(GC.getTechInfo(), "CIV4TechInfos");
	writeReplacementCache(GC.getTechInfoReplacements(), "CIV4TechInfos");
	writeCache(GC.getPromotionInfo(), "CIV4PromotionInfos");
	writeCache(GC.getHandicapInfo(), "CIV4HandicapInfo");
	writeCache(ARTFILEMGR.getBonusArtInfo(), "CIV4ArtDefines_Bonus");
	writeCache(GC.getBonusInfo(), "CIV4BonusInfos");
	writeCache(GC.getImprovementInfo(), "CIV4ImprovementInfos");
	writeCache(ARTFILEMGR.getMovieArtInfo(), "CIV4ArtDefines_Movie");
	writeCache(ARTFILEMGR.getBuildingArtInfo(), "CIV4ArtDefines_Building");
	writeCache(GC.getBuildingClassInfo(), "CIV4BuildingClassInfos");
	writeCache(GC.getBuildingInfo(), "CIV4BuildingInfos");
	writeCache(GC.getCivicInfo(), "CIV4CivicInfos");
	writeCache(ARTFILEMGR.getLeaderheadArtInfo(), "CIV4ArtDefines_Leaderhead");
	writeCache(GC.getLeaderHeadInfo(), "CIV4LeaderHeadInfos");
	writeCache(ARTFILEMGR.getUnitArtInfo(), "CIV4ArtDefines_Unit");
	writeCache(GC.getUnitClassInfo(), "CIV4UnitClassInfos");
	writeCache(GC.getUnitInfo(), "CIV4UnitInfos");
	writeReplacementCache(GC.getUnitInfoReplacements(), "CIV4UnitInfos");
	writeCache(ARTFILEMGR.getCivilizationArtInfo(), "CIV4ArtDefines_Civilization");
	writeCache(GC.getCivilizationInfo(), "CIV4CivilizationInfos");
	writeCache(GC.getEventInfo(), "CIV4EventInfos");
	writeCache(GC.getEventTriggerInfo(), "CIV4EventTriggerInfos");
	writeCache(GC.getColorInfo(), "CIV4ColorVals");
	writeCache(GC.getPlayerColorInfo(), "CIV4PlayerColorInfos");
}

#define	CACHE_VERSION	11	//	Increment this if changing caching code to alter format - this incldues adding new tags
							//	to any info class that has cache reads/write methods

void CvXMLLoadUtilitySetMod::writeCacheHeader(void)
{
	// write info to cache
	CvString szModDirectory = GC.getInitCore().getDLLPath() + "\\";

	CACHESTREAM cacheStream;
	CvString szCacheFile = szModDirectory + "Cache\\_CacheHeader.dat";
	cacheStream.OpenWrite(szCacheFile);
	if (cacheStream.IsOpen())
	{
		int iNum = CACHE_VERSION;
		cacheStream.Write(iNum);

		cacheStream.Close();
	}
}

bool CvXMLLoadUtilitySetMod::readCacheHeader(void)
{
	// write info to cache
	CvString szModDirectory = GC.getInitCore().getDLLPath() + "\\";

	CACHESTREAM cacheStream;
	CvString szCacheFile = szModDirectory + "Cache\\_CacheHeader.dat";
	cacheStream.OpenRead(szCacheFile);
	if (cacheStream.IsOpen())
	{
		int iNum = 0;
		cacheStream.Read(&iNum);

		if ( CACHE_VERSION != iNum )
		{
			return false;
		}

		cacheStream.Close();

		return true;
	}
	else
	{
		return false;
	}
}
