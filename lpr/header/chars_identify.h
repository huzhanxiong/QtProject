#ifndef CHARS_IDENTIFY_H
#define CHARS_IDENTIFY_H

#include <memory>
#include "opencv2/opencv.hpp"
#include "header/character.h"
#include "header/feature.h"

namespace easypr {

class CharsIdentify
{
public:
    static CharsIdentify* instance();

    int classify(cv::Mat f, float& maxVal, bool isChinses = false, bool isAlphabet = false);
    void classify(std::vector<CCharacter>& charVec);

    void classifyChinese(std::vector<CCharacter>& charVec);
    void classifyChineseGray(std::vector<CCharacter>& charVec);

    std::pair<std::string, std::string> identify(cv::Mat input, bool isChinese = false, bool isAlphabet = false);
    int identify(std::vector<cv::Mat> inputs, std::vector< std::pair<std::string, std::string> >& outputs,
               std::vector<bool> isChineseVec);

    std::pair<std::string, std::string> identifyChineseGray(cv::Mat input, float& result, bool& isChinese);

    bool isCharacter(cv::Mat input, std::string& label, float& maxVal, bool isChinese = false);

private:
    CharsIdentify();

    static CharsIdentify* instance_;

    // binary character classifer
    cv::Ptr<cv::ml::ANN_MLP> ann_;

    // binary character classifer, only for chinese
    cv::Ptr<cv::ml::ANN_MLP> annChinese_;

    // gray classifer, only for chinese
    cv::Ptr<cv::ml::ANN_MLP> annGray_;
};

}

#endif  //  CHARS_IDENTIFY_H
