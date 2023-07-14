//==============================================================
//==============================================================
//= geomipmapping.cpp ==========================================
//= Original coders: Trent Polack (trent@voxelsoft.com)		   =
//==============================================================
//= This file (along with geomipmapping.cpp) contains all of   =
//= the information for the geomipmapping terrain component.   =
//==============================================================
//==============================================================


/*
书上说的anti-cracking  指的是抗裂性，处理块之间拼接时候裂痕情况


也不一定
 ?? 这种以patch的方式来渲染，只能适用于 高度图数据 和顶点数一致的情况
例如 256 高度图 是 15 * 15 个patch， 每个patch 是 17 * 17 个顶点  ??
*/


//--------------------------------------------------------------
//--------------------------------------------------------------
//- HEADERS AND LIBRARIES --------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
#include <stdio.h>

#include "../Base Code/gl_app.h"

#include "geomipmapping.h"


//--------------------------------------------------------------
//--------------------------------------------------------------
//- DEFINITIONS ------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------

//--------------------------------------------------------------
// Name:			CGEOMIPMAPPING::Init - public
// Description:		Initiate the geomipmapping system
// Arguments:		- iPatchSize: the size of the patch (in vertices)
//								  a good size is usually around 17 (17x17 verts)
// Return Value:	A boolean value: -true: successful initiation
//									 -false: unsuccessful initiation
//--------------------------------------------------------------
bool CGEOMIPMAPPING::Init( int iPatchSize )
{
	int x, z;
	int iLOD;
	int iDivisor;

	if( m_iSize==0 )
		return false;

	if( m_pPatches )
		Shutdown( );

	//initiate the patch information
	m_iPatchSize= iPatchSize;//17
	m_iNumPatchesPerSide= m_iSize/m_iPatchSize;//256 + 1 高度图257 / 16 + 1 patch的尺寸17 =  每一行15个patch ，256 + 1 的高度图，就是15 * 15 个patch
	m_pPatches= new SGEOMM_PATCH [SQR( m_iNumPatchesPerSide )];//SQR 宏  m_iNumPatchesPerSide * m_iNumPatchesPerSide  15 * 15 个
	if( m_pPatches==NULL )
	{
		Shutdown( );

		g_log.Write( LOG_SUCCESS, "Could not allocate memory for geomipmapping patch system" );
		return false;
	}

	//figure out the maximum level of detail for a patch
	iDivisor= m_iPatchSize-1;
	iLOD= 0;
	while( iDivisor>2 )
	{
		iDivisor= iDivisor>>1;//不断下降，通过能链接三角形的情况，决定能有多少级LOD 17 * 17  就是 16,8,4,2 ，patch 17 * 17 一行有16个三角形
		iLOD++;
	}

	//the max amount of detail
	m_iMaxLOD= iLOD;

	//initialize the patch values
	for( z=0; z<m_iNumPatchesPerSide; z++ )
	{
		for( x=0; x<m_iNumPatchesPerSide; x++ )
		{
			//initialize the patches to the lowest level of detail
			m_pPatches[GetPatchNumber( x, z )].m_iLOD= m_iMaxLOD;//遍历初始化设置每一个patch的最大 LOD，访问方式也是x,z*m_iNumPatchesPerSide跟高度图一样，数组
		}
	}

	g_log.Write( LOG_SUCCESS, "Geomipmapping system successfully initialized" );
	return true;
}

//--------------------------------------------------------------
// Name:			CGEOMIPMAPPING::Shutdown - public
// Description:		Shutdown the geomipmapping system
// Arguments:		None
// Return Value:	None
//--------------------------------------------------------------
void CGEOMIPMAPPING::Shutdown( void )
{
	//delete the patch buffer
	if( m_pPatches )
		delete[] m_pPatches;

	//reset patch values
	m_iPatchSize= 0;
	m_iNumPatchesPerSide= 0;

	m_iMaxLOD= 0;
}

