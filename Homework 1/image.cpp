#include "stdafx.h"
#include "image.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <math.h>
using namespace std;

/**
 * Image
 **/
Image::Image (int width_, int height_){

    assert(width_ > 0);
    assert(height_ > 0);

    width           = width_;
    height          = height_;
    num_pixels      = width * height;
    sampling_method = IMAGE_SAMPLING_POINT;
    
    data.raw = new uint8_t[num_pixels*4];
	int b = 0; //which byte to write to
	for (int j = 0; j < height; j++){
		for (int i = 0; i < width; i++){
			data.raw[b++] = 0;
			data.raw[b++] = 0;
			data.raw[b++] = 0;
			data.raw[b++] = 0;
		}
	}

    assert(data.raw != NULL);
}

Image::Image (const Image& src){
	
	width           = src.width;
    height          = src.height;
    num_pixels      = width * height;
    sampling_method = IMAGE_SAMPLING_POINT;
    
    data.raw = new uint8_t[num_pixels*4];
    
    //memcpy(data.raw, src.data.raw, num_pixels);
    *data.raw = *src.data.raw;
}

Image::Image (char* fname){

	int numComponents; //(e.g., Y, YA, RGB, or RGBA)
	data.raw = stbi_load(fname, &width, &height, &numComponents, 4);
	
	if (data.raw == NULL){
		printf("Error loading image: %s", fname);
		exit(-1);
	}
	

	num_pixels = width * height;
	sampling_method = IMAGE_SAMPLING_POINT;
	
}

Image::~Image (){
    delete data.raw;
    data.raw = NULL;
}

void Image::Write(char* fname){
	
	int lastc = strlen(fname);

	switch (fname[lastc-1]){
	   case 'g': //jpeg (or jpg) or png
	     if (fname[lastc-2] == 'p' || fname[lastc-2] == 'e') //jpeg or jpg
	        stbi_write_jpg(fname, width, height, 4, data.raw, 95);  //95% jpeg quality
	     else //png
	        stbi_write_png(fname, width, height, 4, data.raw, width*4);
	     break;
	   case 'a': //tga (targa)
	     stbi_write_tga(fname, width, height, 4, data.raw);
	     break;
	   case 'p': //bmp
	   default:
	     stbi_write_bmp(fname, width, height, 4, data.raw);
	}
}

void Image::AddNoise (double factor)
{
	int noise = factor * Width() * Height() / 500;
	for (int x = 0; x < noise; x++)
	{
		int x_noise, y_noise;
		x_noise = rand() % (Width() - 1);
		y_noise = rand() % (Height() - 1);
		GetPixel(x_noise, y_noise).r = 0;
		GetPixel(x_noise, y_noise).g = 0;
		GetPixel(x_noise, y_noise).b = 0;
	}
}

void Image::Brighten (double factor)
{
	int x,y;
	for (x = 0 ; x < Width() ; x++)
	{
		for (y = 0 ; y < Height() ; y++)
		{
			Pixel p = GetPixel(x, y);
			Pixel scaled_p = p*factor;
			GetPixel(x,y) = scaled_p;
		}
	}
}


void Image::ChangeContrast (double factor)
{
	int x, y;
	double totalLuminance, averageLuminance;
	totalLuminance = 0;
	for (x = 0; x < Width(); x++)
	{
		for (y = 0; y < Height(); y++)
		{
			Pixel p = GetPixel(x, y);
			totalLuminance = totalLuminance + p.Luminance();
		}
	}
	averageLuminance = totalLuminance / (Width() * Height());
	for (x = 0; x < Width(); x++)
	{
		for (y = 0; y < Height(); y++)
		{
			Pixel p = GetPixel(x, y);
			if (p.Luminance() > averageLuminance) 
			{
				GetPixel(x, y) = p * ((factor * (p.Luminance() - averageLuminance) + averageLuminance) / p.Luminance());
			}
			else if (p.Luminance() < averageLuminance) 
			{
				GetPixel(x, y) = p * (((1 - factor) * (-p.Luminance() + averageLuminance) + p.Luminance()) / p.Luminance());
			}
		}
	}
}


void Image::ChangeSaturation(double factor)
{
	int x, y;
	for (x = 0; x < Width(); x++)
	{
		for (y = 0; y < Height(); y++)
		{
			float rgbList[3];
			char rgb[3];
			rgbList[0] = GetPixel(x, y).r;
			rgbList[1] = GetPixel(x, y).g;
			rgbList[2] = GetPixel(x, y).b;
			rgb[0] = 'r';
			rgb[1] = 'g';
			rgb[2] = 'b';
			for(int i = 0; i < 2; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					float temp;
					if (rgbList[j] < rgbList[j + 1]) 
					{
						temp = rgbList[j];
						rgbList[j] = rgbList[j + 1];
						rgbList[j + 1] = temp;
						temp = rgb[j];
						rgb[j] = rgb[j + 1];
						rgb[j + 1] = temp;
					}
				}
			}
			rgbList[0] = (rgbList[0] - rgbList[1]) * factor + rgbList[1];
			rgbList[2] = (rgbList[2] - rgbList[1]) * factor + rgbList[1];
			for (int i = 0; i < 3; i++) 
			{
				switch(rgb[i]) 
				{
				case 'r':
					GetPixel(x, y).r = rgbList[i];
					break;
				case 'g':
					GetPixel(x, y).g = rgbList[i];
					break;
				case 'b':
					GetPixel(x, y).b = rgbList[i];
					break;
				default:
					break;
				}
			}
		}
	}
}


Image* Image::Crop(int x, int y, int w, int h)
{

	if((x + w) > Width())
	{
		w = Width() - x;
	}
	if ((y + h) > Height())
	{
		h = Height() - y;
	}

	Image* crop;
	crop = new Image(w, h);
	

	for (int m = 0; m < w; m++)
	{
		for (int n = 0; n < h; n++)
		{
			crop->GetPixel(m, n) = GetPixel(m + x, n + y);
		}
	}

	return crop;
}


