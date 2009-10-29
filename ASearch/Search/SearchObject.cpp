/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com

*/

#include <alkaline.hpp>

#include "SearchObject.hpp"
#include <Main/TraceTags.hpp>
    
CSearchObject::CSearchObject(void) :
    m_Site(NULL),
    m_Parent(NULL),
    m_Quant(10),
    m_Start(0),
    m_HasNext(0),
    m_HasTime(0),
    m_IndexSize(0)
{
    
}

CSearchObject::~CSearchObject(void) {
    
}

void CSearchObject::ParseSearchString(bool bVerbose) {
    int curPos = 0;
    int prevPos = 0;
    int inQuote = 0;
    CString MetaCandidate;
    CString Candidate;
    CString MidValue;
    while (curPos < (int) m_SearchString.GetLength()) {        
        switch(m_SearchString[curPos]) {
        case ' ':
            m_SearchString.Mid(prevPos, curPos-prevPos, &MidValue);
            MidValue.Trim32();
            if (!inQuote && AddToVector(MidValue, m_SearchData.m_Words, m_Adjustments, MetaCandidate))
                prevPos = curPos+1;
            break;
        case '+':
        case '-':
            if (curPos) {                
                if (isalnum(m_SearchString[curPos-1]))
                    break;
                if ((m_SearchString[curPos-1] == '>') || 
                    (m_SearchString[curPos-1] == '<') ||
                    (m_SearchString[curPos-1] == '=')) 
                    break;
            }
            m_SearchString.Mid(prevPos, curPos-prevPos, &MidValue);
            MidValue.Trim32();
            if (!inQuote && AddToVector(MidValue, m_SearchData.m_Words, m_Adjustments, MetaCandidate))
                prevPos = curPos;
            break;
        case '\"':
            if (inQuote) {
                m_SearchString.Mid(prevPos, curPos-prevPos+1, &MidValue);
                MidValue.Trim32();
                if (AddToVector(MidValue, m_SearchData.m_Words, m_Adjustments, MetaCandidate)) {
                    prevPos = curPos + 1;
                    inQuote = 0;
                }
            } else inQuote = 1;
            break;
        case ':':
            if (!inQuote) {
                m_SearchString.Mid(prevPos, curPos-prevPos, &MidValue);
                MidValue.Trim32();
                MetaCandidate += MidValue;
                MetaCandidate += ':';
                prevPos = curPos+1;
            }
        }
        curPos++;
    }    

    m_SearchString.Mid(prevPos, curPos-prevPos, &MidValue);
    MidValue.Trim32();
    AddToVector(MidValue, m_SearchData.m_Words, m_Adjustments, MetaCandidate);
    
    int j;
    
    if (bVerbose) {
        // this is just for output
        CString::VectorToStr(m_SearchData.m_Words, ' ', & m_ParsedSearchString);
        cout << " - mapping {" << m_ParsedSearchString << "}";        
    }

    m_SearchData.m_SearchedWords = m_SearchData.m_Words.GetSize();
    
    for (j=(int) (m_Adjustments.GetSize()-1); j >= 0; j--)
        if (m_SearchData.m_Words.Contains(m_Adjustments[j])) 
            m_Adjustments-=j;
    
    for (j=0;j<(int) m_SearchData.m_Words.GetSize();j++) {
         Trace(tagSearch, levInfo, ("CSearchObject::ParseSearchString - word [%s]", m_SearchData.m_Words[j].GetBuffer()));
    }

    for (j=0;j<(int) m_Adjustments.GetSize();j++) {
         Trace(tagSearch, levInfo, ("CSearchObject::ParseSearchString - adjs [%s]", m_Adjustments[j].GetBuffer()));
    }

    if (bVerbose)
        cout.flush();    
}
    
