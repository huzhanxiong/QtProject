这是一款基于Qt对特定车牌进行检索的软件，支持文件及摄像头、视频，其中车牌识别部分的算法主要来自开源项目EasyPR。
软件主要有以下两个界面：

1、文件追踪：对指定路径的图片进行搜索，若搜索到图片中含有特定车牌，会对该车牌进行标记并显示到界面上。

2、视频追踪：选择摄像头或视频文件，对图像中出现的特定车牌进行标记并输出相关信息。

在时序上采用了双线程，一个复杂界面的刷新，一个负责图片的处理。
![image](https://github.com/huzhanxiong/QtProject_License-plate-recognition/blob/master/raw/t1.png)
![image](https://github.com/huzhanxiong/QtProject_License-plate-recognition/blob/master/raw/t2.png)
![image](https://github.com/huzhanxiong/QtProject_License-plate-recognition/blob/master/raw/t3.png)
![image](https://github.com/huzhanxiong/QtProject_License-plate-recognition/blob/master/raw/t4.png)
![image](https://github.com/huzhanxiong/QtProject_License-plate-recognition/blob/master/raw/t5.png)
