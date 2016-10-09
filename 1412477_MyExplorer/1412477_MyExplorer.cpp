// 1412477_MyExplorer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "1412477_MyExplorer.h"
#include <commctrl.h>
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "ComCtl32.lib")
//Dùng để sử dụng hàm StrCpy, StrNCat
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <shellapi.h>

#define MAX_LOADSTRING 100
#define INITIAL_ICON_IN_TREE 8 //Tổng số icon lúc đầu trong tree, mặc định chỉ nạp những gì cần thiết
#define NUMBER_OF_ICON_TO_GROW 0 //Số icon có thể mở rộng

#define DEFAULT_ICON_INDEX 0

#define LVCOL_DRIVE_TYPE		0
#define LVCOL_FOLDER_TYPE		1

//Độ dài tối đa đường dẫn
#define MAX_PATH_LEN 10240

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
RECT rcClient;                       // The parent window's client area.
HINSTANCE g_hInstance;
HWND	g_hWnd;
HWND	g_hTreeView;
HWND	g_hListView;
DriveHelper* g_Drive;

int myComputerIconIndex, desktopIconIndex, driveIconIndex;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

HWND createListView(long lExtStyle, HWND parentWnd, long ID, HINSTANCE hParentInst, int x, int y, int nWidth, int nHeight, long lStyle);
HWND createTreeView(long lExtStyle, HWND parentWnd, long ID, HINSTANCE hParentInst, int x, int y, int nWidth, int nHeight, long lStyle);
void loadMyComputerToTree(DriveHelper *drive, HWND m_hTreeView);
void loadMyComputerToListView(DriveHelper *drive, HWND m_hListView);
void loadExpandedChild(HTREEITEM hCurrSelected, HWND m_hTreeView);
LPCWSTR getPath(HTREEITEM hItem, HWND m_hTreeView);
void loadChild(HTREEITEM &hParent, LPCWSTR path, BOOL bShowHiddenSystem, HWND m_hTreeView);
void loadChild(HWND m_hParent, HWND m_hListView, LPCWSTR path, DriveHelper *drive);
void displayCurrSelectedInfo(HWND m_hParent, HWND m_hListView);
LPCWSTR getCurrPath(HWND m_hTreeView);
void loadCurrSelected(HWND m_hListView);
void loadFileAndFolder(HWND m_hParent, HWND m_hListView, LPCWSTR path);
LPWSTR _GetSize(const WIN32_FIND_DATA &fd);
void initListViewColumn(HWND m_hListView, int type);
LPWSTR convertTimeStampToString(const FILETIME &ftLastWrite); //Convert  Active Directory timestamps (LDAP/Win32 FILETIME) to DateTime in String

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MY1412477_MYEXPLORER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	g_hInstance = hInstance;


	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY1412477_MYEXPLORER));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY1412477_MYEXPLORER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MY1412477_MYEXPLORER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindowEx(0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	

	switch (message)
	{
	case WM_CREATE:
	{
					  g_hWnd = hWnd;
					  g_Drive = new DriveHelper();
					  g_Drive->GetSystemDrives();
					  
					  InitCommonControls();

					  //Get main parent window size
					  GetClientRect(hWnd, &rcClient);
					  int parentWidth = rcClient.right - rcClient.left;
					  int parentHeight = rcClient.bottom - rcClient.top;

					  //Create treeview
					  long extStyle = 0, style = TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS;
					  int x = 0, y = 0;
					  int nWidth = parentWidth / 4, nHeight = parentHeight;

					  //Tree View
					  g_hTreeView = createTreeView(extStyle, hWnd, IDT_TREEVIEW, g_hInstance, x, y, nWidth, nHeight, style);

					  loadMyComputerToTree(g_Drive, g_hTreeView);
					  SetFocus(g_hTreeView);

					  //Create listview
					  x = nWidth + 1;
					  nWidth = (rcClient.right - rcClient.left) * 3 / 4;
					  extStyle = WS_EX_CLIENTEDGE;
					  style = LVS_REPORT | LVS_ICON | LVS_EDITLABELS | LVS_SHOWSELALWAYS;

					  g_hListView = createListView(extStyle, hWnd, IDL_LISTVIEW, g_hInstance,x ,y ,nWidth, nHeight, style);
					  loadMyComputerToListView(g_Drive, g_hListView);

	}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_NOTIFY:
	{
					  int nCurSelIndex;

					  //The program has started and loaded all necessary component

					  NMHDR* notifyMess = (NMHDR*)lParam; //Notification Message
					  LPNMTREEVIEW lpnmTree = (LPNMTREEVIEW)notifyMess; //Contains information about a tree-view notification message

					  switch (notifyMess->code)
					  {
					  case TVN_ITEMEXPANDING: //This event fire when user expand or colapse item in Tree View
						  //Load child item of current selected child if they were not loaded before
						  loadExpandedChild(lpnmTree->itemNew.hItem, g_hTreeView);
						  break;
					  case TVN_SELCHANGED:
						  TreeView_Expand(g_hTreeView, TreeView_GetNextItem(g_hTreeView, NULL, TVGN_CARET), TVE_EXPAND);
						  ListView_DeleteAllItems(g_hListView); //Clear ListView
						  loadChild(hWnd, g_hListView, getCurrPath(g_hTreeView), g_Drive);
						  //m_hTreeView->InsertAfterCur(g_TreeView->GetCurPath());

						  break;

					  case NM_CLICK:
						  //Get hwndFrom handle that cause the event to check whether ListView sent message
						  if (notifyMess->hwndFrom == g_hListView)
						  {
							  nCurSelIndex = ListView_GetNextItem(g_hListView, -1, LVNI_FOCUSED);
							  if (nCurSelIndex != -1)
								  displayCurrSelectedInfo(hWnd, g_hListView);

						  }
						  else
						  {
							  //Do nothing								  
							  break;
						  }
						  break;

					  case NM_DBLCLK:
						  if (notifyMess->hwndFrom == g_hListView)
							  loadCurrSelected(g_hListView);
						  break;
					  }

	}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


HWND createListView(long lExtStyle, HWND parentWnd, long ID, HINSTANCE hParentInst, int x, int y, int nWidth, int nHeight, long lStyle)
{
	//Create
	HWND m_hListView = CreateWindowEx(lExtStyle, WC_LISTVIEW, _T("List View"),
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | lStyle,
		x, y, nWidth, nHeight, parentWnd, (HMENU)ID, hParentInst, NULL);


	//Init 5 columns
	LVCOLUMN lvCol;

	//Let the LVCOLUMN know that we will set the format, header text and width of it
	lvCol.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvCol.fmt = LVCFMT_LEFT;

	lvCol.cx = 150;
	lvCol.pszText = _T("Name");
	ListView_InsertColumn(m_hListView, 0, &lvCol);

	lvCol.fmt = LVCFMT_LEFT;
	lvCol.pszText = _T("Type");
	lvCol.cx = 125;
	ListView_InsertColumn(m_hListView, 1, &lvCol);

	lvCol.fmt = LVCFMT_LEFT;
	lvCol.cx = 125;
	lvCol.pszText = _T("Total Size");
	ListView_InsertColumn(m_hListView, 2, &lvCol);


	lvCol.fmt = LVCFMT_LEFT;
	lvCol.pszText = _T("Free Space");
	lvCol.cx = 125;
	ListView_InsertColumn(m_hListView, 3, &lvCol);

	return m_hListView;
}

HWND createTreeView(long lExtStyle, HWND parentWnd, long ID, HINSTANCE hParentInst, int x, int y, int nWidth, int nHeight, long lStyle)
{
	//Create
	HWND m_hTreeView = CreateWindowEx(lExtStyle, WC_TREEVIEW, _T("Tree View"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_SIZEBOX | WS_VSCROLL | WS_TABSTOP | lStyle,
		x, y, nWidth, nHeight, parentWnd,
		(HMENU)ID, hParentInst, NULL);

	//////////////////////////////////////////////////////
	//I dont know what im doing here T.T
	//After a ton of efford, icon still dont work
	//////////////////////////////////////////////////////
	HIMAGELIST himl;  // handle to image list 
	HBITMAP hbmp;     // handle to bitmap 

	// Create the image list. 
	himl = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK,
		INITIAL_ICON_IN_TREE, NUMBER_OF_ICON_TO_GROW);

	// Add the open file, closed file, and document bitmaps. 
	hbmp = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_DESKTOP));
	desktopIconIndex = ImageList_Add(himl, hbmp, (HBITMAP)NULL);
	DeleteObject(hbmp);

	hbmp = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_MYCOMPUTER));
	myComputerIconIndex = ImageList_Add(himl, hbmp, (HBITMAP)NULL);
	DeleteObject(hbmp);

	hbmp = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_DRIVE));
	driveIconIndex = ImageList_Add(himl, hbmp, (HBITMAP)NULL);
	DeleteObject(hbmp);


	// Associate the image list with the tree-view control. 
	TreeView_SetImageList(m_hTreeView, himl, TVSIL_NORMAL);

	///////////////////////////////////////////////////////

	return m_hTreeView;
}

