/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com

  broken from Index.cpp

*/

#include <alkaline.hpp>

#include "Index.hpp"
#include <Site/Site.hpp>
#include <Encryption/Encryption.hpp>
#include <Internet/EmailAddress.hpp>
#include <Encryption/Md5.hpp>
#include <AlkalineSession/AlkalineSession.hpp>
#include <File/LocalFile.hpp>
#include <Vector/IntVecIter.hpp>
#include <String/GStrings.hpp>
#include <Main/TraceTags.hpp>

bool CIndex::IsReservedKeyword(const CString& Word) {
    static const CString __SORT("SORT:");
    static const CString __HOST("HOST:");
    static const CString __PATH("PATH:");
    static const CString __URL("URL:");
    static const CString __EXT("EXT:");
    static const CString __OPT("OPT:");
    static const CString __BEFORE("BEFORE:");
    static const CString __AFTER("AFTER:");
    static const CString __QUANT("QUANT:");
    static const CString __META("META:");
    
    return
        Word.StartsWithSame(__BEFORE) ||
        Word.StartsWithSame(__AFTER) ||
        Word.StartsWithSame(__QUANT) ||
        Word.StartsWithSame(__EXT) ||
        Word.StartsWithSame(__OPT) ||
        Word.StartsWithSame(__URL) ||
        Word.StartsWithSame(__PATH) ||
        Word.StartsWithSame(__HOST) ||
        Word.StartsWithSame(__SORT) ||
        Word.StartsWithSame(__META);
}

void CIndex::AdjustMetas(CSearchObject& SearchObject) const {
    
    if (! SearchObject.m_SearchOptions.m_OptionMeta.GetSize())
        return;
    
    SearchObject.m_Adjustments.RemoveAll();
    
    int i, k;
    
    for (i=0;i<(int) SearchObject.m_SearchData.m_Words.GetSize();i++) {
        
        if (IsReservedKeyword(SearchObject.m_SearchData.m_Words[i])) 
            continue;
        
        // meta tag insertions
        
        if (SearchObject.m_SearchData.m_Words[i].Pos(':') == -1) {
            CString Word = SearchObject.m_SearchData.m_Words[i];
            
            // find the insertion point
            int MetaInsertPos = 0;
            while (MetaInsertPos < (int) Word.GetLength()) {
                if ((Word[MetaInsertPos] == '+') ||
                    (Word[MetaInsertPos] == '-') ||
					(Word[MetaInsertPos] == '\"')) {
						MetaInsertPos++;
						continue;				
					}
                break;
            }
            
            for (k = 0; k < (int) SearchObject.m_SearchOptions.m_OptionMeta.GetSize(); k++) {            
                if (! k) {
                    SearchObject.m_SearchData.m_Words[i].Insert(MetaInsertPos, ':');
                    SearchObject.m_SearchData.m_Words[i].Insert(MetaInsertPos, SearchObject.m_SearchOptions.m_OptionMeta[k]);
                } else {
                    CString WordAdded = Word;
                    WordAdded.Insert(MetaInsertPos, ':');
                    WordAdded.Insert(MetaInsertPos, SearchObject.m_SearchOptions.m_OptionMeta[k]);
                    SearchObject.m_Adjustments += WordAdded;
                }
            }
        }
    }

    SearchObject.m_SearchData.m_Words += SearchObject.m_Adjustments;
    SearchObject.m_Adjustments.RemoveAll();
}

