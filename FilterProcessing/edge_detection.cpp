#include <cstdio>
#include <cstdlib>
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

const int maxn = 2000;
struct RGBColor {
	BYTE B;
	BYTE G;
	BYTE R;
}rgbs_origin[maxn][maxn], rgbs_gray[maxn][maxn], G_x[maxn][maxn], G_y[maxn][maxn];

struct ColorPalette {
	BYTE B;
	BYTE G;
	BYTE R;
	BYTE Reversed;
}palettes[256], gray_palettes[256];

int Sobel_x[9] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
int Sobel_y[9] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };

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
	assert((int)width*height * 3 == info_header.biSizeImage && info_header.biBitCount == 24);
	for (int i = 0; i < height; i++){
		for (int j = 0; j < width; j++) {
			fread(&rgbs_origin[i][j], sizeof(RGBColor), 1, reader);
			rgbs_gray[i][j].B = rgbs_gray[i][j].G = rgbs_gray[i][j].R =
				(306 * rgbs_origin[i][j].R + 601 * rgbs_origin[i][j].G + 117 * rgbs_origin[i][j].B)>>10;
		}
	}
	fclose(reader);

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (i == 0 || i == height - 1 || j == 0 || j == width - 1) {
				G_x[i][j].B = G_x[i][j].G = G_x[i][j].R = rgbs_gray[i][j].G;
				G_y[i][j].B = G_y[i][j].G = G_y[i][j].R = rgbs_gray[i][j].G;
			}
			else {
				double sum_x = 0, sum_y = 0;
				for (int m = -1; m <= 1; m++) {
					for (int n = -1; n <= 1; n++) {
						sum_x += Sobel_x[(m + 1) * 3 + n + 1] * (int)rgbs_gray[i + m][j + n].B;
						sum_y += Sobel_y[(m + 1) * 3 + n + 1] * (int)rgbs_gray[i + m][j + n].B;
					}
				}
				G_x[i][j].B = G_x[i][j].G = G_x[i][j].R = (BYTE)(sum_x / 4);
				G_y[i][j].B = G_y[i][j].G = G_y[i][j].R = (BYTE)(sum_y / 4);
			}
		}
	}

	FILE *writer = fopen("grayout.bmp", "wb");
	fwrite(&file_header, sizeof(BMPFileHeader), 1, writer);
	fwrite(&info_header, sizeof(BMPInfoHeader), 1, writer);
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			fwrite(&rgbs_gray[i][j], sizeof(RGBColor), 1, writer);
	fclose(writer);

	writer = fopen("GXout.bmp", "wb");
	fwrite(&file_header, sizeof(BMPFileHeader), 1, writer);
	fwrite(&info_header, sizeof(BMPInfoHeader), 1, writer);
	for (int i = 0; i < height; i++) 
		for (int j = 0; j < width; j++)
			fwrite(&G_x[i][j], sizeof(RGBColor), 1, writer);
	fclose(writer);

	writer = fopen("GYout.bmp", "wb");
	fwrite(&file_header, sizeof(BMPFileHeader), 1, writer);
	fwrite(&info_header, sizeof(BMPInfoHeader), 1, writer);
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			fwrite(&G_y[i][j], sizeof(RGBColor), 1, writer);
	fclose(writer);

	Mat img_origin = imread("origin.bmp");
	Mat img_gray = imread("grayout.bmp");
	Mat img_GX = imread("GXout.bmp");
	Mat img_GY = imread("GYout.bmp");
	imshow("origin", img_origin);
	imshow("gray", img_gray);
	imshow("GX", img_GX);
	imshow("GY", img_GY);
	waitKey();
	return 0;
}