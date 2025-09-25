
#include "stdafx.h"
#include "types.h"

#if defined(__linux__) || defined(__APPLE__)
#include <errno.h>
#include <iconv.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#endif

#include <wchar.h>

#include <string>

#include "AddInNative.h"
#include "utils.h"

static const wchar_t *g_PropNames[] = {NULL};
static const wchar_t *g_MethodNames[] = {L"GetVersion", NULL};

static const wchar_t *g_PropNamesRu[] = {NULL};
static const wchar_t *g_MethodNamesRu[] = {L"ПолучитьВерсию", NULL};

static const WCHAR_T g_kClassNames[] = u"CAddInNative";  //|OtherClass1|OtherClass2";

static AppCapabilities g_capabilities = eAppCapabilitiesInvalid;
static std::u16string s_names(g_kClassNames);
//---------------------------------------------------------------------------//
long GetClassObject(const WCHAR_T *wsName, IComponentBase **pInterface) {
    if (!*pInterface) {
        *pInterface = new CAddInNative();
        return (long)*pInterface;
    }
    return 0;
}
//---------------------------------------------------------------------------//
AppCapabilities SetPlatformCapabilities(const AppCapabilities capabilities) {
    g_capabilities = capabilities;
    return eAppCapabilitiesLast;
}
//---------------------------------------------------------------------------//
AttachType GetAttachType() {
    return eCanAttachAny;
}
//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase **pIntf) {
    if (!*pIntf)
        return -1;

    delete *pIntf;
    *pIntf = 0;
    return 0;
}
//---------------------------------------------------------------------------//
const WCHAR_T *GetClassNames() {
    return s_names.c_str();
}
//---------------------------------------------------------------------------//
// CAddInNative
CAddInNative::CAddInNative() {
    m_iMemory = nullptr;
    m_iConnect = nullptr;
}
//---------------------------------------------------------------------------//
CAddInNative::~CAddInNative() {}
//---------------------------------------------------------------------------//
bool CAddInNative::Init(void *pConnection) {
    m_iConnect = (IAddInDefBase *)pConnection;
    return m_iConnect != NULL;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetInfo() {
    return 2000;
}
//---------------------------------------------------------------------------//
void CAddInNative::Done() {}
/////////////////////////////////////////////////////////////////////////////
// ILanguageExtenderBase
//---------------------------------------------------------------------------//
bool CAddInNative::RegisterExtensionAs(WCHAR_T **wsExtensionName) {
    const wchar_t *wsExtension = L"AddInNativeExtension";
    size_t iActualSize = ::wcslen(wsExtension) + 1;
    WCHAR_T *dest = 0;

    if (m_iMemory) {
        if (m_iMemory->AllocMemory((void **)wsExtensionName, (unsigned)iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(wsExtensionName, wsExtension, iActualSize);
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNProps() {
    return eLastProp;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindProp(const WCHAR_T *wsPropName) {
    return -1;
}
//---------------------------------------------------------------------------//
const WCHAR_T *CAddInNative::GetPropName(long lPropNum, long lPropAlias) {
    return 0;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetPropVal(const long lPropNum, tVariant *pvarPropVal) {
    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::SetPropVal(const long lPropNum, tVariant *varPropVal) {
    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropReadable(const long lPropNum) {
    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropWritable(const long lPropNum) {
    return false;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNMethods() {
    return eLastMethod;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindMethod(const WCHAR_T *wsMethodName) {
    long plMethodNum = -1;
    wchar_t *name = 0;

    ::convFromShortWchar(&name, wsMethodName);

    plMethodNum = findName(g_MethodNames, name, eLastMethod);

    if (plMethodNum == -1)
        plMethodNum = findName(g_MethodNamesRu, name, eLastMethod);

    delete[] name;

    return plMethodNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T *CAddInNative::GetMethodName(const long lMethodNum, const long lMethodAlias) {
    if (lMethodNum >= eLastMethod)
        return NULL;

    wchar_t *wsCurrentName = NULL;
    WCHAR_T *wsMethodName = NULL;
    size_t iActualSize = 0;

    switch (lMethodAlias) {
        case 0:  // First language
            wsCurrentName = (wchar_t *)g_MethodNames[lMethodNum];
            break;
        case 1:  // Second language
            wsCurrentName = (wchar_t *)g_MethodNamesRu[lMethodNum];
            break;
        default:
            return 0;
    }

    iActualSize = wcslen(wsCurrentName) + 1;

    if (m_iMemory && wsCurrentName) {
        if (m_iMemory->AllocMemory((void **)&wsMethodName, (unsigned)iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(&wsMethodName, wsCurrentName, iActualSize);
    }

    return wsMethodName;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNParams(const long lMethodNum) {
    switch (lMethodNum) {
        case eMethGetVersion:
            return 0;
        default:
            return 0;
    }
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetParamDefValue(const long lMethodNum, const long lParamNum, tVariant *pvarParamDefValue) {
    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::HasRetVal(const long lMethodNum) {
    switch (lMethodNum) {
        case eMethGetVersion:
            return true;
        default:
            return false;
    }
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsProc(const long lMethodNum, tVariant *paParams, const long lSizeArray) {
    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsFunc(const long lMethodNum, tVariant *pvarRetValue, tVariant *paParams,
                              const long lSizeArray) {
    switch (lMethodNum) {
        case eMethGetVersion: {

            const wchar_t *version = L"1.0.0";
            size_t version_len = wcslen(version);

            if (!m_iMemory->AllocMemory((void **)&pvarRetValue->pwstrVal, (version_len + 1) * sizeof(WCHAR_T))) {
                return false; // Memory allocation failed
            }

            ::convToShortWchar(&pvarRetValue->pwstrVal, version, version_len + 1);

            pvarRetValue->wstrLen = version_len;
            TV_VT(pvarRetValue) = VTYPE_PWSTR;

            return true;
        }
        default:
            return false;
    }
}
void CAddInNative::SetLocale(const WCHAR_T *loc) {}
//---------------------------------------------------------------------------//
void ADDIN_API CAddInNative::SetUserInterfaceLanguageCode(const WCHAR_T *lang) {
    m_userLang.assign(lang);
}
//---------------------------------------------------------------------------//
bool CAddInNative::setMemManager(void *mem) {
    m_iMemory = (IMemoryManager *)mem;
    return m_iMemory != 0;
}
//---------------------------------------------------------А------------------//
long CAddInNative::findName(const wchar_t *names[], const wchar_t *name, const uint32_t size) const {
    long ret = -1;
    for (uint32_t i = 0; i < size; i++) {
        if (!wcscmp(names[i], name)) {
            ret = i;
            break;
        }
    }
    return ret;
}
