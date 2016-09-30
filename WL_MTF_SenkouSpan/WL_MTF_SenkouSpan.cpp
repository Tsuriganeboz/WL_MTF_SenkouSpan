#define	_CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <time.h>

#include "IndicatorInterfaceUnit.h"
#include "TechnicalFunctions.h"

//--- 外部変数
int g_tenkanSenPeriod = 9;
int g_kijunSenPeriod = 26;
int g_senkouSpanPeriod = 52;

int g_timeFrame = 0;			// タイムフレーム
int g_shiftPeriod = 26;

//---
int g_barsLimit = 21 * 24 * 60 / 5;

//---
char timeBuf1[50];
char timeBuf2[50];

// バッファ
TIndexBuffer g_indSenkouSpanA;
TIndexBuffer g_indSenkouSpanB;

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
//void WL_CheckDateTime(double dateTime, char* timeBuf, int bufSiz)
//{
//	time_t time; // define structure of type time_t, c++ base date is 01/01/1970 (25569 days after FT start date of 30/12/1899) 
//	time = (time_t) ((dateTime - 25569) * 86400);
//	struct tm* timeinfo;
//	timeinfo = gmtime(&time); 
//	
//	int year = 1900 + timeinfo->tm_year;
//	int month = 1 + timeinfo->tm_mon;
//	int day = timeinfo->tm_mday;
//
//	double timeOnly = dateTime - (int) dateTime;
//	int hour = (int) (timeOnly * 24);
//	int min = (int) ((timeOnly * 24 - hour) * 60);
//
//	sprintf_s(timeBuf, BUFSIZ, "%04d/%02d/%02d %02d:%02d",
//		year, month, day,
//		hour, min
//		);
//}

