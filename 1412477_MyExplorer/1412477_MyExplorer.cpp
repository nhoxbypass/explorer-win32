// 1412477_MyExplorer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "1412477_MyExplorer.h"
#include <commctrl.h>
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "ComCtl32.lib")
//For StrCpy, StrNCat
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <shellapi.h>

#define MAX_LOADSTRING 100
#define NUMBER_OF_INIT_ICON 8 
#define MAX_EXPAND_ICON 3

#define DEFAULT_ICON_INDEX 0

//For init lv column
#define LVCOL_DRIVE_TYPE		0
#define LVCOL_FOLDER_TYPE		1

//
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
void loadExpandedChild(HTREEITEM hCurrSelected, HWND m_hTreeView); //Load all child and child of child items in treeview
LPCWSTR getPath(HTREEITEM hItem, HWND m_hTreeView); //Get dir path of an item in Treeview
void loadTreeviewItemAt(HTREEITEM &hParent, LPCWSTR path, HWND m_hTreeView); //Load treeview item at dir path provided
void loadListviewItemAt(LPCWSTR path, HWND m_hParent, HWND m_hListView, DriveHelper *drive); //Load listview item at dir path provided
void loadOrExecSelected(HWND m_hListView); //Load selected directory or execute selected file
void loadDirItemToLisview(HWND m_hParent, HWND m_hListView, LPCWSTR path); //Load directory item to Listview
void initListviewColumn(HWND m_hListView, int type); //Initialize Listview column (size, header text,...)
LPWSTR convertTimeStampToString(const FILETIME &ftLastWrite); //Convert Active Directory timestamps (LDAP/Win32 FILETIME) to DateTime in String

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
					  g_Drive->getSystemDrives();
					  
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
					  HTREEITEM currSelected;

					  switch (notifyMess->code)
					  {
					  case TVN_ITEMEXPANDING: //This event fire when user expand or colapse item in Tree View
						  //Load child item of current selected child if they were not loaded before
						  currSelected = lpnmTree->itemNew.hItem;
						  loadExpandedChild(currSelected, g_hTreeView);
						  break;
					  case TVN_SELCHANGED:
						  //Retrieve currently selected item in TreeView
						  currSelected = TreeView_GetSelection(g_hTreeView); //You can explicitly get by TreeView_GetNextItem with TVGN_CARET flag
						  TreeView_Expand(g_hTreeView, currSelected, TVE_EXPAND);
						  
						  ListView_DeleteAllItems(g_hListView); //Clear ListView
						  loadListviewItemAt(getPath(currSelected,g_hTreeView), hWnd, g_hListView, g_Drive);
						  break;

					  case NM_CLICK:
						  break;

					  case NM_DBLCLK:
						  //Get hwndFrom for window handle to the control sending the message
						  //To check whether this event fire by Listview
						  if (notifyMess->hwndFrom == g_hListView)
							  loadOrExecSelected(g_hListView);
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
		NUMBER_OF_INIT_ICON, MAX_EXPAND_ICON);

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
	for (int i = 0; i < g_Drive->getCount(); ++i)
	{
		tvInsert.hParent = hMyComputer; //Add as children of My Computer
		tvInsert.item.iImage = driveIconIndex;
		tvInsert.item.iSelectedImage = driveIconIndex;
		tvInsert.item.pszText = g_Drive->getDisplayName(i); //Get volume label
		tvInsert.item.lParam = (LPARAM)g_Drive->getDriveLetter(i);
		HTREEITEM hDrive = TreeView_InsertItem(m_hTreeView, &tvInsert);

		loadTreeviewItemAt(hDrive, getPath(hDrive, m_hTreeView), m_hTreeView);
	}

	//Expand and select My Computer
	TreeView_Expand(m_hTreeView, hMyComputer, TVE_EXPAND);
	TreeView_SelectItem(m_hTreeView, hMyComputer);
}