void CIndex::AdjustKeywords(CSearchObject& SearchObject) const {

	SearchObject.m_SearchOptions.m_OptWhole = false;
	SearchObject.m_SearchOptions.m_OptAnd = false;
	SearchObject.m_SearchOptions.m_OptCase = false;
	SearchObject.m_SearchOptions.m_OptInsens = false;

    if (! SearchObject.m_SearchOptions.m_OptionExtra.GetSize()) 
        return;
    
    int i, j;

    static const CString __WHOLE("WHOLE");
    static const CString __AND("AND");
    static const CString __CASE("CASE");
	static const CString __INSENS("INSENS");

	// parse for global option settings
	for (j=0; j<(int) SearchObject.m_SearchOptions.m_OptionExtra.GetSize();j++) {
		if (SearchObject.m_SearchOptions.m_OptionExtra[j].Same(__WHOLE)) {
			SearchObject.m_SearchOptions.m_OptWhole = true;
		} else if (SearchObject.m_SearchOptions.m_OptionExtra[j].Same(__AND)) {
			SearchObject.m_SearchOptions.m_OptAnd = true;                
		} else if (SearchObject.m_SearchOptions.m_OptionExtra[j].Same(__CASE)) {
			SearchObject.m_SearchOptions.m_OptCase = true;                
		} else if (SearchObject.m_SearchOptions.m_OptionExtra[j].Same(__INSENS)) {
			SearchObject.m_SearchOptions.m_OptInsens = true;                
		}
	}

	// parse for each setting for every word
    for (i=0;i<(int) SearchObject.m_SearchData.m_Words.GetSize();i++) {
        
        if (IsReservedKeyword(SearchObject.m_SearchData.m_Words[i])) 
            continue;


        // adjust per opt:whole and opt:and

        for (j=0; j<(int) SearchObject.m_SearchOptions.m_OptionExtra.GetSize();j++) {
            if (SearchObject.m_SearchOptions.m_OptionExtra[j].Same(__WHOLE)) {
                SearchObject.m_SearchData.m_Words[i].Quote();
                if (SearchObject.m_SearchData.m_Words[i].GetLength() >= 2) {
                    /* fix, depending on order of words, quoting should not enquote a +/- option */
                    if ((SearchObject.m_SearchData.m_Words[i][1] == '+') ||
                        (SearchObject.m_SearchData.m_Words[i][1] == '-')) {
                        SearchObject.m_SearchData.m_Words[i][0] = SearchObject.m_SearchData.m_Words[i][1];
                        SearchObject.m_SearchData.m_Words[i][1] = '\"';
                    }
                }
            } else if (SearchObject.m_SearchOptions.m_OptionExtra[j].Same(__AND)) {
                int cPos = 0; while (((int) SearchObject.m_SearchData.m_Words[i].GetLength() > cPos) &&
                    (SearchObject.m_SearchData.m_Words[i][cPos] == '\"')) cPos++;
                if (((int) SearchObject.m_SearchData.m_Words[i].GetLength() > cPos) &&
                    (SearchObject.m_SearchData.m_Words[i][cPos] != '+') && (SearchObject.m_SearchData.m_Words[i][cPos] != '-')) {
                    SearchObject.m_SearchData.m_Words[i].Insert(0, "+");
                }
            }
        }
    }

    for (i=0;i<(int) SearchObject.m_Adjustments.GetSize();i++) {
        
        if (IsReservedKeyword(SearchObject.m_Adjustments[i])) 
            continue;
        
        for (j=0;j<(int) SearchObject.m_SearchOptions.m_OptionExtra.GetSize();j++) {
            if (SearchObject.m_SearchOptions.m_OptionExtra[j].Same(__WHOLE)) {
                SearchObject.m_Adjustments[i].Quote();
            }
        }
    }
    
}

