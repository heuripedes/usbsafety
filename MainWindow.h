//---------------------------------------------------------------------------

#ifndef MainWindowH
#define MainWindowH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TMainWindowForm : public TForm
{
__published:	// IDE-managed Components
  TLabel *lblMessage;
  TButton *btnExit;
  TButton *btnHide;
  TTrayIcon *tryIcon;
  void __fastcall btnExitClick(TObject *Sender);
  void __fastcall btnHideClick(TObject *Sender);
  void __fastcall tryIconClick(TObject *Sender);
  void __fastcall FormCreate(TObject *Sender);
private:	// User declarations
	void __fastcall SearchAndDestroy(UnicodeString& Path);
  UnicodeString __fastcall DriveLetterFromMask(ULONG unitmask);
  bool __fastcall IsBadFile(UnicodeString& FileName) const;
  bool __fastcall MustStayHidden(UnicodeString& Path) const;
  UnicodeString __fastcall DetectMaliciousDirDupe(UnicodeString& Path) const;
  void _fastcall SetDisplayMessage(UnicodeString Status);
public:		// User declarations
	__fastcall TMainWindowForm(TComponent* Owner);
  virtual void __fastcall WndProc(TMessage &Msg);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainWindowForm *MainWindowForm;
//---------------------------------------------------------------------------
#endif
