/*****************************************************************************
*  Copyright © 2023 - 2023 dzming.                                           *
*                                                                            *
*  @file     behavior_recognition_client.h                                       *
*  @brief                                                                    *
*  @author   dzming                                                          *
*  @email    dzm_work@163.com                                                *
*                                                                            *
*----------------------------------------------------------------------------*
*  Remark  :                                                                 *
*****************************************************************************/

#ifndef _BEHAVIOR_RECOGNITION_CLIENT_H_
#define _BEHAVIOR_RECOGNITION_CLIENT_H_

#include <string>
#include <vector>
#include <grpc++/grpc++.h>
#include <mutex>
#include <opencv2/opencv.hpp>
#include "protos/behavior_recognition/behavior_recognition.grpc.pb.h"
#include "protos/behavior_recognition/behavior_recognition.pb.h"

class BehaviorRecognitionClient {
public:
    BehaviorRecognitionClient();
    ~BehaviorRecognitionClient();
    struct Result {
        std::vector<std::pair<std::string, double>> labelConfidencePairs;
        unsigned int personId;
        double x1;
        double y1;
        double x2;
        double y2;
    };

    bool setAddress(std::string ip, int port);
    bool setTaskId(int64_t taskId);
    bool informImageId(int64_t imageId);
    bool getResultByImageId(int64_t imageId, std::vector<BehaviorRecognitionClient::Result>& results);
    bool getLatestResult(std::vector<BehaviorRecognitionClient::Result>& results);
private:
    std::mutex stubMutex;
    behaviorRecognition::Communicate::Stub* stub;
    int64_t taskId;
    std::vector<std::string> labels;
    std::atomic<bool> shouldStop;
};



#endif /* _BEHAVIOR_RECOGNITION_CLIENT_H_ */