void Image::ExtractChannel(int channel)
{
	int x, y;
	for (x = 0; x < Width(); x++)
	{
		for (y = 0; y < Height(); y++)
		{
			switch (channel) 
			{
			case 'r':
				GetPixel(x, y).b = 0;
				GetPixel(x, y).g = 0;
				break;
			case 'g':
				GetPixel(x, y).b = 0;
				GetPixel(x, y).r = 0;
				break;
			case 'b':
				GetPixel(x, y).g = 0;
				GetPixel(x, y).r = 0;
				break;
			default:
				break;
			}
		}
	}
}


void Image::Quantize (int nbits)
{
	int bit;
	bit = 256 / pow(2, nbits);
	for (int x = 0; x < Width(); x++)
	{
		for (int y = 0; y < Height(); y++)
		{
			int quotient, remainder;
			quotient = (GetPixel(x, y).r + 1) / bit;
			remainder = (GetPixel(x, y).r + 1) % bit;
			if (remainder < (256 / bit / 2)) 
			{
				if ((quotient * bit - 1) < 0) {
					GetPixel(x, y).r = 0;
				}
				else {
					GetPixel(x, y).r = quotient * bit - 1;
				}
			}
			else
			{
				if (((quotient + 1) * bit - 1) > 255) {
					GetPixel(x, y).r = 255;
				}
				else {
					GetPixel(x, y).r = (quotient + 1) * bit - 1;
				}
			}
			quotient = (GetPixel(x, y).g + 1) / bit;
			remainder = (GetPixel(x, y).g + 1) % bit;
			if (remainder < (256 / bit / 2))
			{
				if ((quotient * bit - 1) < 0) {
					GetPixel(x, y).g = 0;
				}
				else {
					GetPixel(x, y).g = quotient * bit - 1;
				}
			}
			else
			{
				if (((quotient + 1) * bit - 1) > 255) {
					GetPixel(x, y).g = 255;
				}
				else {
					GetPixel(x, y).g = (quotient + 1) * bit - 1;
				}
			}
			quotient = (GetPixel(x, y).b + 1) / bit;
			remainder = (GetPixel(x, y).b + 1) % bit;
			if (remainder < (256 / bit / 2))
			{
				if ((quotient * bit - 1) < 0) {
					GetPixel(x, y).b = 0;
				}
				else {
					GetPixel(x, y).b = quotient * bit - 1;
				}
			}
			else
			{
				if (((quotient + 1) * bit - 1) > 255) {
					GetPixel(x, y).b = 255;
				}
				else {
					GetPixel(x, y).b = (quotient + 1) * bit - 1;
				}
			}
		}
	}
}

void Image::RandomDither (int nbits)
{
	int bit = 256 / pow(2, nbits);
	for (int x = 0; x < Width(); x++)
	{
		for (int y = 0; y < Height(); y++)
		{
			int randomNumber, quotient;
			randomNumber = rand() % 255;
			quotient = (GetPixel(x, y).r + 1) / bit;
			if (GetPixel(x, y).r > randomNumber) {
				if (((quotient + 1) * bit - 1) > 255) {
					GetPixel(x, y).r = 255;
				}
				else{
					GetPixel(x, y).r = (quotient + 1) * bit - 1;
				}
			}
			else {
				if ((quotient * bit - 1) < 0) {
					GetPixel(x, y).r = 0;
				}
				else {
					GetPixel(x, y).r = quotient * bit - 1;
				}
			}
			quotient = (GetPixel(x, y).g + 1) / bit;
			if (GetPixel(x, y).g > randomNumber) {
				if (((quotient + 1) * bit - 1) > 255) {
					GetPixel(x, y).g = 255;
				}
				else {
					GetPixel(x, y).g = (quotient + 1) * bit - 1;
				}
			}
			else {
				if ((quotient * bit - 1) < 0) {
					GetPixel(x, y).g = 0;
				}
				else {
					GetPixel(x, y).g = quotient * bit - 1;
				}
			}
			quotient = (GetPixel(x, y).b + 1) / bit;
			if (GetPixel(x, y).b > randomNumber) {
				if (((quotient + 1) * bit - 1) > 255) {
					GetPixel(x, y).b = 255;
				}
				else {
					GetPixel(x, y).b = (quotient + 1) * bit - 1;
				}
			}
			else {
				if ((quotient * bit - 1) < 0) {
					GetPixel(x, y).b = 0;
				}
				else {
					GetPixel(x, y).b = quotient * bit - 1;
				}
			}
		}
	}
}


static int Bayer4[4][4] =
{
    {15,  7, 13,  5},
    { 3, 11,  1,  9},
    {12,  4, 14,  6},
    { 0,  8,  2, 10}
};


