#include "VideoSourceConfigDialog.h"
#include "VideoSourcePlugin.h"
#include "WindowsHelper.h"
#include "resource.h"

#define HANDLE_DEFAULT default: return false

INT_PTR CALLBACK Config_DlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL Config_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void Config_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
INT_PTR CALLBACK Config_OnNotify(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void Config_OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
void Config_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
void Config_OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized);

VideoSourceConfigDialog::VideoSourceConfigDialog(VideoSourceConfig *config)
{
    this->config = config;
    
    playlistDropListener = new PlaylistDropListener(this);
    bDragging = false;
}

VideoSourceConfigDialog::~VideoSourceConfigDialog()
{
    delete playlistDropListener;
}

bool VideoSourceConfigDialog::Show()
{
    return DialogBoxParam(VideoSourcePlugin::hinstDLL, MAKEINTRESOURCE(IDD_VIDEOCONFIG), API->GetMainWindow(), Config_DlgProc, (LPARAM)this) == IDOK;
}

void VideoSourceConfigDialog::PlaylistFilesDropped(StringList &files)
{
    LVITEM item;
    item.mask       = LVIF_TEXT | LVIF_STATE;
    item.stateMask  = 0;
    item.iSubItem   = 0;
    item.state      = 0;

    int insertIndex = ListView_GetItemCount(hwndPlaylist);
    for(unsigned int i = 0; i < files.Num(); i++) {
        item.pszText = files[i];
        item.iItem = insertIndex++;
        ListView_InsertItem(hwndPlaylist, &item);
    }
}

INT_PTR CALLBACK Config_DlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
        HANDLE_MSG      (hwndDlg,   WM_INITDIALOG,  Config_OnInitDialog);
        HANDLE_MSG      (hwndDlg,   WM_COMMAND,     Config_OnCommand);
        HANDLE_MSG      (hwndDlg,   WM_LBUTTONUP,   Config_OnLButtonUp);
        HANDLE_MSG      (hwndDlg,   WM_MOUSEMOVE,   Config_OnMouseMove);
        HANDLE_MSG      (hwndDlg,   WM_ACTIVATE,    Config_OnActivate);

        case WM_NOTIFY: return Config_OnNotify(hwndDlg, msg, wParam, lParam);
        
        HANDLE_DEFAULT;	
    }
}

