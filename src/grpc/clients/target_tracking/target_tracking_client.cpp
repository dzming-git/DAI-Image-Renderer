#include "grpc/clients/target_tracking/target_tracking_client.h"

TargetTrackingClient::TargetTrackingClient(): stub(nullptr), taskId(0) {
}

TargetTrackingClient::~TargetTrackingClient() {
    if (stub) {
        delete stub;
    }
}

bool TargetTrackingClient::setAddress(std::string ip, std::string port) {
    // TODO 重置时未考虑线程安全
    std::shared_ptr<grpc::ChannelInterface> channel = grpc::CreateChannel(ip + ":" + port, grpc::InsecureChannelCredentials());
    std::unique_ptr<targetTracking::Communicate::Stub> stubTmp = targetTracking::Communicate::NewStub(channel);
    // 重置
    if (stub) {
        delete stub;
    }
    // unique_ptr 转为 普通指针
    stub = stubTmp.get();
    stubTmp.release();
    return true;
}

bool TargetTrackingClient::setTaskId(int64_t taskId) {
    this->taskId = taskId;
    return true;
}

bool TargetTrackingClient::getResultByImageId(int64_t imageId, std::vector<TargetTrackingClient::Result>& results) {
    targetTracking::GetResultByImageIdRequest getResultByImageIdRequest;
    targetTracking::GetResultByImageIdResponse getResultByImageIdResponse;
    grpc::ClientContext context;

    getResultByImageIdRequest.set_taskid(taskId);
    getResultByImageIdRequest.set_imageid(imageId);
    getResultByImageIdRequest.set_wait(true);
    grpc::Status status = stub->getResultByImageId(&context, getResultByImageIdRequest, &getResultByImageIdResponse);
    targetTracking::CustomResponse response = getResultByImageIdResponse.response();
    int32_t code = response.code();
    if (200 != code) {
        auto message = response.message();
        // TODO 以后改成日志
        std::cout << message << std::endl;
        return false;
    }
    int resultsCnt = getResultByImageIdResponse.results_size();
    results.resize(resultsCnt);
    for (int i = 0; i < resultsCnt; ++i) {
        auto result = getResultByImageIdResponse.results(i);
        int id = result.id();
        results[i].id = id;
        auto bboxs = result.bboxs();
        int bboxsCnt = result.bboxs_size();
        results[i].bboxs.resize(bboxsCnt);
        for (int j = 0; j < bboxsCnt; ++j) {
            results[i].bboxs[j].x1 = bboxs[j].x1();
            results[i].bboxs[j].y1 = bboxs[j].y1();
            results[i].bboxs[j].x2 = bboxs[j].x2();
            results[i].bboxs[j].y2 = bboxs[j].y2();
        }
    }
    return true;
}