void Image::OrderedDither(int nbits)
{
	float bit;
	if (nbits == 1) {
		bit = 256 / (pow(2, nbits) - 1);
	}
	else {
		bit = 256 / (pow(2, nbits) - 1);
	}
	for (int x = 0; x < Width(); x++)
	{
		for (int y = 0; y < Height(); y++)
		{
			int i, j;
			float quotient;
			i = x % 4;
			j = y % 4;
			if (GetPixel(x, y).r >= (Bayer4[i][j] * 16)) {
				quotient = ceil(GetPixel(x, y).r / bit);
			}
			else {
				quotient = floor((GetPixel(x, y).r ) / bit);
			}
			if ((quotient * bit - 1) < 0) {
				GetPixel(x, y).r = 0;
			}
			else {
				GetPixel(x, y).r = quotient * bit - 1;
			}
			

			if (GetPixel(x, y).g >= (Bayer4[i][j] * 16)) {
				quotient = ceil((GetPixel(x, y).g ) / bit);
				cout << quotient << endl;
			}
			else {
				quotient = floor((GetPixel(x, y).g ) / bit);
			}
			if ((quotient * bit - 1) < 0) {
				GetPixel(x, y).g = 0;
			}
			else {
				GetPixel(x, y).g = quotient * bit - 1;
			}

			if (GetPixel(x, y).b >= (Bayer4[i][j] * 16)) {
				quotient = ceil((GetPixel(x, y).b ) / bit);
			}
			else {
				quotient = floor((GetPixel(x, y).b ) / bit);
			}
			if ((quotient * bit - 1) < 0) {
				GetPixel(x, y).b = 0;
			}
			else {
				GetPixel(x, y).b = quotient * bit - 1;
			}
			////remainder = (GetPixel(x, y).r + 1) % bit;
			//if (GetPixel(x, y).r >= (Bayer4[i][j] * 16)) {
			//	if (((quotient + 1) * bit - 1) > 255) {
			//		GetPixel(x, y).r = 255;
			//	}
			//	else {
			//		GetPixel(x, y).r = (quotient + 1) * bit - 1;
			//	}
			//}
			//else {
			//	if ((quotient * bit - 1) < 0) {
			//		GetPixel(x, y).r = 0;
			//	}
			//	else {
			//		GetPixel(x, y).r = quotient * bit - 1;
			//	}
			//}
			//quotient = (GetPixel(x, y).g + 1) / bit;
			////remainder = (GetPixel(x, y).g + 1) % bit;
			//if (GetPixel(x, y).g >= (Bayer4[i][j] * 16)) {
			//	if (((quotient + 1) * bit - 1) > 255) {
			//		GetPixel(x, y).g = 255;
			//	}
			//	else {
			//		GetPixel(x, y).g = (quotient + 1) * bit - 1;
			//	}
			//}
			//else {
			//	if ((quotient * bit - 1) < 0) {
			//		GetPixel(x, y).g = 0;
			//	}
			//	else {
			//		GetPixel(x, y).g = quotient * bit - 1;
			//	}
			//}
			//quotient = (GetPixel(x, y).b + 1) / bit;
			////remainder = (GetPixel(x, y).b + 1) % bit;
			//if (GetPixel(x, y).b >= (Bayer4[i][j] * 16)) {
			//	if (((quotient + 1) * bit - 1) > 255) {
			//		GetPixel(x, y).b = 255;
			//	}
			//	else {
			//		GetPixel(x, y).b = (quotient + 1) * bit - 1;
			//	}
			//}
			//else {
			//	if ((quotient * bit - 1) < 0) {
			//		GetPixel(x, y).b = 0;
			//	}
			//	else {
			//		GetPixel(x, y).b = quotient * bit - 1;
			//	}
			//}
		}
	}
}

/* Error-diffusion parameters */
const double
    ALPHA = 7.0 / 16.0,
    BETA  = 3.0 / 16.0,
    GAMMA = 5.0 / 16.0,
    DELTA = 1.0 / 16.0;

