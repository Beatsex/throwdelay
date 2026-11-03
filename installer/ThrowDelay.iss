; Inno Setup script for Throw Delay. Build with:
;   "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer\ThrowDelay.iss
#define AppName "Throw Delay"
#define AppVersion "1.0.0"
#define Publisher "beatsexuell"
#define Artefacts "..\build\ThrowDelay_artefacts\Release"

[Setup]
AppId={{9B5A7C21-4E3D-4C7A-9E51-7D2B6F0A1C33}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#Publisher}
DefaultDirName={autopf}\{#Publisher}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
LicenseFile=..\LICENSE
OutputDir=..\build\installer
OutputBaseFilename=ThrowDelay-{#AppVersion}-Windows
SetupIconFile=..\resources\icon.ico
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
PrivilegesRequired=admin
UninstallDisplayName={#AppName}
UninstallDisplayIcon={app}\icon.ico

[Languages]
Name: "german"; MessagesFile: "compiler:Languages\German.isl"

[Types]
Name: "full"; Description: "Alles"
Name: "custom"; Description: "Auswahl"; Flags: iscustom

[Components]
Name: "vst3";       Description: "VST3-Plugin";       Types: full
Name: "clap";       Description: "CLAP-Plugin";       Types: full
Name: "lv2";        Description: "LV2-Plugin";        Types: full
Name: "standalone"; Description: "Standalone-App";    Types: full

[Files]
Source: "{#Artefacts}\VST3\Throw Delay.vst3\*"; DestDir: "{commoncf64}\VST3\Throw Delay.vst3"; \
  Components: vst3; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#Artefacts}\CLAP\Throw Delay.clap"; DestDir: "{commoncf64}\CLAP"; \
  Components: clap; Flags: ignoreversion
Source: "{#Artefacts}\LV2\Throw Delay.lv2\*"; DestDir: "{commoncf64}\LV2\Throw Delay.lv2"; \
  Components: lv2; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#Artefacts}\Standalone\Throw Delay.exe"; DestDir: "{app}"; \
  Components: standalone; Flags: ignoreversion
; Readme, licence and icon go in for every selection; the icon also marks the
; uninstall entry, which has no exe of its own when only plugins are installed.
Source: "readme.html";          DestDir: "{app}"; Flags: ignoreversion
Source: "..\LICENSE";           DestDir: "{app}"; Flags: ignoreversion
Source: "..\resources\icon.ico"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\Throw Delay.exe"; Components: standalone
Name: "{group}\Anleitung";  Filename: "{app}\readme.html"; IconFilename: "{app}\icon.ico"
Name: "{group}\{#AppName} deinstallieren"; Filename: "{uninstallexe}"; IconFilename: "{app}\icon.ico"

[Run]
Filename: "{app}\readme.html"; Description: "Anleitung öffnen"; \
  Flags: shellexec nowait postinstall skipifsilent