void loadMyComputerToTree(DriveHelper *drive, HWND m_hTreeView)
{
	TV_INSERTSTRUCT tvInsert;

	//Let it know we will change the text, icon and lparam associate with item in treeview
	tvInsert.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

	//Load Desktop
	tvInsert.hParent = NULL;
	tvInsert.hInsertAfter = TVI_ROOT;
	tvInsert.item.iImage = desktopIconIndex;
	tvInsert.item.iSelectedImage = desktopIconIndex;
	tvInsert.item.pszText = _T("Desktop");
	tvInsert.item.lParam = (LPARAM)_T("Desktop");
	HTREEITEM hDesktop = TreeView_InsertItem(m_hTreeView, &tvInsert);

	//Load My Computer
	tvInsert.hParent = hDesktop;
	tvInsert.hInsertAfter = TVI_LAST; //Add to last position
	tvInsert.item.iImage = myComputerIconIndex;
	tvInsert.item.iSelectedImage = myComputerIconIndex;
	tvInsert.item.pszText = _T("My Computer");
	tvInsert.item.lParam = (LPARAM)_T("MyComputer");
	HTREEITEM hMyComputer = TreeView_InsertItem(m_hTreeView, &tvInsert);

	//Load volume
	for (int i = 0; i < g_Drive->GetCount(); ++i)
	{
		tvInsert.hParent = hMyComputer; //Add as children of My Computer
		tvInsert.item.iImage = driveIconIndex;
		tvInsert.item.iSelectedImage = driveIconIndex;
		tvInsert.item.pszText = g_Drive->GetDisplayName(i); //Get volume label
		tvInsert.item.lParam = (LPARAM)g_Drive->GetDriveName(i);
		HTREEITEM hDrive = TreeView_InsertItem(m_hTreeView, &tvInsert);

		loadChild(hDrive, getPath(hDrive, m_hTreeView), FALSE, m_hTreeView);
	}

	//Mặc định cho My Computer expand và select luôn
	TreeView_Expand(m_hTreeView, hMyComputer, TVE_EXPAND);
	TreeView_SelectItem(m_hTreeView, hMyComputer);
}


