#include"CBMPLoader.h"           

CBMPLoader::CBMPLoader()
{
	image = 0;
	imageWidth = 0;
	imageHeight = 0;
}

CBMPLoader::~CBMPLoader()
{
	FreeImage(); 
}

bool CBMPLoader::LoadBitmap(const char *file)
{
	FILE *pFile = 0;

	BITMAPINFOHEADER bitmapInfoHeader;
	BITMAPFILEHEADER header;

	unsigned char textureColors = 0;

	pFile = fopen(file, "rb");
	if(pFile == 0)
		return false;

	fread(&header, sizeof(BITMAPFILEHEADER), 1, pFile);

	if(header.bfType != BITMAP_ID)
	{
		fclose(pFile);       
		return false;
	}

	fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, pFile);

	imageWidth = bitmapInfoHeader.biWidth;
	imageHeight = bitmapInfoHeader.biHeight;

	if(bitmapInfoHeader.biSizeImage == 0)
		bitmapInfoHeader.biSizeImage = bitmapInfoHeader.biWidth *
		bitmapInfoHeader.biHeight * 3;

	fseek(pFile, header.bfOffBits, SEEK_SET);

	image = new unsigned char[bitmapInfoHeader.biSizeImage];

	if(!image)                     
	{
		delete[] image;
		fclose(pFile);
		return false;
	}

	fread(image, 1, bitmapInfoHeader.biSizeImage, pFile);

	for(int index = 0; index < (int)bitmapInfoHeader.biSizeImage; index+=3)
	{
		textureColors = image[index];
		image[index] = image[index + 2];
		image[index + 2] = textureColors;
	}

	fclose(pFile);      
	return true;         
}

void CBMPLoader::FreeImage()
{
	if(image)
	{
		delete[] image;
		image = 0;
	}
}