BOOL Config_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
    VideoSourceConfigDialog *_this = reinterpret_cast<VideoSourceConfigDialog *>(lParam);
    VideoSourceConfig *config = _this->GetConfig();
        
    LocalizeWindow(hwnd);
    
    _this->hwndWidth                    = GetDlgItem(hwnd, IDC_WIDTH);
    _this->hwndHeight                   = GetDlgItem(hwnd, IDC_HEIGHT);
    _this->hwndVolume                   = GetDlgItem(hwnd, IDC_VOLUME);
    _this->hwndStretch                  = GetDlgItem(hwnd, IDC_STRETCH);
    _this->hwndIsAudioOutputToStream    = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_TO_STREAM);
    _this->hwndIsAudioOutputToDevice    = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_TO_DEVICE);
    _this->hwndAudioOutputType          = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_TYPE);
    _this->hwndAudioOutputDevice        = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_DEVICE);
    _this->hwndMediaFileOrUrl           = GetDlgItem(hwnd, IDC_MEDIA_FILE_OR_URL);
    _this->hwndPlaylist                 = GetDlgItem(hwnd, IDC_PLAYLIST); 
    _this->hwndAddMedia                 = GetDlgItem(hwnd, IDC_ADD_MEDIA);
    _this->hwndRemoveMedia              = GetDlgItem(hwnd, IDC_REMOVE_MEDIA);

    _this->playlistDropTarget           = DropTarget::RegisterDropWindow(_this->hwndPlaylist, _this->playlistDropListener);

    Edit_SetText(_this->hwndWidth,      IntString(config->width).Array());
    Edit_SetText(_this->hwndHeight,     IntString(config->height).Array());

    Button_SetCheck(_this->hwndStretch, config->isStretching);
    Slider_SetRange(_this->hwndVolume, 0, 100);
    Slider_SetPos(_this->hwndVolume, config->volume);

    int index = -1;

    auto audioOutputTypes = config->GetAudioOutputTypes();

    for(auto i = audioOutputTypes.begin(); i < audioOutputTypes.end(); i++) {
        AudioOutputType &audioOutputType = *i;
        ComboBox_AddString(_this->hwndAudioOutputType, audioOutputType.GetDescription().Array());
        if (audioOutputType.GetName() == config->audioOutputType) {
            index = (int)(i - audioOutputTypes.begin());
        }
    }

    if (index < 0) {
        index = 0;                
    }
    
    ComboBox_SetCurSel(_this->hwndAudioOutputType, index);

    AudioOutputType &audioOutputType = config->GetAudioOutputTypes()[index];
    auto audioOutputDevices = audioOutputType.GetAudioOutputDevices();
        
    index = -1;

    if (audioOutputDevices.size()) {
        for(auto i = audioOutputDevices.begin(); i < audioOutputDevices.end(); i++) {
            AudioOutputDevice &audioOutputDevice = *i;
            ComboBox_AddString(_this->hwndAudioOutputDevice, audioOutputDevice.GetName().Array());
            if (audioOutputDevice.GetLongName() == config->audioOutputDevice) {
                index = (int)(i - audioOutputDevices.begin());
            }
        }
             
        if (index < 0) {
            index = 0;                
        }

        ComboBox_SetCurSel(_this->hwndAudioOutputDevice, index);
    }

    Button_SetCheck(_this->hwndIsAudioOutputToStream, config->isAudioOutputToStream);
    Button_SetCheck(_this->hwndIsAudioOutputToDevice, !config->isAudioOutputToStream);
    
    LVCOLUMN lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT ; 
    lvc.fmt = LVCFMT_LEFT;
    String playlistFile(STR("PlaylistFile"));
    lvc.pszText = playlistFile;
    lvc.cchTextMax = playlistFile.Length();
    lvc.iSubItem = 0;      
    lvc.iImage = 0;        
    lvc.iOrder = 0;        
    ListView_InsertColumn(_this->hwndPlaylist, 0, &lvc);

    _this->PlaylistFilesDropped(config->playlist);

    FORWARD_WM_COMMAND(
        hwnd, 
        config->isAudioOutputToStream ? IDC_AUDIO_OUTPUT_TO_STREAM : IDC_AUDIO_OUTPUT_TO_DEVICE,
        config->isAudioOutputToStream ? _this->hwndIsAudioOutputToStream : _this->hwndIsAudioOutputToDevice,
        BN_CLICKED,
        SendMessage);
    
    return true;
}

