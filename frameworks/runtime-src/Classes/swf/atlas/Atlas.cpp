
#include "Atlas.h"
#include "VxConvert.h"
#include "VxFile.h"
#include "image/stb_rect_pack.h"
#include "image/stb_image_write.h"
#include "image/stb_image.h"
#include <assert.h>


Atlas::~Atlas()
{
	clear();
	if (m_targetData)
	{
		delete[]m_targetData;
	}
}

std::string  Atlas::getRelativePath(std::string &path)
{
	std::string sTemp = path; ;
	VxConvert::replace_str(sTemp, m_sAssetsPath , "");
	VxConvert::replace_str(sTemp, "\\", "");
	VxConvert::replace_str(sTemp, "/", "");
	return sTemp;
}



void Atlas::loadAllFiles()
{
	for (auto it = m_allFiles.begin(); it != m_allFiles.end(); ++it)
	{

		if ((*it).rfind(".png") == std::string::npos)
		{
				continue;
		}
		int width,  height,  nrChannels;
	
		 char *data = (char*)stbi_load((*it).c_str(), &width, &height, &nrChannels, 0);




		 if (nrChannels != 4)
		 {
			 stbi_image_free(data);
			 continue;
		 }
		{

			MyFileData sData;
			sData.filePath = (*it);
			sData.fileName = getRelativePath(sData.filePath);
			sData.len = width* height*nrChannels;
			sData.pBufData = data;
			sData.isPng = true;
			sData.width = width;
			sData.height = height;
			sData.m_bNeedFree = true;

			m_mapFileDataes[sData.filePath] = sData;
		}
	}

}

void Atlas::readAllFiles()
{
	VxFile::getFiles(m_sAssetsPath, m_allFiles, m_allFileNames);
	loadAllFiles();
}

bool Atlas::genPackRects()
{
	stbrp_context context;

	auto len = m_mapFileDataes.size();
	m_pRects = (stbrp_rect*)malloc(sizeof(stbrp_rect)*len);// [len];
	struct stbrp_rect *rects = m_pRects;
	int  id = 0;
	for (auto it = m_mapFileDataes.begin(); it != m_mapFileDataes.end(); it++)
	{
		int i = id;
		rects[i].id = id;

		rects[i].w =it->second.width+2;
		rects[i].h = it->second.height+2;
		rects[i].x = 0;
		rects[i].y = 0;
		rects[i].was_packed = 0;
		it->second.rect = &rects[i];
		id = id + 1;
	}

	int rectsLength = len;// sizeof(rects) / sizeof(rects[0]);

	int nodeCount = m_targetWidth * 2;
	int  mlen = sizeof(stbrp_node)*nodeCount;
	struct stbrp_node *nodes = (stbrp_node *)malloc(mlen) ;


	stbrp_init_target(&context, m_targetWidth, m_targetHeight, nodes, nodeCount);
	auto ret = stbrp_pack_rects(&context, rects, rectsLength);

	if (ret == 0)
	{
		CCLOG("合图失败");
		CCASSERT(false, "合图失败");
		//assert(false,"合图失败");
	}

	free(nodes) ;
	return ret;
}

void Atlas::copyDataToPackData(int x, int y, int w, int h, char* pData, char*pTargetData)
{
	//h = y + h;
	for (int i = 0; i < h; ++i)
	{
		memcpy(pTargetData+ ((i+y) * m_targetWidth + x) * 4, pData+i*w*4, w * 4);
	}
}

void Atlas::copyAllData()
{
	m_maxHeight = 0;
	m_maxWidth = 0;
	for (auto it = m_mapFileDataes.begin(); it != m_mapFileDataes.end(); it++)
	{
		int h = it->second.rect->y + it->second.rect->h;
		if (m_maxHeight < h)
		{
			m_maxHeight = h;
		}
		int w = it->second.rect->x + it->second.rect->w;
		if (m_maxWidth < w)
		{
			m_maxWidth = w;
		}
		

		copyDataToPackData(it->second.rect->x, it->second.rect->y, it->second.width, it->second.height, it->second.pBufData, (char*)m_targetData);
	}
}


void Atlas::writeStrToFile(std::string sStr)
{
	fwrite(sStr.c_str(), sStr.length(), 1, m_pPinFile);
}

