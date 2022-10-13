#include "CvGameCoreDLL.h"
#include "CvDLLXMLRapid.h"
#include "rapidxml/rapidxml.hpp"
#include <fstream>

enum EncodingTypes
{
	ENC_UTF8,
	ENC_LATIN1
};

// the class that contains the actual XML text and parse information
class FXml
{
public:
	std::vector<char> szText;
	rapidxml::xml_document<> doc;
	rapidxml::xml_node<>* node;
	EncodingTypes encoding;
};

//
// a partial implementation of the CvDLLXMLIFaceBase abstract interface using RapidXML
//

FXml* CvDLLXMLRapid::CreateFXml(FXmlSchemaCache* pSchemaCache)
{
	return new FXml();
}

void CvDLLXMLRapid::DestroyFXml(FXml*& xml)
{
	delete xml;
}

void CvDLLXMLRapid::DestroyFXmlSchemaCache(FXmlSchemaCache*&)
{
}

FXmlSchemaCache* CvDLLXMLRapid::CreateFXmlSchemaCache()
{
	return NULL;
}

// load an xml file into memory, returns true if successfully loaded and false otherwise
bool CvDLLXMLRapid::LoadXml(FXml* xml, const TCHAR* pszXmlFile)
{
	std::ifstream stream;

	std::string szDir = gDLL->getModName();
	//szDir.append("\\");
	szDir.append(pszXmlFile);
	stream.open(szDir.c_str(), std::ios::binary | std::ios::in);
	if (!stream.is_open())
	{
		// could not open file, try from current dir
		stream.open(pszXmlFile, std::ios::binary | std::ios::in);
		if (!stream.is_open())
		{
			// could not open file, try Warlords dir
			szDir = "..\\Warlords\\";
			szDir.append(pszXmlFile);
			stream.open(szDir.c_str(), std::ios::binary | std::ios::in);
			if (!stream.is_open())
			{
				// could not open file, try from one further up
				szDir = "..\\";
				szDir.append(pszXmlFile);
				stream.open(szDir.c_str(), std::ios::binary | std::ios::in);
				if (!stream.is_open())
				{
					return false;
				}
			}
		}
	}
	xml->szText.clear();
	xml->szText = std::vector<char>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
	stream.close();
	xml->szText.push_back('\0');
	try
	{
		xml->doc.parse<rapidxml::parse_declaration_node | rapidxml::parse_no_data_nodes>(&(xml->szText[0]));
	}
	catch (rapidxml::parse_error e)
	{
		const char* cwhat = e.what();
		const char* cwhere = e.where<char>();
		return false;
	}
	xml->node = xml->doc.first_node();
	rapidxml::xml_attribute<>* attr = xml->node->first_attribute("encoding");
	if (attr != NULL)
	{
		std::string encoding = attr->value();
		std::transform(encoding.begin(), encoding.end(), encoding.begin(), ::tolower);
		if (encoding == "utf-8")
		{
			xml->encoding = ENC_UTF8;
		}
		else
		{
			xml->encoding = ENC_LATIN1;
		}
	}
	else
	{
		// default to UTF-8
		xml->encoding = ENC_LATIN1;
	}

	// Civ4 expects to be on the document node at the beginning
	xml->node = &(xml->doc);
	
	return true;
}

// validate the document/xml, returns true if successful and false otherwise,
//	if validate fails we also return an error string to indicate why it failed
bool CvDLLXMLRapid::Validate(FXml* xml, TCHAR* pszError)
{
	// no validation with this parser
	return true;
}

// locate a node in the document, returns true if a node is found and false otherwise
bool CvDLLXMLRapid::LocateNode(FXml* xml, const TCHAR* pszXmlNode)
{
	// split pszXmlNode at / or \ and apply multiple first_node then
	rapidxml::xml_node<>* cur_node = &(xml->doc);
	char* buf = new char[strlen(pszXmlNode)+1];
	strcpy(buf, pszXmlNode);
	for (char* tok = strtok(buf, "/\\"); tok != NULL; tok = strtok(NULL, "/\\"))
	{
		cur_node =cur_node->first_node(tok);
		if (cur_node == NULL)
		{
			delete[] buf;
			return false;
		}
	}
	delete[] buf;
	//if (cur_node->first_node() != NULL)
	//{
		xml->node = cur_node;
	//}
	//else
	//{
	//	xml->node = cur_node->parent();
	//}
	return true;
}