void Image::FloydSteinbergDither(int nbits)
{
	int bit = 256 / pow(2, nbits);
	struct rgb {
		float r, g, b;
	};
	int h = Height();
	int w = Width();
	rgb **floyd = new rgb*[w];
	for (int i = 0; i<w; i++)
	{
		floyd[i] = new rgb[h];
	}
	for (int x = 0; x < Width(); x++)
	{
		for (int y = 0; y < Height(); y++)
		{
			int quotient, remainder;
			quotient = (GetPixel(x, y).r + 1) / bit;
			remainder = (GetPixel(x, y).r + 1) % bit;
			if (remainder < (256 / bit / 2))
			{
				if ((quotient * bit - 1) < 0) {
					floyd[x][y].r = 0;
				}
				else {
					floyd[x][y].r = quotient * bit - 1;
				}
			}
			else
			{
				if (((quotient + 1) * bit - 1) > 255) {
					floyd[x][y].r = 255;
				}
				else {
					floyd[x][y].r = (quotient + 1) * bit - 1;
				}
			}
			quotient = (GetPixel(x, y).g + 1) / bit;
			remainder = (GetPixel(x, y).g + 1) % bit;
			if (remainder < (256 / bit / 2))
			{
				if ((quotient * bit - 1) < 0) {
					floyd[x][y].g = 0;
				}
				else {
					floyd[x][y].g = quotient * bit - 1;
				}
			}
			else
			{
				if (((quotient + 1) * bit - 1) > 255) {
					floyd[x][y].g = 255;
				}
				else {
					floyd[x][y].g = (quotient + 1) * bit - 1;
				}
			}
			quotient = (GetPixel(x, y).b + 1) / bit;
			remainder = (GetPixel(x, y).b + 1) % bit;
			if (remainder < (256 / bit / 2))
			{
				if ((quotient * bit - 1) < 0) {
					floyd[x][y].b = 0;
				}
				else {
					floyd[x][y].b = quotient * bit - 1;
				}
			}
			else
			{
				if (((quotient + 1) * bit - 1) > 255) {
					floyd[x][y].b = 255;
				}
				else {
					floyd[x][y].b = (quotient + 1) * bit - 1;
				}
			}
			int r_error, g_error, b_error;
			r_error = GetPixel(x, y).r - floyd[x][y].r;
			g_error = GetPixel(x, y).g - floyd[x][y].g;
			b_error = GetPixel(x, y).b - floyd[x][y].b;
			GetPixel(x, y).r = floyd[x][y].r;
			GetPixel(x, y).g = floyd[x][y].g;
			GetPixel(x, y).b = floyd[x][y].b;
			if ((x + 1) < w) {
				if ((GetPixel(x + 1, y).r + r_error * ALPHA) > 255) {
					GetPixel(x + 1, y).r = 255;
				}
				else if((GetPixel(x + 1, y).r + r_error * ALPHA) < 0) {
					GetPixel(x + 1, y).r = 0;
				}
				else {
					GetPixel(x + 1, y).r = GetPixel(x + 1, y).r + r_error * ALPHA;
				}
				if ((GetPixel(x + 1, y).g + g_error * ALPHA) > 255) {
					GetPixel(x + 1, y).g = 255;
				}
				else if ((GetPixel(x + 1, y).g + g_error * ALPHA) < 0) {
					GetPixel(x + 1, y).g = 0;
				}
				else {
					GetPixel(x + 1, y).g = GetPixel(x + 1, y).g + g_error * ALPHA;
				}
				if ((GetPixel(x + 1, y).b + b_error * ALPHA) > 255) {
					GetPixel(x + 1, y).b = 255;
				}
				else if ((GetPixel(x + 1, y).b + b_error * ALPHA) < 0) {
					GetPixel(x + 1, y).b = 0;
				}
				else {
					GetPixel(x + 1, y).b = GetPixel(x + 1, y).b + b_error * ALPHA;
				}
			}
			if ((x + 1) < w && (y + 1) < h) {
				if ((GetPixel(x + 1, y + 1).r + r_error * DELTA) > 255) {
					GetPixel(x + 1, y + 1).r = 255;
				}
				else if ((GetPixel(x + 1, y + 1).r + r_error * DELTA) < 0) {
					GetPixel(x + 1, y + 1).r = 0;
				}
				else {
					GetPixel(x + 1, y + 1).r = GetPixel(x + 1, y + 1).r + r_error * DELTA;
				}
				if ((GetPixel(x + 1, y + 1).g + g_error * DELTA) > 255) {
					GetPixel(x + 1, y + 1).g = 255;
				}
				else if ((GetPixel(x + 1, y + 1).g + g_error * DELTA) < 0) {
					GetPixel(x + 1, y + 1).g = 0;
				}
				else {
					GetPixel(x + 1, y + 1).g = GetPixel(x + 1, y + 1).g + g_error * DELTA;
				}
				if ((GetPixel(x + 1, y + 1).b + b_error * DELTA) > 255) {
					GetPixel(x + 1, y + 1).b = 255;
				}
				else if ((GetPixel(x + 1, y + 1).b + b_error * DELTA) < 0) {
					GetPixel(x + 1, y + 1).b = 0;
				}
				else {
					GetPixel(x + 1, y + 1).b = GetPixel(x + 1, y + 1).b + b_error * DELTA;
				}
			}
			if ((x - 1) >= 0 && (y + 1) < h) {
				if ((GetPixel(x - 1, y + 1).r + r_error * BETA) > 255) {
					GetPixel(x - 1, y + 1).r = 255;
				}
				else if ((GetPixel(x - 1, y + 1).r + r_error * BETA) < 0) {
					GetPixel(x - 1, y + 1).r = 0;
				}
				else {
					GetPixel(x - 1, y + 1).r = GetPixel(x - 1, y + 1).r + r_error * BETA;
				}
				if ((GetPixel(x - 1, y + 1).g + g_error * BETA) > 255) {
					GetPixel(x - 1, y + 1).g = 255;
				}
				else if ((GetPixel(x - 1, y + 1).g + g_error * BETA) < 0) {
					GetPixel(x - 1, y + 1).g = 0;
				}
				else {
					GetPixel(x - 1, y + 1).g = GetPixel(x - 1, y + 1).g + g_error * BETA;
				}
				if ((GetPixel(x - 1, y + 1).b + b_error * BETA) > 255) {
					GetPixel(x - 1, y + 1).b = 255;
				}
				else if ((GetPixel(x - 1, y + 1).b + b_error * BETA) < 0) {
					GetPixel(x - 1, y + 1).b = 0;
				}
				else {
					GetPixel(x - 1, y + 1).b = GetPixel(x - 1, y + 1).b + b_error * BETA;
				}
			}
			if ((y + 1) < h) {
				if ((GetPixel(x, y + 1).r + r_error * GAMMA) > 255) {
					GetPixel(x, y + 1).r = 255;
				}
				else if ((GetPixel(x, y + 1).r + r_error * GAMMA) < 0) {
					GetPixel(x, y + 1).r = 0;
				}
				else {
					GetPixel(x, y + 1).r = GetPixel(x, y + 1).r + r_error * GAMMA;
				}
				if ((GetPixel(x, y + 1).g + g_error * GAMMA) > 255) {
					GetPixel(x, y + 1).g = 255;
				}
				else if ((GetPixel(x, y + 1).g + g_error * GAMMA) < 0) {
					GetPixel(x, y + 1).g = 0;
				}
				else {
					GetPixel(x, y + 1).g = GetPixel(x, y + 1).g + g_error * GAMMA;
				}
				if ((GetPixel(x, y + 1).b + b_error * GAMMA) > 255) {
					GetPixel(x, y + 1).b = 255;
				}
				else if ((GetPixel(x, y + 1).b + b_error * GAMMA) < 0) {
					GetPixel(x, y + 1).b = 0;
				}
				else {
					GetPixel(x, y + 1).b = GetPixel(x, y + 1).b + b_error * GAMMA;
				}
			}
		}
	}
}