//+------------------------------------------------------------------+
//| 初期処理　                                                          |
//+------------------------------------------------------------------+
void WL_MTFCalcIchimokuPrepare(char* symbol, int timeFrame, int index,
	double& tenkanSenHigh, double& tenkanSenLow,
	double& kijunSenHigh, double& kijunSenLow,
	double& senkouSpanHigh, double& senkouSpanLow)
{
#if !0
	tenkanSenHigh = iHigh(symbol, timeFrame, iHighest(symbol, timeFrame, MODE_HIGH, g_tenkanSenPeriod, index));
	tenkanSenLow = iLow(symbol, timeFrame, iLowest(symbol, timeFrame, MODE_LOW, g_tenkanSenPeriod, index));

	kijunSenHigh = iHigh(symbol, timeFrame, iHighest(symbol, timeFrame, MODE_HIGH, g_kijunSenPeriod, index));
	kijunSenLow = iLow(symbol, timeFrame, iLowest(symbol, timeFrame, MODE_LOW, g_kijunSenPeriod, index));

	senkouSpanHigh = iHigh(symbol, timeFrame, iHighest(symbol, timeFrame, MODE_HIGH, g_senkouSpanPeriod, index));
	senkouSpanLow = iLow(symbol, timeFrame, iLowest(symbol, timeFrame, MODE_LOW, g_senkouSpanPeriod, index));
#else
	double h = iHigh(symbol, timeFrame, index);
	double l = iLow(symbol, timeFrame, index);

	for (int i = (index + 1); i < (index + g_senkouSpanPeriod); i++)
	{
		if (iHigh(symbol, timeFrame, i) > h)
			h = iHigh(symbol, timeFrame, i);

		if (iLow(symbol, timeFrame, i) < l)
			l = iLow(symbol, timeFrame, i);

		if (i == (index + g_tenkanSenPeriod) - 1)
		{
			tenkanSenHigh = h;
			tenkanSenLow = l;
		}
		else {}

		if (i == (index + g_kijunSenPeriod) - 1)
		{
			kijunSenHigh = h;
			kijunSenLow = l;
		}
		else {}
	}

	senkouSpanHigh = h;
	senkouSpanLow = l;
#endif
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
double WL_MTFCalcTenkanSen(double tenkanSenHigh, double tenkanSenLow)
{
	return (tenkanSenHigh + tenkanSenLow) / 2;
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
double WL_MTFCalcKijunSen(double kijunSenHigh, double& kijunSenLow)
{
	return (kijunSenHigh + kijunSenLow) / 2;
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
double WL_MTFCalcSenkouSpanA(double tenkanSen, double kijunSen)
{
	return (tenkanSen + kijunSen) / 2;
}

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
double WL_MTFCalcSenkouSpanB(double senkouSpanHigh, double senkouSpanLow)
{
	return (senkouSpanHigh + senkouSpanLow) / 2;
}

//+------------------------------------------------------------------+
//| 　　　　　　　　                                                         |
//+------------------------------------------------------------------+
void WL_MTFCalcIchimokuSenkouSpan(char* symbol, int timeFrame, int index,
	double& senkouSpanA, double& senkouSpanB)
{
	double tenkanSenHigh = 0;
	double tenkanSenLow = 0;
	double kijunSenHigh = 0;
	double kijunSenLow = 0;
	double senkouSpanHigh = 0;
	double senkouSpanLow = 0;

	WL_MTFCalcIchimokuPrepare(symbol, timeFrame, index,
		tenkanSenHigh, tenkanSenLow,
		kijunSenHigh, kijunSenLow,
		senkouSpanHigh, senkouSpanLow);


	double tenkanSen = WL_MTFCalcTenkanSen(tenkanSenHigh, tenkanSenLow);
	double kijunSen = WL_MTFCalcKijunSen(kijunSenHigh, kijunSenLow);

	senkouSpanA = WL_MTFCalcSenkouSpanA(tenkanSen, kijunSen);
	senkouSpanB = WL_MTFCalcSenkouSpanB(senkouSpanHigh, senkouSpanLow);
}


//---------------------------------------------------------------------------
// Initialize indicator
//---------------------------------------------------------------------------
EXPORT void __stdcall Init()
{
	IndicatorShortName("MTF Senkou Span");
	SetOutputWindow(ow_ChartWindow);

	// register options
	//AddSeparator("Common");

	RegOption("Tenkan-sen period", ot_Integer, &g_tenkanSenPeriod);
	SetOptionRange("Tenkan-sen period", 1, MaxInt);
	g_tenkanSenPeriod = 9;

	RegOption("Kijun-sen period", ot_Integer, &g_kijunSenPeriod);
	SetOptionRange("Kijun-sen period", 1, MaxInt);
	g_kijunSenPeriod = 26;

	RegOption("Senkou Span period", ot_Integer, &g_senkouSpanPeriod);
	SetOptionRange("Senkou Span period", 1, MaxInt);
	g_senkouSpanPeriod = 52;

	RegOption("Time Frame", ot_TimeFrame, &g_timeFrame);
	g_timeFrame = PERIOD_M5;

	RegOption("Shift period", ot_Integer, &g_shiftPeriod);
	SetOptionRange("Shift period", 1, MaxInt);
	g_shiftPeriod = 26;


	g_indSenkouSpanA = CreateIndexBuffer();
	g_indSenkouSpanB = CreateIndexBuffer();


	IndicatorBuffers(2);
	SetIndexBuffer(0, g_indSenkouSpanA);
	SetIndexBuffer(1, g_indSenkouSpanB);

	SetIndexStyle(0, ds_Line, psSolid, 5, RGB(0xFF, 0x00, 0x00));
	SetIndexLabel(0, "Senkou Span A");
	SetIndexStyle(1, ds_Line, psSolid, 5, RGB(0x00, 0x00, 0xFF));
	SetIndexLabel(1, "Senkou Span B");

}

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
EXPORT void __stdcall OnParamsChange()
{
	SetBufferShift(0, g_shiftPeriod * (g_timeFrame / Timeframe()));
	SetBufferShift(1, g_shiftPeriod * (g_timeFrame / Timeframe()));
}

//---------------------------------------------------------------------------
// Calculate requested bar
//---------------------------------------------------------------------------
EXPORT void __stdcall Calculate(int index)
{
	if (Timeframe() > g_timeFrame)
		return;

	// 処理高速化のため。
	if (index >= g_barsLimit)
		return;

	double indexTime = Time(index);

	int timeFrameIndex = iBarShift(Symbol(), g_timeFrame, indexTime, false);
	if (timeFrameIndex == -1)
		return;

	double senkouSpanA = 0;
	double senkouSpanB = 0;
	WL_MTFCalcIchimokuSenkouSpan(Symbol(), g_timeFrame, timeFrameIndex, senkouSpanA, senkouSpanB);

	g_indSenkouSpanA[index] = senkouSpanA;
	g_indSenkouSpanB[index] = senkouSpanB;
}
