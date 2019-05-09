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

#include "../ApexMax.h"
#include <string>
#include "datas/esstring.h"
#include <stdmat.h>
#include "AmfModel.h"
#include <vector>
#include "datas/masterprinter.hpp"
#include "datas/fileinfo.hpp"
#include <map>
#include "IXTexmaps.h"
#include "MAXex/Maps.h"

#include "RBMClassesJC2.h"
#include "RBMClassesHU.h"
#include "RBMClassesJC3.h"
#include "RBNClasses.h"

#define ADFMATERIAL(classname) void classname##MaterialLoad(AdfProperties*, StdMat2* material, TexmapMapping& textures)
#define ADFMATERIAL_WPROPS(classname) void classname##MaterialLoad(AdfProperties* properties, StdMat2* material, TexmapMapping& textures)

typedef std::vector<BitmapTex*> TexmapMapping;

ADFMATERIAL(RBMCarPaintSimple)
{
	CompositeTex diffComposite;
	CompositeTex::Layer layer1 = diffComposite.GetLayer(0);
	layer1.Map(textures[0]);

	CompositeTex::Layer layer2 = diffComposite.AddLayer();
	layer2.BlendMode(CompositeTex::Multiply);

	IColorVar *var = IColorVar::Create();
	var->SetName(_T("Car Paint Color"));
	layer2.Map(var);

	IColorMask *mask = IColorMask::Create();

	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Red);
	layer2.Mask(mask);

	material->SetSubTexmap(ID_DI, diffComposite);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Green);
	material->SetSubTexmap(ID_SH, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(ID_SS, mask);

	material->SetSubTexmap(ID_BU, NormalBump(textures[1]));	
}

ADFMATERIAL(RBMVegetationFoliage)
{ 
	material->SetTwoSided(TRUE);
	material->LockAmbDiffTex(FALSE);

	if (textures[0])
	{
		textures[0]->SetAlphaSource(ALPHA_FILE);
		textures[0]->SetAlphaAsMono(TRUE);
	}

	material->SetSubTexmap(ID_DI, textures[0]);
	material->SetSubTexmap(ID_OP, textures[0]);
	material->SetSubTexmap(ID_AM, textures[2]);

	material->SetSubTexmap(ID_BU, NormalBump(textures[1]));
}

ADFMATERIAL(RBMFoliageBark)
{
	if (textures[2])
	{
		textures[2]->GetUVGen()->SetMapChannel(2);
		material->SetSubTexmap(ID_DI, Mix(textures[0], textures[2], VertexColor()));
	}
	else
		material->SetSubTexmap(ID_DI, RGBMultiply(textures[0], VertexColor()));

	Texmap *normal = nullptr;

	if (textures[3])
	{
		textures[3]->GetUVGen()->SetMapChannel(2);
		normal = Mix(textures[1], textures[3], VertexColor());
	}
	else
		normal = textures[1];

	material->SetSubTexmap(ID_BU, NormalBump(normal));
}

ADFMATERIAL(RBMBillboardFoliage)
{
	if (textures[0])
	{
		textures[0]->SetAlphaSource(ALPHA_FILE);
		textures[0]->SetAlphaAsMono(TRUE);
	}

	material->SetSubTexmap(ID_DI, textures[0]);
	material->SetSubTexmap(ID_OP, textures[0]);
}

ADFMATERIAL(RBMHalo)
{
	if (textures[0])
	{
		textures[0]->SetAlphaSource(ALPHA_FILE);
		textures[0]->SetAlphaAsMono(TRUE);
	}

	material->SetSubTexmap(ID_DI, RGBMultiply(textures[0], VertexColor()));
	material->SetSubTexmap(ID_OP, textures[0]);
	material->SetSelfIllum(1.0f, 0);
}

ADFMATERIAL(RBMLambert)
{
	CompositeTex diffComposite;
	CompositeTex::Layer layer1 = diffComposite.GetLayer(0);
	layer1.Map(textures[0]);

	if (textures[2])
		textures[2]->GetUVGen()->SetMapChannel(2);

	CompositeTex::Layer layer2 = diffComposite.AddLayer();
	layer2.BlendMode(CompositeTex::Multiply);
	layer2.Map(textures[2]);

	CompositeTex::Layer layer3 = diffComposite.AddLayer();
	layer3.BlendMode(CompositeTex::Multiply);
	layer3.Map(VertexColor());

	material->SetSubTexmap(ID_DI, diffComposite);

	material->SetSubTexmap(ID_BU, NormalBump(textures[1]));
}

ADFMATERIAL(RBMFacade)
{
	CompositeTex diffComposite;
	CompositeTex::Layer layer1 = diffComposite.GetLayer(0);
	
	if (textures[3])
		textures[3]->GetUVGen()->SetMapChannel(2);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[0]);
	mask->SetName(_T("Select Channel"));
	
	layer1.Map(mask);

	CompositeTex::Layer layer2 = diffComposite.AddLayer();
	layer2.Map(textures[3]);
	layer2.BlendMode(CompositeTex::Multiply);

	CompositeTex::Layer layer3 = diffComposite.AddLayer();
	layer3.Map(VertexColor());
	layer3.BlendMode(CompositeTex::Multiply);

	material->SetSubTexmap(ID_DI, diffComposite);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(ID_SS, mask);

	material->SetSubTexmap(ID_BU, NormalBump(textures[1]));
}

ADFMATERIAL_WPROPS(RBMGeneral)
{
	RBMFacadeMaterialLoad(properties, material, textures);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Green);
	material->SetSubTexmap(ID_SH, mask);
}

ADFMATERIAL(RBMWindow)
{
	material->SetTwoSided(TRUE);

	if (textures[0])
	{
		textures[0]->SetAlphaSource(ALPHA_FILE);
		textures[0]->SetAlphaAsMono(TRUE);
	}

	material->SetSubTexmap(ID_DI, RGBMultiply(textures[0], VertexColor()));
	material->SetSubTexmap(ID_OP, textures[0]);

	material->SetSubTexmap(ID_BU, NormalBump(textures[1]));

	material->SetSubTexmap(ID_SS, textures[2]);
}

ADFMATERIAL(RBMSkinnedGeneral)
{
	material->SetSubTexmap(ID_DI, textures[0]);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Green);

	material->SetSubTexmap(ID_SH, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(ID_SS, mask);

	material->SetSubTexmap(ID_BU, NormalBump(textures[1]));
}

ADFMATERIAL_WPROPS(RBMMerged)
{
	RBMSkinnedGeneralMaterialLoad(properties, material, textures);
}

ADFMATERIAL(RBMCarPaint)
{
	CompositeTex diffComposite;
	CompositeTex::Layer layer1 = diffComposite.GetLayer(0);

	CompositeTex::Layer layer2 = diffComposite.AddLayer();
	layer2.BlendMode(CompositeTex::Multiply);

	IColorVar *var = IColorVar::Create();
	var->SetName(_T("Deform Value"));
	
	Texmap *mixer = textures[0];

	if (textures[3])
		mixer = Mix(textures[0], textures[3], var);

	IColorVar *carColor = IColorVar::Create();
	carColor->SetName(_T("Car Paint Color"));

	layer2.Map(carColor);
	layer1.Map(mixer);

	material->SetSubTexmap(ID_DI, diffComposite);

	mixer = textures[2];

	if (textures[5])
		mixer = Mix(textures[2], textures[5], var); 

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, mixer);
	mask->SetDecomposeType(Decompose_Red);
	layer2.Mask(mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, mixer);
	mask->SetDecomposeType(Decompose_Green);
	material->SetSubTexmap(ID_SH, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, mixer);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(ID_SS, mask);

	mixer = textures[1];

	if (textures[4])
		mixer = Mix(textures[1], textures[4], var);

	material->SetSubTexmap(ID_BU, NormalBump(mixer));
}

ADFMATERIAL_WPROPS(RBMDeformWindow)
{
	RBMCarPaintMaterialLoad(properties, material, textures);
}

ADFMATERIAL_WPROPS(RBMGeneral0)
{
	RBMGeneralMaterialLoad(properties, material, textures);
}

ADFMATERIAL_WPROPS(RBMFacade0)
{
	RBMFacadeMaterialLoad(properties, material, textures);
}

ADFMATERIAL(RBMUIOverlay)
{
	if (textures[0])
	{
		textures[0]->SetAlphaSource(ALPHA_FILE);
		textures[0]->SetAlphaAsMono(TRUE);
	}

	material->SetSubTexmap(ID_DI, textures[0]);
	material->SetSubTexmap(ID_OP, textures[0]);
}

