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
#include <stdexcept>

#include "AddInNative.h"
#include "TcpClient.h"
#include "utils.h"

static const wchar_t *g_PropNames[] = {
    L"ErrorMsg",
    L"Connected",
    L"QueueSize",
    NULL
};

static const wchar_t *g_PropNamesRu[] = {
    L"Ошибка",
    L"Подключено",
    L"РазмерОчереди",
    NULL
};

static const wchar_t *g_MethodNames[] = {
    L"Initialize",
    L"SendCommand",
    L"GetStatus",
    L"Disconnect",
    L"HasMessage",
    L"PopMessage",
    L"Receive",
    L"GetVersion",
    NULL
};

static const wchar_t *g_MethodNamesRu[] = {
    L"Инициализация",
    L"ПослатьКоманду",
    L"СостояниеПодключения",
    L"Отключиться",
    L"ЕстьСообщение",
    L"ПолучитьСообщение",
    L"ОжидатьСообщение",
    L"ПолучитьВерсию",
    NULL
};

static const WCHAR_T g_kClassNames[] = u"CAddInNative";

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
    m_tcp = std::make_unique<TcpClient>();
}

//---------------------------------------------------------------------------//
CAddInNative::~CAddInNative() {}

//---------------------------------------------------------------------------//
bool CAddInNative::Init(void *pConnection) {
    m_iConnect = (IAddInDefBase *)pConnection;
    return m_iConnect != NULL;
}

//---------------------------------------------------------------------------//
bool CAddInNative::setMemManager(void *mem) {
    m_iMemory = (IMemoryManager *)mem;
    return m_iMemory != 0;
}

//---------------------------------------------------------------------------//
long CAddInNative::GetInfo() {
    return 2000;
}

//---------------------------------------------------------------------------//
void CAddInNative::Done() {
    if (m_tcp) {
        std::string err;
        m_tcp->disconnect(err);
    }
}

/////////////////////////////////////////////////////////////////////////////
// ILanguageExtenderBase

