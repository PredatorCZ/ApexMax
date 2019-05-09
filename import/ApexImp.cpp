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

#include <map>

#include <triobj.h>
#include <ilayermanager.h>
#include <ilayer.h>
#include <iskin.h>
#include "../samples/modifiers/morpher/include/MorpherApi.h"
#include "MeshNormalSpec.h"
#include "IXTexmaps.h"

#include "../ApexMax.h"
#include "ApexImport.h"
#include "AmfMesh.h"
#include "AmfModel.h"
#include "StuntAreas.h"

#include "datas/esstring.h"
#include "datas/masterprinter.hpp"
#include "MAXex/NodeSuffix.h"
#include "datas/allocator_hybrid.hpp"


#define ApexImp_CLASS_ID	Class_ID(0x85965629, 0x96893331)


class ApexImp : public SceneImport, ApexImport
{
public:
	//Constructor/Destructor
	ApexImp();
	virtual ~ApexImp();

	virtual int				ExtCount();					// Number of extensions supported
	virtual const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
	virtual const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
	virtual const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
	virtual const TCHAR *	AuthorName();				// ASCII Author name
	virtual const TCHAR *	CopyrightMessage();			// ASCII Copyright message
	virtual const TCHAR *	OtherMessage1();			// Other message #1
	virtual const TCHAR *	OtherMessage2();			// Other message #2
	virtual unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
	virtual void			ShowAbout(HWND hWnd);	
	// Show DLL's "About..." box
	virtual int				DoImport(const TCHAR *name, ImpInterface *i, Interface *gi, BOOL suppressPrompts = FALSE);	// Import file

	INodeSuffixer LoadMesh(AmfMesh *imsh);
	void LoadSpriteData(AmfMesh *mesh, INode *nde);
	void ApplyDeform(AmfMesh *mesh, INodeSuffixer &nde);
	int LoadModel(IADF * adf);
	int LoadStuntArea(IADF * adf);
};



static class : public ClassDesc2 
{
public:
	virtual int				IsPublic() 							{ return TRUE; }
	virtual void*			Create(BOOL /*loading = FALSE*/) 	{ return new ApexImp(); }
	virtual const TCHAR *	ClassName() 						{ return GetString(IDS_CLASS_NAME); }
	virtual SClass_ID		SuperClassID() 						{ return SCENE_IMPORT_CLASS_ID; }
	virtual Class_ID		ClassID() 							{ return ApexImp_CLASS_ID; }
	virtual const TCHAR*	Category() 							{ return GetString(IDS_CATEGORY); }
	virtual const TCHAR*	InternalName() 						{ return _T("ApexImp"); }				// returns fixed parsable name (scripter-visible name)
	virtual HINSTANCE		HInstance() 						{ return hInstance; }					// returns owning module handle
}apexImpDesc;


ClassDesc2* GetApexImpDesc() { return &apexImpDesc; }

//--- ApexImp -------------------------------------------------------
ApexImp::ApexImp()
{

}

ApexImp::~ApexImp() 
{

}

int ApexImp::ExtCount()
{
	return 4;
}

const TCHAR *ApexImp::Ext(int n)
{		
	switch (n)
	{
	case 1:
		return _T("rbm");
	case 2:
		return _T("rbn");
	case 3:
		return _T("vmodc");
	default:
		break;
	}

	return _T("modelc");
}

const TCHAR *ApexImp::LongDesc()
{
	return _T("Apex Inport");
}
	
const TCHAR *ApexImp::ShortDesc() 
{			
	return _T("Apex Import");
}

const TCHAR *ApexImp::AuthorName()
{			
	return _T("Lukas Cone");
}

const TCHAR *ApexImp::CopyrightMessage() 
{	
	return _T("Copyright (C) 2014-2019 Lukas Cone");
}

const TCHAR *ApexImp::OtherMessage1() 
{		
	return _T("");
}

const TCHAR *ApexImp::OtherMessage2() 
{		
	return _T("");
}

