
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0347 */
/* Compiler settings for msdasc.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __msdasc_h__
#define __msdasc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IService_FWD_DEFINED__
#define __IService_FWD_DEFINED__
typedef interface IService IService;
#endif 	/* __IService_FWD_DEFINED__ */


#ifndef __IDBPromptInitialize_FWD_DEFINED__
#define __IDBPromptInitialize_FWD_DEFINED__
typedef interface IDBPromptInitialize IDBPromptInitialize;
#endif 	/* __IDBPromptInitialize_FWD_DEFINED__ */


#ifndef __IDataInitialize_FWD_DEFINED__
#define __IDataInitialize_FWD_DEFINED__
typedef interface IDataInitialize IDataInitialize;
#endif 	/* __IDataInitialize_FWD_DEFINED__ */


#ifndef __IDataSourceLocator_FWD_DEFINED__
#define __IDataSourceLocator_FWD_DEFINED__
typedef interface IDataSourceLocator IDataSourceLocator;
#endif 	/* __IDataSourceLocator_FWD_DEFINED__ */


#ifndef __DataLinks_FWD_DEFINED__
#define __DataLinks_FWD_DEFINED__

#ifdef __cplusplus
typedef class DataLinks DataLinks;
#else
typedef struct DataLinks DataLinks;
#endif /* __cplusplus */

#endif 	/* __DataLinks_FWD_DEFINED__ */


#ifndef __MSDAINITIALIZE_FWD_DEFINED__
#define __MSDAINITIALIZE_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSDAINITIALIZE MSDAINITIALIZE;
#else
typedef struct MSDAINITIALIZE MSDAINITIALIZE;
#endif /* __cplusplus */

#endif 	/* __MSDAINITIALIZE_FWD_DEFINED__ */


#ifndef __PDPO_FWD_DEFINED__
#define __PDPO_FWD_DEFINED__

#ifdef __cplusplus
typedef class PDPO PDPO;
#else
typedef struct PDPO PDPO;
#endif /* __cplusplus */

#endif 	/* __PDPO_FWD_DEFINED__ */


#ifndef __RootBinder_FWD_DEFINED__
#define __RootBinder_FWD_DEFINED__

#ifdef __cplusplus
typedef class RootBinder RootBinder;
#else
typedef struct RootBinder RootBinder;
#endif /* __cplusplus */

#endif 	/* __RootBinder_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "oledb.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

/* interface __MIDL_itf_msdasc_0000 */
/* [local] */ 

//-----------------------------------------------------------------------------
// File:				.C or .H file generated by msdasc.idl 
//
// Copyright:		Copyright (c) 1998-1999 Microsoft Corporation
//
// Contents: 		.C or .H file generated by msdasc.idl
//
// Comments: 		
//
//-----------------------------------------------------------------------------

#ifdef _WIN64

typedef LONGLONG			COMPATIBLE_LONG;

#else

typedef LONG COMPATIBLE_LONG;

#endif	// _WIN64
typedef 
enum tagEBindInfoOptions
    {	BIO_BINDER	= 0x1
    } 	EBindInfoOptions;

#define STGM_COLLECTION		0x00002000L
#define STGM_OUTPUT			0x00008000L
#define STGM_OPEN			0x80000000L
#define STGM_RECURSIVE		0x01000000L
#define STGM_STRICTOPEN		0x40000000L


extern RPC_IF_HANDLE __MIDL_itf_msdasc_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msdasc_0000_v0_0_s_ifspec;

#ifndef __IService_INTERFACE_DEFINED__
#define __IService_INTERFACE_DEFINED__

