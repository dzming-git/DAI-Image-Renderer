#include "grpc/servers/image_renderer/image_renderer_server.h"
#include <chrono>
#include <random>
#include <unordered_map>
#include "task_manager/task_manager.h"

ImageRendererServer::ImageRendererServer() {
}

ImageRendererServer::~ImageRendererServer() {
}

grpc::Status ImageRendererServer::getImageByImageId(grpc::ServerContext*, const imageRenderer::GetImageByImageIdRequest *request, imageRenderer::GetImageByImageIdResponse *response) {
    int32_t responseCode = 200;
    std::string responseMessage;
    int64_t taskId = 0;
    auto taskManager = TaskManager::getSingletonInstance();
    request->PrintDebugString();
    try {
        int64_t taskId = request->taskid();
        auto taskInfo = taskManager->getTask(taskId);
        if (nullptr == taskInfo) {
            throw std::runtime_error("Failed to get task. Task ID: " + std::to_string(taskId) + "\n");
        }
        auto imageRequest = request->imagerequest();
        int64_t imageId = imageRequest.imageid();
        std::string format = imageRequest.format();
        bool noImageBuffer = imageRequest.noimagebuffer();
        int paramsCnt = imageRequest.params_size();
        std::vector<int> params(paramsCnt);
        for (int i = 0; i < paramsCnt; ++i) {
            params[i] = imageRequest.params(i);
        }
        int expectedW = imageRequest.expectedw();
        int expectedH = imageRequest.expectedh();
        response->mutable_imageresponse()->set_imageid(0);
        cv::Mat image;
        ImageHarmonyClient::ImageInfo imageInfo;
        imageInfo.height = expectedH;
        imageInfo.width = expectedW;
        imageInfo.imageId = imageId;
        if (!taskInfo->getImage(imageInfo, imageId, image)) {
            throw std::runtime_error("Failed to get image. Task ID: " + std::to_string(taskId) + "\n");
        }
        response->mutable_imageresponse()->set_imageid(imageId);
        response->mutable_imageresponse()->set_height(image.rows);
        response->mutable_imageresponse()->set_width(image.cols);
        // 无参数时，默认使用 .jpg 无损压缩
        if (0 == format.size()) {
            format = ".jpg";
            params = {cv::IMWRITE_PNG_COMPRESSION, 100};
        }
        std::vector<uchar> buf;
        // TODO: 在这里压缩图像会有一些性能冗余
        cv::imencode(format, image, buf, params);
        size_t bufSize = buf.size();
        response->mutable_imageresponse()->set_buffer(&buf[0], buf.size());
    } catch (const std::exception& e) {
        responseCode = 400;
        responseMessage += e.what();
    }
    response->mutable_response()->set_code(responseCode);
    response->mutable_response()->set_message(responseMessage);
    return grpc::Status::OK;
}
