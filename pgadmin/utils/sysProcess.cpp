//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2016, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// sysProcess.cpp - External process
//
//////////////////////////////////////////////////////////////////////////


// wxWindows headers
#include <wx/wx.h>


// App headers
#include "pgAdmin3.h"
#include "utils/sysProcess.h"

#include <wx/filename.h>
#include <wx/file.h>
#include <wx/utils.h>


sysProcess::sysProcess(wxEvtHandler *evh, wxMBConv &conv)
	: wxProcess(evh), m_conv(conv)
{
	pid = 0;
	m_hasEnv = false;
	Redirect();
}


sysProcess::~sysProcess()
{
	if (!m_passFile.IsEmpty() && wxFileExists(m_passFile))
	{
		// Overwrite before removing, the file contained a password
		wxFile f(m_passFile, wxFile::write);
		if (f.IsOpened())
		{
			wxFileOffset len = f.Length();
			f.Seek(0);
			for (wxFileOffset i = 0; i < len + 64; i++)
				f.Write("x", 1);
			f.Close();
		}
		wxRemoveFile(m_passFile);
	}
}


sysProcess *sysProcess::Create(const wxString &exec, wxEvtHandler *evh, wxArrayString *env, wxMBConv &conv)
{
	sysProcess *proc = new sysProcess(evh, conv);
	if (env)
		proc->SetEnvironment(*env);

	if (!proc->Run(exec))
	{
		delete proc;
		proc = 0;
	}
	return proc;
}


bool sysProcess::Run(const wxString &exec)
{
	pid = wxExecute(exec, wxEXEC_ASYNC, this, m_hasEnv ? &m_env : NULL);
	return (pid != 0);
}


// The variables are given to the child process only: setting them with
// wxSetEnv() left PGPASSWORD in pgAdmin's environment, where every process
// started afterwards (for any server) inherited it. PGPASSWORD itself is
// replaced by a private temporary password file (PGPASSFILE), as the
// PostgreSQL documentation recommends: environment variables of a process
// can be read by other processes of the same user.
void sysProcess::SetEnvironment(const wxArrayString &environment)
{
	if (!m_hasEnv)
	{
		wxGetEnvMap(&m_env.env);
		m_hasEnv = true;
	}

	size_t i;
	for (i = 0 ; i < environment.GetCount() ; i++)
	{
		wxString str = environment.Item(i);
		wxString name = str.BeforeFirst('='), value = str.AfterFirst('=');
		if (name == wxT("PGPASSWORD"))
		{
			wxString passFile = wxFileName::CreateTempFileName(wxT("pgpass"));
			wxFile f(passFile, wxFile::write);
			if (!passFile.IsEmpty() && f.IsOpened())
			{
				// any host, port, database and user: hostname:port:db:user:password
				value.Replace(wxT("\\"), wxT("\\\\"));
				value.Replace(wxT(":"), wxT("\\:"));
				f.Write(wxT("*:*:*:*:") + value + wxT("\n"), wxConvUTF8);
				f.Close();
				m_passFile = passFile;
				m_env.env[wxT("PGPASSFILE")] = passFile;
				m_env.env.erase(wxT("PGPASSWORD"));
				continue;
			}
			// could not create the file: fall back to the variable
		}
		m_env.env[name] = value;
	}
}


void sysProcess::Abort()
{
	if (pid)
		wxKill(pid, wxSIGTERM);
}


wxString sysProcess::ReadInputStream()
{
	if (IsInputAvailable())
		return ReadStream(GetInputStream());
	return wxEmptyString;
}


wxString sysProcess::ReadErrorStream()
{
	if (IsErrorAvailable())
		return ReadStream(GetErrorStream());
	return wxEmptyString;
}

void sysProcess::WriteOutputStream(const wxString &out)
{
	// With wxEOL_DOS (=wxEOL_NATIVE in Windows) WriteString() will turn each '\n'
	//   into "\r\n", thus making "\r\n" a wrong "\r\r\n".
	// With wxEOL_UNIX it passes EOL characters as-is, which is preferable.
	wxTextOutputStream tos(*GetOutputStream(), wxEOL_UNIX);
	tos.WriteString(out);
}

wxString sysProcess::ReadStream(wxInputStream *input)
{
	wxString str;

	char buffer[1000 + 1];
	size_t size = 1;
	while (size && !input->Eof())
	{
		input->Read(buffer, sizeof(buffer) - 1);
		size = input->LastRead();
		if (size)
		{
			buffer[size] = 0;
			str.Append(wxString::Format(wxT("%s"), wxString(buffer, m_conv).c_str()));
		}
	}
	return str;
}