//--------------------------------------------------------------
// update 更新各个patch的LOD 的level值
// Name:			CGEOMIPMAPPING::Update - public
// Description:		Update the geomipmapping system
// Arguments:		-camera: the camera object your demo is using
// Return Value:	None
//--------------------------------------------------------------
void CGEOMIPMAPPING::Update( CCAMERA camera )
{
	float fX, fY, fZ;
	float fScaledSize;
	int x, z;
	int iPatch;

	fScaledSize= m_iPatchSize*m_vecScale[0];

	for( z=0; z<m_iNumPatchesPerSide; z++ )
	{
		for( x=0; x<m_iNumPatchesPerSide; x++ )
		{
			iPatch= GetPatchNumber( x, z );

			//compute patch center (used for distance determination
			fX= ( x*m_iPatchSize )+( m_iPatchSize/2.0f );//计算patch中心点x,z  x*m_iPatchSize 是第x个patch * 17顶点，就是这个patch的第一个顶点，+上 17/2 就是中心
			fZ= ( z*m_iPatchSize )+( m_iPatchSize/2.0f );
			fY= GetScaledHeightAtPoint( ( int )fX, ( int )fZ );

			//only scale the X and Z values, the Y value has already been scaled
			fX*= m_vecScale[0];//用上缩放值来计算实际距离
			fZ*= m_vecScale[2];

			//get the distance from the camera to the patch
			//update中每帧更新每个patch距离视野的距离 x^+y^+z^  开方
			m_pPatches[iPatch].m_fDistance= sqrtf( SQR( ( fX-camera.m_vecEyePos[0] ) )+
												   SQR( ( fY-camera.m_vecEyePos[1] ) )+
												   SQR( ( fZ-camera.m_vecEyePos[2] ) ) );

			/*
			这里是硬编码判定距离，决定LOD级别
			更多的判定方式是 根据屏幕像素确定算法 来确定patch的LOD
			过多的LOD切换会导致popping ---- 在3D 计算机图形学中，弹出是指当 3D 对象转换到不同的预先计算的细节级别(LOD) 时出现的不受欢迎的视觉效果，对于观看者来说是突然且明显的
			算法控制不好，会导致频繁切换，影响画面效果
			*/
			//BAD way to determine patch LOD
			if( m_pPatches[iPatch].m_fDistance<500 )
				m_pPatches[iPatch].m_iLOD= 0;
		
			else if( m_pPatches[iPatch].m_fDistance<1000 )
				m_pPatches[iPatch].m_iLOD= 1;

			else if( m_pPatches[iPatch].m_fDistance<2500 )
				m_pPatches[iPatch].m_iLOD= 2;

			else if( m_pPatches[iPatch].m_fDistance>=2500 )
				m_pPatches[iPatch].m_iLOD= 3;
		}
	}
}

//--------------------------------------------------------------
// Name:			CGEOMIPMAPPING::Render - public
// Description:		Render the geomipmapping system
// Arguments:		None
// Return Value:	None
//--------------------------------------------------------------
void CGEOMIPMAPPING::Render( void )
{
	int	x, z;

	//reset the counting variables
	m_iPatchesPerFrame = 0;
	
	m_iVertsPerFrame= 0;
	m_iTrisPerFrame = 0;

	//enable back-face culling
	glEnable( GL_CULL_FACE );

	//render the multitexturing terrain
	if( m_bMultitexture && m_bDetailMapping && m_bTextureMapping )
	{
		glDisable( GL_BLEND );


		//没用shader的情况下，通过图形api，指定两张贴图着色
		//bind the primary color texture to the first texture unit
		glActiveTextureARB( GL_TEXTURE0_ARB );
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, m_texture.GetID( ) );

		//bind the detail color texture to the second texture unit
		glActiveTextureARB( GL_TEXTURE1_ARB );
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, m_detailMap.GetID( ) );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB );
		glTexEnvi( GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 2 );

		//render the patches
		for( z=0; z<m_iNumPatchesPerSide; z++ )
		{
			for( x=0; x<m_iNumPatchesPerSide; x++ )
			{
				RenderPatch( x, z, true, true );//遍历各个patch，开始渲染patch
				m_iPatchesPerFrame++;
			}
		}
	}
	
	//no hardware multitexturing available, or the user only wants to render
	//the detail texture or the color texture
	else
	{
		if( m_bTextureMapping )
		{
			//bind the primary color texture (FOR THE PRIMARY TEXTURE PASS)
			glActiveTextureARB( GL_TEXTURE0_ARB );
			glEnable( GL_TEXTURE_2D );
			glBindTexture( GL_TEXTURE_2D, m_texture.GetID( ) );

			//render the color texture
			for( z=0; z<m_iNumPatchesPerSide; z++ )
			{
				for( x=0; x<m_iNumPatchesPerSide; x++ )
				{
					RenderPatch( x, z, true, true );
					m_iPatchesPerFrame++;
				}
			}
		}

		if( !( m_bTextureMapping && !m_bDetailMapping ) )
		{
			//if the user wants detail mapping, we need to set some things up
			if( m_bDetailMapping )
			{
				//bind the detail texture
				glActiveTextureARB( GL_TEXTURE0_ARB );
				glEnable( GL_TEXTURE_2D );
				glBindTexture( GL_TEXTURE_2D, m_detailMap.GetID( ) );
			
				//only use blending if a texture pass was made
				if( m_bTextureMapping )
				{
					glEnable( GL_BLEND );
					glBlendFunc( GL_ZERO, GL_SRC_COLOR );
				}
			}

			//render either the detail map on top of the texture,
			//only the detail map, or neither
			for( z=0; z<m_iNumPatchesPerSide; z++ )
			{
				for( x=0; x<m_iNumPatchesPerSide; x++ )
				{
					RenderPatch( x, z, true, true );
					m_iPatchesPerFrame++;
				}
			}
		}
	}

	glDisable( GL_BLEND );

	//unbind the texture occupying the second texture unit
	glActiveTextureARB( GL_TEXTURE1_ARB );
	glDisable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, 0 );

	//unbind the texture occupying the first texture unit
	glActiveTextureARB( GL_TEXTURE0_ARB );
	glDisable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, 0 );
}