void CIndex::PrepareKeywords(CSearchObject& SearchObject) const {
    
    Trace(tagSearch, levVerbose, ("CIndex::PrepareKeywords - entering."));
    
    CVector<CString> TmpVector;
    int v, i;
    
    static const CString __SORT("SORT:");
    static const CString __HOST("HOST:");
    static const CString __PATH("PATH:");
    static const CString __URL("URL:");
    static const CString __EXT("EXT:");
    static const CString __OPT("OPT:");
    static const CString __CASE("CASE");
    static const CString __INSENS("INSENS");
    static const CString __BEFORE("BEFORE:");
    static const CString __AFTER("AFTER:");
    static const CString __QUANT("QUANT:");
    static const CString __META("META:");
    
    SearchObject.m_SearchData.m_SearchCaseType = sct_CaseAutomatic;
    SearchObject.m_ParsedSearchString.Empty();
    
    CString MidString;
    
    for (i=(int) SearchObject.m_SearchData.m_Words.GetSize()-1;i>=0;i--){
        if (SearchObject.m_SearchData.m_Words[i].StartsWithSame(__SORT)) {
            SearchObject.m_SearchData.m_Words[i].Mid(__SORT.GetLength(), SearchObject.m_SearchData.m_Words[i].GetLength(), &SearchObject.m_SortType);
            SearchObject.m_SearchData.m_Words[i].Empty();
        } else if (SearchObject.m_SearchData.m_Words[i].StartsWithSame(__HOST)) {
            SearchObject.m_SearchData.m_Words[i].Mid(__HOST.GetLength(), SearchObject.m_SearchData.m_Words[i].GetLength(), &MidString);
            CString::StrToVector(MidString, ',', &TmpVector);
			for (v=0;v<(int) TmpVector.GetSize();v++) if (TmpVector[v].GetLength()) SearchObject.m_SearchOptions.m_OptionHosts += CUrl::UnEscape(TmpVector[v]);
            SearchObject.m_SearchData.m_Words[i].Empty();
        } else if (SearchObject.m_SearchData.m_Words[i].StartsWithSame(__PATH)) {
            SearchObject.m_SearchData.m_Words[i].Mid(__PATH.GetLength(), SearchObject.m_SearchData.m_Words[i].GetLength(), &MidString);
            CString::StrToVector(MidString, ',', &TmpVector);
            for (v=0;v<(int) TmpVector.GetSize();v++) if (TmpVector[v].GetLength()) SearchObject.m_SearchOptions.m_OptionPaths += CUrl::UnEscape(TmpVector[v]);
            SearchObject.m_SearchData.m_Words[i].Empty();
        } else if (SearchObject.m_SearchData.m_Words[i].StartsWithSame(__URL)) {
            SearchObject.m_SearchData.m_Words[i].Mid(__URL.GetLength(), SearchObject.m_SearchData.m_Words[i].GetLength(), &MidString);
            CString::StrToVector(MidString, ',', &TmpVector);
            for (v=0;v<(int) TmpVector.GetSize();v++) 
				if (TmpVector[v].GetLength()) {
					if (TmpVector[v].Pos("://") != -1) {
						SearchObject.m_SearchOptions.m_OptionUrls += CUrl::UnEscape(TmpVector[v]);
					} else {
						SearchObject.m_SearchOptions.m_OptionUrls += ("http://" + CUrl::UnEscape(TmpVector[v]));
					}
				}
            SearchObject.m_SearchData.m_Words[i].Empty();
        } else if (SearchObject.m_SearchData.m_Words[i].StartsWithSame(__EXT)) {
            SearchObject.m_SearchData.m_Words[i].Mid(__EXT.GetLength(), SearchObject.m_SearchData.m_Words[i].GetLength(), &MidString);
            CString::StrToVector(MidString, ',', &TmpVector);
            for (v=0;v<(int) TmpVector.GetSize();v++) if (TmpVector[v].GetLength()) {
                CString Tmp("."); Tmp += TmpVector[v];
                SearchObject.m_SearchOptions.m_OptionExts.AddSortedUnique(Tmp);
            }
            SearchObject.m_SearchData.m_Words[i].Empty();
        } else if (SearchObject.m_SearchData.m_Words[i].StartsWithSame(__OPT)) {
            SearchObject.m_SearchData.m_Words[i].Mid(__OPT.GetLength(), SearchObject.m_SearchData.m_Words[i].GetLength(), &MidString);
            CString::StrToVector(MidString, ',', &TmpVector);
            for (v=0;v<(int) TmpVector.GetSize();v++) if (TmpVector[v].GetLength()) {
                if (TmpVector[v].Same(__CASE)) {
                    SearchObject.m_SearchData.m_SearchCaseType = sct_CaseSensitive;
                } else if (TmpVector[v].Same(__INSENS)) {
                    SearchObject.m_SearchData.m_SearchCaseType = sct_CaseInsensitive;
                } else SearchObject.m_SearchOptions.m_OptionExtra += TmpVector[v];
            }
            SearchObject.m_SearchData.m_Words[i].Empty();
        } else if (SearchObject.m_SearchData.m_Words[i].StartsWithSame(__BEFORE)) {
            SearchObject.m_SearchData.m_Words[i].Mid(__BEFORE.GetLength(), SearchObject.m_SearchData.m_Words[i].GetLength(), &MidString);
            SearchObject.m_SearchOptions.m_DateBeforeValid = CDate::EncodeSimpleDate(MidString, SearchObject.m_SearchOptions.m_DateBefore, SearchObject.m_SearchOptions.m_USFormats);
            SearchObject.m_SearchData.m_Words.RemoveAt(i);
			continue;
        } else if (SearchObject.m_SearchData.m_Words[i].StartsWithSame(__AFTER)) {
            SearchObject.m_SearchData.m_Words[i].Mid(__AFTER.GetLength(), SearchObject.m_SearchData.m_Words[i].GetLength(), &MidString);
            SearchObject.m_SearchOptions.m_DateAfterValid = CDate::EncodeSimpleDate(MidString, SearchObject.m_SearchOptions.m_DateAfter, SearchObject.m_SearchOptions.m_USFormats);
            SearchObject.m_SearchData.m_Words.RemoveAt(i);
			continue;
        } else if (SearchObject.m_SearchData.m_Words[i].StartsWithSame(__META)) {
            SearchObject.m_SearchData.m_Words[i].Mid(__META.GetLength(), SearchObject.m_SearchData.m_Words[i].GetLength(), &MidString);
            CString::StrToVector(MidString, ',', &TmpVector);
            for (v=0;v<(int) TmpVector.GetSize();v++) 
                if (TmpVector[v].GetLength()) {
                    TmpVector[v].UpperCase();
                    SearchObject.m_SearchOptions.m_OptionMeta += TmpVector[v];
                }
            SearchObject.m_SearchData.m_Words.RemoveAt(i);
			continue;
        } else if (SearchObject.m_SearchData.m_Words[i].StartsWithSame(__QUANT)) {
            int QuantTmp;
            SearchObject.m_SearchData.m_Words[i].Mid(__QUANT.GetLength(), SearchObject.m_SearchData.m_Words[i].GetLength(), &MidString);
            if (MidString.IsInt(&QuantTmp)) {
                if (QuantTmp >= -1) SearchObject.m_Quant = QuantTmp;
            }
        } else {
            SearchObject.m_SearchData.m_Words[i].Trim32();

            if (SearchObject.m_ParsedSearchString.GetLength())
                SearchObject.m_ParsedSearchString.Insert(0, ' ');
            SearchObject.m_ParsedSearchString.Insert(0, SearchObject.m_SearchData.m_Words[i]);
        }

        if (! SearchObject.m_SearchData.m_Words[i].GetLength()) {
            SearchObject.m_SearchData.m_Words.RemoveAt(i);
        } else {
            CAlkalineParser::RemoveAccents(SearchObject.m_SearchData.m_Words[i], m_IndexOptions.m_FreeAlpha);
        }
    }

    // remove accents
	for (i = 0; i < (int) SearchObject.m_Adjustments.GetSize(); i++) {
        CAlkalineParser::RemoveAccents(
			SearchObject.m_Adjustments[i], 
			m_IndexOptions.m_FreeAlpha);
	}
    
    // adjust extra keyword options
    AdjustMetas(SearchObject);
    AdjustKeywords(SearchObject);    

    SearchObject.m_SearchData.m_Words += SearchObject.m_Adjustments;    
    SearchObject.m_SearchData.m_Words.QuickSortUnique();

	// digest search terms, those are lowercase query terms without quotes or markup for highlighting
	// meta tags, negations and alike are ignored
	CString::StrToVector(SearchObject.m_ParsedSearchString, ' ', & SearchObject.m_DigestedSearchTerms);
	for (i = 0; i < (int) SearchObject.m_DigestedSearchTerms.GetSize(); i++) {
		SearchObject.m_DigestedSearchTerms[i].Trim('+');
		SearchObject.m_DigestedSearchTerms[i].Trim('\"');
	}
    
    Trace(tagSearch, levVerbose, ("CIndex::PrepareKeywords - leaving."));
}