unsigned int ApexImp::Version()
{				
	return 140;
}

void ApexImp::ShowAbout(HWND hWnd)
{			
	ShowAboutDLG(hWnd);
}

Mtl *CreateMaterial(AmfMaterial *material);

static class : public ITreeEnumProc
{
	const MSTR skelNameHint = _T("hkaSkeleton");
	const MSTR boneNameHint = _T("hkaBone");
	const MSTR skelNameExclude = _T("ragdoll");
public:
	std::vector<INode*> bones;

	void RescanBones()
	{
		bones.clear();
		GetCOREInterface7()->GetScene()->EnumTree(this);
	}

	INode *LookupNode(int ID)
	{
		for (auto &b : bones)
			if (b->UserPropExists(boneNameHint))
			{
				int _ID;
				b->GetUserPropInt(boneNameHint, _ID);

				if (_ID == ID)
					return b;
			}

		TSTRING boneName = _T("Bone") + ToTSTRING(ID);
		INode *node = LookupNode(boneName);

		return node;
	}

	INode *LookupNode(TSTRING &boneName)
	{
		INode *node = GetCOREInterface()->GetINodeByName(boneName.c_str());

		if (!node)
		{
			Object *obj = static_cast<Object*>(CreateInstance(HELPER_CLASS_ID, Class_ID(DUMMY_CLASS_ID, 0)));
			node = GetCOREInterface()->CreateObjectNode(obj);
			node->ShowBone(2);
			node->SetWireColor(0x80ff);
			node->SetName(ToBoneName(boneName));
			bones.push_back(node);
		}

		return node;
	}

	int callback(INode *node)
	{
		Object *refobj = node->EvalWorldState(0).obj;

		//if ((refobj->ClassID() == Class_ID(DUMMY_CLASS_ID, 0) || refobj->ClassID() == Class_ID(BONE_CLASS_ID, 0) || refobj->ClassID() == BONE_OBJ_CLASSID))
		//{
			if (node->UserPropExists(skelNameHint))
			{
				MSTR skelName;
				node->GetUserPropString(skelNameHint, skelName); 
				skelName.toLower();

				const int skelNameExcludeSize = skelNameExclude.Length();
				bool found = true;

				for (int s = 0; s < skelNameExcludeSize; s++)
					if (skelName[s] != skelNameExclude[s])
					{
						found = false;
						break;
					}

				if (found)
					return TREE_CONTINUE;

				bones.push_back(node);
			}	
		//}
		
		return TREE_CONTINUE;
	}
}iBoneScanner;