//---------------------------------------------------------------------------//
bool CAddInNative::RegisterExtensionAs(WCHAR_T **wsExtensionName) {
    const wchar_t *wsExtension = L"vk_tcp";
    size_t iActualSize = ::wcslen(wsExtension) + 1;

    if (m_iMemory) {
        if (m_iMemory->AllocMemory((void **)wsExtensionName,
                                   (unsigned)iActualSize * sizeof(WCHAR_T))) {
            ::convToShortWchar(wsExtensionName, wsExtension, iActualSize);
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------//
long CAddInNative::GetNProps() {
    return eLastProp;
}

//---------------------------------------------------------------------------//
long CAddInNative::FindProp(const WCHAR_T *wsPropName) {
    long propNum = -1;
    wchar_t *name = 0;

    ::convFromShortWchar(&name, wsPropName);

    propNum = findName(g_PropNames, name, eLastProp);
    if (propNum == -1)
        propNum = findName(g_PropNamesRu, name, eLastProp);

    delete[] name;
    return propNum;
}

//---------------------------------------------------------------------------//
const WCHAR_T *CAddInNative::GetPropName(long lPropNum, long lPropAlias) {
    if (lPropNum >= eLastProp)
        return 0;

    wchar_t *wsCurrentName = NULL;
    WCHAR_T *wsPropName = NULL;
    size_t iActualSize = 0;

    switch (lPropAlias) {
        case 0:
            wsCurrentName = (wchar_t *)g_PropNames[lPropNum];
            break;
        case 1:
            wsCurrentName = (wchar_t *)g_PropNamesRu[lPropNum];
            break;
        default:
            return 0;
    }

    iActualSize = wcslen(wsCurrentName) + 1;

    if (m_iMemory && wsCurrentName) {
        if (m_iMemory->AllocMemory((void **)&wsPropName,
                                   (unsigned)iActualSize * sizeof(WCHAR_T))) {
            ::convToShortWchar(&wsPropName, wsCurrentName, iActualSize);
        }
    }

    return wsPropName;
}

//---------------------------------------------------------------------------//
bool CAddInNative::GetPropVal(const long lPropNum, tVariant *pvarPropVal) {
    if (!pvarPropVal)
        return false;

    switch (lPropNum) {
        case ePropErrorMsg:
            return returnWString(getLastError(), pvarPropVal);

        case ePropConnected:
            returnBool(m_tcp && m_tcp->isConnected(), pvarPropVal);
            return true;

        case ePropQueueSize:
            returnInt(m_tcp ? (int)m_tcp->queueSize() : 0, pvarPropVal);
            return true;

        default:
            return false;
    }
}

//---------------------------------------------------------------------------//
bool CAddInNative::SetPropVal(const long lPropNum, tVariant *varPropVal) {
    return false;
}

//---------------------------------------------------------------------------//
bool CAddInNative::IsPropReadable(const long lPropNum) {
    return (lPropNum >= 0 && lPropNum < eLastProp);
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
    long methodNum = -1;
    wchar_t *name = 0;

    ::convFromShortWchar(&name, wsMethodName);

    methodNum = findName(g_MethodNames, name, eLastMethod);
    if (methodNum == -1)
        methodNum = findName(g_MethodNamesRu, name, eLastMethod);

    delete[] name;
    return methodNum;
}

//---------------------------------------------------------------------------//
const WCHAR_T *CAddInNative::GetMethodName(const long lMethodNum, const long lMethodAlias) {
    if (lMethodNum >= eLastMethod)
        return NULL;

    wchar_t *wsCurrentName = NULL;
    WCHAR_T *wsMethodName = NULL;
    size_t iActualSize = 0;

    switch (lMethodAlias) {
        case 0:
            wsCurrentName = (wchar_t *)g_MethodNames[lMethodNum];
            break;
        case 1:
            wsCurrentName = (wchar_t *)g_MethodNamesRu[lMethodNum];
            break;
        default:
            return 0;
    }

    iActualSize = wcslen(wsCurrentName) + 1;

    if (m_iMemory && wsCurrentName) {
        if (m_iMemory->AllocMemory((void **)&wsMethodName,
                                   (unsigned)iActualSize * sizeof(WCHAR_T))) {
            ::convToShortWchar(&wsMethodName, wsCurrentName, iActualSize);
        }
    }

    return wsMethodName;
}

//---------------------------------------------------------------------------//
long CAddInNative::GetNParams(const long lMethodNum) {
    switch (lMethodNum) {
        case eMethInitialize:
            return 2;
        case eMethSendCommand:
            return 2;
        case eMethGetStatus:
            return 0;
        case eMethDisconnect:
            return 0;
        case eMethHasMessage:
            return 0;
        case eMethPopMessage:
            return 0;
        case eMethReceive:
            return 1;
        case eMethGetVersion:
            return 0;
        default:
            return 0;
    }
}

//---------------------------------------------------------------------------//
bool CAddInNative::GetParamDefValue(const long lMethodNum, const long lParamNum,
                                    tVariant *pvarParamDefValue) {
    if (!pvarParamDefValue)
        return false;

    if (lMethodNum == eMethReceive && lParamNum == 0) {
        tVarInit(pvarParamDefValue);
        TV_VT(pvarParamDefValue) = VTYPE_I4;
        TV_I4(pvarParamDefValue) = 0;
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------//
bool CAddInNative::HasRetVal(const long lMethodNum) {
    switch (lMethodNum) {
        case eMethInitialize:
        case eMethSendCommand:
        case eMethGetStatus:
        case eMethDisconnect:
        case eMethHasMessage:
        case eMethPopMessage:
        case eMethReceive:
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
bool CAddInNative::CallAsFunc(const long lMethodNum, tVariant *pvarRetValue,
                              tVariant *paParams, const long lSizeArray) {
    try {
        switch (lMethodNum) {
            case eMethInitialize: {
                if (lSizeArray != 2)
                    return false;

                std::wstring whost = variantToWString(paParams[0]);
                int port = variantToInt(paParams[1]);

                std::string host(whost.begin(), whost.end());
                std::string err;
                bool ok = m_tcp->connectTo(host, port, err);

                if (!ok)
                    setLastError(std::wstring(err.begin(), err.end()));
                else
                    setLastError(L"");

                returnBool(ok, pvarRetValue);
                return true;
            }

            case eMethSendCommand: {
                if (lSizeArray != 2)
                    return false;

                std::wstring wdata = variantToWString(paParams[0]);
                int len = variantToInt(paParams[1]);

                std::string data(wdata.begin(), wdata.end());
                if (len < 0 || len > (int)data.size()) {
                    setLastError(L"Invalid length in SendCommand");
                    returnBool(false, pvarRetValue);
                    return true;
                }

                std::string err;
                bool ok = m_tcp->sendBytes(data.substr(0, len), err);

                if (!ok)
                    setLastError(std::wstring(err.begin(), err.end()));
                else
                    setLastError(L"");

                returnBool(ok, pvarRetValue);
                return true;
            }

            case eMethGetStatus: {
                returnBool(m_tcp && m_tcp->isConnected(), pvarRetValue);
                return true;
            }

            case eMethDisconnect: {
                std::string err;
                bool ok = m_tcp->disconnect(err);

                if (!ok)
                    setLastError(std::wstring(err.begin(), err.end()));
                else
                    setLastError(L"");

                returnBool(ok, pvarRetValue);
                return true;
            }

            case eMethHasMessage: {
                returnBool(m_tcp && m_tcp->hasMessage(), pvarRetValue);
                return true;
            }

            case eMethPopMessage: {
                auto msg = m_tcp->popMessage();
                if (msg.has_value())
                    return returnWString(std::wstring(msg->begin(), msg->end()), pvarRetValue);

                return returnWString(L"", pvarRetValue);
            }

            case eMethReceive: {
                int timeoutMs = 0;
                if (lSizeArray == 1)
                    timeoutMs = variantToInt(paParams[0]);

                auto msg = m_tcp->receive(timeoutMs);
                if (msg.has_value())
                    return returnWString(std::wstring(msg->begin(), msg->end()), pvarRetValue);

                return returnWString(L"", pvarRetValue);
            }

            case eMethGetVersion: {
                return returnWString(L"1.0.0", pvarRetValue);
            }

            default:
                return false;
        }
    } catch (const std::exception& e) {
        std::string msg = e.what();
        setLastError(std::wstring(msg.begin(), msg.end()));
        returnBool(false, pvarRetValue);
        return true;
    }
}

//---------------------------------------------------------------------------//
void CAddInNative::SetLocale(const WCHAR_T *loc) {}

//---------------------------------------------------------------------------//
void ADDIN_API CAddInNative::SetUserInterfaceLanguageCode(const WCHAR_T *lang) {
    m_userLang.assign(lang);
}

//---------------------------------------------------------------------------//
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

//---------------------------------------------------------------------------//
void CAddInNative::setLastError(const std::wstring& text) {
    std::lock_guard<std::mutex> lock(m_errorMutex);
    m_errorMsg = text;
}

//---------------------------------------------------------------------------//
std::wstring CAddInNative::getLastError() const {
    std::lock_guard<std::mutex> lock(m_errorMutex);
    return m_errorMsg;
}

//---------------------------------------------------------------------------//
bool CAddInNative::allocAndCopyWString(const wchar_t* src, WCHAR_T** out) const {
    if (!m_iMemory || !src || !out)
        return false;

    size_t size = wcslen(src) + 1;
    if (!m_iMemory->AllocMemory((void**)out, (unsigned)(size * sizeof(WCHAR_T))))
        return false;

    ::convToShortWchar(out, src, size);
    return true;
}

//---------------------------------------------------------------------------//
bool CAddInNative::returnWString(const std::wstring& value, tVariant* ret) const {
    if (!ret)
        return false;

    tVarInit(ret);

    if (!allocAndCopyWString(value.c_str(), &ret->pwstrVal))
        return false;

    ret->wstrLen = (uint32_t)value.size();
    TV_VT(ret) = VTYPE_PWSTR;
    return true;
}

//---------------------------------------------------------------------------//
void CAddInNative::returnBool(bool value, tVariant* ret) const {
    tVarInit(ret);
    TV_VT(ret) = VTYPE_BOOL;
    TV_BOOL(ret) = value;
}

//---------------------------------------------------------------------------//
void CAddInNative::returnInt(int value, tVariant* ret) const {
    tVarInit(ret);
    TV_VT(ret) = VTYPE_I4;
    TV_I4(ret) = value;
}

//---------------------------------------------------------------------------//
std::wstring CAddInNative::variantToWString(const tVariant& value) const {
    if (TV_VT(&value) == VTYPE_PWSTR && value.pwstrVal) {
        wchar_t* tmp = nullptr;
        ::convFromShortWchar(&tmp, value.pwstrVal, value.wstrLen + 1);
        std::wstring result(tmp ? tmp : L"");
        delete[] tmp;
        return result;
    }

    if (TV_VT(&value) == VTYPE_I4)
        return std::to_wstring(TV_I4(&value));

    throw std::runtime_error("Unsupported string parameter type");
}

//---------------------------------------------------------------------------//
int CAddInNative::variantToInt(const tVariant& value) const {
    if (TV_VT(&value) == VTYPE_I4)
        return TV_I4(&value);

    if (TV_VT(&value) == VTYPE_PWSTR && value.pwstrVal) {
        wchar_t* tmp = nullptr;
        ::convFromShortWchar(&tmp, value.pwstrVal, value.wstrLen + 1);
        std::wstring result(tmp ? tmp : L"");
        delete[] tmp;
        return std::stoi(result);
    }

    throw std::runtime_error("Unsupported integer parameter type");
}