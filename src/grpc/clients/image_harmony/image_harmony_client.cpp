#include "grpc/clients/image_harmony/image_harmony_client.h"

ImageHarmonyClient::ImageHarmonyClient(): stub(nullptr), connectId(0) {
}

ImageHarmonyClient::~ImageHarmonyClient() {
    if (stub) {
        delete stub;
    }
}

bool ImageHarmonyClient::setAddress(std::string ip, std::string port) {
    // TODO 重置时未考虑线程安全
    std::shared_ptr<grpc::ChannelInterface> channel = grpc::CreateChannel(ip + ":" + port, grpc::InsecureChannelCredentials());
    std::unique_ptr<imageHarmony::Communicate::Stub> stubTmp = imageHarmony::Communicate::NewStub(channel);
    // 重置
    if (stub) {
        delete stub;
    }
    // unique_ptr 转为 普通指针
    stub = stubTmp.get();
    stubTmp.release();
    return true;
}

bool ImageHarmonyClient::setLoaderArgsHash(int64_t loaderArgsHash) {
    imageHarmony::RegisterImageTransServiceRequest registerImageHarmonyServiceRequest;
    imageHarmony::RegisterImageTransServiceResponse registerImageHarmonyServiceResponse;
    grpc::ClientContext context;
    registerImageHarmonyServiceRequest.set_loaderargshash(loaderArgsHash);
    registerImageHarmonyServiceRequest.set_isunique(false);
    grpc::Status status = stub->registerImageTransService(&context, registerImageHarmonyServiceRequest, &registerImageHarmonyServiceResponse);
    connectId = registerImageHarmonyServiceResponse.connectid();
    imageHarmony::CustomResponse response = registerImageHarmonyServiceResponse.response();
    int32_t code = response.code();
    if (200 != code) {
        auto message = response.message();
        // TODO 以后改成日志
        std::cout << message << std::endl;
        return false;
    }
    return true;
}
