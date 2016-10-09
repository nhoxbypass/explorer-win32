#pragma once
class CDriveSize
{
private:
	__int64 mTotalSize;
	__int64 mFreeSpace;

	
public:
	CDriveSize();
	CDriveSize(__int64 totalSize, __int64 freeSpace);
	~CDriveSize();

	static LPWSTR Convert(__int64 nSize);

	LPWSTR getTotalSize();
	LPWSTR getFreeSpace();

};

