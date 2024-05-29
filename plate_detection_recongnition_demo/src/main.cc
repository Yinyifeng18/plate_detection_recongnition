// Copyright (c) 2021 by Rockchip Electronics Co., Ltd. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
 #include <unistd.h>
#include "RgaUtils.h"
#include "im2d.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/opencv.hpp>

#include "rga.h"
#include "rknn_api.h"
#include <dirent.h>


#include "rknn.hpp"
#include "ThreadPool.hpp"

using std::queue;
using std::time;
using std::time_t;
using std::vector;
using namespace std;
using namespace cv;
int main(int argc, char **argv)
{
  char *model_name1 = NULL;
  char *model_name2 = NULL;
  if (argc != 4)
  {
    printf("Usage: %s <rknn model> <rknn model> <jpg> \n", argv[0]);
    return -1;
  }
  model_name1 = (char *)argv[1]; // 参数二，模型所在路径
  model_name2 = (char *)argv[2]; // 参数三，模型所在路径
  char *image_name = argv[3];   // 参数四, 视频/摄像头
  printf("模型名称:\t%s\n", model_name1);
  printf("模型名称:\t%s\n", model_name2);
  VideoCapture capture;
  namedWindow("Camera FPS");
  if (strlen(image_name) == 1)
    capture.open((int)(image_name[0] - '0'));
  else
    capture.open(image_name);

  rknn_lite *ptr = new rknn_lite(model_name1);
  rknn_lite *ptr_car = new rknn_lite(model_name2);

  struct timeval time;
  gettimeofday(&time, nullptr);
  auto initTime = time.tv_sec * 1000 + time.tv_usec / 1000;

  gettimeofday(&time, nullptr);
  long tmpTime, lopTime = time.tv_sec * 1000 + time.tv_usec / 1000;
  int frames = 0;
  if (!capture.isOpened())
  {
      // 如果无法打开摄像头，则输出提示信息
      cout << "无法打开摄像头" << endl;
      return -1;
  }
  while (true)
  {
    capture >> ptr->ori_img;
    if (!ptr->ori_img.empty())
    {
      ptr->interf();
      frames++;
      if (!ptr->plate_img.empty())
      {
        ptr_car->ori_img = ptr->ori_img;
        ptr_car->plate_img = ptr->plate_img;
        ptr_car->x1= ptr->x1;
        ptr_car->y1= ptr->y1;
        ptr_car->interf_car();
        ptr->ori_img = ptr_car->ori_img;
      }
      imshow("Camera FPS", ptr->ori_img);

      if(frames % 60 == 0){
          gettimeofday(&time, nullptr);
          tmpTime = time.tv_sec * 1000 + time.tv_usec / 1000;
          printf("60帧平均帧率:\t%f帧\n", 60000.0 / (float)(tmpTime - lopTime));
          lopTime = tmpTime;
      }
    }
    // 按下'q'键退出循环
    if (waitKey(1) == 'q')
    {
        break;
    }
  }

  gettimeofday(&time, nullptr);
  printf("\n平均帧率:\t%f帧\n", float(frames) / (float)(time.tv_sec * 1000 + time.tv_usec / 1000 - initTime + 0.0001) * 1000.0);

  capture.release();
  destroyAllWindows();
  return 0;
}
