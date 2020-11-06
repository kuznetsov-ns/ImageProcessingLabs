#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#define PROCENT 10

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

int clamp(int value, int max, int min)
{
	if (value > max)
		return max;
	else if (value < min)
		return min;
	else
		return value;
}

Mat medianFilter(Mat& ref_img, int rad)
{
	Mat tmp = ref_img.clone();
	int diam = 2 * rad  + 1;
	vector<int> red, blue, green;
	red.resize(diam * diam);
	blue.resize(diam * diam);
	green.resize(diam * diam);
	for (int x = 0; x < ref_img.rows; x++)
	{
		for (int y = 0; y < ref_img.cols; y++)
		{
			for (int i = -rad; i <= rad; i++)
			{
				for (int j = -rad; j <= rad; j++)
				{
					int idx = (i + rad) * diam + j + rad;
					Vec3b color = tmp.at<Vec3b>(clamp(x + j, ref_img.rows - 1, 0), clamp(y + i, ref_img.cols - 1, 0));
					blue[idx] = color[0];
					green[idx] = color[1];
					red[idx] = color[2];
				}
			}
			sort(red.begin(), red.end());
			sort(blue.begin(), blue.end());
			sort(green.begin(), green.end());
			int center_blue = blue[(diam * diam) / 2];
			int center_green = green[(diam * diam) / 2];
			int center_red = red[(diam * diam) / 2];
			Vec3b cl = { static_cast<unsigned char>(center_blue), static_cast<unsigned char>(center_green),
				static_cast<unsigned char>(center_red) };
			tmp.at<Vec3b>(x, y) = cl;
		}
	}
	return tmp;
}

Mat pointFilter(Mat& ref_img, int rad)
{
	Mat tmp = ref_img.clone();
	int diam = 2 * rad + 1;
	vector<int> red, blue, green;
	red.resize(diam * diam);
	blue.resize(diam * diam);
	green.resize(diam * diam);
	for (int x = 0; x < ref_img.rows; x++)
	{
		for (int y = 0; y < ref_img.cols; y++)
		{
			for (int i = -rad; i <= rad; i++)
			{
				for (int j = -rad; j <= rad; j++)
				{
					int idx = (i + rad) * diam + j + rad;
					Vec3b color = tmp.at<Vec3b>(clamp(x + j, ref_img.rows - 1, 0), clamp(y + i, ref_img.cols - 1, 0));
					blue[idx] = color[0];
					green[idx] = color[1];
					red[idx] = color[2];
				}
			}
			sort(red.begin(), red.end());
			sort(blue.begin(), blue.end());
			sort(green.begin(), green.end());
			int r_avg = (red.at(0) + red.at(red.size() - 1)) / 2;
			int g_avg = (green.at(0) + green.at(green.size() - 1)) / 2;
			int b_avg = (blue.at(0) + blue.at(blue.size() - 1)) / 2;
			Vec3b cl = { static_cast<unsigned char>(b_avg), static_cast<unsigned char>(g_avg),
				static_cast<unsigned char>(r_avg) };
			tmp.at<Vec3b>(x, y) = cl;
		}
	}
	return tmp;
}

Mat proc(Mat& ref_img, int n)
{
	if (ref_img.empty())
		throw - 1;
	Mat tmp = ref_img.clone();
	for (int i = 0; i < n; i++)
	{
		int a = rand() % ref_img.cols;
		int b = rand() % ref_img.rows;
		if (ref_img.channels() == 1)
		{
			tmp.at<uchar>(b, a) = 255;
		}
		else if (ref_img.channels() == 3)
		{
			tmp.at<Vec3b>(b, a)[0] = 255;
			tmp.at<Vec3b>(b, a)[1] = 255;
			tmp.at<Vec3b>(b, a)[2] = 255;
		}
	}
	return tmp;
}

int main()
{
    Mat img = imread("img.jpg");
    if (img.empty())
    {
        cout << "Image not found" << endl;
        return -1;
    }

	Mat noise;
	int size_ = (img.cols * img.rows) / 100 * PROCENT;
	noise = proc(img, size_);
	imwrite("noise.jpg", noise);

    Mat gaussian_noise = Mat::zeros(img.rows, img.cols, CV_8UC3);
    randn(gaussian_noise, 0, 30);

	//median
	time_t seconds_1 = time(NULL);
	Mat median_filter = medianFilter(noise, 1);
	time_t seconds_2 = time(NULL);
	cout << "Manual median filter: " << seconds_2 - seconds_1 << " seconds" << endl;
	imwrite("median_filter.jpg", median_filter);

	//median_ocv
	Mat median_filter_ocv;	
	time_t seconds_3 = time(NULL);
	medianBlur(noise, median_filter_ocv, 3);
	time_t seconds_4 = time(NULL);
	cout << "OpenCV median filter: " << seconds_4 - seconds_3 << " seconds" << endl;
	imwrite("median_filter_ocv.jpg", median_filter_ocv);

	//point filter
	Mat point_filter = pointFilter(noise, 3);
	imwrite("point_filter.jpg", point_filter);

	//psnr
	double res_1 = psnr(noise, median_filter);
	double res_2 = psnr(noise, median_filter_ocv);
	cout << res_1 << " " << res_2 << endl;
    return 0;
}