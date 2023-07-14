//==============================================================
//==============================================================
//= geomipmapping.h ============================================
//= Original coders: Trent Polack (trent@voxelsoft.com)		   =
//==============================================================
//= This file (along with geomipmapping.cpp) contains all of   =
//= the information for the geomipmapping terrain component.   =
//==============================================================
//==============================================================
#ifndef __GEOMIPMAPPING_H__
#define __GEOMIPMAPPING_H__


//--------------------------------------------------------------
//--------------------------------------------------------------
//- HEADERS AND LIBRARIES --------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
#include "terrain.h"

#include "../Base Code/camera.h"


//--------------------------------------------------------------
//--------------------------------------------------------------
//- DATA STRUCTURES --------------------------------------------
// 
//一个 patch数据结构 相当一个块 
//--------------------------------------------------------------
//--------------------------------------------------------------
struct SGEOMM_PATCH
{
	float m_fDistance;//patch的中心位置到相机位置的距离

	int  m_iLOD;//当前LOD级别  0最清晰（顶点最多）   4最模糊（顶点最少）
};

//相邻的方位
struct SGEOMM_NEIGHBOR
{
	bool m_bLeft;//左
	bool m_bUp;//下
	bool m_bRight;//右
	bool m_bDown;//上
};


//--------------------------------------------------------------
//--------------------------------------------------------------
//- CLASS ------------------------------------------------------
// 
// geomipmapping 基于 2^n + 1方形高度图，继续用断层高度图算法，生成高度图数据
//--------------------------------------------------------------
//--------------------------------------------------------------
class CGEOMIPMAPPING : public CTERRAIN
{
	private:
		SGEOMM_PATCH* m_pPatches;//patch的数目，15 * 15 个patch
		int			  m_iPatchSize;//动态堆上分配 patch一般是 17 * 17 ，这里就是17个顶点
		int			  m_iNumPatchesPerSide;//256 + 1 高度图257 / 16 + 1 patch的尺寸17 =  每一行15个patch ，256 + 1 的高度图，就是15 * 15 个patch

		int	m_iMaxLOD;//最大LOD级数 ，这里是4 ，0,1,2,3  ,注意书上讲解的 level越高LOD（例如4），表示细节越低（顶点越少）

		int m_iPatchesPerFrame;	//每帧渲染的patch数量 the number of rendered patches per second

	void RenderFan( float cX, float cZ, float iSize, SGEOMM_NEIGHBOR neighbor, bool bMultiTex, bool bDetail );
	void RenderPatch( int PX, int PZ, bool bMultiTex= false, bool bDetail= false );

	//--------------------------------------------------------------
	// Name:			CGEOMIPMAPPING::RenderVertex - private
	// Description:		Render a vertex, mostly used for saving space (in the code)
	// Arguments:		- x, z: vertex to render
	//					- u, v: texture coordinates
	//					- bMultiTex: use multitexturing or not
	// Return Value:	None
	// 
	// 用内联函数
	// 主要用于节省空间
	//--------------------------------------------------------------
	inline void RenderVertex( float x, float z, float u, float v, bool bMultiTex )
	{
		//光照贴图信息
		unsigned char ucColor= GetBrightnessAtPoint( ( int )x, ( int )z );

		//计算顶点光照
		//send the shaded color to the rendering API
		glColor3ub( ( unsigned char )( ucColor*m_vecLightColor[0] ),
				    ( unsigned char )( ucColor*m_vecLightColor[1] ),
				    ( unsigned char )( ucColor*m_vecLightColor[2] ) );

		//贴图纹理坐标绑定
		//send the texture coordinates to the rendering API	
		glMultiTexCoord2fARB( GL_TEXTURE0_ARB, u, v );
		if( bMultiTex )
			glMultiTexCoord2fARB( GL_TEXTURE1_ARB, u*m_iRepeatDetailMap, v*m_iRepeatDetailMap );

		//需将缩放后的顶点坐标发送到渲染 API
		//output the vertex to the rendering API
		glVertex3f( x*m_vecScale[0],
					GetScaledHeightAtPoint( ( int )x, ( int )z ),
					z*m_vecScale[2] );

		//increase the number of vertices rendered
		m_iVertsPerFrame++;
	}

	public:

	bool Init( int iPatchSize );
	void Shutdown( void );
	
	void Update( CCAMERA camera );
	void Render( void );

	//--------------------------------------------------------------
	// Name:			CGEOMIPMAPPING::GetNumPatchesPerFrame - public
	// Description:		Get the number of patches being rendered per frame
	// Arguments:		None
	// Return Value:	A integer value: number of rendered patches per frame
	//--------------------------------------------------------------
	inline int GetNumPatchesPerFrame( void )
	{	return m_iPatchesPerFrame;	}

	//--------------------------------------------------------------
	// patch数组中当前x,z下的patch下标
	// Name:			CTERRAIN::GetPatchNumber - public
	// Description:		Calculate the current patch number
	// Arguments:		-PX, PZ: the patch number
	// Return Value:	An integer value: the patch number (for array access)
	//--------------------------------------------------------------
	inline int GetPatchNumber( int PX, int PZ )
	{	return ( ( PZ*m_iNumPatchesPerSide )+PX );	}

	CGEOMIPMAPPING( void )
	{	}
	~CGEOMIPMAPPING( void )
	{	}
};

#endif	//__GEOMIPMAPPING_H__
