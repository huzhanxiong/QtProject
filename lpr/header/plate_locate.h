#ifndef PLATE_LOCAL_H
#define PLATE_LOCAL_H

#include <opencv2/opencv.hpp>
#include "character.h"
#include "header/config.h"
#include "header/plate.h"


using namespace cv;
using namespace std;

namespace easypr {

class CPlateLocate {
public:
    CPlateLocate();

    static const int TYPE = CV_8UC3;
    static const int WIDTH = 136;
    static const int HEIGHT = 36;

    static const int DEFAULT_GAUSSIANBLUR_SIZE = 5;
    static const int SOBEL_SCALE = 1;
    static const int SOBEL_DELTA = 0;
    static const int SOBEL_DDEPTH = CV_16S;
    static const int SOBEL_X_WEIGHT = 1;
    static const int SOBEL_Y_WEIGHT = 0;
    static const int DEFAULT_MORPH_SIZE_WIDTH = 17;  // 17
    static const int DEFAULT_MORPH_SIZE_HEIGHT = 3;  // 3

    static const int DEFAULT_VERIFY_MIN = 1;   // 3
    static const int DEFAULT_VERIFY_MAX = 24;  // 20
    static const int DEFAULT_ANGLE = 60;  // 30

    bool verifySizes(RotatedRect mr);
    int colorSearch(const Mat &src, const Color r, Mat &out, vector<RotatedRect> &outRects);
    bool calcSafeRect(const RotatedRect &roi_rect, const Mat &src, Rect_<float> &safeBoundRect);
    bool rotation(Mat &in, Mat &out, const Size rect_size, const Point2f center, const double angle);
    bool isdeflection(const Mat &in, const double angle, double &slope);
    void affine(const Mat &in, Mat &out, const double slope);

    int deskew(const Mat &src, const Mat &src_b, vector<RotatedRect> &inRects,
                    vector<CPlate> &outPlates, bool useDeteleArea, Color color);

    int sobelFrtSearch(const Mat &src, vector<Rect_<float> > &outRects);
    int sobelSecSearch(Mat &bound, Point2f refpoint, vector<RotatedRect> &outRects);
    int sobelOper(const Mat &in, Mat &out, int blurSize, int morphW, int morphH);
    int sobelSecSearchPart(Mat &bound, Point2f refpoint,
                                         vector<RotatedRect> &outRects);
    int sobelOperT(const Mat &in, Mat &out, int blurSize, int morphW, int morphH);

    int mserSearch(const Mat &src, vector<Mat>& out,
        vector< vector<CPlate> >& out_plateVec, bool usePlateMser, vector<vector<RotatedRect>>& out_plateRRect,
        int img_index = 0, bool showDebug = false);

    int plateColorLocate(Mat src, vector<CPlate> &candPlates, int index);
    int plateSobelLocate(Mat src, vector<CPlate> &candPlates, int index);
    int plateMserLocate(Mat src, vector<CPlate> &candPlates, int index);

    void setLifemode(bool param);

    inline void setGaussianBlurSize(int param) { m_GaussianBlurSize = param; }
    inline int getGaussianBlurSize() const { return m_GaussianBlurSize; }

    inline void setMorphSizeWidth(int param) { m_MorphSizeWidth = param; }
    inline int getMorphSizeWidth() const { return m_MorphSizeWidth; }

    inline void setMorphSizeHeight(int param) { m_MorphSizeHeight = param; }
    inline int getMorphSizeHeight() const { return m_MorphSizeHeight; }

    inline void setVerifyError(float param) { m_error = param; }
    inline float getVerifyError() const { return m_error; }

    inline void setVerifyAspect(float param) { m_aspect = param; }
    inline float getVerifyAspect() const { return m_aspect; }

    inline void setVerifyMin(int param) { m_verifyMin = param; }
    inline void setVerifyMax(int param) { m_verifyMax = param; }

    inline void setJudgeAngle(int param) { m_angle = param; }

protected:
    int m_GaussianBlurSize;

    int m_MorphSizeWidth;
    int m_MorphSizeHeight;

    float m_error;
    float m_aspect;
    int m_verifyMin;
    int m_verifyMax;

    int m_angle;
};

}
#endif // PLATE_LOCAL_H
