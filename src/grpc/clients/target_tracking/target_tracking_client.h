/*****************************************************************************
*  Copyright © 2023 - 2023 dzming.                                           *
*                                                                            *
*  @file     target_tracking_client.h                                       *
*  @brief                                                                    *
*  @author   dzming                                                          *
*  @email    dzm_work@163.com                                                *
*                                                                            *
*----------------------------------------------------------------------------*
*  Remark  :                                                                 *
*****************************************************************************/

#ifndef _TARGET_TRACKING_CLIENT_H_
#define _TARGET_TRACKING_CLIENT_H_

#include <string>
#include <vector>
#include <grpc++/grpc++.h>
#include <mutex>
#include <opencv2/opencv.hpp>
#include "protos/target_tracking/target_tracking.grpc.pb.h"
#include "protos/target_tracking/target_tracking.pb.h"

class TargetTrackingClient {
public:
    TargetTrackingClient();
    ~TargetTrackingClient();
    struct BoundingBox {
        double x1;
        double y1;
        double x2;
        double y2;
    };
    struct Result {
        int id;
        std::vector<TargetTrackingClient::BoundingBox> bboxs;
    };

    bool setAddress(std::string ip, int port);
    bool setTaskId(int64_t taskId);
    bool getResultByImageId(int64_t imageId, std::vector<TargetTrackingClient::Result>& results);
private:
    std::mutex stubMutex;
    targetTracking::Communicate::Stub* stub;
    int64_t taskId;
    std::atomic<bool> shouldStop;
};

#endif /* _TARGET_TRACKING_CLIENT_H_ */