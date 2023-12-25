#include "task_manager/task_manager.h"

TaskManager* TaskManager::instance = nullptr;
pthread_mutex_t TaskManager::lock;

TaskManager* TaskManager::getSingletonInstance() {
	if (nullptr == instance) {
		pthread_mutex_lock(&lock);
		if (nullptr == instance) {
			instance = new TaskManager();
		}
        pthread_mutex_unlock(&lock);
	}
	return instance;
}

TaskManager::TaskManager() {

}

bool TaskManager::addTask(int64_t taskId) {
	// 已存在
	if (taskMap.find(taskId) != taskMap.end()) {
		return false;
	}
	taskMap[taskId] = new TaskManager::TaskInfo(taskId);
	return true;
}

bool TaskManager::deleteTask(int64_t taskId) {
	// 不存在
	if (taskMap.find(taskId) == taskMap.end()) {
		return false;
	}
	delete taskMap[taskId];
	taskMap.erase(taskId);
	return true;
}

TaskManager::TaskInfo* TaskManager::getTask(int64_t taskId) {
	// 不存在
	if (taskMap.find(taskId) == taskMap.end()) {
		return nullptr;
	}
	return taskMap[taskId];
}


TaskManager::TaskInfo::TaskInfo(int64_t taskId): 
	taskId(taskId),
	imageHarmonyClient(nullptr),
	imageWidth(0), 
	imageHeight(0),
	taskThread(nullptr),
	stopRequested(false),
	stopped(true),
	isImageHarmonySet(false),
	isTargetDetectionSet(false) {
	
}

TaskManager::TaskInfo::~TaskInfo() {
	stop();
	if (taskThread) {
		delete taskThread;
	}
	if (imageHarmonyClient) {
		delete imageHarmonyClient;
	}
	if (targetDetectionClient) {
		delete targetDetectionClient;
	}
} 

bool TaskManager::TaskInfo::initImageHarmony(std::string ip, std::string port, int64_t loaderArgsHash) {
	// TODO 缺少输入合法性检测
	isImageHarmonySet = true;
	imageHarmonyIp = ip;
	imageHarmonyPort = port;
	this->loaderArgsHash = loaderArgsHash;
	// 重置
	if (imageHarmonyClient) {
		delete imageHarmonyClient;
	}
	imageHarmonyClient = new ImageHarmonyClient();
	if (!imageHarmonyClient->setAddress(imageHarmonyIp, imageHarmonyPort)) {
		return false;
	}
	if (!imageHarmonyClient->setLoaderArgsHash(loaderArgsHash)) {
		return false;
	}
	return true;
}

bool TaskManager::TaskInfo::setTargetDetectionAddress(std::string ip, std::string port) {
	// TODO 缺少输入合法性检测
	isTargetDetectionSet = true;
	targetDetectionIp = ip;
	targetDetectionIp = port;
	return true;
}

bool TaskManager::TaskInfo::setImageSize(int w, int h) {
	if (w <= 0 || h <= 0) {
		return false;
	}
	imageWidth = w;
	imageHeight = h;
	return true;
}

bool TaskManager::TaskInfo::start() {
	if (!isImageHarmonySet) {
		return false;
	}

	if (isTargetDetectionSet) {
		// TODO targetDetectionClient
	}
	// TODO 目标追踪、行为识别
	stopRequested = false;
	taskThread = new std::thread([this](){
		stopped = false;
        while (!stopRequested) {
			std::cout << "test" << std::endl;
		}
		stopped = true;
    });
	taskThread->detach();
	return true;
}

bool TaskManager::TaskInfo::stop() {
	pthread_mutex_lock(&stopMutex);
	stopRequested = true;
	while (false == stopped) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	pthread_mutex_unlock(&stopMutex);
	return true;
}
