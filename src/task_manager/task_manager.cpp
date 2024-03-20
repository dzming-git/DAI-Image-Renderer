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
	isImageHarmonySet(false),
	targetDetectionClient(nullptr),
	isTargetDetectionSet(false),
	targetTrackingClient(nullptr),
	isTargetTrackingSet(false),
	behaviorRecognitionClient(nullptr),
	isBehaviorRecognitionSet(false),
	imageWidth(0), 
	imageHeight(0) {
	
}

TaskManager::TaskInfo::~TaskInfo() {
	if (imageHarmonyClient) {
		imageHarmonyClient->disconnectImageLoader();
		delete imageHarmonyClient;
	}
	if (targetDetectionClient) {
		delete targetDetectionClient;
	}
	if (targetTrackingClient) {
		delete targetTrackingClient;
	}
	if (behaviorRecognitionClient) {
		delete behaviorRecognitionClient;
	}
} 

bool TaskManager::TaskInfo::initImageHarmony(std::string ip, int port, int64_t loaderArgsHash) {
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
	if (!imageHarmonyClient->connectImageLoader(loaderArgsHash)) {
		delete imageHarmonyClient;
		return false;
	}
	return true;
}

bool TaskManager::TaskInfo::initTargetDetection(std::string ip, int port) {
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
	if (!targetDetectionClient->getMappingTable()) {
		return false;
	}
	return true;
}

bool TaskManager::TaskInfo::initTargetTracking(std::string ip, int port) {
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

bool TaskManager::TaskInfo::initBehaviorRecognition(std::string ip, int port) {
	// TODO 缺少输入合法性检测
	isBehaviorRecognitionSet = true;
	behaviorRecognitionIp = ip;
	behaviorRecognitionPort = port;
	// 重置
	if (behaviorRecognitionClient) {
		delete behaviorRecognitionClient;
	}
	behaviorRecognitionClient = new BehaviorRecognitionClient();
	if (!behaviorRecognitionClient->setAddress(behaviorRecognitionIp, behaviorRecognitionPort)) {
		delete behaviorRecognitionClient;
		return false;
	}
	if (!behaviorRecognitionClient->setTaskId(taskId)) {
		delete behaviorRecognitionClient;
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

bool TaskManager::TaskInfo::init() {
	if (!isImageHarmonySet) {
		return false;
	}
	return true;
}

enum class TextDirection {
    TOP_DOWN,
    BOTTOM_UP
};

inline void putTextWithBackColor(cv::Mat &imageOutput, const std::vector<std::string>& texts, cv::Scalar textColor, cv::Scalar backColor, cv::Point textOrg, TextDirection direction) {
    int fontFace = cv::FONT_ITALIC;
    double fontScale = imageOutput.rows * 0.0015;
    int thickness = 1;
    int baseline = 0;

    // 计算单行文本的高度，用于多行文本间的间距
    cv::Size textSize = cv::getTextSize("Tg", fontFace, fontScale, thickness, &baseline); // 使用Tg作为基准，因为它能较好地代表大多数字符的高度
    int lineSpacing = textSize.height + baseline + thickness; // 行间距

    for (size_t i = 0; i < texts.size(); ++i) {
        size_t idx = direction == TextDirection::TOP_DOWN ? i : texts.size() - 1 - i;
        const std::string& text = texts[idx];

        // 计算文本大小和基线
        cv::Size textsize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
        baseline += thickness;

        // 调整原点坐标以防止文本出界
        cv::Point currentOrg = textOrg;
        if(currentOrg.x < 0) currentOrg.x = 0;
        if(currentOrg.x > imageOutput.cols - textsize.width) currentOrg.x = imageOutput.cols - textsize.width;
        if(currentOrg.y < textsize.height) currentOrg.y = textsize.height;
        if(currentOrg.y > imageOutput.rows - lineSpacing * texts.size()) currentOrg.y = imageOutput.rows - lineSpacing * texts.size();

        // 根据方向调整文本绘制位置
        if (direction == TextDirection::TOP_DOWN) {
            currentOrg.y += i * lineSpacing;
        } else { // BOTTOM_UP
            currentOrg.y -= (texts.size() - 1 - i) * lineSpacing;
        }

        // 绘制文本背景
        rectangle(imageOutput, currentOrg + cv::Point(0, baseline), currentOrg + cv::Point(textsize.width, -textsize.height), backColor, -1);
        // 绘制文本
        putText(imageOutput, text, currentOrg, fontFace, fontScale, textColor, thickness, cv::LINE_AA);
    }
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
			putTextWithBackColor(imageOutput, {label}, textColor, boxColor, textorg, TextDirection::TOP_DOWN);
		}
	}
	if (isTargetTrackingSet) {
		std::vector<TargetTrackingClient::Result> results;
		targetTrackingClient->getResultByImageId(imageIdOutput, results);
		for (auto result : results) {
			int imageWidth = imageOutput.cols;
			int imageHeight = imageOutput.rows;
			int64_t personId = result.id;
			// 使用personId哈希生成随机颜色
			std::hash<int64_t> hashFn;
			size_t hashedId = hashFn(personId);
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
			}
			cv::polylines(imageOutput, points, false, cv::Scalar(blue, green, red), 2);
		}
	}
	if (isBehaviorRecognitionSet) {
		std::vector<BehaviorRecognitionClient::Result> results;
		// 告知行为识别器最新的图像ID，只用该图像的bboxes信息
		// 行为识别的结果使用当前最新识别的帧的结果
		behaviorRecognitionClient->informImageId(imageIdOutput);
		behaviorRecognitionClient->getLatestResult(results);
		for (auto result : results) {
			int imageWidth = imageOutput.cols;
			int imageHeight = imageOutput.rows;
			int x1 = result.x1 * imageWidth;
			int x2 = result.x2 * imageWidth;
			int y1 = result.y1 * imageHeight;
			int y2 = result.y2 * imageHeight;
			int personId = result.personId;
			std::hash<int64_t> hashFn;
			size_t hashedId = hashFn(personId);
			std::default_random_engine rng(hashedId);
			std::uniform_int_distribution<int> dist(0, 255);
			int red = dist(rng);
			int green = dist(rng);
			int blue = dist(rng);
			std::vector<std::string> labelConfidences;
			for (const auto& labelConfidencePair : result.labelConfidencePairs) {
				std::string label = labelConfidencePair.first;
				double confidence = labelConfidencePair.second;
				// 格式化标签和置信度为字符串，假设我们希望置信度显示为百分比形式，保留两位小数
				char formattedLabelConfidence[100];
				snprintf(formattedLabelConfidence, sizeof(formattedLabelConfidence), "%s: %.2f%%", label.c_str(), confidence * 100);
				labelConfidences.push_back(formattedLabelConfidence);
			}
			cv::Point textorg(x1, y2);
			cv::Scalar textColor(255, 255, 255);
			cv::Scalar boxColor(0, 0, 255);
			putTextWithBackColor(imageOutput, labelConfidences, textColor, boxColor, textorg, TextDirection::BOTTOM_UP);
		}
	}
	return true;
}