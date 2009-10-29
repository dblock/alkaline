#include <platform/include.hpp>
#include <Vector/SVector.hpp>
#include <Vector/Vector.hpp>
#include <Search/SearchTypes.hpp>
#include <SwapVector/SwapVector.hpp>

#ifndef ALKALINE_C_RANK_HPP
#define ALKALINE_C_RANK_HPP

class CRank 
{
public:
    CRank();
    void GetSortedVector(CSearchData *SearchData);
    void GetSortedSwapVector(CSearchData *SearchData);
    void AddVector(CSVector *VectorToAdd, const short int Weight, const bool Mandatory, const bool ShouldBeMissing);
    void AddVector(CSwapVector *VectorToAdd, const short int Weight, const bool Mandatory, const bool ShouldBeMissing);
    void Clear();
    short int GetMaxPossibleQuality();
    void ShowVectors();
    void QuickSort(const int l, const int r);    
private:        
    bool m_bMandatoryWordMissing;    
    CVector<CSVector*> m_VectorsToSort;
    CVector<CSwapVector*> m_SwapVectorsToSort;
    CVector<short int> m_Weight;
    CVector<short int> *m_TempResult;
    CVector<int> *m_Result;
    int m_ProbableFinalSize;
    unsigned short int m_MinValue;
};

#endif
