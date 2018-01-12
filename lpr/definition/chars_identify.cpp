#include "header/chars_identify.h"
#include "header/character.h"
#include "header/feature.h"
#include "header/config.h"

#include <QDebug>


using namespace cv;

namespace easypr {

CharsIdentify* CharsIdentify::instance_ = nullptr;

CharsIdentify* CharsIdentify::instance()
{
    if (!instance_)
    {
        instance_ = new CharsIdentify;
    }
    return instance_;
}


CharsIdentify::CharsIdentify()
{
    LOAD_ANN_MODEL(ann_, kDefaultAnnPath);
    LOAD_ANN_MODEL(annChinese_, kChineseAnnPath);
    LOAD_ANN_MODEL(annGray_, kGrayAnnPath);
}

/*void CharsIdentify::LoadModel(std::string path)
{
    if (path != std::string(kDefaultAnnPath))
    {
        if (!ann_->empty())
            ann_->clear();
        LOAD_ANN_MODEL(ann_, path);
    }
}*/


std::pair<std::string, std::string> CharsIdentify::identifyChineseGray(cv::Mat input, float& out, bool& isChinese)
{
    cv::Mat feature;
    getGrayPlusProject(input, feature);
    float maxVal = -2;
    int result = 0;
    cv::Mat output(1, kChineseNumber, CV_32FC1);

    annGray_->predict(feature, output);

    for (int j = 0; j < kChineseNumber; j++)
    {
        float val = output.at<float>(j);
        //std::cout << "j:" << j << "val:" << val << std::endl;
        if (val > maxVal)
        {
            maxVal = val;
            result = j;
        }
    }
    // no match
    if (-1 == result)
    {
        result = 0;
        maxVal = 0;
        isChinese = false;
    }
    else if (maxVal > 0.9)
        isChinese = true;

    const char* key = chinese_Chars[result];
    std::string s = key;
    out = maxVal;
    return std::make_pair(s, "");
}


void CharsIdentify::classify(std::vector<CCharacter>& charVec)
{
    size_t charVecSize = charVec.size();

    if (charVecSize == 0)
        return;

    Mat featureRows;
    for (size_t index = 0; index < charVecSize; index++)
    {
        Mat charInput = charVec[index].getCharacterMat();
        Mat feature = charFeatures(charInput, kPredictSize);

        featureRows.push_back(feature);
    }

    cv::Mat output(charVecSize, kCharsTotalNumber, CV_32FC1);

    ann_->predict(featureRows, output);

    for (size_t output_index = 0; output_index < charVecSize; output_index++)
    {
        CCharacter& character = charVec[output_index];
        Mat output_row = output.row(output_index);

        int result = 0;
        float maxVal = -2.f;
        std::string label = "";

        bool isChinses = character.getIsChinese();
        if (!isChinses)
        {
            result = 0;
            int j;
            for (j = 0; j < kCharactersNumber; j++)
            {
                float val = output_row.at<float>(j);
                if (val > maxVal)
                {
                    maxVal = val;
                    result = j;
                }
            }
            label = std::make_pair(kChars[result], kChars[result]).second;
        }
        else
        {
            result = kCharactersNumber;
            for (int j = kCharactersNumber; j < kCharsTotalNumber; j++)
            {
                float val = output_row.at<float>(j);
                if (val > maxVal)
                {
                    maxVal = val;
                    result = j;
                }
            }
            const char* key = kChars[result];
            std::string s = key;
            label = std::make_pair(s, s).second;
        }
        character.setCharacterScore(maxVal);
        character.setCharacterStr(label);
    }
}


void CharsIdentify::classifyChineseGray(std::vector<CCharacter>& charVec)
{
    size_t charVecSize = charVec.size();
    if (charVecSize == 0)
        return;

    Mat featureRows;

    for (size_t index = 0; index < charVecSize; index++)
    {
        Mat charInput = charVec[index].getCharacterMat();
        cv::Mat feature;
        getGrayPlusProject(charInput, feature);
        featureRows.push_back(feature);
    }

    cv::Mat output(charVecSize, kChineseNumber, CV_32FC1);
    annGray_->predict(featureRows, output);

    for (size_t output_index = 0; output_index < charVecSize; output_index++)
    {
        CCharacter& character = charVec[output_index];
        Mat output_row = output.row(output_index);
        bool isChinese = true;

        float maxVal = -2;
        int result = 0;

        for (int j = 0; j < kChineseNumber; j++)
        {
            float val = output_row.at<float>(j);
            //std::cout << "j:" << j << "val:" << val << std::endl;
            if (val > maxVal)
            {
                maxVal = val;
                result = j;
            }
        }

        // no match
        if (-1 == result)
        {
            result = 0;
            maxVal = 0;
            isChinese = false;
        }

        auto index = result + kCharsTotalNumber - kChineseNumber;
        const char* key = kChars[index];
        std::string s = key;

        character.setCharacterScore(maxVal);
        character.setCharacterStr(s);
        character.setIsChinese(isChinese);
    }
}


void CharsIdentify::classifyChinese(std::vector<CCharacter>& charVec)
{
    size_t charVecSize = charVec.size();

    if (charVecSize == 0)
        return;

    Mat featureRows;
    for (size_t index = 0; index < charVecSize; index++)
    {
        Mat charInput = charVec[index].getCharacterMat();
        Mat feature = charFeatures(charInput, kChineseSize);
        featureRows.push_back(feature);
    }

    cv::Mat output(charVecSize, kChineseNumber, CV_32FC1);
    annChinese_->predict(featureRows, output);

    for (size_t output_index = 0; output_index < charVecSize; output_index++)
    {
        CCharacter& character = charVec[output_index];
        Mat output_row = output.row(output_index);
        bool isChinese = true;

        float maxVal = -2;
        int result = 0;

        for (int j = 0; j < kChineseNumber; j++)
        {
            float val = output_row.at<float>(j);
            //std::cout << "j:" << j << "val:" << val << std::endl;
            if (val > maxVal)
            {
                maxVal = val;
                result = j;
            }
        }

        // no match
        if (-1 == result)
        {
            result = 0;
            maxVal = 0;
            isChinese = false;
        }

        auto index = result + kCharsTotalNumber - kChineseNumber;
        const char* key = kChars[index];
        std::string s = key;

        character.setCharacterScore(maxVal);
        character.setCharacterStr(s);
        character.setIsChinese(isChinese);
    }
}


int CharsIdentify::classify(cv::Mat f, float& maxVal, bool isChinses, bool isAlphabet)
{
    int result = 0;

    cv::Mat output(1, kCharsTotalNumber, CV_32FC1);
    ann_->predict(f, output);

    maxVal = -2.f;
    if (!isChinses)
    {
        if (!isAlphabet)
        {
            result = 0;
            for (int j = 0; j < kCharactersNumber; j++)
            {
                float val = output.at<float>(j);
                // std::cout << "j:" << j << "val:" << val << std::endl;
                if (val > maxVal)
                {
                    maxVal = val;
                    result = j;
                }
            }
        }
        else
        {
            result = 0;
            // begin with 11th char, which is 'A'
            for (int j = 10; j < kCharactersNumber; j++)
            {
                float val = output.at<float>(j);
                // std::cout << "j:" << j << "val:" << val << std::endl;
                if (val > maxVal)
                {
                    maxVal = val;
                    result = j;
                }
            }
        }
    }
    else
    {
        result = kCharactersNumber;
        for (int j = kCharactersNumber; j < kCharsTotalNumber; j++)
        {
            float val = output.at<float>(j);
            //std::cout << "j:" << j << "val:" << val << std::endl;
            if (val > maxVal)
            {
                maxVal = val;
                result = j;
            }
        }
    }
    //std::cout << "maxVal:" << maxVal << std::endl;

    return result;
}


bool CharsIdentify::isCharacter(cv::Mat input, std::string& label, float& maxVal, bool isChinese)
{
    cv::Mat feature = charFeatures(input, kPredictSize);
    auto index = static_cast<int>(classify(feature, maxVal, isChinese));

    if (isChinese)
    {
        //std::cout << "maxVal:" << maxVal << std::endl;
    }

    float chineseMaxThresh = 0.2f;

    if (maxVal >= 0.9 || (isChinese && maxVal >= chineseMaxThresh))
    {
        if (index < kCharactersNumber)
            label = std::make_pair(kChars[index], kChars[index]).second;
        else
        {
            const char* key = kChars[index];
            std::string s = key;
            label = std::make_pair(s, s).second;
        }
        return true;
    }
    else
        return false;
}


std::pair<std::string, std::string> CharsIdentify::identify(cv::Mat input, bool isChinese, bool isAlphabet)
{
    cv::Mat feature = charFeatures(input, kPredictSize);
    float maxVal = -2;
    auto index = static_cast<int>(classify(feature, maxVal, isChinese, isAlphabet));
    if (index < kCharactersNumber)
    {
        return std::make_pair(kChars[index], kChars[index]);
    }
    else
    {
        const char* key = kChars[index];
        std::string s = key;
        return std::make_pair(s, s);
    }
}


}