ADFMATERIAL_WPROPS(RBMScope)
{
	RBMUIOverlayMaterialLoad(properties, material, textures);
}

ADFMATERIAL_WPROPS(RBMSkinnedGeneral0)
{
	RBMSkinnedGeneralMaterialLoad(properties, material, textures);
}

ADFMATERIAL(RBMSkinnedGeneralDecal)
{
	Texmap *diff = textures[0];

	if (textures[3])
	{
		textures[3]->GetUVGen()->SetMapChannel(2);
		diff = RGBMultiply(textures[0], textures[3]);
	}

	material->SetSubTexmap(ID_DI, diff);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Green);

	material->SetSubTexmap(ID_SH, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(ID_SS, mask);

	material->SetSubTexmap(ID_BU, NormalBump(textures[1]));
}

ADFMATERIAL(RBMVegetationFoliage3)
{
	int baseColorMap = ID_DI;
	int opacityMap = ID_OP;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;
	int ambientMap = ID_AM;

	if (textures[0])
	{
		textures[0]->SetAlphaSource(ALPHA_FILE);
		textures[0]->SetAlphaAsMono(TRUE);
	}

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		opacityMap = PhysicalMaterial::CutoutMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
		ambientMap = PhysicalMaterial::TransparencyMap;
	}

	material->SetSubTexmap(ambientMap, textures[2]);

	material->SetSubTexmap(baseColorMap, textures[0]);
	material->SetSubTexmap(opacityMap, textures[0]);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[3]);
	mask->SetDecomposeType(Decompose_Red);

	material->SetSubTexmap(metallicMap, mask);
	
	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[3]);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(roughnesMap, mask);

	material->SetSubTexmap(bumpMap, NormalBump(textures[1]));
}

ADFMATERIAL(RBMFoliageBark2)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;

	if (textures[3])
		textures[3]->GetUVGen()->SetMapChannel(2);

	if (textures[4])
		textures[4]->GetUVGen()->SetMapChannel(2);

	if (textures[0])
	{
		textures[0]->SetAlphaSource(ALPHA_FILE);
		textures[0]->SetAlphaAsMono(TRUE);
	}

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
	}

	Texmap *hmap = RGBMultiply(textures[3], textures[4]);

	material->SetSubTexmap(baseColorMap, Mix(textures[0], textures[5], hmap));

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, Mix(textures[2], textures[6], hmap));
	mask->SetDecomposeType(Decompose_Red);

	material->SetSubTexmap(metallicMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, Mix(textures[2], textures[7], hmap));
	mask->SetDecomposeType(Decompose_Green);
	material->SetSubTexmap(roughnesMap, mask);

	material->SetSubTexmap(bumpMap, NormalBump(Mix(textures[1], textures[8], hmap)));
}

ADFMATERIAL(RBMGeneralSimple)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;
	int ambientMap = ID_AM;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
		ambientMap = PhysicalMaterial::TransparencyMap;
	}

	if (textures[3])
		textures[3]->GetUVGen()->SetMapChannel(2);

	material->SetSubTexmap(baseColorMap, RGBMultiply(textures[0], VertexColor()));

	material->SetSubTexmap(ambientMap, textures[3]);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Red);

	material->SetSubTexmap(metallicMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Green);
	material->SetSubTexmap(roughnesMap, mask);

	material->SetSubTexmap(bumpMap, NormalBump(textures[1]));
}

ADFMATERIAL(RBMBavariumShiled)
{
	int opacityMap = ID_OP;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		opacityMap = PhysicalMaterial::CutoutMap;
	}

	if (textures[0])
	{
		textures[0]->SetAlphaSource(ALPHA_FILE);
		textures[0]->SetAlphaAsMono(TRUE);
	}

	material->SetSubTexmap(opacityMap, textures[0]);

	material->SetSelfIllum(1.0f, 0);
}

ADFMATERIAL(RBMWindow1)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int opacityMap = ID_OP;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		opacityMap = PhysicalMaterial::CutoutMap;
	}
	else
		material->SetTwoSided(TRUE);

	if (textures[0])
	{
		textures[0]->SetAlphaSource(ALPHA_FILE);
		textures[0]->SetAlphaAsMono(TRUE);
	}

	material->SetSubTexmap(baseColorMap, RGBMultiply(textures[0], VertexColor()));
	material->SetSubTexmap(opacityMap, textures[0]);
	material->SetSubTexmap(bumpMap, NormalBump(textures[1]));
	material->SetSubTexmap(roughnesMap, textures[2]);
}

ADFMATERIAL(RBMLayered)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
	}

	Texmap *diff = textures[0];
	Texmap *bump = textures[1];
	Texmap *rough = textures[2];
	Texmap *metal = textures[6];


	if (textures[8])
	{
		textures[8]->GetUVGen()->SetMapChannel(2);
		
		IColorMask *hmap = IColorMask::Create();
		hmap->SetSubTexmap(0, textures[8]);
		hmap->SetDecomposeType(Decompose_Red);

		diff = Mix(textures[0], textures[3], hmap);
		rough = Mix(textures[2], textures[5], hmap);
		bump = Mix(textures[1], textures[4], hmap);

		if (metal)
			metal = Mix(textures[6], textures[7], hmap);

	}
	else
	{
		textures[3] = nullptr;
		textures[4] = nullptr;
		textures[5] = nullptr;
		textures[7] = nullptr;
	}

	material->SetSubTexmap(baseColorMap, RGBMultiply(diff, VertexColor()));

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, rough);
	mask->SetDecomposeType(Decompose_Green);
	material->SetSubTexmap(roughnesMap, mask);
	
	if (metal)
	{
		mask = IColorMask::Create();
		mask->SetSubTexmap(0, metal);
		mask->SetDecomposeType(Decompose_Red);
		
		material->SetSubTexmap(metallicMap, mask);
	}

	material->SetSubTexmap(bumpMap, NormalBump(bump));
}

ADFMATERIAL(RBMLandmark)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
	}

	material->SetSubTexmap(baseColorMap, RGBMultiply(textures[0], VertexColor()));

	material->SetSubTexmap(roughnesMap, textures[2]);

	material->SetSubTexmap(bumpMap, NormalBump(textures[1]));
}

ADFMATERIAL_WPROPS(RBMGeneralMK3)
{
	RBMGeneralMK3 *props = static_cast<RBMGeneralMK3*>(properties);
	bool usedecals = (props->flags & 0x200) != 0;

	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
	}

	Texmap *diff = textures[0];
	Texmap *rough = textures[1];
	Texmap *metal = textures[2];
	Texmap *bump = textures[3];

	if (textures[5])
		textures[5]->GetUVGen()->SetMapChannel(2);

	if (textures[11])
	{
		textures[11]->GetUVGen()->SetMapChannel(2);
		textures[11]->SetAlphaSource(ALPHA_FILE);
		textures[11]->SetAlphaAsMono(TRUE);
	}

	if (textures[12])
		textures[12]->GetUVGen()->SetMapChannel(2);

	if (textures[13])
		textures[13]->GetUVGen()->SetMapChannel(2);

	if (textures[14])
		textures[14]->GetUVGen()->SetMapChannel(2);

	if (textures[6])
	{
		textures[6]->GetUVGen()->SetMapChannel(2);

		Texmap *map6 = textures[6];
		if (map6)
			map6 = RGBMultiply(textures[5], textures[6]);
		else
			map6 = textures[5];

		IColorMask *mask = IColorMask::Create();
		mask->SetSubTexmap(0, map6);
		mask->SetName(_T("Select Channel"));

		diff = Mix(textures[0], textures[7], mask);
		rough = Mix(textures[1], textures[8], mask);
		metal = Mix(textures[2], textures[9], mask);
		bump = Mix(textures[3], textures[10], mask);
	}

	if (usedecals)
	{
		IColorMask *mask = IColorMask::Create();
		mask->SetSubTexmap(0, textures[11]);
		mask->SetName(_T("Decal mask"));
		mask->SetDecomposeType(Decompose_Alpha);

		diff = Mix(diff, textures[11], mask);
		rough = Mix(rough, textures[12], mask);
		metal = Mix(metal, textures[13], mask);
		bump = Mix(bump, textures[14], mask);
	}

	material->SetSubTexmap(baseColorMap, RGBMultiply(diff, VertexColor()));

	material->SetSubTexmap(roughnesMap, rough);
	material->SetSubTexmap(metallicMap, metal);

	material->SetSubTexmap(bumpMap, NormalBump(bump));
}

