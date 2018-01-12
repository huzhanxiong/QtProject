#include "plate_detect.h"
#include "header/config.h"
#include "header/core_func.h"
#include "QDebug"


namespace easypr {

CPlateDetect::CPlateDetect()
{
    m_maxPlates = 5;
    m_type = 0;
}

CPlateDetect::~CPlateDetect()
{
}


int CPlateDetect::plateDetect(Mat src, std::vector<CPlate> &resultVec, int img_index)
{
    std::vector<CPlate> sobel_Plates;
    sobel_Plates.reserve(16);
    std::vector<CPlate> color_Plates;
    color_Plates.reserve(16);
    std::vector<CPlate> mser_Plates;
    mser_Plates.reserve(16);
    std::vector<CPlate> all_result_Plates;
    all_result_Plates.reserve(64);

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            if (!m_type || m_type & PR_DETECT_SOBEL)
            {
                plateSobelLocate(src, sobel_Plates, img_index);
                for (auto plate : sobel_Plates)
                {
                    plate.setPlateLocateType(SOBEL);
                    all_result_Plates.push_back(plate);
                }
            }
        }
        #pragma omp section
        {
            if (!m_type || m_type & PR_DETECT_COLOR)
            {
                plateColorLocate(src, color_Plates, img_index);
                for (auto plate : color_Plates)
                {
                    plate.setPlateLocateType(COLOR);
                    all_result_Plates.push_back(plate);
                }
            }
        }
        #pragma omp section
        {
            if (!m_type || m_type & PR_DETECT_CMSER)
            {
                plateMserLocate(src, mser_Plates, img_index);
                for (auto plate : mser_Plates)
                {
                    plate.setPlateLocateType(CMSER);
                    all_result_Plates.push_back(plate);
                }
            }
        }
    }

    // use nms to judge plate
    LOAD_SVM_MODEL(svm_, kHistSvmPath);
    plateJudgeUsingNMS(all_result_Plates, resultVec, m_maxPlates);
    return 0;
}


// set the score of plate
// 0 is plate, -1 is not.
int CPlateDetect::plateSetScore(CPlate& plate)
{
    Mat features;
    getHistomPlusColoFeatures(plate.getPlateMat(), features);
    float score = svm_->predict(features, noArray(), cv::ml::StatModel::Flags::RAW_OUTPUT);

    // score is the distance of margin，below zero is plate, up is not
    // when score is below zero, the samll the value, the more possibliy to be a plate.
    plate.setPlateScore(score);
    if (score < 0.5)
        return 0;
    else
        return -1;
}


void CPlateDetect::NMS(std::vector<CPlate> &inVec, std::vector<CPlate> &resultVec, double overlap)
{
    std::sort(inVec.begin(), inVec.end());
    std::vector<CPlate>::iterator it = inVec.begin();
    for (; it != inVec.end(); ++it)
    {
        CPlate plateSrc = *it;
        //std::cout << "plateScore:" << plateSrc.getPlateScore() << std::endl;
        Rect rectSrc = plateSrc.getPlatePos().boundingRect();
        std::vector<CPlate>::iterator itc = it + 1;
        for (; itc != inVec.end();)
        {
            CPlate plateComp = *itc;
            Rect rectComp = plateComp.getPlatePos().boundingRect();
            float iou = computeIOU(rectSrc, rectComp);
            if (iou > overlap)
                itc = inVec.erase(itc);
            else
                ++itc;
        }
    }
    resultVec = inVec;
}


int CPlateDetect::plateJudgeUsingNMS(const std::vector<CPlate> &inVec, std::vector<CPlate> &resultVec, int maxPlates)
{
    std::vector<CPlate> plateVec;
    int num = inVec.size();
    bool useCascadeJudge = true;

    for (int j = 0; j < num; j++)
    {
        CPlate plate = inVec[j];
        Mat inMat = plate.getPlateMat();
        int result = plateSetScore(plate);
        if (0 == result)
        {
            if (plate.getPlateLocateType() == CMSER)
            {
                int w = inMat.cols;
                int h = inMat.rows;
                Mat tmpmat = inMat(Rect_<double>(w * 0.05, h * 0.1, w * 0.9, h * 0.8));
                Mat tmpDes = inMat.clone();
                resize(tmpmat, tmpDes, Size(inMat.size()));
                plate.setPlateMat(tmpDes);
                if (useCascadeJudge)
                {
                    int resultCascade = plateSetScore(plate);
                    if (plate.getPlateLocateType() != CMSER)
                        plate.setPlateMat(inMat);
                    if (resultCascade == 0)
                    {
                        if (0)
                        {
                            imshow("tmpDes", tmpDes);
                            waitKey(0);
                            destroyWindow("tmpDes");
                        }
                        plateVec.push_back(plate);
                    }
                    else
                        plateVec.push_back(plate);
                }
            }
            else
                plateVec.push_back(plate);
        }
    }

    std::vector<CPlate> reDupPlateVec;
    double overlap = 0.5;
    // use NMS to get the result plates
    NMS(plateVec, reDupPlateVec, overlap);
    // sort the plates due to their scores
    std::sort(reDupPlateVec.begin(), reDupPlateVec.end());
    // output the plate judge plates
    std::vector<CPlate>::iterator it = reDupPlateVec.begin();
    int count = 0;
    for (; it != reDupPlateVec.end(); ++it)
    {
        resultVec.push_back(*it);
        if (0)
        {
            imshow("plateMat", it->getPlateMat());
            waitKey(0);
            destroyWindow("plateMat");
        }
        count++;
        if (count >= maxPlates)
            break;
    }
    return 0;
}


}
