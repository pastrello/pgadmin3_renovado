//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2016, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// frmSplash.h - Splash Screen
//
//////////////////////////////////////////////////////////////////////////

#ifndef SPLASH_H
#define SPLASH_H

// Class declarations
class frmSplash : public wxFrame
{
public:
	frmSplash(wxFrame *parent);
	void OnPaint(wxPaintEvent &);

private:
	void SetWindowShape();
	void OnWindowCreate(wxWindowCreateEvent &WXUNUSED(evt));

	wxBitmap splash;
	DECLARE_EVENT_TABLE()
};

// Size and painting of the modern splash / About window, used when no
// branding.ini provides its own image. The lines are drawn at the bottom.
wxSize pgModernSplashSize(wxWindow *win);
void pgDrawModernSplash(wxWindow *win, wxDC &dc, const wxArrayString &lines);

#endif
