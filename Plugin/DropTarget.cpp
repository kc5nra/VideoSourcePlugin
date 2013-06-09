//
//	DROPTARGET.CPP
//
//	By J Brown 2004 
//
//	www.catch22.net
//

#include "DropTarget.h"
#include <shellapi.h>



//
//	Constructor for the DropTarget class
//
DropTarget::DropTarget(HWND hwnd, DropTargetListener *dropTargetListener)
{
	m_lRefCount  = 0;
	m_fAllowDrop = false;
    m_hWnd = hwnd;
    m_pDropTargetListener = dropTargetListener;
}

//
//	Destructor for the DropTarget class
//
DropTarget::~DropTarget()
{
	
}

//
//	Position the edit control's caret under the mouse
//
void PositionCursor(HWND hwndEdit, POINTL pt)
{
	LRESULT curpos; 
		
	// get the character position of mouse
	ScreenToClient(hwndEdit, (POINT *)&pt);
	curpos = SendMessage(hwndEdit, EM_CHARFROMPOS, 0, MAKELPARAM(pt.x, pt.y));

	// set cursor position
	SendMessage(hwndEdit, EM_SETSEL, LOWORD(curpos), LOWORD(curpos));
}

//
//	IUnknown::QueryInterface
//
HRESULT __stdcall DropTarget::QueryInterface (REFIID iid, void ** ppvObject)
{
	if(iid == IID_IDropTarget || iid == IID_IUnknown)
	{
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else
	{
		*ppvObject = 0;
		return E_NOINTERFACE;
	}
}

//
//	IUnknown::AddRef
//
ULONG __stdcall DropTarget::AddRef(void)
{
	return InterlockedIncrement(&m_lRefCount);
}	

//
//	IUnknown::Release
//
ULONG __stdcall DropTarget::Release(void)
{
	LONG count = InterlockedDecrement(&m_lRefCount);
		
	if(count == 0)
	{
		delete this;
		return 0;
	}
	else
	{
		return count;
	}
}

//
//	QueryDataObject private helper routine
//
bool DropTarget::QueryDataObject(IDataObject *pDataObject)
{
    FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	// does the data object support CF_TEXT using a HGLOBAL?
	return pDataObject->QueryGetData(&fmtetc) == S_OK ? true : false;
}

//
//	DropEffect private helper routine
//
DWORD DropTarget::DropEffect(DWORD grfKeyState, POINTL pt, DWORD dwAllowed)
{
	DWORD dwEffect = 0;

	// 1. check "pt" -> do we allow a drop at the specified coordinates?
	
	// 2. work out that the drop-effect should be based on grfKeyState
	if(grfKeyState & MK_CONTROL)
	{
		dwEffect = dwAllowed & DROPEFFECT_COPY;
	}
	else if(grfKeyState & MK_SHIFT)
	{
		dwEffect = dwAllowed & DROPEFFECT_MOVE;
	}
	
	// 3. no key-modifiers were specified (or drop effect not allowed), so
	//    base the effect on those allowed by the dropsource
	if(dwEffect == 0)
	{
		if(dwAllowed & DROPEFFECT_COPY) dwEffect = DROPEFFECT_COPY;
		if(dwAllowed & DROPEFFECT_MOVE) dwEffect = DROPEFFECT_MOVE;
	}
	
	return dwEffect;
}


//
//	IDropTarget::DragEnter
//
//
//
HRESULT __stdcall DropTarget::DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	// does the dataobject contain data we want?
	m_fAllowDrop = QueryDataObject(pDataObject);
	
	if(m_fAllowDrop)
	{
		// get the dropeffect based on keyboard state
		*pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);

		SetFocus(m_hWnd);

		PositionCursor(m_hWnd, pt);
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

//
//	IDropTarget::DragOver
//
//
//
HRESULT __stdcall DropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	if(m_fAllowDrop)
	{
		*pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);
		PositionCursor(m_hWnd, pt);
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

//
//	IDropTarget::DragLeave
//
HRESULT __stdcall DropTarget::DragLeave(void)
{
	return S_OK;
}

//
//	IDropTarget::Drop
//
//
HRESULT __stdcall DropTarget::Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	PositionCursor(m_hWnd, pt);

	if(m_fAllowDrop) {
		DropData(m_hWnd, pDataObject);
		*pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);
	} else {
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}



void DropTarget::DropData(HWND hwnd, IDataObject *pDataObject)
{
	// construct a FORMATETC object
	FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgmed;

	// See if the dataobject contains any TEXT stored as a HGLOBAL
	if(pDataObject->QueryGetData(&fmtetc) == S_OK)
	{
		// Yippie! the data is there, so go get it!
		if(pDataObject->GetData(&fmtetc, &stgmed) == S_OK)
		{
            
			// we asked for the data as a HGLOBAL, so access it appropriately
			PVOID data = GlobalLock(stgmed.hGlobal);

            HDROP &hdrop = (HDROP &)data;
            
            UINT uNumFiles = DragQueryFile(hdrop, -1, NULL, 0);

            StringList files;
            for (UINT uFile = 0; uFile < uNumFiles; uFile++ ) {
                // Get the next filename from the HDROP info.
                TCHAR szNextFile [MAX_PATH];
                if (DragQueryFile(hdrop, uFile, szNextFile, MAX_PATH ) > 0 ) {
                    files.Add(String(szNextFile));
                }
            }

            m_pDropTargetListener->FilesDropped(files);

            // Set up an LVITEM for the file we're about to insert.
			
			GlobalUnlock(stgmed.hGlobal);

			// release the data using the COM API
			ReleaseStgMedium(&stgmed);
		}
	}
}

DropTarget *DropTarget::RegisterDropWindow(HWND hwnd, DropTargetListener *dropTargetListener)
{
	DropTarget *pDropTarget = new DropTarget(hwnd, dropTargetListener);

	// acquire a strong lock
	CoLockObjectExternal(pDropTarget, TRUE, FALSE);

	// tell OLE that the window is a drop target
	RegisterDragDrop(hwnd, pDropTarget);

	return pDropTarget;
}

void DropTarget::UnregisterDropWindow(HWND hwnd, IDropTarget *pDropTarget)
{
	// remove drag+drop
	RevokeDragDrop(hwnd);

	// remove the strong lock
	CoLockObjectExternal(pDropTarget, FALSE, TRUE);

	// release our own reference
	pDropTarget->Release();
}