//
//  CCSWFNode.m
//  GSGGameSWF
//
//  Created by dario on 13-02-25.
//  Modify by chenee (chenee543216@gmail.com) on 13-05-20
//  @chenee: 1\change Objective-C to C++ ;2\change opengl es 1.1 to opengl es 2.0
//

#include "CCSWFNode.h"
#include "CCGameSWF.h"
#include "cocos2d.h"
#include "gameswf.h"
#include "gameswf_player.h"
#include "gameswf_root.h"
//#include "ccMacros.h"
#include "gameswf_types.h"
#include "gameswf_impl.h"

using namespace cocos2d;

//@interface CCSWFNode_touchContainer : NSObject
//{
//    CGPoint m_position;
//    int m_state;
//}
//
//@property (readonly) CGPoint position;
//@property (readonly) int state;
//
//+(id) touchContainerWithPosition:(CGPoint)position andState:(int)state;
//-(id) initWithPosition:(CGPoint)position andState:(int)state;
//
//@end
class CCSWFNode_touchContainer : public cocos2d::CCNode
{
    CCPoint m_position;
    int m_state;
    CCSWFNode_touchContainer(CCPoint position,int state);
//    bool touchContainerWithPosition(CCPoint position, int state);

};


//@implementation CCSWFNode_touchContainer

//@synthesize position = m_position;
//@synthesize state = m_state;
CCSWFNode_touchContainer::CCSWFNode_touchContainer(CCPoint position, int state)
{
    m_position = position;
    m_state = state;
}
//bool CCSWFNode_touchContainer::touchContainerWithPosition(CCPoint position, int state)
//{
////    return [[CCSWFNode_touchContainer alloc] initWithPosition:position andState:state];
//    return true;
//}
//
//void CCSWFNode_touchContainer::initWithPositionAndState(CCPoint position, int state)
//{
////    self = [super init];
////    if (self)
////    {
//        m_position = position;
//        m_state = state;
////    }
////    return self;
//}
//
//@end



//@interface CCSWFNode_imp : NSObject
//{
//@public
//    gameswf::gc_ptr<gameswf::player> m_player;
//    gameswf::gc_ptr<gameswf::root>	m_movie;
//}
//
//-(id) initWithSWFFile:(NSString*)file;
//
//@end
class CCSWFNode_imp : public cocos2d::CCNode
{
public:
    gameswf::gc_ptr<gameswf::player> m_player;
    gameswf::gc_ptr<gameswf::root>	m_movie;
    
    bool initWithSWFFile(const char *file);
//    void dealloc();

};

//@implementation CCSWFNode_imp

bool CCSWFNode_imp::initWithSWFFile(const char *file)
{
    std::string pathKey = file;
    
    pathKey = CCFileUtils::sharedFileUtils()->fullPathForFilename(pathKey.c_str());
    if (pathKey.size() == 0)
    {
        return false;
    }
//    self = [super init];
//    if (self)
//    {
        // make sure CCGameSWF is initialized //
//        [CCGameSWF sharedInstance];
        CCGameSWF::sharedInstance();
        m_player = new gameswf::player();
        m_movie = m_player->load_file(pathKey.c_str(), pathKey.c_str());
        if (m_movie == NULL)
        {
            printf("ERROR: Cannot open input file %s", file);
//            [self release];
            return false;
        }
//    }
//	
//	return self;
    return true;
}

//void CCSWFNode_imp::dealloc()
//{
//    delete m_movie;
//    delete m_player;
////    [super dealloc];
//}

//@end



//@implementation CCSWFNode
CCSWFNode::CCSWFNode()
{
//    CCLOG("CCSWFNode init ....");
}
CCSWFNode::~CCSWFNode()
{
    this->stopAction();
    this->removeFromParentAndCleanup(true);
    this->imp->m_movie = NULL;
    this->imp->m_player = NULL;
    
    delete imp;
//    CCLOG("CCSWFNode delete ....");
}

CCString* CCSWFNode::movieName()
{
//    return [NSString stringWithUTF8String:imp->m_movie->m_movie->m_name.c_str()];
    return CCString::createWithFormat(imp->m_movie->m_movie->m_name.c_str());
}

void CCSWFNode::setMovieName(char *movieName)
{
    imp->m_movie->m_movie->m_name = movieName;// UTF8String];
}

