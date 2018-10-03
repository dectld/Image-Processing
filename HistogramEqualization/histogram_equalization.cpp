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
}rgbs_origin[maxn], rgbs_processed[maxn];

int grays[256];
BYTE transform_func[256];

void get_transform_func(int n_nums) {
	memset(transform_func, 0, sizeof(transform_func));
	double sum = 0;
	for (int i = 0; i < 256; i++) {
		sum += grays[i];
		transform_func[i] = (BYTE)(sum / n_nums * 255);
	}
}

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
	assert((int)width*height * 3 == info_header.biSizeImage&&info_header.biBitCount == 24);
	memset(grays, 0, sizeof(grays));
	int idx = 0;
	for (int i = 0; i < width*height; i++) {
		fread(&rgbs_origin[i], sizeof(RGBColor), 1, reader);
		idx = rgbs_origin[i].R;
		grays[idx]++;
	}
	fclose(reader);

	get_transform_func(width*height);

	FILE *writer = fopen("out.bmp", "wb");
	fwrite(&file_header, sizeof(BMPFileHeader), 1, writer);
	fwrite(&info_header, sizeof(BMPInfoHeader), 1, writer);
	for (int i = 0; i < width*height; i++) {
		idx = rgbs_origin[i].B;
		rgbs_processed[i].B = rgbs_processed[i].G = rgbs_processed[i].R = transform_func[idx];
		fwrite(&rgbs_processed[i], sizeof(RGBColor), 1, writer);
	}
	fclose(writer);

	Mat img_origin = imread("origin.bmp");
	Mat img_out = imread("out.bmp");
	imshow("origin", img_origin);
	imshow("out", img_out);
	waitKey();
	//system("pause");
	return 0;
}