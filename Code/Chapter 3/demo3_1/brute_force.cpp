//==============================================================
//==============================================================
//= brute_force.cpp ============================================
//= Original coders: Trent Polack (trent@voxelsoft.com)		   =
//==============================================================
//= This file (along with brute_force.h) contains the		   =
//= information for a brute force terrain implementation.	   =
//==============================================================
//==============================================================


//--------------------------------------------------------------
//--------------------------------------------------------------
//- HEADERS AND LIBRARIES --------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
#include <stdio.h>

#include "../Base Code/gl_app.h"

#include "brute_force.h"


//brute 野蛮无脑式渲染地形数据
/*
0---2---4---6---8
| / | / | / | / |
1---3---5---7---9

GL_TRIANGLE_STRIP 这个三角形连接模式不一样，只需上下两行，两两顶点链接
前提是顶点数量以上分布模式

地形的顶点数和高度图数据的长度一样，128的高度图就是128个地形顶点
*/
//--------------------------------------------------------------
//--------------------------------------------------------------
//- DEFINITIONS ------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------

//--------------------------------------------------------------
// Name:			CBRUTE_FORCE::Render - public
// Description:		Render the terrain height field
// Arguments:		None
// Return Value:	None
//--------------------------------------------------------------
void CBRUTE_FORCE::Render( void )
{
	float fTexLeft, fTexBottom, fTexTop;
	int	x, z;

	//reset the counting variables
	m_iVertsPerFrame= 0;
	m_iTrisPerFrame = 0;

	//cull non camera-facing polygons
	glEnable( GL_CULL_FACE );

	//if the texture is loaded then enable texturing and bind our texture
	if( m_bTextureMapping )
	{
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, m_texture.GetID( ) );//贴图是512*512
	}

	//loop through the Z-axis of the terrain
	for( z=0; z<m_iSize-1; z++ )
	{
		//begin a new triangle strip
		glBegin( GL_TRIANGLE_STRIP );

			//loop through the X-axis of the terrain
			//this is where the triangle strip is constructed
			for( x=0; x<m_iSize-1; x++ )
			{
				//calculate the texture coordinates
				fTexLeft  = ( float )x/m_iSize;//uv压缩到128的高度图尺寸比例下，贴图采样是不满全图的，只采样到512/4
				fTexBottom= ( float )z/m_iSize;
				fTexTop	  = ( float )( z+1 )/m_iSize;

				//set the color with OpenGL, and render the point
				glColor3ub( 255, 255, 255 );
				glTexCoord2f( fTexLeft, fTexBottom );
				glVertex3f( ( float )x, GetScaledHeightAtPoint( x, z ), ( float )z );//用得还是，断层算法或中点式断层算法得到的随机高度图数据

				//set the color with OpenGL, and render the point
				glColor3ub( 255, 255, 255 );
				glTexCoord2f( fTexLeft, fTexTop );
				glVertex3f( ( float )x, GetScaledHeightAtPoint( x, z+1 ), ( float )z+1 );

				//increase the vertex count by two (which is how many we sent to the API)
				m_iVertsPerFrame+= 2;

				//there are no triangles being rendered on the first X-loop, they just start the
				//triangle strip off
				if( x!= 0 )
					m_iTrisPerFrame+= 2;
			}

		//end the triangle strip
		glEnd( );
	}
}