//--------------------------------------------------------------
// Name:			CGEOMIPMAPPING::RenderPatch - private
// Description:		Render a patch of terrain
// Arguments:		-PX, PZ: the patch location
//					-bMultitex: use multitexturing or not
//					-bDetail: render with a detail map or not
// Return Value:	None
// PX, PZ 遍历到的当前patch的x，y
// 计算要渲染的fan的数量，fan的中心点
//--------------------------------------------------------------
void CGEOMIPMAPPING::RenderPatch( int PX, int PZ, bool bMultiTex, bool bDetail )
{

	/*
	这里是渲染一个patch，一个patch里面的逐个fan来计算渲染，距离，然后fan里面再执行需要的 vertexs，连接三角形

	*/
	SGEOMM_NEIGHBOR patchNeighbor;//这个patch的邻居
	SGEOMM_NEIGHBOR fanNeighbor;//这个fan的邻居fan
	float fSize;
	float fHalfSize;
	float x, z;
	int iPatch= GetPatchNumber( PX, PZ );
	int iDivisor;
	int iLOD;

	//find out information about the patch to the current patch's left, if the patch is of a
	//greater detail or there is no patch to the left, we can render the mid-left vertex
	//了解当前patch左侧的patch信息，如果左侧的patch的更详细或左侧没有patch，我们可以渲染左中顶点
	// 注意，如果摄像机观察点，在地图中间，就会有左边的lod 分布 和 右边的lod分布 相反
	//			2
	//			1
	//			0 
	// 3 2 1 0 eys 0 1 2 3    
	// 3 2 1 0	0
	//			1
	//			2
	// 下面的判定就是包含了两边情况
	//找左边  继续找四个方向
	if( m_pPatches[GetPatchNumber( PX-1, PZ )].m_iLOD<=m_pPatches[iPatch].m_iLOD || PX==0 )
		patchNeighbor.m_bLeft= true;//表示左边有一个清晰度高的patch，它需要减顶点来避免裂缝，我自己就不用，可以渲染左边线的中间点(the mid-left vertex)
	else
		patchNeighbor.m_bLeft= false;

	//find out about the upper patch //下面的patch的lod ，比当前渲染的patch的lod 小  或者  当前是最后一个patch
	if( m_pPatches[GetPatchNumber( PX, PZ+1 )].m_iLOD<=m_pPatches[iPatch].m_iLOD || PZ==m_iNumPatchesPerSide )
		patchNeighbor.m_bUp= true;//表示下面有一个清晰度高的patch，可以渲染下边线的中间点
	else
		patchNeighbor.m_bUp= false;

	//find out about the right patch
	if( m_pPatches[GetPatchNumber( PX+1, PZ )].m_iLOD<=m_pPatches[iPatch].m_iLOD || PX==m_iNumPatchesPerSide )
		patchNeighbor.m_bRight= true;
	else
		patchNeighbor.m_bRight= false;

	//find out about the lower patch
	if( m_pPatches[GetPatchNumber( PX, PZ-1 )].m_iLOD<=m_pPatches[iPatch].m_iLOD || PZ==0 )
		patchNeighbor.m_bDown= true;
	else
		patchNeighbor.m_bDown= false;

	//we need to determine the distance between each triangle-fan that
	//we will be rendering
	//决定每一个fan之间的距离  其实要算出隔了多少个三角形
	fSize   = ( float )m_iPatchSize;//17
	iDivisor= m_iPatchSize-1;//16
	iLOD    = m_pPatches[iPatch].m_iLOD;

	//find out how many fan divisions we are going to have
	//找出还有多少扇形分区
	/*
	这里需要iLOD-- 先用再减，目的是可以进入多一次，LOD 0 == 0  时候，继续进入 
	
	*/

	while( iLOD-->=0 )
		iDivisor= iDivisor>>1;//iDivisor用int，可以用移位计算，效率会高，计算会更快

	//如果0 就退出循环，就会出现 iLOD =0 时，iDivisor = 16，fSize = iDivisor 的情况，fSize/= iDivisor相除就是 fSize = 1
	// 如果LOD 0 的两个patch之间，是要间隔2个单位，不能是1，不然会重叠，看看书上的图就知道
	// 
	// 因为下面是执行渲染fan，就是连接扇形模式的三角形，所以是计算三角形之间的距离，扇形的中心点之间距离。
	/*
		5    4----3 ----4----3
			 |	/ |		|	 |
		6    0----2 ----0----2
				\ |		|	 |
		7    8    1	----8----1 

		5    4----3 ----4----3
			 |	/ |		|	 |
		6    0----2 ----0----2
				\ |		|	 |
		7    8    1	----8----1
		计算0 与 隔壁 的0 的距离，就是这个fan的中心点距离  
		LOD 0 就是清晰度最高的level时候， 每一个fan都会渲染，就是足够多的顶点，17 * 17 顶点都用上
		这时候两两之间的距离就是2，距离少，因为fan多
		LOD 越大 ，level 3，需要的fan越少，两两之间的距离就会越大
	*/
	 
	// 
	//the size between the center of each triangle fan
	fSize/= iDivisor;//level 3  --> iDivisor = 1    fSize = 17  fHalfSize = 8

	//half the size between the center of each triangle fan (this will be
	//the size between each vertex)
	fHalfSize= fSize/2.0f; //这个就是两两fan的中心点之间的单位距离
	for( z=fHalfSize; ( ( int )z+fHalfSize )<m_iPatchSize+1; z+=fSize )
	{
		for( x=fHalfSize; ( ( int )x+fHalfSize )<m_iPatchSize+1; x+=fSize )
		{
			//if this fan is in the left row, we may need to adjust it's rendering to
			//prevent cracks
			//如果这个fan在左，我们可能需要调整它的渲染以防止出现裂缝
			if( x==fHalfSize )//第一列
				fanNeighbor.m_bLeft= patchNeighbor.m_bLeft;
			else
				fanNeighbor.m_bLeft= true;//不是第一个，后面的fan，左边都会有fan，所以是true

			//if this fan is in the bottom row, we may need to adjust it's rendering to
			//prevent cracks
			if( z==fHalfSize )//第一行，
				fanNeighbor.m_bDown= patchNeighbor.m_bDown;//上边线的情况，所以要跟patch的领居一样情况
			else
				fanNeighbor.m_bDown= true;

			//if this fan is in the right row, we may need to adjust it's rendering to
			//prevent cracks
			if( x>=( m_iPatchSize-fHalfSize ) )
				fanNeighbor.m_bRight= patchNeighbor.m_bRight;
			else
				fanNeighbor.m_bRight= true;

			//if this fan is in the top row, we may need to adjust it's rendering to
			//prevent cracks
			if( z>=( m_iPatchSize-fHalfSize ) )
				fanNeighbor.m_bUp= patchNeighbor.m_bUp;//最后一行，下边线的情况
			else
				fanNeighbor.m_bUp= true;

			//PX*m_iPatchSize 是过了多少个patch，多少个顶点的单位距离，+x 就是当前fan的中心点单位
			//render the triangle fan
			RenderFan( ( PX*m_iPatchSize )+x, ( PZ*m_iPatchSize )+z,
					   fSize, fanNeighbor, bMultiTex, bDetail );
		}
	}
}

