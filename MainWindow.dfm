object MainWindowForm: TMainWindowForm
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Usb Safety'
  ClientHeight = 80
  ClientWidth = 345
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object lblMessage: TLabel
    AlignWithMargins = True
    Left = 0
    Top = 8
    Width = 341
    Height = 33
    Alignment = taCenter
    AutoSize = False
    Caption = 'Pronto'
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
    Left = 266
    Top = 47
    Width = 75
    Height = 25
    Caption = 'Hide'
    Default = True
    TabOrder = 1
    OnClick = btnHideClick
  end
  object tryIcon: TTrayIcon
    BalloonTimeout = 1000
    Visible = True
    OnClick = tryIconClick
    Left = 10
  end
end