//+(id) nodeWithSWFFile:(NSString*)file
//{
//    return [[[CCSWFNode alloc] initWithSWFFile:file] autorelease];
//}
CCSWFNode* CCSWFNode::create(const char* file)
{
    CCSWFNode* swf = new CCSWFNode();
    if (! swf->initWithSWFFile(file)) {
        swf->release();
        return NULL;
    }
    
    return swf;
}

bool CCSWFNode::initWithSWFFile(const char* file)
{
//    self = [super init];
//    if (self)
//    {
//        imp = [[CCSWFNode_imp alloc] initWithSWFFile:file];
    imp = new CCSWFNode_imp();
    if (!imp->initWithSWFFile(file))
    {
//            [self release];
        imp->release();
        return false;
    }
	m_movieWidth = (imp->m_movie->m_def->m_frame_size.m_x_max - imp->m_movie->m_def->m_frame_size.m_x_min)/20.0 ;
	m_movieHeight = (imp->m_movie->m_def->m_frame_size.m_y_max - imp->m_movie->m_def->m_frame_size.m_y_min)/20.0 ;
    m_localScaleX = (imp->m_movie->get_movie_width() / m_movieWidth);
    m_localScaleY = -(imp->m_movie->get_movie_height() / m_movieHeight);
    m_scaleX = 1.0;
    m_scaleY = 1.0;
    
   // m_touchEvents = new CCArray();//[[NSMutableArray alloc] init];
    
    this->setContentSize(CCSize(m_movieWidth, m_movieHeight));
    this->setScale(1.0f);
    this->setAnchorPoint(ccp(0.5,0.5));
    
    this->isGrouped = false;
    this->isRuning = false;
    this->m_listener = NULL;
    this->m_endListener = NULL;
    this->repeat = false;
    this->iFrameCount = imp->m_movie->get_frame_count();

//        [self setContentSize:CGSizeMake(m_movieWidth, m_movieHeight)];
//        [self setScale:1.0];
//        [self setAnchorPoint:ccp(0.5f, 0.5f)];
//    }
    return true;
}
void CCSWFNode::setFlipX(bool flipX)
{
//    [super setFlipX:flipX];
    if (flipX) {
//        m_localScaleX *= -1;
//        m_scaleX = m_scaleX*-1;
//        [super setScaleX:m_localScaleX*-1];
        setScaleX(m_localScaleX * -1);
        
    }
}
void CCSWFNode::setFlipY(bool flipY)
{
    if (flipY) {
        setScaleY(m_localScaleY * -1);
    }
}
float CCSWFNode::scale()
{
    CCAssert(m_scaleX == m_scaleY, "CCNode#scale. ScaleX != ScaleY. Don't know which one to return");
	return m_scaleX;
}

void CCSWFNode::setScale(float scale)
{
    m_scaleX = m_scaleY = scale;
//    [super setScaleX:m_localScaleX * m_scaleX];
//    [super setScaleY:m_localScaleY * m_scaleY];
    setScaleX(m_localScaleX*m_scaleX);
    setScaleY(m_localScaleY*m_scaleY);
}

float CCSWFNode::scaleX()
{
    return m_scaleX;
}

void CCSWFNode::setScaleX(float scaleX)
{
    m_scaleX = scaleX;
//    [super setScaleX:m_localScaleX * m_scaleX];
//    setScaleX(m_localScaleX*m_scaleX);
    cocos2d::CCSprite::setScaleX(m_scaleX);
}

float CCSWFNode::scaleY()
{
    return m_scaleY;
}

void CCSWFNode::setScaleY(float scaleY)
{
    m_scaleY = scaleY;
//    [super setScaleY:m_localScaleY];
//    setScaleY(m_localScaleY);
    CCSprite::setScaleY(m_scaleY);
    
}

//void CCSWFNode::dealloc()
//{
////    m_touchEvents->release();
////    [m_touchEvents release];
////    [super dealloc];
//    this->stopAction();
////    this->imp->dealloc();
//    delete imp;
//}

