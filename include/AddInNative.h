#ifndef __ADDINNATIVE_H__
#define __ADDINNATIVE_H__

#include <string>
#include <memory>
#include <mutex>

#include "ComponentBase.h"
#include "AddInDefBase.h"
#include "IMemoryManager.h"

class TcpClient;

class CAddInNative : public IComponentBase
{
public:
    enum Props
    {
        ePropErrorMsg = 0,
        ePropConnected,
        ePropQueueSize,
        eLastProp
    };

    enum Methods
    {
        eMethInitialize = 0,
        eMethSendCommand,
        eMethGetStatus,
        eMethDisconnect,
        eMethHasMessage,
        eMethPopMessage,
        eMethReceive,
        eMethGetVersion,
        eLastMethod
    };

    CAddInNative(void);
    virtual ~CAddInNative();

    virtual bool ADDIN_API Init(void*) override;
    virtual bool ADDIN_API setMemManager(void* mem) override;
    virtual long ADDIN_API GetInfo() override;
    virtual void ADDIN_API Done() override;

    virtual bool ADDIN_API RegisterExtensionAs(WCHAR_T**) override;
    virtual long ADDIN_API GetNProps() override;
    virtual long ADDIN_API FindProp(const WCHAR_T* wsPropName) override;
    virtual const WCHAR_T* ADDIN_API GetPropName(long lPropNum, long lPropAlias) override;
    virtual bool ADDIN_API GetPropVal(const long lPropNum, tVariant* pvarPropVal) override;
    virtual bool ADDIN_API SetPropVal(const long lPropNum, tVariant* varPropVal) override;
    virtual bool ADDIN_API IsPropReadable(const long lPropNum) override;
    virtual bool ADDIN_API IsPropWritable(const long lPropNum) override;

    virtual long ADDIN_API GetNMethods() override;
    virtual long ADDIN_API FindMethod(const WCHAR_T* wsMethodName) override;
    virtual const WCHAR_T* ADDIN_API GetMethodName(const long lMethodNum,
                                                   const long lMethodAlias) override;
    virtual long ADDIN_API GetNParams(const long lMethodNum) override;
    virtual bool ADDIN_API GetParamDefValue(const long lMethodNum, const long lParamNum,
                                            tVariant *pvarParamDefValue) override;
    virtual bool ADDIN_API HasRetVal(const long lMethodNum) override;
    virtual bool ADDIN_API CallAsProc(const long lMethodNum,
                                      tVariant* paParams, const long lSizeArray) override;
    virtual bool ADDIN_API CallAsFunc(const long lMethodNum,
                                      tVariant* pvarRetValue, tVariant* paParams,
                                      const long lSizeArray) override;

    virtual void ADDIN_API SetLocale(const WCHAR_T* loc) override;
    virtual void ADDIN_API SetUserInterfaceLanguageCode(const WCHAR_T* lang) override;

private:
    long findName(const wchar_t* names[], const wchar_t* name, const uint32_t size) const;

    void setLastError(const std::wstring& text);
    std::wstring getLastError() const;

    bool allocAndCopyWString(const wchar_t* src, WCHAR_T** out) const;
    bool returnWString(const std::wstring& value, tVariant* ret) const;
    void returnBool(bool value, tVariant* ret) const;
    void returnInt(int value, tVariant* ret) const;

    std::wstring variantToWString(const tVariant& value) const;
    int variantToInt(const tVariant& value) const;

private:
    IAddInDefBase* m_iConnect;
    IMemoryManager* m_iMemory;
    std::u16string m_userLang;

    std::unique_ptr<TcpClient> m_tcp;
    std::wstring m_errorMsg;
    mutable std::mutex m_errorMutex;
};

#endif // __ADDINNATIVE_H__