void loadMyComputerToListView(DriveHelper *drive, HWND m_hListView)
{
	SetDlgItemText(GetDlgItem(g_hWnd, IDC_ADDRESS), IDC_ADDRESS_EDIT, _T("My Computer"));
	//InitDriveColumn();
	//DeleteAll();
	LV_ITEM lv;

	for (int i = 0; i < drive->GetCount(); ++i)
	{
		//Let ListView know that we'r going to change item text, image and param
		lv.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		lv.iItem = i;
		lv.iImage = DEFAULT_ICON_INDEX;

		lv.iSubItem = 0;
		lv.pszText = drive->GetDisplayName(i);
		lv.lParam = (LPARAM)drive->GetDriveName(i);
		ListView_InsertItem(m_hListView, &lv);

		//
		lv.mask = LVIF_TEXT;

		//First column is Date Modified
		lv.iSubItem = 1;
		lv.pszText = drive->getDriveType(i);
		ListView_SetItem(m_hListView, &lv);

		//Cột tiếp theo là Size
		lv.iSubItem = 2;

		if (wcscmp(drive->getDriveType(i), CD_ROM) != 0)
			lv.pszText = drive->getTotalSize(i);
		else
			lv.pszText = NULL;

		ListView_SetItem(m_hListView, &lv);

		//Cột cuối cùng là Free Space
		lv.iSubItem = 3;

		if (wcscmp(drive->getDriveType(i), CD_ROM) != 0)
			lv.pszText = drive->getFreeSpace(i);
		else
			lv.pszText = NULL;

		ListView_SetItem(m_hListView, &lv);
	}//for

	TCHAR *buffer = new TCHAR[34];
	wsprintf(buffer, _T("My Computer có tổng cộng %d ổ đĩa"), drive->GetCount());
	//SendMessage(GetDlgItem(g_hWnd, IDC_STATUSBAR), SB_SETTEXT, 0, (LPARAM)buffer);
	//SendMessage(GetDlgItem(g_hWnd, IDC_STATUSBAR), SB_SETTEXT, 2, (LPARAM)_T("My Computer"));
}


