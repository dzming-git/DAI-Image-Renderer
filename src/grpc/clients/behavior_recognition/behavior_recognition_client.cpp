#include "grpc/clients/behavior_recognition/behavior_recognition_client.h"

BehaviorRecognitionClient::BehaviorRecognitionClient(): stub(nullptr), taskId(0) {
}

BehaviorRecognitionClient::~BehaviorRecognitionClient() {
    std::lock_guard<std::mutex> lock(stubMutex);
    if (stub) {
        delete stub;
        stub = nullptr;
    }
}

bool BehaviorRecognitionClient::setAddress(std::string ip, std::string port) {
    // TODO 重置时未考虑线程安全
    std::shared_ptr<grpc::ChannelInterface> channel = grpc::CreateChannel(ip + ":" + port, grpc::InsecureChannelCredentials());
    std::unique_ptr<behaviorRecognition::Communicate::Stub> stubTmp = behaviorRecognition::Communicate::NewStub(channel);
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

bool BehaviorRecognitionClient::setTaskId(int64_t taskId) {
    this->taskId = taskId;
    return true;
}

bool BehaviorRecognitionClient::informImageId(int64_t imageId) {
    behaviorRecognition::InformImageIdRequest request;
    request.set_taskid(this->taskId);
    request.set_imageid(imageId);

    behaviorRecognition::InformImageIdResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub->informImageId(&context, request, &response);

    if (status.ok() && response.response().code() == 200) {
        return true;
    } else {
        std::cerr << "InformImageId failed with code: " << response.response().code()
                  << ", message: " << response.response().message() << std::endl;
        return false;
    }
}

bool BehaviorRecognitionClient::getResultByImageId(int64_t imageId, std::vector<BehaviorRecognitionClient::Result>& results) {
    behaviorRecognition::GetResultByImageIdRequest request;
    request.set_taskid(this->taskId);
    request.set_imageid(imageId);

    behaviorRecognition::GetResultByImageIdResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub->getResultByImageId(&context, request, &response);

    if (status.ok() && response.response().code() == 200) {
        // 正确处理结果的代码...
        for (const auto& result_proto : response.results()) {
            Result result;
            for (const auto& label_info : result_proto.labelinfos()) {
                result.labelConfidencePairs.emplace_back(label_info.label(), label_info.confidence());
            }
            result.personId = result_proto.personid();
            result.x1 = result_proto.x1();
            result.y1 = result_proto.y1();
            result.x2 = result_proto.x2();
            result.y2 = result_proto.y2();
            results.push_back(result);
        }
        return true;
    } else {
        std::cerr << "GetResultByImageId failed with code: " << response.response().code()
                  << ", message: " << response.response().message() << std::endl;
        return false;
    }
}

bool BehaviorRecognitionClient::getLatestResult(std::vector<BehaviorRecognitionClient::Result>& results) {
    behaviorRecognition::GetLatestResultRequest request;
    request.set_taskid(this->taskId);

    behaviorRecognition::GetLatestResultResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub->getLatestResult(&context, request, &response);

    if (status.ok() && response.response().code() == 200) {
        // 正确处理结果的代码...
        for (const auto& result_proto : response.results()) {
            Result result;
            for (const auto& label_info : result_proto.labelinfos()) {
                result.labelConfidencePairs.emplace_back(label_info.label(), label_info.confidence());
            }
            result.personId = result_proto.personid();
            result.x1 = result_proto.x1();
            result.y1 = result_proto.y1();
            result.x2 = result_proto.x2();
            result.y2 = result_proto.y2();
            results.push_back(result);
        }
        return true;
    } else {
        std::cerr << "GetLatestResult failed with code: " << response.response().code()
                  << ", message: " << response.response().message() << std::endl;
        return false;
    }
}
