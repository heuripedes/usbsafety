object MainWindowForm: TMainWindowForm
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = 'Usb Safety'
  ClientHeight = 76
  ClientWidth = 331
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object lblMessage: TLabel
    AlignWithMargins = True
    Left = 0
    Top = 8
    Width = 327
    Height = 33
    Alignment = taCenter
    AutoSize = False
    Caption = 'Ready'
    Layout = tlCenter
  end
  object btnExit: TButton
    Left = 8
    Top = 47
    Width = 75
    Height = 25
    Caption = 'Exit'
    TabOrder = 0
    OnClick = btnExitClick
  end
  object btnHide: TButton
    Left = 252
    Top = 47
    Width = 75
    Height = 25
    Caption = 'Hide'
    Default = True
    TabOrder = 1
    OnClick = btnHideClick
  end
  object btnAutorun: TButton
    Left = 120
    Top = 47
    Width = 105
    Height = 25
    Caption = 'Turn off autorun'
    TabOrder = 2
    Visible = False
    OnClick = btnAutorunClick
  end
  object tryIcon: TTrayIcon
    BalloonTimeout = 1000
    Visible = True
    OnClick = tryIconClick
    Left = 10
  end
  object XPManifest1: TXPManifest
    Left = 50
  end
end