INodeSuffixer ApexImp::LoadMesh(AmfMesh *imsh)
{
	INodeSuffixer nde;

	if (!imsh->properlyLinked)
		return nde;

	TriObject *obj = CreateNewTriObject();
	Mesh *msh = &obj->GetMesh();

	msh->setNumVerts(imsh->Header.vertexCount);
	msh->setNumFaces(imsh->Header.indexCount / 3);

	const int numVerts = msh->numVerts;
	const int numFaces = msh->numFaces;
	int currentMap = 1;	
	int matid = 0;

	for (auto &d : imsh->streamAttributes)
	{
		switch (d.Header.usage)
		{
		case AmfUsage_Position:
		{
			Matrix3 localCorMat = corMat;
			float *packer = reinterpret_cast<float*>(d.Header.packingData);

			if (*packer > FLT_EPSILON)
				localCorMat.Scale({ *packer,*packer,*packer });

			localCorMat.Scale({ IDC_EDIT_SCALE_value,IDC_EDIT_SCALE_value,IDC_EDIT_SCALE_value });

			for (int v = 0; v < numVerts; v++)
			{
				Point3 tmp;
				d.Evaluate(&d, v, &tmp); 
				msh->setVert(v, localCorMat.VectorTransform(tmp));
			}
			break;
		}
		case AmfUsage_Normal:
		case AmfUsage_TangentSpace:
		{
			msh->SpecifyNormals();
			MeshNormalSpec *normalSpec = msh->GetSpecifiedNormals();
			normalSpec->ClearNormals();
			normalSpec->SetNumNormals(numVerts);
			normalSpec->SetNumFaces(numFaces);
			nde.UseNormals();

			for (int v = 0; v < numVerts; v++)
			{
				Point3 temp;
				d.Evaluate(&d, v, &temp);
				normalSpec->Normal(v) = corMat.VectorTransform(temp);
				normalSpec->SetNormalExplicit(v, true);
			}

			for (auto &s : imsh->subMeshes)
			{
				for (int f = 0; f < numFaces; f++)
				{
					MeshNormalFace &normalFace = normalSpec->Face(f);
					USVector tmp;
					s.GetFace(f, tmp);
					normalFace.SpecifyAll();
					normalFace.SetNormalID(0, tmp.X);
					normalFace.SetNormalID(1, tmp.Y);
					normalFace.SetNormalID(2, tmp.Z);
				}
			}
			break;
		}
		case AmfUsage_TextureCoordinate:
		{
			msh->setMapSupport(currentMap, 1);
			msh->setNumMapVerts(currentMap, numVerts);
			msh->setNumMapFaces(currentMap, numFaces);
			nde.AddChannel(currentMap);
			Vector2 packer = *reinterpret_cast<Vector2*>(d.Header.packingData);

			if (!packer.Length())
				packer = Vector2(1.0f, 1.0f);

			Matrix3 uvscale = {
				Point3{ packer.X, 0.f, 0.f },
				Point3{ 0.f, -packer.Y, 0.f },
				Point3{ 0.f, 0.f, 1.f },
				Point3{ 0.f, 1.f, 0.f }
			};
			for (int v = 0; v < numVerts; v++)
			{
				Vector2 temp;
				d.Evaluate(&d, v, &temp);			
				msh->Map(currentMap).tv[v] = uvscale.PointTransform({ temp.X, temp.Y, 0.f });
			}

			currentMap++;
			break;
		}
		case AmfUsage_Color:
		{	
			const bool colorOnly = d.Header.format == AmfFormat_R32_UNIT_UNSIGNED_VEC_AS_FLOAT_c;

			if (colorOnly)
			{
				msh->setMapSupport(0, 1);
				msh->setNumMapVerts(0, numVerts);
				nde.AddColor();

				for (int v = 0; v < numVerts; v++)
				{
					Point3 temp;
					d.Evaluate(&d, v, &temp);
					msh->Map(0).tv[v] = temp;
				}
			}
			else
			{
				msh->setMapSupport(-2, 1);
				msh->setNumMapVerts(-2, numVerts);
				msh->setMapSupport(0, 1);
				msh->setNumMapVerts(0, numVerts);
				nde.AddFullColor();

				for (int v = 0; v < numVerts; v++)
				{
					Vector4 temp;
					d.Evaluate(&d, v, &temp);
					msh->Map(0).tv[v] = reinterpret_cast<Point3&>(temp);
					msh->Map(-2).tv[v] = { temp.W, temp.W, temp.W };
				}
			}
			break;
		}

		default:
			break;
		}
	}

	int currentFaceOffset = 0;

	for (auto &s : imsh->subMeshes)
	{
		const int curNumFaces = s.Header.indexCount / 3;

		for (int f = 0; f < curNumFaces; f++)
		{
			Face &face = msh->faces[currentFaceOffset + f];
			face.setEdgeVisFlags(1, 1, 1);
			USVector tmp;
			s.GetFace(f, tmp);
			face.v[0] = tmp.X;
			face.v[1] = tmp.Y;
			face.v[2] = tmp.Z;
			face.setMatID(matid);

			for (int &i : nde)
				msh->Map(i).tf[currentFaceOffset + f].setTVerts(tmp.X, tmp.Y, tmp.Z);
		}

		currentFaceOffset += curNumFaces;
		matid++;
	}

	msh->InvalidateGeomCache();
	msh->InvalidateTopologyCache();
	nde.node = GetCOREInterface()->CreateObjectNode(obj);
	return nde;
}