int CIndex::CompareResults(CSortType SortType, int First, int Second) {
    /* [last-modified][content-length][date][title][header][server] */
    int nResult = 0;
    switch (SortType) {
    case CSTDate:
        nResult = m_INFManager.GetTimeT(First) - m_INFManager.GetTimeT(Second);
        break;
    case CSTDateInverse:
        nResult = m_INFManager.GetTimeT(Second) - m_INFManager.GetTimeT(First);
        break;
    case CSTTitle:
        nResult = m_INFManager[First][3].CompareInsens(m_INFManager[Second][3]);
        break;
    case CSTUrl:
        nResult = m_URLManager.GetUrlTree().GetAt(First).CompareInsens(m_URLManager.GetUrlTree().GetAt(Second));
        break;
    case CSTSizeInverse:
        nResult = m_INFManager.GetDocumentSize(First) - m_INFManager.GetDocumentSize(Second);
        break;
    case CSTSize:
        nResult = m_INFManager.GetDocumentSize(Second) - m_INFManager.GetDocumentSize(First);
        break;
    default:
        if (First > Second) nResult = 1;
        else if (First < Second) nResult = -1;
        else nResult = 0;
    }
    return nResult;
}

void CIndex::SortResults(CSearchObject& SearchObject, CSortType SortType, int l, int r) {
    int i = l;
    int j = r;
    int pivot = SearchObject.m_SearchData.m_Results[(l+r)/2];
    while (i<=j) {
        while(CompareResults(SortType, SearchObject.m_SearchData.m_Results[i], pivot) < 0) i++;
        while(CompareResults(SortType, SearchObject.m_SearchData.m_Results[j], pivot) > 0) j--;
        if (i<=j){
            int t = SearchObject.m_SearchData.m_Results[i];
            int tt = SearchObject.m_SearchData.m_ResultsPositions[i];
            SearchObject.m_SearchData.m_Results[i] = SearchObject.m_SearchData.m_Results[j];
            SearchObject.m_SearchData.m_ResultsPositions[i] = SearchObject.m_SearchData.m_ResultsPositions[j];
            SearchObject.m_SearchData.m_Results[j] = t;
            SearchObject.m_SearchData.m_ResultsPositions[j] = tt;
            i++;
            j--;
        }
    }
    if (l < j) SortResults(SearchObject, SortType, l, j);
    if (i < r) SortResults(SearchObject, SortType, i, r);
}

