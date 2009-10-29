/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  ______________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com
  
*/

#ifndef BASE_CERTIF_HPP
#define BASE_CERTIF_HPP

#include <platform/include.hpp>
#include <String/String.hpp>
#include <Encryption/Encryption.hpp>
#include <File/LocalFile.hpp>
#include <Tree/XmlTree.hpp>
#include <Io/HttpIo.hpp>

#define UnregBinary "AEINIAL@JBGE@FIHLG@CFGLMCDCMMAAALNHHIDAALEGIMOICIBBCKAGDHIHOOGEDHK@BHGKLDCMJAKMMN@"
#define UnregCertif "BFCEGENGAEEGKBIFMHCLCENJINMHBBMMJKEDJKIHGGKMKDILM@LI@LAKDILLIF"
#define ManageCertif "KCGJLHNMFOMDDO@KFEOFEFL@KBJA"
#define AdminCertif "OEAOLFJAOEOFIHFKNIAODOCNFKOGOH@LOCFDKBJCOKME@GDIHJCFFFKKEOJFIJBLIKENKFGNNJBEMNHBLDKOGCIDJENFB@GMBBGOOEHCIKGKD@OKFCJIAOENGGLJIABBFA@DGNFJO@DHKLICFCOBJKBDOLJEAIJGBELOLLALLIO@BCN@NMC@DB"

class CCertif : public CObject {
	readonly_property(CString, HostCertifEntry);
	property(CString, Extension);
	property(CString, Category);
	readonly_property(bool, Certified);
	readonly_property(CString, Filename);
	property(CString, ScriptName);
	property(CString, CertifyUrl);
private:
	void MakeHostCertifEntry(void);
public:
	void CertifyFail(CHttpIo&, bool = true);
	CCertif(const CString& = CString::EmptyCString, const CString& = CString::EmptyCString);
	virtual ~CCertif(void);
	bool Certify(void);
	bool Certify(const CString&);
	CString CalculateCertif(void) const;
	bool WriteCertificate(const CString&);
	CString CalculateCertif(bool Encrypt) const;
	bool IsValidCertificate(const CString& Key) const;
  
        void PopulateXmlTree(CXmlTree& XmlTree) const;
};

#endif