// locate the first sibling node in this sub-tree layer that has the xml tag name,
// returns true if a node is found and false otherwise
bool CvDLLXMLRapid::LocateFirstSiblingNodeByTagName(FXml* xml, TCHAR* pszTagName)
{
	rapidxml::xml_node<>* cur_node = xml->node->parent();
	if (cur_node != NULL)
	{
		cur_node = cur_node->first_node(pszTagName);
		if (cur_node != NULL)
		{
			xml->node = cur_node;
			return true;
		}
	}
	return false;
}

// locate the next sibling node in this sub-tree layer that has the xml tag name,
// returns true if a node is found and false otherwise
bool CvDLLXMLRapid::LocateNextSiblingNodeByTagName(FXml* xml, TCHAR* pszTagName)
{
	rapidxml::xml_node<>* cur_node = xml->node->next_sibling(pszTagName);
	if (cur_node != NULL)
	{
		xml->node = cur_node;
		return true;
	}
	return false;
}

// Set the last located xml node member variable to the next sibling of the current
//	last located xml node, returns true if a next sibling was found and false otherwise
bool CvDLLXMLRapid::NextSibling(FXml* xml)
{
	return LocateNextSiblingNodeByTagName(xml, NULL);
}

// Set the last located xml node member variable to the prev sibling of the current
//	last located xml node, returns true if a next sibling was found and false otherwise
bool CvDLLXMLRapid::PrevSibling(FXml* xml)
{
	rapidxml::xml_node<>* cur_node = xml->node->previous_sibling();
	if (cur_node != NULL)
	{
		xml->node = cur_node;
		return true;
	}
	return false;
}

// Set the last located xml node member variable to the it's first child,
//	returns true if a next child was found and set, false otherwise
bool CvDLLXMLRapid::SetToChild(FXml* xml)
{
	return SetToChildByTagName(xml, NULL);
}

// set to the child node that has the tag name
bool CvDLLXMLRapid::SetToChildByTagName(FXml* xml, const TCHAR* szTagName)
{
	rapidxml::xml_node<>* cur_node = xml->node->first_node(szTagName);
	if (cur_node != NULL)
	{
		xml->node = cur_node;
		return true;
	}
	return false;
}

// Set the last located xml node member variable to the it's parent,
//	returns true if it's parent is found and set, false otherwise
bool CvDLLXMLRapid::SetToParent(FXml* xml)
{
	rapidxml::xml_node<>* cur_node = xml->node->parent();
	if (cur_node != NULL)
	{
		xml->node = cur_node;
		return true;
	}
	return false;
}

// add a node to the document as the child of the last located node,
// returns true if the node was successfully added and false otherwise
bool CvDLLXMLRapid::AddChildNode(FXml* xml, TCHAR* pszNewNode)
{
	// not implemented
	return false;
}

// adds a node to the document as a sibling to the last node located,
// node is added before the last node located,
// returns true if the node was successfully added and false otherwise
bool CvDLLXMLRapid::AddSiblingNodeBefore(FXml* xml, TCHAR* pszNewNode)
{
	// not implemented
	return false;
}

// adds a node to the document as a sibling to the last node located,
// node is added after the last node located,
// returns true if the node was successfully added and false otherwise
bool CvDLLXMLRapid::AddSiblingNodeAfter(FXml* xml, TCHAR* pszNewNode)
{
	// not implemented
	return false;
}

// write the xml document to disk, returns true if the write was successful and false otherwise
bool CvDLLXMLRapid::WriteXml(FXml* xml, TCHAR* pszXmlFile)
{
	// not implemented
	return false;
}

