/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com

*/

#include <alkaline.hpp>

#include "ConfigBase.hpp"

CConfigBase :: CConfigBase(void) :
    m_pConfigurationOptions(NULL),
    m_nConfigurationOptions(0) 
{

}

CConfigBase :: CConfigBase(const CConfigBase& /* Other */) : 
    m_pConfigurationOptions(NULL),
    m_nConfigurationOptions(0) 
{

}

CConfigBase :: ~CConfigBase(void) {
    if (m_pConfigurationOptions)
        delete[] m_pConfigurationOptions;    
}

CConfigBase& CConfigBase :: Copy(const CConfigBase& Other) {
    CStringTable::operator=(Other);
    return * this;
}
    
bool CConfigBase :: BooleanEval(const CString& _Value) {
    CString Value(_Value); Value.Trim32();
    if (!Value.GetLength()) return false;
    if (Value.Same("Y")) return true;
    else if (Value.Same("N")) return false;
    else if (CString::StrToInt(Value) == 1) return true;
    else if (Value.Same("YES")) return true;
    else if (Value.Same("NO")) return false;
    else return false;
}

int CConfigBase :: TranslateSize(CString iStr, int Default) {
    if (iStr.GetLength()) {
        iStr.Trim32();
        int l_Multiplier = 1;
        if (iStr.EndsWith("MB")) {
            l_Multiplier = 1000000;
            iStr.Delete(iStr.GetLength() - strlen("MB"), iStr.GetLength());
            iStr.Trim32();
        } else if (iStr.EndsWith("M")) {
            l_Multiplier = 1000000;
            iStr.Delete(iStr.GetLength() - strlen("M"), iStr.GetLength());
            iStr.Trim32();
        } else if (iStr.EndsWith("KB")) {
            l_Multiplier = 1000;
            iStr.Delete(iStr.GetLength() - strlen("KB"), iStr.GetLength());
            iStr.Trim32();
        } else if (iStr.EndsWith("K")) {
            l_Multiplier = 1000;
            iStr.Delete(iStr.GetLength() - strlen("K"), iStr.GetLength());
            iStr.Trim32();
        }
	
        if (iStr.GetLength() && (iStr[0] == '-')) {
            l_Multiplier = - l_Multiplier;
            iStr.Delete(0, 1);
        }
        
        for (int i=0;i<(int) iStr.GetLength();i++) {
            if (!isdigit(iStr[i])) {
                cout << "  [ignoring invalid size setting: " << iStr << "]" << endl;
                return Default;
            }
        }
        
        if (iStr.GetLength()) {
            return (CString::StrToInt(iStr) * l_Multiplier);
        }  else {
            cout << "  [ignoring invalid size setting: " << iStr << "]" << endl;
            return Default;
        }
    } else return Default;
}

void CConfigBase :: DumpVirtual(CConfigOption& /* ConfigurationOption */) const {
    cout << "virtual unknown type";
}