void loadMyComputerToListView(DriveHelper *drive, HWND m_hListView)
{
	//Init column of Listview
	initListviewColumn(m_hListView, LVCOL_DRIVE_TYPE);
	LV_ITEM lv;

	for (int i = 0; i < drive->getCount(); ++i)
	{
		//Let ListView know that we'r going to change item text, image and param
		lv.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;

		//Load Label name and default icon to first column
		lv.iItem = i;
		lv.iImage = DEFAULT_ICON_INDEX;
		lv.iSubItem = 0;
		lv.pszText = drive->getDisplayName(i);
		lv.lParam = (LPARAM)drive->getDriveLetter(i);
		ListView_InsertItem(m_hListView, &lv);

		//
		lv.mask = LVIF_TEXT;

		//Load Type of directory to second column
		lv.iSubItem = 1;
		lv.pszText = drive->getDriveType(i);
		ListView_SetItem(m_hListView, &lv);

		//Load total size to third column
		lv.iSubItem = 2;
		if (wcscmp(drive->getDriveType(i), CD_ROM) != 0)
			lv.pszText = drive->getTotalSize(i);
		else
			lv.pszText = NULL;
		ListView_SetItem(m_hListView, &lv);

		//Load Free Space to last column
		lv.iSubItem = 3;
		if (wcscmp(drive->getDriveType(i), CD_ROM) != 0)
			lv.pszText = drive->getFreeSpace(i);
		else
			lv.pszText = NULL;

		//Set
		ListView_SetItem(m_hListView, &lv);
	}
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
				loadTreeviewItemAt(hCurrSelectedChild, getPath(hCurrSelectedChild, m_hTreeView), m_hTreeView);
			}
		} 
		while (hCurrSelectedChild = TreeView_GetNextSibling(m_hTreeView, hCurrSelectedChild));

		/*
		//Another way
		//This is quite a mess -_-

		//HTREEITEM hCurrSelectedChildOfChild = TreeView_GetChild(m_hTreeView, hCurrSelectedChild);
		if (hCurrSelectedChildOfChild == NULL)
		{
			loadTreeviewItemAt(hCurrSelectedChild, getPath(hCurrSelectedChild, m_hTreeView), FALSE, m_hTreeView);

			while (hCurrSelectedChild = TreeView_GetNextSibling(m_hTreeView, hCurrSelectedChild))
			{
				if (TreeView_GetChild(m_hTreeView, hCurrSelectedChild) == NULL)
				{
					loadTreeviewItemAt(hCurrSelectedChild, getPath(hCurrSelectedChild, m_hTreeView), FALSE, m_hTreeView);
				}
			}
		}
		*/
	}
	else
	{
		//Possitively will not happen :D
		loadTreeviewItemAt(hCurrSelected, getPath(hCurrSelected, m_hTreeView), m_hTreeView);
	}
}

void loadTreeviewItemAt(HTREEITEM &hParent, LPCWSTR path, HWND m_hTreeView)
{
	//Get path
	TCHAR buffer[MAX_PATH_LEN];
	StrCpy(buffer, path); //Copy the path of item (include drive letter path)
	StrCat(buffer, _T("\\*")); //Add to find all item in directory

	//Insert new item to TreeView
	TV_INSERTSTRUCT tvInsert;
	tvInsert.hParent = hParent;
	tvInsert.hInsertAfter = TVI_SORT;
	tvInsert.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvInsert.item.iImage = DEFAULT_ICON_INDEX;
	tvInsert.item.iSelectedImage = DEFAULT_ICON_INDEX;

	WIN32_FIND_DATA ffd; //Contains information about the file that is found by Find first file and Find next file
	HANDLE hFind = FindFirstFileW(buffer, &ffd);

	//If the function fails or fails to locate files from the search string
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	TCHAR* folderPath;

	do 
	{
		DWORD fileAttribute = ffd.dwFileAttributes;
		if ((fileAttribute & FILE_ATTRIBUTE_DIRECTORY) //Get only directory and folder
			&& (fileAttribute != FILE_ATTRIBUTE_HIDDEN) //Not hidden
			&& (_tcscmp(ffd.cFileName, _T(".")) != 0) && (_tcscmp(ffd.cFileName, _T("..")) != 0)) //Ignore . (curr dir) and .. (parent dir)
		{
			//Set file name
			tvInsert.item.pszText = ffd.cFileName;
			folderPath = new TCHAR[wcslen(path) + wcslen(ffd.cFileName) + 2];

			//Set path
			StrCpy(folderPath, path);
			if (wcslen(path) != 3)
				StrCat(folderPath, _T("\\"));
			StrCat(folderPath, ffd.cFileName);

			tvInsert.item.lParam = (LPARAM)folderPath;

			HTREEITEM hItem = TreeView_InsertItem(m_hTreeView, &tvInsert);
		}
	} 
	while (FindNextFileW(hFind, &ffd));
	
}