LPCWSTR getPath(HTREEITEM hItem, HWND m_hTreeView)
{
	TVITEMEX tv; //Specifies or receives attributes of a tree-view item.
	tv.mask = TVIF_PARAM;
	tv.hItem = hItem;
	TreeView_GetItem(m_hTreeView, &tv); //Retrieves some or all of a tree-view item's attributes. 
	return (LPCWSTR)tv.lParam;
}

LPCWSTR getPath(HWND m_hListView, int iItem)
{
	LVITEM lv;
	lv.mask = LVIF_PARAM;
	lv.iItem = iItem;
	lv.iSubItem = 0;
	ListView_GetItem(m_hListView, &lv);
	return (LPCWSTR)lv.lParam;
}

void loadExpandedChild(HTREEITEM hCurrSelected, HWND m_hTreeView)
{
	HTREEITEM myDesktop = TreeView_GetRoot(m_hTreeView); //Return the top most or very first item of the TreeView
	HTREEITEM myComputer = TreeView_GetChild(m_hTreeView, myDesktop); //Return the first child item of Desktop (MyComputer)
	if (hCurrSelected == myComputer) //If currently select My Computer -> dont load
		return;

	HTREEITEM hCurrSelectedChild = TreeView_GetChild(m_hTreeView, hCurrSelected); //Get the first child of treeview item
	
	if (hCurrSelectedChild != NULL)
	{
		do 
		{
			//Get child of this Current selected child, if result is NULL -> It never be loaded
			if (TreeView_GetChild(m_hTreeView, hCurrSelectedChild) == NULL)
			{
				//Load all child of Current Selected Child	
				loadChild(hCurrSelectedChild, getPath(hCurrSelectedChild, m_hTreeView), FALSE, m_hTreeView);
			}
		} 
		while (hCurrSelectedChild = TreeView_GetNextSibling(m_hTreeView, hCurrSelectedChild));

		/*
		//Another way
		//This is quite a mess -_-

		//HTREEITEM hCurrSelectedChildOfChild = TreeView_GetChild(m_hTreeView, hCurrSelectedChild);
		if (hCurrSelectedChildOfChild == NULL)
		{
			loadChild(hCurrSelectedChild, getPath(hCurrSelectedChild, m_hTreeView), FALSE, m_hTreeView);

			while (hCurrSelectedChild = TreeView_GetNextSibling(m_hTreeView, hCurrSelectedChild))
			{
				if (TreeView_GetChild(m_hTreeView, hCurrSelectedChild) == NULL)
				{
					loadChild(hCurrSelectedChild, getPath(hCurrSelectedChild, m_hTreeView), FALSE, m_hTreeView);
				}
			}
		}
		*/
	}
	else
	{
		//Possitively will not happen :D
		loadChild(hCurrSelected, getPath(hCurrSelected, m_hTreeView), FALSE, m_hTreeView);
	}
}

