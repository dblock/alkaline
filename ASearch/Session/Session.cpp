/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  ______________________________________________

  written by Daniel Doubrovkine - dblock@vestris.com

  Revision history:

  29.08.1999: server-side includes light implementation

*/

#include <alkaline.hpp>

#include "Session.hpp"

#include <File/LocalFile.hpp>
#include <Internet/HtmlTag.hpp>
#include <Internet/HtmlParser.hpp>
#include <File/RemoteFile.hpp>
#include <Mv4/EquivManager.hpp>
#include <Date/Date.hpp>

CMutex CSession::m_LogMutex;

CSession::CSession(void) : CStringTable() {
	m_SSIEnabled = true;
}

CSession::~CSession(void) {

}

void CSession::TraverseOptions(CHttpIo& /* HttpIo */, const CString& Buffer) {
	static const CString __DashDash("--");
	static const CString __LSet("<!--SET ");
	static const CString __LGet("<!--");
	static const CString __RSet("-->");
	CString Local(Buffer);
	CString Extract;
	
	CString Name;
	CString Value;
	
	Local.UpperCase();
	int left = Local.Pos(__LSet, 0);
	while ((left>=0)&&(left < (int) Local.GetLength())) {
		int right = Local.Pos(__RSet, left);
		if (right>=0) {
			left+=__LSet.GetLength();

			// check whether we have embedded tags
			int next_left = Local.Pos(__LGet, left);
			while ((next_left >= 0) && (right >= 0) && (next_left < right)) {
				right = Local.Pos(__RSet, right + __RSet.GetLength());
				next_left = Local.Pos(__LGet, next_left + __LGet.GetLength());
			}

			if (right == -1) {
				// unclosed tag, ignore this block
				continue;
			}

			Buffer.Mid(left, right - left, &Extract);			
			
			int dashPos = Extract.Pos(__DashDash);
			if (dashPos != -1) {
				Extract.Mid(0, dashPos, & Name);
				Extract.Mid(dashPos + 2, Extract.GetLength(), & Value);

				Set(Name, Value);
			}
			left = Local.Pos(__LSet, right);
			right+=__RSet.GetLength();
		} else left+=__LSet.GetLength();
	}
}

