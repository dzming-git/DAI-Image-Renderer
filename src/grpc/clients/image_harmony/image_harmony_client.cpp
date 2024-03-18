#include "grpc/clients/image_harmony/image_harmony_client.h"

ImageHarmonyClient::ImageHarmonyClient(): stub(nullptr), connectId(0) {
}

ImageHarmonyClient::~ImageHarmonyClient() {
    std::lock_guard<std::mutex> lock(stubMutex);
    if (stub) {
        delete stub;
        stub = nullptr;
    }
}

bool ImageHarmonyClient::setAddress(std::string ip, std::string port) {
    // TODO 重置时未考虑线程安全
    std::shared_ptr<grpc::ChannelInterface> channel = grpc::CreateChannel(ip + ":" + port, grpc::InsecureChannelCredentials());
    std::unique_ptr<imageHarmony::Communicate::Stub> stubTmp = imageHarmony::Communicate::NewStub(channel);
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

bool ImageHarmonyClient::connectImageLoader(int64_t loaderArgsHash) {
    if (nullptr == stub) {
        return false;
    }
    imageHarmony::RegisterImageTransServiceRequest registerImageHarmonyServiceRequest;
    imageHarmony::RegisterImageTransServiceResponse registerImageHarmonyServiceResponse;
    grpc::ClientContext context;
    registerImageHarmonyServiceRequest.set_loaderargshash(loaderArgsHash);
    registerImageHarmonyServiceRequest.set_isunique(false);
    grpc::Status status = stub->registerImageTransService(&context, registerImageHarmonyServiceRequest, &registerImageHarmonyServiceResponse);
    imageHarmony::CustomResponse response = registerImageHarmonyServiceResponse.response();
    int32_t code = response.code();
    if (200 != code) {
        auto message = response.message();
        // TODO 以后改成日志
        std::cout << message << std::endl;
        return false;
    }
    connectId = registerImageHarmonyServiceResponse.connectid();
    if (0 == connectId) {
        return false;
    }
    return true;
}

bool ImageHarmonyClient::disconnectImageLoader() {
    if (0 == connectId) {
        return true;
    }
    if (nullptr == stub) {
        return false;
    }
    imageHarmony::UnregisterImageTransServiceRequest request;
    imageHarmony::UnregisterImageTransServiceResponse response;
    grpc::ClientContext context;
    request.set_connectid(connectId);
    grpc::Status status = stub->unregisterImageTransService(&context, request, &response);
    imageHarmony::CustomResponse customResponse = response.response();
    int32_t code = customResponse.code();
    if (200 != code) {
        auto message = customResponse.message();
        // TODO 以后改成日志
        std::cout << message << std::endl;
        return false;
    }
    return true;
}

bool ImageHarmonyClient::getImageByImageId(ImageHarmonyClient::ImageInfo imageInfo, int64_t& imageIdOutput, cv::Mat& imageOutput) {
    if (nullptr == stub) {
        return false;
    }
    imageHarmony::GetImageByImageIdRequest getImageByImageIdRequest;
    imageHarmony::GetImageByImageIdResponse getImageByImageIdResponse;
    grpc::ClientContext context;

    getImageByImageIdRequest.set_connectid(connectId);
    getImageByImageIdRequest.mutable_imagerequest()->set_imageid(imageInfo.imageId);
    getImageByImageIdRequest.mutable_imagerequest()->set_format(imageInfo.format);
    getImageByImageIdRequest.mutable_imagerequest()->mutable_params()->Add(cv::IMWRITE_JPEG_QUALITY);
    getImageByImageIdRequest.mutable_imagerequest()->mutable_params()->Add(imageInfo.quality);
    getImageByImageIdRequest.mutable_imagerequest()->set_expectedw(imageInfo.width);
    getImageByImageIdRequest.mutable_imagerequest()->set_expectedh(imageInfo.height);
    grpc::Status status = stub->getImageByImageId(&context, getImageByImageIdRequest, &getImageByImageIdResponse);
    
    if (!status.ok()) {
        std::cout << "Error: " << status.error_code() << ": " << status.error_message() << std::endl;
        return false;
    }
    
    auto response = getImageByImageIdResponse.response();
    if (200 != response.code()) {
        std::cout << response.code() << ": " << response.message() << std::endl;
        return false;
    }
    
    imageIdOutput = getImageByImageIdResponse.imageresponse().imageid();

    if (!imageIdOutput) {
        std::cout << "image ID is 0" << std::endl;
        return false;
    }

    std::string buffer = getImageByImageIdResponse.imageresponse().buffer();
    std::vector<uint8_t> vecBuffer(buffer.begin(), buffer.end());
    cv::Mat image = cv::imdecode(vecBuffer, cv::IMREAD_COLOR);
    
    imageOutput = image.clone();
    
    return true;
}