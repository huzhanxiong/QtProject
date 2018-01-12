#ifndef PLATE_DETECT_H
#define PLATE_DETECT_H

#include "header/plate_locate.h"
#include "header/chars_recognise.h"


namespace easypr {

class CPlateDetect : public CCharsRecognise, public CPlateLocate
{
public:
    CPlateDetect();
    ~CPlateDetect();

    int plateDetect(Mat src, std::vector<CPlate> &resultVec, int img_index = 0);

    //inline void setPDLifemode(bool param) { m_plateLocate->setLifemode(param); }
    inline void setMaxPlates(int param) { m_maxPlates = param; }
    inline void setDetectType(int param) { m_type = param; }

    int plateJudgeUsingNMS(const std::vector<CPlate>&, std::vector<CPlate>&, int maxPlates = 5);
    int plateSetScore(CPlate& plate);
    void NMS(std::vector<CPlate> &inVec, std::vector<CPlate> &resultVec, double overlap);

private:
    int m_maxPlates;
    int m_type;
    //CPlateLocate* m_plateLocate;
    cv::Ptr<ml::SVM> svm_;
};

}

#endif // PLATE_DETECT_H
