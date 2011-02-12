//---------------------------------------------------------------------------

#include <vcl.h>

#include <windows.h>
#include <dbt.h>

#pragma hdrstop

#include "MainWindow.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

/*
fonte: http://support.microsoft.com/kb/967715

Para desativar autorun: (reproduzir aqui)
1 .Clique em Iniciar, em Executar, digite regedit na caixa Abrir e clique em OK.
2. Localize e clique na seguinte entrada do Registro:
   HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\policies\Explorer\NoDriveTypeAutorun
3. Clique com o botão direito em NoDriveTypeAutoRun e clique em Modificar.
4. Na caixa Dados do valor, digite 0xFF para desabilitar todos os tipos de unidades.
   Ou use um valor diferente como descrito na seção "Como desabilitar seletivamente
   recursos específicos do Autorun" para desabilitar seletivamente unidades específicas.
5. Clique em OK e saia do Editor de Registro.
6. Reinicie o computador.
*/
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

  return UnicodeString(char(character + 'A'));
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

  /* Folder.exe infection must be treated separatedly */
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
    SetDisplayMessage("Não foi possivel ler " + Path);
    return;
  }

  do {
    UnicodeString fpath = Path + "\\" + sr.Name;
    UINT fattr = GetFileAttributesW(fpath.w_str());

    if (fattr == INVALID_FILE_ATTRIBUTES) {
      continue;
    }

    SetDisplayMessage("Analizando " + fpath);

    /* é um diretorio oculto, procure um link de mesmo nome */
    if (fattr & FILE_ATTRIBUTE_DIRECTORY) {
      UnicodeString sDupe;
      int iMaxFailures = 3;

      while (iMaxFailures && (sDupe = DetectMaliciousDirDupe(fpath)) != NULL) {
        if (DeleteFileW(sDupe.w_str())) {
          SetDisplayMessage(sDupe + " excluído.");
        } else {
          iMaxFailures--;
        }
      }
    } else {
      if (IsBadFile(sr.Name)) {
        DeleteFileW(fpath.w_str());
        SetDisplayMessage(fpath + " excluído.");
      }
    }

    if (!MustStayHidden(sr.Name)) {
      UINT before = fattr;
      if (fattr & FILE_ATTRIBUTE_HIDDEN) {
        fattr -= FILE_ATTRIBUTE_HIDDEN;
      }

      if (fattr & FILE_ATTRIBUTE_SYSTEM) {
        fattr -= FILE_ATTRIBUTE_SYSTEM;
      }

      // minimize writes
      if (before != fattr) {
        SetFileAttributesW(fpath.w_str(), fattr);
        SetDisplayMessage(fpath + " restaurado.");
      }
    }
  } while (FindNext(sr) == 0);

  FindClose(sr);

  SetDisplayMessage("Pronto");
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
  PDEV_BROADCAST_VOLUME lpdbv = NULL;
  UnicodeString devunit;
  char szMsg[80];

  if (Msg.Msg != WM_DEVICECHANGE) {
    goto exitsub;
  }


  switch (Msg.WParam) {
  case DBT_DEVICEARRIVAL:
    if (lpdb->dbch_devicetype != DBT_DEVTYP_VOLUME) {
      goto exitsub;
    }

    lpdbv = PDEV_BROADCAST_VOLUME(lpdb);

    devunit = DriveLetterFromMask(lpdbv->dbcv_unitmask) + ":";

    SetDisplayMessage(devunit +  " foi montado.");

    SearchAndDestroy(devunit);

    break;
  case DBT_DEVICEREMOVECOMPLETE:
    if (lpdb->dbch_devicetype != DBT_DEVTYP_VOLUME) {
      goto exitsub;
    }

    lpdbv = PDEV_BROADCAST_VOLUME(lpdb);

    devunit = DriveLetterFromMask(lpdbv->dbcv_unitmask) + ":";
    SetDisplayMessage(devunit + " foi desmontado.");

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
 SearchAndDestroy(UnicodeString("g:"));
}
//---------------------------------------------------------------------------

