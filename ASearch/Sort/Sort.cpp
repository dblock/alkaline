#include <alkaline.hpp>
#include "Sort.hpp"

CSort::CSort()
{

}

void CSort::ShowVectors()
{

    for(unsigned int i=0;i<m_VectorsToSort.GetSize();i++)
    {
        for(unsigned int j=0;j<(*(m_VectorsToSort[i])).GetSize();j++)
            cout<<(*(m_VectorsToSort[i]))[j]<<" ";
        cout<<endl<<endl;
    }

}

CVector<int> CSort::GetSortedVector()
{
    int Max=0;
    unsigned int i,j;
    CVector<int> TempResult;
    CVector<int> Result;
    
    for(i=0;i<m_VectorsToSort.GetSize();i++)
        if (CSABS((*(m_VectorsToSort[i]))[(*(m_VectorsToSort[i])).GetSize()-1]) > Max) 
            Max = CSABS((*(m_VectorsToSort[i]))[(*(m_VectorsToSort[i])).GetSize()-1]);

    // cout<<"MAX IS : "<<Max<<" Vector number is : "<<m_VectorsToSort.GetSize()<<endl;

    TempResult.SetSize(Max+1);

    for(i=0;i<m_VectorsToSort.GetSize();i++)
    {
        for(j=0;j<(*(m_VectorsToSort[i])).GetSize();j++)
            TempResult[CSABS((*(m_VectorsToSort[i]))[j])]=1;
    }

    Result.SetDim(Max,false);

    for(i=0;i<TempResult.GetSize();i++)
        if (TempResult[i]==1) Result+=i;
   

    return Result; 
}


CSVector CSort::GetSortedSVector()
{
    int Max=0;
    unsigned int i,j;
    CSVector TempResult;
    CSVector Result;
    
    for(i=0;i<m_VectorsToSort.GetSize();i++)
        if (CSABS((*(m_VectorsToSort[i]))[(*(m_VectorsToSort[i])).GetSize()-1]) > Max) 
            Max = CSABS((*(m_VectorsToSort[i]))[(*(m_VectorsToSort[i])).GetSize()-1]);

    // cout<<"MAX IS : "<<Max<<" Vector number is : "<<m_VectorsToSort.GetSize()<<endl;

    TempResult.SetSize(Max+1,true,false);

    for(i=0;i<m_VectorsToSort.GetSize();i++)
    {
        for(j=0;j<(*(m_VectorsToSort[i])).GetSize();j++)
            TempResult[CSABS((*(m_VectorsToSort[i]))[j])]=1;
    }

    Result.SetDim(Max,false);

    for(i=0;i<TempResult.GetSize();i++)
        if (TempResult[i]==1) Result.Append(i);
   

    return Result; 
}



CSVector CSort::GetSortedSwapVector()
{
    int Max=0;
    unsigned int i;
    CSVector TempResult;
    CSVector Result;
    
    for(i=0;i<m_SwapVectorsToSort.GetSize();i++)
    {
//		m_SwapVectorsToSort[i]->Display();
//		cout<<"Last Val: "<<CSABS(m_SwapVectorsToSort[i]->GetLastValue())<<endl;
        if (CSABS(m_SwapVectorsToSort[i]->GetLastValue()) > Max) 
            Max = CSABS(m_SwapVectorsToSort[i]->GetLastValue());
	}
//    cout<<"MAX IS : "<<Max<<" Vector number is : "<<m_VectorsToSort.GetSize()<<endl;

    TempResult.SetSize(Max+1,true,true);

    for(i=0;i<m_SwapVectorsToSort.GetSize();i++)
    {
	m_SwapVectorsToSort[i]->PopulateSortArray(TempResult);
    }


    Result.SetDim(Max,false);

    for(i=0;i<TempResult.GetSize();i++)
        if (TempResult[i]==1) Result.Append(i);
    return Result; 
}


void CSort::AddVector(CSVector* VectorToAdd)
{
	if (VectorToAdd!=NULL)
		if (VectorToAdd->GetSize()>0)
			m_VectorsToSort.Add(VectorToAdd);

}

void CSort::AddVector(CSwapVector* VectorToAdd)
{
	if (VectorToAdd!=NULL)
		if (VectorToAdd->GetSize()>0)
			m_SwapVectorsToSort.Add(VectorToAdd);

}


void CSort::SetVectors(CVector<CSVector*>& AllVectors)
{
	if (AllVectors.GetSize()>0)
	{
		m_VectorsToSort.SetDim(AllVectors.GetSize(),false);
		for(unsigned int i=0;i<AllVectors.GetSize();i++)
			if (AllVectors[i]->GetSize()>0)
				m_VectorsToSort+=AllVectors[i];
	}

}

void CSort::SetVectors(CVector<CSwapVector*>& AllVectors)
{
	if (AllVectors.GetSize()>0)
	{
		m_SwapVectorsToSort.SetDim(AllVectors.GetSize(),false);
		for(unsigned int i=0;i<AllVectors.GetSize();i++)
			if (AllVectors[i]->GetSize()>0)
				m_SwapVectorsToSort+=AllVectors[i];
	}

}

void CSort::Clear()
{
    m_VectorsToSort.RemoveAll();
	m_SwapVectorsToSort.RemoveAll();
}
