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


//brute Ұ������ʽ��Ⱦ��������
/*
0---2---4---6---8
| / | / | / | / |
1---3---5---7---9

GL_TRIANGLE_STRIP �������������ģʽ��һ����ֻ���������У�������������
ǰ���Ƕ����������Ϸֲ�ģʽ

���εĶ������͸߶�ͼ���ݵĳ���һ����128�ĸ߶�ͼ����128�����ζ���
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
		glBindTexture( GL_TEXTURE_2D, m_texture.GetID( ) );//��ͼ��512*512
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
				fTexLeft  = ( float )x/m_iSize;//uvѹ����128�ĸ߶�ͼ�ߴ�����£���ͼ�����ǲ���ȫͼ�ģ�ֻ������512/4
				fTexBottom= ( float )z/m_iSize;
				fTexTop	  = ( float )( z+1 )/m_iSize;

				//set the color with OpenGL, and render the point
				glColor3ub( 255, 255, 255 );
				glTexCoord2f( fTexLeft, fTexBottom );
				glVertex3f( ( float )x, GetScaledHeightAtPoint( x, z ), ( float )z );//�õû��ǣ��ϲ��㷨���е�ʽ�ϲ��㷨�õ�������߶�ͼ����

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