

#ifndef __SwfManager_H__
#define __SwfManager_H__

#include "cocos2d.h"
#include "gameswf.h"
#include "gameswf_player.h"
#include "gameswf_root.h"
#include "atlas/Atlas.h"
using namespace cocos2d;

class  SwfManager : public Node
{
public:
	enum 
	{
		SWF_NONE,
		SWF_PARSING,
		SWF_GEN_IMAGE_START,
		SWF_GEN_IMAGE_END,
	};

	typedef std::function<void(bool)> GenDataCallback;

	struct CallBackData{
		std::string sName;
		GenDataCallback cb;
	};

	SwfManager();
    virtual ~SwfManager();
	static SwfManager* getInstance();
	virtual bool init();

	void genAniData(std::string sFile, GenDataCallback func);
	void setCachePath(std::string sPath) { m_sCachePath = sPath; };
	std::string getFileName(std::string& sPath);

protected:
	void loop(float dt);
	bool genAtlasImage();
	void checkToGenData();
	void dealGenImageEnd();
	bool parseSwf(std::string sFile);
	void updatePath();
	void threadProc();
	void setParseState(int nStatus);

	static void* _threadProc(void* arg);

	std::list<CallBackData> m_list;
	std::map<std::string, GenDataCallback> m_sFiles;
	std::string m_srcPath;
	std::string m_desPath;
	std::string m_sCachePath;
	gameswf::player* m_pPlayer ;
	gameswf::render_handler* m_pRender;
	gameswf::glyph_provider* m_glyph_provider;
	Atlas m_sAtlas;
	int m_nParsingState = SWF_NONE;
	std::thread* m_thread;
	bool m_bThreadEnd;
	std::string m_sCurKey;

	bool m_bHadCreateBitmap;
	bool m_bGenAltasSuccess;
};

#endif  // SwfManager

