#include "grpc/servers/service_coordinator/service_coordinator_server.h"
#include <chrono>
#include <random>
#include <unordered_map>
#include "task_manager/task_manager.h"

ServiceCoordinatorServer::ServiceCoordinatorServer() {
}

ServiceCoordinatorServer::~ServiceCoordinatorServer() {
}

grpc::Status ServiceCoordinatorServer::informPreviousServiceInfo(grpc::ServerContext*, const serviceCoordinator::InformPreviousServiceInfoRequest *request, serviceCoordinator::InformPreviousServiceInfoResponse *response) {
    int32_t responseCode = 200;
    std::string responseMessage;
    int64_t taskId = 0;
    bool addTaskFlag = false;
    auto taskManager = TaskManager::getSingletonInstance();
    request->PrintDebugString();
    try {
        taskId = request->taskid();
        std::unordered_map<std::string, std::string> args;
        int argsCnt = request->args_size();
        for (int i = 0; i < argsCnt; ++i) {
            auto arg = request->args(i);
            args.emplace(arg.key(), arg.value());
        }
        auto taskInfo = taskManager->getTask(taskId);
        if (nullptr == taskInfo) {
            if (!taskManager->addTask(taskId)) {
                throw std::runtime_error("Failed to add task. Task ID: " + std::to_string(taskId) + "\n");
            }
            addTaskFlag = true;
            taskInfo = taskManager->getTask(taskId);
            if (nullptr == taskInfo) {
                throw std::runtime_error("Failed to get task. Task ID: " + std::to_string(taskId) + "\n");
            }
        }

        std::string preServiceName = request->preservicename();
        std::string preServiceIp = request->preserviceip();
        std::string preServicePort = request->preserviceport();
        if ("image harmony" == preServiceName) {
            if (args.find("LoaderArgsHash") == args.end()) {
                throw std::runtime_error("LoaderArgsHash is not set.\n");
            }
            int64_t loaderArgsHash = std::stoll(args["LoaderArgsHash"]);
            if (!taskInfo->initImageHarmony(preServiceIp, preServicePort, loaderArgsHash)) {
                throw std::runtime_error("Failed to set image harmony address. Task ID: " + std::to_string(taskId) + "\n");
            }
        }
        else if ("target detection" == preServiceName) {
            if (!taskInfo->initTargetDetection(preServiceIp, preServicePort)) {
                throw std::runtime_error("Failed to set target detection address. Task ID: " + std::to_string(taskId) + "\n");
            }
        }
        else if ("target tracking" == preServiceName) {
            if (!taskInfo->initTargetTracking(preServiceIp, preServicePort)) {
                throw std::runtime_error("Failed to set target tracking address. Task ID: " + std::to_string(taskId) + "\n");
            }
        }
        else {
            throw std::runtime_error("The current version of this service does not support the feature:" + preServiceName + "\n");
        }
    } catch (const std::exception& e) {
        if (addTaskFlag) {
            taskManager->deleteTask(taskId);
        }
        responseCode = 400;
        responseMessage += e.what();
    }
    response->mutable_response()->set_code(responseCode);
    response->mutable_response()->set_message(responseMessage);
    return grpc::Status::OK;
}

grpc::Status ServiceCoordinatorServer::informCurrentServiceInfo(grpc::ServerContext*, const serviceCoordinator::InformCurrentServiceInfoRequest* request, serviceCoordinator::InformCurrentServiceInfoResponse* response) {
    int32_t responseCode = 200;
    std::string responseMessage;
    try {
        std::unordered_map<std::string, std::string> args;
        int argsCnt = request->args_size();
        for (int i = 0; i < argsCnt; ++i) {
            auto arg = request->args(i);
            args.emplace(arg.key(), arg.value());
        }
        // TODO do something
    } catch (const std::exception& e) {
        responseCode = 400;
        responseMessage += e.what();
    }

    response->mutable_response()->set_code(responseCode);
    response->mutable_response()->set_message(responseMessage);
    return grpc::Status::OK;
}

grpc::Status ServiceCoordinatorServer::start(grpc::ServerContext*, const serviceCoordinator::StartRequest* request, serviceCoordinator::StartResponse* response) {
    int32_t responseCode = 200;
    std::string responseMessage;
    auto taskManager = TaskManager::getSingletonInstance();
    try {
        int64_t taskId = request->taskid();
        auto taskInfo = taskManager->getTask(taskId);
        if (nullptr == taskInfo) {
            if (!taskManager->addTask(taskId)) {
                throw std::runtime_error("Failed to add task. Task ID: " + std::to_string(taskId) + "\n");
            }
            taskInfo = taskManager->getTask(taskId);
            if (nullptr == taskInfo) {
                throw std::runtime_error("Failed to get task. Task ID: " + std::to_string(taskId) + "\n");
            }
        }
        if (!taskInfo->start()) {
            throw std::runtime_error("Failed to start task. Task ID: " + std::to_string(taskId) + "\n");
        }
    } catch (const std::exception& e) {
        responseCode = 400;
        responseMessage += e.what();
    }
    response->mutable_response()->set_code(responseCode);
    response->mutable_response()->set_message(responseMessage);
    return grpc::Status::OK;
}

grpc::Status ServiceCoordinatorServer::stop(grpc::ServerContext*, const serviceCoordinator::StopRequest* request, serviceCoordinator::StopResponse* response) {
    int32_t responseCode = 200;
    std::string responseMessage;
    auto taskManager = TaskManager::getSingletonInstance();
    try {
        int64_t taskId = request->taskid();
        auto taskInfo = taskManager->getTask(taskId);
        if (nullptr == taskInfo) {
            if (!taskManager->addTask(taskId)) {
                throw std::runtime_error("Failed to add task. Task ID: " + std::to_string(taskId) + "\n");
            }
            taskInfo = taskManager->getTask(taskId);
            if (nullptr == taskInfo) {
                throw std::runtime_error("Failed to get task. Task ID: " + std::to_string(taskId) + "\n");
            }
        }
        if (!taskInfo->stop()) {
            throw std::runtime_error("Failed to stop task. Task ID: " + std::to_string(taskId) + "\n");
        }
    } catch (const std::exception& e) {
        responseCode = 400;
        responseMessage += e.what();
    }
    response->mutable_response()->set_code(responseCode);
    response->mutable_response()->set_message(responseMessage);
    return grpc::Status::OK;
}