void CCSWFNode::onEnterTransitionDidFinish()
{
    
//    [self scheduleUpdate];
//    if(isRuning)
//        return;
//    
//    this->isRuning = true;
////    this->imp->m_movie->goto_frame(48);
////    this->imp->m_movie->set_play_state((gameswf::character::play_state)1);
////    this->imp->m_movie->set_play_run();
//    schedule(schedule_selector(CCSWFNode::update),1/2);
    
//    schedule(schedule_selector(CCSWFNode::update));


//    [[CCTouchDispatcher sharedDispatcher] addTargetedDelegate:self priority:1 swallowsTouches:YES];
}

void CCSWFNode::onExit()
{
//    [self unscheduleAllSelectors];
    if(! isRuning)
        return;
    
    unschedule(schedule_selector(CCSWFNode::update));
    this->isRuning = false;
//    [[CCTouchDispatcher sharedDispatcher] removeDelegate:self];
}
void CCSWFNode::runAction()
{
    if(isRuning)
        return;
    
    //if in singleton group we stop other sibling.
    if (isGrouped) {
       CCSWFNodeGroup* g = (CCSWFNodeGroup*)this->getParent();
        if (g->getSingleton()) {
            g->stopAction();
        }
    }
    
    this->isRuning = true;
    //reset 
    this->imp->m_movie->goto_frame(0);
    this->imp->m_movie->set_play_run();
    
    schedule(schedule_selector(CCSWFNode::update),1/60);
}
void CCSWFNode::stopAction()
{
    if(! isRuning)
        return;
    unschedule(schedule_selector(CCSWFNode::update));
    this->isRuning = false;
}

void CCSWFNode::addFrameListener(int iFrame, CCObject* target,SEL_MenuHandler selector)
{
    CCAssert(target != NULL, "swf action should not be NULL");

    if(iFrame == -1){
        m_endListener = target;
        m_pfnEndSelector = selector;
    }else if(iFrame > 0){
        m_listener = target;
        m_pfnSelector = selector;
        iListenFrame = iFrame;
    }
}
void CCSWFNode::setNextAction(CCSWFNode* target)
{
    CCAssert(target != NULL, "next swf action should not be NULL");
    
    m_endListener = target;
   // m_pfnEndSelector = menu_selector(CCSWFNode::runAction);
}
void CCSWFNode::setRepeat(bool flag)
{
    this->repeat = flag;
}
bool CCSWFNode::getRepeat()
{
    return repeat;
}


void CCSWFNode::update(float dt)
{

	bool tobestop = false;
	int iFrame = imp->m_movie->get_current_frame();
	if (m_listener && iFrame == iListenFrame) {
		(m_listener->*m_pfnSelector)(this);
	}
	if (!repeat) {
		if (iFrame == iFrameCount - 1) {
			tobestop = true;
		}
	}
	imp->m_movie->advance(dt);

	if (tobestop) {
		this->stopAction();
		if (m_endListener) {
			//(this->m_endListener->*m_pfnEndSelector)(this);
		}
	}
}

void CCSWFNode::onDraw(const Mat4& transform, bool /*transformUpdated*/)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


#if CC_ENABLE_GL_STATE_CACHE
	GL::useProgram(0);//valid program is NON_ZERO unsigned int
#endif

	auto sAnchorPoint = this->getAnchorPoint();
	auto sSize = this->getContentSize();
	auto wp = this->getParent()->convertToNodeSpace(this->getPosition());

	//auto &s = Director::getInstance()->getWinSize();
	float dx = sSize.width*sAnchorPoint.x;
	float dy = sSize.height*sAnchorPoint.y;

	float m[16] = { 0,0,0,wp.x - dx, 0,0,0,wp.y+sSize.height - dy, 0,0,1,0, 0,0,0,1 };
	//imp->m_movie->display(&matrixMVP.m[0]);
	auto pre = utils::getTimeInMilliseconds();
	imp->m_movie->display(&m[0]);
	auto cur = utils::getTimeInMilliseconds();
	CCLOG("%d", cur - pre);
	CHECK_GL_ERROR_DEBUG();

}

void CCSWFNode::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	if (!isRuning) {
		return;
	}
	bool transformUpdated = flags & FLAGS_TRANSFORM_DIRTY;
	_customCommand.init(_globalZOrder, transform, flags);
	_customCommand.func = CC_CALLBACK_0(CCSWFNode::onDraw, this, transform, transformUpdated);

	renderer->addCommand(&_customCommand);


}

//@end