void Config_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(id)
    {
    case IDC_AUDIO_OUTPUT_TO_DEVICE:
    case IDC_AUDIO_OUTPUT_TO_STREAM:
        {
            if (codeNotify == BN_CLICKED) {
                VideoSourceConfigDialog *_this = (VideoSourceConfigDialog *)GetWindowLongPtr(hwnd, DWLP_USER);

                HWND hwndIsAudioOutputToStream = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_TO_STREAM);
                HWND hwndIsAudioOutputToDevice = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_TO_DEVICE);
                HWND hwndAudioOutputType = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_TYPE);
                HWND hwndAudioOutputDevice = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_DEVICE);

                bool isAudioOutputToStream = id == IDC_AUDIO_OUTPUT_TO_STREAM;

                EnableWindow(hwndAudioOutputType, !isAudioOutputToStream);
                EnableWindow(hwndAudioOutputDevice, !isAudioOutputToStream);
                    
                if (!isAudioOutputToStream) {
                    int index = ComboBox_GetCurSel(_this->hwndAudioOutputType);
                    if (index > 0) {
                        AudioOutputType &type = _this->GetConfig()->GetAudioOutputTypes()[index];
                        EnableWindow(hwndAudioOutputDevice, (type.GetAudioOutputDevices().size()) ? true : false);
                    }
                }
            }
        break;
        }
    case IDC_AUDIO_OUTPUT_TYPE:
        {
            if (codeNotify == CBN_SELCHANGE) {

                VideoSourceConfigDialog *_this = (VideoSourceConfigDialog *)GetWindowLongPtr(hwnd, DWLP_USER);

                int index = ComboBox_GetCurSel(_this->hwndAudioOutputType);

                ComboBox_ResetContent(_this->hwndAudioOutputDevice);

                AudioOutputType &audioOutputType = _this->GetConfig()->GetAudioOutputTypes()[index];
                auto audioOutputDevices = audioOutputType.GetAudioOutputDevices();
        
                index = -1;

                if (audioOutputDevices.size()) {
                    if (audioOutputDevices.size()) {
                        for(auto i = audioOutputDevices.begin(); i < audioOutputDevices.end(); i++) {
                            AudioOutputDevice &audioOutputDevice = *i;
                            ComboBox_AddString(_this->hwndAudioOutputDevice, audioOutputDevice.GetName().Array());
                            if (audioOutputDevice.GetLongName() == _this->GetConfig()->audioOutputDevice) {
                                index = (int)(i - audioOutputDevices.begin());
                            }
                        }
                    }        

                    if (index < 0) {
                        index = 0;                
                    }

                    ComboBox_SetCurSel(_this->hwndAudioOutputDevice, index);
                    EnableWindow(_this->hwndAudioOutputDevice, true);
                } else {
                    EnableWindow(_this->hwndAudioOutputDevice, false);
                }                    
            }
            break;
        }
    case IDC_ADD_MEDIA:
        {
            if (codeNotify == BN_CLICKED) {
                
                VideoSourceConfigDialog *_this = (VideoSourceConfigDialog *)GetWindowLongPtr(hwnd, DWLP_USER);

                StringList media;
                media.Add(GetEditText(_this->hwndMediaFileOrUrl));
                _this->PlaylistFilesDropped(media);
            }
            break;
        }
    case IDC_REMOVE_MEDIA:
        {
            if (codeNotify == BN_CLICKED) {
                
                VideoSourceConfigDialog *_this = (VideoSourceConfigDialog *)GetWindowLongPtr(hwnd, DWLP_USER);

                // Dropped item is selected?
                LVITEM lvi;
                lvi.iSubItem = 0;
                lvi.mask = LVIF_STATE;
                lvi.stateMask = LVIS_SELECTED;
                    
                int iPos = ListView_GetNextItem(_this->hwndPlaylist, -1, LVNI_SELECTED);
                while (iPos != -1) {
                    // Delete from original position
                    ListView_DeleteItem(_this->hwndPlaylist, iPos);
                    iPos = ListView_GetNextItem(_this->hwndPlaylist, -1, LVNI_SELECTED);
                }
            }
            break;
        }
    case IDC_BROWSE_MEDIA:
        {
            if (codeNotify == BN_CLICKED) {
                VideoSourceConfigDialog *_this = (VideoSourceConfigDialog *)GetWindowLongPtr(hwnd, DWLP_USER);
                
                
                //lololololol

                TSTR lpFile = (TSTR)Allocate(32*1024*sizeof(TCHAR));
                zero(lpFile, 32*1024*sizeof(TCHAR));

                OPENFILENAME ofn;
                zero(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.lpstrFile = lpFile;
                ofn.hwndOwner = hwnd;
                ofn.nMaxFile = 32*1024*sizeof(TCHAR);

                ofn.lpstrFilter = TEXT("VSP Supported Files (*.anm;*.asf;*.avi;*.bik;*.dts;*.dxa;*.flv;*.fli;*.flc;*.flx;*.h261;*.h263;*.h264;*.m4v;*.mkv;*.mjp;*.mlp;*.mov;*.mp4;*.3gp;*.3g2;*.mj2;*.mvi;*.pmp;*.rm;*.rmvb;*.rpl;*.smk;*.swf;*.vc1;*.wmv;*.ts;*.vob;*.mts;*.m2ts;*.m2t;*.mpg;*.mxf;*.ogm;*.qt;*.tp;*.dvr-ms;*.amv)\0*.anm;*.asf;*.avi;*.bik;*.dts;*.dxa;*.flv;*.fli;*.flc;*.flx;*.h261;*.h263;*.h264;*.m4v;*.mkv;*.mjp;*.mlp;*.mov;*.mp4;*.3gp;*.3g2;*.mj2;*.mvi;*.pmp;*.rm;*.rmvb;*.rpl;*.smk;*.swf;*.vc1;*.wmv;*.ts;*.vob;*.mts;*.m2ts;*.m2t;*.mpg;*.mxf;*.ogm;*.qt;*.tp;*.dvr-ms;*.amv\0");
                ofn.nFilterIndex = 1;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

                TCHAR curDirectory[MAX_PATH+1];
                GetCurrentDirectory(MAX_PATH, curDirectory);

                BOOL bOpenFile = GetOpenFileName(&ofn);

                TCHAR newDirectory[MAX_PATH+1];
                GetCurrentDirectory(MAX_PATH, newDirectory);

                SetCurrentDirectory(curDirectory);
                StringList files;
                if(bOpenFile)
                {
                    TSTR lpCurFile = lpFile+ofn.nFileOffset;

                    while(lpCurFile && *lpCurFile)
                    {
                        String strPath;
                        strPath << newDirectory << TEXT("\\") << lpCurFile;
                        files.Add(strPath);
                        lpCurFile += slen(lpCurFile)+1;
                    }
                }

                _this->PlaylistFilesDropped(files);

                Free(lpFile);
            }
            break;
        }
    case IDOK:
        {
            VideoSourceConfigDialog *_this = (VideoSourceConfigDialog *)GetWindowLongPtr(hwnd, DWLP_USER);
            VideoSourceConfig *config = _this->GetConfig();

            config->width = GetEditText(_this->hwndWidth).ToInt();
            config->height = GetEditText(_this->hwndHeight).ToInt();
            config->volume = Slider_GetPos(_this->hwndVolume);
            config->isStretching = Button_IsChecked(_this->hwndStretch);
            config->isAudioOutputToStream = Button_IsChecked(_this->hwndIsAudioOutputToStream);

            int audioOutputTypeIndex = ComboBox_GetCurSel(_this->hwndAudioOutputType);
            int audioOutputDeviceIndex = ComboBox_GetCurSel(_this->hwndAudioOutputDevice);

            AudioOutputType &type = config->GetAudioOutputTypes()[audioOutputTypeIndex];
            config->audioOutputType = type.GetName();
            if (audioOutputDeviceIndex >= 0) {
                AudioOutputDevice &device = type.GetAudioOutputDevices()[audioOutputDeviceIndex];
                config->audioOutputDevice = device.GetLongName();
            }
         
            config->playlist.Clear();

            int itemCount = ListView_GetItemCount(_this->hwndPlaylist);
            LVITEM item = { 0 };
            item.iSubItem = 0;
            item.mask = LVIF_TEXT;
            TCHAR buf[MAX_PATH];

            for (int i = 0; i < itemCount; i++) {
                item.iItem = i;
                item.iSubItem = 0;
                item.cchTextMax = MAX_PATH;
                item.pszText = buf;
                ListView_GetItem(_this->hwndPlaylist, &item);
                config->playlist.Add(item.pszText);
            }

            if (_this->playlistDropTarget) {
                _this->playlistDropTarget->Release();
                _this->playlistDropTarget = nullptr;
            }

            EndDialog(hwnd, IDOK);
            break;
        }
    case IDCANCEL:
        {
            VideoSourceConfigDialog *_this = (VideoSourceConfigDialog *)GetWindowLongPtr(hwnd, DWLP_USER);

            if (_this->playlistDropTarget) {
                _this->playlistDropTarget->Release();
                _this->playlistDropTarget = nullptr;
            }

            EndDialog(hwnd, IDCANCEL);
            break;
        }
    }
}

