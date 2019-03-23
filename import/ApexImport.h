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
#include "resource.h"
#include <tchar.h>
#include <string>
#include <windows.h>
#include <matrix3.h>
#include "MAXex/win/CFGMacros.h"

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

static const Matrix3 corMat = {
	Point3{ -1.f,0.f,0.f },
	Point3{ 0.f,0.f,1.f },
	Point3{ 0.f,1.f, 0.f },
	Point3{ 0.f, 0.f, 0.f } };