//==============================================================
//==============================================================
//= quadtree.cpp ===============================================
//= Original coders: Trent Polack (trent@voxelsoft.com)		   =
//==============================================================
//= This file (along with quadtree.cpp) contains all of the    =
//= information for the quadtree terrain component.            =
//==============================================================
//==============================================================


//--------------------------------------------------------------
//--------------------------------------------------------------
//- HEADERS AND LIBRARIES --------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
#include <stdio.h>

#include "../Base Code/gl_app.h"

#include "quadtree.h"
#include <iostream>

using namespace std;

//--------------------------------------------------------------
//--------------------------------------------------------------
//- GLOBALS ----------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
//fan iFanCode table
char g_cQTFanCode[] = { 10, 8, 8, 12, 8, 0, 12, 14, 8, 12, 0, 14, 12, 14, 14, 0 };
char g_cQTFanStart[]= { 3,  3, 0,  3, 1, 0,  0,  3, 2,  2, 0,  2,  1,  1,  0, 0 };


//--------------------------------------------------------------
//--------------------------------------------------------------
//- DEFINITIONS ------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------

//--------------------------------------------------------------
// Name:			CQUADTREE::Init - public
// Description:		Initialize the quadtree engine
// Arguments:		None
// Return Value:	A boolean value: -true: successful init
//									 -false: unsuccessful init
//--------------------------------------------------------------
bool CQUADTREE::Init( void )
{
	int x, z;

	//create memory for the quadtree matrix
	m_ucpQuadMtrx= new unsigned char [SQR( m_iSize )];
 	if( m_ucpQuadMtrx==NULL )
	{
		g_log.Write( LOG_FAILURE, "Could not initialize memory for the quadtree matrix" );
		return false;
	}

	//initialize the quadtree matrix to 'true' (for all possible nodes)
	for( z=0; z<m_iSize; z++ )
	{
		for( x=0; x<m_iSize; x++ )
		{
			m_ucpQuadMtrx[GetMatrixIndex(x, z)] = 1;// true;//默认都是1
		}
	}

	PropagateRoughness();


	//for (z = 0; z < m_iSize; z++) {
	//	char buffer[5];
	//	char result[5 * 513];
	//	memset(result, 0, sizeof(result));
	//	memset(buffer, 0, 5);
	//	char* num_string;
	//	for (x = 0; x < m_iSize; x++) {
	//		int c = 255;// m_ucpQuadMtrx[GetMatrixIndex(x, z)];
	//		
	//		//char temp[5];
	//		//memset(temp, 0, sizeof(temp));
	//		//int ret = snprintf(buffer, sizeof(buffer), "%d", c);
	//		buffer[0] = '2';
	//		buffer[1] = '5';
	//		buffer[2] = '5';
	//		num_string = buffer;
	//		//strcpy(temp, num_string);
	//		//strcat(temp, "|");

	//		strcat(result, num_string);
	//		//g_log.Write(LOG_PLAINTEXT, num_string);
	//	}
	//	//
	//	// 
	//	g_log.Write(LOG_PLAINTEXT, result);
	//}
	//for (z = 0; z < m_iSize; z++) {
	//	char buffer[5];
	//	char result[5 * 513];
	//	memset(buffer, 0, sizeof(buffer));
	//	memset(result, 0, sizeof(result));
	//	for (x = 0; x < m_iSize; x++) {
	//		int c = m_ucpQuadMtrx[GetMatrixIndex(x, z)];

	//		int ret = snprintf(buffer, sizeof(buffer), "%d", c);

	//		strcat(result, buffer);
	//		strcat(result, "|");
	//		//g_log.Write(LOG_PLAINTEXT, num_string);
	//	}
	//	//
	//	// 
	//	g_log.Write(LOG_PLAINTEXT, result);
	//}


	//intialization was a success
	g_log.Write( LOG_SUCCESS, "The quadtree terrain engine has been successfully initialized" );
	return true;
}

//--------------------------------------------------------------
// Name:			CQUADTREE::Shutdown - public
// Description:		Shutdown the quadtree engine
// Arguments:		None
// Return Value:	None
//--------------------------------------------------------------
void CQUADTREE::Shutdown( void )
{
	//free the memory stored in the quadtree matrix
	if( m_ucpQuadMtrx )
		delete[] m_ucpQuadMtrx;
}

//--------------------------------------------------------------
// Name:			CQUADTREE::Update - public
// Description:		Update the quadtree engine
// Arguments:		-camera: a camera object
// Return Value:	None
//--------------------------------------------------------------
void CQUADTREE::Update( CCAMERA camera )
{
	float fCenter;

	//calculate the center of the terrain mesh
	fCenter= ( m_iSize-1 )/2.0f;

	//build the mesh through top-down quadtree traversal
	RefineNode( fCenter, fCenter, m_iSize, camera );
}

