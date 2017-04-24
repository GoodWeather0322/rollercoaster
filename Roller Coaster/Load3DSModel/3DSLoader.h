#ifndef __3DSLOADER_H__ 
#define __3DSLOADER_H__ 

#include "vector.h" 
#include "CBMPLoader.h" 
#include <math.h> 
#include <vector>
#include <GL/GLU.h>
using namespace std;          

#define PRIMARY       0x4D4D 

#define OBJECTINFO    0x3D3D	
#define VERSION       0x0002		
#define EDITKEYFRAME  0xB000		

#define MATERIAL	  0xAFFF		
#define OBJECT		  0x4000		

#define MATNAME       0xA000		
#define MATDIFFUSE    0xA020		
#define MATMAP        0xA200		
#define MATMAPFILE    0xA300		
#define OBJ_MESH	  0x4100		 


#define OBJ_VERTICES  0x4110		
#define OBJ_FACES	  0x4120		
#define OBJ_MATERIAL  0x4130		
#define OBJ_UV		  0x4140		

 
struct tFace		 
{	 
	int vertIndex[3];			  
	int coordIndex[3];			   
}; 

struct tMatInfo 
{	 
	char  strName[255];			   
	char  strFile[255];			   
	BYTE  color[3];				
	int   texureId;				
	float uTile;				   
	float vTile;			
	float uOffset;			  
	float vOffset;				 
} ; 

struct t3DObject	 
{	 
	int  numOfVerts;		     
	int  numOfFaces;			  
	int  numTexVertex;			 
	int  materialID;			 
	bool bHasTexture;			  
	char strName[255];			
	Vector3  *pVerts;			 
	Vector3  *pNormals;		      
	Vector2  *pTexVerts;		
	tFace *pFaces;				  
}; 

struct t3DModel	 
{	int numOfObjects;			
int numOfMaterials;			 
vector<tMatInfo>pMaterials;	  
vector<t3DObject> pObject;	 
}; 

struct tChunk	 
{	 
	unsigned short int ID;		 
	unsigned int length;		  
	unsigned int bytesRead;		
}; 

#define MAX_TEXTURES  100 

class C3DSLoader 
{ 
public: 
	C3DSLoader();								 
	virtual ~C3DSLoader(); 
	void Draw();
	void Init(char *filename); 

private: 
	int  GetString(char *);								 

	bool Import3DS(t3DModel *pModel, char *strFileName); 

	void LoadTexture(char* filename, GLuint textureArray[], GLuint textureID);

	void ReadChunk(tChunk *);		 

	void ReadNextChunk(t3DModel *pModel, tChunk *); 

	void ReadNextObjChunk(t3DModel *pModel,t3DObject *pObject,tChunk *); 

	void ReadNextMatChunk(t3DModel *pModel, tChunk *);	 

	void ReadColor(tMatInfo *pMaterial, tChunk *pChunk); 

	void ReadVertices(t3DObject *pObject, tChunk *); 

	void ReadVertexIndices(t3DObject *pObject,tChunk *); 

	void ReadUVCoordinates(t3DObject *pObject,tChunk *); 

	void ReadObjMat(t3DModel *pModel,t3DObject *pObject,tChunk *pPreChunk); 

	void ComputeNormals(t3DModel *pModel);	 

	void CleanUp();										 

	FILE	     *m_FilePointer;						 
	tChunk	     *m_CurrentChunk;            
	tChunk	     *m_TempChunk;              
	CBMPLoader   m_BMPTexture;               
	GLuint       m_textures[MAX_TEXTURES];    
	t3DModel     m_3DModel;                  
}; 

#endif