/* interface IService */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_IService;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("06210E88-01F5-11D1-B512-0080C781C384")
    IService : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE InvokeService( 
            /* [in] */ IUnknown *pUnkInner) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IServiceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IService * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IService * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IService * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *InvokeService )( 
            IService * This,
            /* [in] */ IUnknown *pUnkInner);
        
        END_INTERFACE
    } IServiceVtbl;

    interface IService
    {
        CONST_VTBL struct IServiceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IService_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IService_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IService_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IService_InvokeService(This,pUnkInner)	\
    (This)->lpVtbl -> InvokeService(This,pUnkInner)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [local] */ HRESULT STDMETHODCALLTYPE IService_InvokeService_Proxy( 
    IService * This,
    /* [in] */ IUnknown *pUnkInner);


void __RPC_STUB IService_InvokeService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IService_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_msdasc_0353 */
/* [local] */ 

typedef DWORD DBPROMPTOPTIONS;

typedef 
enum tagDBPROMPTOPTIONSENUM
    {	DBPROMPTOPTIONS_NONE	= 0,
	DBPROMPTOPTIONS_WIZARDSHEET	= 0x1,
	DBPROMPTOPTIONS_PROPERTYSHEET	= 0x2,
	DBPROMPTOPTIONS_BROWSEONLY	= 0x8,
	DBPROMPTOPTIONS_DISABLE_PROVIDER_SELECTION	= 0x10,
	DBPROMPTOPTIONS_DISABLESAVEPASSWORD	= 0x20
    } 	DBPROMPTOPTIONSENUM;



extern RPC_IF_HANDLE __MIDL_itf_msdasc_0353_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msdasc_0353_v0_0_s_ifspec;

#ifndef __IDBPromptInitialize_INTERFACE_DEFINED__
#define __IDBPromptInitialize_INTERFACE_DEFINED__

/* interface IDBPromptInitialize */
/* [restricted][local][unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDBPromptInitialize;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2206CCB0-19C1-11D1-89E0-00C04FD7A829")
    IDBPromptInitialize : public IUnknown
    {
    public:
        virtual /* [local][helpstring] */ HRESULT STDMETHODCALLTYPE PromptDataSource( 
            /* [in] */ IUnknown *pUnkOuter,
            /* [in] */ HWND hWndParent,
            /* [in] */ DBPROMPTOPTIONS dwPromptOptions,
            /* [in] */ ULONG cSourceTypeFilter,
            /* [size_is][in] */ DBSOURCETYPE *rgSourceTypeFilter,
            /* [in] */ LPCOLESTR pwszszzProviderFilter,
            /* [in] */ REFIID riid,
            /* [iid_is][out][in] */ IUnknown **ppDataSource) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE PromptFileName( 
            /* [in] */ HWND hWndParent,
            /* [in] */ DBPROMPTOPTIONS dwPromptOptions,
            /* [in] */ LPCOLESTR pwszInitialDirectory,
            /* [in] */ LPCOLESTR pwszInitialFile,
            /* [out] */ LPOLESTR *ppwszSelectedFile) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDBPromptInitializeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDBPromptInitialize * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDBPromptInitialize * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDBPromptInitialize * This);
        
        /* [local][helpstring] */ HRESULT ( STDMETHODCALLTYPE *PromptDataSource )( 
            IDBPromptInitialize * This,
            /* [in] */ IUnknown *pUnkOuter,
            /* [in] */ HWND hWndParent,
            /* [in] */ DBPROMPTOPTIONS dwPromptOptions,
            /* [in] */ ULONG cSourceTypeFilter,
            /* [size_is][in] */ DBSOURCETYPE *rgSourceTypeFilter,
            /* [in] */ LPCOLESTR pwszszzProviderFilter,
            /* [in] */ REFIID riid,
            /* [iid_is][out][in] */ IUnknown **ppDataSource);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *PromptFileName )( 
            IDBPromptInitialize * This,
            /* [in] */ HWND hWndParent,
            /* [in] */ DBPROMPTOPTIONS dwPromptOptions,
            /* [in] */ LPCOLESTR pwszInitialDirectory,
            /* [in] */ LPCOLESTR pwszInitialFile,
            /* [out] */ LPOLESTR *ppwszSelectedFile);
        
        END_INTERFACE
    } IDBPromptInitializeVtbl;

    interface IDBPromptInitialize
    {
        CONST_VTBL struct IDBPromptInitializeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBPromptInitialize_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBPromptInitialize_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBPromptInitialize_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBPromptInitialize_PromptDataSource(This,pUnkOuter,hWndParent,dwPromptOptions,cSourceTypeFilter,rgSourceTypeFilter,pwszszzProviderFilter,riid,ppDataSource)	\
    (This)->lpVtbl -> PromptDataSource(This,pUnkOuter,hWndParent,dwPromptOptions,cSourceTypeFilter,rgSourceTypeFilter,pwszszzProviderFilter,riid,ppDataSource)

#define IDBPromptInitialize_PromptFileName(This,hWndParent,dwPromptOptions,pwszInitialDirectory,pwszInitialFile,ppwszSelectedFile)	\
    (This)->lpVtbl -> PromptFileName(This,hWndParent,dwPromptOptions,pwszInitialDirectory,pwszInitialFile,ppwszSelectedFile)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [local][helpstring] */ HRESULT STDMETHODCALLTYPE IDBPromptInitialize_PromptDataSource_Proxy( 
    IDBPromptInitialize * This,
    /* [in] */ IUnknown *pUnkOuter,
    /* [in] */ HWND hWndParent,
    /* [in] */ DBPROMPTOPTIONS dwPromptOptions,
    /* [in] */ ULONG cSourceTypeFilter,
    /* [size_is][in] */ DBSOURCETYPE *rgSourceTypeFilter,
    /* [in] */ LPCOLESTR pwszszzProviderFilter,
    /* [in] */ REFIID riid,
    /* [iid_is][out][in] */ IUnknown **ppDataSource);


void __RPC_STUB IDBPromptInitialize_PromptDataSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDBPromptInitialize_PromptFileName_Proxy( 
    IDBPromptInitialize * This,
    /* [in] */ HWND hWndParent,
    /* [in] */ DBPROMPTOPTIONS dwPromptOptions,
    /* [in] */ LPCOLESTR pwszInitialDirectory,
    /* [in] */ LPCOLESTR pwszInitialFile,
    /* [out] */ LPOLESTR *ppwszSelectedFile);


void __RPC_STUB IDBPromptInitialize_PromptFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDBPromptInitialize_INTERFACE_DEFINED__ */


#ifndef __IDataInitialize_INTERFACE_DEFINED__
#define __IDataInitialize_INTERFACE_DEFINED__

/* interface IDataInitialize */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDataInitialize;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2206CCB1-19C1-11D1-89E0-00C04FD7A829")
    IDataInitialize : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDataSource( 
            /* [in] */ IUnknown *pUnkOuter,
            /* [in] */ DWORD dwClsCtx,
            /* [unique][in] */ LPCOLESTR pwszInitializationString,
            /* [in] */ REFIID riid,
            /* [iid_is][out][in] */ IUnknown **ppDataSource) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetInitializationString( 
            /* [in] */ IUnknown *pDataSource,
            /* [in] */ boolean fIncludePassword,
            /* [out] */ LPOLESTR *ppwszInitString) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CreateDBInstance( 
            /* [in] */ REFCLSID clsidProvider,
            /* [in] */ IUnknown *pUnkOuter,
            /* [in] */ DWORD dwClsCtx,
            /* [unique][in] */ LPOLESTR pwszReserved,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown **ppDataSource) = 0;
        
        virtual /* [local][helpstring] */ HRESULT STDMETHODCALLTYPE CreateDBInstanceEx( 
            /* [in] */ REFCLSID clsidProvider,
            /* [in] */ IUnknown *pUnkOuter,
            /* [in] */ DWORD dwClsCtx,
            /* [unique][in] */ LPOLESTR pwszReserved,
            /* [unique][in] */ COSERVERINFO *pServerInfo,
            /* [in] */ ULONG cmq,
            /* [size_is][out][in] */ MULTI_QI *rgmqResults) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE LoadStringFromStorage( 
            /* [unique][in] */ LPCOLESTR pwszFileName,
            /* [out] */ LPOLESTR *ppwszInitializationString) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE WriteStringToStorage( 
            /* [unique][in] */ LPCOLESTR pwszFileName,
            /* [unique][in] */ LPCOLESTR pwszInitializationString,
            /* [in] */ DWORD dwCreationDisposition) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDataInitializeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDataInitialize * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDataInitialize * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDataInitialize * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetDataSource )( 
            IDataInitialize * This,
            /* [in] */ IUnknown *pUnkOuter,
            /* [in] */ DWORD dwClsCtx,
            /* [unique][in] */ LPCOLESTR pwszInitializationString,
            /* [in] */ REFIID riid,
            /* [iid_is][out][in] */ IUnknown **ppDataSource);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetInitializationString )( 
            IDataInitialize * This,
            /* [in] */ IUnknown *pDataSource,
            /* [in] */ boolean fIncludePassword,
            /* [out] */ LPOLESTR *ppwszInitString);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *CreateDBInstance )( 
            IDataInitialize * This,
            /* [in] */ REFCLSID clsidProvider,
            /* [in] */ IUnknown *pUnkOuter,
            /* [in] */ DWORD dwClsCtx,
            /* [unique][in] */ LPOLESTR pwszReserved,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown **ppDataSource);
        
        /* [local][helpstring] */ HRESULT ( STDMETHODCALLTYPE *CreateDBInstanceEx )( 
            IDataInitialize * This,
            /* [in] */ REFCLSID clsidProvider,
            /* [in] */ IUnknown *pUnkOuter,
            /* [in] */ DWORD dwClsCtx,
            /* [unique][in] */ LPOLESTR pwszReserved,
            /* [unique][in] */ COSERVERINFO *pServerInfo,
            /* [in] */ ULONG cmq,
            /* [size_is][out][in] */ MULTI_QI *rgmqResults);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *LoadStringFromStorage )( 
            IDataInitialize * This,
            /* [unique][in] */ LPCOLESTR pwszFileName,
            /* [out] */ LPOLESTR *ppwszInitializationString);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *WriteStringToStorage )( 
            IDataInitialize * This,
            /* [unique][in] */ LPCOLESTR pwszFileName,
            /* [unique][in] */ LPCOLESTR pwszInitializationString,
            /* [in] */ DWORD dwCreationDisposition);
        
        END_INTERFACE
    } IDataInitializeVtbl;

    interface IDataInitialize
    {
        CONST_VTBL struct IDataInitializeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDataInitialize_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDataInitialize_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDataInitialize_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDataInitialize_GetDataSource(This,pUnkOuter,dwClsCtx,pwszInitializationString,riid,ppDataSource)	\
    (This)->lpVtbl -> GetDataSource(This,pUnkOuter,dwClsCtx,pwszInitializationString,riid,ppDataSource)

#define IDataInitialize_GetInitializationString(This,pDataSource,fIncludePassword,ppwszInitString)	\
    (This)->lpVtbl -> GetInitializationString(This,pDataSource,fIncludePassword,ppwszInitString)

#define IDataInitialize_CreateDBInstance(This,clsidProvider,pUnkOuter,dwClsCtx,pwszReserved,riid,ppDataSource)	\
    (This)->lpVtbl -> CreateDBInstance(This,clsidProvider,pUnkOuter,dwClsCtx,pwszReserved,riid,ppDataSource)

#define IDataInitialize_CreateDBInstanceEx(This,clsidProvider,pUnkOuter,dwClsCtx,pwszReserved,pServerInfo,cmq,rgmqResults)	\
    (This)->lpVtbl -> CreateDBInstanceEx(This,clsidProvider,pUnkOuter,dwClsCtx,pwszReserved,pServerInfo,cmq,rgmqResults)

#define IDataInitialize_LoadStringFromStorage(This,pwszFileName,ppwszInitializationString)	\
    (This)->lpVtbl -> LoadStringFromStorage(This,pwszFileName,ppwszInitializationString)

#define IDataInitialize_WriteStringToStorage(This,pwszFileName,pwszInitializationString,dwCreationDisposition)	\
    (This)->lpVtbl -> WriteStringToStorage(This,pwszFileName,pwszInitializationString,dwCreationDisposition)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDataInitialize_GetDataSource_Proxy( 
    IDataInitialize * This,
    /* [in] */ IUnknown *pUnkOuter,
    /* [in] */ DWORD dwClsCtx,
    /* [unique][in] */ LPCOLESTR pwszInitializationString,
    /* [in] */ REFIID riid,
    /* [iid_is][out][in] */ IUnknown **ppDataSource);


void __RPC_STUB IDataInitialize_GetDataSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDataInitialize_GetInitializationString_Proxy( 
    IDataInitialize * This,
    /* [in] */ IUnknown *pDataSource,
    /* [in] */ boolean fIncludePassword,
    /* [out] */ LPOLESTR *ppwszInitString);


void __RPC_STUB IDataInitialize_GetInitializationString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDataInitialize_CreateDBInstance_Proxy( 
    IDataInitialize * This,
    /* [in] */ REFCLSID clsidProvider,
    /* [in] */ IUnknown *pUnkOuter,
    /* [in] */ DWORD dwClsCtx,
    /* [unique][in] */ LPOLESTR pwszReserved,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown **ppDataSource);


void __RPC_STUB IDataInitialize_CreateDBInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IDataInitialize_RemoteCreateDBInstanceEx_Proxy( 
    IDataInitialize * This,
    /* [in] */ REFCLSID clsidProvider,
    /* [in] */ IUnknown *pUnkOuter,
    /* [in] */ DWORD dwClsCtx,
    /* [unique][in] */ LPOLESTR pwszReserved,
    /* [unique][in] */ COSERVERINFO *pServerInfo,
    /* [in] */ ULONG cmq,
    /* [size_is][size_is][in] */ const IID **rgpIID,
    /* [size_is][size_is][out] */ IUnknown **rgpItf,
    /* [size_is][out] */ HRESULT *rghr);


void __RPC_STUB IDataInitialize_RemoteCreateDBInstanceEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDataInitialize_LoadStringFromStorage_Proxy( 
    IDataInitialize * This,
    /* [unique][in] */ LPCOLESTR pwszFileName,
    /* [out] */ LPOLESTR *ppwszInitializationString);


void __RPC_STUB IDataInitialize_LoadStringFromStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDataInitialize_WriteStringToStorage_Proxy( 
    IDataInitialize * This,
    /* [unique][in] */ LPCOLESTR pwszFileName,
    /* [unique][in] */ LPCOLESTR pwszInitializationString,
    /* [in] */ DWORD dwCreationDisposition);


void __RPC_STUB IDataInitialize_WriteStringToStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDataInitialize_INTERFACE_DEFINED__ */



#ifndef __MSDASC_LIBRARY_DEFINED__
#define __MSDASC_LIBRARY_DEFINED__

/* library MSDASC */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_MSDASC;

#ifndef __IDataSourceLocator_INTERFACE_DEFINED__
#define __IDataSourceLocator_INTERFACE_DEFINED__

/* interface IDataSourceLocator */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IDataSourceLocator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2206CCB2-19C1-11D1-89E0-00C04FD7A829")
    IDataSourceLocator : public IDispatch
    {
    public:
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_hWnd( 
            /* [retval][out] */ COMPATIBLE_LONG *phwndParent) = 0;
        
        virtual /* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE put_hWnd( 
            /* [in] */ COMPATIBLE_LONG hwndParent) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE PromptNew( 
            /* [retval][out] */ IDispatch **ppADOConnection) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE PromptEdit( 
            /* [out][in] */ IDispatch **ppADOConnection,
            /* [retval][out] */ VARIANT_BOOL *pbSuccess) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDataSourceLocatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDataSourceLocator * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDataSourceLocator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDataSourceLocator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDataSourceLocator * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDataSourceLocator * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDataSourceLocator * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDataSourceLocator * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE *get_hWnd )( 
            IDataSourceLocator * This,
            /* [retval][out] */ COMPATIBLE_LONG *phwndParent);
        
        /* [helpstring][propput] */ HRESULT ( STDMETHODCALLTYPE *put_hWnd )( 
            IDataSourceLocator * This,
            /* [in] */ COMPATIBLE_LONG hwndParent);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *PromptNew )( 
            IDataSourceLocator * This,
            /* [retval][out] */ IDispatch **ppADOConnection);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *PromptEdit )( 
            IDataSourceLocator * This,
            /* [out][in] */ IDispatch **ppADOConnection,
            /* [retval][out] */ VARIANT_BOOL *pbSuccess);
        
        END_INTERFACE
    } IDataSourceLocatorVtbl;

    interface IDataSourceLocator
    {
        CONST_VTBL struct IDataSourceLocatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDataSourceLocator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDataSourceLocator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDataSourceLocator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDataSourceLocator_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDataSourceLocator_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDataSourceLocator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDataSourceLocator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDataSourceLocator_get_hWnd(This,phwndParent)	\
    (This)->lpVtbl -> get_hWnd(This,phwndParent)

#define IDataSourceLocator_put_hWnd(This,hwndParent)	\
    (This)->lpVtbl -> put_hWnd(This,hwndParent)

#define IDataSourceLocator_PromptNew(This,ppADOConnection)	\
    (This)->lpVtbl -> PromptNew(This,ppADOConnection)

#define IDataSourceLocator_PromptEdit(This,ppADOConnection,pbSuccess)	\
    (This)->lpVtbl -> PromptEdit(This,ppADOConnection,pbSuccess)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IDataSourceLocator_get_hWnd_Proxy( 
    IDataSourceLocator * This,
    /* [retval][out] */ COMPATIBLE_LONG *phwndParent);


void __RPC_STUB IDataSourceLocator_get_hWnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propput] */ HRESULT STDMETHODCALLTYPE IDataSourceLocator_put_hWnd_Proxy( 
    IDataSourceLocator * This,
    /* [in] */ COMPATIBLE_LONG hwndParent);


