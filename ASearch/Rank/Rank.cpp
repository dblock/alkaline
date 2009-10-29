#include <alkaline.hpp>
#include "Rank.hpp"
#include <Search/Search.hpp>

#define MAX_INT 0x7FFFFFFF

#define FSHOW(X,Y) cout<<X<<endl;for(int z=0;z<Y.GetSize();z++) cout<<Y[z]<<" ";cout<<endl;

CRank::CRank()
{    
    m_ProbableFinalSize = 0;
    m_MinValue = 0;
    m_bMandatoryWordMissing = false;
}

void CRank::ShowVectors()
{
    
    for(unsigned int i=0;i<m_VectorsToSort.GetSize();i++)
    {
        for(unsigned int j=0;j<m_VectorsToSort[i]->GetSize();j++)
            cout<<(*m_VectorsToSort[i])[j]<<" ";
        cout<<endl<<endl;
    }
    
}

void CRank::GetSortedVector(CSearchData *SearchData)
{
    int Max=0;
    unsigned int i,j;
 
	if (true==m_bMandatoryWordMissing)
	{
		SearchData->m_Results.SetSize(0);
		SearchData->m_QualityThreshold=0;
		return;
	}

    m_Result = & (SearchData->m_Results);
    m_TempResult = & (SearchData->m_ResultsQuality);
    
    for(i=0;i<m_VectorsToSort.GetSize();i++) 
	{        
        int nLastVectorPosition = m_VectorsToSort[i]->GetSize() - 1;
        assert(nLastVectorPosition >= 0);
        if ( abs((*m_VectorsToSort[i])[nLastVectorPosition]) > Max) 
            Max = abs((*m_VectorsToSort[i])[nLastVectorPosition]);
    }
    m_TempResult->SetSize(Max + 1);
    m_TempResult->SetAll(0);    
    
    for(i=0;i<m_VectorsToSort.GetSize();i++)
        for(j=0;j<m_VectorsToSort[i]->GetSize();j++)
            (*m_TempResult)[abs((*m_VectorsToSort[i])[j])] += m_Weight[i];            
    
    (*m_Result).SetSize(Max + 1);
    
    int NewVectorCount = 0;
    
    for(i=0;i<m_TempResult->GetSize();i++)
    {
        if ((*m_TempResult)[i] > m_MinValue) 
		{
            (* m_Result)[i] = i;
            NewVectorCount++;
        } 
        else 
            (* m_Result)[i] = 0;            
    }

    QuickSort(0, Max);

    m_Result->SetSize(NewVectorCount);
    m_TempResult->SetSize(NewVectorCount);

    SearchData->m_QualityThreshold = m_MinValue;
}



void CRank::GetSortedSwapVector(CSearchData *SearchData)
{
    int Max=0;
    unsigned int i,j;
 
	if (true==m_bMandatoryWordMissing)
	{
		SearchData->m_Results.SetSize(0);
		SearchData->m_QualityThreshold=0;
		return;
	}

    m_Result = & (SearchData->m_Results);

    m_TempResult = & (SearchData->m_ResultsQuality);
    
    for(i=0;i<m_SwapVectorsToSort.GetSize();i++) 
	{        
        if ( abs(m_SwapVectorsToSort[i]->GetLastValue()) > Max) 
            Max = abs(m_SwapVectorsToSort[i]->GetLastValue());
    }
        
    m_TempResult->SetSize(Max + 1);
    m_TempResult->SetAll(0);    
    
    for(i=0;i<m_SwapVectorsToSort.GetSize();i++)
    {
        for(j=0;j<m_SwapVectorsToSort[i]->GetSize();j++)
			m_SwapVectorsToSort[i]->PopulateRankArray(*m_TempResult,m_Weight[i]);
    }

    (*m_Result).SetSize(Max + 1);
    
    int NewVectorCount = 0;
    
    for(i=0;i<m_TempResult->GetSize();i++)
    {
        if ((*m_TempResult)[i] > m_MinValue) 
		{
            (* m_Result)[i] = i;
            NewVectorCount++;
        } else 
		{
            (* m_Result)[i] = 0;            
        }
    }

     //FSHOW("TEMP2", (* m_TempResult));
        
    QuickSort(0, Max);

    m_Result->SetSize(NewVectorCount);
    m_TempResult->SetSize(NewVectorCount);

	//cout<<"MIN VALUE :"<<m_MinValue<<endl;
     //FSHOW("TEMP2", (* m_TempResult));
    // FSHOW("TEMP2", (* m_Result));

    SearchData->m_QualityThreshold = m_MinValue;
}



