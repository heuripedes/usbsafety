//---------------------------------------------------------------------------

#include <vcl.h>

#include <windows.h>
#include <dbt.h>

#pragma hdrstop

#include "MainWindow.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TMainWindowForm *MainWindowForm;
//---------------------------------------------------------------------------
__fastcall TMainWindowForm::TMainWindowForm(TComponent* Owner)
  : TForm(Owner)
{
}
//---------------------------------------------------------------------------

inline char* ToCString(UnicodeString& str) {
  return AnsiString(str).c_str();
}
//---------------------------------------------------------------------------

UnicodeString __fastcall TMainWindowForm::DriveLetterFromMask(ULONG unitmask)
{
  char character;

  for (character = 0; character < 26; ++character) {
    if (unitmask & 0x1)
      break;
    unitmask = unitmask >> 1;
  }

  return char(character + 'A');
}
//---------------------------------------------------------------------------

bool __fastcall TMainWindowForm::IsBadFile(UnicodeString& FileName) const
{
#include "BadFilesList.h"

  static const UnicodeString BadSuffixes[] = {
    ".jpg.exe",
    ".jpg.scr",
    ".gif.exe",
    ".png.exe",
    ".txt.com",
    ".dll.vbs",
    NULL
  };

  int i = 0;

  while (BadFiles[i] != NULL) {
    if (FileName.CompareIC(BadFiles[i]) == 0) {
      return true;
    }
    ++i;
  }

  i = 0;
  while (BadSuffixes[i] != NULL) {
    if (FileName.Pos(BadSuffixes[i])) {
      return true;
    }
    ++i;
  }

  return false;
}
//---------------------------------------------------------------------------

bool __fastcall TMainWindowForm::MustStayHidden(UnicodeString& FileName) const
{
  static const UnicodeString MustStayHidden[] = {"RECYCLER", NULL};

  // Linux: GVFS trash folders
  if (FileName.Pos(".Trash")) {
    return true;
  }

  // MacOS: Spotlight's index folder
  if (FileName.Pos("_MACOSX")) {
    return true;
  }

  int i = 0;

  while (MustStayHidden[i] != NULL) {
    if (FileName.CompareIC(MustStayHidden[i]) == 0) {
      return true;
    }
    ++i;
  }

  return false;
}
//---------------------------------------------------------------------------

UnicodeString __fastcall TMainWindowForm::DetectMaliciousDirDupe(UnicodeString& Path) const
{
  static const UnicodeString MaliciousExts[] = {".lnk", NULL};
  UINT iAttr;
  UnicodeString sPath;

  int i = 0;

  while (MaliciousExts[i] != NULL) {
    sPath = Path + MaliciousExts[i];
    iAttr = GetFileAttributesW(sPath.w_str());

    // file found or accessible
    if (iAttr != INVALID_FILE_ATTRIBUTES) {
      return sPath;
    }

    ++i;
  }

  // Folder.exe infection must be treated separatedly
  sPath = Path + ".exe";
  iAttr = GetFileAttributesW(sPath.w_str());

  if (iAttr != INVALID_FILE_ATTRIBUTES) {
    HANDLE hFile = CreateFileW(sPath.w_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE && GetFileSize(hFile, NULL) < 0x100000) {
      CloseHandle(hFile);
      return sPath;
    }
  }

  return NULL;
}
//---------------------------------------------------------------------------

void __fastcall TMainWindowForm::SearchAndDestroy(UnicodeString& Path)
{
  int attr =
    faDirectory | faAnyFile | faHidden | faSysFile | faNormal | faReadOnly;

  TSearchRec sr;

  if (FindFirst(Path + "\\*", attr, sr)) {
    SetDisplayMessage("Could not read " + Path);
    FindClose(sr);
    return;
  }

  SetDisplayMessage("Analyzing " + Path);

  do {
    UnicodeString& sFileName = sr.Name;
    UnicodeString sFilePath  = Path + "\\" + sFileName;

    UINT iFileAttr = GetFileAttributesW(sFilePath.w_str());

    if (iFileAttr == INVALID_FILE_ATTRIBUTES) {
      continue;
    }

    SetDisplayMessage("Analyzing " + sFilePath);

    // find suspicious files named as Folder.ext
    if (iFileAttr & FILE_ATTRIBUTE_DIRECTORY) {
      UnicodeString sDupe;
      int iMaxFailures = 3;

      while (iMaxFailures && (sDupe = DetectMaliciousDirDupe(sFilePath)) != NULL) {
        if (DeleteFileW(sDupe.w_str())) {
          SetDisplayMessage(sDupe + " removed.");
        } else {
          iMaxFailures--;
        }
      }
    } else {
      // find suspicious files e.g. autorun.inf or random.jpg.exe
      if (IsBadFile(sFileName)) {
        DeleteFileW(sFilePath.w_str());
        SetDisplayMessage(sFilePath + " removed.");
      }
    }

    // some files or directories don't need to be visible
    if (!MustStayHidden(sFileName)) {
      UINT before = iFileAttr;
      if (iFileAttr & FILE_ATTRIBUTE_HIDDEN) {
        iFileAttr -= FILE_ATTRIBUTE_HIDDEN;
      }

      if (iFileAttr & FILE_ATTRIBUTE_SYSTEM) {
        iFileAttr -= FILE_ATTRIBUTE_SYSTEM;
      }

      // minimize writes
      if (before != iFileAttr) {
        SetFileAttributesW(sFilePath.w_str(), iFileAttr);
        SetDisplayMessage(sFilePath + " restaured.");
      }
    }
  } while (FindNext(sr) == 0);

  FindClose(sr);

  SetDisplayMessage("Ready");
}
//---------------------------------------------------------------------------

