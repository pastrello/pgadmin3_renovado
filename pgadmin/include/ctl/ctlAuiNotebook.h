//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2016, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// ctlAuiNotebook.cpp - Custom AUI Notebook class
//
//////////////////////////////////////////////////////////////////////////

// The primary purpose of this class is to pass child focus events from
// the notebook to the parent window. This is the only way we can grab
// focus events from the page controls.

#ifndef CTLAUINOTEBOOK_H
#define CTLAUINOTEBOOK_H

// wxWindows headers
#include <wx/wx.h>
#include <wx/aui/auibook.h>

class ctlAuiNotebook : public wxAuiNotebook
{
public:
	ctlAuiNotebook(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxAUI_NB_DEFAULT_STYLE) :
		wxAuiNotebook(parent, id, pos, size, style)
	{
		// Flat, theme-coloured tabs instead of the old gradient ones
		SetArtProvider(new wxAuiFlatTabArt());
	}

protected:
	void OnChildFocus(wxChildFocusEvent &evt);

	DECLARE_EVENT_TABLE()
};

// Flat docking panes: no blue gradient captions, colours from the current
// (light or dark) theme. Call right after wxAuiManager::SetManagedWindow().
#include <wx/aui/framemanager.h>
#include <wx/aui/dockart.h>
#include <wx/settings.h>

inline void pgApplyAuiStyle(wxAuiManager &manager)
{
	wxAuiDockArt *art = manager.GetArtProvider();
	wxColour face = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
	wxColour text = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
	bool dark = wxSystemSettings::GetAppearance().IsDark();
	// active pane: a slightly lighter/darker band than the window face
	wxColour active = face.ChangeLightness(dark ? 125 : 92);
	wxColour inactive = face.ChangeLightness(dark ? 110 : 97);

	art->SetMetric(wxAUI_DOCKART_GRADIENT_TYPE, wxAUI_GRADIENT_NONE);
	art->SetColour(wxAUI_DOCKART_BACKGROUND_COLOUR, face);
	art->SetColour(wxAUI_DOCKART_SASH_COLOUR, face);
	art->SetColour(wxAUI_DOCKART_BORDER_COLOUR, face.ChangeLightness(dark ? 140 : 85));
	art->SetColour(wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR, active);
	art->SetColour(wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR, active);
	art->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR, inactive);
	art->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR, inactive);
	art->SetColour(wxAUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR, text);
	art->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR, wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
	art->SetMetric(wxAUI_DOCKART_CAPTION_SIZE, manager.GetManagedWindow()->FromDIP(24));
	art->SetMetric(wxAUI_DOCKART_SASH_SIZE, manager.GetManagedWindow()->FromDIP(5));
}

#endif

