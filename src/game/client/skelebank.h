#ifndef GAME_CLIENT_SKELEBANK_H
#define GAME_CLIENT_SKELEBANK_H

#include <math.h>
#include <base/math.h>

#include <game/mapitems.h>
#include <game/client/render.h>
#include <game/client/spine.h>


enum Skeletons
{
	SKELETON_BODY1,
	SKELETON_BODY2,
	SKELETON_BODY3,
	SKELETON_MONSTER1,
	NUM_SKELETONS
};

enum Atlases
{
	ATLAS_BODY1,
	ATLAS_BODY2,
	ATLAS_BODY3,
	ATLAS_MONSTER1,
	NUM_ATLASES
};


class CSkelebankSkeleton : public CAnimSkeletonInfo
{
public:
	//CSkelebank *m_pSkelebank;

	//CSkelebankSkeleton(CSkelebank *pSkelebank)
	CSkelebankSkeleton()
	{
		//m_pSkelebank = pSkelebank;
		m_aName[0] = 0;
		m_External = 0;
		m_pJsonData = 0x0;
	}

	~CSkelebankSkeleton()
	{
		mem_free(m_pJsonData); // eh..
	}
	
	char m_aName[128];
	int m_External;
	char *m_pJsonData; // owned-ptr
};


class CSkelebankAtlas : public CTextureAtlas
{
public:
	//CSkelebank *m_pSkelebank;

	//CSkelebankAtlas(CSkelebank *pSkelebank)
	CSkelebankAtlas()
	{
		//m_pSkelebank = pSkelebank;
		m_aName[0] = 0;
		m_External = 0;
	}

	~CSkelebankAtlas()
	{
		mem_free(m_pAtlasData); // eh..
	}

	char m_aName[128];
	int m_External;
	char *m_pAtlasData; // owned_ptr
};



class CSkelebank
{
	CSpineReader m_SpineReader;
	class IStorage *m_pStorage;
	CRenderTools *m_pRenderTools;
	
public:
	CSkelebank(CRenderTools *pRenderTools);
	~CSkelebank();
	
	void Init(class IStorage *pStorage);
	
	CSpineReader *SpineReader() { return &m_SpineReader; }
	class IStorage *Storage() { return m_pStorage; };
	CRenderTools *RenderTools() { return m_pRenderTools; }
	
	array<CSkelebankSkeleton*> m_lSkeletons;
	array<CSkelebankAtlas*> m_lAtlases;

	
	
	void AddSkeleton(const char *pFilename, int StorageType);
	void AddAtlas(const char *pFilename, int StorageType);
	

	static void ExtractName(const char *pFileName, char *pName, int BufferSize)
	{
		const char *pExtractedName = pFileName;
		const char *pEnd = 0;
		for(; *pFileName; ++pFileName)
		{
			if(*pFileName == '/' || *pFileName == '\\')
				pExtractedName = pFileName+1;
			else if(*pFileName == '.')
				pEnd = pFileName;
		}

		int Length = pEnd > pExtractedName ? min(BufferSize, (int)(pEnd-pExtractedName+1)) : BufferSize;
		str_copy(pName, pExtractedName, Length);
	}
};


#endif
