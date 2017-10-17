program DisplayControl;

{$APPTYPE CONSOLE}
{$R *.res}

uses
  Windows, SysUtils, Registry;

const
  EDD_GET_DEVICE_INTERFACE_NAME = 1;
  ENUM_REGISTRY_SETTINGS = DWORD(-2);

procedure ScreenEnable(dwIndex: integer);
var
  Display: TDisplayDevice;
  DevMode: TDevMode;
begin
  Display.cb:=SizeOf(TDisplayDevice);
  EnumDisplayDevices(nil, dwIndex, Display, EDD_GET_DEVICE_INTERFACE_NAME);
  EnumDisplaySettings(PChar(@Display.DeviceName[0]), ENUM_REGISTRY_SETTINGS, DevMode);
  DevMode.dmFields:=DM_BITSPERPEL or DM_PELSWIDTH or DM_PELSHEIGHT or DM_DISPLAYFREQUENCY or DM_DISPLAYFLAGS or DM_POSITION;
  if (Display.StateFlags and DISPLAY_DEVICE_PRIMARY_DEVICE) <> DISPLAY_DEVICE_PRIMARY_DEVICE then begin
    ChangeDisplaySettingsEx(PChar(@Display.DeviceName[0]), DevMode, 0, CDS_UPDATEREGISTRY or CDS_NORESET, nil);
    ChangeDisplaySettingsEx(nil, PDevMode(nil)^, 0, 0, nil);
  end;
end;

procedure ScreenDisable(dwIndex: integer);
var
  Display: TDisplayDevice;
  DevMode: TDevMode;
begin
  Display.cb:=SizeOf(TDisplayDevice);
  EnumDisplayDevices(nil, dwIndex, Display, EDD_GET_DEVICE_INTERFACE_NAME);
  ZeroMemory(@DevMode, SizeOf(TDevMode));
  DevMode.dmSize:=SizeOf(TDevMode);
  DevMode.dmBitsPerPel:=32;
  DevMode.dmFields:=DM_BITSPERPEL or DM_PELSWIDTH or DM_PELSHEIGHT or DM_DISPLAYFREQUENCY or DM_DISPLAYFLAGS or DM_POSITION;
  if (Display.StateFlags and DISPLAY_DEVICE_PRIMARY_DEVICE) <> DISPLAY_DEVICE_PRIMARY_DEVICE then begin
    ChangeDisplaySettingsEx(PChar(@Display.DeviceName[0]), DevMode, 0, CDS_UPDATEREGISTRY or CDS_NORESET, nil);
    ChangeDisplaySettingsEx(nil, PDevMode(nil)^, 0, 0, nil);
  end;
end;

var
  Reg: TRegistry;
begin
  Reg:=TRegistry.Create;
  Reg.RootKey:=HKEY_CURRENT_USER;
  if Reg.OpenKey('\Software\TrueOpenVR', false) = true then begin
    if AnsiLowerCase(ParamStr(1)) = '/on' then ScreenEnable(Reg.ReadInteger('ScreenIndex') - 1);
    if AnsiLowerCase(ParamStr(1)) = '/off' then ScreenDisable(Reg.ReadInteger('ScreenIndex') - 1);
  end;
  Reg.CloseKey;
  Reg.Free;
end.
