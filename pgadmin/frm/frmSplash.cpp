//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2016, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// frmSplash.cpp - Splash Screen
//
//////////////////////////////////////////////////////////////////////////

// wxWindows headers
#include <wx/wx.h>
#include <wx/image.h>

// App headers
#include "pgAdmin3.h"
#include "frm/frmSplash.h"


// Copyright text
#include "copyright.h"
#include "version.h"

#include <wx/graphics.h>
#include "images/database.pngc"

BEGIN_EVENT_TABLE(frmSplash, wxFrame)
	EVT_PAINT(frmSplash::OnPaint)

#ifdef __WXGTK__
	EVT_WINDOW_CREATE(frmSplash::OnWindowCreate)
#endif
END_EVENT_TABLE()

frmSplash::frmSplash(wxFrame *parent)
#ifndef __WXDEBUG__
	: wxFrame((wxFrame *)NULL, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(100, 100), 0 | wxFRAME_SHAPED | wxSIMPLE_BORDER | wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP)
#else
	: wxFrame((wxFrame *)NULL, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(100, 100), 0 | wxFRAME_SHAPED | wxSIMPLE_BORDER | wxFRAME_NO_TASKBAR)
#endif
{
	appearanceFactory->SetIcons(this);
	if (appearanceFactory->IsBranded())
	{
		splash = appearanceFactory->GetSplashImage();
		SetClientSize(splash.GetWidth(), splash.GetHeight());
#ifndef __WXGTK__
		SetWindowShape();
#endif
	}
	else
		SetClientSize(pgModernSplashSize(this));

	CenterOnScreen();
}

void frmSplash::SetWindowShape()
{
	wxRegion region(splash);
	SetShape(region);
}

void frmSplash::OnPaint(wxPaintEvent &WXUNUSED(event))
{
	wxPoint pos = appearanceFactory->GetSplashTextPos();

	wxPaintDC dc(this);
	if (!appearanceFactory->IsBranded())
	{
		wxArrayString lines;
		lines.Add(VERSION_WITHOUT_DATE);
		lines.Add(COPYRIGHT);
		lines.Add(LICENSE);
		pgDrawModernSplash(this, dc, lines);
		return;
	}
	dc.DrawBitmap(splash, 0, 0, true);

	dc.SetTextForeground(appearanceFactory->GetSplashTextColour());
	dc.SetFont(appearanceFactory->GetSplashTextFont());

	if (appearanceFactory->IsBranded())
	{
		dc.DrawText(_("This program is based on pgAdmin III"), pos);
		pos.y += appearanceFactory->GetSplashTextOffset();
	}
	dc.DrawText(VERSION_WITHOUT_DATE, pos);
	pos.y += appearanceFactory->GetSplashTextOffset();
	dc.DrawText(COPYRIGHT, pos);
	pos.y += appearanceFactory->GetSplashTextOffset();
	dc.DrawText(LICENSE, pos);
}

void frmSplash::OnWindowCreate(wxWindowCreateEvent &WXUNUSED(evt))
{
	SetWindowShape();
}


wxSize pgModernSplashSize(wxWindow *win)
{
	return win->FromDIP(wxSize(540, 300));
}

// Modern splash: dark blue gradient, the database icon and the title drawn
// with antialiased text (no bitmap needed, sharp at any DPI).
void pgDrawModernSplash(wxWindow *win, wxDC &paintDC, const wxArrayString &lines)
{
	wxSize size = win->GetClientSize();
	wxGraphicsContext *gc = NULL;
	if (wxPaintDC *pdc = wxDynamicCast(&paintDC, wxPaintDC))
		gc = wxGraphicsContext::Create(*pdc);
	else if (wxWindowDC *wdc = wxDynamicCast(&paintDC, wxWindowDC))
		gc = wxGraphicsContext::Create(*wdc);
	if (!gc)
		return;

	// Background
	gc->SetBrush(gc->CreateLinearGradientBrush(0, 0, size.x, size.y,
	             wxColour(0x0F, 0x17, 0x2A), wxColour(0x1E, 0x3A, 0x8A)));
	gc->SetPen(*wxTRANSPARENT_PEN);
	gc->DrawRectangle(0, 0, size.x, size.y);

	// Soft decorative circles
	gc->SetBrush(wxBrush(wxColour(0x60, 0xA5, 0xFA, 28)));
	gc->DrawEllipse(size.x - win->FromDIP(230), -win->FromDIP(120), win->FromDIP(360), win->FromDIP(360));
	gc->SetBrush(wxBrush(wxColour(0x93, 0xC5, 0xFD, 18)));
	gc->DrawEllipse(size.x - win->FromDIP(140), win->FromDIP(150), win->FromDIP(220), win->FromDIP(220));

	// Database icon
	int iconSize = win->FromDIP(84);
	wxBitmap icon = database_png_bundle().GetBitmap(wxSize(iconSize, iconSize));
	int left = win->FromDIP(36), top = win->FromDIP(40);
	gc->DrawBitmap(icon, left, top, iconSize, iconSize);

	// Title and subtitle
	int textLeft = left + iconSize + win->FromDIP(20);
	gc->SetFont(wxFont(wxFontInfo(30).Bold().FaceName(wxT("Segoe UI"))), *wxWHITE);
	double tw, th;
	gc->GetTextExtent(wxT("pgAdmin III"), &tw, &th);
	gc->DrawText(wxT("pgAdmin III"), textLeft, top + win->FromDIP(4));
	gc->SetFont(wxFont(wxFontInfo(12).FaceName(wxT("Segoe UI"))), wxColour(0x93, 0xC5, 0xFD));
	gc->DrawText(wxString::FromUTF8("renovado  \xC2\xB7  PostgreSQL 13 \xE2\x80\x93 18"),
	             textLeft, top + th + win->FromDIP(4));

	// Separator and information lines at the bottom
	int y = size.y - win->FromDIP(28) - (int)lines.GetCount() * win->FromDIP(18);
	gc->SetPen(wxPen(wxColour(0xFF, 0xFF, 0xFF, 40), 1));
	gc->StrokeLine(left, y - win->FromDIP(12), size.x - left, y - win->FromDIP(12));
	gc->SetFont(wxFont(wxFontInfo(9).FaceName(wxT("Segoe UI"))), wxColour(0xCB, 0xD5, 0xE1));
	for (size_t i = 0; i < lines.GetCount(); i++)
		gc->DrawText(lines[i], left, y + (int)i * win->FromDIP(18));

	delete gc;
}
