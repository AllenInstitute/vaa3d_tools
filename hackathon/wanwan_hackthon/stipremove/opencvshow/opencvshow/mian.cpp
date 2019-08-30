#include<opencv2\opencv.hpp>
#include<vector>
#include<opencv.hpp>
using namespace cv;
using namespace std;
#include<iostream> 
#include "highgui.h"
int main()
{	//����һ���Ҷ�ͼ��	
	//cv::Mat src = cv::imread("F:\\postegraduate\\1.tif", IMREAD_GRAYSCALE);
	/*------------------------------------------------------------1.����ͼ��-----------------------------------------------------------------------------------------*/
	
	cv::Mat src = cv::imread("D:\\Allen\\vaa3d\\vaa3d_tools\\hackathon\\wanwan\\1\\1.tif", IMREAD_GRAYSCALE);
	if (!src.data)
	{
		std::cout << "Image Load Fail!!!" << "\n";
		return 1;
	}
	//cv::namedWindow("SRC");
	/*imshow("SRC", src);
	imwrite("D:\\Allen\\vaa3d\\vaa3d_tools\\hackathon\\wanwan\\1\\yuantu.tif", src);
	cv::waitKey();
	cv::destroyAllWindows();*/
	/*------------------------------------------------------------------------------------2.fft,show Ƶ��ͼ-------------------------------------------------------------------------------------------------*/
	//����ͼ��ߴ絽��ѳߴ磬�߽���0c	
	//cv2.resize(src, (src.shape[1] * 0.5, src.shape[0] * 0.5));
	//int r = cv::getOptimalDFTSize(src.rows);//padding��ĸߣ����������ߴ羭��DFT�任���������ųߴ�
	//int c = cv::getOptimalDFTSize(src.cols);//padding��Ŀ�
	//cv::Mat padded;
	////����ͼ��߽磻����ͼ�����ͼ��top/bottom/left/right�ĸ������������ص�ֵ���߽����ͣ����0
	//cv::copyMakeBorder(src, padded, 0, r - src.rows, 0, c - src.cols, cv::BORDER_CONSTANT,cv::Scalar::all(0));
	//cv::copyMakeBorder(src, padded, 0, 0, 0, 0, cv::BORDER_CONSTANT, cv::Scalar::all(0));

	//Ϊ����Ҷ�任�Ľ��������������ʵ�����鲿��������Ҫ����һ����ά�����飩����洢�ռ�,	
	//��Ҫ������float�����洢	
	//��󽫶�ά����ϲ�Ϊ��ͨ��--����Ҷ�任��Ҫ	

	//cv::Mat dst1[] = { cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F) };
	cv::Mat dst1[] = { cv::Mat_<float>(src), cv::Mat::zeros(src.size(), CV_32F) };//padding���ͼ��ֵ����ͨ��1��ʵ����ͨ��2�鲿��ȫ0
	cv::Mat dst2;
	cv::merge(dst1, 2, dst2);//dst1��2����ͨ���ϳɶ�ͨ��dst2
	//FFT�任,������ɴ洢��dst2��	
	cv::dft(dst2, dst2);
	
	//����������ɷ�ֵ	
	cv::split(dst2, dst1);//�Ѷ�ͨ��ͼ��dst2�ֽ�Ϊ��ά���飬���浽dst1��,dst1[0]�д�ŵ�Ϊʵ����dst1[1]�д�ŵ��鲿
	Mat mag;
	Mat angle;
	cv::magnitude(dst1[0], dst1[1], mag);//ʵ�����鲿���������������dst1[0]������͵�һ����������ͬ�ĳߴ�����ͣ���	;�õ�ԭͼ��ֵmag
	cv::phase(dst1[0], dst1[1], angle);//�õ�ԭͼ��λ
	cv::Mat magnitudeImage = mag;  	//��ֵ�����ŵ�mat magbitudeImage��
	
	//�����߶������Ա�����ʾ	
	//����log(1 + sqrt(Re(DFT(dst2))**2 + Im(DFT(dst2))**2))	
	magnitudeImage += cv::Scalar::all(1);
	cv::log(magnitudeImage, magnitudeImage);

	//���к��طֲ�����ͼ����	
	//���������л������У�����Ƶ�ײü�	
	//magnitudeImage = magnitudeImage(cv::Rect(0, 0, magnitudeImage.cols & -2, magnitudeImage.rows & -2));//�κ�һ����&-2�Ľ��һ����ż����λ�ĺ����㣬-2=11111110���κ���������������λ��Ϊ0��һ����ż��

	//�������и���Ҷͼ������ޣ�ʹԭ��λ��ͼ������	
	//�������е�ͼ��padding������
	////������ֵ��һ����0~1֮�䣬������ΪmagnitudeImage�е����������Ǹ����ͣ���ʱ��imshow()����ʾ�������Ὣ����ֵ����255�������Ҫ��һ����0~1֮��	
	cv::normalize(magnitudeImage, magnitudeImage, 0, 1, cv::NORM_MINMAX);
	cv::Mat a(cv::Size(2 * magnitudeImage.cols, 2 * magnitudeImage.rows), CV_32F);

	cv::Mat q0(a(cv::Rect(0, 0, magnitudeImage.cols, magnitudeImage.rows)));

	cv::Mat q1(a(cv::Rect(magnitudeImage.cols, 0, magnitudeImage.cols, magnitudeImage.rows)));

	cv::Mat q2(a(cv::Rect(0, magnitudeImage.rows, magnitudeImage.cols, magnitudeImage.rows)));

	cv::Mat q3(a(cv::Rect(magnitudeImage.cols, magnitudeImage.rows, magnitudeImage.cols, magnitudeImage.rows)));

	magnitudeImage.copyTo(q0);
	magnitudeImage.copyTo(q1);
	magnitudeImage.copyTo(q2);
	magnitudeImage.copyTo(q3);
	//ԭʼ����Ƶ��
	cv::Mat IMAGE(a(cv::Rect(magnitudeImage.cols/2, magnitudeImage.rows/2, magnitudeImage.cols, magnitudeImage.rows)));
	cv::Mat IMAGE_8;
	normalize(IMAGE, IMAGE_8, 0, 255, CV_MINMAX);
	IMAGE_8.convertTo(IMAGE_8, CV_8U);
	cv::namedWindow("spectrum magnitude", WINDOW_NORMAL);//���ڴ���
	imshow("spectrum magnitude", IMAGE_8);
	cv::waitKey();//�ȴ�����
	//����ͼ��
	imwrite("D:\\Allen\\vaa3d\\vaa3d_tools\\hackathon\\wanwan\\1\\pinpu.tif", IMAGE_8);

	//------------------------------------------test--------------------------------------------
	//������ʵ���鲿�ϳ�һ�����󣬽�������ƽ�ƣ���ԭ��0,2�У�ͼ��
   /* cv:Mat ImgDFT;
	cv::merge(dst1, 2, ImgDFT);
	idft(ImgDFT, ImgDFT, DFT_SCALE | DFT_REAL_OUTPUT);
	Mat imp = ImgDFT;
	normalize(imp, imp, 0, 255, CV_MINMAX);
	imp.convertTo(imp, CV_8U);
	Mat Output = imp(Range(0, magnitudeImage.rows), Range(0, magnitudeImage.cols));
	imshow("Output", Output);
	waitKey();
	cv::destroyAllWindows();*/
	///*cv::Mat image(magnitudeImage(cv::Rect(0, 0, 2*cx,2* cy)));*/
	//imwrite("D:\\Allen\\vaa3d\\vaa3d_tools\\hackathon\\wanwan\\1\\fft.tif", image);
	/*imshow("spectrum magnitude", IMAGE);
	cv::namedWindow("spectrum magnitude", 0);
	cv::waitKey();
	cv::destroyAllWindows();*/
	/*----------------------------------------------------------------------------------------3.creat mask--------------------------------------------------------------------------*/
	int filter_axis_dir = 10;//��ת�Ƕȣ���x������ļн�
	int filter_cutoff = 10;//ֵԽ������Խ��
	int filter_radius = 8;//ֵԽ������Խ��
	//����ˮƽ��״�˲���
	int max2 = (magnitudeImage.cols > magnitudeImage.rows ? magnitudeImage.cols : magnitudeImage.rows);
	int dim_max2 = max2*2;//ȡ�����нϴ��ֵ��2
	//cv::Mat H_max = Mat::ones(dim_max2, dim_max2, CV_32F);//���������
	//cv::Mat H_max(dim_max2, dim_max2, CV_8UC1, cv::Scalar(1));//����double����
	cv::Mat H_max(dim_max2, dim_max2, CV_32F, cv::Scalar(1));//����ȫ1�����
	cv::Mat mask_1(2 * filter_radius, dim_max2, CV_32F, cv::Scalar(0));//0�ڣ�255�ף�20��
	cv::Mat mask_2(dim_max2, 2 * filter_cutoff, CV_32F, cv::Scalar(1));//int rows,int cols,type��20��
	
	int centerpos_max = round(dim_max2 / 2);
	//mask����ֵ
	cv::Mat mask_region_1(H_max(cv::Rect(0, centerpos_max - filter_radius, dim_max2, filter_radius*2)));//ȡ��������,�У���
	mask_1.copyTo(mask_region_1);
	cv::Mat mask_region_2(H_max(cv::Rect(centerpos_max - filter_cutoff, 0, 2 * filter_cutoff, dim_max2)));//ȡ��������,�У���
	mask_2.copyTo(mask_region_2);
	//ʵ�־�����ת
	cv::Point center = cv::Point(centerpos_max, centerpos_max);//��ת����
	cv::Mat rot_mat = cv::getRotationMatrix2D(center, filter_axis_dir, 1.0);//��÷���任����
	cv::Size dst_sz(dim_max2, dim_max2);
	cv::warpAffine(H_max, H_max, rot_mat, dst_sz);
	//��ȡ��slice��ͬ��С�ľ���
	cv::Mat filter(H_max(cv::Rect(centerpos_max - round((magnitudeImage.cols - 1) / 2), centerpos_max - round((magnitudeImage.rows - 1) / 2), magnitudeImage.cols, magnitudeImage.rows)));//ȡ��������
	cv::GaussianBlur(filter, filter, cv::Size(11, 11), 5, 5);//sigamaԽ��ƽ��Ч��ģ��Ч��Խ���ԣ��˱���Ϊ������
	cv::multiply(IMAGE, filter, filter);//��ˣ�ȥ����������Ƶ����0
	//mask figure������ƽ�ƺ��
	cv::Mat filter_8;
	normalize(filter, filter_8, 0, 255, CV_MINMAX);
	filter_8.convertTo(filter_8, CV_8U);
	cv::namedWindow("mask figure");
	imshow("mask figure", filter_8);
	cv::waitKey();//�ȴ��û�����
	cv::destroyAllWindows();//����ָ������
	//save image
	imwrite("D:\\Allen\\vaa3d\\vaa3d_tools\\hackathon\\wanwan\\1\\mask.tif", filter_8);
	/*--------------------------------------------------------------------------------4.mask ������ƽ�ƣ���ԭԭʼ�����Ļ�״̬--------------------------------------------------------------*/
	//IMAGE
	//filter  ���������Ӧλ����ֵ��ˣ����ص��Ǿ���

	cv::Mat b(cv::Size(2 * filter.cols, 2 * filter.rows), CV_32F);

	cv::Mat d0(b(cv::Rect(0, 0, filter.cols, filter.rows)));

	cv::Mat d1(b(cv::Rect(filter.cols, 0, filter.cols, filter.rows)));

	cv::Mat d2(b(cv::Rect(0, filter.rows, filter.cols, filter.rows)));

	cv::Mat d3(b(cv::Rect(filter.cols, filter.rows, filter.cols, filter.rows)));

	filter.copyTo(d0);
	filter.copyTo(d1);
	filter.copyTo(d2);
	filter.copyTo(d3);
	cv::Mat ifft_filter(b(cv::Rect(filter.cols / 2, filter.rows / 2, filter.cols, filter.rows)));//��ʱ�˲���iff_filterֻ��ʵ��
	/*----------------------------------------------------------------------------------5.��Ƶ���˲�--------------------------------------------------------------*/
	//Ƶ�ף������ף����Ļ�ֻ��Ϊ��ȷ��mask�ķ�Χ��Ƶ������˲�ʱ������ֻ���Է���
	cv::multiply(dst1[0], ifft_filter, dst1[0]);//��ʵ�����
	cv::multiply(dst1[1], ifft_filter, dst1[1]);//���鲿���
	//ʵ���鲿������ͬ����ʱ���ǶȲ��䣬��λ��λ�ƣ�����
     cv:Mat output;
	//cv::multiply(dst2, FITER, output);//dst2 ���ǵ����ķ����׶���fft�����ʵ�������鲿�������Ľ�����������÷�����IMAGE������ᶪʧ��λ��Ϣ�� 
	cv::merge(dst1, 2, output);//ʵ���鲿��ϣ��õ�����+��λ��Ϣ
	//show �˲����Ƶ��
	cv::Mat output1[] = { cv::Mat_<float>(output), cv::Mat::zeros(output.size(), CV_32F) };
	cv::split(output, output1);
	cv::Mat REAL;
	cv::magnitude(output1[0], output1[1], REAL);
	REAL += cv::Scalar::all(1);
	cv::log(REAL, REAL);
	cv::normalize(REAL, REAL, 0, 1, cv::NORM_MINMAX);
	//�˲���Ƶ��
	cv::Mat REAL_8;
	normalize(REAL, REAL_8, 0, 255, CV_MINMAX);
	REAL_8.convertTo(REAL_8, CV_8U);
	cv::namedWindow("chonggou");
	imshow("chonggou", REAL_8);
	cv::waitKey();
	cv::destroyAllWindows();
	//save image
	imwrite("D:\\Allen\\vaa3d\\vaa3d_tools\\hackathon\\wanwan\\1\\chonggou.tif", REAL_8);
	/*----------------------------------------------------------------------------------6.��Ƶ��ӳ��ؿռ���--------------------------------------------------------------*/

   /* cv:Mat OUTPUT;
	cv::merge(output, 2, OUTPUT);*/
	idft(output, output, DFT_SCALE | DFT_REAL_OUTPUT);
	Mat imp = output;
	normalize(imp, imp, 0, 255, CV_MINMAX);
	imp.convertTo(imp, CV_8U);
	//Mat Output = imp(Range(0, magnitudeImage.rows), Range(0, magnitudeImage.cols));
	cv::namedWindow("Output");
	imshow("Output", imp);
	cv::waitKey();
	cv::destroyAllWindows();
	imwrite("D:\\Allen\\vaa3d\\vaa3d_tools\\hackathon\\wanwan\\1\\slice_stripremove.tif", imp);
	return 0;
}
