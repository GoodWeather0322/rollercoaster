#include "3DSLoader.h"   
#include "CBMPLoader.h"   

C3DSLoader::C3DSLoader()   
{   
	m_CurrentChunk = new tChunk;      
	m_TempChunk = new tChunk;      
	m_3DModel.numOfObjects = 0;   
	m_3DModel.numOfMaterials = 0;   
	for (int i=0; i<MAX_TEXTURES; i++)    
		m_textures[i] = 0;   
}   
 
void C3DSLoader::CleanUp()   
{   
	fclose(m_FilePointer);         
	delete m_CurrentChunk;          
	delete m_TempChunk;            
}   

C3DSLoader::~C3DSLoader()   
{   
	m_3DModel.numOfObjects = 0;   
	m_3DModel.numOfMaterials = 0;   
	m_3DModel.pObject.clear();   
	m_3DModel.pMaterials.clear();   
	glDeleteTextures(MAX_TEXTURES, m_textures);   
}   

void C3DSLoader::Init(char *filename)   
{       
	Import3DS(&m_3DModel, filename);       

	for(int i =0; i<m_3DModel.numOfMaterials;i++)   
	{    
		if(strlen(m_3DModel.pMaterials[i].strFile)>0)                 
			LoadTexture(m_3DModel.pMaterials[i].strFile,m_textures, i);          

		   
		m_3DModel.pMaterials[i].texureId = i;                        
	}    
}   

void C3DSLoader::Draw()    
{   
	glPushAttrib(GL_CURRENT_BIT);   
	glDisable(GL_TEXTURE_2D);   

	for(int i = 0; i < m_3DModel.numOfObjects; i++)   
	{   
		if(m_3DModel.pObject.size() <= 0)    
			break;                    

		t3DObject *pObject = &m_3DModel.pObject[i];  

		if(pObject->bHasTexture)                  
		{     
			glEnable(GL_TEXTURE_2D);               
			glBindTexture(GL_TEXTURE_2D, m_textures[pObject->materialID]);   
		}    
		else      
			glDisable(GL_TEXTURE_2D);               

		glColor3ub(255, 255, 255);   

		glBegin(GL_TRIANGLES);   
		for(int j = 0; j < pObject->numOfFaces; j++)    
		{for(int tex = 0; tex < 3; tex++)                    
		{   
			int index = pObject->pFaces[j].vertIndex[tex]; 

			glNormal3f(pObject->pNormals[index].x,pObject->pNormals[index].y,     
				pObject->pNormals[index].z);     

			if(pObject->bHasTexture)                        
			{     
				if(pObject->pTexVerts)                      
					glTexCoord2f(pObject->pTexVerts[index].x,pObject->pTexVerts[index].y);   
			}   
			else   
			{     
				if(m_3DModel.pMaterials.size() && pObject->materialID>= 0)    
				{      
					BYTE *pColor = m_3DModel.pMaterials[pObject->materialID].color;   
					glColor3ub(pColor[0],pColor[1],pColor[2]);   
				}   
			}   
			glVertex3f(pObject->pVerts[index].y,pObject->pVerts[index].z,pObject->pVerts[index].x);   
		}   
		}   
		glEnd();   
	}   
	glEnable(GL_TEXTURE_2D);   

	glPopAttrib();   
}   

void C3DSLoader::LoadTexture(char* filename, GLuint textureArray[], GLuint textureID)   
{   

	if(!filename)   
		return;   

	if(!m_BMPTexture.LoadBitmap(filename))   
	{   
		exit(0);   
	}   
	glGenTextures(1,&m_textures[textureID]);   

	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);   
	glBindTexture(GL_TEXTURE_2D, m_textures[textureID]);   

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);   
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);   

	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, m_BMPTexture.imageWidth, m_BMPTexture.imageHeight, GL_RGB,    
		GL_UNSIGNED_BYTE, m_BMPTexture.image);   

}   

