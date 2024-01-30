#include "task_manager/task_manager.h"
#include <random>

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
	targetDetectionClient(nullptr),
	targetTrackingClient(nullptr),
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
		delete imageHarmonyClient;
		return false;
	}
	if (!imageHarmonyClient->setLoaderArgsHash(loaderArgsHash)) {
		delete imageHarmonyClient;
		return false;
	}
	return true;
}

bool TaskManager::TaskInfo::initTargetDetection(std::string ip, std::string port) {
	// TODO 缺少输入合法性检测
	isTargetDetectionSet = true;
	targetDetectionIp = ip;
	targetDetectionPort = port;
	// 重置
	if (targetDetectionClient) {
		delete targetDetectionClient;
	}
	targetDetectionClient = new TargetDetectionClient();
	if (!targetDetectionClient->setAddress(targetDetectionIp, targetDetectionPort)) {
		delete targetDetectionClient;
		return false;
	}
	if (!targetDetectionClient->setTaskId(taskId)) {
		delete targetDetectionClient;
		return false;
	}
	return true;
}

bool TaskManager::TaskInfo::initTargetTracking(std::string ip, std::string port) {
	// TODO 缺少输入合法性检测
	isTargetTrackingSet = true;
	targetTrackingIp = ip;
	targetTrackingPort = port;
	// 重置
	if (targetTrackingClient) {
		delete targetTrackingClient;
	}
	targetTrackingClient = new TargetTrackingClient();
	if (!targetTrackingClient->setAddress(targetTrackingIp, targetTrackingPort)) {
		delete targetTrackingClient;
		return false;
	}
	if (!targetTrackingClient->setTaskId(taskId)) {
		delete targetTrackingClient;
		return false;
	}
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

bool TaskManager::TaskInfo::addInterestLaebl(std::string label) {
	interestLabelsSet.insert(label);
}

bool TaskManager::TaskInfo::start() {
	if (!isImageHarmonySet) {
		return false;
	}
	// 初始化 target detection
	if (isTargetDetectionSet) {
		// TODO 没有设置循环上限
		while (true) {
			targetDetection::ModelState modelState;
			if (!targetDetectionClient->checkModelState(taskId, modelState)) {
				std::cout << "connect target detect failed" << std::endl;
				isTargetDetectionSet = false;
				delete targetDetectionClient;
			}
			if (targetDetection::ModelState::NotSet == modelState) {
				std::cout << "model is not set" << std::endl;
				isTargetDetectionSet = false;
				delete targetDetectionClient;
			}
			else if (targetDetection::ModelState::NotLoaded == modelState) {
				targetDetectionClient->loadModel(taskId);
			}
			else if (targetDetection::ModelState::Loading == modelState) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			else if (targetDetection::ModelState::LoadingCompleted == modelState) {
				break;
			}
		}
	}
	if (isTargetDetectionSet) {
		targetDetectionClient->getMappingTable();
	}

	// TODO 目标追踪、行为识别
	// stopRequested = false;
	// taskThread = new std::thread([this](){
	// 	stopped = false;



    //     while (!stopRequested) {
	// 		// TODO 任务的循环主体

	// 		cv::imwrite(std::to_string(taskId) + ".jpg", image);
	// 		// cv::imshow(std::to_string(taskId), image);
	// 		// cv::waitKey(1);
	// 	}
	// 	stopped = true;
    // });
	// taskThread->detach();
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

bool TaskManager::TaskInfo::getImage(ImageHarmonyClient::ImageInfo imageInfo, int64_t &imageIdOutput, cv::Mat &imageOutput) {
	if (!imageHarmonyClient->getImageByImageId(imageInfo, imageIdOutput, imageOutput)) {
		return false;
	}
	// TODO 未来使用GPU渲染
	if (isTargetDetectionSet) {
		std::vector<TargetDetectionClient::Result> results;
		targetDetectionClient->getResultByImageId(imageIdOutput, results);
		for (auto result : results) {
			// 
			if (!interestLabelsSet.empty() && interestLabelsSet.find(result.label) != interestLabelsSet.end()) {
				continue;
			}
			int imageWidth = imageOutput.cols;
			int imageHeight = imageOutput.rows;
			int x1 = result.x1 * imageWidth;
			int x2 = result.x2 * imageWidth;
			int y1 = result.y1 * imageHeight;
			int y2 = result.y2 * imageHeight;
			std::string label = result.label;

			int width = ceil(imageOutput.rows * .006);
			int fontFace = cv::FONT_ITALIC;
			double fontScale = imageOutput.rows * 0.0015;
			int thickness = 1;
			int baseline = 0;
			cv::Size textsize = cv::getTextSize(label, fontFace, fontScale, thickness, &baseline);
			baseline += thickness;

			cv::Point textorg(x1, y1);
			cv::Scalar textColor(255, 255, 255);
			cv::Scalar boxColor(0, 0, 255);
			if(textorg.x < 0) textorg.x = 0;
			if(textorg.x > imageOutput.cols - textsize.width) textorg.x = imageOutput.cols - textsize.width;
			if(textorg.y < textsize.height + 1) textorg.y = textsize.height + 1;
			if(textorg.y > imageOutput.rows) textorg.y = imageOutput.rows;
			rectangle(imageOutput, textorg + cv::Point(width / 2, 0), cv::Point(x2, y2), boxColor, width); // 框
			rectangle(imageOutput, textorg, textorg + cv::Point(textsize.width, -textsize.height - 1), boxColor, -1); // label背景
//            putText(imageOutput, label, textorg, fontFace, fontScale, Scalar::all(255), thickness, LINE_AA); // label 白色
			putText(imageOutput, label, textorg, fontFace, fontScale, textColor, thickness, cv::LINE_AA); // label 反色
		}
	}
	if (isTargetTrackingSet) {
		std::vector<TargetTrackingClient::Result> results;
		targetTrackingClient->getResultByImageId(imageIdOutput, results);
		for (auto result : results) {
			int imageWidth = imageOutput.cols;
			int imageHeight = imageOutput.rows;
			int64_t id = result.id;
			// 使用id哈希生成随机颜色
			std::hash<int64_t> hashFn;
			size_t hashedId = hashFn(id);
			std::default_random_engine rng(hashedId);
			std::uniform_int_distribution<int> dist(0, 255);
			int red = dist(rng);
			int green = dist(rng);
			int blue = dist(rng);

			auto bboxs = result.bboxs;
			std::vector<cv::Point> points(bboxs.size());
			for (int i = 0; i < bboxs.size(); ++i) {
				int xBottomMiddle = (bboxs[i].x1 + bboxs[i].x2) * imageWidth / 2;
				int yBottomMiddle = bboxs[i].y2 * imageHeight;
				points[i] = cv::Point(xBottomMiddle, yBottomMiddle);
				// int x1 = bboxs[i].x1 * imageWidth;
				// int y1 = bboxs[i].y1 * imageHeight;
				// int x2 = bboxs[i].x2 * imageWidth;
				// int y2 = bboxs[i].y2 * imageHeight;
			}
			cv::polylines(imageOutput, points, false, cv::Scalar(blue, green, red), 2);
		}
	}
	return true;
}