void CIndex::SortResults(CSearchObject& SearchObject, const CString& SortType) {
    
    Trace(tagSearch, levVerbose, ("CIndex::SortResults - entering."));

    if (SearchObject.m_SearchData.m_Results.GetSize() == 0)
        return;
    
    if (SortType.Same("DATE")) {
        m_INFManager.StartReading();
        SortResults(SearchObject, CSTDate, 0, SearchObject.m_SearchData.m_Results.GetSize()-1);
        m_INFManager.StopReading();
    } else if (SortType.Same("IDATE")) {
        m_INFManager.StartReading();
        SortResults(SearchObject, CSTDateInverse, 0, SearchObject.m_SearchData.m_Results.GetSize()-1);
        m_INFManager.StopReading();
    } else if (SortType.Same("SIZE")) {
        m_INFManager.StartReading();
        SortResults(SearchObject, CSTSize, 0, SearchObject.m_SearchData.m_Results.GetSize()-1);
        m_INFManager.StopReading();    
    } else if (SortType.Same("ISIZE")) {
        m_INFManager.StartReading();        
        SortResults(SearchObject, CSTSizeInverse, 0, SearchObject.m_SearchData.m_Results.GetSize()-1);
        m_INFManager.StopReading();
    } else if (SortType.Same("TITLE")) {
        m_INFManager.StartReading();        
        SortResults(SearchObject, CSTTitle, 0, SearchObject.m_SearchData.m_Results.GetSize()-1);
        m_INFManager.StopReading();     
    } else if (SortType.Same("URL")) {
        m_URLManager.StartReading();        
        SortResults(SearchObject, CSTUrl, 0, SearchObject.m_SearchData.m_Results.GetSize()-1);
        m_URLManager.StopReading();
	} else if (SortType.Same("DOMAIN")) {
		SortResultsByDomain(SearchObject);
	}
    
    Trace(tagSearch, levVerbose, ("CIndex::SortResults - leaving."));
}

void CIndex::SortResultsByDomain(CSearchObject& SearchObject) {

	// domain sort
	// sort all per url
    m_URLManager.StartReading();        
    SortResults(SearchObject, CSTUrl, 0, SearchObject.m_SearchData.m_Results.GetSize()-1);

	if (! SearchObject.m_SearchData.m_Results.GetSize())
		return;
	
	// remove duplicates
	
	register unsigned long j = 0;	
	CString NextUrlString = m_URLManager.GetUrlTree().GetAt(SearchObject.m_SearchData.m_Results[j]);
	CUrl NextUrl(NextUrlString);
	// use quality to describe the number of results for this domain
	SearchObject.m_SearchData.m_ResultsQuality[j] = 1;
	for (register unsigned long i = 1; i < SearchObject.m_SearchData.m_Results.GetSize(); i++) {
		CString CurrentUrlString = m_URLManager.GetUrlTree().GetAt(SearchObject.m_SearchData.m_Results[i]);
		CUrl CurrentUrl(CurrentUrlString);

		if (CurrentUrl.GetHttpServer() != NextUrl.GetHttpServer()) {
			// write counter into result set			
			j++;
			if (j != i) {
				SearchObject.m_SearchData.m_Results[j] = SearchObject.m_SearchData.m_Results[i];
				SearchObject.m_SearchData.m_ResultsPositions[j] = SearchObject.m_SearchData.m_ResultsPositions[i];				
			}
			SearchObject.m_SearchData.m_ResultsQuality[j] = 1;			
			NextUrl = m_URLManager.GetUrlTree().GetAt(SearchObject.m_SearchData.m_Results[j]);
		} else {
			SearchObject.m_SearchData.m_ResultsQuality[j]++;			
		}
	}

	SearchObject.m_SearchData.m_Results.SetSize(j + 1);
	SearchObject.m_SearchData.m_ResultsPositions.SetSize(j + 1);
	SearchObject.m_SearchData.m_ResultsQuality.SetSize(j + 1);

	m_URLManager.StopReading();

	// patch each entry to point to domain results
}

