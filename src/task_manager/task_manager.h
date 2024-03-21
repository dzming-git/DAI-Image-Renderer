/*****************************************************************************
*  Copyright © 2023 - 2023 dzming.                                           *
*                                                                            *
*  @file     task_manager.h                                                  *
*  @brief                                                                    *
*  @author   dzming                                                          *
*  @email    dzm_work@163.com                                                *
*                                                                            *
*----------------------------------------------------------------------------*
*  Remark  :                                                                 *
*****************************************************************************/

#ifndef _TASK_MANAGER_H_
#define _TASK_MANAGER_H_

#include <thread>
#include <unordered_map>
#include "grpc/clients/image_harmony/image_harmony_client.h"
#include "grpc/clients/target_detection/target_detection_client.h"
#include "grpc/clients/target_tracking/target_tracking_client.h"
#include "grpc/clients/behavior_recognition/behavior_recognition_client.h"

class TaskManager {
public:
    static TaskManager* getSingletonInstance();
    class TaskInfo;

    bool addTask(int64_t taskId);
    bool deleteTask(int64_t taskId);
    TaskManager::TaskInfo* getTask(int64_t taskId);

private:
    TaskManager();

    static TaskManager* instance;
    static pthread_mutex_t lock;

    std::unordered_map<int64_t, TaskManager::TaskInfo*> taskMap;
};

class TaskManager::TaskInfo {
public:
    TaskInfo(int64_t taskId);
    ~TaskInfo();
    bool initImageHarmony(std::string ip, int port, int64_t loaderArgsHash);
    bool initTargetDetection(std::string ip, int port);
    bool initTargetTracking(std::string ip, int port);
    bool initBehaviorRecognition(std::string ip, int port);
    bool setImageSize(int w, int h);
    bool addInterestLaebl(std::string label);
    bool init();
    bool getImage(ImageHarmonyClient::ImageInfo imageInfo, int64_t &imageIdOutput, cv::Mat &imageOutput);
    bool getImageSize(ImageHarmonyClient::ImageInfo imageInfo, int64_t &imageIdOutput, int& width, int& height);
private:
    int64_t taskId;

    int imageWidth;
    int imageHeight;

    std::unordered_set<std::string> interestLabelsSet;

    // imageHarmony必要
    bool isImageHarmonySet;
    std::string imageHarmonyIp;
    int imageHarmonyPort;
    int64_t loaderArgsHash;
    ImageHarmonyClient* imageHarmonyClient;
    
    // 目标检测
    bool isTargetDetectionSet;
    std::string targetDetectionIp;
    int targetDetectionPort;
    TargetDetectionClient* targetDetectionClient;

    // 目标跟踪
    bool isTargetTrackingSet;
    std::string targetTrackingIp;
    int targetTrackingPort;
    TargetTrackingClient* targetTrackingClient;

    // 行为识别
    bool isBehaviorRecognitionSet;
    std::string behaviorRecognitionIp;
    int behaviorRecognitionPort;
    BehaviorRecognitionClient* behaviorRecognitionClient;
};

#endif /* _TASK_MANAGER_H_ */
