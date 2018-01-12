//////////////////////////////////////////////////////////////////////////
// Version:		1.0
// Date:	    2018-1-10
// Author:	    huzhanxiong
// Copyright:   huzhanxiong
// Desciption:
// A software to find the License plate you want,
// the part of license plate recognition is from EasyPR.
//////////////////////////////////////////////////////////////////////////
#include "lpr.h"
#include "ui_lpr.h"
#include <opencv2/opencv.hpp>
#include "header/plate_locate.h"
#include "plate_detect.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <sys/time.h>


using namespace cv;
using namespace std;
using namespace easypr;


void plate_recognize(Mat src, vector<CPlate> &plateVecOut, bool type);
process_inf *process_infNow_file;
process_inf *process_infNow_video;


lpr::lpr(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::lpr), process_Thread(NULL)
{
    ui->setupUi(this);
    this->setWindowTitle("车牌识别系统");

    process_infNow_file = new process_inf;
    process_infNow_video = new process_inf;

    startThreadObj();
}

lpr::~lpr()
{
    if(process_Thread)
        process_Thread->quit();
    pic_process->isRun1 = 0;
    pic_process->isRun2 = 0;
    process_Thread->wait();

    delete process_infNow_file;
    delete process_infNow_video;

    delete ui;
}


/************************ Other Thread ************************/
void lpr::startThreadObj()  //新建线程做图片处理
{
    process_Thread = new QThread;
    pic_process = new picProcess;
    pic_process->moveToThread(process_Thread);
    //不同线程之间的信号连接
    connect(process_Thread, &QThread::finished, process_Thread, &QObject::deleteLater);
    connect(process_Thread, &QThread::finished, pic_process, &QObject::deleteLater);
    connect(this, &lpr::operate_process1, pic_process, &picProcess::doWork1);
    connect(this, &lpr::operate_process2, pic_process, &picProcess::doWork2);
    connect(pic_process, &picProcess::refresh_processBar, this, &lpr::processBar_refresh);
    connect(pic_process, &picProcess::process_finish, this, &lpr::showFinesh_result);
    connect(pic_process, &picProcess::refresh_labelImage, this, &lpr::labelImage_refresh);
    connect(pic_process, &picProcess::refresh_slideVideo, this, &lpr::slideVideo_refresh);
    connect(pic_process, &picProcess::videoFile_over, this, &lpr::showVideoFile_over);
    //开启线程
    if(!process_Thread->isRunning())
        process_Thread->start();
}


void picProcess::doWork1()  //文件处理的循环任务
{
    while(isRun1 && (process_infNow_file->current_processNum < process_infNow_file->pic_vector.size()))
    {
        Mat src = imread(process_infNow_file->pic_vector[process_infNow_file->current_processNum].c_str());
        //cv::resize(src, src, Size(), 0.8, 0.8, INTER_LINEAR);
        if(src.empty())
            continue;

        vector<CPlate> plateVec;
        plate_recognize(src, plateVec, 0);

        bool is_find = 0;
        foreach (CPlate plate, plateVec)  //将识别到的车牌与要搜索的车牌进行对比
        {
            std::string plate_string = plate.getPlateStr();
            //qDebug() << plate_string.c_str();

            foreach(string plateLocalX, process_infNow_file->plateLocal)
            {
                std::string chinese_local(&plateLocalX[0], 3);
                std::string chinese_plate(&plate_string[7], 3);
                uint8_t diff_num = chinese_local == chinese_plate ? 0 : 1;
                for(int i=0; i<6; i++)
                {
                    if(plateLocalX[3+i] == plate_string[10+i])
                        continue;
                    else
                        diff_num++;
                }

                if(diff_num <= process_infNow_file->max_diff && plate_string.size()==16)  //判断字符差异是否超过最大值
                {
                    RotatedRect theRect = plate.getPlatePos();
                    Point2f rect_points[4];
                    theRect.points(rect_points);
                    for (int j = 0; j < 4; j++)
                        line(src, rect_points[j], rect_points[(j + 1) % 4], Scalar(0,0,255), 2, 8);
                    is_find = 1;
                }
            }
        }

        if(is_find)  //存入匹配的图片信息
        {
            process_infNow_file->result_palte.push_back(src);

            process_infNow_file->right_platelocat.push_back
                    (process_infNow_file->pic_vector[process_infNow_file->current_processNum]);
        }

        process_infNow_file->current_processNum++;
        emit refresh_processBar();  //刷新界面中的processBar

        if(!(process_infNow_file->current_processNum < process_infNow_file->pic_vector.size()))
        {
            emit process_finish();  //在界面中显示处理结果
            isRun1 = 0;
        }
    }
}