bool C3DSLoader::Import3DS(t3DModel *pModel, char *strFileName)   
{      
	char strMessage[255] = {0};   

	m_FilePointer = fopen(strFileName, "rb");   

	if(!m_FilePointer)    
	{      
		sprintf(strMessage, "%s!", strFileName);   
		return false;   
	}   

	ReadChunk(m_CurrentChunk);   

	if (m_CurrentChunk->ID != PRIMARY)   
	{      
		return false;   
	}   

	ReadNextChunk(pModel, m_CurrentChunk);   
 
	ComputeNormals(pModel);   

	CleanUp();   

	return true;   
}   
int C3DSLoader::GetString(char *pBuffer)   
{      
	int index = 0;   

	fread(pBuffer, 1, 1, m_FilePointer);   

	while (*(pBuffer + index++) != 0)    
	{    
		fread(pBuffer + index, 1, 1, m_FilePointer);   
	}   

	return strlen(pBuffer) + 1;   
}    

void C3DSLoader::ReadChunk(tChunk *pChunk)   
{      
	pChunk->bytesRead = fread(&pChunk->ID, 1, 2, m_FilePointer);   

	pChunk->bytesRead += fread(&pChunk->length, 1, 4, m_FilePointer);   
}   

void C3DSLoader::ReadNextChunk(t3DModel *pModel, tChunk *pPreChunk)   
{      
	t3DObject newObject = {0};                 
	tMatInfo newTexture = {0};              
	unsigned int version = 0;              
	int buffer[50000] = {0};                
	m_CurrentChunk = new tChunk;             

	while (pPreChunk->bytesRead < pPreChunk->length)   
	{       
		ReadChunk(m_CurrentChunk);   

		switch (m_CurrentChunk->ID)   
		{   

		case VERSION:                              

			m_CurrentChunk->bytesRead += fread(&version, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);   
 
			break;   

		case OBJECTINFO:                           

			ReadChunk(m_TempChunk);   
  
			m_TempChunk->bytesRead += fread(&version, 1, m_TempChunk->length - m_TempChunk->bytesRead, m_FilePointer);   
 
			m_CurrentChunk->bytesRead += m_TempChunk->bytesRead;   
  
			ReadNextChunk(pModel, m_CurrentChunk);   
			break;   

		case MATERIAL:                             

			pModel->numOfMaterials++;   
 
			pModel->pMaterials.push_back(newTexture);   

			ReadNextMatChunk(pModel, m_CurrentChunk);   
			break;   

		case OBJECT:                               

			pModel->numOfObjects++;   
 
			pModel->pObject.push_back(newObject);   

			memset(&(pModel->pObject[pModel->numOfObjects - 1]), 0, sizeof(t3DObject));   

			m_CurrentChunk->bytesRead += GetString(pModel->pObject[pModel->numOfObjects - 1].strName);   
 
			ReadNextObjChunk(pModel, &(pModel->pObject[pModel->numOfObjects - 1]), m_CurrentChunk);   
			break;   

		case EDITKEYFRAME:   

			m_CurrentChunk->bytesRead += fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);   
			break;   
		default:    
  
			m_CurrentChunk->bytesRead += fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);   
			break;   
		}   

		pPreChunk->bytesRead += m_CurrentChunk->bytesRead;   
	}   

	delete m_CurrentChunk;   
	m_CurrentChunk = pPreChunk;   

}    
void C3DSLoader::ReadNextObjChunk(t3DModel *pModel, t3DObject *pObject, tChunk *pPreChunk)   
{      
	int buffer[50000] = {0};              
	m_CurrentChunk = new tChunk;   
  
	while (pPreChunk->bytesRead < pPreChunk->length)   
	{      
		ReadChunk(m_CurrentChunk);   
  
		switch (m_CurrentChunk->ID)   
		{   
		case OBJ_MESH:                   
			ReadNextObjChunk(pModel, pObject, m_CurrentChunk);   
			break;   
		case OBJ_VERTICES:             
			ReadVertices(pObject, m_CurrentChunk);   
			break;   
		case OBJ_FACES:                  
			ReadVertexIndices(pObject, m_CurrentChunk);   
			break;   
		case OBJ_MATERIAL:              
			ReadObjMat(pModel, pObject, m_CurrentChunk);               
			break;   
		case OBJ_UV:                      
			ReadUVCoordinates(pObject, m_CurrentChunk);   
			break;   
		default:     
			m_CurrentChunk->bytesRead += fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);   
			break;   
		}      
		pPreChunk->bytesRead += m_CurrentChunk->bytesRead;   
	}   
	delete m_CurrentChunk;   
	m_CurrentChunk = pPreChunk;   
}   
void C3DSLoader::ReadNextMatChunk(t3DModel *pModel, tChunk *pPreChunk)   
{      
	int buffer[50000] = {0};         
	m_CurrentChunk = new tChunk;  
	while (pPreChunk->bytesRead < pPreChunk->length)   
	{      
		ReadChunk(m_CurrentChunk);   
		switch (m_CurrentChunk->ID)   
		{   
		case MATNAME:                      
			m_CurrentChunk->bytesRead += fread(pModel->pMaterials[pModel->numOfMaterials - 1].strName, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);   
			break;   
		case MATDIFFUSE:                         
			ReadColor(&(pModel->pMaterials[pModel->numOfMaterials - 1]), m_CurrentChunk);   
			break;   
		case MATMAP:                           
			ReadNextMatChunk(pModel, m_CurrentChunk);   
			break;   
		case MATMAPFILE:                       
			m_CurrentChunk->bytesRead += fread(pModel->pMaterials[pModel->numOfMaterials - 1].strFile, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);   
			break;   
		default:      
			m_CurrentChunk->bytesRead += fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);   
			break;   
		}     
		pPreChunk->bytesRead += m_CurrentChunk->bytesRead;   
	}   
	delete m_CurrentChunk;   
	m_CurrentChunk = pPreChunk;   
}   