void loadChild(HTREEITEM &hParent, LPCWSTR path, BOOL bShowHiddenSystem, HWND m_hTreeView)
{
	TCHAR buffer[MAX_PATH_LEN];
	StrCpy(buffer, path); //Copy the path of item (include drive letter path)

	StrCat(buffer, _T("\\*")); //Add to find all item in directory

	TV_INSERTSTRUCT tvInsert;
	tvInsert.hParent = hParent;
	tvInsert.hInsertAfter = TVI_SORT;
	tvInsert.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvInsert.item.iImage = DEFAULT_ICON_INDEX;
	tvInsert.item.iSelectedImage = DEFAULT_ICON_INDEX;

	WIN32_FIND_DATA ffd; //Contains information about the file that is found by Find first file and Find next file
	HANDLE hFile = FindFirstFileW(buffer, &ffd);
	BOOL bFound = 1;

	if (hFile == INVALID_HANDLE_VALUE)
		bFound = FALSE;

	TCHAR* folderPath;
	while (bFound)
	{
		DWORD fileAttribute = ffd.dwFileAttributes;
		if ((fileAttribute & FILE_ATTRIBUTE_DIRECTORY ) //Get only directory file and compressed file
			&& (fileAttribute != FILE_ATTRIBUTE_HIDDEN)//Not hidden
			&& (StrCmp(ffd.cFileName, _T(".")) != 0) && (StrCmp(ffd.cFileName, _T("..")) != 0) )
		{
			tvInsert.item.pszText = ffd.cFileName;
			folderPath = new TCHAR[wcslen(path) + wcslen(ffd.cFileName) + 2];

			StrCpy(folderPath, path);
			if (wcslen(path) != 3)
				StrCat(folderPath, _T("\\"));
			StrCat(folderPath, ffd.cFileName);

			tvInsert.item.lParam = (LPARAM)folderPath;
			HTREEITEM hItem = TreeView_InsertItem(m_hTreeView, &tvInsert);
		}

		bFound = FindNextFileW(hFile, &ffd);
	}
}

void loadChild(HWND m_hParent, HWND m_hListView, LPCWSTR path, DriveHelper *drive)
{
	if (path == NULL)
		return;

	ListView_DeleteAllItems(m_hListView);
	LV_ITEM lv;

	if (!StrCmp(path, _T("Desktop")))
	{
		//Load Desktop to Listview (My Computer)
		initListViewColumn(m_hListView, LVCOL_FOLDER_TYPE);

		lv.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		lv.iItem = 0;
		lv.iSubItem = 0;
		lv.pszText = _T("My Computer");
		lv.iImage = IDI_MYCOMPUTER;
		lv.lParam = (LPARAM)_T("MyComputer");
		ListView_InsertItem(m_hListView, &lv);
	}
	else
	if (!StrCmp(path, _T("MyComputer")))
	{
		initListViewColumn(m_hListView, LVCOL_DRIVE_TYPE);
		
		for (int i = 0; i < drive->GetCount(); ++i)
		{
			//Nạp cột đầu tiên cũng là thông tin chính
			lv.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
			lv.iItem = i;
			lv.iImage = DEFAULT_ICON_INDEX;

			lv.iSubItem = 0;
			lv.pszText = drive->GetDisplayName(i);
			lv.lParam = (LPARAM)drive->GetDriveName(i);
			ListView_InsertItem(m_hListView, &lv);

			//Nạp các cột còn lại (Type, Size, Free Space)
			lv.mask = LVIF_TEXT;

			//Cột đầu tiên là Type
			lv.iSubItem = 1;
			lv.pszText = drive->getDriveType(i);
			ListView_SetItem(m_hListView, &lv);

			//Cột tiếp theo là Size
			lv.iSubItem = 2;

			if (wcscmp(drive->getDriveType(i), CD_ROM) != 0)
				lv.pszText = drive->getTotalSize(i);
			else
				lv.pszText = NULL;

			ListView_SetItem(m_hListView, &lv);

			//Cột cuối cùng là Free Space
			lv.iSubItem = 3;

			if (wcscmp(drive->getDriveType(i), CD_ROM) != 0)
				lv.pszText = drive->getFreeSpace(i);
			else
				lv.pszText = NULL;

			ListView_SetItem(m_hListView, &lv);
		}
	}
	else
		loadFileAndFolder(g_hWnd,m_hListView,path);
}

void displayCurrSelectedInfo(HWND m_hParent, HWND m_hListView)
{
	int nCurSelIndex = ListView_GetNextItem(GetDlgItem(m_hParent, IDL_LISTVIEW), -1, LVNI_FOCUSED);
	TCHAR *text = new TCHAR[256];
	LVITEM lv;
	lv.mask = LVIF_TEXT;
	lv.iItem = nCurSelIndex;
	lv.iSubItem = 0;
	lv.pszText = text;
	lv.cchTextMax = 256;

	ListView_GetItem(m_hListView, &lv);

	lv.iSubItem = 2;
	ListView_GetItem(m_hListView, &lv);
}

