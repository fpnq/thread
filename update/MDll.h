//------------------------------------------------------------------------------------------------------------------------------
//单元名称：动态连接库单元
//单元描述：主要处理动态连接库的装载、映射等
//创建日期：2007.3.15
//创建人员：卢明远
//修改纪录
//修改日期		修改人员	修改内容和原因
//
//
//
//
//
//------------------------------------------------------------------------------------------------------------------------------
#ifndef __MEngine_MDllH__
#define __MEngine_MDllH__
//------------------------------------------------------------------------------------------------------------------------------
#include "MErrorCode.h"
//------------------------------------------------------------------------------------------------------------------------------
class MDll
{
protected:
	#ifndef LINUXCODE

		HINSTANCE					m_hDll;

	#else

		void					*	m_lpDll;
		char						m_szDllSelfPath[MAX_PATH];

	#endif
public:
	MDll(void);
	virtual ~MDll();
public:
	//装载动态连接库
	// modify by yuanjj for dllmain call 2014-03-12
	int  LoadDll(MString strFileName, void *hModule = NULL, BOOL bdllmain = TRUE );
	// modify end
	//获取函数指针
	void * GetDllFunction(MString strFunctionName);
	//卸摘动态连接库
	void CloseDll(void);
	void * GetDllHand();
#ifdef LINUXCODE
	char	*GetDllSelfPath();
private:
	void	MergeDllSelfPath(const char *, void *);
#endif
};
//------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------