void picProcess::doWork2()  //视频处理的循环任务
{
    Mat image_rec;

    while(isRun2)
    {
      double time_start, time_over;
      time_start = (double)clock();

        process_infNow_video->camera_isRun = 1;
        if(process_infNow_video->camera_set)  //视频位置设定
        {
          process_infNow_video->camera.set(CV_CAP_PROP_POS_FRAMES, process_infNow_video->frame_currentCount);
          process_infNow_video->camera_set = 0;
        }
        process_infNow_video->camera >> image_rec;
        process_infNow_video->camera_isRun = 0;

        if(process_infNow_video->isCount)  //图像源为视频文件
        {
            process_infNow_video->frame_currentCount++;
            emit refresh_slideVideo();  //刷新界面中的slide

            if(process_infNow_video->frame_currentCount > process_infNow_video->frame_count)  //判断文件是否读取完毕
            {
                isRun2 = 0;
                process_infNow_video->isCount = 0;
                process_infNow_video->start_task = 0;

                emit videoFile_over();  //处理完成
                break;
            }
        }

        if(!image_rec.empty())
        {
            vector<CPlate> plateVec;
            plate_recognize(image_rec, plateVec, 1);

            process_infNow_video->right_platelocat.clear();
            foreach (CPlate plate, plateVec)
            {
                std::string plate_string = plate.getPlateStr();
                //qDebug() << plate_string.c_str();

                foreach(string plateLocalX, process_infNow_video->plateLocal)
                {
                    std::string chinese_local(&plateLocalX[0], 3);
                    std::string chinese_plate(&plate_string[7], 3);
                    uint8_t diff_num = chinese_local == chinese_plate ? 0 : 1;
                    for(int i=0; i<6; i++)
                    {
                        if(plateLocalX[3+i] == plate_string[10+i])
                            continue;
                        else
                            diff_num++;
                    }

                    if(diff_num <= process_infNow_video->max_diff && plate_string.size()==16)
                    {
                        RotatedRect theRect = plate.getPlatePos();
                        Point2f rect_points[4];
                        theRect.points(rect_points);
                        for (int j = 0; j < 4; j++)
                            line(image_rec, rect_points[j], rect_points[(j + 1) % 4], Scalar(0,0,255), 2, 8);

                        process_infNow_video->isFind = 1;

                        process_infNow_video->right_platelocat.push_back(plateLocalX);
                    }
                }
            }
            //!image_rec.copyTo(process_infNow_video->image_process);  /*error*/
            process_infNow_video->image_process = image_rec.clone();
            emit refresh_labelImage();  //刷新界面图像
        }
      time_over = (double)clock();
      qDebug() << "process a image need the time is:" << (time_over - time_start)/1000 << "ms";  /*error*/
    }
}


/************************ ui refresh ************************/
void lpr::processBar_refresh()
{
    ui->plate_processBar->setValue(process_infNow_file->current_processNum*100
                                   / process_infNow_file->pic_vector.size());
}


void lpr::labelImage_refresh()
{
    if(!pic_process->isRun2)
        return;

    Mat pic_show;
    cv::resize(process_infNow_video->image_process, pic_show, Size(600,450), 0, 0, INTER_LINEAR);
    ui->label_image_video->setPixmap(QPixmap::fromImage(Mat_QImage(pic_show)));

    if(process_infNow_video->isFind)
    {
        string searchInfo = "已找到与 ";
        foreach(string plateStr, process_infNow_video->right_platelocat)
            searchInfo += (plateStr + ' ');
        searchInfo += "相近的车牌，并已进行标记";
        ui->searchInfo_edit_video->append(QString::fromStdString(searchInfo));

        process_infNow_video->isFind = 0;
    }
}


