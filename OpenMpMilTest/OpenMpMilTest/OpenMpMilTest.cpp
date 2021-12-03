// OpenMpMilTest.cpp: 主要專案檔。

#include "stdafx.h"
#include <omp.h>
#include "mil.h"

using namespace System;
using namespace System::Diagnostics;

#define IMG_PATH MIL_TEXT("D:\\VirtualBox_Share\\Image1.tif")

void GoldenMaker(MIL_ID sys, MIL_ID input, int resizeCount);

MIL_ID milApplication;
MIL_ID milSystem;

MIL_ID oriImage00;
MIL_ID oriImage01;
MIL_ID oriImage02;
MIL_ID oriImage03;
MIL_ID oriImage04;

int main(array<System::String ^> ^args)
{
	milApplication = M_NULL;
	milSystem = M_NULL;

	oriImage00 = M_NULL;
	oriImage01 = M_NULL;
	oriImage02 = M_NULL;
	oriImage03 = M_NULL;
	oriImage04 = M_NULL;

	MIL_ID dscImage01 = M_NULL;

	MIL_ID* imageAry = new MIL_ID[4];
	int* resizeCountAry = new int[4];

	MIL_INT sizeX = 0;
	MIL_INT sizeY = 0;
	MIL_INT sizeBit = 0;

	Stopwatch^ sw = gcnew Stopwatch();

	MappAlloc(M_DEFAULT, &milApplication);
	MsysAlloc(MIL_TEXT("M_SYSTEM_HOST"), M_DEFAULT, M_DEFAULT, &milSystem);

	MbufDiskInquire(IMG_PATH, M_SIZE_X, &sizeX);
	MbufDiskInquire(IMG_PATH, M_SIZE_Y, &sizeY);
	MbufDiskInquire(IMG_PATH, M_SIZE_BIT, &sizeBit);

	oriImage00 = MbufAlloc2d(milSystem, sizeX, sizeY, sizeBit + M_UNSIGNED, M_IMAGE + M_PROC, M_NULL);
	MbufLoad(IMG_PATH, oriImage00);

	oriImage01 = MbufAlloc2d(milSystem, sizeX, sizeY, sizeBit + M_UNSIGNED, M_IMAGE + M_PROC, M_NULL);
	oriImage02 = MbufAlloc2d(milSystem, sizeX, sizeY, sizeBit + M_UNSIGNED, M_IMAGE + M_PROC, M_NULL);
	oriImage03 = MbufAlloc2d(milSystem, sizeX, sizeY, sizeBit + M_UNSIGNED, M_IMAGE + M_PROC, M_NULL);
	oriImage04 = MbufAlloc2d(milSystem, sizeX, sizeY, sizeBit + M_UNSIGNED, M_IMAGE + M_PROC, M_NULL);
	dscImage01 = MbufAlloc2d(milSystem, sizeX, sizeY, sizeBit + M_UNSIGNED, M_IMAGE + M_PROC, M_NULL);

	imageAry[0] = oriImage01;
	imageAry[1] = oriImage02;
	imageAry[2] = oriImage03;
	imageAry[3] = oriImage04;

	resizeCountAry[0] = 0;
	resizeCountAry[1] = 3;
	resizeCountAry[2] = 3;
	resizeCountAry[3] = 5;

	for (int n = 0; n < 5; n++)
	{
		MbufCopy(oriImage00, oriImage01);
		MbufCopy(oriImage00, oriImage02);
		MbufCopy(oriImage00, oriImage03);
		MbufCopy(oriImage00, oriImage04);
		MbufCopy(oriImage00, dscImage01);

		sw->Reset();
		sw->Start();

		//GoldenMaker(milSystem, imageAry[0], resizeCountAry[0]);
		//GoldenMaker(milSystem, imageAry[1], resizeCountAry[1]);
		//GoldenMaker(milSystem, imageAry[2], resizeCountAry[2]);
		//GoldenMaker(milSystem, imageAry[3], resizeCountAry[3]);

		//#pragma omp parallel for
		for (int i = 0; i < 4; i++)
		{
			GoldenMaker(milSystem, imageAry[i], resizeCountAry[i]);
		}

		//MimConvolve(imageAry[0], imageAry[0], M_SMOOTH + M_OVERSCAN_ENABLE);

		sw->Stop();
		Console::WriteLine("Time :" + sw->ElapsedMilliseconds + " ms");
		Console::WriteLine("----------------------------------------\n");
	}

	MosPrintf(MIL_TEXT("Press <ENTER> to continue\n"));
	MosGetch();

	imageAry[0] = M_NULL;
	imageAry[1] = M_NULL;
	imageAry[2] = M_NULL;
	imageAry[3] = M_NULL;
	delete[] imageAry;

	MbufFree(dscImage01);
	MbufFree(oriImage04);
	MbufFree(oriImage03);
	MbufFree(oriImage02);
	MbufFree(oriImage01);
	MbufFree(oriImage00);
	MsysFree(milSystem);
	MappFree(milApplication);

	return 0;
}

void GoldenMaker(MIL_ID sys, MIL_ID input, int resizeCount)
{
	Stopwatch^ sw = gcnew Stopwatch();

	MIL_ID resizeImage = M_NULL;

	MIL_INT sizeX = 0;
	MIL_INT sizeY = 0;
	MIL_INT sizeBit = 0;
	MIL_INT resizeSizeX = 0;
	MIL_INT resizeSizeY = 0;

	double resizeRatio = 0.0;

	sizeX = MbufInquire(input, M_SIZE_X, M_NULL);
	sizeY = MbufInquire(input, M_SIZE_Y, M_NULL);
	sizeBit = MbufInquire(input, M_SIZE_BIT, M_NULL);
	resizeRatio = System::Math::Pow(0.5, resizeCount);
	resizeSizeX = (MIL_INT)(resizeRatio * sizeX);
	resizeSizeY = (MIL_INT)(resizeRatio * sizeY);

	sw->Reset();
	sw->Start();

	resizeImage = MbufAlloc2d(sys, resizeSizeX, resizeSizeY, sizeBit + M_UNSIGNED, M_IMAGE + M_PROC, M_NULL);
	MbufClear(resizeImage, 0L);
	//MbufSave(MIL_TEXT("D:\\Test\\resizeImage0.tif"), resizeImage);

	MimResize(input, resizeImage, M_FILL_DESTINATION, M_FILL_DESTINATION, M_BILINEAR + M_OVERSCAN_DISABLE);
	MimConvolve(resizeImage, resizeImage, M_SMOOTH);
	//MbufSave(MIL_TEXT("D:\\Test\\resizeImage1.tif"), resizeImage);

	MimResize(resizeImage, input, M_FILL_DESTINATION, M_FILL_DESTINATION, M_BILINEAR + M_OVERSCAN_DISABLE);

	MbufFree(resizeImage);

	sw->Stop();
	Console::WriteLine("GoldenMaker Time :" + sw->ElapsedMilliseconds + " ms");
}
