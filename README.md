定位原理：
当车载摄像头捕获到与其最近的二维码时，通过比较图像中二维码中心点与图像的中心点来计算出车体当前相对该二维码的位置（当车体在二维码的正下方时，二维码将出现在图像中的中心位置），利用二维码的坐标值即可得出车体在坐标系中的位置。
![image](https://github.com/huzhanxiong/QtProject_License-plate-recognition/blob/master/raw/master/red.png)
![image](https://github.com/huzhanxiong/QtProject_License-plate-recognition/blob/master/raw/t1.png)
计算方法：
图像差值计算
