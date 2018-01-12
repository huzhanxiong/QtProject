#include "header/chars_recognise.h"
#include "header/character.h"
#include <ctime>

#include <QDebug>


namespace easypr {

CCharsRecognise::CCharsRecognise() { m_charsSegment = new CCharsSegment(); }
CCharsRecognise::~CCharsRecognise()
{
    if(m_charsSegment)
    {
        delete m_charsSegment;
        m_charsSegment = NULL;
    }
}


int CCharsRecognise::charsRecognise(CPlate& plate, std::string& plateLicense)
{
    std::vector<Mat> matChars;
    std::vector<Mat> grayChars;
    Mat plateMat = plate.getPlateMat();

    Color color;
    if (plate.getPlateLocateType() == CMSER)
        color = plate.getPlateColor();
    else
    {
        int w = plateMat.cols;
        int h = plateMat.rows;
        Mat tmpMat = plateMat(Rect_<double>(w * 0.1, h * 0.1, w * 0.8, h * 0.8));
        color = getPlateType(tmpMat, true);
    }

    int result = m_charsSegment->charsSegmentUsingOSTU(plateMat, matChars, grayChars, color);

    if (result == 0)
    {
        int num = matChars.size();
        for (int j = 0; j < num; j++)
        {
            Mat charMat = matChars.at(j);
            Mat grayChar = grayChars.at(j);
            if (color != Color::BLUE)
            grayChar = 255 - grayChar;

            bool isChinses = false;
            std::pair<std::string, std::string> character;
            float maxVal;
            if (0 == j)
            {
                isChinses = true;
                bool judge = true;
                character = CharsIdentify::instance()->identifyChineseGray(grayChar, maxVal, judge);
                /** plateLicense.append(character.second); **/
                plateLicense.append(character.first);

                // set plate chinese mat and str
                plate.setChineseMat(grayChar);
                plate.setChineseKey(character.first);
            }
            else if (1 == j)
            {
                isChinses = false;
                bool isAbc = true;
                character = CharsIdentify::instance()->identify(charMat, isChinses, isAbc);
                plateLicense.append(character.second);
            }
            else
            {
                isChinses = false;
                character = CharsIdentify::instance()->identify(charMat, isChinses);
                plateLicense.append(character.second);
            }

            CCharacter charResult;
            charResult.setCharacterMat(charMat);
            charResult.setCharacterGrayMat(grayChar);
            if (isChinses)
                charResult.setCharacterStr(character.first);
            else
                charResult.setCharacterStr(character.second);

            plate.addReutCharacter(charResult);
        }
        if (plateLicense.size() < 7)
            return -1;
    }

    return result;
}





}
