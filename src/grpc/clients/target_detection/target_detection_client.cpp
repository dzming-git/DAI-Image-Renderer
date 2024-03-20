#include "grpc/clients/target_detection/target_detection_client.h"

TargetDetectionClient::TargetDetectionClient(): stub(nullptr), taskId(0) {
    shouldStop.store(false);
}

TargetDetectionClient::~TargetDetectionClient() {
    shouldStop.store(true);
    std::lock(stubMutex, labelsMutex); // 同时锁定两个互斥锁
    std::lock_guard<std::mutex> lk1(stubMutex, std::adopt_lock);
    std::lock_guard<std::mutex> lk2(labelsMutex, std::adopt_lock);
    if (stub) {
        delete stub;
        stub = nullptr;
    }
}

bool TargetDetectionClient::setAddress(std::string ip, int port) {
    if (shouldStop.load()) return false;
    // TODO 重置时未考虑线程安全
    std::shared_ptr<grpc::ChannelInterface> channel = grpc::CreateChannel(ip + ":" + std::to_string(port), grpc::InsecureChannelCredentials());
    std::unique_ptr<targetDetection::Communicate::Stub> stubTmp = targetDetection::Communicate::NewStub(channel);
    // 重置
    if (stub) {
        delete stub;
        stub = nullptr;
    }
    // unique_ptr 转为 普通指针
    stub = stubTmp.get();
    stubTmp.release();
    return true;
}

bool TargetDetectionClient::setTaskId(int64_t taskId) {
    if (shouldStop.load()) return false;
    this->taskId = taskId;
    return true;
}

bool TargetDetectionClient::getMappingTable() {
    if (shouldStop.load()) return false;
    if (nullptr == stub) {
        return false;
    }
    targetDetection::GetResultMappingTableRequest getResultMappingTableRequest;
    targetDetection::GetResultMappingTableResponse getResultMappingTableResponse;
    grpc::ClientContext context;

    getResultMappingTableRequest.set_taskid(taskId);
    grpc::Status status = stub->getResultMappingTable(&context, getResultMappingTableRequest, &getResultMappingTableResponse);
    targetDetection::CustomResponse response = getResultMappingTableResponse.response();
    int32_t code = response.code();
    if (200 != code) {
        auto message = response.message();
        // TODO 以后改成日志
        std::cout << message << std::endl;
        return false;
    }
    int labelsCnt = getResultMappingTableResponse.labels_size();
    labels.resize(labelsCnt);
    for (int i = 0; i < labelsCnt; ++i) {
        labels[i] = getResultMappingTableResponse.labels(i);
    }
    return true;
}

bool TargetDetectionClient::getResultByImageId(int64_t imageId, std::vector<TargetDetectionClient::Result>& results) {
    if (shouldStop.load()) return false;
    std::lock_guard<std::mutex> lock(labelsMutex);
    if (nullptr == stub) {
        return false;
    }
    if (labels.empty()) {
        std::cout << "labels is empty" << std::endl;
        return false;
    }
    targetDetection::GetResultIndexByImageIdRequest getResultIndexByImageIdRequest;
    targetDetection::GetResultIndexByImageIdResponse getResultIndexByImageIdResponse;
    grpc::ClientContext context;

    getResultIndexByImageIdRequest.set_taskid(taskId);
    getResultIndexByImageIdRequest.set_imageid(imageId);
    getResultIndexByImageIdRequest.set_wait(true);
    grpc::Status status = stub->getResultIndexByImageId(&context, getResultIndexByImageIdRequest, &getResultIndexByImageIdResponse);
    targetDetection::CustomResponse response = getResultIndexByImageIdResponse.response();
    int32_t code = response.code();
    if (200 != code) {
        auto message = response.message();
        // TODO 以后改成日志
        std::cout << message << std::endl;
        return false;
    }
    int resultsCnt = getResultIndexByImageIdResponse.results_size();
    results.resize(resultsCnt);
    for (int i = 0; i < resultsCnt; ++i) {
        auto result = getResultIndexByImageIdResponse.results(i);
        int c = result.labelid();
        if (c >= labels.size()) {
            std::cout << "Invalid label ID: " << c << std::endl;
            results[i].label = std::to_string(c);
        }
        else {
            results[i].label = labels[c];
        }
        results[i].confidence = result.confidence();
        results[i].x1 = result.x1();
        results[i].y1 = result.y1();
        results[i].x2 = result.x2();
        results[i].y2 = result.y2();
    }
    return true;
}

bool TargetDetectionClient::loadModel(int64_t taskId) {
    if (shouldStop.load()) return false;
    if (nullptr == stub) {
        return false;
    }
    targetDetection::LoadModelRequest loadModelRequest;
    targetDetection::LoadModelResponse loadModelResponse;
    grpc::ClientContext context;

    loadModelRequest.set_taskid(taskId);
    grpc::Status status = stub->loadModel(&context, loadModelRequest, &loadModelResponse);
    targetDetection::CustomResponse response = loadModelResponse.response();
    int32_t code = response.code();
    if (200 != code) {
        auto message = response.message();
        // TODO 以后改成日志
        std::cout << message << std::endl;
        return false;
    }
    return true;
}

bool TargetDetectionClient::checkModelState(int64_t taskId, targetDetection::ModelState& modelState) {
    if (shouldStop.load()) return false;
    if (nullptr == stub) {
        return false;
    }
    targetDetection::CheckModelStateRequest checkModelStateRequest;
    targetDetection::CheckModelStateResponse checkModelStateResponse;
    grpc::ClientContext context;

    checkModelStateRequest.set_taskid(taskId);
    grpc::Status status = stub->checkModelState(&context, checkModelStateRequest, &checkModelStateResponse);
    targetDetection::CustomResponse response = checkModelStateResponse.response();
    int32_t code = response.code();
    if (200 != code) {
        auto message = response.message();
        // TODO 以后改成日志
        std::cout << message << std::endl;
        return false;
    }
    modelState = checkModelStateResponse.modelstate();
    return true;
}