void ApexImp::LoadSpriteData(AmfMesh *mesh, INode *nde)
{
	const int numNodes = static_cast<int>(mesh->spritePositions.size());
	ISkinImportData *cskin = nullptr;

	if (numNodes > 1)
	{
		Modifier *cmod = (Modifier*)GetCOREInterface()->CreateInstance(OSM_CLASS_ID, SKIN_CLASSID);
		GetCOREInterface7()->AddModifier(*nde, *cmod);
		cskin = (ISkinImportData*)cmod->GetInterface(I_SKINIMPORTDATA);
	}

	INodeTab nodes;
	nodes.Resize(numNodes);

	int curBone = 0;

	for (auto &b : mesh->spritePositions)
	{
		Object *obj = static_cast<Object*>(CreateInstance(HELPER_CLASS_ID, Class_ID(DUMMY_CLASS_ID, 0)));
		INode *node = GetCOREInterface()->CreateObjectNode(obj);
		node->ShowBone(2);
		Matrix3 localCorMat = corMat;
		localCorMat.Scale({ IDC_EDIT_SCALE_value,IDC_EDIT_SCALE_value,IDC_EDIT_SCALE_value });
		localCorMat.SetTrans(localCorMat.PointTransform(reinterpret_cast<Point3&>(b)));
		node->SetNodeTM(0, localCorMat);
		node->SetWireColor(0x80ff);
		node->SetName(ToBoneName((TSTRING(_T("SpriteBone")) + ToTSTRING(curBone))));

		if (cskin)
		{
			cskin->AddBoneEx(node, 0);
			nodes.AppendNode(node);
		}
		else
		{
			node->AttachChild(nde);
			return;
		}

		curBone++;
	}

	const int numVerts = mesh->Header.vertexCount;
	nde->EvalWorldState(0);

	for (auto &d : mesh->streamAttributes)
	{
		switch (d.Header.usage)
		{
		case AmfUsage_BoneIndex:
		{
			for (int v = 0; v < numVerts; v++)
			{
				int temp;
				d.Evaluate(&d, v, &temp);

				Tab<INode*> cbn;
				Tab<float> cwt;
				cbn.SetCount(1);
				cwt.SetCount(1);
				cbn[0] = nodes[temp];
				cwt[0] = 1.0f;

				cskin->AddWeights(nde, v, cbn, cwt);
			}

			break;
		}
		}
	}
}

void LoadDeform(AmfMesh *mesh, INode *nde)
{
	AmfStreamAttribute *deform = nullptr,
		*points = nullptr;

	for (auto &d : mesh->streamAttributes)
	{
		switch (d.Header.usage)
		{
		case AmfUsage_DeformNormal_c:
			deform = &d;
			break;
		case AmfUsage_DeformPoints_c:
			points = &d;
			break;
		}
	}

	Modifier *cmod = (Modifier*)GetCOREInterface()->CreateInstance(OSM_CLASS_ID, MR3_CLASS_ID);
	GetCOREInterface7()->AddModifier(*nde, *cmod);

	MaxMorphModifier morpher = {};
	morpher.Init(cmod);
	
	const int numVerts = mesh->Header.vertexCount;

	{
		MaxMorphChannel chan = morpher.GetMorphChannel(0);
		chan.Reset(true, true, numVerts);
		chan.SetName(_T("Deform"));

		for (int v = 0; v < numVerts; v++)
		{
			Point3 temp;
			deform->Evaluate(deform, v, &temp);

			chan.SetMorphPointDelta(v, corMat.VectorTransform(temp) * 2.0f);
		}
	}

	if (points)
	{
		int curChan = 1;

		for (auto &r : mesh->boneIndexLookup)
			if (r > 0)
			{
				MaxMorphChannel chan = morpher.GetMorphChannel(curChan++);
				chan.Reset(true, true, numVerts);
				chan.SetName((TSTRING(_T("cp")) + ToTSTRING(r)).c_str());
			}

		for (int v = 0; v < numVerts; v++)
		{
			Point3 temp;
			deform->Evaluate(deform, v, &temp);

			Vector4 cpoints;
			points->Evaluate(points, v, &cpoints);

			Vector4 indicies = cpoints.Convert<float>() * 127.996f;
			Vector4 indiciesFloored(floorf(indicies.X), floorf(indicies.Y), floorf(indicies.Z), floorf(indicies.W));
			UIVector4 indiciesOut = indiciesFloored.Convert<uint>() + 128;

			Vector4 weights = indicies - indiciesFloored;

			for (int c = 0; c < 4; c++)
			{
				MaxMorphChannel chan = morpher.GetMorphChannel(indiciesOut[c] + 1);
				chan.SetMorphPointDelta(v, corMat.VectorTransform(temp) * 2.0f * weights[c]);
			}
		}
	}
}

