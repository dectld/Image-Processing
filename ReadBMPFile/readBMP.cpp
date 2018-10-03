#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <opencv2\opencv.hpp>
using namespace std;
using namespace cv;

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned char BYTE;

#pragma pack(2)
struct BMPFileHeader {
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
}file_header;

#pragma pack(2)
struct BMPInfoHeader {
	DWORD   biSize;
	DWORD    biWidth;
	DWORD    biHeight;
	WORD    biPlanes;
	WORD   biBitCount;
	DWORD   biCompression;
	DWORD   biSizeImage;
	DWORD    biXPelsPerMeter;
	DWORD    biYPelsPerMeter;
	DWORD   biClrUsed;
	DWORD   bitClrImportant;
}info_header;

const int maxn = 2000*1000;
struct RGBColor {
	BYTE B;
	BYTE G;
	BYTE R;
}rgbs_origin[maxn], rgbs_gray[maxn];

struct ColorPalette {
	BYTE B;
	BYTE G;
	BYTE R;
	BYTE Reversed;
}palettes[256], gray_palettes[256];

BYTE idxs[maxn];

int main() {
	FILE *reader = fopen("origin.bmp", "rb");
	fread(&file_header, sizeof(BMPFileHeader), 1, reader);
	fread(&info_header, sizeof(BMPInfoHeader), 1, reader);
	int width = info_header.biWidth;
	int height = info_header.biHeight;
	if (width % 4)
		width += 4 - width % 4;
	if (height % 4)
		height += 4 - height % 4;
	assert((int)width*height*3 == info_header.biSizeImage && info_header.biBitCount == 24);
	for (int i = 0; i < width*height; i++)
		fread(&rgbs_origin[i], sizeof(RGBColor), 1, reader);
	fclose(reader);

	FILE *writer = fopen("grayout.bmp", "wb");
	fwrite(&file_header, sizeof(BMPFileHeader), 1, writer);
	fwrite(&info_header, sizeof(BMPInfoHeader), 1, writer);
	for (int i = 0; i < width*height; i++) {
		unsigned char res = (306 * rgbs_origin[i].R + 601 * rgbs_origin[i].G + 
			117 * rgbs_origin[i].B) >> 10;
		rgbs_gray[i].B = rgbs_gray[i].G = rgbs_gray[i].R = res;
		fwrite(&rgbs_gray[i], sizeof(RGBColor), 1, writer);
	}
	fclose(writer);

	writer = fopen("out256.bmp", "wb");
	file_header.bfSize = 54 + 1024 + width * height;
	file_header.bfOffBits = 54 + 1024;
	info_header.biBitCount = 8;
	info_header.biSizeImage = width * height;
	info_header.biClrUsed = 256;
	fwrite(&file_header, sizeof(BMPFileHeader), 1, writer);
	fwrite(&info_header, sizeof(BMPInfoHeader), 1, writer);
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			for (int k = 0; k < 4; k++) {
				palettes[32 * i + 4 * j + k].B = i * 32;
				palettes[32 * i + 4 * j + k].G = j * 32;
				palettes[32 * i + 4 * j + k].R = k * 64;
				palettes[32 * i + 4 * j + k].Reversed = 0;
			}
		}
	}
	for (int i = 0; i < 256; i++)
		fwrite(&palettes[i], sizeof(ColorPalette), 1, writer);
	memset(idxs, 0, sizeof(idxs));
	for (int i = 0; i < width*height; i++) {
		unsigned char c1 = 0, c2 = 0, c3 = 0;
		c1 = ((int)rgbs_origin[i].B + 16) / 32;
		if (c1 > 7) c1 = 7;
		c2 = ((int)rgbs_origin[i].G + 16) / 32;
		if (c2 > 7) c2 = 7;
		c3 = ((int)rgbs_origin[i].R + 32) / 64;
		if (c3 > 3) c3 = 3;
		idxs[i] = 32 * c1 + 4 * c2 + c3;
	}
	fwrite(idxs, width*height, 1, writer);
	fclose(writer);

	writer = fopen("grayout256.bmp", "wb");
	fwrite(&file_header, sizeof(BMPFileHeader), 1, writer);
	fwrite(&info_header, sizeof(BMPInfoHeader), 1, writer);
	for (int i = 0; i < 256; i++) {
		gray_palettes[i].B = gray_palettes[i].G = gray_palettes[i].R = i;
		gray_palettes[i].Reversed = 0;
		fwrite(&gray_palettes[i], sizeof(ColorPalette), 1, writer);
	}
	memset(idxs, 0, sizeof(idxs));
	for (int i = 0; i < width*height; i++)
		idxs[i] = (306 * rgbs_origin[i].R + 601 * rgbs_origin[i].G +
			117 * rgbs_origin[i].B) / 1024;
	fwrite(idxs, width*height, 1, writer);
	fclose(writer);

	Mat img_origin = imread("origin.bmp");
	Mat img_out256 = imread("out256.bmp");
	Mat img_outgray = imread("grayout.bmp");
	Mat img_outgray256 = imread("grayout256.bmp");
	imshow("origin", img_origin);
	imshow("out256", img_out256);
	imshow("grayout", img_outgray);
	imshow("grayout256", img_outgray256);
	waitKey();
	return 0;
}