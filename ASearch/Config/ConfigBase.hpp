/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com

*/

#ifndef ALKALINE_CONFIG_BASE_HPP
#define ALKALINE_CONFIG_BASE_HPP

#include <platform/include.hpp>
#include <String/String.hpp>
#include <String/StringTable.hpp>
#include <HashTable/HashTable.hpp>
#include <Tree/XmlTree.hpp>
#include <String/GStrings.hpp>

typedef CVector<CString> CStringVector;

class CConfigBase : public CStringTable {    
    
protected:
    
    typedef enum {
        ccoDigit,
        ccoDigitPos,
        ccoDigitBound,
        ccoTranslateSet,
        ccoBool,
        ccoBoolInverted,
        ccoString,
        ccoVirtual,
        ccoStringPos,
        ccoArray,
        ccoEvalPair
    } CConfigOptionType;
    
    typedef struct {
        char * csName;
        CConfigOptionType ccoType;
        void * pOption;
        int lArg1;
        int lArg2;
        char * csDescription;
    } CConfigOption;    

protected:

    virtual void DumpVirtual(CConfigOption& ConfigurationOption) const;
    virtual void FinalizeVirtual(CConfigOption& ConfigurationOption);
    virtual void CreateConfigurationOptions(CConfigOption *, int);        
    virtual void CreateConfigurationOptions(void);    
    copy_property(CConfigOption *, pConfigurationOptions);
    property(int, nConfigurationOptions);
    
public:
    
    inline CString GetOption(const CString& Name) const { return GetValue(Name); }
    inline void SetOption(const CString& Name, const CString& Value) { Set(Name, Value); }
    
    void DumpOptions(void) const;    
    
    static bool BooleanEval(const CString& Value);
    static int TranslateSize(CString, int = 1000000);

    virtual void Finalize(void);	
    bool FinalizeDigitSetting(int& Value, const CString& Name, int = -1) const;
    bool FinalizeDigitValue(int& Value, const CString&, int UpperLimit, int Default) const;
    
    CConfigBase(void);
    CConfigBase(const CConfigBase&);
    virtual ~CConfigBase(void);
    virtual CConfigBase& Copy(const CConfigBase& Other);
};

#endif
