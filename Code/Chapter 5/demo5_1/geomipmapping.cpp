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
����˵��anti-cracking  ָ���ǿ����ԣ������֮��ƴ��ʱ���Ѻ����


Ҳ��һ��
 ?? ������patch�ķ�ʽ����Ⱦ��ֻ�������� �߶�ͼ���� �Ͷ�����һ�µ����
���� 256 �߶�ͼ �� 15 * 15 ��patch�� ÿ��patch �� 17 * 17 ������  ??
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
	m_iNumPatchesPerSide= m_iSize/m_iPatchSize;//256 + 1 �߶�ͼ257 / 16 + 1 patch�ĳߴ�17 =  ÿһ��15��patch ��256 + 1 �ĸ߶�ͼ������15 * 15 ��patch
	m_pPatches= new SGEOMM_PATCH [SQR( m_iNumPatchesPerSide )];//SQR ��  m_iNumPatchesPerSide * m_iNumPatchesPerSide  15 * 15 ��
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
		iDivisor= iDivisor>>1;//�����½���ͨ�������������ε�������������ж��ټ�LOD 17 * 17  ���� 16,8,4,2 ��patch 17 * 17 һ����16��������
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
			m_pPatches[GetPatchNumber( x, z )].m_iLOD= m_iMaxLOD;//������ʼ������ÿһ��patch����� LOD�����ʷ�ʽҲ��x,z*m_iNumPatchesPerSide���߶�ͼһ��������
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
// update ���¸���patch��LOD ��levelֵ
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
			fX= ( x*m_iPatchSize )+( m_iPatchSize/2.0f );//����patch���ĵ�x,z  x*m_iPatchSize �ǵ�x��patch * 17���㣬�������patch�ĵ�һ�����㣬+�� 17/2 ��������
			fZ= ( z*m_iPatchSize )+( m_iPatchSize/2.0f );
			fY= GetScaledHeightAtPoint( ( int )fX, ( int )fZ );

			//only scale the X and Z values, the Y value has already been scaled
			fX*= m_vecScale[0];//��������ֵ������ʵ�ʾ���
			fZ*= m_vecScale[2];

			//get the distance from the camera to the patch
			//update��ÿ֡����ÿ��patch������Ұ�ľ��� x^+y^+z^  ����
			m_pPatches[iPatch].m_fDistance= sqrtf( SQR( ( fX-camera.m_vecEyePos[0] ) )+
												   SQR( ( fY-camera.m_vecEyePos[1] ) )+
												   SQR( ( fZ-camera.m_vecEyePos[2] ) ) );

			/*
			������Ӳ�����ж����룬����LOD����
			������ж���ʽ�� ������Ļ����ȷ���㷨 ��ȷ��patch��LOD
			�����LOD�л��ᵼ��popping ---- ��3D �����ͼ��ѧ�У�������ָ�� 3D ����ת������ͬ��Ԥ�ȼ����ϸ�ڼ���(LOD) ʱ���ֵĲ��ܻ�ӭ���Ӿ�Ч�������ڹۿ�����˵��ͻȻ�����Ե�
			�㷨���Ʋ��ã��ᵼ��Ƶ���л���Ӱ�컭��Ч��
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


		//û��shader������£�ͨ��ͼ��api��ָ��������ͼ��ɫ
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
				RenderPatch( x, z, true, true );//��������patch����ʼ��Ⱦpatch
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
// PX, PZ �������ĵ�ǰpatch��x��y
// ����Ҫ��Ⱦ��fan��������fan�����ĵ�
//--------------------------------------------------------------
void CGEOMIPMAPPING::RenderPatch( int PX, int PZ, bool bMultiTex, bool bDetail )
{

	/*
	��������Ⱦһ��patch��һ��patch��������fan��������Ⱦ�����룬Ȼ��fan������ִ����Ҫ�� vertexs������������

	*/
	SGEOMM_NEIGHBOR patchNeighbor;//���patch���ھ�
	SGEOMM_NEIGHBOR fanNeighbor;//���fan���ھ�fan
	float fSize;
	float fHalfSize;
	float x, z;
	int iPatch= GetPatchNumber( PX, PZ );
	int iDivisor;
	int iLOD;

	//find out information about the patch to the current patch's left, if the patch is of a
	//greater detail or there is no patch to the left, we can render the mid-left vertex
	//�˽⵱ǰpatch����patch��Ϣ���������patch�ĸ���ϸ�����û��patch�����ǿ�����Ⱦ���ж���
	// ע�⣬���������۲�㣬�ڵ�ͼ�м䣬�ͻ�����ߵ�lod �ֲ� �� �ұߵ�lod�ֲ� �෴
	//			2
	//			1
	//			0 
	// 3 2 1 0 eys 0 1 2 3    
	// 3 2 1 0	0
	//			1
	//			2
	// ������ж����ǰ������������
	//�����  �������ĸ�����
	if( m_pPatches[GetPatchNumber( PX-1, PZ )].m_iLOD<=m_pPatches[iPatch].m_iLOD || PX==0 )
		patchNeighbor.m_bLeft= true;//��ʾ�����һ�������ȸߵ�patch������Ҫ�������������ѷ죬���Լ��Ͳ��ã�������Ⱦ����ߵ��м��(the mid-left vertex)
	else
		patchNeighbor.m_bLeft= false;

	//find out about the upper patch //�����patch��lod ���ȵ�ǰ��Ⱦ��patch��lod С  ����  ��ǰ�����һ��patch
	if( m_pPatches[GetPatchNumber( PX, PZ+1 )].m_iLOD<=m_pPatches[iPatch].m_iLOD || PZ==m_iNumPatchesPerSide )
		patchNeighbor.m_bUp= true;//��ʾ������һ�������ȸߵ�patch��������Ⱦ�±��ߵ��м��
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
	//����ÿһ��fan֮��ľ���  ��ʵҪ������˶��ٸ�������
	fSize   = ( float )m_iPatchSize;//17
	iDivisor= m_iPatchSize-1;//16
	iLOD    = m_pPatches[iPatch].m_iLOD;

	//find out how many fan divisions we are going to have
	//�ҳ����ж������η���
	/*
	������ҪiLOD-- �����ټ���Ŀ���ǿ��Խ����һ�Σ�LOD 0 == 0  ʱ�򣬼������� 
	
	*/

	while( iLOD-->=0 )
		iDivisor= iDivisor>>1;//iDivisor��int����������λ���㣬Ч�ʻ�ߣ���������

	//���0 ���˳�ѭ�����ͻ���� iLOD =0 ʱ��iDivisor = 16��fSize = iDivisor �������fSize/= iDivisor������� fSize = 1
	// ���LOD 0 ������patch֮�䣬��Ҫ���2����λ��������1����Ȼ���ص����������ϵ�ͼ��֪��
	// 
	// ��Ϊ������ִ����Ⱦfan��������������ģʽ�������Σ������Ǽ���������֮��ľ��룬���ε����ĵ�֮����롣
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
		����0 �� ���� ��0 �ľ��룬�������fan�����ĵ����  
		LOD 0 ������������ߵ�levelʱ�� ÿһ��fan������Ⱦ�������㹻��Ķ��㣬17 * 17 ���㶼����
		��ʱ������֮��ľ������2�������٣���Ϊfan��
		LOD Խ�� ��level 3����Ҫ��fanԽ�٣�����֮��ľ���ͻ�Խ��
	*/
	 
	// 
	//the size between the center of each triangle fan
	fSize/= iDivisor;//level 3  --> iDivisor = 1    fSize = 17  fHalfSize = 8

	//half the size between the center of each triangle fan (this will be
	//the size between each vertex)
	fHalfSize= fSize/2.0f; //�����������fan�����ĵ�֮��ĵ�λ����
	for( z=fHalfSize; ( ( int )z+fHalfSize )<m_iPatchSize+1; z+=fSize )
	{
		for( x=fHalfSize; ( ( int )x+fHalfSize )<m_iPatchSize+1; x+=fSize )
		{
			//if this fan is in the left row, we may need to adjust it's rendering to
			//prevent cracks
			//������fan�������ǿ�����Ҫ����������Ⱦ�Է�ֹ�����ѷ�
			if( x==fHalfSize )//��һ��
				fanNeighbor.m_bLeft= patchNeighbor.m_bLeft;
			else
				fanNeighbor.m_bLeft= true;//���ǵ�һ���������fan����߶�����fan��������true

			//if this fan is in the bottom row, we may need to adjust it's rendering to
			//prevent cracks
			if( z==fHalfSize )//��һ�У�
				fanNeighbor.m_bDown= patchNeighbor.m_bDown;//�ϱ��ߵ����������Ҫ��patch�����һ�����
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
				fanNeighbor.m_bUp= patchNeighbor.m_bUp;//���һ�У��±��ߵ����
			else
				fanNeighbor.m_bUp= true;

			//PX*m_iPatchSize �ǹ��˶��ٸ�patch�����ٸ�����ĵ�λ���룬+x ���ǵ�ǰfan�����ĵ㵥λ
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
// ����fan�����ĵ㣬��ʼ����������
//--------------------------------------------------------------
void CGEOMIPMAPPING::RenderFan( float cX, float cZ, float fSize, SGEOMM_NEIGHBOR neighbor, bool bMultiTex, bool bDetail )
{
	float fTexLeft, fTexBottom, fMidX, fMidZ, fTexRight, fTexTop;
	float fHalfSize= fSize/2.0f;

	//����UV
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
	//GL_TRIANGLE_FAN���Ƹ��������γ�һ���������У���v0Ϊ��ʼ�㣬��v0��v1��v2������v0��v2��v3������v0��v3��v4����
	/*
	* 
	��0Ϊ����  ÿ��patch�����Ӷ��㷽ʽ
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
		//render the CENTER vertex  fan 0 ���ĵ� 
		RenderVertex( cX, cZ, fMidX, fMidZ, bMultiTex );

		//render the LOWER-LEFT vertex ��ͼ ���½� 3 �����
		RenderVertex( cX-fHalfSize, cZ-fHalfSize, fTexLeft, fTexBottom, bMultiTex );		

		//only render the next vertex if the left patch is NOT of a lower LOD
		if( neighbor.m_bLeft )//2 ���Ի���һ���м�Ķ��㣬��Ϊ����Ǹ������ȵ�fan������ȥ��һ���м�ģ��������ѷ�
		{
			RenderVertex( cX-fHalfSize, cZ, fTexLeft, fMidZ, bMultiTex );
			m_iTrisPerFrame++;
		}
	
		//render the UPPER-LEFT vertex ���Ͻ�
		RenderVertex( cX-fHalfSize, cZ+fHalfSize, fTexLeft, fTexTop, bMultiTex );
		m_iTrisPerFrame++;

		//only render the next vertex if the upper patch is NOT of a lower LOD
		if( neighbor.m_bUp )
		{
			RenderVertex( cX, cZ+fHalfSize, fMidX, fTexTop, bMultiTex );
			m_iTrisPerFrame++;
		}

		//render the UPPER-RIGHT vertex ���Ͻ�
		RenderVertex( cX+fHalfSize, cZ+fHalfSize, fTexRight, fTexTop, bMultiTex );
		m_iTrisPerFrame++;

		//only render the next vertex if the right patch is NOT of a lower LOD
		if( neighbor.m_bRight )
		{
			//render the MID-RIGHT vertex
			RenderVertex( cX+fHalfSize, cZ, fTexRight, fMidZ, bMultiTex );
			m_iTrisPerFrame++;
		}

		//render the LOWER-RIGHT vertex ���½�
		RenderVertex( cX+fHalfSize, cZ-fHalfSize, fTexRight, fTexBottom, bMultiTex );
		m_iTrisPerFrame++;	

		//only render the next vertex if the bottom patch is NOT of a lower LOD
		if( neighbor.m_bDown )
		{
			//render the LOWER-MID vertex
			RenderVertex( cX, cZ-fHalfSize, fMidX, fTexBottom, bMultiTex );	
			m_iTrisPerFrame++;
		}

		//render the LOWER-LEFT vertex ���½�
		RenderVertex( cX-fHalfSize, cZ-fHalfSize, fTexLeft, fTexBottom, bMultiTex );
		m_iTrisPerFrame++;	

	//end the triangle strip
	glEnd( );
}