ADFMATERIAL(RBMGeneral6)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int ambientMap = ID_AM;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		ambientMap = PhysicalMaterial::TransparencyMap;
	}

	if (textures[3])
		textures[3]->GetUVGen()->SetMapChannel(2);

	material->SetSubTexmap(baseColorMap, RGBMultiply(textures[0], VertexColor()));

	material->SetSubTexmap(ambientMap, textures[3]);

	material->SetSubTexmap(roughnesMap, textures[2]);

	material->SetSubTexmap(bumpMap, NormalBump(textures[1]));
}

ADFMATERIAL(RBMCarLight)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metalnessMap = ID_SH;
	int emisiveMap = ID_SI;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metalnessMap = PhysicalMaterial::MetalnessMap;
		emisiveMap = PhysicalMaterial::EmissionMap;
	}

	if (textures[3])
		textures[3]->GetUVGen()->SetMapChannel(2);

	if (textures[4])
		textures[4]->GetUVGen()->SetMapChannel(2);

	if (textures[5])
		textures[5]->GetUVGen()->SetMapChannel(2);

	material->SetSubTexmap(baseColorMap, RGBMultiply(textures[0], textures[3]));
	material->SetSubTexmap(bumpMap, NormalBump(RGBMultiply(textures[1], textures[4])));

	IColorVar *var1 = IColorVar::Create();
	var1->SetName(_T("Emissive Color1"));

	IColorVar *var2 = IColorVar::Create();
	var2->SetName(_T("Emissive Color2"));

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[5]);
	mask->SetDecomposeType(Decompose_Red);

	Texmap *emis = Mix(var1, var2, mask);

	var1 = IColorVar::Create();
	var1->SetName(_T("Emissive Color3"));

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[5]);
	mask->SetDecomposeType(Decompose_Green);

	emis = Mix(emis, var1, mask);

	var1 = IColorVar::Create();
	var1->SetName(_T("Emissive Color4"));

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[5]);
	mask->SetDecomposeType(Decompose_Blue);

	emis = Mix(emis, var1, mask);

	material->SetSubTexmap(emisiveMap, emis);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Red);

	material->SetSubTexmap(roughnesMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Green);

	material->SetSubTexmap(metalnessMap, mask);
}

ADFMATERIAL(RBMCarPaint14)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metalnessMap = ID_SH;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metalnessMap = PhysicalMaterial::MetalnessMap;
	}

	if (textures[7])
		textures[7]->GetUVGen()->SetMapChannel(2);

	if (textures[8])
		textures[8]->GetUVGen()->SetMapChannel(2);

	if (textures[9])
		textures[9]->GetUVGen()->SetMapChannel(2);

	if (textures[11])
		textures[11]->GetUVGen()->SetMapChannel(3);

	CompositeTex diffComposite;
	diffComposite.Resize(7);

	CompositeTex::Layer layer = diffComposite.GetLayer(0);
	layer.BlendMode(CompositeTex::Normal);
	layer.Map(textures[10]);

	layer = diffComposite.GetLayer(1);
	layer.BlendMode(CompositeTex::Normal);
	layer.Map(textures[11]);

	layer = diffComposite.GetLayer(2);
	layer.BlendMode(CompositeTex::Normal);
	layer.Map(textures[5]);

	layer = diffComposite.GetLayer(3);
	layer.BlendMode(CompositeTex::Average);
	layer.Map(textures[6]);
	layer.Mask(textures[3]);

	layer = diffComposite.GetLayer(4);
	layer.BlendMode(CompositeTex::Average);
	layer.Map(textures[7]);

	layer = diffComposite.GetLayer(5);
	layer.BlendMode(CompositeTex::Average);
	IColorVar *var = IColorVar::Create();
	var->SetName(_T("Paint Color"));
	layer.Map(var);

	layer = diffComposite.GetLayer(6);
	layer.BlendMode(CompositeTex::Add);
	layer.Map(textures[0]);

	material->SetSubTexmap(baseColorMap, diffComposite);

	CompositeTex bumpComposite;
	bumpComposite.Resize(3);

	layer = bumpComposite.GetLayer(0);
	layer.BlendMode(CompositeTex::Normal);
	layer.Map(textures[4]);

	layer = bumpComposite.GetLayer(1);
	layer.BlendMode(CompositeTex::Average);
	layer.Map(textures[8]);

	layer = bumpComposite.GetLayer(2);
	layer.BlendMode(CompositeTex::Average);
	layer.Map(textures[1]);
	
	material->SetSubTexmap(bumpMap, NormalBump(bumpComposite));

	CompositeTex propComposite;
	layer = propComposite.GetLayer(0);
	layer.BlendMode(CompositeTex::Normal);
	layer.Map(textures[9]);

	layer = propComposite.AddLayer();
	layer.BlendMode(CompositeTex::Average);
	layer.Map(textures[2]);


	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, propComposite);
	mask->SetDecomposeType(Decompose_Red);

	material->SetSubTexmap(roughnesMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, propComposite);
	mask->SetDecomposeType(Decompose_Green);

	material->SetSubTexmap(metalnessMap, mask);
}

ADFMATERIAL(RBMGeneral3)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metalnessMap = ID_SH;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metalnessMap = PhysicalMaterial::MetalnessMap;
	}

	if (textures[3])
	{
		textures[3]->GetUVGen()->SetMapChannel(2);
		textures[3]->SetAlphaSource(ALPHA_FILE);
		textures[3]->SetAlphaAsMono(TRUE);
	}

	if (textures[4])
		textures[4]->GetUVGen()->SetMapChannel(2);

	if (textures[5])
		textures[5]->GetUVGen()->SetMapChannel(2);

	Texmap *diff = textures[0];
	Texmap *prop = textures[2];
	Texmap *bump = textures[1];

	if (textures[3])
	{
		IColorMask *mask = IColorMask::Create();
		mask->SetSubTexmap(0, textures[3]);
		mask->SetName(_T("Decal mask"));
		mask->SetDecomposeType(Decompose_Alpha);

		diff = Mix(diff, textures[3], mask);
		prop = Mix(prop, textures[5], mask);
		bump = Mix(bump, textures[4], mask);
	}

	material->SetSubTexmap(baseColorMap, diff);

	IColorMask *propMask = IColorMask::Create();
	propMask->SetSubTexmap(0, prop);
	propMask->SetDecomposeType(Decompose_Red);

	material->SetSubTexmap(roughnesMap, propMask);

	propMask = IColorMask::Create();
	propMask->SetSubTexmap(0, prop);
	propMask->SetDecomposeType(Decompose_Green);

	material->SetSubTexmap(metalnessMap, propMask);

	material->SetSubTexmap(bumpMap, NormalBump(bump));
}

ADFMATERIAL(RBMCharacter9)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metalnessMap = ID_SH;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metalnessMap = PhysicalMaterial::MetalnessMap;
	}

	CompositeTex diffComposite;
	diffComposite.Resize(6);

	CompositeTex::Layer layer = diffComposite.GetLayer(0);
	layer.BlendMode(CompositeTex::Normal);
	layer.Map(textures[7]);

	layer = diffComposite.GetLayer(1);
	layer.BlendMode(CompositeTex::Normal);
	layer.Map(textures[8]);

	layer = diffComposite.GetLayer(2);
	layer.BlendMode(CompositeTex::Normal);
	layer.Map(textures[9]);
	
	layer = diffComposite.GetLayer(3);
	layer.BlendMode(CompositeTex::Average);
	IColorVar *var = IColorVar::Create();
	var->SetName(_T("Blood Blend"));
	layer.Mask(var);
	layer.Map(textures[6]);

	layer = diffComposite.GetLayer(4);
	layer.BlendMode(CompositeTex::Average);
	layer.Map(textures[3]);	

	layer = diffComposite.GetLayer(5);
	layer.BlendMode(CompositeTex::Add);
	layer.Map(textures[0]);

	material->SetSubTexmap(baseColorMap, diffComposite);

	material->SetSubTexmap(bumpMap, NormalBump(RGBMultiply(textures[1], textures[4])));

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Green);

	Texmap *metal = mask;

	if (textures[5])
		metal = RGBMultiply(mask, textures[5]);

	material->SetSubTexmap(metalnessMap, metal);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Red);

	material->SetSubTexmap(roughnesMap, mask);
}

