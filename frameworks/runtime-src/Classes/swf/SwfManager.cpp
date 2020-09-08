#include "SwfManager.h"


#include "gameswf_types.h"
#include "gameswf_impl.h"
#include "gameswf_character.h"
#include "tu_file.h"
#include "VxConvert.h"


using namespace cocos2d;


static void CCGameSWF_fscommand_handler(gameswf::character* movie, const char* command, const char* arg)
{

}

static tu_file* CCGameSWF_file_opener(const char* url_or_path)
{
	assert(url_or_path);


	auto da = cocos2d::FileUtils::getInstance()->getDataFromFile(url_or_path);

	//CCLOG("tu_file %s %p %d %p", sName.c_str(), m_data, da.getSize(), pFile);

	return new tu_file(tu_file::memory_buffer, da.getSize(), da.getBytes());
}

static void CCGameSWF_log_handler(bool error, const char* message)
{
	if (error) {
		CCLOG("::ERROR:: %s", message);
	}
	else {
		CCLOG(":::: %s", message);
	}
}

static SwfManager *_SwfManagerCache = nullptr;


SwfManager* SwfManager::getInstance()
{
	if (!_SwfManagerCache)
	{
		_SwfManagerCache = new (std::nothrow) SwfManager();
		_SwfManagerCache->init();
		//_SwfManagerCache->retain();
	}

	return _SwfManagerCache;
}


SwfManager::SwfManager()
	:m_pPlayer(nullptr)
	, m_pRender(nullptr)
	, m_glyph_provider(nullptr)
	, m_bThreadEnd(false)
{
	m_nParsingState = SWF_NONE;
	Director::getInstance()->getScheduler()->schedule(schedule_selector(SwfManager::loop), this, 0, false);
	m_thread = new (std::nothrow)std::thread(&SwfManager::_threadProc, this);
	m_thread->detach();
}

SwfManager::~SwfManager()
{
	CC_SAFE_DELETE(m_pRender);
	CC_SAFE_DELETE(m_pPlayer);
	CC_SAFE_DELETE(m_glyph_provider);
	m_bThreadEnd = true;
	CC_SAFE_DELETE(m_thread);
}


bool SwfManager::init()
{
	gameswf::register_file_opener_callback(&CCGameSWF_file_opener);
	gameswf::register_fscommand_callback(&CCGameSWF_fscommand_handler);
	gameswf::register_log_callback(&CCGameSWF_log_handler);
	m_pRender = gameswf::create_render_handler_ogles();
	gameswf::set_render_handler(m_pRender);

	m_glyph_provider = gameswf::create_glyph_provider_tu();
	gameswf::set_glyph_provider(m_glyph_provider);

	m_sCachePath = cocos2d::FileUtils::getInstance()->getWritablePath() + "swf/";
	return true;
}

void SwfManager::loop(float dt)
{
	if (this->getParent() == nullptr)
	{
		Director::getInstance()->getRunningScene()->addChild(this);
		this->setLocalZOrder(-100);
		this->setGlobalZOrder(-100);
	}

	//auto oldMultisample = glIsEnabled(GL_MULTISAMPLE);
	//glEnable(GL_MULTISAMPLE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (m_nParsingState == SWF_NONE)
	{
		checkToGenData();
	}
	else if (m_nParsingState == SWF_PARSING)
	{
		if (m_pPlayer)
		{
			m_bHadCreateBitmap = false;
			for (int i = 0; i < 50; ++i)
			{
				if (!m_pPlayer->isDrawFameEnd())
				{
					if (!m_bHadCreateBitmap)
					{
						m_pPlayer->drawFrame();
					}
					
				}
				else
				{
					setParseState(SWF_GEN_IMAGE_START);
					break;

				}
			}
			
		}
	}
	else if (m_nParsingState == SWF_GEN_IMAGE_END)
	{
		dealGenImageEnd();
		setParseState(SWF_NONE);
	}

	//if (!oldMultisample)
	{
		//glDisable(GL_MULTISAMPLE);
	}
}