void LoadSkin(AmfMesh *mesh, INode *nde)
{
	INodeTab bones;
	const int numVerts = mesh->Header.vertexCount;
	std::vector<AmfStreamAttribute*> weights;
	std::vector<AmfStreamAttribute*> bonesids;

	for (auto &d : mesh->streamAttributes)
	{
		switch (d.Header.usage)
		{
		case AmfUsage_BoneIndex:
			bonesids.push_back(&d);
			break;
		case AmfUsage_BoneWeight:
			weights.push_back(&d);
			break;
		}
	}

	if (!bonesids.size())
		return;

	Modifier *cmod = static_cast<Modifier*>(GetCOREInterface()->CreateInstance(OSM_CLASS_ID, SKIN_CLASSID));
	GetCOREInterface7()->AddModifier(*nde, *cmod);
	ISkinImportData *cskin = static_cast<ISkinImportData*>(cmod->GetInterface(I_SKINIMPORTDATA));

	for (auto &b : mesh->boneIndexLookup)
	{
		INode *cnde = iBoneScanner.LookupNode(b);
		bones.AppendNode(cnde);
		cskin->AddBoneEx(cnde, 0);
	}

	nde->EvalWorldState(0);	

	if (!weights.size())
	{
		AmfStreamAttribute &cbns = *bonesids[0];

		for (int v = 0; v < numVerts; v++)
		{
			uchar temp;
			cbns.Evaluate(&cbns, v, &temp);

			Tab<INode*> cbn;
			Tab<float> cwt;
			cbn.SetCount(1);
			cwt.SetCount(1);
			cbn[0] = bones[temp];
			cwt[0] = 1.0f;

			cskin->AddWeights(nde, v, cbn, cwt);
		}
	}
	else if (weights.size() == 1)
	{
		AmfStreamAttribute &cbns = *bonesids[0];
		AmfStreamAttribute &cwts = *weights[0];

		for (int v = 0; v < numVerts; v++)
		{
			UCVector4 bns;
			cbns.Evaluate(&cbns, v, &bns);

			Vector4 wts;
			cwts.Evaluate(&cwts, v, &wts);

			Tab<INode*> cbn;
			Tab<float> cwt;
			cbn.SetCount(4);
			cwt.SetCount(4);
			
			cbn[0] = bones[bns.X];
			cwt[0] = wts.X;
			cbn[1] = bones[bns.Y];
			cwt[1] = wts.Y;
			cbn[2] = bones[bns.Z];
			cwt[2] = wts.Z;
			cbn[3] = bones[bns.W];
			cwt[3] = wts.W;

			cskin->AddWeights(nde, v, cbn, cwt);
		}
	}
	else
	{
		for (int v = 0; v < numVerts; v++)
		{
			Tab<INode*> cbn;
			Tab<float> cwt;
			cbn.SetCount(8);
			cwt.SetCount(8);

			for (int d = 0; d < bonesids.size(); d++)
			{
				UCVector4 bns;
				bonesids[d]->Evaluate(bonesids[d], v, &bns);

				Vector4 wts;
				weights[d]->Evaluate(weights[d], v, &wts);

				for (int s = 0; s < 4; s++)
				{
					cbn[s + d * 4] = bones[bns[s]];
					cwt[s + d * 4] = wts[s];
				}
			}

			cskin->AddWeights(nde, v, cbn, cwt);
		}
	}

}