void C3DSLoader::ReadColor(tMatInfo *pMaterial, tChunk *pChunk)   
{         
	ReadChunk(m_TempChunk);   
	m_TempChunk->bytesRead += fread(pMaterial->color, 1, m_TempChunk->length - m_TempChunk->bytesRead, m_FilePointer);   
	pChunk->bytesRead += m_TempChunk->bytesRead;   
}   
 
void C3DSLoader::ReadVertexIndices(t3DObject *pObject, tChunk *pPreChunk)   
{      
	unsigned short index = 0;                 
	pPreChunk->bytesRead += fread(&pObject->numOfFaces, 1, 2, m_FilePointer);     
	pObject->pFaces = new tFace [pObject->numOfFaces];   
	memset(pObject->pFaces, 0, sizeof(tFace) * pObject->numOfFaces);   
	for(int i = 0; i < pObject->numOfFaces; i++)   
	{   for(int j = 0; j < 4; j++)   
	{      
		pPreChunk->bytesRead += fread(&index, 1, sizeof(index), m_FilePointer);   
		if(j < 3)   
		{        
			pObject->pFaces[i].vertIndex[j] = index;   
		}   
	}   
	}   
}   
  
void C3DSLoader::ReadUVCoordinates(t3DObject *pObject, tChunk *pPreChunk)   
{      
	pPreChunk->bytesRead += fread(&pObject->numTexVertex, 1, 2, m_FilePointer);   

	pObject->pTexVerts = new Vector2 [pObject->numTexVertex];   
 
	pPreChunk->bytesRead += fread(pObject->pTexVerts, 1, pPreChunk->length - pPreChunk->bytesRead, m_FilePointer);   
}   

