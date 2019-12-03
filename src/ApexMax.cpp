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

#include "ApexMax.h"
#include <IPathConfigMgr.h>
#include <3dsmaxport.h>
#include <iparamm2.h>

const Matrix3 corMat = 
{
	Point3{ -1.f,0.f,0.f },
	Point3{ 0.f,0.f,1.f },
	Point3{ 0.f,1.f, 0.f },
	Point3{ 0.f, 0.f, 0.f } 
};

const TCHAR _name[] = _T("Apex Tool");
const TCHAR _info[] = _T("\n" ApexMax_COPYRIGHT "\nVersion " ApexMax_VERSION);
const TCHAR _license[] = _T("Apex Tool uses ApexLib, Copyright(C) 2014-2019 Lukas Cone.");
const TCHAR _homePage[] = _T("https://lukascone.wordpress.com/2019/02/22/apex-engine-plugin");

#include "MAXex/win/AboutDlg.h"

ApexImport::ApexImport(): CFGFile(nullptr), hWnd(nullptr),
flags(IDC_CH_DEBUGNAME_checked, IDC_CH_DUMPMATINFO_checked), IDConfigValue(IDC_EDIT_SCALE)(145.f) {}

void ApexImport::BuildCFG()
{
	cfgpath = IPathConfigMgr::GetPathConfigMgr()->GetDir(APP_PLUGCFG_DIR);
	cfgpath.append(_T("\\ApexImpSettings.ini"));
	CFGFile = cfgpath.c_str();
}

void ApexImport::LoadCFG()
{
	BuildCFG();
	TCHAR buffer[16];
	GetCFGChecked(IDC_CH_DEBUGNAME);
	GetCFGValue(IDC_EDIT_SCALE);
	GetCFGChecked(IDC_CH_DUMPMATINFO);
	GetCFGChecked(IDC_CH_CLEARLISTENER);
	GetCFGChecked(IDC_CH_FORCESTDMAT);
	GetCFGChecked(IDC_CH_ENABLEVIEWMAT);
}

void ApexImport::SaveCFG()
{
	BuildCFG();

	SetCFGChecked(IDC_CH_DEBUGNAME);
	SetCFGChecked(IDC_CH_DUMPMATINFO);
	SetCFGChecked(IDC_CH_CLEARLISTENER);
	SetCFGChecked(IDC_CH_FORCESTDMAT);
	SetCFGChecked(IDC_CH_ENABLEVIEWMAT);

	TCHAR buffer[16];
	SetCFGValue(IDC_EDIT_SCALE);

	WriteText(hkpresetgroup, _T("Apex Engine"), CFGFile, _T("Name"));
	WriteText(hkpresetgroup, _T("bsk|ban"), CFGFile, _T("Extensions"));
	WriteValue(hkpresetgroup, IDConfigValue(IDC_EDIT_SCALE), CFGFile, buffer, _T("Scale"));
	WriteCorrectionMatrix(corMat, CFGFile, buffer);
	
}

static INT_PTR CALLBACK DialogCallbacks(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	ApexImport *imp = DLGetWindowLongPtr<ApexImport*>(hWnd);

	switch (message) {
	case WM_INITDIALOG:
		CenterWindow(hWnd, GetParent(hWnd));
		imp = reinterpret_cast<ApexImport*>(lParam);
		DLSetWindowLongPtr(hWnd, lParam);
		imp->hWnd = hWnd;
		imp->LoadCFG();
		SetupIntSpinner(hWnd, IDC_SPIN_SCALE, IDC_EDIT_SCALE, 0, 5000, imp->IDC_EDIT_SCALE_value);
		SetWindowText(hWnd, _T("Apex Import v" ApexMax_VERSION));
		return TRUE;

	case WM_CLOSE:
		EndDialog(hWnd, 0);
		imp->SaveCFG();
		return 1;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BT_DONE:
			EndDialog(hWnd, 1);
			imp->SaveCFG();
			return 1;
		case IDC_BT_ABOUT:
			ShowAboutDLG(hWnd);
			return 1;
		case IDC_BT_CANCEL:
			EndDialog(hWnd, 0);
			imp->SaveCFG();
			return 1;

			MSGCheckbox(IDC_CH_DEBUGNAME); break;
			MSGCheckbox(IDC_CH_DUMPMATINFO); break;
			MSGCheckbox(IDC_CH_CLEARLISTENER); break;
			MSGCheckbox(IDC_CH_FORCESTDMAT); break;
			MSGCheckbox(IDC_CH_ENABLEVIEWMAT); break;
		}

	case CC_SPINNER_CHANGE:
		switch (LOWORD(wParam))
		{
		case IDC_SPIN_SCALE:
			imp->IDC_EDIT_SCALE_value = reinterpret_cast<ISpinnerControl *>(lParam)->GetFVal();
			break;
		}

	}
	return 0;
}

int ApexImport::SpawnDialog()
{
	return DialogBoxParam(hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		GetActiveWindow(),
		DialogCallbacks, (LPARAM)this);
}