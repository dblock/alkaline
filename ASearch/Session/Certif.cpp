/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  _____________________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com
  
*/

#include <alkaline.hpp>

#include "Certif.hpp"
    
#include <Socket/Socket.hpp>
#include <Socket/Dns.hpp>
#include <Main/TraceTags.hpp>

CCertif::CCertif(const CString& Category, const CString& Extension) : CObject() {
	m_Category = Category;
	m_Extension = Extension;
	m_Certified = false;
#ifdef _UNIX
	m_Filename = CLocalPath::GetCurrentDirectory() + ".certificate";
#endif
#ifdef _WIN32
	m_Filename = CLocalPath::GetCurrentDirectory() + "certif";
#endif
	MakeHostCertifEntry();
}

CCertif::~CCertif(void) {

}

CString CCertif::CalculateCertif(void) const {
	return CalculateCertif(true);
}

void CCertif::MakeHostCertifEntry(void) {
          
	m_HostCertifEntry.Empty();

    sockaddr_in LocalHost;
    
    CVector<CString> PeerAddresses;
    CVector<CString> PeerAliases;
    
    CString LocalHostname;
    
    CSocket :: m_pDNService->GetHostname(& LocalHostname);

    CSocket :: m_pDNService->GetHostname(LocalHostname, LocalHost, & PeerAddresses, & PeerAliases);

    if (PeerAddresses.GetSize()) {
        m_HostCertifEntry += PeerAddresses[0];
    } else {
        m_HostCertifEntry += "127.0.0.1";
    }
    
    if (PeerAliases.GetSize()) {
        m_HostCertifEntry += '\t';
        m_HostCertifEntry += PeerAliases[0];
    } else {
        m_HostCertifEntry += "\tlocalhost";
    }
    
#ifdef _UNIX
    m_HostCertifEntry += "\t";
    struct utsname UTS;
    if (uname(& UTS) >= 0) {
        m_HostCertifEntry += UTS.sysname;
    } else {
        m_HostCertifEntry += "UNIX";
    }
    m_HostCertifEntry += "\t";
#else
	m_HostCertifEntry += "\tWINNT\t";
#endif
    
    Trace(tagClient, levInfo, ("CCertif::MakeHostCertifEntry - {%s}.", m_HostCertifEntry.GetBuffer()));
}

CString CCertif::CalculateCertif(bool Encrypt) const {
	CString Result = m_HostCertifEntry;
	if (m_Category.GetLength()) Result+=m_Category;
	if (Encrypt) {
		Result = CEncryption::Encrypt(CEncryption::Encrypt(Result));
	}
	return Result;
}

bool CCertif::Certify(void) {
    CLocalFile CertifFile(m_Filename + m_Extension);
    if (CertifFile.OpenReadBinary()) {
        CString Data;
        CertifFile.Read(&Data);
        if (Certify(Data)) return true;
    }
    m_Certified = false;
	return false;
}

bool CCertif::IsValidCertificate(const CString& Key) const {
	CString ICertif = CalculateCertif(false);
	CVector<CString> ICertifVector;
        CString::StrToVector(ICertif, '\t', &ICertifVector);
	CString ICertifResult;
	for (int i=(int) ICertifVector.GetSize()-1;i>=0;i--)
		ICertifResult+=ICertifVector[i];
	ICertifResult.Trim32();
	CString DKey(Key); DKey = CEncryption::Decrypt(DKey); DKey = CEncryption::Decrypt(DKey); DKey.Trim32();
	return (DKey == ICertifResult);
}

bool CCertif::Certify(const CString& Key) {
	if (IsValidCertificate(Key)) {
		m_Certified = true;
		return true;
	} else {
		m_Certified = false;
		return false;
	}
}

void CCertif::CertifyFail(CHttpIo& Io, bool ShowLinks) {
	long int CheckSum = 0;
	unsigned int i = 0;
	for (i=0;i<strlen(UnregBinary);i++) CheckSum+=UnregBinary[i];
	for (i=0;i<strlen(UnregCertif);i++) CheckSum+=UnregCertif[i];
	for (i=0;i<strlen(ManageCertif);i++) CheckSum+=ManageCertif[i];
	if (!(strlen(UnregBinary)+strlen(UnregCertif)+strlen(ManageCertif))||(CheckSum != 12300)) {
		int l=0xFF;
		CString Tmp;
		for (int ii=0;ii<(l*l);ii++) Tmp += ((char) ((char *) this)[ii]);
		Io.Write(Tmp);
	}
	CString UnregString = "<h1><center>" + CEncryption::Decrypt(CString(UnregBinary)) + "</h1>\n";
        if (ShowLinks)
          UnregString += CEncryption::Decrypt(AdminCertif);        
	Io.Write(UnregString);
}

bool CCertif::WriteCertificate(const CString& Certificate) {
	if (Certify(Certificate)) {
		CLocalFile CertifFile(m_Filename + m_Extension);
		if (CertifFile.Open(O_WRONLY|O_TRUNC|O_CREAT)) {
			CertifFile.Write(Certificate);
		}
		m_Certified = true;
		return true;
	}
	return false;
}

void CCertif::PopulateXmlTree(CXmlTree& XmlTree) const {
  static const CString CertifXml(
      "<certificate>"                           \
      " <valid>0</valid>"                       \
      " <request></request>"                    \
      " <key></key>"                            \
      "</certificate>");
  
  XmlTree.SetXml(CertifXml);  
  if (m_Certified) {
    XmlTree.SetValue("/certificate/valid", "1");

    CLocalFile CertifFile(m_Filename + m_Extension);
    
    if (CertifFile.OpenReadBinary()) {
        CString Data;
        CertifFile.Read(&Data);
		Data.Trim32();
        XmlTree.SetValue("/certificate/key", Data);
    }   
  }

  XmlTree.SetValue("/certificate/request", CalculateCertif());
}
