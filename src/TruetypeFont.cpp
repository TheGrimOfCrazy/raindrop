#include <fstream>
#include "Global.h"
#include "TruetypeFont.h"
#include "GameWindow.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "utf8.h"

TruetypeFont::TruetypeFont(GString Filename, float Scale)
{
	std::ifstream ifs (Filename.c_str(), std::ios::binary);
	
	if (!ifs.is_open())
	{
		IsValid = false;
		return;
	}

	scale = Scale;

	// tell size
	ifs.seekg(0, ifs.end);
	offs = ifs.tellg();
	ifs.seekg(0, ifs.beg);

	data = new unsigned char[offs];
	info = new stbtt_fontinfo;

	// read data
	ifs.read((char*)data, offs);

	IsValid = (stbtt_InitFont(info, data, stbtt_GetFontOffsetForIndex(data, 0)) != 0);
	windowscale = 0;

	if (IsValid)
	{
		SetupTexture();
		UpdateWindowScale();

		WindowFrame.AddTTF(this);
	}
}

TruetypeFont::~TruetypeFont()
{
	WindowFrame.RemoveTTF(this);
	delete data;
	delete info;
	ReleaseTextures();
}

void TruetypeFont::UpdateWindowScale()
{
	if (windowscale == WindowFrame.GetWindowVScale())
		return;

	float oldscale = windowscale;
	windowscale = WindowFrame.GetWindowVScale();

	float oldrealscale = realscale;
	realscale = stbtt_ScaleForPixelHeight(info, scale * windowscale);
	virtualscale = stbtt_ScaleForPixelHeight(info, scale);
#ifdef VERBOSE_DEBUG
	wprintf(L"change scale %f -> %f, realscale %f -> %f\n", oldscale, windowscale, oldrealscale, realscale);
#endif
}

void TruetypeFont::Invalidate()
{
	for (std::map <int, codepdata>::iterator i = Texes.begin();
		i != Texes.end();
		i++)
	{
		i->second.gltx = 0;
	}
}

void TruetypeFont::CheckCodepoint(int cp)
{
	if (Texes.find(cp) != Texes.end())
	{
		if (Texes[cp].scl != windowscale)
		{
#ifdef VERBOSE_DEBUG
			wprintf(L"releasing %d\n", cp);
#endif
			ReleaseCodepoint(cp); // force regeneration if scale changed
		}
	}
}



TruetypeFont::codepdata &TruetypeFont::GetTexFromCodepoint(int cp)
{
	if (Texes.find(cp) == Texes.end())
	{
		codepdata newcp;
		int w, h, xofs, yofs;
#ifdef VERBOSE_DEBUG
		wprintf(L"generating %d\n", cp);
#endif
		newcp.tex = stbtt_GetCodepointBitmap(info, 0, realscale, cp, &w, &h, &xofs, &yofs);
		newcp.gltx = 0;
		newcp.scl = WindowFrame.GetWindowVScale();
		newcp.tw = w;
		newcp.th = h;
		
		// get size etc.. for how it'd be if the screen weren't resized
		void * tx = stbtt_GetCodepointBitmap(info, 0, virtualscale, cp, &w, &h, &xofs, &yofs);
		newcp.xofs = xofs;
		newcp.yofs = yofs;
		newcp.w = w;
		newcp.h = h;

		free(tx);
		Texes[cp] = newcp;
		return Texes[cp];
	}else
	{
		return Texes[cp];
	}
}

float TruetypeFont::GetHorizontalLength(const char *In)
{
	const char* Text = In;
	size_t len = strlen(In);
	float Out = 0;

	try {
		utf8::iterator<const char*> it(Text, Text, Text + len);
		utf8::iterator<const char*> itend(Text + len, Text, Text + len);
		for (; it != itend; ++it)
		{
			CheckCodepoint(*it); // Force a regeneration of this if necessary
			codepdata &cp = GetTexFromCodepoint(*it);
			Out += cp.w + cp.xofs;
		}
	}
	catch (...)
	{
	}

	return Out;
}