void CConfigBase :: DumpOptions(void) const {
    for (int i = 0; i < m_nConfigurationOptions; i++) {        
        cout << m_pConfigurationOptions[i].csName << ": ";
        switch(m_pConfigurationOptions[i].ccoType) {    
            // simple digit with optional boundaries
        case ccoDigit:            
            cout << "number, default=" << * (int *) m_pConfigurationOptions[i].pOption;
            if (m_pConfigurationOptions[i].lArg1 != -1)
                cout << ", reset=" << m_pConfigurationOptions[i].lArg1;            
            break;            
        case ccoDigitBound:
            cout << "number, default=" << * (int *) m_pConfigurationOptions[i].pOption;
            cout << ", min=" << m_pConfigurationOptions[i].lArg1;
            cout << ", max=" << m_pConfigurationOptions[i].lArg2;
            break;            
            // positive digit with a default value
        case ccoDigitPos:
            cout << "positive number, default=" << * (int *) m_pConfigurationOptions[i].pOption;
            if (m_pConfigurationOptions[i].lArg1 != -1)
                cout << ", reset=" << m_pConfigurationOptions[i].lArg1;            
            break;            
        case ccoTranslateSet:
            cout << "size, default=" << * (int *) m_pConfigurationOptions[i].pOption;
            break;            
        case ccoBool:
            cout << "boolean, default=" << CString::BoolToStr(* (bool *) m_pConfigurationOptions[i].pOption);
            break;
        case ccoBoolInverted:
            // inverted boolean options depricated
            // cout << "inverted boolean, default=" << CString::BoolToStr(!(* (bool *) m_pConfigurationOptions[i].pOption));    
            break;
        case ccoVirtual:
            DumpVirtual(m_pConfigurationOptions[i]);
            break;
        case ccoString:
            cout << "string, default=" << 
                ((* (CString *) m_pConfigurationOptions[i].pOption).GetLength() ? 
                 (* (CString *) m_pConfigurationOptions[i].pOption).GetBuffer() : "(not set)");
            break;
        case ccoStringPos:
            cout << "non-empty string, default=" << 
                ((* (CString *) m_pConfigurationOptions[i].pOption).GetLength() ? 
                 (* (CString *) m_pConfigurationOptions[i].pOption).GetBuffer() : "(not set)");
            break;
        case ccoArray:
            cout << "array, default=";
            if (((CVector<CString> *) m_pConfigurationOptions[i].pOption)->GetSize()) {
                for (int j=0; j< (int) ((CVector<CString> *) m_pConfigurationOptions[i].pOption)->GetSize(); j++) {
                    if (j)
                        cout << ", ";
                    cout << (* (CVector<CString> *) m_pConfigurationOptions[i].pOption)[j];
                }
            } else cout << "(not set)";
            break;
        case ccoEvalPair:
            cout << "complex pair";
            break;
        }
        cout << endl << "  [" << m_pConfigurationOptions[i].csDescription << "]" << endl;
    }    
}
        
bool CConfigBase :: FinalizeDigitSetting(int& Value, const CString& Name, int Default) const {
    
    // cout << "FinalizeDigitSetting for " << Value << " / " << Name << endl;
    
    CString LocalValue(GetValue(Name));
    LocalValue.Trim32();
    if ((LocalValue.IsInt(&Value))&&(Value < Default)) {
        Value = Default;
        cout << "  [ignoring invalid " << Name << " setting: " << GetValue(Name) << ", defaulting to " << Value << "]" << endl;
        return false;
    }
    return true;
}

bool CConfigBase :: FinalizeDigitValue(int& Value, const CString& Setting, int LowerLimit, int Default) const {
    if (Value < LowerLimit) {
        Value = Default;
        cout << "  [adjusting " << Setting << "=" << Value << "]" << endl;
        return true;
    } else return false;
}

void CConfigBase :: CreateConfigurationOptions(void) {
    
}

void CConfigBase :: CreateConfigurationOptions(
    CConfigOption * LocalConfigurationOptions, 
    int nConfigurationOptions) {
    
    assert(m_pConfigurationOptions == NULL);
    
    if (! nConfigurationOptions)
        return;
    
    m_nConfigurationOptions = nConfigurationOptions;    
    m_pConfigurationOptions = new CConfigOption[m_nConfigurationOptions];
    
    for (int i = 0; i < m_nConfigurationOptions; i++) {
        m_pConfigurationOptions[i].csName = LocalConfigurationOptions[i].csName;
        m_pConfigurationOptions[i].ccoType = LocalConfigurationOptions[i].ccoType;
        m_pConfigurationOptions[i].pOption = LocalConfigurationOptions[i].pOption;
        m_pConfigurationOptions[i].lArg1 = LocalConfigurationOptions[i].lArg1;
        m_pConfigurationOptions[i].lArg2 = LocalConfigurationOptions[i].lArg2;
        m_pConfigurationOptions[i].csDescription = LocalConfigurationOptions[i].csDescription;
    }
}

void CConfigBase :: FinalizeVirtual(CConfigOption& /* ConfigurationOption */) {
    
}
    