//--------------------------------------------------------------
// Name:			CGEOMIPMAPPING::RenderFan - private
// Description:		Update the geomipmapping system
// Arguments:		-cX, cZ: center of the triangle fan to render
//					-fSize: half of the fan's entire size
//					-neightbor: the fan's neighbor structure (used to avoid cracking)
//					-bMultitex: use multitexturing or not
//					-bDetail: render with a detail map or not
// Return Value:	None
// 根据fan的中心点，开始连接三角形
//--------------------------------------------------------------
void CGEOMIPMAPPING::RenderFan( float cX, float cZ, float fSize, SGEOMM_NEIGHBOR neighbor, bool bMultiTex, bool bDetail )
{
	float fTexLeft, fTexBottom, fMidX, fMidZ, fTexRight, fTexTop;
	float fHalfSize= fSize/2.0f;

	//计算UV
	//calculate the texture coordinates if we're not doing multitexturing, but still detail mapping
	if( bDetail && !bMultiTex )
	{
		//calculate the texture coordinates
		fTexLeft  = ( ( float )fabs( cX-fHalfSize )/m_iSize )*m_iRepeatDetailMap;
		fTexBottom= ( ( float )fabs( cZ-fHalfSize )/m_iSize )*m_iRepeatDetailMap;
		fTexRight = ( ( float )fabs( cX+fHalfSize )/m_iSize )*m_iRepeatDetailMap;
		fTexTop	  = ( ( float )fabs( cZ+fHalfSize )/m_iSize )*m_iRepeatDetailMap;

		fMidX= ( ( fTexLeft+fTexRight )/2 );
		fMidZ= ( ( fTexBottom+fTexTop )/2 );
	}

	//calculate the texture coordinates otherwise
	else
	{
		//calculate the texture coordinates
		fTexLeft  = ( ( float )fabs( cX-fHalfSize )/m_iSize );
		fTexBottom= ( ( float )fabs( cZ-fHalfSize )/m_iSize );
		fTexRight = ( ( float )fabs( cX+fHalfSize )/m_iSize );
		fTexTop	  = ( ( float )fabs( cZ+fHalfSize )/m_iSize );

		fMidX= ( ( fTexLeft+fTexRight )/2 );
		fMidZ= ( ( fTexBottom+fTexTop )/2 );
	}


	//begin a new triangle strip
	//GL_TRIANGLE_FAN绘制各三角形形成一个扇形序列，以v0为起始点，（v0，v1，v2）、（v0，v2，v3）、（v0，v3，v4）。
	/*
	* 
	以0为中心  每个patch的链接顶点方式
	4---3
    | / |
	0---2
	  \ |
	    1
	
	7    8----1
		 |	/ |
	6    0----2
			\ |
	5    4    3
	*/
	glBegin( GL_TRIANGLE_FAN );
		//render the CENTER vertex  fan 0 中心点 
		RenderVertex( cX, cZ, fMidX, fMidZ, bMultiTex );

		//render the LOWER-LEFT vertex 上图 左下角 3 这个点
		RenderVertex( cX-fHalfSize, cZ-fHalfSize, fTexLeft, fTexBottom, bMultiTex );		

		//only render the next vertex if the left patch is NOT of a lower LOD
		if( neighbor.m_bLeft )//2 可以画多一个中间的顶点，因为左边是高清晰度的fan，它会去掉一个中间的，来避免裂缝
		{
			RenderVertex( cX-fHalfSize, cZ, fTexLeft, fMidZ, bMultiTex );
			m_iTrisPerFrame++;
		}
	
		//render the UPPER-LEFT vertex 左上角
		RenderVertex( cX-fHalfSize, cZ+fHalfSize, fTexLeft, fTexTop, bMultiTex );
		m_iTrisPerFrame++;

		//only render the next vertex if the upper patch is NOT of a lower LOD
		if( neighbor.m_bUp )
		{
			RenderVertex( cX, cZ+fHalfSize, fMidX, fTexTop, bMultiTex );
			m_iTrisPerFrame++;
		}

		//render the UPPER-RIGHT vertex 右上角
		RenderVertex( cX+fHalfSize, cZ+fHalfSize, fTexRight, fTexTop, bMultiTex );
		m_iTrisPerFrame++;

		//only render the next vertex if the right patch is NOT of a lower LOD
		if( neighbor.m_bRight )
		{
			//render the MID-RIGHT vertex
			RenderVertex( cX+fHalfSize, cZ, fTexRight, fMidZ, bMultiTex );
			m_iTrisPerFrame++;
		}

		//render the LOWER-RIGHT vertex 右下角
		RenderVertex( cX+fHalfSize, cZ-fHalfSize, fTexRight, fTexBottom, bMultiTex );
		m_iTrisPerFrame++;	

		//only render the next vertex if the bottom patch is NOT of a lower LOD
		if( neighbor.m_bDown )
		{
			//render the LOWER-MID vertex
			RenderVertex( cX, cZ-fHalfSize, fMidX, fTexBottom, bMultiTex );	
			m_iTrisPerFrame++;
		}

		//render the LOWER-LEFT vertex 左下角
		RenderVertex( cX-fHalfSize, cZ-fHalfSize, fTexLeft, fTexBottom, bMultiTex );
		m_iTrisPerFrame++;	

	//end the triangle strip
	glEnd( );
}