ADFMATERIAL(RBMCharacter6)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metalnessMap = ID_SH;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metalnessMap = PhysicalMaterial::MetalnessMap;
	}

	CompositeTex diffComposite;
	diffComposite.Resize(6);

	CompositeTex::Layer layer = diffComposite.GetLayer(0);
	layer.BlendMode(CompositeTex::Normal);
	layer.Map(textures[7]);

	layer = diffComposite.GetLayer(1);
	layer.BlendMode(CompositeTex::Normal);
	layer.Map(textures.size() > 8 ? textures[8] : nullptr);

	layer = diffComposite.GetLayer(2);
	layer.BlendMode(CompositeTex::Average);
	IColorVar *var = IColorVar::Create();
	var->SetName(_T("Blood Blend"));
	layer.Mask(var);
	layer.Map(textures[6]);

	layer = diffComposite.GetLayer(3);
	layer.BlendMode(CompositeTex::Average);
	layer.Map(textures[3]);

	layer = diffComposite.GetLayer(4);
	layer.BlendMode(CompositeTex::Add);
	layer.Map(textures[0]);

	material->SetSubTexmap(baseColorMap, diffComposite);

	material->SetSubTexmap(bumpMap, NormalBump(RGBMultiply(textures[1], textures[4])));

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Green);

	Texmap *metal = mask;

	if (textures[5])
		metal = RGBMultiply(mask, textures[5]);

	material->SetSubTexmap(metalnessMap, metal);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Red);

	material->SetSubTexmap(roughnesMap, mask);
}

ADFMATERIAL(RBMRoad)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
	}

	material->SetSubTexmap(baseColorMap, RGBMultiply(textures[6], RGBMultiply(textures[4],Mix(textures[0],textures[2],VertexColor()))));
	Texmap *bump = RGBMultiply(RGBMultiply(textures[7], RGBMultiply(textures[5], Mix(textures[1], textures[3], VertexColor()))), textures[8]);
	material->SetSubTexmap(bumpMap, NormalBump(bump));

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, bump);
	mask->SetDecomposeType(Decompose_Blue);

	material->SetSubTexmap(roughnesMap, mask);
}

ADFMATERIAL(RBMGeneralSimple3)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
	}

	material->SetSubTexmap(baseColorMap, textures[0]);
	material->SetSubTexmap(bumpMap, NormalBump(textures[1]));

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Blue);

	material->SetSubTexmap(roughnesMap, mask);
}

ADFMATERIAL(RBNGeneral)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;
	int ambientMap = ID_AM;
	int emisiveMap = ID_SI;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
		ambientMap = PhysicalMaterial::TransparencyMap;
		emisiveMap = PhysicalMaterial::EmissionMap;

		PhysicalMaterial(material).InvertRoughness(1);
	}

	Texmap *diff = textures[0];
	Texmap *bump = textures[1];

	if (textures[4] || textures[6])
	{
		CompositeTex comp;
		CompositeTex::Layer layer1 = comp.GetLayer(0);
		layer1.Map(diff);

		if (textures[4])
		{
			layer1 = comp.AddLayer();
			layer1.Map(textures[4]);
			layer1.BlendMode(CompositeTex::Average);
		}

		if (textures[6])
		{
			textures[6]->GetUVGen()->SetMapChannel(2);
			layer1 = comp.AddLayer();
			layer1.Map(textures[6]);
			layer1.BlendMode(CompositeTex::Average);
		}

		diff = comp;
	}

	if (textures[5] || textures[7])
	{
		CompositeTex comp;
		CompositeTex::Layer layer1 = comp.GetLayer(0);
		layer1.Map(bump);

		if (textures[5])
		{
			layer1 = comp.AddLayer();
			layer1.Map(textures[5]);
			layer1.BlendMode(CompositeTex::Average);
		}

		if (textures[7])
		{
			textures[7]->GetUVGen()->SetMapChannel(2);
			layer1 = comp.AddLayer();
			layer1.Map(textures[7]);
			layer1.BlendMode(CompositeTex::Average);
		}

		bump = comp;
	}

	material->SetSubTexmap(baseColorMap, RGBMultiply(diff, VertexColor()));

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Red);
	material->SetSubTexmap(ambientMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Green);
	material->SetSubTexmap(roughnesMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(metallicMap, mask);

	material->SetSubTexmap(bumpMap, NormalBump(bump));

	material->SetSubTexmap(emisiveMap, textures[3]);

}

ADFMATERIAL(RBNCarPaint)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;
	int ambientMap = ID_AM;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
		ambientMap = PhysicalMaterial::TransparencyMap;

		PhysicalMaterial(material).InvertRoughness(1);
	}

	Texmap *diff = textures[0];
	Texmap *bump = textures[1];

	if (textures[3])
	{
		textures[3]->GetUVGen()->SetMapChannel(2);
		diff = RGBMultiply(diff, textures[3]);
	}

	if (textures[4])
	{
		textures[4]->GetUVGen()->SetMapChannel(2);
		bump = RGBMultiply(bump, textures[4]);
	}

	material->SetSubTexmap(baseColorMap, diff);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Red);
	material->SetSubTexmap(ambientMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Green);
	material->SetSubTexmap(roughnesMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(metallicMap, mask);

	material->SetSubTexmap(bumpMap, NormalBump(bump));
}

ADFMATERIAL(RBNCharacter)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;
	int ambientMap = ID_AM;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
		ambientMap = PhysicalMaterial::TransparencyMap;

		PhysicalMaterial(material).InvertRoughness(1);
	}

	Texmap *diff = textures[0];
	Texmap *bump = textures[1];

	if (textures[3] || textures[4])
	{
		CompositeTex comp;
		CompositeTex::Layer layer1 = comp.GetLayer(0);
		layer1.Map(diff);

		if (textures[3])
		{
			layer1 = comp.AddLayer();
			layer1.Map(textures[3]);
			layer1.BlendMode(CompositeTex::Average);

			IColorVar *var = IColorVar::Create();
			var->SetName(_T("Blood Blend"));

			layer1.Mask(var);
		}

		if (textures[4])
		{
			layer1 = comp.AddLayer();
			layer1.Map(textures[4]);
			layer1.Mask(textures[7]);
			layer1.BlendMode(CompositeTex::Average);
		}

		diff = comp;
	}

	if (textures[5] || textures[6])
	{
		CompositeTex comp;
		CompositeTex::Layer layer1 = comp.GetLayer(0);
		layer1.Map(bump);

		if (textures[5])
		{
			layer1 = comp.AddLayer();
			layer1.Map(textures[5]);
			layer1.BlendMode(CompositeTex::Average);
		}

		if (textures[6])
		{
			textures[6]->GetUVGen()->SetMapChannel(2);
			layer1 = comp.AddLayer();

			Mix mx;
			mx.Map1(textures[6]);
			mx.Map2Active(0);
			mx.Color2(Color(0.5f, 0.5f, 1.0f));

			IColorVar *var = IColorVar::Create();
			var->SetName(_T("Wrinkle Blend"));

			mx.Mask(var);

			layer1.Map(mx);
			layer1.BlendMode(CompositeTex::Average);
		}

		bump = comp;
	}

	material->SetSubTexmap(baseColorMap, diff);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Red);
	material->SetSubTexmap(ambientMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Green);
	material->SetSubTexmap(roughnesMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(metallicMap, mask);

	material->SetSubTexmap(bumpMap, NormalBump(bump));
}

ADFMATERIAL(RBNWindow)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;
	int ambientMap = ID_AM;
	int opacityMap = ID_OP;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
		ambientMap = PhysicalMaterial::TransparencyMap;
		opacityMap = PhysicalMaterial::CutoutMap;

		PhysicalMaterial(material).InvertRoughness(1);
	}

	if (textures[0])
	{
		textures[0]->SetAlphaSource(ALPHA_FILE);
		textures[0]->SetAlphaAsMono(TRUE);
	}

	Texmap *diff = textures[0];
	Texmap *bump = textures[1];

	if (textures[3])
	{
		textures[3]->SetAlphaSource(ALPHA_FILE);
		textures[3]->SetAlphaAsMono(TRUE);
		diff = RGBMultiply(diff, textures[3]);
	}

	material->SetSubTexmap(baseColorMap, diff);
	material->SetSubTexmap(opacityMap, diff);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Red);
	material->SetSubTexmap(ambientMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Green);
	material->SetSubTexmap(roughnesMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(metallicMap, mask);

	material->SetSubTexmap(bumpMap, NormalBump(bump));
}

