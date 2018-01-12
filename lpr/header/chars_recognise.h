#ifndef CHARS_RECOGNISE_H
#define CHARS_RECOGNISE_H


#include "header/chars_segment.h"
#include "header/chars_identify.h"
#include "header/core_func.h"
#include "header/plate.h"
#include "header/config.h"

namespace easypr {

class CCharsRecognise
{
public:
    CCharsRecognise();
    ~CCharsRecognise();

    int charsRecognise(CPlate& plate, std::string& plateLicense);


    inline std::string getPlateColor(Color in) const
    {
        std::string color = "未知";
        if (BLUE == in) color = "蓝牌";
        if (YELLOW == in) color = "黄牌";
        if (WHITE == in) color = "白牌";
        return color;
    }

private:
    //！字符分割
    CCharsSegment* m_charsSegment;
};

}

#endif // CHARS_RECOGNISE_H
