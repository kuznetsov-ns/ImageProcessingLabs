#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <ctime>

using namespace std;
using namespace cv;

double psnr(Mat& ref_image, Mat& obj_image)
{
	double mse = 0;
	double div = 0;
	int width = ref_image.cols;
	int height = ref_image.rows;
	double psnr = 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			div = ref_image.at<uchar>(y, x) - obj_image.at<uchar>(y, x);

			mse += div * div;

		}
	}

	mse = mse / (width * height);
	psnr = 10 * log10(255 * 255 / mse);

	return psnr;
}

Mat monochrome(Mat& ref_image)
{
	Mat result;
	result = ref_image.clone();

	for (int y = 0; y < result.rows; y++)
	{
		for (int x = 0; x < result.cols; x++)
		{
			double tmp = result.at<Vec3b>(y, x).val[0] * 0.148 + result.at<Vec3b>(y, x).val[1] * 0.5547 + result.at<Vec3b>(y, x).val[2] * 0.2952;
			result.at<Vec3b>(y, x).val[0] = tmp;
			result.at<Vec3b>(y, x).val[1] = tmp;
			result.at<Vec3b>(y, x).val[2] = tmp;
		}
	}

	return result;
}

Mat BGR_2_YCrCb(Mat& image)
{
	double Y = 0.0;
	double Cb = 0.0;
	double Cr = 0.0;
	Mat result = image.clone();
	for (int y = 0; y < result.rows; y++)
	{
		for (int x = 0; x < result.cols; x++)
		{
			Y = result.at<Vec3b>(y, x).val[0] * 0.114 + result.at<Vec3b>(y, x).val[1] * 0.587 + result.at<Vec3b>(y, x).val[2] * 0.299;
			Cb = 0.564*(result.at<Vec3b>(y, x).val[0] - Y) + 128.0;
			Cr = 0.713*(result.at<Vec3b>(y, x).val[2] - Y) + 128.0;
			result.at<Vec3b>(y, x).val[0] = Y;
			result.at<Vec3b>(y, x).val[1] = Cr;
			result.at<Vec3b>(y, x).val[2] = Cb;
		}
	}
	return result;
}

Mat YCrCb_2_BGR(Mat& image)
{
	Mat result = image.clone();
	for (int y = 0; y < result.rows; y++)
	{
		for (int x = 0; x < result.cols; x++)
		{
			result.at<Vec3b>(y, x).val[0] = image.at<Vec3b>(y, x).val[0] + 1.772*(image.at<Vec3b>(y, x).val[2] - 128.0);
			result.at<Vec3b>(y, x).val[1] = image.at<Vec3b>(y, x).val[0] - 0.344*(image.at<Vec3b>(y, x).val[2] - 128.0)
				- 0.714*(image.at<Vec3b>(y, x).val[1] - 128.0);
			result.at<Vec3b>(y, x).val[2] = image.at<Vec3b>(y, x).val[0] + 1.402*(image.at<Vec3b>(y, x).val[1] - 128.0);
		}
	}
	return result;
}

Mat LumY(Mat& image)
{
	Mat result = image.clone();

	for (int y = 0; y < result.rows; y++)
	{
		for (int x = 0; x < result.cols; x++)
		{
			if (result.at<Vec3b>(y, x).val[0] + 50 > 255) result.at<Vec3b>(y, x).val[0] = 255;
			else result.at<Vec3b>(y, x).val[0] += 50;
		}
	}

	return result;
}

Mat LumBGR(Mat& image)
{
	Mat result = image.clone();

	for (int y = 0; y < result.rows; y++)
	{
		for (int x = 0; x < result.cols; x++)
		{
			if (result.at<Vec3b>(y, x).val[0] + 50 > 255) result.at<Vec3b>(y, x).val[0] = 255;
			else result.at<Vec3b>(y, x).val[0] += 50;
			if (result.at<Vec3b>(y, x).val[1] + 50 > 255) result.at<Vec3b>(y, x).val[1] = 255;
			else result.at<Vec3b>(y, x).val[1] += 50;
			if (result.at<Vec3b>(y, x).val[2] + 50 > 255) result.at<Vec3b>(y, x).val[2] = 255;
			else result.at<Vec3b>(y, x).val[2] += 50;
		}
	}
	return result;
}

int main()
{
    Mat A = imread("img.jpg");

    if (A.empty())
    {
        cout << "Image not found" << endl;
        return -1;
    }

	Mat B;
    cvtColor(A, B, COLOR_BGR2GRAY);
    imwrite("COLOR_BGR2GRAY.jpg", B);

	Mat C;
	C = monochrome(A);
	imwrite("Manual_Monochrome.jpg", C);

	Mat D;
	time_t seconds_1 = time(NULL);
	cvtColor(A, D, COLOR_BGR2YCrCb);
	time_t seconds_2 = time(NULL);
	cout << "OpenCV BGR2YCrCb: " << seconds_2 - seconds_1 << " seconds" << endl;
	imwrite("BGR2YCbCr_OpenCV.jpg", D);

	Mat F;
	time_t seconds_3 = time(NULL);
	F = BGR_2_YCrCb(A);
	time_t seconds_4 = time(NULL);
	cout << "Manual BGR2YCrCb: " << seconds_4 - seconds_3 << " seconds" << endl;
	imwrite("BGR2YCbCr_Manual.jpg", F);

	Mat E;
	E = LumY(D);
	imwrite("AdjustingBrightnessYCbCr.jpg", E);

	Mat G;
	G = LumBGR(A);
	imwrite("AdjustingBrightnessBGR.jpg", G);

	double PSNR_1 = psnr(A, B);
	double PSNR_2 = psnr(A, C);
	double PSNR_3 = psnr(A, D);
	double PSNR_4 = psnr(A, F);
	double PSNR_5 = psnr(D, E);
	double PSNR_6 = psnr(A, G);

	cout << "PSNR(Original, CL_BGR2GRAY) = " << PSNR_1 << endl;
	cout << "PSNR(Original, Manual_Monochrome) = " << PSNR_2 << endl;
	cout << "PSNR(Original, BGR2YCbCr_OpenCV) = " << PSNR_3 << endl;
	cout << "PSNR(Original, BGR2YCbCr_Manual) = " << PSNR_4 << endl;
	cout << "PSNR(BGR2YCbCr_OpenCV, AdjustingBrightnessYCbCr) = " << PSNR_5 << endl;
	cout << "PSNR(Original, AdjustingBrightnessBGR) = " << PSNR_6 << endl;

    return 0;
}