ADFMATERIAL(RBNXXXX)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;
	int ambientMap = ID_AM;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
		ambientMap = PhysicalMaterial::TransparencyMap;

		PhysicalMaterial(material).InvertRoughness(1);
	}

	if (textures[3])
	{
		textures[3]->SetAlphaSource(ALPHA_FILE);
		textures[3]->SetAlphaAsMono(TRUE);
	}

	IColorMask *hmap = IColorMask::Create();
	hmap->SetSubTexmap(0, textures[3]);
	hmap->SetDecomposeType(Decompose_Alpha);

	Texmap *diff = Mix(textures[0], textures[3], hmap);
	Texmap *rough = Mix(textures[2], textures[5], hmap);
	Texmap *bump = Mix(textures[1], textures[4], hmap);


	material->SetSubTexmap(baseColorMap, diff);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, rough);
	mask->SetDecomposeType(Decompose_Red);
	material->SetSubTexmap(ambientMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, rough);
	mask->SetDecomposeType(Decompose_Green);
	material->SetSubTexmap(roughnesMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, rough);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(metallicMap, mask);

	material->SetSubTexmap(bumpMap, NormalBump(bump));
}

ADFMATERIAL(LandmarkConstants)
{
	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
	}

	material->SetSubTexmap(baseColorMap, textures[0]);
	material->SetSubTexmap(bumpMap, NormalBump(textures[1]));
}

ADFMATERIAL_WPROPS(EmissiveUIConstants)
{
	EmissiveUIConstants *props = static_cast<EmissiveUIConstants*>(properties->GetProperties());

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		PhysicalMaterial mat(material);
		mat.Emission(1.0f);
		mat.EmissionColor(reinterpret_cast<Color&>(props->primaryColor));
		mat.BaseColor(mat.EmissionColor());
		mat.EmissionLuminance(100.0f);
	}
	else
	{
		material->SetSelfIllum(1.0f, 0);
		material->SetSelfIllumColor(reinterpret_cast<Color&>(props->primaryColor), 0);
	}
}

ADFMATERIAL_WPROPS(HologramConstants)
{
	HologramConstants *props = static_cast<HologramConstants*>(properties->GetProperties());
	int bumpMap = ID_BU;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		PhysicalMaterial mat(material);
		mat.Emission(props->emissiveIntensity);
		mat.EmissionColor(reinterpret_cast<Color&>(props->emissiveColor));
		mat.BaseColor(mat.EmissionColor());
		mat.EmissionLuminance(100.0f);
		bumpMap = PhysicalMaterial::BumpMap;
	}
	else
	{
		material->SetSelfIllum(props->emissiveIntensity, 0);
		material->SetSelfIllumColor(reinterpret_cast<Color&>(props->emissiveColor), 0);
	}

	material->SetSubTexmap(bumpMap, NormalBump(textures[0]));
}

ADFMATERIAL(FoliageConstants)
{
	int baseColorMap = ID_DI;
	int opacityMap = ID_OP;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;

	if (textures[0])
	{
		textures[0]->SetAlphaSource(ALPHA_FILE);
		textures[0]->SetAlphaAsMono(TRUE);
	}

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		opacityMap = PhysicalMaterial::CutoutMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
	}

	material->SetSubTexmap(baseColorMap, textures[0]);
	material->SetSubTexmap(opacityMap, textures[0]);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Red);

	material->SetSubTexmap(metallicMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(roughnesMap, mask);

	material->SetSubTexmap(bumpMap, NormalBump(textures[1]));
}

ADFMATERIAL_WPROPS(BarkConstants)
{
	BarkConstants *props = static_cast<BarkConstants*>(properties->GetProperties());

	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;
	int opacityMap = ID_OP;

	if (textures[3])
	{
		StdUVGen *uvGen = textures[3]->GetUVGen();

		if (props->flags[BarkConstantsFlags::detailNormalUseUV2])
			uvGen->SetMapChannel(2);

		uvGen->SetUScl(props->detailNormalTileU, 0);
		uvGen->SetVScl(props->detailNormalTileV, 0);
	}
	
	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
		opacityMap = PhysicalMaterial::CutoutMap;
		PhysicalMaterial(material).BumpMapAmmount(props->normalStrength);
	}
	else
		material->SetTexmapAmt(bumpMap, props->normalStrength, 0);

	if (textures[0] && props->isGrass)
	{
		textures[0]->SetAlphaSource(ALPHA_FILE);
		textures[0]->SetAlphaAsMono(TRUE);
		material->SetSubTexmap(opacityMap, textures[0]);
	}

	material->SetSubTexmap(baseColorMap, textures[0]);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Red);

	material->SetSubTexmap(metallicMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Green);
	material->SetSubTexmap(roughnesMap, mask);

	material->SetSubTexmap(bumpMap, NormalBump(RGBMultiply(textures[1], textures[3])));
}

ADFMATERIAL(EyeGlossConstants)
{
	int baseColorMap = ID_DI;
	int reflMap = ID_RL;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		reflMap = PhysicalMaterial::ReflectMap;
	}

	material->SetSubTexmap(baseColorMap, textures[0]);
	material->SetSubTexmap(reflMap, textures[1]);
}

ADFMATERIAL_WPROPS(HairConstants)
{
	HairConstants *props = static_cast<HairConstants*>(properties->GetProperties());

	int baseColorMap = ID_DI;
	int opacityMap = ID_OP;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;
	
	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		opacityMap = PhysicalMaterial::CutoutMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
	}
	else if (props->flags[HairConstantsFlags::doubleSided])
		material->SetTwoSided(TRUE);

	if (textures[0] && props->flags[HairConstantsFlags::alphaTest])
	{
		textures[0]->SetAlphaSource(ALPHA_FILE);
		textures[0]->SetAlphaAsMono(TRUE);
		material->SetSubTexmap(opacityMap, textures[0]);
	}

	material->SetSubTexmap(baseColorMap, textures[0]);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Red);

	material->SetSubTexmap(metallicMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(roughnesMap, mask);

	material->SetSubTexmap(bumpMap, NormalBump(textures[1]));
}

ADFMATERIAL_WPROPS(CharacterConstants)
{
	CharacterConstants *props = static_cast<CharacterConstants*>(properties->GetProperties());

	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;
	int emissiveMap = ID_SI;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
		emissiveMap = PhysicalMaterial::EmissionColorMap;
		PhysicalMaterial(material).EmissionLuminance(100.0f);
	}

	Texmap *diff = textures[0];
	Texmap *bump = textures[1];

	if (props->flags[CharacterConstantsFlags::useDetail])
	{
		if (textures[4])
		{
			textures[4]->GetUVGen()->SetUScl(props->detailTilingFactorUV.X, 0);
			textures[4]->GetUVGen()->SetVScl(props->detailTilingFactorUV.Y, 0);
			diff = RGBMultiply(diff, textures[4]);
		}

		if (textures[5])
		{
			textures[5]->GetUVGen()->SetUScl(props->detailTilingFactorUV.X, 0);
			textures[5]->GetUVGen()->SetVScl(props->detailTilingFactorUV.Y, 0);
			bump = RGBMultiply(bump, textures[5]);
		}
	}

	if (props->flags[CharacterConstantsFlags::useTint])
	{
		IColorMask *tintMask = IColorMask::Create();
		tintMask->SetSubTexmap(0, textures[8]);
		tintMask->SetDecomposeType(Decompose_Red);

		IColorVar *tVar = IColorVar::Create();
		tVar->SetName(_T("Color 0"));
		
		IColorVar *tVar2 = IColorVar::Create();
		tVar2->SetName(_T("Color 1"));

		Texmap *reslt = Mix(tVar, tVar2, tintMask);

		tVar = IColorVar::Create();
		tVar->SetName(_T("Color 2"));

		tintMask = IColorMask::Create();
		tintMask->SetSubTexmap(0, textures[8]);
		tintMask->SetDecomposeType(Decompose_Green);

		reslt = Mix(reslt, tVar, tintMask);

		tVar = IColorVar::Create();
		tVar->SetName(_T("Color 3"));

		tintMask = IColorMask::Create();
		tintMask->SetSubTexmap(0, textures[8]);
		tintMask->SetDecomposeType(Decompose_Blue);

		reslt = Mix(reslt, tVar, tintMask);
		diff = RGBMultiply(diff, reslt);
	}

	material->SetSubTexmap(baseColorMap, diff);
	material->SetSubTexmap(emissiveMap, textures[3]);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Red);
	material->SetSubTexmap(metallicMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(roughnesMap, mask);

	material->SetSubTexmap(bumpMap, NormalBump(bump));
}

