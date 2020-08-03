#pragma once

#include "cocos2d.h"
#include "Macros.h"


using namespace cocos2d;
NS_CCR_BEGIN

enum 
{
	SHADER_DEFAULT_ID,
	SHADER_GRAY_ID ,
	SHADER_WATER_WAVE_ID,
	SHADER_WATER_ID,
	SHADER_SPINE_GRAY_ID,
	SHADER_SPRITE_LIGHT_ID,
	SHADER_PLAYER_LIGHT,
	SHADER_DES_LIGHT,
	SHADER_HIGH_LIGHT_ID,
	SHADER_HIGH_LIGHT2_ID,
	SHADER_MONSTER_LIGHT_ID,
	SHADER_CIRCLE_LIGHT_ID,
	
};


class ShaderCache 
{
public:


	cocos2d::GLProgramState* getGLProgramState(int nId);
	cocos2d::GLProgramState* createGLProgramState(const std::string &sVerSrc,const std::string &sFlagSrc);
	cocos2d::GLProgramState* createGLProgramStateWithFile(const std::string sVerFile,const std::string sFlagFile);

public:
	ShaderCache();
	~ShaderCache();

	std::unordered_map<int, cocos2d::GLProgramState*> m_sGLProgramStates;

};



NS_CCR_END