void __RPC_STUB IDataSourceLocator_put_hWnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDataSourceLocator_PromptNew_Proxy( 
    IDataSourceLocator * This,
    /* [retval][out] */ IDispatch **ppADOConnection);


void __RPC_STUB IDataSourceLocator_PromptNew_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IDataSourceLocator_PromptEdit_Proxy( 
    IDataSourceLocator * This,
    /* [out][in] */ IDispatch **ppADOConnection,
    /* [retval][out] */ VARIANT_BOOL *pbSuccess);


void __RPC_STUB IDataSourceLocator_PromptEdit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDataSourceLocator_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_DataLinks;

#ifdef __cplusplus

class DECLSPEC_UUID("2206CDB2-19C1-11D1-89E0-00C04FD7A829")
DataLinks;
#endif

EXTERN_C const CLSID CLSID_MSDAINITIALIZE;

#ifdef __cplusplus

class DECLSPEC_UUID("2206CDB0-19C1-11D1-89E0-00C04FD7A829")
MSDAINITIALIZE;
#endif

EXTERN_C const CLSID CLSID_PDPO;

#ifdef __cplusplus

class DECLSPEC_UUID("CCB4EC60-B9DC-11D1-AC80-00A0C9034873")
PDPO;
#endif

EXTERN_C const CLSID CLSID_RootBinder;