ADFMATERIAL_WPROPS(CharacterSkinConstants)
{
	// 3, 4 detail, useDetail, never used
	// 5 wrinkle?, useWrinkle, never used
	// 6 fur, useFur, used on animals, object space normal map or fur distribution heightmap?

	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
	}

	Texmap *diff = textures[0];
	Texmap *bump = textures[1];

	material->SetSubTexmap(baseColorMap, diff);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Red);
	material->SetSubTexmap(metallicMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(roughnesMap, mask);

	material->SetSubTexmap(bumpMap, NormalBump(bump));
}

ADFMATERIAL_WPROPS(CarPaintConstants)
{
	// 3 unk, emisive?
	// 4 mask, tint
	// 5 dirt mask, stale
	// 6 vehicle metal, stale
	// 7 damage normal, stale
	// 8 decal diffuse, uv2
	// 9 decal normal
	// 10 decal mpm
	// 11 average layer, uv3

	CarPaintConstants *props = static_cast<CarPaintConstants*>(properties->GetProperties());

	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
	}

	Texmap *diff = textures[0];
	Texmap *bump = textures[1];
	Texmap *mpm = textures[2];

	if (props->flags0[CarPaintConstantsFlags0::tint])
	{
		IColorMask *tintMask = IColorMask::Create();
		tintMask->SetSubTexmap(0, textures[4]);
		tintMask->SetDecomposeType(Decompose_Red);

		IColorVar *tVar = IColorVar::Create();
		tVar->SetName(_T("Color 0"));

		IColorVar *tVar2 = IColorVar::Create();
		tVar2->SetName(_T("Color 1"));

		Texmap *reslt = Mix(tVar, tVar2, tintMask);

		tVar = IColorVar::Create();
		tVar->SetName(_T("Color 2"));

		tintMask = IColorMask::Create();
		tintMask->SetSubTexmap(0, textures[4]);
		tintMask->SetDecomposeType(Decompose_Green);

		reslt = Mix(reslt, tVar, tintMask);

		tVar = IColorVar::Create();
		tVar->SetName(_T("Color 3"));

		tintMask = IColorMask::Create();
		tintMask->SetSubTexmap(0, textures[4]);
		tintMask->SetDecomposeType(Decompose_Blue);

		reslt = Mix(reslt, tVar, tintMask);
		diff = RGBMultiply(diff, reslt);
	}

	if (props->flags0[CarPaintConstantsFlags0::decals])
	{
		if (textures[8])
		{
			textures[8]->GetUVGen()->SetMapChannel(2);
			textures[8]->SetAlphaSource(ALPHA_FILE);
			textures[8]->SetAlphaAsMono(TRUE);
		}

		if (textures[9])
			textures[9]->GetUVGen()->SetMapChannel(2);

		if (textures[10])
			textures[10]->GetUVGen()->SetMapChannel(2);

		IColorMask *mask = IColorMask::Create();
		mask->SetSubTexmap(0, textures[8]);
		mask->SetName(_T("Decal mask"));
		mask->SetDecomposeType(Decompose_Alpha);

		diff = Mix(diff, textures[8], mask);
		mpm = Mix(mpm, textures[10], mask);
		bump = Mix(bump, textures[9], mask);
	}

	if (textures[11])
	{
		textures[11]->GetUVGen()->SetMapChannel(3);
		diff = Mix(diff, textures[11]);
	}

	material->SetSubTexmap(baseColorMap, diff);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, mpm);
	mask->SetDecomposeType(Decompose_Red);
	material->SetSubTexmap(metallicMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, mpm);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(roughnesMap, mask);

	material->SetSubTexmap(bumpMap, NormalBump(bump));
}

ADFMATERIAL_WPROPS(WindowConstants)
{
	WindowConstants *props = static_cast<WindowConstants*>(properties->GetProperties());

	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int opacityMap = ID_OP;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		opacityMap = PhysicalMaterial::CutoutMap;
	}
	else
		material->SetTwoSided(!props->flags[WindowConstantsFlags::oneSided]);

	if (textures[0])
	{
		textures[0]->SetAlphaSource(ALPHA_FILE);
		textures[0]->SetAlphaAsMono(TRUE);
	}

	material->SetSubTexmap(baseColorMap, RGBMultiply(textures[0], VertexColor()));
	material->SetSubTexmap(opacityMap, textures[0]);
	material->SetSubTexmap(bumpMap, NormalBump(textures[1]));
	material->SetSubTexmap(roughnesMap, textures[2]);
}

ADFMATERIAL_WPROPS(CarLightConstants)
{
	CarLightConstants *props = static_cast<CarLightConstants*>(properties->GetProperties());

	if (textures[3])
	{
		textures[3]->GetUVGen()->SetUScl(props->detailTiling.X, 0);
		textures[3]->GetUVGen()->SetVScl(props->detailTiling.Y, 0);
	}

	if (textures[4])
	{
		textures[4]->GetUVGen()->SetUScl(props->detailTiling.X, 0);
		textures[4]->GetUVGen()->SetVScl(props->detailTiling.Y, 0);
	}

	RBMCarLightMaterialLoad(properties, material, textures);
}

ADFMATERIAL_WPROPS(GeneralConstants)
{
	GeneralConstants *props = static_cast<GeneralConstants*>(properties->GetProperties());

	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int opacityMap = ID_OP;
	int emissiveMap = ID_SI;
	int dispMap = ID_DP;
	int metallicMap = ID_SH;

	if (textures[5])
	{
		textures[5]->GetUVGen()->SetUScl(props->detailNormalTileU, 0);
		textures[5]->GetUVGen()->SetVScl(props->detailNormalTileV, 0);
	}

	if (textures[10])
		textures[10]->GetUVGen()->SetMapChannel(2);

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		opacityMap = PhysicalMaterial::CutoutMap;
		emissiveMap = PhysicalMaterial::EmissionColorMap;
		dispMap = PhysicalMaterial::DisplacementMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
	}

	Texmap *diff = Mix(textures[0], textures[6], textures[10]);
	Texmap *bump = Mix(textures[1], textures[7], textures[10]);
	Texmap *mpm = Mix(textures[2], textures[8], textures[10]);
	Texmap *tess = Mix(textures[3], textures[9], textures[10]);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, mpm);
	mask->SetDecomposeType(Decompose_Red);
	material->SetSubTexmap(metallicMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, mpm);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(roughnesMap, mask);

	material->SetSubTexmap(baseColorMap, RGBMultiply(diff, VertexColor()));
	material->SetSubTexmap(emissiveMap, textures[4]);
	material->SetSubTexmap(bumpMap, NormalBump(RGBMultiply(bump, textures[5])));
	material->SetSubTexmap(dispMap, tess);
}