//--------------------------------------------------------------
// Name:			CQUADTREE::Render - public
// Description:		Render the quadtree engine
// Arguments:		None
// Return Value:	None
//--------------------------------------------------------------
void CQUADTREE::Render( void )
{
	float fCenter;

	//reset the counting variables
	m_iVertsPerFrame= 0;
	m_iTrisPerFrame = 0;

	//calculate the center of the mesh
	fCenter= ( m_iSize-1 )/2.0f;

	//enable back-face culling
	glDisable( GL_CULL_FACE );

	//use hardware multitexturing for the texture map and the detail map
	if( m_bMultitexture && m_bDetailMapping && m_bTextureMapping )
	{
		glDisable( GL_BLEND );

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

		//render the main node (which will recurse down to the other nodes)
		RenderNode( fCenter, fCenter, m_iSize, true, true );
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

			//render the main node (which will recurse down to the other nodes)
			RenderNode( fCenter, fCenter, m_iSize, false, false );
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
			//render the main node (which will recurse down to the other nodes)
			RenderNode( fCenter, fCenter, m_iSize, false, m_bDetailMapping );
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



/*
			8	           1               2
						   |
			               |              
						   |
				UL Node*4  |   UR Node*8
			7	-----------9-----------    3
						   |
			               |              
						   |
				LL Node*2  |   LR Node*1                          靠近高度图数据和四叉树矩阵数组的 开头0的index元素，就是Lower，就是下面说的bottom
			6	           5               4                       相当于是从左下角开始形成一个地形的意思

高度图分布     下面算法取值，跟上图的顶点对应
xz


03   13    23
---------------------------------
02   12    22     upper-mid     |
01   11    21                   |
00   10    20     bottom-mid    |
*/


//--------------------------------------------------------------
// 处理四叉树bytes数据
// 传播高度贴图的粗糙度（这样更多的三角形将应用于贴图的粗糙区域）
// Name:			CQUADTREE::PropagateRoughness - private
// Description:		Propagate the roughness of the height map (so more 
//					triangles will get applied to rougher areas of the map)
// Arguments:		None
// Return Value:	None
//--------------------------------------------------------------
void CQUADTREE::PropagateRoughness(void)
{
	float fKUpperBound;
	int iDH, iD2, iLocalD2, iLocalH;
	int iEdgeLength, iEdgeOffset;
	int iChildOffset;
	int x, z;

	//set the iEdgeLength to 3 (lowest length possible)
	iEdgeLength = 3;

	//遍历直到最高节点
	// 自下而上遍历所有四叉树的节点，从最低级别开始 逐步向上
	//start off at the lowest level of detail, and traverse up to the highest node (lowest detail)
	while (iEdgeLength <= m_iSize)
	{
		//offset of node edges (since all edges are the same length
		iEdgeOffset = (iEdgeLength - 1) >> 1;//除2  =====>   3-1 = 2  /  2 =  1

		//offset of the node's children's edges
		iChildOffset = (iEdgeLength - 1) >> 2;//除4

		for (z = iEdgeOffset; z < m_iSize; z += (iEdgeLength - 1))
		{
			for (x = iEdgeOffset; x < m_iSize; x += (iEdgeLength - 1))
			{
				//compute "iLocalD2" values for this node
				//upper-mid
				iLocalD2 = (int)ceil(abs(((GetTrueHeightAtPoint(x - iEdgeOffset, z + iEdgeOffset) +
					GetTrueHeightAtPoint(x + iEdgeOffset, z + iEdgeOffset)) >> 1) -
					GetTrueHeightAtPoint(x, z + iEdgeOffset)));

				//right-mid
				iDH = (int)ceil(abs(((GetTrueHeightAtPoint(x + iEdgeOffset, z + iEdgeOffset) +
					GetTrueHeightAtPoint(x + iEdgeOffset, z - iEdgeOffset)) >> 1) -
					GetTrueHeightAtPoint(x + iEdgeOffset, z)));
				iLocalD2 = MAX(iLocalD2, iDH);

				//bottom-mid
				iDH = (int)ceil(abs(((GetTrueHeightAtPoint(x - iEdgeOffset, z - iEdgeOffset) +
					GetTrueHeightAtPoint(x + iEdgeOffset, z - iEdgeOffset)) >> 1) -
					GetTrueHeightAtPoint(x, z - iEdgeOffset)));
				iLocalD2 = MAX(iLocalD2, iDH);

				//left-mid
				iDH = (int)ceil(abs(((GetTrueHeightAtPoint(x - iEdgeOffset, z + iEdgeOffset) +
					GetTrueHeightAtPoint(x - iEdgeOffset, z - iEdgeOffset)) >> 1) -
					GetTrueHeightAtPoint(x - iEdgeOffset, z)));
				iLocalD2 = MAX(iLocalD2, iDH);

				//bottom-left to top-right diagonal
				iDH = (int)ceil(abs(((GetTrueHeightAtPoint(x - iEdgeOffset, z - iEdgeOffset) +
					GetTrueHeightAtPoint(x + iEdgeOffset, z + iEdgeOffset)) >> 1) -
					GetTrueHeightAtPoint(x, z)));
				iLocalD2 = MAX(iLocalD2, iDH);

				//bottom-right to top-left diagonal
				iDH = (int)ceil(abs(((GetTrueHeightAtPoint(x + iEdgeOffset, z - iEdgeOffset) +
					GetTrueHeightAtPoint(x - iEdgeOffset, z + iEdgeOffset)) >> 1) -
					GetTrueHeightAtPoint(x, z)));
				iLocalD2 = MAX(iLocalD2, iDH);

				//if (iLocalD2 > 0) {
				//	char buffer[5];
				//	int ret = snprintf(buffer, sizeof(buffer), "%d", iLocalD2);
				//	char* num_string = buffer;
				//	g_log.Write(LOG_PLAINTEXT, num_string);
				//}
				// 
				

				//make iLocalD2 a value between 0-255
				iLocalD2 = (int)ceil((iLocalD2 * 3.0f) / iEdgeLength);
				
				//test minimally sized block
				if (iEdgeLength == 3)
				{
					iD2 = iLocalD2;

					//compute the "iLocalH" value
					//upper right
					iLocalH = GetTrueHeightAtPoint(x + iEdgeOffset, z + iEdgeOffset);

					//right mid
					iLocalH = MAX(iLocalH, GetTrueHeightAtPoint(x + iEdgeOffset, z));

					//lower right
					iLocalH = MAX(iLocalH, GetTrueHeightAtPoint(x + iEdgeOffset, z - iEdgeOffset));

					//bottom mid
					iLocalH = MAX(iLocalH, GetTrueHeightAtPoint(x, z - iEdgeOffset));

					//lower left
					iLocalH = MAX(iLocalH, GetTrueHeightAtPoint(x - iEdgeOffset, z - iEdgeOffset));

					//left mid
					iLocalH = MAX(iLocalH, GetTrueHeightAtPoint(x - iEdgeOffset, z));

					//upper left
					iLocalH = MAX(iLocalH, GetTrueHeightAtPoint(x - iEdgeOffset, z + iEdgeOffset));

					//upper mid
					iLocalH = MAX(iLocalH, GetTrueHeightAtPoint(x, z + iEdgeOffset));

					//center
					iLocalH = MAX(iLocalH, GetTrueHeightAtPoint(x, z));

					//store the maximum iLocalH value in the matrix
					m_ucpQuadMtrx[GetMatrixIndex(x + 1, z)] = iLocalH;
				}

				else
				{
					fKUpperBound = 1.0f * m_fMinResolution / (2.0f * (m_fMinResolution - 1.0f));

					//use d2 values from farther up on the quadtree
					iD2 = (int)ceil(MAX(fKUpperBound * (float)GetQuadMatrixData(x, z), (float)iLocalD2));
					iD2 = (int)ceil(MAX(fKUpperBound * (float)GetQuadMatrixData(x - iEdgeOffset, z), (float)iD2));
					iD2 = (int)ceil(MAX(fKUpperBound * (float)GetQuadMatrixData(x + iEdgeOffset, z), (float)iD2));
					iD2 = (int)ceil(MAX(fKUpperBound * (float)GetQuadMatrixData(x, z + iEdgeOffset), (float)iD2));
					iD2 = (int)ceil(MAX(fKUpperBound * (float)GetQuadMatrixData(x, z - iEdgeOffset), (float)iD2));

					//get the max local height values of the 4 nodes (LL, LR, UL, UR)
					iLocalH = GetTrueHeightAtPoint(x + iChildOffset, z + iChildOffset);
					iLocalH = MAX(iLocalH, GetTrueHeightAtPoint(x + iChildOffset, z - iChildOffset));
					iLocalH = MAX(iLocalH, GetTrueHeightAtPoint(x - iChildOffset, z - iChildOffset));
					iLocalH = MAX(iLocalH, GetTrueHeightAtPoint(x - iChildOffset, z + iChildOffset));

					//store the max value in the quadtree matrix
					m_ucpQuadMtrx[GetMatrixIndex(x + 1, z)] = iLocalH;// 21
				}

				//store the values we calculated for iD2 into the quadtree matrix
				m_ucpQuadMtrx[GetMatrixIndex(x, z)] = iD2;//11
				m_ucpQuadMtrx[GetMatrixIndex(x - 1, z)] = iD2;//01

				//propogate the value up the quadtree
				/*
				02       22
				
				00       20
				*/
				m_ucpQuadMtrx[GetMatrixIndex(x - iEdgeOffset, z - iEdgeOffset)] = MAX(GetQuadMatrixData(x - iEdgeOffset, z - iEdgeOffset), iD2);//00
				m_ucpQuadMtrx[GetMatrixIndex(x - iEdgeOffset, z + iEdgeOffset)] = MAX(GetQuadMatrixData(x - iEdgeOffset, z + iEdgeOffset), iD2);//02
				m_ucpQuadMtrx[GetMatrixIndex(x + iEdgeOffset, z + iEdgeOffset)] = MAX(GetQuadMatrixData(x + iEdgeOffset, z + iEdgeOffset), iD2);//22
				m_ucpQuadMtrx[GetMatrixIndex(x + iEdgeOffset, z - iEdgeOffset)] = MAX(GetQuadMatrixData(x + iEdgeOffset, z - iEdgeOffset), iD2);//20
			}
		}

		//move up to the next quadtree level (lower level of detail)
		/*
		* 其实是为了达到在给上面 iEdgeLength - 1
		3 =>  2
		5     4
		9     8
		17    16
		33    32
		64    64
		*/
		iEdgeLength = (iEdgeLength << 1) - 1;
	}
}

//--------------------------------------------------------------
// Name:			CQUADTREE::RefineNode - private
// 改进精炼四叉树节点
// Description:		Refine a quadtree node (update the quadtree matrix)
// Arguments:		-x, z: center of current node
//					-iEdgeLength: length of the current node's edge
//					-camera: camera object (for calculating distance)
// Return Value:	None
//--------------------------------------------------------------
void CQUADTREE::RefineNode( float x, float z, int iEdgeLength, CCAMERA camera )
{
	float fViewDistance, f;
	float fChildOffset;
	int iX, iZ;
	int iChildEdgeLength;
	//bool bSubdivide;//是否可以再细分
	int iBlend;

	iX= ( int )x;
	iZ= ( int )z;

	//开始调用递归前，第一次传入iEdgeLength 是地形最大值，比如 513
	// x z  开始是中心点  ，513/2 
	//实际上这里是根据摄像机与地面中心的距离，来评估四叉树要细分到的程度
	//每次update来计算整棵树的节点情况，细节开放到多少，从而控制lod的级别
	//离相机越远，就不需要再细分，只停留几个节点，细节就不会多，顶点也不会再多，就达到lod的级别了


	//计算节点中心和摄像机的距离 书上 L1 - Norm 的这个公式
	//calculate the distance from the current point (L1 NORM, which, essentially, is a faster version of the 
	//normal distance equation you may be used to... yet again, thanks to Chris Cookson)
	fViewDistance= ( float )( fabs( camera.m_vecEyePos[0]-( x*m_vecScale[0] ) )+
							  fabs( camera.m_vecEyePos[1]-GetQuadMatrixData( iX+1, iZ ) )+
							  fabs( camera.m_vecEyePos[2]-( z*m_vecScale[2] ) ) );


	//通过粗糙度计算后，令这里获取的GetQuadMatrixData的四叉树矩阵值，计算这个距离，可以得到更多细分的细节
	//compute the 'f' value (as stated in Roettger's whitepaper of this algorithm)
	//这个f值用于判定当前节点是否需要 继续细分子节点
	f= fViewDistance/( ( float )iEdgeLength*m_fMinResolution*
					   MAX( m_fDesiredResolution*GetQuadMatrixData( iX-1, iZ )/3, 1.0f ) );
	

	//1 则可以对当前节点细分，否则它是一个叶子节点

	//通过粗糙度处理后，令到这个f计算更多时候能 <1.0，这样就能分出更多的节点，细节就更多
	if (f < 1.0f)
		//bSubdivide= true;	
		iBlend = 255;
	else
		//bSubdivide= false;
		iBlend = 0;

	//store whether or not the current node gets subdivided
	m_ucpQuadMtrx[GetMatrixIndex(iX, iZ)] = iBlend;// bSubdivide;//0 or 1

	if( iBlend )//非0
	{
		//else, we need to recurse down farther into the quadtree
		//这里判定3，其实就是表示 2 * 2的高度图，因为定义size时候是 2 + 1 = 3  256 + 1 = 257 
		//只有2 * 2的地形高度图时候，其实就只有一个节点的四叉树，没必要再细分了
		//另外在最后一个叶子结点，到下面的>> 1 ----> 除以2后， 计算的值就是小于3的
		if( !( iEdgeLength<=3 ) )
		{
			fChildOffset    = ( float )( ( iEdgeLength-1 ) >> 2 );//除以4   ---- 按位移效率高
			iChildEdgeLength= ( iEdgeLength+1 ) >> 1;//除以2

			//refine the various child nodes
			//lower left
			RefineNode( x-fChildOffset, z-fChildOffset, iChildEdgeLength, camera );

			//lower right
			RefineNode( x+fChildOffset, z-fChildOffset, iChildEdgeLength, camera );

			//upper left
			RefineNode( x-fChildOffset, z+fChildOffset, iChildEdgeLength, camera );

			//upper right
			RefineNode( x+fChildOffset, z+fChildOffset, iChildEdgeLength, camera );
		}
	} 
}

//--------------------------------------------------------------
// Name:			CQUADTREE::RenderNode - private
// Description:		Render leaf (no children) quadtree nodes
// Arguments:		-x, z: center of current node
//					-iEdgeLength: length of the current node's edge
//					-bMultiTex: use multitexturing for rendering
//					-bDetail: use a detail map when rendering
// Return Value:	None
//--------------------------------------------------------------
void CQUADTREE::RenderNode( float x, float z, int iEdgeLength, bool bMultiTex, bool bDetail )
{
	//开始调用递归前，第一次传入iEdgeLength 是地形最大值，比如 513
	// x z  开始是中心点  ，513/2 


	float fTexLeft, fTexBottom, fMidX, fMidZ, fTexRight, fTexTop;
	float fChildOffset;
	float fEdgeOffset;
	int iStart, iFanCode;
	int iChildEdgeLength;
	int iChildOffset;
	int iEdgeOffset;
	int iAdjOffset;
	int iFanLength;
	int iFanPosition;
	int bSubdivide;
	int iX, iZ;

	iX= ( int )x;
	iZ= ( int )z;

	//compute the edge offset of the current node
	iEdgeOffset= ( iEdgeLength-1 )/2;
	fEdgeOffset= ( iEdgeLength-1 )/2.0f;

	//compute the offset to the nodes near the current node
	iAdjOffset= iEdgeLength-1;

	if( bDetail && !bMultiTex )
	{
		//calculate the texture coordinates
		fTexLeft  = ( ( float )fabs( x-fEdgeOffset )/m_iSize )*m_iRepeatDetailMap;
		fTexBottom= ( ( float )fabs( z-fEdgeOffset )/m_iSize )*m_iRepeatDetailMap;
		fTexRight = ( ( float )fabs( x+fEdgeOffset )/m_iSize )*m_iRepeatDetailMap;
		fTexTop	  = ( ( float )fabs( z+fEdgeOffset )/m_iSize )*m_iRepeatDetailMap;

		fMidX= ( ( fTexLeft+fTexRight )/2.0f );
		fMidZ= ( ( fTexBottom+fTexTop )/2.0f );
	}

	else
	{
		//calculate the texture coordinates
		fTexLeft  = ( ( float )fabs( x-fEdgeOffset )/m_iSize );
		fTexBottom= ( ( float )fabs( z-fEdgeOffset )/m_iSize );
		fTexRight = ( ( float )fabs( x+fEdgeOffset )/m_iSize );
		fTexTop	  = ( ( float )fabs( z+fEdgeOffset )/m_iSize );

		fMidX= ( ( fTexLeft+fTexRight )/2.0f );
		fMidZ= ( ( fTexBottom+fTexTop )/2.0f );
	}

	//get the blend factor from the current quadtree value
	bSubdivide= GetQuadMatrixData( iX, iZ );


	if( bSubdivide > 0 )
	{
		//顶点很少，只有3个，直接渲染顶点
		//is this the smallest node?
		if( iEdgeLength<=3 )
		{
			//render a triangle fan to represent the node
			glBegin( GL_TRIANGLE_FAN );

				//center vertex
				RenderVertex( x, z, fMidX, fMidZ, bMultiTex );

				//lower left vertex    --> LL
				RenderVertex( x-fEdgeOffset, z-fEdgeOffset, fTexLeft, fTexBottom, bMultiTex );

				//lower mid, skip if the adjacent node is of a lower detail level
				if( ( ( iZ-iAdjOffset )<0 ) || GetQuadMatrixData( iX, iZ-iAdjOffset )!=0 )
				{
					RenderVertex( x, z-fEdgeOffset, fMidX, fTexBottom, bMultiTex );
					m_iTrisPerFrame++;
				}

				//bottom right vertex   -->  LR
				RenderVertex( x+fEdgeOffset, z-fEdgeOffset, fTexRight, fTexBottom, bMultiTex );
				m_iTrisPerFrame++;

				//right mid, skip if the adjacent node is of a lower detail level
				if( ( ( iX+iAdjOffset )>=m_iSize ) || GetQuadMatrixData( iX+iAdjOffset, iZ )!=0 )
				{
					RenderVertex( x+fEdgeOffset, z, fTexRight, fMidZ, bMultiTex );
					m_iTrisPerFrame++;
				}

				//upper right vertex
				RenderVertex( x+fEdgeOffset, z+fEdgeOffset, fTexRight, fTexTop, bMultiTex );
				m_iTrisPerFrame++;

				//upper mid, skip if the adjacent node is of a lower detail level
				if( ( ( iZ+iAdjOffset )>=m_iSize ) || GetQuadMatrixData( iX, iZ+iAdjOffset )!=0 )
				{
					RenderVertex( x, z+fEdgeOffset, fMidX, fTexTop, bMultiTex );
					m_iTrisPerFrame++;
				}

				//upper left vertex
				RenderVertex( x-fEdgeOffset, z+fEdgeOffset, fTexLeft, fTexTop, bMultiTex );
				m_iTrisPerFrame++;

				//left mid, skip if the adjacent node is of a lower detail level
				if( ( ( iX-iAdjOffset )<0 ) || GetQuadMatrixData( iX-iAdjOffset, iZ )!=0 )
				{
					RenderVertex( x-fEdgeOffset, z, fTexLeft, fMidZ, bMultiTex );
					m_iTrisPerFrame++;
				}

				//bottom left vertex again   ---->  lower left vertex    --> LL
				RenderVertex( x-fEdgeOffset, z-fEdgeOffset, fTexLeft, fTexBottom, bMultiTex );
				m_iTrisPerFrame++;
			glEnd( );
			return;

		}
		
		else
		{
			//calculate the child node's offset values
			iChildOffset= ( iEdgeLength-1 )/4;
			fChildOffset= ( float )iChildOffset;

			//calculate the edge length of the child nodes
			iChildEdgeLength= ( iEdgeLength+1 )/2;

			/*      
				    1               2
						   |
			8              |              3
						   |
				UL Node*4  |   UR Node*8
				-----------9-----------
						   |
			7              |              4
						   |
				LL Node*2  |   LR Node*1
				    6               5
			*/
			//粗糙度的算法可以令这里拿到不为0的值，可以增加这个面的，令到细节增加

			//位运算  可以加快速度  相当于 +=  四个子节点，检查需要渲染的节点情况
			//calculate the bit-iFanCode for the fan arrangement (which fans need to be rendered)
			//upper right
			iFanCode = ( GetQuadMatrixData( iX+iChildOffset, iZ+iChildOffset )!=0 )*8;//8

			//upper left
			iFanCode|= ( GetQuadMatrixData( iX-iChildOffset, iZ+iChildOffset )!=0 )*4;//4

			//lower left
			iFanCode|= ( GetQuadMatrixData( iX-iChildOffset, iZ-iChildOffset )!=0 )*2;//2

			//lower right
			iFanCode|= ( GetQuadMatrixData( iX+iChildOffset, iZ-iChildOffset )!=0 );//1

			//now, use the previously calculate codes, and render some tri-fans :)
			//this node has four children, no rendering is needed (for this node at least), but
			//we need to recurse down to this node's children
			if( iFanCode==QT_NO_FAN )//有四个叶子节点
			{
				//lower left
				RenderNode( x-fChildOffset, z-fChildOffset, iChildEdgeLength );

				//lower right
				RenderNode( x+fChildOffset, z-fChildOffset, iChildEdgeLength );

				//upper left
				RenderNode( x-fChildOffset, z+fChildOffset, iChildEdgeLength );

				//upper right
				RenderNode( x+fChildOffset, z+fChildOffset, iChildEdgeLength );
				return;
			}

			//render the lower left and upper right fans
			if( iFanCode==QT_LL_UR )//5 ----> UL + LR 刚好是5
			{
				//最后一级LOD，没有再细分的四叉树叶子节点，所以直接连接4个顶点，两个三角形，形成一个面
				//the upper right fan
				glBegin( GL_TRIANGLE_FAN );
					//center vertex
					RenderVertex( x, z, fMidX, fMidZ, bMultiTex );

					//right mid vertex
					RenderVertex( x+fEdgeOffset, z, fTexRight, fMidZ, bMultiTex );

					//upper right vertex
					RenderVertex( x+fEdgeOffset, z+fEdgeOffset, fTexRight, fTexTop, bMultiTex );
					m_iTrisPerFrame++;

					//upper mid vertex
					RenderVertex( x, z+fEdgeOffset, fMidX, fTexTop, bMultiTex );
					m_iTrisPerFrame++;
				glEnd( );

				//lower left fan  同上
				glBegin( GL_TRIANGLE_FAN );
					//center vertex
					RenderVertex( x, z, fMidX, fMidZ, bMultiTex );

					//left mid
					RenderVertex( x-fEdgeOffset, z, fTexLeft, fMidZ, bMultiTex );

					//bottom left
					RenderVertex( x-fEdgeOffset, z-fEdgeOffset, fTexLeft, fTexBottom, bMultiTex );
					m_iTrisPerFrame++;

					//bottom mid
					RenderVertex( x, z-fEdgeOffset, fMidX, fTexBottom, bMultiTex );
					m_iTrisPerFrame++;
				glEnd( );

				//继续递归，有叶子节点，还有LOD级别
				//recurse further down to the upper left and lower right nodes
				RenderNode( x-fChildOffset, z+fChildOffset, iChildEdgeLength );
				RenderNode( x+fChildOffset, z-fChildOffset, iChildEdgeLength );
				return;

			}

			//render the lower-right and upper-left triangles fans
			if( iFanCode==QT_LR_UL )//另一边  也是同上
			{
				//upper left fan
				glBegin( GL_TRIANGLE_FAN );
					//center vertex
					RenderVertex( x, z, fMidX, fMidZ, bMultiTex );

					//upper mid vertex
					RenderVertex( x, z+fEdgeOffset, fMidX, fTexTop, bMultiTex );

					//upper left vertex
					RenderVertex( x-fEdgeOffset, z+fEdgeOffset, fTexLeft, fTexTop, bMultiTex );
					m_iTrisPerFrame++;

					//left mid vertex
					RenderVertex( x-fEdgeOffset, z, fTexLeft, fMidZ, bMultiTex );
					m_iTrisPerFrame++;
				glEnd( );

				//lower right fan
				glBegin( GL_TRIANGLE_FAN );
					//center vertex
					RenderVertex( x, z, fMidX, fMidZ, bMultiTex );

					//lower mid vertex
					RenderVertex( x, z-fEdgeOffset, fMidX, fTexBottom, bMultiTex );

					//lower right vertex
					RenderVertex( x+fEdgeOffset, z-fEdgeOffset, fTexRight, fTexBottom, bMultiTex );
					m_iTrisPerFrame++;

					//right mid vertex
					RenderVertex( x+fEdgeOffset, z, fTexRight, fMidZ, bMultiTex );
					m_iTrisPerFrame++;
				glEnd( );

				//recurse further down to the upper right and lower left nodes
				RenderNode( x+fChildOffset, z+fChildOffset, iChildEdgeLength );
				RenderNode( x-fChildOffset, z-fChildOffset, iChildEdgeLength );
				return;
			}

			//最后叶子节点，直接渲染顶点
			//this node is a leaf-node, render a complete fan
			if( iFanCode==QT_COMPLETE_FAN )
			{
				glBegin( GL_TRIANGLE_FAN );
					//center vertex
					RenderVertex( x, z, fMidX, fMidZ, bMultiTex );

					//render the lower left vertex
					RenderVertex( x-fEdgeOffset, z-fEdgeOffset, fTexLeft, fTexBottom, bMultiTex );

					//lower mid, skip if the adjacent node is of a lower detail level
					if( ( ( iZ-iAdjOffset )<0 ) || GetQuadMatrixData( iX, iZ-iAdjOffset )!=0 )
					{
						RenderVertex( x, z-fEdgeOffset, fMidX, fTexBottom, bMultiTex );
						m_iTrisPerFrame++;
					}

					//lower right vertex
					RenderVertex( x+fEdgeOffset, z-fEdgeOffset, fTexRight, fTexBottom, bMultiTex );
					m_iTrisPerFrame++;

					//right mid, skip if the adjacent node is of a lower detail level
					if( ( ( iX+iAdjOffset )>=m_iSize ) || GetQuadMatrixData( iX+iAdjOffset, iZ )!=0 )
					{
						RenderVertex( x+fEdgeOffset, z, fTexRight, fMidZ, bMultiTex );
						m_iTrisPerFrame++;
					}

					//upper right vertex
					RenderVertex( x+fEdgeOffset, z+fEdgeOffset, fTexRight, fTexTop, bMultiTex );
					m_iTrisPerFrame++;

					//upper mid, skip if the adjacent node is of a lower detail level
					if( ( ( iZ+iAdjOffset )>=m_iSize ) || GetQuadMatrixData( iX, iZ+iAdjOffset )!=0 )
					{
						RenderVertex( x, z+fEdgeOffset, fMidX, fTexTop, bMultiTex );
						m_iTrisPerFrame++;
					}

					//upper left
					RenderVertex( x-fEdgeOffset, z+fEdgeOffset, fTexLeft, fTexTop, bMultiTex );
					m_iTrisPerFrame++;

					//left mid, skip if the adjacent node is of a lower detail level
					if( ( ( iX-iAdjOffset )<0 ) || GetQuadMatrixData( iX-iAdjOffset, iZ )!=0 )
					{
						RenderVertex( x-fEdgeOffset, z, fTexLeft, fMidZ, bMultiTex );
						m_iTrisPerFrame++;
					}

					//lower left vertex
					RenderVertex( x-fEdgeOffset, z-fEdgeOffset, fTexLeft, fTexBottom, bMultiTex );
					m_iTrisPerFrame++;
				glEnd( );
				return;
			}

			//上面处理了对角有叶子结点的情况，还要处理UL UR 有叶子， LL LR 没有， 还有反过来  这些特殊有叶子节点的情况

			/*      
			8	           1               2
						   |
			               |              
						   |
				UL Node*4  |   UR Node*8
			7	-----------9-----------    3
						   |
			               |              
						   |
				LL Node*2  |   LR Node*1   
			6	           5               4
			*/

			//the remaining cases are only partial fans, so we need to figure out what to render
			//(thanks to Chris Cookson for this idea)
			iStart= g_cQTFanStart[iFanCode];// 1  2  4   8有叶子 ,  下两个有叶子 3   上两个有叶子 12,       左6    右9  
			//还有 2+4+8 =14 除了LR 其他三个都是有叶子结点的情况  还有其他几个情况，想一下就知道了

			iFanLength= 0;

			//1&1=1 1&0=0 0&0=0 
			// a&=b,就是将a与b做按位“与”运算，结果赋值给a,也就相当于a=a&b;
			//a |= b就是将a, b 做按位”或“运算，结果给a, 相当于a = a | b  相当于+=
			//calculate the fan length by computing the index of the first non-zero bit in g_cQTFanCode
			//0 或者 false 就继续while
			//这里判断8 是最后一个面  当LOD 最低级别0，最清晰时候 是8个三角形组成的  9个顶点
			//14 & 1 = 1 就是iFanLength = 1 时候就跳出循环了，得到fan是1个
			while( !( ( ( long )g_cQTFanCode[iFanCode] )&( 1<<iFanLength ) ) && iFanLength<8 )
				iFanLength++;
			//if (iFanCode == 14) {
				//char buffer[256];
				//int ret = snprintf(buffer, sizeof(buffer), "iFanLength:%d   1<<0:%d     g_cQTFanCode[14]:%d    (g_cQTFanCode[14])&(1<<0):%d", iFanLength, 
				//	1 << 0, g_cQTFanCode[14], ((long)g_cQTFanCode[14])&(1<<0));
				//char* num_string = buffer;
				//g_log.Write(LOG_PLAINTEXT, num_string);
				//11:01:51     iFanLength:1   1<<0:1     g_cQTFanCode[14]:14    (g_cQTFanCode[14])&(1<<0):0
				//std::cout << "iFanLenght:" << iFanLength << endl;
				//printf("%s",num_string);
			//}

			//render a triangle fan
			glBegin( GL_TRIANGLE_FAN );
				//center vertex
				RenderVertex( x, z, fMidX, fMidZ, bMultiTex );

				//render a triangle fan
				for( iFanPosition=iFanLength; iFanPosition>0; iFanPosition-- )
				{
					switch( iStart )
					{
						//lower right node
						case QT_LR_NODE:
							//lower mid, skip if the adjacent node is of a lower detail level
							if( ( ( iZ-iAdjOffset )<0 ) || GetQuadMatrixData( iX, iZ-iAdjOffset )!=0 || iFanPosition==iFanLength )
							{
								RenderVertex( x, z-fEdgeOffset, fMidX, fTexBottom, bMultiTex );
								m_iTrisPerFrame++;
							}

							//lower right vertex
							RenderVertex( x+fEdgeOffset, z-fEdgeOffset, fTexRight, fTexBottom, bMultiTex );
							m_iTrisPerFrame++;

							//finish off the fan with a right mid vertex
							if( iFanPosition==1 )
							{
								RenderVertex( x+fEdgeOffset, z, fTexRight, fMidZ, bMultiTex );
								m_iTrisPerFrame++;
							}
							break;

						//lower left node
						case QT_LL_NODE:
							//left mid, skip if the adjacent node is of a lower detail level
							if( ( ( x-iAdjOffset )<0 ) || GetQuadMatrixData( iX-iAdjOffset, iZ )!=0 || iFanPosition==iFanLength )
							{
								RenderVertex( x-fEdgeOffset, z, fTexLeft, fMidZ, bMultiTex );
								m_iTrisPerFrame++;
							}

							//lower left vertex
							RenderVertex( x-fEdgeOffset, z-fEdgeOffset, fTexLeft, fTexBottom, bMultiTex );
							m_iTrisPerFrame++;

							//finish off the fan with a lower mid vertex
							if( iFanPosition==1 )
							{
								RenderVertex( x, z-fEdgeOffset, fMidX, fTexBottom, bMultiTex );
								m_iTrisPerFrame++;
							}
							break;

						//upper left node
						case QT_UL_NODE:
							//upper mid, skip if the adjacent node is of a lower detail level
							if( ( ( iZ+iAdjOffset )>=m_iSize ) || GetQuadMatrixData( iX, iZ+iAdjOffset )!=0 || iFanPosition==iFanLength )
							{
								RenderVertex( x, z+fEdgeOffset, fMidX, fTexTop, bMultiTex );
								m_iTrisPerFrame++;
							}

							//upper left vertex
							RenderVertex( x-fEdgeOffset, z+fEdgeOffset, fTexLeft, fTexTop, bMultiTex );

							//finish off the fan with a left mid vertex
							if( iFanPosition==1 )
							{
								RenderVertex( x-fEdgeOffset, z, fTexLeft, fMidZ, bMultiTex );
								m_iTrisPerFrame++;
							}
							break;

						//upper right node
						case QT_UR_NODE:
							//right mid, skip if the adjacent node is of a lower detail level
							if( ( ( iX+iAdjOffset )>=m_iSize ) || GetQuadMatrixData( iX+iAdjOffset, iZ )!=0 || iFanPosition==iFanLength )
							{
								RenderVertex( x+fEdgeOffset, z, fTexRight, fMidZ, bMultiTex );
								m_iTrisPerFrame++;
							}

							//upper right vertex
							RenderVertex( x+fEdgeOffset, z+fEdgeOffset, fTexRight, fTexTop, bMultiTex );
							m_iTrisPerFrame++;

							//finish off the fan with a top mid vertex
							if( iFanPosition==1 )
							{
								RenderVertex( x, z+fEdgeOffset, fMidX, fTexTop, bMultiTex );
								m_iTrisPerFrame++;
							}
							break;
					}

					iStart--;
					iStart&= 3;//  -1 & 3  = 3  为了重置开始位置
				}
			glEnd( );

			//上面已经处理 不是对角的  没有叶子的顶点连接，下面这里再看哪些有叶子，逐个调用递归
			//now, recurse down to children (special cases that weren't handled earlier)
			for( iFanPosition=( 4-iFanLength ); iFanPosition>0; iFanPosition-- )
			{
				switch( iStart )
				{
					//lower right node
					case QT_LR_NODE:
						RenderNode( x+fChildOffset, z-fChildOffset, iChildEdgeLength );
						break;

					//lower left node
					case QT_LL_NODE:
						RenderNode( x-fChildOffset, z-fChildOffset, iChildEdgeLength );
						break;

					//upper left node
					case QT_UL_NODE:
						RenderNode( x-fChildOffset, z+fChildOffset, iChildEdgeLength );
						break;

					//upper right node
					case QT_UR_NODE:
						RenderNode( x+fChildOffset, z+fChildOffset, iChildEdgeLength );
						break;
				}

				iStart--;
				iStart&= 3;
			}

			return;
		}
	}
}