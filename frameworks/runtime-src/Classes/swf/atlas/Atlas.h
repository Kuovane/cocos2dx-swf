
#ifndef __XG_ATLAS_H__
#define __XG_ATLAS_H__


#include "VxFile.h"

class MyFileData {
public:
	MyFileData()
	{
		pBufData = nullptr;
		len = 0;
		width = 0;
		height = 0;
		isPng = false;
		isInPackage = false;
		bufPos = 0;
		m_bNeedFree = true;
	}

	void setBufPos(int pos) {
		bufPos = pos;
	};


	bool isPngFile()
	{
		return isPng;
	}




	std::string filePath;
	std::string fileName;
	char* pBufData;
	bool isPng;
	bool isInPackage;
	int len;
	int width;
	int height;
	int s;

	int bufPos;
	bool m_bNeedFree;

	struct stbrp_rect *rect;
};


class Atlas
{
public:

	~Atlas();
	void reinit();
	void clear();
	void initPath(std::string& sPath);
	bool genAtalsImage();
	bool genAtlasImageByMem();
	void addBitmapData(unsigned char*pData, std::string sFileName, int w, int h,bool bNeedFree=false);

protected:
	std::string  getRelativePath(std::string &path);
	void loadAllFiles();
	void readAllFiles();
	bool genPackRects();
	bool savePackFile();
	void copyDataToPackData(int x,int y,int w,int h,char* pSrcData,char*pTargetData);
	void copyAllData();
	void writeStrToFile(std::string sStr);

	std::string m_sPath;
	std::string m_sAssetsPath;
	std::vector<std::string> m_allFiles;
	std::vector<std::string> m_allFileNames;

	FILE *m_pPinFile = nullptr;

	std::map<std::string, MyFileData> m_mapFileDataes;
	struct stbrp_rect *m_pRects = nullptr;
	unsigned char* m_targetData = nullptr;
	int m_targetWidth = 2048;
	int m_targetHeight = 2048;
	int m_maxHeight;
	int m_maxWidth;
};

#endif	// __XG_ATLAS_H__