void CRank::QuickSort(const int l, const int r) 
{
  int i = l;
  int j = r;
  int pivot = (*m_TempResult)[(l+r)/2];  
  while (i<=j) {
    while(((*m_TempResult)[i]>pivot)&&(i <= r)) 
	{      
      i++;
    }
    while(((*m_TempResult)[j]<pivot)&&(j >= l)) 
	{      
      j--;
    }
        
    if (i<=j)
	{
      
      int t = (*m_TempResult)[i];
      (*m_TempResult)[i] = (*m_TempResult)[j];
      (*m_TempResult)[j] = t;

      t = (*m_Result)[i];
      (*m_Result)[i] = (*m_Result)[j];
      (*m_Result)[j] = t;

      i++;
      j--;
    }
  }
  if (l < j) QuickSort(l, j);
  if (i < r) QuickSort(i, r);
}

short int CRank::GetMaxPossibleQuality()    
{
    short int Value=0;
    for(unsigned int i=0;i<m_Weight.GetSize();i++)
        if (m_Weight[i]>0) 
            Value+=m_Weight[i];
        
        Value-=m_MinValue;
        return Value;
}

void CRank::AddVector(CSVector *VectorToAdd, const short int Weight, const bool Mandatory, const bool ShouldBeMissing)
{
    //printf("One vector added with value %d Mandatory : %d %d\n",Weight,Mandatory,ShouldBeMissing);
	//cout<<"Values are: "<<*VectorToAdd<<endl;
    if (VectorToAdd->GetSize())
    {
        m_VectorsToSort+=VectorToAdd;
        if (Mandatory)
        {
            m_Weight.Add(100+Weight);
            m_MinValue+=100;
        }
        else if (ShouldBeMissing)
        {
            m_Weight.Add(-500);
        }
        else
        {
            m_Weight.Add(Weight);
        }
        m_ProbableFinalSize+=VectorToAdd->GetSize();
    }
	else 
	{
		if (Mandatory)
			m_bMandatoryWordMissing=true;
	}
}


void CRank::AddVector(CSwapVector *VectorToAdd, const short int Weight, const bool Mandatory, const bool ShouldBeMissing)
{
    //printf("One vector added with value %d Mandatory : %d %d\n",Weight,Mandatory,ShouldBeMissing);
	//cout<<"Values are: "<<*VectorToAdd<<endl;
    if (VectorToAdd->GetSize())
    {
        m_SwapVectorsToSort+=VectorToAdd;
        if (Mandatory)
        {
            m_Weight.Add(100+Weight);
            m_MinValue+=100;
        }
        else if (ShouldBeMissing)
        {
            m_Weight.Add(-500);
        }
        else
        {
            m_Weight.Add(Weight);
        }
        m_ProbableFinalSize+=VectorToAdd->GetSize();
    }
	else 
	{
		if (Mandatory)
			m_bMandatoryWordMissing=true;
	}
}


void CRank::Clear()
{
    m_VectorsToSort.RemoveAll();	
    m_SwapVectorsToSort.RemoveAll();
    m_Weight.RemoveAll();   
    m_ProbableFinalSize=0;
    m_MinValue=0;
    m_bMandatoryWordMissing=false;        

}