/*

  Server-side includes (Light)

  include [virtual=url][file=local]
  echo [var=name]
  fsize [list of files]
  fcreated [list of files]
  flastmod [list of files]
  exec [cmd=cmdline][cgi=cgiurl]

*/
void CSession::TraverseSSI(const CString SSIString, CHttpIo& HttpIo) {
	CHtmlTag SSITag;
        CString MidString;
        SSIString.Mid(1, SSIString.GetLength(), &MidString);
	CHtmlParser::ParseToken(MidString, SSITag);
	if (SSITag.GetName().Same("INCLUDE")) {
		/*
		  include [virtual=url][file=local]
		  warning: there's no verification of localness of
		  the path (this does not comply to the SSI standard)
		*/
		for (register int i=0;i<(int)SSITag.GetParameters().GetSize();i++) {
			if (SSITag.GetParameters().GetNameAt(i).Same("VIRTUAL")) {
				CRemoteFile RemoteFile(SSITag.GetParameters().GetValueAt(i));				
				if (m_Proxy.GetLength()) {
					RemoteFile.Get(m_Proxy); 
				} else {
					RemoteFile.Get();
				}
				if (RemoteFile.GetRStatusValue() == 200) HttpIo.Write(RemoteFile.GetRData());
				else {
					HttpIo.Write(
						"[Server error: SSI tag <b>&lt;!--#" + SSITag.GetName() + " " + SSITag.GetParameters().GetNameAt(i) + 
						"=\"" + SSITag.GetParameters().GetValueAt(i) + "\"--&gt; - " + 
						CString::IntToStr(RemoteFile.GetRStatusValue()) + " " + RemoteFile.GetHttpRequest().GetRStatusString() + " - " + RemoteFile.GetError() + "</b>]");
				}
			} else if (SSITag.GetParameters().GetNameAt(i).Same("FILE")) {
				CLocalFile LocalFile(SSITag.GetParameters().GetValueAt(i));                                
                                LocalFile.Read(&MidString);
				if (LocalFile.GetSize() > 0) 
                                  HttpIo.Write(MidString);
				else HttpIo.Write("[Server error: SSI tag <b>&lt;!--#" + SSITag.GetName() + " " + SSITag.GetParameters().GetNameAt(i) + "=\"" + SSITag.GetParameters().GetValueAt(i) + "\"--&gt; - file empty or does not exist</b>]");
			} else {
				HttpIo.Write("[Server error: unsupported SSI tag <b>&lt;!--#" + SSITag.GetName() + " " + SSITag.GetParameters().GetNameAt(i) + "=\"" + SSITag.GetParameters().GetValueAt(i) + "\"--&gt;</b>]");
			}
		}
	} else if (SSITag.GetName().Same("EXEC")) {
		/*
		  exec [cmd=cmdline][cgi=cgiurl]
		  warning: there's no localness verification for either
		  command lines or CGIs, but unlike include this
		  will output only text / * mime-format documents
		*/
		for (register int i=0;i<(int)SSITag.GetParameters().GetSize();i++) {
			if (SSITag.GetParameters().GetNameAt(i).Same("CMD")) {
				/*
				  under SSI standard, we must pass the following
				  environment variables to the newly spawned
				  process + all CGI stuff:
				  
				  DOCUMENT_NAME: The current filename.
				  DOCUMENT_URI: The virtual path to this document (such as /docs/tutorials/foo.shtml).
				  QUERY_STRING_UNESCAPED: The unescaped version of any search query the client sent, with all shell-special characters escaped with \.
				  DATE_LOCAL: The current date, local time zone. Subject to the timefmt parameter to the config command.
				  DATE_GMT: Same as DATE_LOCAL but in Greenwich mean time.
				  LAST_MODIFIED: The last modification date of the current document. Subject to timefmt like the others.
				  
				  the current implementation is minimal and
				  does not support this; a blocking process
				  might remain forever running, a process
				  showing modal windows might crash under NT
				  
				*/				
				FILE * PHandle;
				if ((PHandle = base_popen((const char *) SSITag.GetParameters().GetValueAt(i).GetBuffer(), "rt")) != NULL) {
					char Buffer[128];
					while (!feof(PHandle)) {
						if(fgets(Buffer, 128, PHandle) != NULL )
							HttpIo.Write(Buffer);
					}
					base_pclose(PHandle);
				} else HttpIo.Write("[Server error: SSI tag <b>&lt;!--#" + SSITag.GetName() + " " + SSITag.GetParameters().GetNameAt(i) + "=\"" + SSITag.GetParameters().GetValueAt(i) + "\"--&gt; - error spawning command/*</b>]");
			} else if (SSITag.GetParameters().GetNameAt(i).Same("CGI")) {
				CRemoteFile RemoteFile(SSITag.GetParameters().GetValueAt(i));				
				if (m_Proxy.GetLength()) RemoteFile.Get(m_Proxy); else RemoteFile.Get();
				if (RemoteFile.GetRStatusValue() == 200) {
					CString RemoteFileContentType = RemoteFile.GetHttpRequest().GetRFields().FindElement("Content-type").GetValue("Content-type");
					if ((RemoteFileContentType.GetLength() == 0)||(RemoteFileContentType.StartsWithSame("TEXT/"))) {
						HttpIo.Write(RemoteFile.GetRData());
					} else HttpIo.Write("[Server error: SSI tag <b>&lt;!--#" + SSITag.GetName() + " " + SSITag.GetParameters().GetNameAt(i) + "=\"" + SSITag.GetParameters().GetValueAt(i) + "\"--&gt; - Content-type is not text/*</b>]");
				} else HttpIo.Write("[Server error: SSI tag <b>&lt;!--#" + SSITag.GetName() + " " + SSITag.GetParameters().GetNameAt(i) + "=\"" + SSITag.GetParameters().GetValueAt(i) + "\"--&gt; - " + RemoteFile.GetError() + "</b>]");
			} else {
				HttpIo.Write("[Server error: unsupported SSI tag <b>&lt;!--#" + SSITag.GetName() + " " + SSITag.GetParameters().GetNameAt(i) + "=\"" + SSITag.GetParameters().GetValueAt(i) + "\"--&gt;</b>]");
			}
		}
	} else if (SSITag.GetName().Same("FSIZE")) {
		/* fsize [list of files] with Bytes, Kb, Mb, Gb formatting */
		for (register int i=0;i<(int)SSITag.GetParameters().GetSize();i++) {
			struct_stat Stat;
			if (CLocalFile::GetFileStat(SSITag.GetParameters().GetNameAt(i), Stat)) {
				HttpIo.Write(CString::BytesToStr(Stat.st_size));
			} else HttpIo.Write("[Server error: SSI tag <b>&lt;!--#" + SSITag.GetName() + " " + SSITag.GetParameters().GetNameAt(i) + "=\"" + SSITag.GetParameters().GetValueAt(i) + "\"--&gt; - file does not exist</b>]");
		}
	} else if (SSITag.GetName().Same("FCREATED")) {
		/* fcreated [list of files], no locale formatting */
		for (register int i=0;i<(int)SSITag.GetParameters().GetSize();i++) {
			struct_stat Stat;
			if (CLocalFile::GetFileStat(SSITag.GetParameters().GetNameAt(i), Stat)) {
				CString CurrentTime; CDate::ctime_r(&Stat.st_ctime, CurrentTime);
				HttpIo.Write(CurrentTime);
			} else HttpIo.Write("[Server error: SSI tag <b>&lt;!--#" + SSITag.GetName() + " " + SSITag.GetParameters().GetNameAt(i) + "=\"" + SSITag.GetParameters().GetValueAt(i) + "\"--&gt; - file does not exist</b>]");
		}
	} else if (SSITag.GetName().Same("FLASTMOD")) {
		/* flastmod [list of files], no locale formatting */
		for (register int i=0;i<(int)SSITag.GetParameters().GetSize();i++) {
			struct_stat Stat;
			if (CLocalFile::GetFileStat(SSITag.GetParameters().GetNameAt(i), Stat)) {
				CString CurrentTime; CDate::ctime_r(&Stat.st_mtime, CurrentTime);
				HttpIo.Write(CurrentTime);
			} else HttpIo.Write("[Server error: SSI tag <b>&lt;!--#" + SSITag.GetName() + " " + SSITag.GetParameters().GetNameAt(i) + "=\"" + SSITag.GetParameters().GetValueAt(i) + "\"--&gt; - file does not exist</b>]");
		}
	} else if (SSITag.GetName().Same("ECHO")) {
		/*
		  echo [var=envvar], not outputing (none) as Apache when
		  undefined environment variable
		*/
		for (register int i=0;i<(int)SSITag.GetParameters().GetSize();i++) {
			if (SSITag.GetParameters().GetNameAt(i).Same("VAR")) {
				HttpIo.Write(getenv((const char *) SSITag.GetParameters().GetValueAt(i).GetBuffer()));
			} else {
				HttpIo.Write("[Server error: unsupported SSI tag <b>&lt;!--#" + SSITag.GetName() + " " + SSITag.GetParameters().GetNameAt(i) + "=\"" + SSITag.GetParameters().GetValueAt(i) + "\"--&gt;</b>]");
			}
		}
	} else {
		HttpIo.Write("[Server error: unsupported SSI tag <b>&lt;!--#" + SSITag.GetName() + " ...--&gt;</b>]");
	}
}