// set the inserted node's attribute, returns true if successful and false otherwise
bool CvDLLXMLRapid::SetInsertedNodeAttribute(FXml* xml, TCHAR* pszAttributeName, TCHAR* pszAttributeValue)
{
	// not implemented
	return false;
}

// get the size of the text from the last located node, returns the size
int CvDLLXMLRapid::GetLastNodeTextSize(FXml* xml)
{
	return xml->node->value_size();
}

// get the text from the last located node, returns true if successful and false otherwise
bool CvDLLXMLRapid::GetLastNodeText(FXml* xml, TCHAR* pszText)
{
	strcpy(pszText, xml->node->value());
	return true;
}

// get the value from the last located node, returns true if successful and false otherwise
bool CvDLLXMLRapid::GetLastNodeValue(FXml* xml, std::string& pszText)
{
	// this will return the encoded string
	pszText = xml->node->value();
	return true;
}

bool CvDLLXMLRapid::GetLastNodeValue(FXml* xml, std::wstring& pszText)
{
	// this will return the decoded string
	// if we have latin1 we first convert it to UTF-8
	unsigned char* in = (unsigned char*)xml->node->value();
	int bufsize = 2*strlen(xml->node->value())+2;
	unsigned char *utf8buf = new unsigned char[bufsize];
	if (xml->encoding == ENC_LATIN1)
	{
		unsigned char* out = utf8buf;
		while (*in)
		{
			if (*in<128)
			{
				*out++=*in++;
			}
			else
			{
				*out++=0xc2+(*in>0xbf);
				*out++=(*in++&0x3f)+0x80;
			}
		}
		in = utf8buf;
	}
	wchar *buf = new wchar[bufsize];
	int iNumWritten = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (char*)in, -1, buf, bufsize);
	FAssertMsg(iNumWritten < bufsize, "UTF8 text too long, increase buffer size");
	// if the conversion fails, return false
	if (iNumWritten <= 0)
	{
		gDLL->MessageBoxA((char*)in, "UTF-8 decoding failed");
		delete [] buf;
		delete [] utf8buf;
		return false;
	}
	pszText = buf;
	delete [] buf;
	delete [] utf8buf;
	return true;
}

// get the value from the last located node, returns true if successful and false otherwise
bool CvDLLXMLRapid::GetLastNodeValue(FXml* xml, char* pszText)
{
	// this is quite dangerous, I hope it is not used
	strcpy(pszText, xml->node->value());
	return true;
}

bool CvDLLXMLRapid::GetLastNodeValue(FXml* xml, wchar* pszText)
{
	// this is quite dangerous
	std::wstring str;
	if (!GetLastNodeValue(xml, str))
	{
		return false;
	}
	wcscpy(pszText, str.c_str());
	return true;
}

// get the boolean value from the last located node, returns true if successful and false otherwise
bool CvDLLXMLRapid::GetLastNodeValue(FXml* xml, bool* pbVal)
{
	// to be implemented
	return false;
}

// get the integer value from the last located node, returns true if successful and false otherwise
bool CvDLLXMLRapid::GetLastNodeValue(FXml* xml, int* piVal)
{
	// to be implemented
	return false;
}

// get the float value from the last located node, returns true if successful and false otherwise
bool CvDLLXMLRapid::GetLastNodeValue(FXml* xml, float* pfVal)
{
	// to be implemented
	return false;
}

// get the unsigned int value from the last located node, returns true if successful and false otherwise
bool CvDLLXMLRapid::GetLastNodeValue(FXml* xml, unsigned int* puiVal)
{
	// to be implemented
	return false;
}

// get the size of the text from the last inserted node, returns the size
int CvDLLXMLRapid::GetInsertedNodeTextSize(FXml* xml)
{
	// not implemented
	return 0;
}

// get the text from the last inserted node, returns true if successful and false otherwise
bool CvDLLXMLRapid::GetInsertedNodeText(FXml* xml, TCHAR* pszText)
{
	// not implemented
	return false;
}

