#ifndef CHARS_SEGMENT_H
#define CHARS_SEGMENT_H

#include "opencv2/opencv.hpp"
#include "header/config.h"

using namespace cv;
using namespace std;

namespace easypr {

class CCharsSegment
{
public:
    CCharsSegment();

    //! using methods to segment chars in plate
    int charsSegmentUsingOSTU(Mat input, std::vector<Mat>& resultVec, std::vector<Mat>& grayChars, Color color = BLUE);

    bool verifyCharSizes(Mat r);

    // find the best chinese binaranzation method
    void judgeChinese(Mat in, Mat& out, Color plateType);
    void judgeChineseGray(Mat in, Mat& out, Color plateType);

    Mat preprocessChar(Mat in);

    //! to find the position of chinese
    Rect GetChineseRect(const Rect rectSpe);

    //! find the character refer to city, like "suA" A
    int GetSpecificRect(const std::vector<Rect>& vecRect);

    //! Do two things
    //  1.remove rect in the left of city character
    //  2.from the city rect, to the right, choose 6 rects
    int RebuildRect(const std::vector<Rect>& vecRect, std::vector<Rect>& outRect, int specIndex);

    static const int CHAR_SIZE = 20;
    static const int HORIZONTAL = 1;
    static const int VERTICAL = 0;

    static const int DEFAULT_LIUDING_SIZE = 7;
    static const int DEFAULT_MAT_WIDTH = 136;
    static const int DEFAULT_COLORTHRESHOLD = 150;

private:
    int m_theMatWidth;
};

}

#endif // CHARS_SEGMENT_H