ADFMATERIAL_WPROPS(GeneralR2Constants)
{
	/*
	0 diff
	1 nrm
	2 mpm
	3 null
	4 emisive
	5 detail diff
	6 detail nrm
	7 blend mask, uv2, red 1-2, green red-3?
	8 diff 2, uv2
	9 nrm 2, uv2
	10 mpm2, uv2
	11 decal? always dummy
	12 tint, uv2 (average?)
	13 nrm, always dummy
	14 diff 3
	15 nrm 3
	16 mpm 3
	17 color var mask
	*/

	GeneralR2Constants *props = static_cast<GeneralR2Constants *>(properties->GetProperties());

	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int opacityMap = ID_OP;
	int emissiveMap = ID_SI;
	int metallicMap = ID_SH;

	if (textures[7])
		textures[7]->GetUVGen()->SetMapChannel(2);

	if (textures[8])
		textures[8]->GetUVGen()->SetMapChannel(2);

	if (textures[9])
		textures[9]->GetUVGen()->SetMapChannel(2);

	if (textures[10])
		textures[10]->GetUVGen()->SetMapChannel(2);

	if (textures[12] && props->flags[GeneralR2ConstantsFlags::tintUV2])
		textures[12]->GetUVGen()->SetMapChannel(2);

	if (textures[17])
		textures[17]->GetUVGen()->SetMapChannel(2);

	if (textures[5])
	{
		textures[5]->GetUVGen()->SetUScl(props->detailRepeatU, 0);
		textures[5]->GetUVGen()->SetVScl(props->detailRepeatV, 0);

		if (props->flags[GeneralR2ConstantsFlags::detailUV2])
			textures[5]->GetUVGen()->SetMapChannel(2);
	}

	if (textures[6])
	{
		textures[6]->GetUVGen()->SetUScl(props->detailRepeatU, 0);
		textures[6]->GetUVGen()->SetVScl(props->detailRepeatV, 0);

		if (props->flags[GeneralR2ConstantsFlags::detailUV2])
			textures[6]->GetUVGen()->SetMapChannel(2);
	}

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		opacityMap = PhysicalMaterial::CutoutMap;
		emissiveMap = PhysicalMaterial::EmissionColorMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
	}

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[7]);
	mask->SetDecomposeType(Decompose_Red);

	Texmap *diff = Mix(RGBMultiply(textures[0], textures[5]), textures[8], mask);
	Texmap *bump = Mix(RGBMultiply(textures[1], textures[6]), textures[9], mask);
	Texmap *mpm = Mix(textures[2], textures[10], mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[7]);
	mask->SetDecomposeType(Decompose_Green);

	diff = Mix(diff, textures[14], mask);
	bump = Mix(bump, textures[15], mask);
	mpm = Mix(mpm, textures[16], mask);

	CompositeTex comp;
	CompositeTex::Layer layer1 = comp.GetLayer(0);
	layer1.Map(diff);

	layer1 = comp.AddLayer();
	layer1.Map(textures[12]);
	layer1.BlendMode(CompositeTex::Average);

	if (props->flags[GeneralR2ConstantsFlags::useColorMask] && textures[17])
	{
		IColorVar *var1 = IColorVar::Create();
		var1->SetName(_T("Tint Color1"));

		IColorVar *var2 = IColorVar::Create();
		var2->SetName(_T("Tint Color2"));

		IColorMask *mask = IColorMask::Create();
		mask->SetSubTexmap(0, textures[17]);
		mask->SetDecomposeType(Decompose_Red);

		diff = Mix(var1, var2, mask);

		var1 = IColorVar::Create();
		var1->SetName(_T("Tint Color3"));

		mask = IColorMask::Create();
		mask->SetSubTexmap(0, textures[17]);
		mask->SetDecomposeType(Decompose_Green);

		diff = Mix(diff, var1, mask);

		var1 = IColorVar::Create();
		var1->SetName(_T("Tint Color4"));

		mask = IColorMask::Create();
		mask->SetSubTexmap(0, textures[17]);
		mask->SetDecomposeType(Decompose_Blue);

		diff = Mix(diff, var1, mask);

		layer1 = comp.AddLayer();
		layer1.Map(diff);
		layer1.BlendMode(CompositeTex::Multiply);
	}

	diff = comp;

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, mpm);
	mask->SetDecomposeType(Decompose_Red);
	material->SetSubTexmap(metallicMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, mpm);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(roughnesMap, mask);

	material->SetSubTexmap(baseColorMap, RGBMultiply(diff, VertexColor()));
	material->SetSubTexmap(emissiveMap, textures[4]);
	material->SetSubTexmap(bumpMap, NormalBump(bump));
}

ADFMATERIAL_WPROPS(GeneralMkIIIConstants) // only 1 model, Generation Zero
{
	GeneralMkIIIConstants *props = static_cast<GeneralMkIIIConstants *>(properties->GetProperties());

	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
	}

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[1]);
	mask->SetDecomposeType(Decompose_Red);

	material->SetSubTexmap(baseColorMap, textures[0]);
	material->SetSubTexmap(roughnesMap, mask);
	material->SetSubTexmap(metallicMap, textures[2]);
	material->SetSubTexmap(bumpMap, NormalBump(textures[3]));
}

ADFMATERIAL_WPROPS(FoliageConstants_GZ)
{
	/*FoliageConstants_GZ
	0 diff
	1 nrm
	2 ao
	3 mpm
	*/
	RBMVegetationFoliage3MaterialLoad(properties, material, textures);
}

ADFMATERIAL(BarkConstants_GZ)
{
	/*BarkConstants_GZ
	0 diff
	1 nrm
	2 mpm
	3 blend mask?
	4 dummy mask
	5 diff 2
	6 nrm 2
	7 dummy black
	8 mpm 2
	*/

	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;

	if (textures[3])
		textures[3]->GetUVGen()->SetMapChannel(2);

	if (textures[4])
		textures[4]->GetUVGen()->SetMapChannel(2);

	if (textures[0])
	{
		textures[0]->SetAlphaSource(ALPHA_FILE);
		textures[0]->SetAlphaAsMono(TRUE);
	}

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
	}

	Texmap *hmap = RGBMultiply(textures[3], textures[4]);

	material->SetSubTexmap(baseColorMap, Mix(textures[0], textures[5], hmap));

	Texmap *mpmMix = Mix(textures[2], textures[8], hmap);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, mpmMix);
	mask->SetDecomposeType(Decompose_Red);

	material->SetSubTexmap(metallicMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, mpmMix);
	mask->SetDecomposeType(Decompose_Green);
	material->SetSubTexmap(roughnesMap, mask);

	material->SetSubTexmap(bumpMap, NormalBump(Mix(textures[1], textures[6], hmap)));
}

ADFMATERIAL_WPROPS(CarLightConstants_GZ)
{
	/*CarLightConstants_GZ
	0 diff
	1 nrm
	2 mpm
	3 detail diff
	4 detail nrm
	5 emisive mask? dummy white
	*/
	RBMCarLightMaterialLoad(properties, material, textures);
}

ADFMATERIAL_WPROPS(GeneralJC3Constants_HU)
{
	/*GeneralJC3Constants_HU
	0 diff
	1 nrm
	2 mpm
	3 ao
	*/
	RBMGeneralSimpleMaterialLoad(properties, material, textures);
}

ADFMATERIAL_WPROPS(CarPaintMMConstants_HU)
{
	/*CarPaintMMConstants_HU
	0 diff
	1 nrm
	2 mpm
	3 body mask
	4 damage nrm
	5 damage diff
	6 dirt mask
	7 decal diff
	8 decal nrm
	9 decal mpm
	10 detail diff, layered diff
	11 layerred diff 2
	*/
	RBMCarPaint14MaterialLoad(properties, material, textures);
}

ADFMATERIAL_WPROPS(GeneralConstants_HU)
{
	RBMGeneralSimpleMaterialLoad(properties, material, textures);
}

ADFMATERIAL(PropConstants_HU)
{
	/*PropConstants_HU
	0 diff
	1 nrm
	2 mpm
	3 dummy checker
	4 dummy mpm
	5 dummy nrm
	*/

	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
	}

	material->SetSubTexmap(baseColorMap, textures[0]);

	Texmap *mpmMix = textures[2];

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, mpmMix);
	mask->SetDecomposeType(Decompose_Red);

	material->SetSubTexmap(metallicMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, mpmMix);
	mask->SetDecomposeType(Decompose_Green);
	material->SetSubTexmap(roughnesMap, mask);

	material->SetSubTexmap(bumpMap, NormalBump(textures[1]));
}

ADFMATERIAL_WPROPS(CharacterConstants_HU)
{
	/*CharacterConstants_HU
	0 diff
	1 nrm
	2 mpm
	*/

	CharacterSkinConstantsMaterialLoad(properties, material, textures);
}

ADFMATERIAL_WPROPS(GeneralR2Constants_HU)
{
	GeneralR2ConstantsMaterialLoad(properties, material, textures);
}

ADFMATERIAL_WPROPS(CharacterConstants_GZ)
{
	/*CharacterConstants_GZ
	0 diff
	1 nrm
	2 mpm
	3 dummy grey
	4 dummy nrm
	5 dummy
	6 dummy
	7 dummy black
	8 detail spec?(eyewear)
	*/
	RBMCharacter6MaterialLoad(properties, material, textures);
}

ADFMATERIAL_WPROPS(CharacterSkinConstants_GZ)
{
	/*CharacterSkinConstants_GZ
	0 diff
	1 nrm
	2 mpm
	3 dummy
	4 dummy nrm
	5 dummy
	6 dummy
	7 dummy nrm
	*/
	RBMCharacter6MaterialLoad(properties, material, textures);
}

