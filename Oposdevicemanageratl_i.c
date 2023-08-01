

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 05:14:07 2038
 */
/* Compiler settings for Oposdevicemanageratl.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.01.0628 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        EXTERN_C __declspec(selectany) const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif // !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IOposDeviceManager,0x2f1ee96f,0x1a60,0x4954,0x9d,0xdd,0xc4,0x75,0xb9,0x74,0x25,0x3c);


MIDL_DEFINE_GUID(IID, IID_IMyDeviceManagerEvents,0xFD22695F,0x84FF,0x427B,0xB7,0x63,0xB1,0xDD,0x83,0x76,0xE3,0x7E);


MIDL_DEFINE_GUID(IID, LIBID_OposdevicemanageratlLib,0xfc266689,0x1baa,0x4157,0xa8,0x47,0x23,0xda,0xab,0x73,0x48,0xac);


MIDL_DEFINE_GUID(IID, DIID__IOposDeviceManagerEvents,0x47704e5b,0xbea7,0x4c8b,0xad,0xa7,0x34,0x7e,0x95,0x15,0xbd,0xad);


MIDL_DEFINE_GUID(CLSID, CLSID_OposDeviceManager,0x3c4ad21c,0x1f81,0x46e5,0x9d,0x7f,0x35,0x73,0xf8,0xb6,0x30,0x2f);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