bool Atlas::savePackFile()
{
	std::string sSlash = "/";
	int index = m_sPath.rfind(sSlash, m_sPath.length()-2);
	auto sFileName = m_sPath.substr(index+1);
	VxConvert::replace_str(sFileName, sSlash,"");
	std::string sPackName = m_sPath+ sFileName+"_swf_pin.png";
	stbi_write_png_compression_level = 100;
	int nTargetHeight = m_targetHeight;
	int nTargetWidth = m_targetWidth;
	for (int h=64; h <= m_targetHeight; h = h*2)
	{
		if (h > m_maxHeight)
		{
			nTargetHeight = h;
			break;
		}
	}
	for (int w = 64; w <= m_targetWidth; w = w * 2)
	{
		if (w > m_maxWidth)
		{
			nTargetWidth = w;
			break;
		}
	}

	
	unsigned char* pTargetData =(unsigned char*) malloc(nTargetWidth*nTargetHeight * 4);
	for (int i =0; i < nTargetHeight;i++)
	{
		memcpy(pTargetData + nTargetWidth*i*4, m_targetData+ m_targetWidth * i*4, nTargetWidth * 4);
	}
	
	stbi_flip_vertically_on_write(0);
	auto ret = stbi_write_png(sPackName.c_str(), nTargetWidth, nTargetHeight,4, pTargetData,0);
	if (ret == 0)
	{
		return false;
	}
	CCLOG("sPackName %s", sPackName.c_str());

	{
		static char pBuf[512] = { 0 };
		std::string pin = m_sPath + sFileName + "_swf_pin.lua";
		 m_pPinFile = fopen(pin.c_str(), "w");
		if (m_pPinFile)
		{
			memset(pBuf, 0, sizeof(pBuf));
			sprintf(pBuf, "local path=\"%s\"\n", (sFileName + "_swf_pin.png").c_str());
			writeStrToFile(pBuf);

			memset(pBuf, 0, sizeof(pBuf));
			sprintf(pBuf, "local pinMap = {\n");
			writeStrToFile(pBuf);

			for (auto it = m_mapFileDataes.begin(); it != m_mapFileDataes.end(); it++)
			{
				memset(pBuf, 0, sizeof(pBuf));
				sprintf(pBuf, "[\"%s\"]={x=%d,y=%d,width=%d,height=%d},\n", it->second.fileName.c_str(), it->second.rect->x, it->second.rect->y, it->second.width, it->second.height);
				writeStrToFile(pBuf);
				//copyDataToPackData(it->second.rect->x, it->second.rect->y, it->second.width, it->second.height, it->second.pBufData, (char*)m_targetData);
			}

			memset(pBuf, 0, sizeof(pBuf));
			sprintf(pBuf, "}\n");
			writeStrToFile(pBuf);

			memset(pBuf, 0, sizeof(pBuf));
			sprintf(pBuf, "return pinMap");
			writeStrToFile(pBuf);

			fclose(m_pPinFile);
		}
		
	}
	return true;
}

void  Atlas::addBitmapData(unsigned char*pData, std::string sFileName, int w, int h, bool bNeedFree)
{
	MyFileData sData;
	sData.filePath = sFileName;
	sData.fileName = getRelativePath(sData.filePath);
	sData.len = w * h*4;
	sData.pBufData = (char*)pData;
	sData.isPng = true;
	sData.width = w;
	sData.height = h;
	sData.m_bNeedFree = bNeedFree;

	m_mapFileDataes[sData.filePath] = sData;
}

void Atlas::clear()
{
	if (m_pRects)
	{
		free(m_pRects);
		m_pRects = nullptr;
	}

	for (auto it = m_mapFileDataes.begin(); it != m_mapFileDataes.end(); it++)
	{
		if (it->second.m_bNeedFree)
		{
			stbi_image_free(it->second.pBufData);
		}

	}
	m_mapFileDataes.clear();
	m_allFiles.clear();
	m_allFileNames.clear();
}

void Atlas::reinit()
{
	clear();
}

void Atlas::initPath(std::string &sPath)
{
	if (!m_targetData)
	{
		m_targetData = new unsigned char[m_targetWidth * m_targetHeight * 4];
	}
	memset(m_targetData, 0, m_targetWidth * m_targetHeight * 4);
	m_sAssetsPath = sPath + "assets/";
	m_sPath = sPath;
	reinit();
	if (!cocos2d::FileUtils::getInstance()->isDirectoryExist(m_sPath))
	{
		cocos2d::FileUtils::getInstance()->createDirectory(m_sPath);
	}
	
}

bool Atlas::genAtalsImage()
{
	//initPath(sPath);
	readAllFiles();
	bool ret = genPackRects();
	if(!ret)
	{
		return ret;
	}

	copyAllData();
	savePackFile();
	return true;
}

bool Atlas::genAtlasImageByMem()
{
	if (m_mapFileDataes.empty())
	{
		return false;
	}
	bool ret = genPackRects();
	if (!ret)
	{
		return ret;
	}

	copyAllData();
	return savePackFile();
}