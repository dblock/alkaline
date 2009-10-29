#include <Vector/SVector.hpp>
#include <Vector/Vector.hpp>
#include <SwapVector/SwapVector.hpp>


#ifndef CSORT_HPP
#define CSORT_HPP


class CSort;

class CSort
{
public:

CSort();
CVector<int> GetSortedVector();

CSVector GetSortedSVector();

CSVector GetSortedSwapVector();

void AddVector(CSVector* VectorToAdd);
void AddVector(CSwapVector* VectorToAdd);
void SetVectors(CVector<CSVector*>& AllVectors);
void SetVectors(CVector<CSwapVector*>& AllVectors);
void Clear();

void ShowVectors();
private:
CVector<CSVector*> m_VectorsToSort;
CVector<CSwapVector*> m_SwapVectorsToSort;


};

#endif
