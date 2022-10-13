#pragma once

#ifndef CvDLLXMLRapid_h
#define CvDLLXMLRapid_h

#include "CvDLLXmlIFaceBase.h"

//
// a partial implementation of the CvDLLXMLIFaceBase abstract interface using RapidXML
//
class FXml;
class FXmlSchemaCache;
class CvDLLXMLRapid : public CvDLLXmlIFaceBase
{
public:
	virtual FXml* CreateFXml(FXmlSchemaCache* pSchemaCache=0);
	virtual void DestroyFXml(FXml*& xml);

	virtual void DestroyFXmlSchemaCache(FXmlSchemaCache*&);
	virtual FXmlSchemaCache* CreateFXmlSchemaCache();

	// load an xml file into memory, returns true if successfully loaded and false otherwise
	virtual bool LoadXml(FXml* xml, const TCHAR* pszXmlFile);
	// validate the document/xml, returns true if successful and false otherwise,
	//	if validate fails we also return an error string to indicate why it failed
	virtual bool Validate(FXml* xml, TCHAR* pszError=NULL);
	// locate a node in the document, returns true if a node is found and false otherwise
	virtual bool LocateNode(FXml* xml, const TCHAR* pszXmlNode);
	// locate the first sibling node in this sub-tree layer that has the xml tag name,
	// returns true if a node is found and false otherwise
	virtual bool LocateFirstSiblingNodeByTagName(FXml* xml, TCHAR* pszTagName);
	// locate the next sibling node in this sub-tree layer that has the xml tag name,
	// returns true if a node is found and false otherwise
	virtual bool LocateNextSiblingNodeByTagName(FXml* xml, TCHAR* pszTagName);
	// Set the last located xml node member variable to the next sibling of the current
	//	last located xml node, returns true if a next sibling was found and false otherwise
	virtual bool NextSibling(FXml* xml);
	// Set the last located xml node member variable to the prev sibling of the current
	//	last located xml node, returns true if a next sibling was found and false otherwise
	virtual bool PrevSibling(FXml* xml);
	// Set the last located xml node member variable to the it's first child,
	//	returns true if a next child was found and set, false otherwise
	virtual bool SetToChild(FXml* xml);
	// set to the child node that has the tag name
	virtual bool SetToChildByTagName(FXml* xml, const TCHAR* szTagName);
	// Set the last located xml node member variable to the it's parent,
	//	returns true if it's parent is found and set, false otherwise
	virtual bool SetToParent(FXml* xml);
	// add a node to the document as the child of the last located node,
	// returns true if the node was successfully added and false otherwise
	virtual bool AddChildNode(FXml* xml, TCHAR* pszNewNode);
	// adds a node to the document as a sibling to the last node located,
	// node is added before the last node located,
	// returns true if the node was successfully added and false otherwise
	virtual bool AddSiblingNodeBefore(FXml* xml, TCHAR* pszNewNode);
	// adds a node to the document as a sibling to the last node located,
	// node is added after the last node located,
	// returns true if the node was successfully added and false otherwise
	virtual bool AddSiblingNodeAfter(FXml* xml, TCHAR* pszNewNode);
	// write the xml document to disk, returns true if the write was successful and false otherwise
	virtual bool WriteXml(FXml* xml, TCHAR* pszXmlFile);
	// set the inserted node's attribute, returns true if successful and false otherwise
	virtual bool SetInsertedNodeAttribute(FXml* xml, TCHAR* pszAttributeName, TCHAR* pszAttributeValue);
	// get the size of the text from the last located node, returns the size
	virtual int GetLastNodeTextSize(FXml* xml);
	// get the text from the last located node, returns true if successful and false otherwise
	virtual bool GetLastNodeText(FXml* xml, TCHAR* pszText);
	// get the value from the last located node, returns true if successful and false otherwise
	virtual bool GetLastNodeValue(FXml* xml, std::string& pszText);
	virtual bool GetLastNodeValue(FXml* xml, std::wstring& pszText);
	// get the value from the last located node, returns true if successful and false otherwise
	virtual bool GetLastNodeValue(FXml* xml, char* pszText);
	virtual bool GetLastNodeValue(FXml* xml, wchar* pszText);
	// get the boolean value from the last located node, returns true if successful and false otherwise
	virtual bool GetLastNodeValue(FXml* xml, bool* pbVal);
	// get the integer value from the last located node, returns true if successful and false otherwise
	virtual bool GetLastNodeValue(FXml* xml, int* piVal);
	// get the float value from the last located node, returns true if successful and false otherwise
	virtual bool GetLastNodeValue(FXml* xml, float* pfVal);
	// get the unsigned int value from the last located node, returns true if successful and false otherwise
	virtual bool GetLastNodeValue(FXml* xml, unsigned int* puiVal);
	// get the size of the text from the last inserted node, returns the size
	virtual int GetInsertedNodeTextSize(FXml* xml);
	// get the text from the last inserted node, returns true if successful and false otherwise
	virtual bool GetInsertedNodeText(FXml* xml, TCHAR* pszText);
	// set the text from the last located node, returns true if successful and false otherwise
	//bool SetLastNodeText(TCHAR* pszText);
	// set the text from the last inserted node, returns true if successful and false otherwise
	virtual bool SetInsertedNodeText(FXml* xml, TCHAR* pszText);
	// get the type of the last located node, returns true if there was a type set and false otherwise
	virtual bool GetLastLocatedNodeType(FXml* xml, TCHAR* pszType);
	// get the type of the last inserted node, returns true if there was a type set and false otherwise
	virtual bool GetLastInsertedNodeType(FXml* xml, TCHAR* pszType);
	// Indicates if the last located node is a comment node or not, returns true if it is and false otherwise
	virtual bool IsLastLocatedNodeCommentNode(FXml* xml);
	// returns the number of elements that have the tag name
	virtual int NumOfElementsByTagName(FXml* xml, TCHAR* pszTagName);
	// returns the number of children with the tag name
	virtual int NumOfChildrenByTagName(FXml* xml, const TCHAR* pszTagName);
	// returns the number of siblings the current selected item/node has
	virtual int GetNumSiblings(FXml* xml);
	// returns the number of children the current selected item/node has
	virtual int GetNumChildren(FXml* xml);
	// returns the tag name of the last located node, returns true if there is/was a last located node and false otherwise
	virtual bool GetLastLocatedNodeTagName(FXml* xml, TCHAR* pszTagName);
	virtual bool IsAllowXMLCaching();
	virtual void MapChildren(FXml*);
};

#endif