#ifdef __cplusplus

class DECLSPEC_UUID("FF151822-B0BF-11D1-A80D-000000000000")
RootBinder;
#endif
#endif /* __MSDASC_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* [local][helpstring] */ HRESULT STDMETHODCALLTYPE IDataInitialize_CreateDBInstanceEx_Proxy( 
    IDataInitialize * This,
    /* [in] */ REFCLSID clsidProvider,
    /* [in] */ IUnknown *pUnkOuter,
    /* [in] */ DWORD dwClsCtx,
    /* [unique][in] */ LPOLESTR pwszReserved,
    /* [unique][in] */ COSERVERINFO *pServerInfo,
    /* [in] */ ULONG cmq,
    /* [size_is][out][in] */ MULTI_QI *rgmqResults);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IDataInitialize_CreateDBInstanceEx_Stub( 
    IDataInitialize * This,
    /* [in] */ REFCLSID clsidProvider,
    /* [in] */ IUnknown *pUnkOuter,
    /* [in] */ DWORD dwClsCtx,
    /* [unique][in] */ LPOLESTR pwszReserved,
    /* [unique][in] */ COSERVERINFO *pServerInfo,
    /* [in] */ ULONG cmq,
    /* [size_is][size_is][in] */ const IID **rgpIID,
    /* [size_is][size_is][out] */ IUnknown **rgpItf,
    /* [size_is][out] */ HRESULT *rghr);



/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