void ApexImp::ApplyDeform(AmfMesh *mesh, INodeSuffixer &nde)
{
	const int numNodes = static_cast<int>(mesh->spritePositions.size());

	if (numNodes)
	{
		LoadSpriteData(mesh, nde);

		if (numNodes > 1)
			nde.UseSkin();

		goto _ApplyDeformNameNode;
	}

	for (auto &d : mesh->streamAttributes)
		if (d.Header.usage == AmfUsage_DeformNormal_c)
		{
			LoadDeform(mesh, nde);
			nde.UseMorph();
			goto _ApplyDeformNameNode;
		}

	if (mesh->boneIndexLookup.size() > 1)
	{
		LoadSkin(mesh, nde);
		nde.UseSkin();
	}
	else if (mesh->boneIndexLookup.size() > 0)
	{
		INode *cnde = iBoneScanner.LookupNode(mesh->boneIndexLookup[0]);

		if (cnde)
			cnde->AttachChild(nde);
	}

_ApplyDeformNameNode:
	TSTRING ndeName = static_cast<TSTRING>(esString(mesh->subMeshes[0].meshName->string));

	if (flags[IDC_CH_DEBUGNAME_checked])
	{
		TSTRING className = static_cast<TSTRING>(esString(mesh->meshType->string));
		ndeName.append(nde.Generate(&className));
	}

	nde.node->SetName(ToBoneName(ndeName));
}

void DumpMaterialProps(AmfMaterial *mat)
{
	if (!mat->attributes)
		return;

	Reflector *refl = dynamic_cast<Reflector*>(mat->attributes);

	if (!refl)
		return;
	
	const int numReflValues = refl->GetNumReflectedValues();

	if (!numReflValues)
		return;

	printer << esString(mat->name->string) << " attributes: " >> 1;

	for (int t = 0; t < numReflValues; t++)
	{
		const Reflector::KVPair &pair = refl->GetReflectedPair(t);

		printer << '\t' << esString(pair.name) << " = " << esString(pair.value) >> 1;
	}
}