int CSession::ExecuteTag(const CString& TagString, int nPos, CString& Result, CHttpIo& HttpIo) {
		
	static const CString __LComment("<!--");
	static const CString __RComment("-->");
	// see whether we have embedded tags

	Result.Empty();

	int nLeftComment = TagString.Pos(__LComment, nPos);

	// the input must start with a tag
	if (nLeftComment != nPos) {
		Result += "<font color=red>[missing opening comment]</font>";		
		return -1;
	}

	nLeftComment += __LComment.GetLength();
	int nRightComment = TagString.Pos(__RComment, nLeftComment);
	int nNextLeftComment = TagString.Pos(__LComment, nLeftComment);

	if (nRightComment < 0) {
		// no closing comment here
		Result += "<font color=red>[missing closing comment]</font>";
		return -1;
	}

	if (nNextLeftComment >= 0 && nNextLeftComment < nRightComment) {
		
		// embedded comment
		// get the left part of the current tag
		TagString.Mid(nLeftComment, nNextLeftComment - nLeftComment, & Result);

		// process the embedded tag, returns the position after the last right closing tag
		CString TagExecutePart;		
		nRightComment = ExecuteTag(TagString, nNextLeftComment, TagExecutePart, HttpIo);
		if (nRightComment == -1) {
			Result += "<font color=red>[missing closing comment]</font>";
			Result += TagExecutePart;
			return -1;
		}
		Result += TagExecutePart;
		int nNextRightComment = TagString.Pos(__RComment, nRightComment);
		if (nNextRightComment == -1) {
			Result += "<font color=red>[missing closing comment]</font>";
			return -1;
		}
		
		CString RightPart;
		TagString.Mid(nRightComment, nNextRightComment - nRightComment, & RightPart);
		Result += RightPart;

		nRightComment = nNextRightComment + __RComment.GetLength();

	} else {

		// not an embedded comment
		TagString.Mid(nLeftComment, nRightComment - nLeftComment, & Result);
		nRightComment += __RComment.GetLength();

	}

	// process this tag

	if (Result.GetLength()) {

		CString SavedIoBuffer;
		SavedIoBuffer.MoveFrom(HttpIo.GetBuffer());

		if (Result[0] == '#') {
			// save the IO buffer
			// server-side includes
			if (m_SSIEnabled) {
				TraverseSSI(Result, HttpIo);
			}
			Result.MoveFrom(HttpIo.GetBuffer());
		} else if (Result.StartsWithSame("SET")) {
			Result.Empty();
		} else {
			// save the IO buffer
			// Alkaline specific option			
			if (ProcessTag(HttpIo, Result)) {
				Result.MoveFrom(HttpIo.GetBuffer());
			} else {
				Result.Insert(0, __LComment);
				Result.Append(__RComment);
			}	
		}

		HttpIo.GetBuffer().MoveFrom(SavedIoBuffer);
	}

	return nRightComment;
}