void Config_OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
	VideoSourceConfigDialog *_this = (VideoSourceConfigDialog *)GetWindowLongPtr(hwnd, DWLP_USER);
    
    int iPos;
    int iRet;
    LVHITTESTINFO lvhti;
    LVITEM lvi;
    TCHAR buf[MAX_PATH];
    
    // End the drag-and-drop process
    _this->bDragging = FALSE;
    ImageList_DragLeave(_this->hwndPlaylist);
    ImageList_EndDrag();
    ImageList_Destroy(_this->hDragImageList);

    ReleaseCapture();

    // Determine the dropped item
    lvhti.pt.x = x;
    lvhti.pt.y = y;
    ClientToScreen(hwnd, &lvhti.pt);
    ScreenToClient(_this->hwndPlaylist, &lvhti.pt);
    ListView_HitTest(_this->hwndPlaylist, &lvhti);

    // Out of the ListView?
    if (lvhti.iItem == -1) {
        return;
    }

    // Not in an item?
    if (((lvhti.flags & LVHT_ONITEMLABEL) == 0) && 
        ((lvhti.flags & LVHT_ONITEMSTATEICON) == 0)) {
        return;
    }

    // Dropped item is selected?
    lvi.iItem = lvhti.iItem;
    lvi.iSubItem = 0;
    lvi.mask = LVIF_STATE;
    lvi.stateMask = LVIS_SELECTED;
    ListView_GetItem(_this->hwndPlaylist, &lvi);

    if (lvi.state & LVIS_SELECTED) {
        return;
    }

    int dropIndex = lvi.iItem;

    // Rearrange the items
    iPos = ListView_GetNextItem(_this->hwndPlaylist, -1, LVNI_SELECTED);
    while (iPos != -1) {
        // First, copy one item
        lvi.iItem = iPos;
        lvi.iSubItem = 0;
        lvi.cchTextMax = MAX_PATH;
        lvi.pszText = buf;
        lvi.stateMask = ~LVIS_SELECTED;
        lvi.mask = LVIF_STATE | LVIF_IMAGE 
                    | LVIF_INDENT | LVIF_PARAM | LVIF_TEXT;
        
        ListView_GetItem(_this->hwndPlaylist, &lvi);
        
        lvi.iItem = lvhti.iItem;
        if (lvhti.iItem < iPos) {
            lvi.iItem = lvhti.iItem;
        } else {
            lvi.iItem = lvhti.iItem + 1;
        }

        // Insert the main item
        iRet = ListView_InsertItem(_this->hwndPlaylist, &lvi);
        if (lvi.iItem < iPos) {
            lvhti.iItem++;
        }
        if (iRet <= iPos) {
            iPos++;
        }

        // Delete from original position
        ListView_DeleteItem(_this->hwndPlaylist, iPos);
        iPos = ListView_GetNextItem(_this->hwndPlaylist, -1, LVNI_SELECTED);
    }

}