void Image::Blur(int n)
{
	struct rgb {
		float r, g, b;
	};
	int h = Height();
	int w = Width();
	rgb **blurred = new rgb*[w];
	for(int i=0; i<w; i++)
	{
		blurred[i] = new rgb[h];
	}
	
	for (int x = 0; x < Width(); x++)
	{
		for (int y = 0; y < Height(); y++)
		{
			float rTotal = 0;
			float gTotal = 0;
			float bTotal = 0;
			float weightTotal = 0;
			for (int z = 1; z <= n; z++) //neighbor
			{
				for(int i = x - z; i <= x + z; i++)//top side and bottom side
				{
					if(i >= 0 && i < w && (y - z) >= 0)//inside array
					{
						float temp = 0.3989 * exp(-pow(z, 2) / 2);
						rTotal = rTotal + temp * GetPixel(i, y - z).r;
						gTotal = gTotal + temp * GetPixel(i, y - z).g;
						bTotal = bTotal + temp * GetPixel(i, y - z).b;
						weightTotal = weightTotal + temp;
					}
					if (i >= 0 && i < w && (y + z) < h)//inside array
					{
						float temp = 0.3989 * exp(-pow(z, 2) / 2);
						rTotal = rTotal + temp * GetPixel(i, y + z).r;
						gTotal = gTotal + temp * GetPixel(i, y + z).g;
						bTotal = bTotal + temp * GetPixel(i, y + z).b;
						weightTotal = weightTotal + 0.3989 * exp(-pow(z, 2) / 2);
					}
				}
				for (int i = y - z + 1; i <= y + z - 1; i++)//left side and right side
				{
					if (i >= 0 && i < h && (x - z) >= 0)//inside array
					{
						float temp = 0.3989 * exp(-pow(z, 2) / 2);
						rTotal = rTotal + temp * GetPixel(x - z, i).r;
						gTotal = gTotal + temp * GetPixel(x - z, i).g;
						bTotal = bTotal + temp * GetPixel(x - z, i).b;
						weightTotal = weightTotal + 0.3989 * exp(-pow(z, 2) / 2);
					}
					if (i >= 0 && i < h && (x + z) < w)//inside array
					{
						float temp = 0.3989 * exp(-pow(z, 2) / 2);
						rTotal = rTotal + temp * GetPixel(x + z, i).r;
						gTotal = gTotal + temp * GetPixel(x + z, i).g;
						bTotal = bTotal + temp * GetPixel(x + z, i).b;
						weightTotal = weightTotal + 0.3989 * exp(-pow(z, 2) / 2);
					}
				}
			}
			blurred[x][y].r = rTotal / weightTotal;
			blurred[x][y].g = gTotal / weightTotal;
			blurred[x][y].b = bTotal / weightTotal;
		}
	}

	for (int x = 0; x < Width(); x++)
	{
		for (int y = 0; y < Height(); y++)
		{
			GetPixel(x, y).r = blurred[x][y].r;
			GetPixel(x, y).g = blurred[x][y].g;
			GetPixel(x, y).b = blurred[x][y].b;
		}
	}
}

void Image::Sharpen(int n)
{
	struct rgb {
		float r, g, b;
	};
	int h = Height();
	int w = Width();
	rgb **sharpen = new rgb*[w];
	for (int i = 0; i<w; i++)
	{
		sharpen[i] = new rgb[h];
	}

	for (int x = 0; x < Width(); x++)
	{
		for (int y = 0; y < Height(); y++)
		{
			float rTotal = 0;
			float gTotal = 0;
			float bTotal = 0;
			int pixelTotal = 0;
			for (int z = 1; z <= n; z++) //neighbor
			{
				for (int i = x - z; i <= x + z; i++)//top side and bottom side
				{
					if (i >= 0 && i < w && (y - z) >= 0)//inside array
					{
						rTotal = rTotal + GetPixel(i, y - z).r;
						gTotal = gTotal + GetPixel(i, y - z).g;
						bTotal = bTotal + GetPixel(i, y - z).b;
						pixelTotal++;
					}
					if (i >= 0 && i < w && (y + z) < h)//inside array
					{
						rTotal = rTotal + GetPixel(i, y + z).r;
						gTotal = gTotal + GetPixel(i, y + z).g;
						bTotal = bTotal + GetPixel(i, y + z).b;
						pixelTotal++;
					}
				}
				for (int i = y - z + 1; i <= y + z - 1; i++)//left side and right side
				{
					if (i >= 0 && i < h && (x - z) >= 0)//inside array
					{
						rTotal = rTotal + GetPixel(x - z, i).r;
						gTotal = gTotal + GetPixel(x - z, i).g;
						bTotal = bTotal + GetPixel(x - z, i).b;
						pixelTotal++;
					}
					if (i >= 0 && i < h && (x + z) < w)//inside array
					{
						rTotal = rTotal + GetPixel(x + z, i).r;
						gTotal = gTotal + GetPixel(x + z, i).g;
						bTotal = bTotal + GetPixel(x + z, i).b;
						pixelTotal++;
					}
				}
			}
			sharpen[x][y].r = (pixelTotal + 1) * GetPixel(x, y).r - rTotal;
			sharpen[x][y].g = (pixelTotal + 1) * GetPixel(x, y).g - gTotal;
			sharpen[x][y].b = (pixelTotal + 1) * GetPixel(x, y).b - bTotal;
			if (sharpen[x][y].r > 255) {
				sharpen[x][y].r = 255;
			}
			else if(sharpen[x][y].r < 0){
				sharpen[x][y].r = 0;
			}
			if (sharpen[x][y].g > 255) {
				sharpen[x][y].g = 255;
			}
			else if (sharpen[x][y].g < 0) {
				sharpen[x][y].g = 0;
			}
			if (sharpen[x][y].b > 255) {
				sharpen[x][y].b = 255;
			}
			else if (sharpen[x][y].b < 0) {
				sharpen[x][y].b = 0;
			}
		}
	}

	for (int x = 0; x < Width(); x++)
	{
		for (int y = 0; y < Height(); y++)
		{
			GetPixel(x, y).r = sharpen[x][y].r;
			GetPixel(x, y).g = sharpen[x][y].g;
			GetPixel(x, y).b = sharpen[x][y].b;
		}
	}
}

void Image::EdgeDetect()
{
	struct rgb {
		float r, g, b;
	};
	int h = Height();
	int w = Width();
	rgb **edge = new rgb*[w];
	for (int i = 0; i<w; i++)
	{
		edge[i] = new rgb[h];
	}

	for (int x = 0; x < Width(); x++)
	{
		for (int y = 0; y < Height(); y++)
		{
			float rTotal = 0;
			float gTotal = 0;
			float bTotal = 0;
			int pixelTotal = 0;
			
			for(int i = -1; i <= 1; i++)
			{
				for(int j = -1; j <= 1; j++)
				{
					if(j != 0 || i != 0)//omit the central point
					{
						if ((x + i) >= 0 && (x + i) < w && (y + j) >= 0 && (y + j) < h)//check if inside the image
						{
							rTotal = rTotal + GetPixel(x + i, y + j).r;
							gTotal = gTotal + GetPixel(x + i, y + j).g;
							bTotal = bTotal + GetPixel(x + i, y + j).b;
							pixelTotal++;
						}
					}
				}
			}
			
			edge[x][y].r = pixelTotal * GetPixel(x, y).r - rTotal;
			edge[x][y].g = pixelTotal * GetPixel(x, y).g - gTotal;
			edge[x][y].b = pixelTotal * GetPixel(x, y).b - bTotal;
			if (edge[x][y].r > 255) {
				edge[x][y].r = 255;
			}
			else if (edge[x][y].r < 0) {
				edge[x][y].r = 0;
			}
			if (edge[x][y].g > 255) {
				edge[x][y].g = 255;
			}
			else if (edge[x][y].g < 0) {
				edge[x][y].g = 0;
			}
			if (edge[x][y].b > 255) {
				edge[x][y].b = 255;
			}
			else if (edge[x][y].b < 0) {
				edge[x][y].b = 0;
			}
		}
	}

	for (int x = 0; x < Width(); x++)
	{
		for (int y = 0; y < Height(); y++)
		{
			GetPixel(x, y).r = edge[x][y].r;
			GetPixel(x, y).g = edge[x][y].g;
			GetPixel(x, y).b = edge[x][y].b;
		}
	}
}