void SwfManager::dealGenImageEnd()
{
	auto it = m_sFiles.find(m_sCurKey);
	if (it != m_sFiles.end())
	{
		m_sFiles.erase(it);
	}
	for (auto it = m_list.begin(); it != m_list.end() ; )
	{
		if (memcmp((*it).sName.c_str(), m_sCurKey.c_str(), m_sCurKey.length()) == 0)
		{
			(*it).cb(m_bGenAltasSuccess);
			m_list.erase(it++);
		}
		else
		{
			++it;
		}
	}
}

bool SwfManager::genAtlasImage()
{
	if (m_pPlayer)
	{
		if (m_pPlayer->m_wirteBitmapFunc)
		{
			m_bGenAltasSuccess = m_sAtlas.genAtlasImageByMem();
		}
		else
		{
			m_bGenAltasSuccess = m_sAtlas.genAtalsImage();
		}
	}

	return true;
}

void SwfManager::checkToGenData()
{
	auto it = m_sFiles.begin();
	if (it != m_sFiles.end())
	{
		m_sCurKey = it->first;
		parseSwf(it->first);
	}
}

void SwfManager::genAniData(std::string sFile, GenDataCallback func)
{
	CallBackData sItem;
	sItem.sName = sFile;
	sItem.cb = func;
	m_list.push_back(sItem);
	m_sFiles[sFile] = func;
}

std::string SwfManager::getFileName(std::string& sPath)
{
	static char sSlash = '/';
	VxConvert::replace_str(sPath, "\\", "/");
	int index = sPath.rfind(sSlash, sPath.length() - 2);
	auto dotIndex = sPath.rfind(".");
	auto sFileName = sPath.substr(index + 1, dotIndex - index - 1);
	//auto sFileName = sPath.substr(index + 1);
	return sFileName;
}

bool SwfManager::parseSwf(std::string sFile)
{
	m_srcPath = FileUtils::getInstance()->fullPathForFilename(sFile);
	VxConvert::replace_str(m_srcPath, "\\", "/");
	updatePath();

	if (m_pPlayer)
	{
		delete m_pPlayer;
	}
	m_sAtlas.initPath(m_desPath);
	m_pPlayer = new gameswf::player();
#if 0
	m_pPlayer->setWriteBitmapFunc([&](unsigned char*pData, std::string sFileName, int w, int h, bool bNeedFree)
	{
		m_sAtlas.addBitmapData(pData, sFileName, w, h, bNeedFree);
		m_bHadCreateBitmap = true;
	});
#endif
	m_pPlayer->load_file((m_desPath).c_str(), m_srcPath.c_str());
	setParseState(SWF_PARSING);
	return true;
}

void SwfManager::updatePath()
{
	static char sChars[] = { '/' };

	int index0 = m_srcPath.rfind(sChars[0]);
	int index = index0;
	if (index0 == std::string::npos )
	{
		index = 0;
	}
	

	auto dotIndex = m_srcPath.rfind(".");
	auto sFileName = m_srcPath.substr(index+1, dotIndex - index-1);
	//m_desPath = m_srcPath.substr(0, dotIndex) + "/";// VxConvert::getObliqueLine();
	//m_desPath = cocos2d::FileUtils::getInstance()->getWritablePath()+"swf/"+ m_sFileName + "/";
	m_desPath  = m_sCachePath+ sFileName + "/";
	VxConvert::replace_str(m_desPath, "\\", "/");
	CCLOG("updatePath %s", m_desPath.c_str());

}

void SwfManager::setParseState(int nStatus)
{
	m_nParsingState = nStatus;
}

void SwfManager::threadProc()
{
	while (!m_bThreadEnd)
	{
		if (m_nParsingState == SWF_GEN_IMAGE_START)
		{
			genAtlasImage();
			m_sAtlas.clear();
			setParseState(SWF_GEN_IMAGE_END);
		}
#if CC_PLATFORM_WIN32 == CC_TARGET_PLATFORM
		Sleep(50);
#else
		usleep(1000*50);
#endif

	};
	
}

void* SwfManager::_threadProc(void* arg)
{
	SwfManager* client = (SwfManager*)arg;
	client->threadProc();
	return NULL;
}