bool CSearchObject::AddToVector(
    const CString& Candidate, 
    CVector<CString>& Target, 
    CVector<CString>& AdjustTarget, 
    CString& MetaCandidate) {
    
    
    if (Candidate.GetLength()) {
        CVector<CString> CandidateVector;
        CString::StrToVector(Candidate, ' ', &CandidateVector);
        CString MidValue;
        for (int k=0;k<(int) CandidateVector.GetSize();k++)
            if (CandidateVector[k].GetLength()) {
                if (MetaCandidate.GetLength()) {
                    
                    CString NewResult;
                    int i=0;
                    while ((i < (int) CandidateVector[k].GetLength()) &&
                           ((CandidateVector[k][i] == '+') || 
                            (CandidateVector[k][i] == '\"') || 
                            (CandidateVector[k][i] == '-'))) {
                        NewResult += CandidateVector[k][i];
                        i++;
                    }
                    // there's a special case for complex meta options (non-meta tags)
                    // for example url:specific:port 
                    if (MetaCandidate.GetCount(':') == 1)
                        MetaCandidate.UpperCase();
                    NewResult += MetaCandidate;
                    CandidateVector[k].Mid(i, CandidateVector[k].GetLength(), &MidValue);
                    NewResult += MidValue;
                    // handle "+, should become +"
                    if ((NewResult.GetLength() >= 1) &&
                        (NewResult[0] == '\"') &&
                        ((NewResult[1] == '+') || 
                         (NewResult[1] == '-'))) {
                        NewResult[0] = NewResult[1];
                        NewResult[1] = '\"';
                    }
                    Target += NewResult;
                
                } else {
                    
                    Target += CandidateVector[k];

                    // do not add special metas for a meta tag
                    bool bMetaTag = false;
                    bool bNumericTag = false;
                    bool bStarTag = ((CandidateVector[k].GetLength() == 1) && (CandidateVector[k][0] == '*'));
                    
                    // meta tag is foo:bar
                    // numeric tag is foo:[-]num
                    int p = 0;
                    while (p < (int) CandidateVector[k].GetLength() - 1) {
                                                    
                        switch(CandidateVector[k][p]) {
                        case ':':
                            
                            bMetaTag = true;
                            p = CandidateVector[k].GetLength();
                            break;
                            
                        case '=':
                        case '<':
                        case '>':                            
                            
                            p++;
                            
                            switch(CandidateVector[k][p]) {
                            case '-':
                            case '+':
                                if (p < (int) CandidateVector[k].GetLength() - 1)
                                    p++;
                            }
                            
                            if (isdigit(CandidateVector[k][p])) {
                                bNumericTag = true;
                                p = CandidateVector[k].GetLength();
                            }
                            
                            break;
                            
                        default:
                            p++;
                        }
                    }
    
                    if (!bMetaTag && !bStarTag && !bNumericTag) {
                        
                        CString CandidateString(CandidateVector[k]);
                        
                        if (CandidateString.GetLength()) {
                            
                            int MetaInsertPos = 0;                        
                            
                            if (CandidateString[0] == '+') {
                                // +blah => +blah META:blah
                                CandidateString.Delete(0, 1);
                            } else if (CandidateString[0] == '-') {
                                // -blah => -blah -META:blah
                                MetaInsertPos++;
                            }
                            
                            // we might have a quote
                            if (CandidateString.GetLength() && (CandidateString[MetaInsertPos] == '\"')) {
                                MetaInsertPos++;
                            }
                            
                            // $(BUGBUG) this should be safe

                            if (MetaInsertPos < (int) CandidateString.GetLength()) {                            
    
                                CString CvkMetaTag;
    
                                CvkMetaTag = CandidateString;
                                CvkMetaTag.Insert(MetaInsertPos, "TITLE:"); 
                                AdjustTarget+=CvkMetaTag;
                                
                                CvkMetaTag = CandidateString;
                                CvkMetaTag.Insert(MetaInsertPos, "DESCRIPTION:"); 
                                AdjustTarget+=CvkMetaTag;
                                
                                CvkMetaTag = CandidateString;
                                CvkMetaTag.Insert(MetaInsertPos, "KEYWORDS:");
                                AdjustTarget+=CvkMetaTag;
                                
                            }
                            
                        }
                    }
                    
                }
            }
        MetaCandidate.Empty();
        return true;
    } else return false;
}