void _fastcall TMainWindowForm::SetDisplayMessage(UnicodeString Status)
{
  lblMessage->Caption = Status;

  if (Visible) {
    Refresh();
  } else {
    tryIcon->BalloonHint = Status;
    tryIcon->ShowBalloonHint();
  }

}
//---------------------------------------------------------------------------

void __fastcall TMainWindowForm::WndProc(TMessage &Msg)
{
  PDEV_BROADCAST_HDR lpdb = PDEV_BROADCAST_HDR(Msg.LParam);
  PDEV_BROADCAST_VOLUME lpdbv;
  UnicodeString devunit;
  char szMsg[80];

  if (Msg.Msg != WM_DEVICECHANGE || lpdb == NULL) {
    goto exitsub;
  }

  if (lpdb->dbch_devicetype != DBT_DEVTYP_VOLUME) {
    goto exitsub;
  }

  switch (Msg.WParam) {
  case DBT_DEVICEARRIVAL:
    lpdbv = PDEV_BROADCAST_VOLUME(lpdb);

    devunit = DriveLetterFromMask(lpdbv->dbcv_unitmask) + ":";

    SetDisplayMessage(devunit +  " mounted.");

    SearchAndDestroy(devunit);

    break;
  case DBT_DEVICEREMOVECOMPLETE:
    lpdbv = PDEV_BROADCAST_VOLUME(lpdb);

    devunit = DriveLetterFromMask(lpdbv->dbcv_unitmask) + ":";
    SetDisplayMessage(devunit + " unmounted.");

    break;
  }

exitsub:
  TForm::WndProc(Msg);
}
//---------------------------------------------------------------------------

void __fastcall TMainWindowForm::btnExitClick(TObject *Sender)
{
  tryIcon->Visible = false;
  Application->Terminate();
}
//---------------------------------------------------------------------------

void __fastcall TMainWindowForm::btnHideClick(TObject *Sender)
{
  Hide();
}
//---------------------------------------------------------------------------

void __fastcall TMainWindowForm::tryIconClick(TObject *Sender)
{
  if (Visible) {
    Hide();
  } else {
    Show();
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainWindowForm::FormCreate(TObject *Sender)
{
  /*DWORD dwCurValue;
  HKEY key;
  DWORD dwType = REG_DWORD;
  DWORD dwSize = sizeof(DWORD);
  LONG status;

//  HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\policies\Explorer\NoDriveTypeAutorun
  status = RegOpenKey(HKEY_CURRENT_USER,
    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\policies\\Explorer",
    &key);

  if (status == ERROR_SUCCESS) {
    status = RegQueryValueEx(key, "NoDriveTypeAutoRun", NULL, &dwType, (LPBYTE)&dwCurValue, &dwSize);

    if (status == ERROR_SUCCESS && dwCurValue != 0xff) {
      SetDisplayMessage("It is recommended that you turn off autorun.");
      ShowMessage(int(dwCurValue));
      //btnAutorun->Show();
    }
  }

  RegCloseKey(key);
  */
 //SearchAndDestroy(UnicodeString("g:"));
}
//---------------------------------------------------------------------------

void __fastcall TMainWindowForm::btnAutorunClick(TObject *Sender)
{
  /* only works with administrative rights */
  /*DWORD dwCurValue = 0xff;
  HKEY key;
  DWORD dwType = REG_DWORD;
  DWORD dwSize = sizeof(DWORD);
  LONG status;

//  HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\policies\Explorer\NoDriveTypeAutorun
  status = RegOpenKey(HKEY_CURRENT_USER,
    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\policies\\Explorer",
    &key);

  if (status == ERROR_SUCCESS) {
    status = RegSetValueEx(key, "NoDriveTypeAutoRun", 0, dwType, (LPBYTE)dwCurValue, dwSize);

    if (status == ERROR_SUCCESS && dwCurValue == 0xff) {
      SetDisplayMessage("Autorun turned off.");
      btnAutorun->Hide();
    }
  }

  RegCloseKey(key);*/
}
//---------------------------------------------------------------------------