void CConfigBase :: Finalize(void) {
    
    CString TmpString;
    
    for (int i = 0; i < m_nConfigurationOptions; i++) {
        
        // cout << "Looking at " << m_pConfigurationOptions[i].csName << endl;
        
        switch(m_pConfigurationOptions[i].ccoType) {    
            // simple digit with optional boundaries
        case ccoDigit:
    
            FinalizeDigitSetting(
                * (int *) (m_pConfigurationOptions[i].pOption), 
                m_pConfigurationOptions[i].csName,
                (int) m_pConfigurationOptions[i].lArg1);
            
            break;
            
        case ccoDigitBound:
            
            FinalizeDigitSetting(
                * (int *) m_pConfigurationOptions[i].pOption, 
                m_pConfigurationOptions[i].csName);
            
            if (m_pConfigurationOptions[i].lArg1 != m_pConfigurationOptions[i].lArg2) {            
                FinalizeDigitValue(
                    * (int *) m_pConfigurationOptions[i].pOption,
                    m_pConfigurationOptions[i].csName,
                    (int) m_pConfigurationOptions[i].lArg1,
                    (int) m_pConfigurationOptions[i].lArg2);
            }
            
            break;
            
            // positive digit with a default value
        case ccoDigitPos:
    
            FinalizeDigitSetting(
                * (int *) m_pConfigurationOptions[i].pOption, 
                m_pConfigurationOptions[i].csName);
            
            if (* (int *) m_pConfigurationOptions[i].pOption < 0) {
                * (int *) m_pConfigurationOptions[i].pOption = m_pConfigurationOptions[i].lArg1;
                cout << "  [adjusting " << 
                    m_pConfigurationOptions[i].csName << 
                    " to " << * (int *) m_pConfigurationOptions[i].pOption << "]" << endl;
            }
            
            break;
            
        case ccoTranslateSet:
            
            * (int *) m_pConfigurationOptions[i].pOption = TranslateSize(
                GetValue(m_pConfigurationOptions[i].csName),
                * (int *) m_pConfigurationOptions[i].pOption);
            
            Set(m_pConfigurationOptions[i].csName, CString::IntToStr(* (int *) m_pConfigurationOptions[i].pOption));
            
            break;
    
        case ccoBool:
                
            TmpString = GetValue(m_pConfigurationOptions[i].csName);

            TmpString.Trim32();
            
            if (TmpString.GetLength())
                * (bool *) m_pConfigurationOptions[i].pOption = BooleanEval(TmpString);
            
            break;
        case ccoBoolInverted:
    
            TmpString = GetValue(m_pConfigurationOptions[i].csName);
            TmpString.Trim32();
            
            if (TmpString.GetLength())
                * (bool *) m_pConfigurationOptions[i].pOption = ! BooleanEval(TmpString);
            
            break;

        case ccoVirtual:
    
            FinalizeVirtual(m_pConfigurationOptions[i]);
            break;
            
        case ccoString:
    
            (* (CString *) m_pConfigurationOptions[i].pOption) = GetValue(m_pConfigurationOptions[i].csName);
            (* (CString *) m_pConfigurationOptions[i].pOption).Trim32();
            
            break;

        case ccoStringPos:
        
            TmpString = GetValue(m_pConfigurationOptions[i].csName);
            TmpString.Trim32();
            
            if (TmpString.GetLength()) {
                (* (CString *) m_pConfigurationOptions[i].pOption) = TmpString;
            }
            
            break;
    
        case ccoArray:
            
            TmpString = GetValue(m_pConfigurationOptions[i].csName);
            TmpString.Trim32();
            
            if (TmpString.GetLength()) {

                CVector<CString> * pVector = (CVector<CString> *) m_pConfigurationOptions[i].pOption;

                CString::StrToVector(
                    TmpString,
                    ',',
                    pVector);                        

                if (pVector && pVector->GetSize()) {
                    for (register int j = (int) pVector->GetSize() - 1; j >= 0; j--) {
                        (* pVector)[j].Trim32();
                        if (! (* pVector)[j].GetLength()) {
                            pVector->RemoveAt(j);
                        }
                    }
                }
            }                     
            
            break;
    
        case ccoEvalPair:
            break;
        }
    }    
}