bool CIndex::GetMostRecentDate(int PageUID, time_t& Date) const {    
    m_INFManager.StartReading();
    Date = m_INFManager.GetTimeT(PageUID);
    m_INFManager.StopReading();
    return (Date != 0);
}

void CIndex::PrepareResults(CSearchObject& SearchObject) const {
    
    Trace(tagSearch, levVerbose, ("CIndex::PrepareResults - entering."));
    
    /* assume host: entry */
    if (SearchObject.m_SearchOptions.m_DateBeforeValid || SearchObject.m_SearchOptions.m_DateAfterValid) {
        time_t Current, After, Before; 
        
        if (SearchObject.m_SearchOptions.m_DateBeforeValid) 
            Before = mktime(&SearchObject.m_SearchOptions.m_DateBefore); 
        else Before = 0;
        
        if (SearchObject.m_SearchOptions.m_DateAfterValid) 
            After = mktime(&SearchObject.m_SearchOptions.m_DateAfter); 
        else After = 0;

        Trace(tagSearch, levInfo, ("CIndex::PrepareResults - before:%d, after:%d", Before, After));
        
        After += (24 * 60 * 60);

        if (SearchObject.m_SearchData.m_Results.GetSize() != 0) {
            for (int i=(int) SearchObject.m_SearchData.m_Results.GetSize()-1;i>=0;i--) {
                if (GetMostRecentDate(SearchObject.m_SearchData.m_Results[i], Current)) {
                    
                    Trace(tagSearch, levInfo, ("CIndex::PrepareResults - result %d [%d]", i, Current));
                    
                    if (Before && (Current > Before)) {
                        
                        Trace(tagSearch, levInfo, ("CIndex::PrepareResults - removing because of before."));
                        
                        SearchObject.m_SearchData.m_Results.RemoveAt(i);
                        SearchObject.m_SearchData.m_ResultsPositions.RemoveAt(i);
                        continue;
                    }
                    
                    if (After && (Current < After)) {
                        
                        Trace(tagSearch, levInfo, ("CIndex::PrepareResults - removing because of after."));
                        
                        SearchObject.m_SearchData.m_Results.RemoveAt(i);
                        SearchObject.m_SearchData.m_ResultsPositions.RemoveAt(i);
                        continue;
                    }
                }
            }
        }
    }
    
    if (SearchObject.m_SearchOptions.m_OptionHosts.GetSize() ||
        SearchObject.m_SearchOptions.m_OptionPaths.GetSize() ||
        SearchObject.m_SearchOptions.m_OptionUrls.GetSize()) {
        
        /* at least one scope matching */
        
        int p;

        if (SearchObject.m_SearchData.m_Results.GetSize() != 0) {
            for (int i=(int) SearchObject.m_SearchData.m_Results.GetSize()-1;i>=0;i--) {
                
                int l_Found = 0;
                
                CUrl URLDigest(GetURLLink(SearchObject.m_SearchData.m_Results[i]));
                
                if (SearchObject.m_SearchOptions.m_OptionHosts.GetSize()) {
                    for (p=0;p<(int) SearchObject.m_SearchOptions.m_OptionHosts.GetSize();p++) {
						
						Trace(tagSearch, levInfo, ("CIndex::PrepareResults - url host: %s, restriction host: %s", URLDigest.GetHost().GetBuffer(), SearchObject.m_SearchOptions.m_OptionHosts[p].GetBuffer()));

                        if (URLDigest.GetHost().EndsWith(SearchObject.m_SearchOptions.m_OptionHosts[p])) {
                            l_Found = 1;
                            break;
                        }
                    }
                }
                
                if (!l_Found && SearchObject.m_SearchOptions.m_OptionPaths.GetSize())
                    for (p=0;p<(int) SearchObject.m_SearchOptions.m_OptionPaths.GetSize();p++) {

						Trace(tagSearch, levInfo, ("CIndex::PrepareResults - url path: %s, restriction path: %s", URLDigest.GetHttpPath().GetBuffer(), SearchObject.m_SearchOptions.m_OptionPaths[p].GetBuffer()));

                        if (URLDigest.GetHttpPath().StartsWith(SearchObject.m_SearchOptions.m_OptionPaths[p])) {
                            l_Found = 1;
                            break;
                        }
                    }
                    
                    if (!l_Found && SearchObject.m_SearchOptions.m_OptionUrls.GetSize())
                        for (p=0;p<(int) SearchObject.m_SearchOptions.m_OptionUrls.GetSize();p++) {

							Trace(tagSearch, levInfo, ("CIndex::PrepareResults - url brute: %s, restriction brute: %s", URLDigest.GetBrute().GetBuffer(), SearchObject.m_SearchOptions.m_OptionUrls[p].GetBuffer()));

                            if (URLDigest.GetBrute().StartsWith(SearchObject.m_SearchOptions.m_OptionUrls[p])) {
                                l_Found = 1;
                                break;
                            }
                        }
                        
                        if (l_Found && SearchObject.m_SearchOptions.m_OptionExts.GetSize()) {
                            l_Found = 0;
							for (p=0;p<(int) SearchObject.m_SearchOptions.m_OptionExts.GetSize();p++) {

								Trace(tagSearch, levInfo, ("CIndex::PrepareResults - url filename: %s, restriction extension: %s", URLDigest.GetHttpFilename().GetBuffer(), SearchObject.m_SearchOptions.m_OptionExts[p].GetBuffer()));

                                if (URLDigest.GetHttpFilename().EndsWith(SearchObject.m_SearchOptions.m_OptionExts[p])) {
                                    l_Found = 1;
                                    break;
                                }
							}
                        }
                        
                        if (!l_Found) {
                            SearchObject.m_SearchData.m_Results.RemoveAt(i);
                            SearchObject.m_SearchData.m_ResultsPositions.RemoveAt(i);
                        }
            }
        }
    } else if (SearchObject.m_SearchOptions.m_OptionExts.GetSize()) {
        if (SearchObject.m_SearchData.m_Results.GetSize() != 0) {
            for (int i=(int) SearchObject.m_SearchData.m_Results.GetSize()-1;i>=0;i--) {
                int l_Found = 0, p;
                CUrl URLDigest(GetURLLink(SearchObject.m_SearchData.m_Results[i]));
                for (p=0;p<(int) SearchObject.m_SearchOptions.m_OptionExts.GetSize();p++)
                    if (URLDigest.GetHttpFilename().EndsWith(SearchObject.m_SearchOptions.m_OptionExts[p])) {
                        l_Found = 1;
                        break;
                    }
                    if (!l_Found) {
                        SearchObject.m_SearchData.m_Results.RemoveAt(i);
                        SearchObject.m_SearchData.m_ResultsPositions.RemoveAt(i);
                    }
            }
        }
    }
    
    Trace(tagSearch, levVerbose, ("CIndex::PrepareResults - leaving."));
}