void C3DSLoader::ReadVertices(t3DObject *pObject, tChunk *pPreChunk)   
{      
	pPreChunk->bytesRead += fread(&(pObject->numOfVerts), 1, 2, m_FilePointer);   

	pObject->pVerts = new Vector3 [pObject->numOfVerts];   
	memset(pObject->pVerts, 0, sizeof(Vector3) * pObject->numOfVerts);   

	pPreChunk->bytesRead += fread(pObject->pVerts, 1, pPreChunk->length - pPreChunk->bytesRead, m_FilePointer);   
   
	for(int i = 0; i < pObject->numOfVerts; i++)   
	{        
		float fTempY = pObject->pVerts[i].y;   
		pObject->pVerts[i].y = pObject->pVerts[i].z;   
		pObject->pVerts[i].z = -fTempY;   
	}   
}   

void C3DSLoader::ReadObjMat(t3DModel *pModel, t3DObject *pObject, tChunk *pPreChunk)   
{      
	char strMaterial[255] = {0};         
	int buffer[50000] = {0};           
 
	pPreChunk->bytesRead += GetString(strMaterial);   

	for(int i = 0; i < pModel->numOfMaterials; i++)   
	{      
		if(strcmp(strMaterial, pModel->pMaterials[i].strName) == 0)   
		{      
			pObject->materialID = i;   
			if(strlen(pModel->pMaterials[i].strFile) > 0) {   
  
				pObject->bHasTexture = true;   
			}      
			break;   
		}   
		else   
		{         
			pObject->materialID = -1;   
		}   
	}   
	pPreChunk->bytesRead += fread(buffer, 1, pPreChunk->length - pPreChunk->bytesRead, m_FilePointer);   
}   
  
void C3DSLoader::ComputeNormals(t3DModel *pModel)   
{      
	Vector3 vVector1, vVector2, vNormal, vPoly[3];   
  
	if(pModel->numOfObjects <= 0)   
		return;   
 
	for(int index = 0; index < pModel->numOfObjects; index++)   
	{        
		t3DObject *pObject = &(pModel->pObject[index]);   

		Vector3 *pNormals       = new Vector3 [pObject->numOfFaces];   
		Vector3 *pTempNormals   = new Vector3 [pObject->numOfFaces];   
		pObject->pNormals        = new Vector3 [pObject->numOfVerts];   
   
		for(int i=0; i < pObject->numOfFaces; i++)   
		{  
			vPoly[0] = pObject->pVerts[pObject->pFaces[i].vertIndex[0]];   
			vPoly[1] = pObject->pVerts[pObject->pFaces[i].vertIndex[1]];   
			vPoly[2] = pObject->pVerts[pObject->pFaces[i].vertIndex[2]];   

			vVector1 = vPoly[0] - vPoly[2];          
			vVector2 = vPoly[2] - vPoly[1];               
			vNormal  = vVector1.crossProduct(vVector2);   
			pTempNormals[i] = vNormal;                     
			vNormal  = vNormal.normalize();              
			pNormals[i] = vNormal;                       
		}   

		Vector3 vSum(0.0,0.0,0.0);   
		Vector3 vZero = vSum;   
		int shared=0;   
   
		for (int i = 0; i < pObject->numOfVerts; i++)            
		{   
			for (int j = 0; j < pObject->numOfFaces; j++) 
			{                                              
				if (pObject->pFaces[j].vertIndex[0] == i ||    
					pObject->pFaces[j].vertIndex[1] == i ||    
					pObject->pFaces[j].vertIndex[2] == i)   
				{      
					vSum = vSum + pTempNormals[j];   
					shared++;                                  
				}   
			}         
			pObject->pNormals[i] = vSum / float(-shared);   

			pObject->pNormals[i] = pObject->pNormals[i].normalize();     
			vSum = vZero;                                  
			shared = 0;                                        
		}   
		delete [] pTempNormals;   
		delete [] pNormals;   
	}   
}   