void lpr::showFinesh_result()
{
    pic_process->isRun1 = 0;

    ui->searchInfo_edit_file->append("共找到车牌相关图片数:");
    ui->searchInfo_edit_file->append(QString("%1").arg(process_infNow_file->right_platelocat.size()));
    if(process_infNow_file->right_platelocat.size() > 0)
    {
        ui->searchInfo_edit_file->append("路径如下:");
        foreach (string platelocat, process_infNow_file->right_platelocat)
            ui->searchInfo_edit_file->append(platelocat.c_str());
    }
    ui->searchInfo_edit_file->append("搜索结束\r\n");
    ui->searchInfo_label_file->setPixmap(QPixmap::fromImage(QImage(":/image/image/green.png")));

    if(process_infNow_file->result_palte.size()>0)
    {
        pic_num = process_infNow_file->result_palte.size();

        Mat pic_show;
        cv::resize(process_infNow_file->result_palte[current_picNum], pic_show, Size(600,450), 0, 0, INTER_LINEAR);
        QImage img = Mat_QImage(pic_show);
        ui->label_image_file->setPixmap(QPixmap::fromImage(img));
    }
}


void lpr::slideVideo_refresh()
{
    ui->videoSlider->setValue(process_infNow_video->frame_currentCount);
}


void lpr::showVideoFile_over()
{
    ui->searchInfo_edit_video->append("搜索结束\r\n");
    ui->searchInfo_label_video->setPixmap(QPixmap::fromImage(QImage(":/image/image/green.png")));
}
/****************************************************/


/************************ Plate Operatoe ************************/
void plate_recognize(Mat src, vector<CPlate> &plateVecOut, bool type)  //车牌识别函数入口
{
    CPlateDetect lpr;

    lpr.setLifemode(true);
    lpr.setMaxPlates(10);

    if(!type)
        lpr.setDetectType(process_infNow_file->detect_type);
    else
        lpr.setDetectType(process_infNow_video->detect_type);

    // resize to uniform sizes
    float scale = 1.f;
    Mat img = uniformResize(src, scale);

    // 1. plate detect
    std::vector<CPlate> plateVec;

    int resultPD = lpr.plateDetect(img, plateVec);

    if (resultPD == 0)
    {
        size_t num = plateVec.size();
        for (size_t j = 0; j < num; j++)
        {
            CPlate& item = plateVec.at(j);
            Mat plateMat = item.getPlateMat();

            // scale the rect to src;
            item.setPlateScale(scale);
            RotatedRect rect = item.getPlatePos();
            item.setPlatePos(scaleBackRRect(rect, 1.f / scale));

            // get plate color
            Color color = item.getPlateColor();
            if (color == UNKNOWN)
            {
                color = getPlateType(plateMat, true);
                item.setPlateColor(color);
            }
            std::string plateColor = lpr.getPlateColor(color);

            // 2. chars recognize
            std::string plateIdentify = "";
            int resultCR = lpr.charsRecognise(item, plateIdentify);
            if (resultCR == 0)
            {
                std::string license = plateColor + ":" + plateIdentify;
                item.setPlateStr(license);
                plateVecOut.push_back(item);
            }
            else
            {
                std::string license = plateColor;
                item.setPlateStr(license);
                plateVecOut.push_back(item);
            }
        }
    }
}


QFileInfoList GetFileList(QString path)  //获取文件列表
{
    QDir dir(path);
    if(dir.exists())
    {
        QFileInfoList file_list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
        QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

        for(int i=0; i<folder_list.size(); i++)
        {
            QString name = folder_list.at(i).absoluteFilePath();
            QFileInfoList child_file_list = GetFileList(name);
            file_list.append(child_file_list);
        }

        return file_list;
    }
}


bool plate_define(string platex)  //车牌格式检验
{
    if(platex.size() != 9)
        return 0;

    string chinese_list("川鄂赣甘贵桂黑沪冀津京吉辽鲁蒙闽宁青琼陕苏晋皖湘新豫渝粤云藏浙");
    string chinese(&platex[0], 3);
    for(int i=0; i<chinese_list.size(); i+=3)
    {
        if( chinese == string(&chinese_list[i], 3) )
            break;
        else if(i == chinese_list.size()-3)
            return 0;
    }
    string letter_list("ABCDEFGHJKLMNPQRSTUVWXYZ");
    if(string(&platex[3], 1).find_first_of(letter_list) == string::npos)
        return 0;

    string string_list = letter_list + "0123456789";
    for(int i=4; i<9; i++)
        if(string(&platex[i], 1).find_first_of(string_list) == string::npos)
            return 0;

    return 1;
}
/****************************************************/


