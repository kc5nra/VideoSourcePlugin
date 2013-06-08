#pragma once

#include "OBSApi.h"

class VideoSourceConfig;

class VideoSourceConfigDialog
{
private:
    VideoSourceConfig *config;

public:
    HWND hwndPathOrUrl;
    HWND hwndWidth;
    HWND hwndHeight;
    HWND hwndVolume;
    HWND hwndStretch;
    HWND hwndIsAudioOutputToStream;
    HWND hwndIsAudioOutputToDevice;
    HWND hwndAudioOutputType;
    HWND hwndAudioOutputDevice;

public:
    VideoSourceConfigDialog(VideoSourceConfig *config);
    ~VideoSourceConfigDialog();

public:
    bool Show();

public:
    VideoSourceConfig *GetConfig() { return config; }
/*!
 * VideoSourceConfigDialog class declaration
 */

class VideoSourceConfigDialog: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( VideoSourceConfigDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    VideoSourceConfigDialog();
    VideoSourceConfigDialog( wxWindow* parent, wxWindowID id = SYMBOL_VIDEOSOURCECONFIGDIALOG_IDNAME, const wxString& caption = SYMBOL_VIDEOSOURCECONFIGDIALOG_TITLE, const wxPoint& pos = SYMBOL_VIDEOSOURCECONFIGDIALOG_POSITION, const wxSize& size = SYMBOL_VIDEOSOURCECONFIGDIALOG_SIZE, long style = SYMBOL_VIDEOSOURCECONFIGDIALOG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_VIDEOSOURCECONFIGDIALOG_IDNAME, const wxString& caption = SYMBOL_VIDEOSOURCECONFIGDIALOG_TITLE, const wxPoint& pos = SYMBOL_VIDEOSOURCECONFIGDIALOG_POSITION, const wxSize& size = SYMBOL_VIDEOSOURCECONFIGDIALOG_SIZE, long style = SYMBOL_VIDEOSOURCECONFIGDIALOG_STYLE );

    /// Destructor
    ~VideoSourceConfigDialog();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin VideoSourceConfigDialog event handler declarations

////@end VideoSourceConfigDialog event handler declarations

////@begin VideoSourceConfigDialog member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end VideoSourceConfigDialog member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin VideoSourceConfigDialog member variables
////@end VideoSourceConfigDialog member variables
};

};