bool CIndex::Search(CSearchObject& SearchObject) {
    
    Trace(tagSearch, levVerbose, ("CIndex::Search - entering."));
    
    bool Result = false;
    SearchObject.m_SearchOptions.m_DateAfterValid = false;
    SearchObject.m_SearchOptions.m_DateBeforeValid = false;
    
    PrepareKeywords(SearchObject);
    
    // cout << "find:[" << SearchObject.m_SearchData.m_Words << "]" << endl;

    for (int n = 0; n < (int) SearchObject.m_SearchData.m_Words.GetSize(); n++) {
        Trace(tagSearch, levInfo, ("CIndex::Search - [%s]", SearchObject.m_SearchData.m_Words[n].GetBuffer()));
    }

    SearchObject.m_HasNext = 0;
	SearchObject.m_IndexSize = m_Searcher.GetAlreadyIndexedPages().CountBits(true);

    if (SearchObject.m_SearchData.m_Words.GetSize()) {
        
        Result = m_Searcher.Find(SearchObject.m_SearchData);   
        
        if (SearchObject.m_SearchData.m_Results.GetSize() != 0) {
            PrepareResults(SearchObject);
            SortResults(SearchObject, SearchObject.m_SortType);
            SearchObject.m_HasNext = SearchObject.m_SearchData.m_Results.GetSize();
            
            if (SearchObject.m_Quant == -1) {
                SearchObject.m_SearchResults.SetDim(SearchObject.m_SearchData.m_Results.GetSize());
            } else {
                SearchObject.m_SearchResults.SetDim(BASE_MAX(BASE_MIN(100, SearchObject.m_Quant), 10000));
            }

            Trace(tagSearch, levInfo, ("CIndex::Search - redim of displayable search results [%d]", SearchObject.m_SearchResults.GetDim()));

            time_t time_t_time; time(&time_t_time); struct tm tm_time;
            base_localtime(time_t_time, tm_time);
            int curDayCount = CDate::DayCount(tm_time.tm_mon, tm_time.tm_mday, tm_time.tm_year);
            /* structure passed for values */
            CIndexMapV IndexMapStructure;
            /* Index parent object */
            IndexMapStructure.m_Parent = SearchObject.m_Parent;
            /* get the string to map to */
            CString MapString = SearchObject.m_Parent->GetValue("MAP");
            /* get the map to use for the date */
            IndexMapStructure.m_DateMap = SearchObject.m_Parent->GetValue("DATE");
            struct tm fDate;
            /* get the ForceQuote option */
            IndexMapStructure.m_ForceQuote = CString::StrToInt(SearchObject.m_Parent->GetValue("QUOTE"));
            
            for (int l=SearchObject.m_Start; 
                 (l<(int) SearchObject.m_SearchData.m_Results.GetSize())&&
                 ((l<(SearchObject.m_Start+SearchObject.m_Quant) || (SearchObject.m_Quant < 0))
                 ); l++) {
                
                // Trace(tagSearch, levInfo, ("CIndex::Search - mapping result [%d]", l));
                
                int lResult = SearchObject.m_SearchData.m_Results[l];

                m_INFManager.StartReading();            
                int INFManagerSize = (int) m_INFManager.GetSize();
                m_INFManager.StopReading();
                
                m_URLManager.StartReading();
                int URLManagerSize = (int) m_URLManager.GetUrlTree().GetSize();
                m_URLManager.StopReading();
                
                if ((INFManagerSize > lResult)&&
                    (URLManagerSize > lResult)) {
                    
                    /* URL / Search Index */
                    IndexMapStructure.m_SearchData = &SearchObject.m_SearchData;
                    IndexMapStructure.m_ResultPosition = SearchObject.m_SearchData.m_ResultsPositions[l];
                    IndexMapStructure.m_Url = GetURLLink(lResult);

					// domain sort will attempt to patch the url to the topmost one
					if (SearchObject.m_SortType.Same("DOMAIN")) {
						CUrl DomainUrl(IndexMapStructure.m_Url);
						IndexMapStructure.m_Url = DomainUrl.GetHttpServer();
						IndexMapStructure.m_HashInfo.SetSize(6);
						IndexMapStructure.m_HashInfo[1] = "0";						
					} else {					
						m_INFManager.StartReading();
						IndexMapStructure.m_HashInfo = m_INFManager[lResult];
						m_INFManager.StopReading();
					}
					
                    /* replace a portion of the URL by something else if set from parent */
                    SearchObject.m_Parent->ProcessResultsURL(IndexMapStructure.m_Url);

                    Trace(tagSearch, levInfo, ("CIndex::Search - url [%s]", IndexMapStructure.m_Url.GetBuffer()));
                    
                    if (IndexMapStructure.m_HashInfo.GetSize() >= 6) {
    
                        /* results index */                        
                        IndexMapStructure.m_Index = l + 1;
                    
                        CreateDateObjects(
                            * SearchObject.m_Parent, 
                            IndexMapStructure,
                            fDate,
                            curDayCount);
                        
                        SearchObject.m_SearchResults += GetMapTerm(MapString, IndexMapStructure);                        
                    }
                }
            }
        }
    }
    
    Trace(tagSearch, levVerbose, ("CIndex::Search - leaving."));
    
    return Result;
}