Image* Image::Scale(double sx, double sy)
{
	int new_x, new_y;
	new_x = floor(sx * Width());
	new_y = floor(sy * Height());
	Image* scale;
	scale = new Image(new_x, new_y);//create a new array for the new picture

	for (int i = 0; i < new_x; i++) {
		for (int j = 0; j < new_y; j++) {
			scale->GetPixel(i, j) = Image::Sample(i / sx, j / sy);
		}
	}

	return scale;
}

Image* Image::Rotate(double angle)
{
	int h, w;
	h = Height();
	w = Width();
	int central_x, central_y;
	central_x = ceil(w / 2) - 1;
	central_y = ceil(h / 2) - 1;
	int oriUpLeft_x, oriUpLeft_y, oriUpRight_x, oriUpRight_y, oriDownLeft_x, oriDownLeft_y, oriDownRight_x, oriDownRight_y;
	oriUpLeft_x = 0 - central_x;
	oriUpLeft_y = 0 - central_y;
	oriUpRight_x = w - 1 - central_x;
	oriUpRight_y = 0 - central_y;
	oriDownLeft_x = 0 - central_x;
	oriDownLeft_y = h - 1 - central_y;
	oriDownRight_x = w - 1 - central_x;
	oriDownRight_y = h - 1 - central_y;
	float newUpLeft_x, newUpLeft_y, newUpRight_x, newUpRight_y, newDownLeft_x, newDownLeft_y, newDownRight_x, newDownRight_y;
	float radian;
	radian = angle * 3.1416 / 180;
	newUpLeft_x = oriUpLeft_x * cos(radian) - oriUpLeft_y * sin(radian);
	newUpLeft_y = oriUpLeft_x * sin(radian) + oriUpLeft_y * cos(radian);
	newUpRight_x = oriUpRight_x * cos(radian) - oriUpRight_y * sin(radian);
	newUpRight_y = oriUpRight_x * sin(radian) + oriUpRight_y * cos(radian);
	newDownLeft_x = oriDownLeft_x * cos(radian) - oriDownLeft_y * sin(radian);
	newDownLeft_y = oriDownLeft_x * sin(radian) + oriDownLeft_y * cos(radian);
	newDownRight_x = oriDownRight_x * cos(radian) - oriDownRight_y * sin(radian);
	newDownRight_y = oriDownRight_x * sin(radian) + oriDownRight_y * cos(radian);

	float upLeftCor_x, upLeftCor_y, upRightCor_x, upRightCor_y, downLeftCor_x, downLeftCor_y, downRightCor_x, downRightCor_y;
	upLeftCor_x = newUpLeft_x + central_x;
	upLeftCor_y = newUpLeft_y + central_y;
	upRightCor_x = newUpRight_x + central_x;
	upRightCor_y = newUpRight_y + central_y;
	downLeftCor_x = newDownLeft_x + central_x;
	downLeftCor_y = newDownLeft_y + central_y;
	downRightCor_x = newDownRight_x + central_x;
	downRightCor_y = newDownRight_y + central_y;

	int uppest, leftest, downest, rightest;
	leftest = min(min(upLeftCor_x, upRightCor_x), min(downLeftCor_x, downRightCor_x));
	uppest = min(min(upLeftCor_y, upRightCor_y), min(downLeftCor_y, downRightCor_y));
	rightest = max(max(upLeftCor_x, upRightCor_x), max(downLeftCor_x, downRightCor_x));
	downest = max(max(upLeftCor_y, upRightCor_y), max(downLeftCor_y, downRightCor_y));

	int newImageWidth = rightest - leftest;
	int newImageHeight = downest - uppest;

	Image* newImage;
	newImage = new Image(newImageWidth, newImageHeight);

	for (int i = 0; i < newImageWidth; i++) {
		for (int j = 0; j < newImageHeight; j++) {
			int temp_x = i + leftest - central_x;
			int temp_y = j + uppest - central_y;
			int rotatedTemp_x, rotatedTemp_y;
			rotatedTemp_x = round(temp_x * cos(-radian) - temp_y * sin(-radian)) + central_x;
			rotatedTemp_y = round(temp_x * sin(-radian) + temp_y * cos(-radian)) + central_y;
			if (rotatedTemp_x >= 0 && rotatedTemp_x < w && rotatedTemp_y >= 0 && rotatedTemp_y < h) {
				newImage->GetPixel(i, j) = Sample(rotatedTemp_x, rotatedTemp_y);
			}
			else {
				newImage->GetPixel(i, j).r = 0;
				newImage->GetPixel(i, j).g = 0;
				newImage->GetPixel(i, j).b = 0;
			}
		}
	}

	return newImage;
}