LPCWSTR getCurrPath(HWND m_hTreeView)
{
	return getPath(TreeView_GetNextItem(m_hTreeView, NULL, TVGN_CARET), m_hTreeView);
}

void loadCurrSelected(HWND m_hListView)
{
	LPCWSTR path = getPath(m_hListView,ListView_GetSelectionMark(m_hListView));

	WIN32_FIND_DATA fd;
	GetFileAttributesEx(path, GetFileExInfoStandard, &fd);

	//Nếu là thư mục
	if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		ListView_DeleteAllItems(m_hListView);
		loadFileAndFolder(g_hWnd,m_hListView,path);
	}
	else //Nếu là tập tin thì chạy nó
		ShellExecute(NULL, _T("open"), path, NULL, NULL, SW_SHOWNORMAL);
}

void loadFileAndFolder(HWND m_hParent, HWND m_hListView, LPCWSTR path)
{
	initListViewColumn(m_hListView, LVCOL_FOLDER_TYPE);
	TCHAR buffer[10240];
	StrCpy(buffer, path);

	if (wcslen(path) == 3) //Nếu quét các ổ đĩa
		StrCat(buffer, _T("*"));
	else
		StrCat(buffer, _T("\\*"));

	//Bắt đầu tìm các file và folder trong thư mục
	WIN32_FIND_DATA fd;
	HANDLE hFile;
	BOOL bFound = true;
	LV_ITEM lv;

	TCHAR * folderPath;
	int nItemCount = 0;

	//Chạy lần thứ nhất lấy các thư mục
	hFile = FindFirstFileW(buffer, &fd);
	bFound = TRUE;

	if (hFile == INVALID_HANDLE_VALUE)
		bFound = FALSE;

	while (bFound)
	{
		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
			((fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != FILE_ATTRIBUTE_HIDDEN) &&
			(StrCmp(fd.cFileName, _T(".")) != 0) && (StrCmp(fd.cFileName, _T("..")) != 0))
		{
			folderPath = new TCHAR[wcslen(path) + wcslen(fd.cFileName) + 2];
			StrCpy(folderPath, path);

			if (wcslen(path) != 3)
				StrCat(folderPath, _T("\\"));

			StrCat(folderPath, fd.cFileName);

			//First column is Name
			lv.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
			lv.iItem = nItemCount;
			lv.iSubItem = 0;
			lv.pszText = fd.cFileName;
			lv.lParam = (LPARAM)folderPath;
			lv.iImage = DEFAULT_ICON_INDEX;
			ListView_InsertItem(m_hListView, &lv);

			//Second column is Date Modified
			ListView_SetItemText(m_hListView, nItemCount, 1, convertTimeStampToString(fd.ftLastWriteTime));

			//Third column is Type
			ListView_SetItemText(m_hListView, nItemCount, 2, _T("File folder"));

			//Last column is Size
			//Let check Explorer whether it show size of file or folder
			
			++nItemCount;
		}

		bFound = FindNextFileW(hFile, &fd);
	}

	DWORD folderCount = nItemCount;
	/*************************************************************************************/
	//Chạy lần thứ hai để lấy các tập tin T_T Cách bưởi chưa từng thấy !!! Nhưng không còn cách nào khác
	TCHAR *filePath;
	DWORD fileSizeCount = 0;
	DWORD fileCount = 0;

	hFile = FindFirstFileW(buffer, &fd);
	bFound = TRUE;

	if (hFile == INVALID_HANDLE_VALUE)
		bFound = FALSE;

	while (bFound)
	{
		//Ignore all Directory and Folder
		if (((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) &&
			((fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != FILE_ATTRIBUTE_SYSTEM) &&
			((fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != FILE_ATTRIBUTE_HIDDEN))
		{
			filePath = new TCHAR[wcslen(path) + wcslen(fd.cFileName) + 2];
			StrCpy(filePath, path);

			if (wcslen(path) != 3)
				StrCat(filePath, _T("\\"));

			StrCat(filePath, fd.cFileName);

			//Cột thứ nhất là tên hiển thị của tập tin
			lv.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
			lv.iItem = nItemCount;
			lv.iSubItem = 0;
			lv.pszText = fd.cFileName;
			lv.lParam = (LPARAM)filePath;
		
			ListView_InsertItem(m_hListView, &lv);

			//Second column is Date Modified
			ListView_SetItemText(m_hListView, nItemCount, 1, convertTimeStampToString(fd.ftLastWriteTime));

			//Third column is Type
			//ListView_SetItemText(m_hListView, nItemCount, 2, _GetType(fd));

			//Last column is Size
			ListView_SetItemText(m_hListView, nItemCount, 3, _GetSize(fd));
			fileSizeCount += fd.nFileSizeLow;


			++nItemCount;
			++fileCount;
		}

		bFound = FindNextFileW(hFile, &fd);
	}

	TVITEMEX tv;
	TCHAR *folder = new TCHAR[512];
	TCHAR *info = new TCHAR[256];

	tv.mask = TVIF_TEXT;
	tv.hItem = TreeView_GetNextItem(GetDlgItem(m_hParent, IDT_TREEVIEW), NULL, TVGN_CARET);
	tv.pszText = folder;
	tv.cchTextMax = 256;
	TreeView_GetItem(GetDlgItem(m_hParent, IDT_TREEVIEW), &tv);
}

LPWSTR _GetSize(const WIN32_FIND_DATA &fd)
{
	DWORD dwSize = fd.nFileSizeLow;

	return CDriveSize::Convert(dwSize);
}

void initListViewColumn(HWND m_hListView, int type)
{
	LVCOLUMN lvCol;
	if (type == LVCOL_DRIVE_TYPE)
	{
		lvCol.mask = LVCF_TEXT | LVCF_FMT;

		lvCol.fmt = LVCFMT_LEFT | LVCF_WIDTH;
		lvCol.cx = 100;
		lvCol.pszText = _T("Type");
		ListView_SetColumn(m_hListView, 1, &lvCol);

		lvCol.fmt = LVCFMT_RIGHT | LVCF_WIDTH;
		lvCol.cx = 80;
		lvCol.pszText = _T("Total Size");
		ListView_SetColumn(m_hListView, 2, &lvCol);

		lvCol.cx = 80;
		lvCol.pszText = _T("Free Space");
		ListView_SetColumn(m_hListView, 3, &lvCol);
	}
	else if (type == LVCOL_FOLDER_TYPE)
	{
		lvCol.mask = LVCF_WIDTH;
		lvCol.cx = 180;
		ListView_SetColumn(m_hListView, 0, &lvCol);

		lvCol.mask = LVCF_TEXT | LVCF_FMT;
		lvCol.fmt = LVCFMT_RIGHT;
		lvCol.pszText = _T("Date Modified");
		ListView_SetColumn(m_hListView, 1, &lvCol);


		lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
		lvCol.fmt = LVCFMT_LEFT;
		lvCol.cx = 130;
		lvCol.pszText = _T("Type");
		ListView_SetColumn(m_hListView, 2, &lvCol);

		lvCol.pszText = _T("Size");
		ListView_SetColumn(m_hListView, 3, &lvCol);
	}
	else
	{
		//Reserve for initialize or create LV
	}
}

LPWSTR convertTimeStampToString(const FILETIME &ftLastWrite)
{

	TCHAR *buffer = new TCHAR[50];

	//The SYSTEMTIME structure represents a date and time using individual members 
	//for the month, day, year, weekday, hour, minute, second, and millisecond. 
	SYSTEMTIME st;

	char szLocalDate[255], szLocalTime[255];
	
	//To display the FILETIME in a meaningful way, you first need to convert it to a SYSTEMTIME
	FileTimeToSystemTime(&ftLastWrite, &st);
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_AUTOLAYOUT, &st, NULL,
		(LPWSTR)szLocalDate, 255);
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, (LPWSTR)szLocalTime, 255);

	//Concat to string
	wsprintf(buffer, L"%s %s", szLocalDate, szLocalTime);

	return buffer;
}