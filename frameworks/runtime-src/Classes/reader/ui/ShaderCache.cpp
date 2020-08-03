
#include "ShaderCache.h"
NS_CCR_BEGIN

ShaderCache::ShaderCache()
{
	m_sGLProgramStates[SHADER_DEFAULT_ID]  = GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP);
	m_sGLProgramStates[SHADER_GRAY_ID] = createGLProgramStateWithFile("shader/gray.vsh","shader/gray.fsh");

	m_sGLProgramStates[SHADER_SPRITE_LIGHT_ID] = createGLProgramStateWithFile("shader/spriteLight.vsh", "shader/spriteLight.fsh");
}

ShaderCache::~ShaderCache()
{

}

GLProgramState* ShaderCache::getGLProgramState(int nId)
{
	auto it = m_sGLProgramStates.find(nId);
	if(it != m_sGLProgramStates.end())
	{
		return it->second;
	}
	return NULL;
}

GLProgramState* ShaderCache::createGLProgramState(const std::string &sVerSrc,const std::string &sFlagSrc)
{
	auto glprogram = GLProgram::createWithByteArrays(sVerSrc.c_str(),sFlagSrc.c_str());
	auto glprogramstate = GLProgramState::getOrCreateWithGLProgram(glprogram);
	
	return glprogramstate;
}

GLProgramState* ShaderCache::createGLProgramStateWithFile(const std::string sVerFile,const std::string sFlagFile)
{
	auto glprogram = GLProgram::createWithFilenames(sVerFile,sFlagFile);
	auto glprogramstate = GLProgramState::getOrCreateWithGLProgram(glprogram);
	return glprogramstate;
}

NS_CCR_END