void Image::Fun()
{
	Image newImage(Width(), Height()); 
	int central_x, central_y;
	central_x = ceil(Width() / 2) - 1;
	central_y = ceil(Height() / 2) - 1;
	if (Width() > Height()) {
		for (int i = 0; i < Width(); i++) {
			for (int j = 0; j < Height(); j++) {
				float radian = 0.3 * (Height() / 2 - sqrt(pow((central_x - i), 2) + pow((central_y - j), 2))) * 3.1416 / 180;
				int vector_x = i - central_x;
				int vector_y = j - central_y;
				if ((pow(vector_x, 2) + pow(vector_y, 2)) < pow(Height() / 2, 2)) {
					float rotatedVector_x = vector_x * cos(radian) - vector_y * sin(radian);
					float rotatedVector_y = vector_x * sin(radian) + vector_y * cos(radian);
					float rotated_x = rotatedVector_x + central_x;
					float rotated_y = rotatedVector_y + central_y;
					newImage.GetPixel(i, j) = Sample(rotated_x, rotated_y);
				}
				else {
					newImage.GetPixel(i, j) = GetPixel(i, j);
				}
			}
		}
	}
	else {
		for (int i = 0; i < Width(); i++) {
			for (int j = 0; j < Height(); j++) {
				float radian = 0.3 * (Width() / 2 - sqrt(pow((central_x - i), 2) + pow((central_y - j), 2))) * 3.1416 / 180;
				int vector_x = i - central_x;
				int vector_y = j - central_y;
				if ((pow(vector_x, 2) + pow(vector_y, 2)) < pow(Width() / 2, 2)) {
					float rotatedVector_x = vector_x * cos(radian) - vector_y * sin(radian);
					float rotatedVector_y = vector_x * sin(radian) + vector_y * cos(radian);
					float rotated_x = rotatedVector_x + central_x;
					float rotated_y = rotatedVector_y + central_y;
					newImage.GetPixel(i, j) = Sample(rotated_x, rotated_y);
				}
				else {
					newImage.GetPixel(i, j) = GetPixel(i, j);
				}
			}
		}
	}
	
	for (int i = 0; i < Width(); i++) {
		for (int j = 0; j < Height(); j++) {
			GetPixel(i, j) = newImage.GetPixel(i, j);
		}
	}

}

/**
 * Image Sample
 **/
void Image::SetSamplingMethod(int method)
{
    assert((method >= 0) && (method < IMAGE_N_SAMPLING_METHODS));
    sampling_method = method;
}