// set the text from the last located node, returns true if successful and false otherwise
//bool SetLastNodeText(TCHAR* pszText);
// set the text from the last inserted node, returns true if successful and false otherwise
bool CvDLLXMLRapid::SetInsertedNodeText(FXml* xml, TCHAR* pszText)
{
	// not implemented
	return false;
}

// get the type of the last located node, returns true if there was a type set and false otherwise
bool CvDLLXMLRapid::GetLastLocatedNodeType(FXml* xml, TCHAR* pszType)
{
	// we don't have types, some code that relies on this should be rewritten
	return false;
}

// get the type of the last inserted node, returns true if there was a type set and false otherwise
bool CvDLLXMLRapid::GetLastInsertedNodeType(FXml* xml, TCHAR* pszType)
{
	// not implemented
	return false;
}

// Indicates if the last located node is a comment node or not, returns true if it is and false otherwise
bool CvDLLXMLRapid::IsLastLocatedNodeCommentNode(FXml* xml)
{
	// we currently parse without comments which is better anyway so this should always be false
	return xml->node->type() == rapidxml::node_comment;
}

// returns the number of elements that have the tag name
int CvDLLXMLRapid::NumOfElementsByTagName(FXml* xml, TCHAR* pszTagName)
{
	// the code that uses this wants the same resolution as LocateNode
	// split pszTagName at / or \ and apply multiple first_node then
	rapidxml::xml_node<>* cur_node = &(xml->doc);
	char* buf = new char[strlen(pszTagName)+1];
	strcpy(buf, pszTagName);
	char* lasttok = NULL;
	for (char* tok = strtok(buf, "/\\"); tok != NULL; tok = strtok(NULL, "/\\"))
	{
		cur_node =cur_node->first_node(tok);
		if (cur_node == NULL)
		{
			delete[] buf;
			return 0;
		}
		lasttok = tok;
	}
	
	cur_node = cur_node->parent();
	rapidxml::xml_node<>* backup_node = xml->node;
	xml->node = cur_node;
	int iCount = NumOfChildrenByTagName(xml, lasttok);
	xml->node = backup_node;
	delete[] buf;
	return iCount;
}

// returns the number of children with the tag name
int CvDLLXMLRapid::NumOfChildrenByTagName(FXml* xml, const TCHAR* pszTagName)
{
	int iCount = 0;
	rapidxml::xml_node<>* cur_node = xml->node->first_node(pszTagName);
	while (cur_node != NULL)
	{
		iCount++;
		cur_node = cur_node->next_sibling(pszTagName);
	}
	return iCount;
}

// returns the number of siblings the current selected item/node has
int CvDLLXMLRapid::GetNumSiblings(FXml* xml)
{
	int iCount = 0;
	rapidxml::xml_node<>* cur_node = xml->node;
	xml->node = cur_node->parent();
	if (xml->node != NULL)
	{
		iCount = GetNumChildren(xml);
	}
	xml->node = cur_node;
	return iCount;
}

// returns the number of children the current selected item/node has
int CvDLLXMLRapid::GetNumChildren(FXml* xml)
{
	int iCount = 0;
	rapidxml::xml_node<>* cur_node = xml->node->first_node();
	while (cur_node != NULL)
	{
		iCount++;
		cur_node = cur_node->next_sibling();
	}
	return iCount;
}

// returns the tag name of the last located node, returns true if there is/was a last located node and false otherwise
bool CvDLLXMLRapid::GetLastLocatedNodeTagName(FXml* xml, TCHAR* pszTagName)
{
	strcpy(pszTagName, xml->node->name());
	return true;
}

bool CvDLLXMLRapid::IsAllowXMLCaching()
{
	return false;
}

void CvDLLXMLRapid::MapChildren(FXml*)
{
}


CvDLLXmlIFaceBase* getRapidXML()
{
	static CvDLLXMLRapid instance;
	return &instance;
}