/************************ table_file Operatoe ************************/
void lpr::on_path_but_file_clicked()  //文件选择
{
    file_path_tab1 = QFileDialog::getExistingDirectory(0, "搜索路径选择");
    ui->path_edit_file->setText(file_path_tab1);
}


void lpr::on_searchStart_but_file_clicked()  //开始搜索
{
    if(pic_process->isRun2)
    {
        QMessageBox warning(QMessageBox::Warning, "警告",
                            "有其他任务在进行，请先暂停或等其执行完", 0, this);
        warning.addButton("确定", QMessageBox::AcceptRole);
        if(warning.exec() == QMessageBox::AcceptRole)
            return;
    }
    if(pic_process->isRun1)
    {
        pic_process->isRun1 = 0;
        ui->searchInfo_edit_file->append("搜索结束\r\n");
        ui->searchInfo_label_file->setPixmap(QPixmap::fromImage(QImage(":/image/image/green.png")));
    }

    //属性初始化
    pic_num = 0;
    current_picNum = 0;
    ui->label_image_file->clear();
    process_infNow_file->pic_vector.clear();
    process_infNow_file->result_palte.clear();
    process_infNow_file->right_platelocat.clear(); 
    process_infNow_file->current_processNum = 0;

    if(ui->searchStop_but_file->text() == ("继续搜索"))
        ui->searchStop_but_file->setText("暂停搜索");

    if(file_path_tab1.isEmpty())
    {
        QMessageBox warning(QMessageBox::Warning, "文件打开错误", "请选择文件路径", 0, this);
        warning.addButton("确定", QMessageBox::AcceptRole);
        if(warning.exec() == QMessageBox::AcceptRole)
            return;
    }
    if(!process_infNow_file->detect_type)
    {
        QMessageBox warning(QMessageBox::Warning, "检测算法未设置", "请选择检测车牌的算法类型", 0, this);
        warning.addButton("确定", QMessageBox::AcceptRole);
        if(warning.exec() == QMessageBox::AcceptRole)
            return;
    }

    QFileInfoList file_list = GetFileList(file_path_tab1);
    ui->searchInfo_edit_file->append("找到文件总数:");
    ui->searchInfo_edit_file->append(QString::number( file_list.size() ));

    QString filestr;
    foreach (QFileInfo fileInfo, file_list)  //筛选有效文件
    {
        filestr = fileInfo.suffix();
        if(filestr != "jpg" && filestr != "png" && filestr != "jpeg" && filestr != "bmp")
            continue;
        else
             process_infNow_file->pic_vector.push_back(fileInfo.filePath().toStdString());
    }
    ui->searchInfo_edit_file->append("有效文件个数:");
    ui->searchInfo_edit_file->append(QString::number( process_infNow_file->pic_vector.size() ));

    if(process_infNow_file->pic_vector.size()>0)
    {
        ui->searchInfo_label_file->setPixmap(QPixmap::fromImage(QImage(":/image/image/red.png")));
        ui->searchInfo_edit_file->append("正在搜索中......");

        pic_process->isRun1 = 1;
        emit operate_process1();  //开始处理
    }
    else
    {
        ui->searchInfo_edit_file->append("搜索结束\r\n");
        ui->searchInfo_label_file->setPixmap(QPixmap::fromImage(QImage(":/image/image/green.png")));
    }
}


void lpr::on_searchStop_but_file_clicked()  //暂停搜索
{
    if((process_infNow_file->current_processNum < process_infNow_file->pic_vector.size()))
    {
        pic_process->isRun1 = !pic_process->isRun1;

        if(pic_process->isRun1)
        {
            ui->searchStop_but_file->setText("暂停搜索");
            ui->searchInfo_label_file->setPixmap(QPixmap::fromImage(QImage(":/image/image/red.png")));
            emit operate_process1();
        }
        else
        {
            ui->searchStop_but_file->setText("继续搜索");
            ui->searchInfo_label_file->setPixmap(QPixmap::fromImage(QImage(":/image/image/green.png")));
        }
    }
}