void CSession::TraverseTags(CHttpIo& HttpIo, const CString& Buffer) {
	static const CString __LComment("<!--");
	static const CString __RComment("-->");
	int p_left = 0;
	int left = Buffer.Pos(__LComment, 0);
	CString MidString;	
	while (left >= 0) {

		// show the text before the <!--
		Buffer.Mid(p_left, left - p_left, &MidString);
		HttpIo.Write(MidString);
		
		int right = ExecuteTag(Buffer, left, MidString, HttpIo);

		HttpIo.Write(MidString);

		if (right == -1)
			break;

		p_left = right;
		left = Buffer.Pos(__LComment, right);
	}

	Buffer.Mid(p_left, Buffer.GetLength(), & MidString);
	HttpIo.Write(MidString);
}

bool CSession::ProcessTag(CHttpIo& /* HttpIo */, const CString& /* Tag */) {
	return false;
}

void CSession::TraversePost(CHttpIo& /* HttpIo */, const CString& /* Buffer */) {

}

CSession& CSession::Execute(CHttpIo& HttpIo, const CString& TemplateContents) {
	TraverseOptions(HttpIo, TemplateContents);

    if (HttpIo.GetRStatus() == 400) return * this;
	if (HttpIo.IsPost()) {
		TraversePost(HttpIo, TemplateContents);
		if (HttpIo.GetRStatus() == 400) return * this;
	}
	TraverseTags(HttpIo, TemplateContents);
	return * this;
}

CString& CSession::MapTermEach(CString& Term, int /* Dummy */) const {
	return Term;
}

CString CSession::Map(const CString& Source) const {	
	CString Result;
	MapTerm(Source, Result, 0);
	return Result;
}

void CSession::ErrorPage(const CString& Error, CHttpIo& HttpIo) const {
	HttpIo.SetRStatus(400);
	HttpIo.Write("<html>\n<head><title>Server Error</title></head>\n<body bgcolor=\"#FFFFFF\">\n" + Error + "\n</body></html>\n");
}

CString& CSession::MapTerm(const CString& Source, CString& Target, int Dummy) const {
	MAP_TERM_MACRO(Source, Target, MapTerm, MapTermEach, false, Dummy);
}

void CSession::WriteLog(const CString& LogFilename, const CString& String) const {
    if (!LogFilename.GetLength())
        return;
    
    CLocalFile LogStream(LogFilename);
    if (LogStream.Open(O_WRONLY | O_APPEND | O_CREAT)) {
        m_LogMutex.Lock();
        LogStream.WriteLine(String);
        m_LogMutex.UnLock();        
    }
    
}
