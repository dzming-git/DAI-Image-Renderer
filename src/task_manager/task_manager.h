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
    bool initImageHarmony(std::string ip, std::string port, int64_t loaderArgsHash);
    bool initTargetDetection(std::string ip, std::string port);
    bool setImageSize(int w, int h);
    bool addInterestLaebl(std::string label);
    bool start();
    bool stop();
    bool getImage(ImageHarmonyClient::ImageInfo imageInfo, int64_t &imageIdOutput, cv::Mat &imageOutput);
private:
    int64_t taskId;

    std::thread* taskThread;
    int imageWidth;
    int imageHeight;

    std::unordered_set<std::string> interestLabelsSet;

    // imageHarmony必要
    bool isImageHarmonySet;
    std::string imageHarmonyIp;
    std::string imageHarmonyPort;
    int64_t loaderArgsHash;
    ImageHarmonyClient* imageHarmonyClient;
    
    
    bool isTargetDetectionSet;
    std::string targetDetectionIp;
    std::string targetDetectionPort;
    TargetDetectionClient* targetDetectionClient;

    pthread_mutex_t stopMutex;
    bool stopRequested;
    bool stopped;
};

#endif /* _TASK_MANAGER_H_ */