void lpr::on_next_but_clicked()  //下一张图像
{
    if(pic_num>0)
    {
        current_picNum++;
        if(current_picNum >= pic_num)
            current_picNum = 0;

        Mat pic_show;
        cv::resize(process_infNow_file->result_palte[current_picNum], pic_show, Size(600,450), 0, 0, INTER_LINEAR);
        QImage img = Mat_QImage(pic_show);
        ui->label_image_file->setPixmap(QPixmap::fromImage(img));
    }
}


void lpr::on_previous_but_clicked()  //上一张图像
{
    if(pic_num>0)
    {
        current_picNum--;
        if(current_picNum < 0)
            current_picNum = pic_num-1;

        Mat pic_show;
        cv::resize(process_infNow_file->result_palte[current_picNum], pic_show, Size(600,450), 0, 0, INTER_LINEAR);
        QImage img = Mat_QImage(pic_show);
        ui->label_image_file->setPixmap(QPixmap::fromImage(img));
    }
}


void lpr::on_plateInfo_clearBut_file_clicked()  //清空搜索栏信息
{
    ui->searchInfo_edit_file->clear();
}


void lpr::on_operatePla_but_file_clicked()  //输入要搜索的车牌
{
    string plate = ui->plate_edit_file->text().toStdString();

    switch (ui->comboBoxPla_file->currentIndex())
    {
        case 0:
            if(ui->checkBoxPla_file->isChecked())
            {
                if( !plate_define(plate) )
                {
                    QMessageBox warning(QMessageBox::Warning, "格式错误！",
                                        "请输入正确的车牌", 0, this);
                    warning.addButton("确定", QMessageBox::AcceptRole);
                    if(warning.exec() == QMessageBox::AcceptRole)
                        return;
                }
                else
                    process_infNow_file->plateLocal.push_back(plate);
            }
            else
                process_infNow_file->plateLocal.push_back(plate);

            ui->operatePla_edit_file->clear();
            foreach (string platex, process_infNow_file->plateLocal)
                ui->operatePla_edit_file->append(QString::fromStdString(platex));
            break;

        case 1:
            if(process_infNow_file->plateLocal.size()>0)
                process_infNow_file->plateLocal.erase(process_infNow_file->plateLocal.end());
            else
                break;
            ui->operatePla_edit_file->clear();
            foreach (string platex, process_infNow_file->plateLocal)
                ui->operatePla_edit_file->append(QString::fromStdString(platex));
            break;

        case 2:
            while(process_infNow_file->plateLocal.size()>0)
                process_infNow_file->plateLocal.erase(process_infNow_file->plateLocal.end());
            ui->operatePla_edit_file->clear();
            break;

        default:
            break;
    }
}


void lpr::on_checkBoxMSER_file_clicked()
{
    if(ui->checkBoxMSER_file->isChecked())
        process_infNow_file->detect_type |= easypr::PR_DETECT_CMSER;
    else if(!ui->checkBoxMSER_file->isChecked() && process_infNow_file->detect_type == 0x04)
        ui->checkBoxMSER_file->setChecked(1);
    else
        process_infNow_file->detect_type &= 0x03;
}


void lpr::on_checkBoxCOLOR_file_clicked()
{
    if(ui->checkBoxCOLOR_file->isChecked())
        process_infNow_file->detect_type |= easypr::PR_DETECT_COLOR;
    else if(!ui->checkBoxCOLOR_file->isChecked() && process_infNow_file->detect_type == 0x02)
        ui->checkBoxCOLOR_file->setChecked(1);
    else
        process_infNow_file->detect_type &= 0x05;
}


void lpr::on_checkBoxSOBEL_file_clicked()
{
    if(ui->checkBoxSOBEL_file->isChecked())
        process_infNow_file->detect_type |= easypr::PR_DETECT_SOBEL;
    else if(!ui->checkBoxSOBEL_file->isChecked() && process_infNow_file->detect_type == 0x01)
        ui->checkBoxSOBEL_file->setChecked(1);
    else
        process_infNow_file->detect_type &= 0x06;
}


void lpr::on_maxdiff_box_file_currentIndexChanged(int index)
{
    process_infNow_file->max_diff = index;
}
/****************************************************/


