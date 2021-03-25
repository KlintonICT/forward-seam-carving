#include <stdio.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <math.h>

using namespace cv;
using namespace std;

int Cl(int A, int B, int C, int D) {
	return abs(A - B) + abs(C - D);
}

int Cu(int A, int B) {
	return abs(A - B);
}

int Cr(int A, int B, int C, int D) {
	return abs(A - B) + abs(C - D);
}

void findMin(int L, int U, int R, int *min, int *direction) {
	int LUR[] = { L,U,R };
	int minv = L;
	int minindex = 0;

	for (int i = 0; i < 3; i++) {
		if (LUR[i] < minv) {
			minv = LUR[i];
			minindex = i;
		}
	}

	*min = minv;
	*direction = minindex + 1;
}

int main() {
	//Display an Image
	Mat img = imread("1.jpg", IMREAD_COLOR);
	namedWindow("Petty", WINDOW_AUTOSIZE);
	imshow("Petty", img);
	int c = cvWaitKey(0);
	Size sz = img.size();
	int height = sz.height;
	int width = sz.width;

	// Esc = 27, a = 97, d = 100, s 115, w = 119
	while (c != 27) {
		//Looping till get the command 'a', 'd', 'w', 's'
		while (c != 97 && c != 100 && c != 115 && c != 119) {
			c = cvWaitKey(0);
		}
		// Keyboard command ::: 'a' and 'd' => vertical best seam, 'w' and 's' => horizontal best seam
		// 'a' => Reduce width, 'd' => increase width
		vector<int> bestSeamVert;
		if (c == 97 || c == 100) {
			// Construct M matrix and K matrix in the vertical direction
			// *** WRITE YOUR CODE ***
			Mat addPaddingImg;
			Mat img_grey;
			cvtColor(img, img_grey, COLOR_BGR2GRAY);
			Mat M(Size(width, height), CV_8UC1, Scalar(0));
			Mat K(Size(width, height), CV_8UC1, Scalar(0));
			copyMakeBorder(img_grey, addPaddingImg, 1, 1, 1, 1, BORDER_REPLICATE);
			for (int row = 1; row < addPaddingImg.rows - 1; row++) { // loop as real size img
				for (int col = 1; col < addPaddingImg.cols - 1; col++) { // loop as real size img
					int cl = Cl((int)addPaddingImg.at<uchar>(row, col + 1), (int)addPaddingImg.at<uchar>(row, col - 1), (int)addPaddingImg.at<uchar>(row - 1, col), (int)addPaddingImg.at<uchar>(row, col - 1));
					int cu = Cu((int)addPaddingImg.at<uchar>(row, col + 1), (int)addPaddingImg.at<uchar>(row, col - 1));
					int cr = Cr((int)addPaddingImg.at<uchar>(row, col + 1), (int)addPaddingImg.at<uchar>(row, col - 1), (int)addPaddingImg.at<uchar>(row - 1, col), (int)addPaddingImg.at<uchar>(row, col + 1));
					int i = row - 1; //padding img row - 1 = real img row
					int j = col - 1; // padding img col - 1 = real img col
					if (i == 0) {
						M.at<uchar>(i, j) = cu;
						K.at<uchar>(i, j) = 0;
					}
					else{
						if (j == 0) { // no left 
							int mu = (int)M.at<uchar>(i - 1, j) + cu;
							int mr = (int)M.at<uchar>(i - 1, j + 1) + cr;
							if (mu <= mr) {
								M.at<uchar>(i, j) = mu;
								K.at<uchar>(i, j) = 2;
							}
							else {
								M.at<uchar>(i, j) = mr;
								K.at<uchar>(i, j) = 3;
							}
						}
						else if (j !=0 && j < width - 1) {
							int ml = (int)M.at<uchar>(i - 1, j - 1) + cl;
							int mu = (int)M.at<uchar>(i - 1, j) + cu;
							int mr = (int)M.at<uchar>(i - 1, j + 1) + cr;
							int result, track;
							findMin(ml, mu, mr, &result, &track);
							M.at<uchar>(i, j) = result;
							K.at<uchar>(i, j) = track;
						}
						else { // no right
							int ml = (int)M.at<uchar>(i - 1, j - 1) + cl;
							int mu = (int)M.at<uchar>(i - 1, j) + cu;
							if (ml <= mu) {
								M.at<uchar>(i, j) = ml;
								K.at<uchar>(i, j) = 1;
							}
							else {
								M.at<uchar>(i, j) = mu;
								K.at<uchar>(i, j) = 2;
							}
						}
					}
				}
			}
			cvWaitKey(1);
			imshow("Matrix M", M);
			c == 97 ? imwrite("Delete_M_vertical.jpg", M) : imwrite("Increase_M_vertical.jpg", M);
			// Find the best seam in the vertical direction
			// *** WRITE YOUR CODE ***
			//bestseam(Size(width, height), CV_8UC1, Scalar(0));
			Mat bestSeamMat(Size(width, height), CV_8UC1, Scalar(0));
			int minColIndex = 0;
			// Find the the minimum of last row
			for (int i = 0; i < width; i++) {
				int col = (int)M.at<uchar>(height - 1, i);
				int col1 = (int)M.at<uchar>(height - 1, minColIndex);
				if (col < col1) minColIndex = i;
			}
			int row = height - 1; //last row
			bestSeamMat.at<uchar>(row, minColIndex) = 255;
			bestSeamVert.push_back(minColIndex);
			while (row > 0) {
				int track = (int)K.at<uchar>(row, minColIndex);
				if (track == 1) {
					bestSeamMat.at<uchar>(row - 1, minColIndex - 1) = 255;
					bestSeamVert.push_back(minColIndex - 1);
					minColIndex--;
				}
				else if (track == 2 || track == 0) {
					bestSeamMat.at<uchar>(row - 1, minColIndex) = 255;
					bestSeamVert.push_back(minColIndex);
				}
				else if (track == 3) {
					bestSeamMat.at<uchar>(row - 1, minColIndex + 1) = 255;
					bestSeamVert.push_back(minColIndex + 1);
					minColIndex++;
				}
				row--;
			}
			reverse(bestSeamVert.begin(), bestSeamVert.end());
			cvWaitKey(1);
			imshow("Best Seam", bestSeamMat);
			img_grey.release();
			addPaddingImg.release();
			M.release();
			K.release();
			bestSeamMat.release();
		}
		vector<int> bestSeamHori;
		// 'w' => increase height, 's' => reduce height
		if (c == 115 || c == 119) {
			// Construct M matrix and K matrix in the horizontal direction
			// *** WRITE YOUR CODE ***

			Mat img_gray;
			Mat addPaddingImg;
			cvtColor(img,img_gray, COLOR_BGR2GRAY);
			Mat M(Size(width, height), CV_8UC1, Scalar(0));
			Mat K(Size(width, height), CV_8UC1, Scalar(0));
			copyMakeBorder(img_gray, addPaddingImg, 1, 1, 1, 1, BORDER_REPLICATE);
			for (int col = 1; col < addPaddingImg.cols - 1; col++) { // loop as real size img
				for (int row = 1; row < addPaddingImg.rows - 1; row++) { // loop as real size img
					int cl = Cl((int)addPaddingImg.at<uchar>(row, col - 1), (int)addPaddingImg.at<uchar>(row + 1, col), (int)addPaddingImg.at<uchar>(row - 1, col), (int)addPaddingImg.at<uchar>(row + 1, col));
					int cu = Cu((int)addPaddingImg.at<uchar>(row - 1, col), (int)addPaddingImg.at<uchar>(row + 1, col));
					int cr = Cr((int)addPaddingImg.at<uchar>(row, col - 1), (int)addPaddingImg.at<uchar>(row - 1, col), (int)addPaddingImg.at<uchar>(row - 1, col), (int)addPaddingImg.at<uchar>(row + 1, col));
					int i = row - 1; //padding img row - 1 = real img row
					int j = col - 1;// padding img col - 1 = real img col
					if ( j == 0 ) {
						M.at<uchar>(i, j) = cu;
						K.at<uchar>(i, j) = 0;
					}
					else {
						if (i == 0) { // no right 
							int ml = (int)M.at<uchar>(i + 1, j - 1) + cl;
							int mu = (int)M.at<uchar>(i, j - 1) + cu;
							if (ml <= mu) {
								M.at<uchar>(i, j) = ml;
								K.at<uchar>(i, j) = 1;
							}
							else {
								M.at<uchar>(i, j) = mu;
								K.at<uchar>(i, j) = 2;
							}
						}
						else if (i < height - 1) {
							int ml = (int)M.at<uchar>(i + 1, j - 1) + cl;
							int mu = (int)M.at<uchar>(i , j - 1) + cu; 
							int mr = (int)M.at<uchar>(i - 1, j - 1) + cr;
							int result, track;
							findMin(ml, mu, mr, &result, &track);
							M.at<uchar>(i, j) = result;
							K.at<uchar>(i, j) = track;
						}
						else {  // no left
							int mu = (int)M.at<uchar>(i, j - 1) + cu;
							int mr = (int)M.at<uchar>(i - 1, j - 1) + cr;
							if (mu <= mr) {
								M.at<uchar>(i, j) = mu;
								K.at<uchar>(i, j) = 2;
							}
							else {
								M.at<uchar>(i, j) = mr;
								K.at<uchar>(i, j) = 3;
							}
						}
					}
				}
			}
			cvWaitKey(1);
			imshow("Matrix M", M);
			c == 115 ? imwrite("Delete_M_horizontal.jpg", M) : imwrite("Increase_M_horizontal.jpg", M);
			// Find the best seam in the horizontal direction
			// *** WRITE YOUR CODE ***
			
			Mat bestSeamMat(Size(width, height), CV_8UC1, Scalar(0));
			int minRowIndex = 0;
			//Find the minimum of last col
			for (int j = 0; j < height; j++) {
				int row = (int)M.at<uchar>(j, width - 1);
				int row1 = (int)M.at<uchar>(minRowIndex, width - 1);
				if (row < row1) minRowIndex = j;
			}
			int col = width - 1;
			bestSeamMat.at<uchar>(minRowIndex, col) = 255;
			bestSeamHori.push_back(minRowIndex);
			while (col > 0) {
				int track = (int)K.at<uchar>(minRowIndex, col);
				if (track == 1) {
					bestSeamMat.at<uchar>(minRowIndex + 1, col - 1) = 255;
					bestSeamHori.push_back(minRowIndex + 1);
					minRowIndex++;
				}
				else if (track == 2) {
					bestSeamMat.at<uchar>(minRowIndex, col - 1) = 255;
					bestSeamHori.push_back(minRowIndex);
				}
				else if (track == 3) {
					bestSeamMat.at<uchar>(minRowIndex - 1, col - 1) = 255;
					bestSeamHori.push_back(minRowIndex - 1);
					minRowIndex--;
				}
				col--;
			}
			reverse(bestSeamHori.begin(), bestSeamHori.end());
			cvWaitKey(1);
			imshow("Best Seam", bestSeamMat);
			img_gray.release();
			addPaddingImg.release();
			M.release();
			K.release();
			bestSeamMat.release();
		}
		// Insert or delete the best seam
		if ( c == 97 ) {
			// Reduce width or delete seam vertically
			// Copy the pixels into this image
			Mat img_new(height, --width, CV_8UC3, Scalar(0, 0, 0));
			// *** WRITE YOUR CODE ***
		
			for (int row = 0; row < height; row++) {
				for (int col = 0; col < width; col++) {
					for (int channel = 0; channel < img.channels(); channel++) {
						if (col > bestSeamVert[row]) {
							img_new.at<Vec3b>(row, col)[channel] = img.at<Vec3b>(row, col + 1)[channel];
						}
						else {
							img_new.at<Vec3b>(row, col)[channel] = img.at<Vec3b>(row, col)[channel];
						}
					}
				}
			}
			// Show the resized image
			imshow("Petty", img_new);
			imwrite("Delete_real_image_vertical.jpg", img_new);
			// Clone img_new into img for the next loop processing
			img.release();
			img = img_new.clone();
			img_new.release();
			bestSeamVert.clear();
		}
		if ( c == 100 ) {
			// Increase width or insert seam vertically
			// Copy the pixels into this image
			Mat img_new(height, ++width, CV_8UC3, Scalar(0, 0, 0));
			// *** WRITE YOUR CODE ***
			int newimg_row = 0, newimg_col = 0;
			//cout << width <<" "<<bestseam.cols<<endl;
			for (int row = 0; row < height; row++) {
				newimg_col = 0;
				for (int col = 0; col < width; col++) {
					for (int channel = 0; channel < img.channels(); channel++) {
						if (col > bestSeamVert[row]) {
							img_new.at<Vec3b>(row, col)[channel] = img.at<Vec3b>(row, col - 1)[channel];
						}
						else {
							img_new.at<Vec3b>(row, col)[channel] = img.at<Vec3b>(row, col)[channel];
						}
					}
				}
			}
			// Show the resized image
			imshow("Petty", img_new);
			imwrite("Increase_real_image_vertical.jpg", img_new);
			// Clone img_new into img for the next loop processing
			img.release();
			img = img_new.clone();
			img_new.release();
			bestSeamVert.clear();
		}
		if ( c == 115 ) {
			// Reduce height or delete seam horizontally
			// Copy the pixels into this image
			Mat img_new(--height, width, CV_8UC3, Scalar(0, 0, 0));
			// *** WRITE YOUR CODE ***
			for (int col = 0; col < width; col++) {
				for (int row = 0; row < height; row++) {
					for (int channel = 0; channel < img.channels(); channel++) {
						if (row > bestSeamHori[col]) {
							img_new.at<Vec3b>(row, col)[channel] = img.at<Vec3b>(row + 1, col)[channel];
						}
						else {
							img_new.at<Vec3b>(row, col)[channel] = img.at<Vec3b>(row, col)[channel];
						}
					}
				}
			}
			// Show the resized image
			imshow("Petty", img_new);
			imwrite("Delete_real_image_horizontal.jpg", img_new);
			// Clone img_new into img for the next loop processing
			img.release();
			img = img_new.clone();
			img_new.release();
			bestSeamHori.clear();
		}
		if (c == 119) {
			// Increase height or insert seam horizontally
			// Copy the pixels into this image
			Mat img_new(++height, width, CV_8UC3, Scalar(0, 0, 0));
			// *** WRITE YOUR CODE ***
			for (int col = 0; col < width; col++) {
				for (int row = 0; row < height; row++) {
					for (int channel = 0; channel < img.channels(); channel++) {
						if (row > bestSeamHori[col]) {
							img_new.at<Vec3b>(row, col)[channel] = img.at<Vec3b>(row - 1, col)[channel];
						} 
						else {
							img_new.at<Vec3b>(row, col)[channel] = img.at<Vec3b>(row, col)[channel];
						}
					}
				}
			}
			// Show the resized image
			imshow("Petty", img_new);
			imwrite("Increase_real_image_horizontal.jpg", img_new);
			// Clone img_new into img for the next loop processing
			img.release();
			img = img_new.clone();
			img_new.release();
			bestSeamHori.clear();
		}
		if (c == 27) break;
		c = cvWaitKey(0);
	}
	return 0;
}