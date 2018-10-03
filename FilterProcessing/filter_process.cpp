#include <cstdio>
#include <cstdlib>
#include <algorithm>
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
}rgbs_origin[maxn][maxn], rgbs_medium[maxn][maxn], rgbs_mean[maxn][maxn];

BYTE rgbs_cache[3][9];

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
	assert(width*height * 3 == info_header.biSizeImage&&info_header.biBitCount == 24);
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			fread(&rgbs_origin[i][j], sizeof(RGBColor), 1, reader);
	fclose(reader);

	FILE *writer = fopen("mediumout.bmp", "wb");
	fwrite(&file_header, sizeof(BMPFileHeader), 1, writer);
	fwrite(&info_header, sizeof(BMPInfoHeader), 1, writer);
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (i == 0 || i == height - 1 || j == 0 || j == width - 1) {
				rgbs_medium[i][j].B = rgbs_origin[i][j].B;
				rgbs_medium[i][j].G = rgbs_origin[i][j].G;
				rgbs_medium[i][j].R = rgbs_origin[i][j].R;
				rgbs_mean[i][j].B = rgbs_origin[i][j].B;
				rgbs_mean[i][j].G = rgbs_origin[i][j].G;
				rgbs_mean[i][j].R = rgbs_origin[i][j].R;
			}
			else {
				double mean_B = 0, mean_G = 0, mean_R = 0;
				for (int m = -1; m <= 1; m++) {
					for (int n = -1; n <= 1; n++) {
						rgbs_cache[0][(m + 1) * 3 + (n + 1)] = rgbs_origin[i + m][j + n].B;
						rgbs_cache[1][(m + 1) * 3 + (n + 1)] = rgbs_origin[i + m][j + n].G;
						rgbs_cache[2][(m + 1) * 3 + (n + 1)] = rgbs_origin[i + m][j + n].R;
						mean_B += rgbs_origin[i + m][j + n].B;
						mean_G += rgbs_origin[i + m][j + n].G;
						mean_R += rgbs_origin[i + m][j + n].R;
					}
				}
				for (int k = 0; k < 3; k++)
					sort(rgbs_cache[k], rgbs_cache[k] + 9);
				rgbs_medium[i][j].B = rgbs_cache[0][4];
				rgbs_medium[i][j].G = rgbs_cache[1][4];
				rgbs_medium[i][j].R = rgbs_cache[2][4];
				rgbs_mean[i][j].B = (BYTE)(mean_B / 9);
				rgbs_mean[i][j].G = (BYTE)(mean_G / 9);
				rgbs_mean[i][j].R = (BYTE)(mean_R / 9);
				//printf("%u %u %u\n", rgbs_mean[i][j].B, rgbs_mean[i][j].G, rgbs_mean[i][j].R);
			}
			fwrite(&rgbs_medium[i][j], sizeof(RGBColor), 1, writer);
		}
	}
	fclose(writer);

	writer = fopen("meanout.bmp", "wb");
	fwrite(&file_header, sizeof(BMPFileHeader), 1, writer);
	fwrite(&info_header, sizeof(BMPInfoHeader), 1, writer);
	for (int i = 0; i < height; i++) 
		for (int j = 0; j < width; j++) 
			fwrite(&rgbs_mean[i][j], sizeof(RGBColor), 1, writer);
	fclose(writer);
	Mat img_origin = imread("origin.bmp");
	Mat img_medium = imread("mediumout.bmp");
	Mat img_mean = imread("meanout.bmp");
	imshow("origin", img_origin);
	imshow("medium", img_medium);
	imshow("mean", img_mean);
	waitKey();
	//system("pause");
	return 0;
}