/************************ table_video Operatoe ************************/
void lpr::on_path_but_video_clicked()  //视频文件选择
{
    file_path_tab2 = QFileDialog::getOpenFileName(this, QString("选择读取的视频文件"), "../",
                                    QString("mp4(*.mp4) ;; mkv(*.mkv) ;; wmv(*.wmv) ;; avi(*avi)"));
                                    // type(*.mp3 *.wav *.ape *.flac *.acc *.amr *.ogg *.wma)
    ui->path_edit_video->setText(file_path_tab2);
}


void lpr::on_operatePla_but_video_clicked()  //输入要搜索的车牌
{
    string plate = ui->plate_edit_video->text().toStdString();

    switch (ui->comboBoxPla_video->currentIndex())
    {
        case 0:
            if(ui->checkBoxPla_video->isChecked())
            {
                if( !plate_define(plate) )
                {
                    QMessageBox warning(QMessageBox::Warning, "格式错误！",
                                        "请输入正确的车牌", 0, this);
                    warning.addButton("确定", QMessageBox::AcceptRole);
                    if(warning.exec() == QMessageBox::AcceptRole)
                        return;
                }
                else
                    process_infNow_video->plateLocal.push_back(plate);
            }
            else
                process_infNow_video->plateLocal.push_back(plate);

            ui->operatePla_edit_video->clear();
            foreach (string platex, process_infNow_video->plateLocal)
                ui->operatePla_edit_video->append(QString::fromStdString(platex));
            break;

        case 1:
            if(process_infNow_video->plateLocal.size()>0)
                process_infNow_video->plateLocal.erase(process_infNow_video->plateLocal.end());
            else
                break;
            ui->operatePla_edit_video->clear();
            foreach (string platex, process_infNow_video->plateLocal)
                ui->operatePla_edit_video->append(QString::fromStdString(platex));
            break;

        case 2:
            while(process_infNow_video->plateLocal.size()>0)
                process_infNow_video->plateLocal.erase(process_infNow_video->plateLocal.end());
            ui->operatePla_edit_video->clear();
            break;

        default:
            break;
    }
}


void lpr::on_searchStart_but_video_clicked()  //开始搜索
{
    if(pic_process->isRun1)
    {
        QMessageBox warning(QMessageBox::Warning, "警告",
                            "有其他任务在进行，请先暂停或等其执行完", 0, this);
        warning.addButton("确定", QMessageBox::AcceptRole);
        if(warning.exec() == QMessageBox::AcceptRole)
            return;
    }

    if(pic_process->isRun2)
    {
        pic_process->isRun2 = 0;
        ui->searchInfo_edit_video->append("搜索结束\r\n");
        ui->searchInfo_label_video->setPixmap(QPixmap::fromImage(QImage(":/image/image/green.png")));
    }

    ui->videoSlider->setValue(0);
    ui->label_image_video->clear();
    process_infNow_video->isFind = 0;
    process_infNow_video->isCount = 0;
    process_infNow_video->start_task = 0;
    process_infNow_video->frame_count = 0;
    process_infNow_video->frame_currentCount = 0;

    while(process_infNow_video->camera_isRun)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    if(process_infNow_video->camera.isOpened())
        process_infNow_video->camera.release();


    if(!process_infNow_video->detect_type)
    {
        QMessageBox warning(QMessageBox::Warning, "检测算法未设置", "请选择检测车牌的算法类型", 0, this);
        warning.addButton("确定", QMessageBox::AcceptRole);
        if(warning.exec() == QMessageBox::AcceptRole)
            return;
    }

    if(ui->searchStop_but_video->text() == ("继续搜索"))
        ui->searchStop_but_video->setText("暂停搜索");
    
    if(ui->radioCam_but_video->isChecked())  //选择图像源
    {
        if(process_infNow_video->camera.open(ui->comboBoxCam_video->currentIndex()))
        {
            pic_process->isRun2 = 1;
            process_infNow_video->start_task = 1;
            ui->searchInfo_edit_video->append("正在搜索中......");
            ui->searchInfo_label_video->setPixmap(QPixmap::fromImage(QImage(":/image/image/red.png")));

            emit operate_process2();
        }
        else
        {
            QMessageBox warning(QMessageBox::Warning, "摄像头打开错误", "请检查摄像头是否连接，选择是否正确", 0, this);
            warning.addButton("确定", QMessageBox::AcceptRole);
            if(warning.exec() == QMessageBox::AcceptRole)
                return;
        }
    }
    else if(ui->radioVid_but_video->isChecked())
    {
        if(file_path_tab2.isEmpty())
        {
            QMessageBox warning(QMessageBox::Warning, "文件打开错误", "请选择文件路径", 0, this);
            warning.addButton("确定", QMessageBox::AcceptRole);
            if(warning.exec() == QMessageBox::AcceptRole)
                return;
        }
        else
        {
            if(process_infNow_video->camera.open(file_path_tab2.toStdString().c_str()))
            {
                process_infNow_video->frame_count = process_infNow_video->camera.get(CV_CAP_PROP_FRAME_COUNT);
                ui->videoSlider->setMaximum(process_infNow_video->frame_count);
                process_infNow_video->isCount = 1;

                pic_process->isRun2 = 1;
                process_infNow_video->start_task = 1;

                ui->searchInfo_edit_video->append("正在搜索中......");
                ui->searchInfo_label_video->setPixmap(QPixmap::fromImage(QImage(":/image/image/red.png")));

                emit operate_process2();
            }
            else
            {
                QMessageBox warning(QMessageBox::Warning, "文件打开错误", "请确认文件是否有效", 0, this);
                warning.addButton("确定", QMessageBox::AcceptRole);
                if(warning.exec() == QMessageBox::AcceptRole)
                    return;
            }
        }
    }
    else
    {
        QMessageBox warning(QMessageBox::Warning, "图像源错误", "请选择图像的获取来源", 0, this);
        warning.addButton("确定", QMessageBox::AcceptRole);
        if(warning.exec() == QMessageBox::AcceptRole)
            return;
    }
}