ADFMATERIAL_WPROPS(HairConstants_GZ)
{
	/*HairConstants_GZ
	0 diff
	1 nrm
	2 mpm
	3 color mask
	*/

	HairConstants_GZ *props = static_cast<HairConstants_GZ *>(properties->GetProperties());

	int baseColorMap = ID_DI;
	int opacityMap = ID_OP;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int metallicMap = ID_SH;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		opacityMap = PhysicalMaterial::CutoutMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		metallicMap = PhysicalMaterial::MetalnessMap;
	}
	else if (props->flags[HairConstantsFlags_GZ::doubleSided])
		material->SetTwoSided(TRUE);

	if (textures[0] && props->flags[HairConstantsFlags_GZ::alphaTest])
	{
		textures[0]->SetAlphaSource(ALPHA_FILE);
		textures[0]->SetAlphaAsMono(TRUE);
		material->SetSubTexmap(opacityMap, textures[0]);
	}

	Texmap *diff = textures[0];

	if (props->flags[HairConstantsFlags_GZ::useColorMask])
	{
		IColorVar *var1 = IColorVar::Create();
		var1->SetName(_T("Tint Color1"));

		IColorVar *var2 = IColorVar::Create();
		var2->SetName(_T("Tint Color2"));

		IColorMask *mask = IColorMask::Create();
		mask->SetSubTexmap(0, textures[3]);
		mask->SetDecomposeType(Decompose_Red);

		diff = Mix(var1, var2, mask);

		var1 = IColorVar::Create();
		var1->SetName(_T("Tint Color3"));

		mask = IColorMask::Create();
		mask->SetSubTexmap(0, textures[3]);
		mask->SetDecomposeType(Decompose_Green);

		diff = Mix(diff, var1, mask);

		var1 = IColorVar::Create();
		var1->SetName(_T("Tint Color4"));

		mask = IColorMask::Create();
		mask->SetSubTexmap(0, textures[3]);
		mask->SetDecomposeType(Decompose_Blue);

		diff = Mix(diff, var1, mask);
		diff = RGBMultiply(textures[0], diff);
	}

	material->SetSubTexmap(baseColorMap, diff);

	IColorMask *mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Red);

	material->SetSubTexmap(metallicMap, mask);

	mask = IColorMask::Create();
	mask->SetSubTexmap(0, textures[2]);
	mask->SetDecomposeType(Decompose_Blue);
	material->SetSubTexmap(roughnesMap, mask);

	material->SetSubTexmap(bumpMap, NormalBump(textures[1]));
}

ADFMATERIAL_WPROPS(WindowConstants_GZ)
{
	/*WindowConstants_GZ
	1 diff
	2 nrm
	3 mpm
	4 bullet nrm
	5 bullet mpm
	6 crack nrm
	7 crack mpm
	*/

	WindowConstants_GZ *props = static_cast<WindowConstants_GZ *>(properties->GetProperties());

	int baseColorMap = ID_DI;
	int bumpMap = ID_BU;
	int roughnesMap = ID_SS;
	int opacityMap = ID_OP;

	if (material->ClassID() == PHYSIC_MAT_CLASSID)
	{
		baseColorMap = PhysicalMaterial::BaseColorMap;
		bumpMap = PhysicalMaterial::BumpMap;
		roughnesMap = PhysicalMaterial::RoughnessMap;
		opacityMap = PhysicalMaterial::CutoutMap;
	}
	else
		material->SetTwoSided(!props->flags[WindowConstantsFlags_GZ::oneSided]);

	if (textures[0])
	{
		textures[0]->SetAlphaSource(ALPHA_FILE);
		textures[0]->SetAlphaAsMono(TRUE);
	}

	material->SetSubTexmap(baseColorMap, RGBMultiply(textures[0], VertexColor()));
	material->SetSubTexmap(opacityMap, textures[0]);
	material->SetSubTexmap(bumpMap, NormalBump(textures[1]));
	material->SetSubTexmap(roughnesMap, textures[2]);
}


#define ADDMATERIAL(classname) {classname::ADFHASH, classname##MaterialLoad},
#define ADDMATERIALADF(classname) {classname::HASH, classname##MaterialLoad},

static const std::map<ApexHash, void(*)(AdfProperties *, StdMat2 *, TexmapMapping &)> materialStorage =
{
	StaticFor(ADDMATERIAL,
		RBMCarPaintSimple,
		RBMFoliageBark,
		RBMVegetationFoliage,
		RBMBillboardFoliage,
		RBMHalo,
		RBMLambert,
		RBMFacade,
		RBMGeneral,
		RBMWindow,
		RBMMerged,
		RBMSkinnedGeneral,
		RBMCarPaint,
		RBMDeformWindow,

		RBMFacade0,
		RBMGeneral0,
		RBMUIOverlay,
		RBMScope,
		RBMSkinnedGeneral0,
		RBMSkinnedGeneralDecal,

		RBMVegetationFoliage3,
		RBMFoliageBark2,
		RBMGeneralSimple,
		RBMBavariumShiled,
		RBMWindow1,
		RBMLayered,
		RBMLandmark,
		RBMGeneralMK3,
		RBMGeneral6,
		RBMCarLight,
		RBMCarPaint14,
		RBMGeneral3,
		RBMCharacter9,
		RBMCharacter6,
		RBMRoad,
		RBMGeneralSimple3,

		RBNGeneral,
		RBNCarPaint,
		RBNCharacter,
		RBNWindow,
		RBNXXXX
	)

	StaticFor(ADDMATERIALADF,
		LandmarkConstants,
		EmissiveUIConstants,
		HologramConstants,
		FoliageConstants,
		BarkConstants,
		EyeGlossConstants,
		HairConstants,
		CharacterConstants,
		CharacterSkinConstants,
		CarPaintConstants,
		CarLightConstants,
		WindowConstants,
		GeneralConstants,
		GeneralR2Constants,
		GeneralMkIIIConstants,
		FoliageConstants_GZ,
		BarkConstants_GZ,
		CarLightConstants_GZ,
		GeneralJC3Constants_HU,
		CarPaintMMConstants_HU,
		GeneralConstants_HU,
		PropConstants_HU,
		CharacterConstants_HU,
		CharacterSkinConstants_GZ,
		HairConstants_GZ,
		WindowConstants_GZ
	)
};

bool LinkedTexmap(MtlBase *item, Texmap *ref)
{
	int numsubs = item->NumSubTexmaps();

	if (item->ClassID() == Class_ID(COMPOSITE_CLASS_ID, 0))
	{
		CompositeTex tx(static_cast<MultiTex*>(item));

		int numLayers = tx.NumLayers();

		for (int t = 0; t < numLayers; t++)
		{
			Texmap *subitem = tx.GetLayer(t).Map();

			if (subitem == ref)
				return true;
			else if (subitem && LinkedTexmap(subitem, ref))
				return true;

			subitem = tx.GetLayer(t).Mask();

			if (subitem == ref)
				return true;
			else if (subitem && LinkedTexmap(subitem, ref))
				return true;
		}
	}
	else
		for (int t = 0; t < numsubs; t++)
		{
			Texmap *subitem = item->GetSubTexmap(t);

			if (subitem == ref)
				return true;
			else if (subitem && LinkedTexmap(subitem, ref))
				return true;
		}

	return false;
}

Mtl *CreateMaterial(AmfMaterial *material)
{
	StdMat2 *mat = nullptr;

	if (material->materialType == AmfMaterial::MaterialType_PBR)
		mat = PhysicalMaterial();

	if (!mat)
		mat = NewDefaultStdMat();

	mat->SetName(static_cast<TSTRING>(esString(material->name->string)).c_str());

	if (!material->attributes)
	{
		printerror("Could not find attributes for: ", << mat->GetName());
		return mat;
	}

	if (!materialStorage.count(material->attributes->typeHash))
	{
		printerror("Could not find material function for: ", << mat->GetName());
		return mat;
	}

	TexmapMapping texmaps;
	texmaps.reserve(material->textures.size());

	for (auto &t : material->textures)
	{
		BitmapTex *ctex = nullptr;

		if (t->hash)
		{
			ctex = NewDefaultBitmapTex();
			TSTRING mapName = esString(t->string);
			ctex->SetMapName(mapName.c_str());
			ctex->SetName(TFileInfo(mapName).GetFileName().c_str());
		}
		
		texmaps.push_back(ctex);
	}

	materialStorage.at(material->attributes->typeHash)(material->attributes, mat, texmaps);

	int texID = 0;

	for (auto &t : texmaps)
	{
		if (t && !LinkedTexmap(mat, t))
		{
			printwarning("Unused texture[", << texID << "] \"" << t->GetMapName() << "\" for: " << mat->GetName())
		}

		texID++;
	}

	return mat;
}