void loadListviewItemAt(LPCWSTR path, HWND m_hParent, HWND m_hListView, DriveHelper *drive)
{
	//If path is NULL, quit
	if (path == NULL)
		return;

	LV_ITEM lv;

	if (_tcscmp(path, _T("Desktop")) == 0)
	{
		//Load Desktop to Listview (My Computer)
		initListviewColumn(m_hListView, LVCOL_FOLDER_TYPE);

		lv.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		lv.iItem = 0;
		lv.iSubItem = 0;
		lv.pszText = _T("My Computer");
		lv.iImage = IDI_MYCOMPUTER;
		lv.lParam = (LPARAM)_T("MyComputer");
		ListView_InsertItem(m_hListView, &lv); //Inserts a new item in a list-view control
	}
	else if (_tcscmp(path, _T("MyComputer")) == 0)
	{
		//Load My Computer to Listview (Drives, Volume,..)
		initListviewColumn(m_hListView, LVCOL_DRIVE_TYPE);
		
		for (int i = 0; i < drive->getCount(); ++i)
		{
			//Let Listview know that we gonna change it's text, image and param
			lv.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
			lv.iItem = i;
			lv.iImage = DEFAULT_ICON_INDEX;

			//Add label of drives or volume
			lv.iSubItem = 0;
			lv.pszText = drive->getDisplayName(i);
			lv.lParam = (LPARAM)drive->getDriveLetter(i);
			ListView_InsertItem(m_hListView, &lv);

			//Load (Type, Size, Free Space)
			lv.mask = LVIF_TEXT;

			//Load Drives's Type to second column
			lv.iSubItem = 1;
			lv.pszText = drive->getDriveType(i);
			ListView_SetItem(m_hListView, &lv); //Sets some or all of a list - view item's attributes.

			//Load size to third column
			lv.iSubItem = 2;
			if (wcscmp(drive->getDriveType(i), CD_ROM) != 0)
				lv.pszText = drive->getTotalSize(i);
			else
				lv.pszText = NULL;

			ListView_SetItem(m_hListView, &lv);

			//Load FreeSpace to last column
			lv.iSubItem = 3;
			if (wcscmp(drive->getDriveType(i), CD_ROM) != 0)
				lv.pszText = drive->getFreeSpace(i);
			else
				lv.pszText = NULL;

			ListView_SetItem(m_hListView, &lv);
		}
	}
	else
		loadDirItemToLisview(g_hWnd,m_hListView,path);
}


void loadOrExecSelected(HWND m_hListView)
{
	LPCWSTR filePath = getPath(m_hListView,ListView_GetSelectionMark(m_hListView));

	WIN32_FIND_DATA fd;

	//Retrieves attributes for a specified file or directory.
	if (GetFileAttributesEx(filePath, GetFileExInfoStandard, &fd) != 0)
	{
		//Check whether it's folder or directory
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			//Delete and reload item in Listview
			ListView_DeleteAllItems(m_hListView);
			loadDirItemToLisview(g_hWnd, m_hListView, filePath);
		}
		else 
		{
			//If it's file -> run it
			//ShellExecute is a function to Open specified file or folder with lpOperation _T("open")
			//specify "SW_SHOWNORMAL" flag for displaying the window for the first time
			ShellExecute(NULL, _T("open"), filePath, NULL, NULL, SW_SHOWNORMAL);
		}
	}
}