void lpr::on_searchStop_but_video_clicked()  //暂停搜索
{
    if(!process_infNow_video->isCount && !process_infNow_video->start_task)
        return;

    pic_process->isRun2 = !pic_process->isRun2;

    if(pic_process->isRun2)
    {
        ui->searchStop_but_video->setText("暂停搜索");
        pic_process->isRun2 = 1;

        ui->searchInfo_edit_video->append("正在搜索中......");
        ui->searchInfo_label_video->setPixmap(QPixmap::fromImage(QImage(":/image/image/red.png")));

        emit operate_process2();
    }
    else
    {
        ui->searchStop_but_video->setText("继续搜索");
        pic_process->isRun2 = 0;

        ui->searchInfo_edit_video->append("搜索中断\r\n");
        ui->searchInfo_label_video->setPixmap(QPixmap::fromImage(QImage(":/image/image/green.png")));
    }
}


void lpr::on_plateInfo_clearBut_video_clicked()  //清空搜索栏信息
{
    ui->searchInfo_edit_video->clear();
}


void lpr::on_checkBoxMSER_video_clicked()
{
    if(ui->checkBoxMSER_video->isChecked())
        process_infNow_video->detect_type |= easypr::PR_DETECT_CMSER;
    else if(!ui->checkBoxMSER_video->isChecked() && process_infNow_video->detect_type == 0x04)
        ui->checkBoxMSER_video->setChecked(1);
    else
        process_infNow_video->detect_type &= 0x03;
}


void lpr::on_checkBoxCOLOR_video_clicked()
{
    if(ui->checkBoxCOLOR_video->isChecked())
        process_infNow_video->detect_type |= easypr::PR_DETECT_COLOR;
    else if(!ui->checkBoxCOLOR_video->isChecked() && process_infNow_video->detect_type == 0x02)
        ui->checkBoxCOLOR_video->setChecked(1);
    else
        process_infNow_video->detect_type &= 0x05;
}


void lpr::on_checkBoxSOBEL_video_clicked()
{
    if(ui->checkBoxSOBEL_video->isChecked())
        process_infNow_video->detect_type |= easypr::PR_DETECT_SOBEL;
    else if(!ui->checkBoxSOBEL_video->isChecked() && process_infNow_video->detect_type == 0x01)
        ui->checkBoxSOBEL_video->setChecked(1);
    else
        process_infNow_video->detect_type &= 0x06;
}


void lpr::on_maxdiff_box_video_currentIndexChanged(int index)  //修改字符差异最大允许值
{
    process_infNow_video->max_diff = index;
}


void lpr::on_videoSlider_sliderMoved(int position)
{
    if(process_infNow_video->isCount)
    {
        process_infNow_video->frame_currentCount = position;
        process_infNow_video->camera_set = 1;
    }
}