void Config_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	VideoSourceConfigDialog *_this = (VideoSourceConfigDialog *)GetWindowLongPtr(hwnd, DWLP_USER);

    if (!_this->bDragging) {
        return;
    }

    POINT p;
    p.x = x;
    p.y = y;

    ClientToScreen(hwnd, &p);
    ImageList_DragMove(p.x, p.y);
}

INT_PTR CALLBACK Config_OnNotify(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    POINT p;
    POINT pt;
    int iHeight;
    HIMAGELIST hOneImageList;
    HIMAGELIST hTempImageList;
    IMAGEINFO           imf;

    BOOL bFirst;

    int iPos;

    switch (((LPNMHDR)lParam)->code)
    {
        case LVN_BEGINDRAG:
        {
            VideoSourceConfigDialog *_this = (VideoSourceConfigDialog *)GetWindowLongPtr(hwnd, DWLP_USER);

            // You can set your customized cursor here

            p.x = 8;
            p.y = 8;

            // Ok, now we create a drag-image for all selected items
            bFirst = TRUE;
            iPos = ListView_GetNextItem(_this->hwndPlaylist, -1, LVNI_SELECTED);
            while (iPos != -1) {
                if (bFirst) {
                    // For the first selected item,
                    // we simply create a single-line drag image
                    _this->hDragImageList = ListView_CreateDragImage(_this->hwndPlaylist, iPos, &p);
                    ImageList_GetImageInfo(_this->hDragImageList, 0, &imf);
                    iHeight = imf.rcImage.bottom;
                    bFirst = FALSE;
                }else {
                    // For the rest selected items,
                    // we create a single-line drag image, then
                    // append it to the bottom of the complete drag image
                    hOneImageList = ListView_CreateDragImage(_this->hwndPlaylist, iPos, &p);
                    hTempImageList = ImageList_Merge(_this->hDragImageList, 
                                        0, hOneImageList, 0, 0, iHeight);
                    ImageList_Destroy(_this->hDragImageList);
                    ImageList_Destroy(hOneImageList);
                    _this->hDragImageList = hTempImageList;
                    ImageList_GetImageInfo(_this->hDragImageList, 0, &imf);
                    iHeight = imf.rcImage.bottom;
                }
                iPos = ListView_GetNextItem(_this->hwndPlaylist, iPos, LVNI_SELECTED);
            }

            // Now we can initialize then start the drag action
            ImageList_BeginDrag(_this->hDragImageList, 0, 0, 0);

            pt = ((NM_LISTVIEW*) ((LPNMHDR)lParam))->ptAction;
            ClientToScreen(_this->hwndPlaylist, &pt);

            ImageList_DragEnter(GetDesktopWindow(), pt.x, pt.y);

            _this->bDragging = TRUE;

            // Don't forget to capture the mouse
            SetCapture(hwnd);

            break;
        }
    }

    return FALSE;
}

void Config_OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized)
{
    VideoSourceConfigDialog *_this = (VideoSourceConfigDialog *)GetWindowLongPtr(hwnd, DWLP_USER);
    if (state == WA_INACTIVE) {
        if (_this && _this->bDragging) {
            _this->bDragging = FALSE;
            ImageList_DragLeave(_this->hwndPlaylist);
            ImageList_EndDrag();
            ImageList_Destroy(_this->hDragImageList);
            ReleaseCapture();
        }
    }
}

