#ifndef __CBMPLOADER_H__
#define __CBMPLOADER_H__

#define BITMAP_ID 0x4D42

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <windows.h>

using namespace std;

class CBMPLoader
{
public:
	CBMPLoader();
	~CBMPLoader();

	bool LoadBitmap(const char* filename);
	void FreeImage();            

	unsigned int ID;              
	int imageWidth;                 
	int imageHeight;               
	unsigned char *image;         
};

#endif 
