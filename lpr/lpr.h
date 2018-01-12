//////////////////////////////////////////////////////////////////////////
// Version:		1.0
// Date:	    2018-1-10
// Author:	    huzhanxiong
// Copyright:   huzhanxiong
// Desciption:
// A software to find the License plate you want,
// the part of license plate recognition is from EasyPR.
//////////////////////////////////////////////////////////////////////////
#ifndef LPR_H
#define LPR_H

#include <QMainWindow>
#include <opencv2/opencv.hpp>

#include <QThread>
#include <QObject>
#include <QTimer>


class process_inf   //图片处理相关信息
{
public:
    cv::Mat image_process;

    int current_processNum = 0;
    uint8_t max_diff = 0;
    bool isFind = 0;
    std::vector<std::string> pic_vector;  //待处理的文件位置
    std::vector<std::string> right_platelocat;  //匹配到的文件位置
    std::vector<std::string> plateLocal;  //需匹配的车牌号
    std::vector<cv::Mat> result_palte;  //标记后的图片

    uchar detect_type = 0;

    cv::VideoCapture camera;
    int frame_count = 0;
    int frame_currentCount = 0;
    bool start_task = 0;
    bool isCount = 0;
    bool camera_set = 0;
    bool camera_isRun = 0;
};



class picProcess : public QObject
{
    Q_OBJECT

public:
    bool isRun1 = 0;
    bool isRun2 = 0;

signals:
    void refresh_processBar();
    void refresh_labelImage();
    void refresh_slideVideo();
    void videoFile_over();
    void process_finish();

public slots:
    void doWork1();
    void doWork2();
};



namespace Ui {
class lpr;
}

class lpr : public QMainWindow
{
    Q_OBJECT

public:
    explicit lpr(QWidget *parent = 0);
    ~lpr();

private:
    Ui::lpr *ui;

public:
    inline QImage Mat_QImage(cv::Mat mat)
    {
        QImage img;
        if(mat.channels() == 3)
            img = QImage((const uchar*)(mat.data), mat.cols, mat.rows,
                         mat.cols*mat.channels(),
                         QImage::Format_RGB888).rgbSwapped();
        else
            img = QImage((const uchar*)(mat.data), mat.cols, mat.rows,
                         mat.cols*mat.channels(),
                         QImage::Format_Indexed8);
        return img;
    }

private slots:
    void on_path_but_file_clicked();
    void on_searchStart_but_file_clicked();
    void on_next_but_clicked();
    void on_previous_but_clicked();
    void on_plateInfo_clearBut_file_clicked();
    void on_searchStop_but_file_clicked();    
    void on_operatePla_but_file_clicked();
    void on_path_but_video_clicked();
    void on_operatePla_but_video_clicked();
    void on_searchStart_but_video_clicked();
    void on_searchStop_but_video_clicked();
    void on_maxdiff_box_video_currentIndexChanged(int index);
    void on_plateInfo_clearBut_video_clicked();

public:
    QString file_path_tab1;
    QString file_path_tab2;
    int pic_num = 0;
    int current_picNum = 0;

signals:
    void operate_process1();
    void operate_process2();

private slots:
    void processBar_refresh();
    void labelImage_refresh();
    void slideVideo_refresh();
    void showVideoFile_over();
    void showFinesh_result();


    void on_maxdiff_box_file_currentIndexChanged(int index);

    void on_checkBoxMSER_video_clicked();

    void on_checkBoxCOLOR_video_clicked();

    void on_checkBoxSOBEL_video_clicked();

    void on_checkBoxMSER_file_clicked();

    void on_checkBoxCOLOR_file_clicked();

    void on_checkBoxSOBEL_file_clicked();

    void on_videoSlider_sliderMoved(int position);

private:
    picProcess *pic_process;
    QThread *process_Thread;
    void startThreadObj();
};

#endif // LPR_H