int ApexImp::LoadModel(IADF *adf)
{
	AmfMeshHeader *msh = adf->FindInstance<AmfMeshHeader>();
	AmfModel *mod = adf->FindInstance<AmfModel>();

	if (!msh || !mod)
		return FALSE;

	IColorVar *_test = IColorVar::Create();

	if (!_test)
	{
		printwarning("Xplorer not loaded, materials won't be created for Apex Tool import.")
	}

	ILayerManager* manager = GetCOREInterface13()->GetLayerManager();
	std::map<ApexHash, Mtl*> materials;

	if (_test)
		for (auto &m : mod->materials)
		{
			bool forced = m->materialType == AmfMaterial::MaterialType_PBR && flags[IDC_CH_FORCESTDMAT_checked];

			if (forced)
				m->materialType = AmfMaterial::MaterialType_Traditional;

			Mtl *cMat = CreateMaterial(m);

			if (flags[IDC_CH_ENABLEVIEWMAT_checked])
				GetCOREInterface()->ActivateTexture(cMat, cMat);

			materials[m->name->hash] = cMat;

			if (flags[IDC_CH_DUMPMATINFO_checked])
				DumpMaterialProps(m);

			if (forced)
				m->materialType = AmfMaterial::MaterialType_PBR;
		}

	for (auto &lod : msh->lodGroups)
	{
		MSTR layName = L"LOD";
		layName.append(ToTSTRING(lod.Header.index).c_str());

		ILayer *currLayer = manager->GetLayer(layName);

		if (!currLayer)
			currLayer = manager->CreateLayer(layName);

		int curMesh = 0;

		for (auto &m : lod.meshes)
		{
			INodeSuffixer nde = LoadMesh(&m);

			if (!nde.node)
			{
				TSTRING ndeName = static_cast<TSTRING>(esString(m.subMeshes[0].meshName->string));
				printerror("[Apex] Couldn't import model: ", << ndeName << " LOD: " << lod.Header.index);
				continue;
			}

			if (m.subMeshes.size() > 1)
			{
				MultiMtl *mtl = NewDefaultMultiMtl();
				mtl->SetNumSubMtls(static_cast<int>(m.subMeshes.size()));

				int curMat = 0;

				for (auto &s : m.subMeshes)
					if (materials.count(s.meshName->hash))
					{
						mtl->SetSubMtl(curMat, materials[s.meshName->hash]);
						curMat++;
					}

				nde.node->SetMtl(mtl);
			}
			else if (materials.count(m.subMeshes[0].meshName->hash))
				nde.node->SetMtl(materials[m.subMeshes[0].meshName->hash]);

			ApplyDeform(&m, nde);
			currLayer->AddToLayer(nde);
			curMesh++;
		}
	}

	return TRUE;
}

int ApexImp::LoadStuntArea(IADF *adf)
{
	ADFStuntAreas *are = adf->FindInstance<ADFStuntAreas>();

	if (!are || !are->numStuntAreas)
		return FALSE;

	for (int a = 0; a < are->numStuntAreas; a++)
	{
		StuntArea &area = are->stuntAreas[a];

		TriObject *obj = CreateNewTriObject();
		Mesh *msh = &obj->GetMesh();

		msh->setNumVerts(area.numDeformPoints);
		msh->setNumFaces(area.numFaces);

		Matrix3 localCorMat = corMat;
		localCorMat.Scale({ IDC_EDIT_SCALE_value,IDC_EDIT_SCALE_value,IDC_EDIT_SCALE_value });


		for (int v = 0; v < area.numDeformPoints; v++)
			msh->setVert(v, localCorMat.PointTransform(reinterpret_cast<Point3&>(area.vertices[v])));

		for (int f = 0; f < area.numFaces; f++)
		{
			Face &face = msh->faces[f];
			face.setEdgeVisFlags(1, 1, 1);
			face.v[0] = area.faces[f][0];
			face.v[1] = area.faces[f][1];
			face.v[2] = area.faces[f][2];
		}

		msh->InvalidateGeomCache();
		msh->InvalidateTopologyCache();

		INode *nde = GetCOREInterface()->CreateObjectNode(obj);
		TSTRING boneName = _T("ASA_");
		boneName += esString(area.name.str->string);
		nde->SetName(ToBoneName(boneName));

		boneName = esString(area.partName.str->string);

		INode *parent = iBoneScanner.LookupNode(boneName);
		parent->AttachChild(nde);

	}

	return TRUE;
}

int ApexImp::DoImport(const TCHAR* filename, ImpInterface* /*importerInt*/, Interface* /*ip*/, BOOL suppressPrompts)
{
	char *oldLocale = setlocale(LC_NUMERIC, NULL);
	setlocale(LC_NUMERIC, "en-US");

	if (!suppressPrompts)
		if (!SpawnDialog())
			return 1;

	if (flags[IDC_CH_CLEARLISTENER_checked])
		ExecuteMAXScriptScript(_T("ClearListener()"), TRUE);

	iBoneScanner.RescanBones();


	IADF *adf = IADF::Create(filename);

	if (!adf)
		return FALSE;

	if (!LoadStuntArea(adf))
		return LoadModel(adf);

	setlocale(LC_NUMERIC, oldLocale);

	delete adf;
	return TRUE;
}
	
