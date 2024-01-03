#include "grpc/servers/image_renderer/image_renderer_server.h"
#include <chrono>
#include <random>
#include <unordered_map>
#include "task_manager/task_manager.h"

auto taskManager = TaskManager::getSingletonInstance();

ImageRendererServer::ImageRendererServer() {
}

ImageRendererServer::~ImageRendererServer() {
}

grpc::Status ImageRendererServer::getImageByImageId(grpc::ServerContext*, const imageRenderer::GetImageByImageIdRequest *request, imageRenderer::GetImageByImageIdResponse *response) {
    int32_t responseCode = 200;
    std::string responseMessage;
    int64_t taskId = 0;
    request->PrintDebugString();
    try {

    } catch (const std::exception& e) {
        responseCode = 400;
        responseMessage += e.what();
    }
    response->mutable_response()->set_code(responseCode);
    response->mutable_response()->set_message(responseMessage);
    return grpc::Status::OK;
}
