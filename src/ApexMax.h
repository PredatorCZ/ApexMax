/*  Apex Tool for 3ds Max
	Copyright(C) 2014-2019 Lukas Cone

	This program is free software : you can redistribute it and / or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once
#include "MAXex/3DSMaxSDKCompat.h"
#include <istdplug.h>
#include <iparamb2.h>
#include <iparamm2.h>
#include <maxtypes.h>
#include <maxscript/maxscript.h>
#include "MAXex/win/CFGMacros.h"

#include <impexp.h>
#include <direct.h>
#include <commdlg.h>

#include "resource.h"
#include <tchar.h>
#include <string>
#include <windows.h>
#include <matrix3.h>

#include "ApexApi.h"

#define APEXMAX_VERSION_MAJOR 1
#define APEXMAX_VERSION_MINOR 6

#define APEXMAX_VERSION APEXMAX_VERSION_MAJOR##.##APEXMAX_VERSION_MINOR
static constexpr int APEXMAX_VERSIONINT = APEXMAX_VERSION_MAJOR * 100 + APEXMAX_VERSION_MINOR;

extern HINSTANCE hInstance;

class ApexImport
{
public:
	TSTRING cfgpath;
	const TCHAR *CFGFile;
	HWND hWnd;

	enum ConfigBoolean
	{
		IDConfigBool(IDC_CH_DEBUGNAME),
		IDConfigBool(IDC_CH_DUMPMATINFO),
		IDConfigBool(IDC_CH_CLEARLISTENER),
		IDConfigBool(IDC_CH_FORCESTDMAT),
		IDConfigBool(IDC_CH_ENABLEVIEWMAT),
	};

	NewIDConfigValue(IDC_EDIT_SCALE);

	EnumFlags<uchar, ConfigBoolean> flags;

	ApexImport();
	~ApexImport() {}
	int LoadModel();

	void BuildCFG();
	void LoadCFG();
	void SaveCFG();

	int SpawnDialog();

};

void ShowAboutDLG(HWND hWnd);

extern const Matrix3 corMat;