Pixel Image::Sample (double u, double v){
	Pixel temp;
	switch (sampling_method)
	{
	case 0: {//point sampling
		int ori_x, ori_y;
		ori_x = round(u);
		ori_y = round(v);
		if (ori_x > (Width() - 1)) {
			ori_x = (Width() - 1);
		}
		if (ori_y > (Height() - 1)) {
			ori_y = (Height() - 1);
		}
		temp = GetPixel(ori_x, ori_y);
		break;
	}
	case 1: {//linear sampling
		int upLeft_x, upLeft_y, upRight_x, upRight_y, downLeft_x, downLeft_y, downRight_x, downRight_y;
		upLeft_x = floor(u);
		upLeft_y = floor(v);
		upRight_x = ceil(u);
		upRight_y = floor(v);
		downLeft_x = floor(u);
		downLeft_y = ceil(v);
		downRight_x = ceil(u);
		downRight_y = ceil(v);
		if (upRight_x >= Width()) {
			upRight_x = Width() - 1;
		}
		else if (upRight_x < 0) {
			upRight_x = 0;
		}
		if (upRight_y >= Height()) {
			upRight_y = Height() - 1;
		}
		else if (upRight_y < 0) {
			upRight_y = 0;
		}

		if (upLeft_x >= Width()) {
			upLeft_x = Width() - 1;
		}
		else if (upLeft_x < 0) {
			upLeft_x = 0;
		}
		if (upLeft_y >= Height()) {
			upLeft_y = Height() - 1;
		}
		else if (upLeft_y < 0) {
			upLeft_y = 0;
		}

		if (downRight_x >= Width()) {
			downRight_x = Width() - 1;
		}
		else if (downRight_x < 0) {
			downRight_x = 0;
		}
		if (downRight_y >= Height()) {
			downRight_y = Height() - 1;
		}
		else if (downRight_y < 0) {
			downRight_y = 0;
		}

		if (downLeft_x >= Width()) {
			downLeft_x = Width() - 1;
		}
		else if (downLeft_x < 0) {
			downLeft_x = 0;
		}
		if (downLeft_y >= Height()) {
			downLeft_y = Height() - 1;
		}
		else if (downLeft_y < 0) {
			downLeft_y = 0;
		}
		Pixel upLeft, upRight, downLeft, downRight;
		upLeft = GetPixel(upLeft_x, upLeft_y);
		upRight = GetPixel(upRight_x, upRight_y);
		downLeft = GetPixel(downLeft_x, downLeft_y);
		downRight = GetPixel(downRight_x, downRight_y);
		Pixel up, down;
		float alpha, beta;
		alpha = downRight_x - u;
		beta = downRight_y - v;
		up.r = upRight.r - (upRight.r - upLeft.r) * alpha;
		up.g = upRight.g - (upRight.g - upLeft.g) * alpha;
		up.b = upRight.b - (upRight.b - upLeft.b) * alpha;
		down.r = downRight.r - (downRight.r - downLeft.r) * alpha;
		down.g = downRight.g - (downRight.g - downLeft.g) * alpha;
		down.b = downRight.b - (downRight.b - downLeft.b) * alpha;
		temp.r = down.r - (down.r - up.r) * beta;
		temp.g = down.g - (down.g - up.g) * beta;
		temp.b = down.b - (down.b - up.b) * beta;
		break;
	}
	case 2: {//Gaussian Sampling
		int upLeft_x, upLeft_y, upRight_x, upRight_y, downLeft_x, downLeft_y, downRight_x, downRight_y;
		upLeft_x = floor(u);
		upLeft_y = floor(v);
		upRight_x = ceil(u);
		upRight_y = floor(v);
		downLeft_x = floor(u);
		downLeft_y = ceil(v);
		downRight_x = ceil(u);
		downRight_y = ceil(v);
		if (upRight_x >= Width()) {
			upRight_x = Width() - 1;
		}
		if (downRight_x >= Width()) {
			downRight_x = Width() - 1;
		}
		if (downRight_y >= Height()) {
			downRight_y = Height() - 1;
		}
		if (downLeft_y >= Height()) {
			downLeft_y = Height() - 1;
		}
		Pixel upLeft, upRight, downLeft, downRight;
		upLeft = GetPixel(upLeft_x, upLeft_y);
		upRight = GetPixel(upRight_x, upRight_y);
		downLeft = GetPixel(downLeft_x, downLeft_y);
		downRight = GetPixel(downRight_x, downRight_y);
		float up, down, left, right;
		right = downRight_x - u;
		down = downRight_y - v;
		up = 1 - down;
		left = 1 - right;
		float tr, tg, tb, upLeftWeight, upRightWeight, downRightWeight, downLeftWeight;
		upLeftWeight = 0.3989 * exp((pow(up, 2) + pow(left, 2)) / (-2));
		upRightWeight = 0.3989 * exp((pow(up, 2) + pow(right, 2)) / (-2));
		downLeftWeight = 0.3989 * exp((pow(down, 2) + pow(left, 2)) / (-2));
		downRightWeight = 0.3989 * exp((pow(down, 2) + pow(right, 2)) / (-2));
		tr = upLeft.r * upLeftWeight + upRight.r * upRightWeight + downLeft.r * downLeftWeight + downRight.r * downRightWeight;
		tg = upLeft.g * upLeftWeight + upRight.g * upRightWeight + downLeft.g * downLeftWeight + downRight.g * downRightWeight;
		tb = upLeft.b * upLeftWeight + upRight.b * upRightWeight + downLeft.b * downLeftWeight + downRight.b * downRightWeight;

		float totalWeight = upLeftWeight + upRightWeight + downLeftWeight + downRightWeight;
		
		float tempDistance, tempWeight;
		if ((upLeft_y - 1) >= 0) {
			tempDistance = pow(left, 2) + pow((1 + up), 2);
			tempWeight = 0.3989 * exp(tempDistance / (-2));
			totalWeight = totalWeight + tempWeight;
			tr = tr + GetPixel(upLeft_x, upLeft_y - 1).r * tempWeight;
			tg = tg + GetPixel(upLeft_x, upLeft_y - 1).g * tempWeight;
			tb = tb + GetPixel(upLeft_x, upLeft_y - 1).b * tempWeight;
			tempDistance = pow(right, 2) + pow((1 + up), 2);
			tempWeight = 0.3989 * exp(tempDistance / (-2));
			totalWeight = totalWeight + tempWeight;
			tr = tr + GetPixel(upRight_x, upLeft_y - 1).r * tempWeight;
			tg = tg + GetPixel(upRight_x, upLeft_y - 1).g * tempWeight;
			tb = tb + GetPixel(upRight_x, upLeft_y - 1).b * tempWeight;
		}

		if ((downLeft_y + 1) < Height()) {
			tempDistance = pow(left, 2) + pow((1 + down), 2);
			tempWeight = 0.3989 * exp(tempDistance / (-2));
			totalWeight = totalWeight + tempWeight;
			tr = tr + GetPixel(downLeft_x, downLeft_y + 1).r * tempWeight;
			tg = tg + GetPixel(downLeft_x, downLeft_y + 1).g * tempWeight;
			tb = tb + GetPixel(downLeft_x, downLeft_y + 1).b * tempWeight;
			tempDistance = pow(right, 2) + pow((1 + down), 2);
			tempWeight = 0.3989 * exp(tempDistance / (-2));
			totalWeight = totalWeight + tempWeight;
			tr = tr + GetPixel(downRight_x, downRight_y + 1).r * tempWeight;
			tg = tg + GetPixel(downRight_x, downRight_y + 1).g * tempWeight;
			tb = tb + GetPixel(downRight_x, downRight_y + 1).b * tempWeight;
		}
		
		if ((downLeft_x - 1) >= 0) {
			tempDistance = pow(up, 2) + pow((1 + left), 2);
			tempWeight = 0.3989 * exp(tempDistance / (-2));
			totalWeight = totalWeight + tempWeight;
			tr = tr + GetPixel(upLeft_x - 1, upLeft_y).r * tempWeight;
			tg = tg + GetPixel(upLeft_x - 1, upLeft_y).g * tempWeight;
			tb = tb + GetPixel(upLeft_x - 1, upLeft_y).b * tempWeight;
			tempDistance = pow(down, 2) + pow((1 + left), 2);
			tempWeight = 0.3989 * exp(tempDistance / (-2));
			totalWeight = totalWeight + tempWeight;
			tr = tr + GetPixel(downLeft_x - 1, downLeft_y).r * tempWeight;
			tg = tg + GetPixel(downLeft_x - 1, downLeft_y).g * tempWeight;
			tb = tb + GetPixel(downLeft_x - 1, downLeft_y).b * tempWeight;
		}

		if ((downRight_x + 1) < Width()) {
			tempDistance = pow(up, 2) + pow((1 + right), 2);
			tempWeight = 0.3989 * exp(tempDistance / (-2));
			totalWeight = totalWeight + tempWeight;
			tr = tr + GetPixel(upRight_x + 1, upRight_y).r * tempWeight;
			tg = tg + GetPixel(upRight_x + 1, upRight_y).g * tempWeight;
			tb = tb + GetPixel(upRight_x + 1, upRight_y).b * tempWeight;
			tempDistance = pow(down, 2) + pow((1 + right), 2);
			tempWeight = 0.3989 * exp(tempDistance / (-2));
			totalWeight = totalWeight + tempWeight;
			tr = tr + GetPixel(downRight_x + 1, downRight_y).r * tempWeight;
			tg = tg + GetPixel(downRight_x + 1, downRight_y).g * tempWeight;
			tb = tb + GetPixel(downRight_x + 1, downRight_y).b * tempWeight;
		}
		
		temp.r = tr / totalWeight;
		temp.g = tg / totalWeight;
		temp.b = tb / totalWeight;

		break;
	}
	default:
		break;
	}

	return temp;
}