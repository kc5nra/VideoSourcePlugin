#pragma once

#include "OBSApi.h"
#include "WindowsHelper.h"
#include <vector>

class DropTargetListener {
public:
    virtual void FilesDropped(StringList &files) = 0;
};

//
//	This is our definition of a class which implements
//  the IDropTarget interface
//
class DropTarget : public IDropTarget
{
public:
	// Constructor
	DropTarget(HWND hwnd, DropTargetListener *dropTargetListener);
	~DropTarget();

private:

	// internal helper function
	DWORD DropEffect(DWORD grfKeyState, POINTL pt, DWORD dwAllowed);
	bool  QueryDataObject(IDataObject *pDataObject);
    void DropData(HWND hwnd, IDataObject *pDataObject);

	// Private member variables
	LONG	m_lRefCount;
	HWND	m_hWnd;
	bool    m_fAllowDrop;

	IDataObject *m_pDataObject;

    DropTargetListener *m_pDropTargetListener;

public:
    // IUnknown implementation
	HRESULT __stdcall QueryInterface (REFIID iid, void ** ppvObject);
	ULONG	__stdcall AddRef (void);
	ULONG	__stdcall Release (void);

	// IDropTarget implementation
	HRESULT __stdcall DragEnter (IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
	HRESULT __stdcall DragOver (DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
	HRESULT __stdcall DragLeave (void);
	HRESULT __stdcall Drop (IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);

public:
    static DropTarget *DropTarget::RegisterDropWindow(HWND hwnd, DropTargetListener *dropTargetListener);
    static void UnregisterDropWindow(HWND hwnd, IDropTarget *pDropTarget);


};