void loadDirItemToLisview(HWND m_hParent, HWND m_hListView, LPCWSTR path)
{
	initListviewColumn(m_hListView, LVCOL_FOLDER_TYPE);
	TCHAR buffer[10240];

	//Copy path to buffer
	StrCpy(buffer, path);

	if (wcslen(path) == 3)
		StrCat(buffer, _T("*"));
	else
		StrCat(buffer, _T("\\*"));

	//Variables
	WIN32_FIND_DATA fd; //Contains information about the file that is found by the FindFirstFile or FindNextFile function
	HANDLE hFind = INVALID_HANDLE_VALUE;
	LV_ITEM lv;
	TCHAR* temporaryPath;
	int itemIndex = 0;


	//Find file and folder in this directory
	//Get search handle to search folder 
	hFind = FindFirstFileW(buffer, &fd);

	//If the function fails or fails to locate files from the search string
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	//Iterator
	do 
	{
		//Get only folder
		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
			((fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != FILE_ATTRIBUTE_HIDDEN) &&
			(_tcscmp(fd.cFileName, _T(".")) != 0) && (_tcscmp(fd.cFileName, _T("..")) != 0)) //Ignore . (curr dir) and .. (parent dir)
		{
			//Get path of this folder
			temporaryPath = new TCHAR[wcslen(path) + wcslen(fd.cFileName) + 2];
			StrCpy(temporaryPath, path);

			if (wcslen(path) != 3)
				StrCat(temporaryPath, _T("\\"));

			StrCat(temporaryPath, fd.cFileName);


			//Add name and path to first column
			//Name: fd.cFileName
			//Path: (LPARAM)temporaryPath
			lv.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
			lv.iItem = itemIndex;
			lv.iSubItem = 0;
			lv.pszText = fd.cFileName;
			lv.iImage = DEFAULT_ICON_INDEX;
			lv.lParam = (LPARAM)temporaryPath;
			ListView_InsertItem(m_hListView, &lv);

			//Second column is Date Modified
			//fd.ftLastWriteTime is the number of 100-nanosecond intervals since January 1, 1601 (UTC)
			ListView_SetItemText(m_hListView, itemIndex, 1, convertTimeStampToString(fd.ftLastWriteTime)); //Changes the text of a list - view item or subitem

			//Add "File folder" value to Third column
			ListView_SetItemText(m_hListView, itemIndex, 2, _T("File folder"));

			//Last column is Size
			//Let check Explorer whether it show size of file or folder

			//Increase the index
			itemIndex++;
		}

		//Continues a file search from a previous call to the FindFirstFileW function
		//Return non-zero if successfully found, otherwise return zero
	} 
	while (FindNextFileW(hFind, &fd));

	
	//Get all file in this directory
	//Get search handle to search file 
	hFind = FindFirstFileW(buffer, &fd);
	
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	//Iterator
	do 
	{
		//Ignore all Directory and Folder
		if (((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) &&
			((fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != FILE_ATTRIBUTE_SYSTEM) &&
			((fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != FILE_ATTRIBUTE_HIDDEN))
		{
			//Get file path
			temporaryPath = new TCHAR[wcslen(path) + wcslen(fd.cFileName) + 2];
			StrCpy(temporaryPath, path);

			if (wcslen(path) != 3)
				StrCat(temporaryPath, _T("\\"));

			StrCat(temporaryPath, fd.cFileName);

			//Add name and path to first column
			lv.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
			lv.iItem = itemIndex;
			lv.iSubItem = 0;
			lv.pszText = fd.cFileName;
			lv.lParam = (LPARAM)temporaryPath;

			ListView_InsertItem(m_hListView, &lv);

			//Second column is Date Modified
			ListView_SetItemText(m_hListView, itemIndex, 1, convertTimeStampToString(fd.ftLastWriteTime));

			//Third column is Type
			//ListView_SetItemText(m_hListView, itemIndex, 2, _GetType(fd));

			//Last column is Size
			DWORD fileSizeLow = fd.nFileSizeLow; //The low-order DWORD value of the file size, in bytes
			ListView_SetItemText(m_hListView, itemIndex, 3, CDriveSize::convertByteToStringSize(fileSizeLow));

			itemIndex++;
		}
	} 
	while (FindNextFileW(hFind, &fd));
	
}

void initListviewColumn(HWND m_hListView, int type)
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
	
	//The FILETIME Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
	//To display the FILETIME in a meaningful way, you first need to convert it to a SYSTEMTIME
	FileTimeToSystemTime(&ftLastWrite, &st);
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_AUTOLAYOUT, &st, NULL,
		(LPWSTR)szLocalDate, 255);
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, (LPWSTR)szLocalTime, 255);

	//Concat to string
	wsprintf(buffer, L"%s %s", szLocalDate, szLocalTime);

	return buffer;
}