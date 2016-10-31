#include "DetectionBasedTracker_jni.h"
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect.hpp>
#include <functional>
#include <android/log.h>

#define LOG_TAG "FaceDetection/DetectionBasedTracker"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

#define DEBUG_LOG_ENABLED 0

using namespace std;
using namespace cv;

void callAndLogExceptions(std::function<void()> callable, string fName, JNIEnv *jenv);

class CascadeDetectorAdapter : public DetectionBasedTracker::IDetector {

private:
    CascadeDetectorAdapter();

    cv::Ptr<cv::CascadeClassifier> Detector;

public:

    CascadeDetectorAdapter(cv::Ptr<cv::CascadeClassifier> detector)
            : IDetector(), Detector(detector) {
        LOGD("CascadeDetectorAdapter::Detect::Detect");
        CV_Assert(detector);
    }

    void detect(const cv::Mat &Image, std::vector<cv::Rect> &objects) override {
        if(DEBUG_LOG_ENABLED){
            LOGD("CascadeDetectorAdapter::Detect: begin");
            LOGD("CascadeDetectorAdapter::Detect: scaleFactor=%.2f, minNeighbours=%d, minObjSize=(%dx%d), maxObjSize=(%dx%d)",
                 scaleFactor, minNeighbours, minObjSize.width, minObjSize.height, maxObjSize.width,
                 maxObjSize.height);
        }
        
        Detector->detectMultiScale(Image, objects, scaleFactor, minNeighbours, 0, minObjSize,
                                   maxObjSize);
    }

    virtual ~CascadeDetectorAdapter() {
        LOGD("CascadeDetectorAdapter::Detect::~Detect");
    }

};

struct DetectorAggregator {

    cv::Ptr<CascadeDetectorAdapter> mainDetector;
    cv::Ptr<CascadeDetectorAdapter> trackingDetector;

    cv::Ptr<DetectionBasedTracker> tracker;

    DetectorAggregator(cv::Ptr<CascadeDetectorAdapter> &_mainDetector,
                       cv::Ptr<CascadeDetectorAdapter> &_trackingDetector) :
            mainDetector(_mainDetector),
            trackingDetector(_trackingDetector) {
        CV_Assert(_mainDetector);
        CV_Assert(_trackingDetector);

        DetectionBasedTracker::Parameters DetectorParams;
        tracker = makePtr<DetectionBasedTracker>(mainDetector, trackingDetector, DetectorParams);
    }

};

void callAndLogExceptions(std::function<void()> callable, string fName, JNIEnv *jenv) {
    try {

        callable();

    } catch (cv::Exception &e) {

        fName.append(" caught cv::Exception: %s");

        LOGD(fName.c_str(), e.what());

        jclass je = jenv->FindClass("org/opencv/core/CvException");
        if (!je)
            je = jenv->FindClass("java/lang/Exception");

        jenv->ThrowNew(je, e.what());

    } catch (...) {

        fName.append(" caught unknown exception");
        const char *resultStr = fName.c_str();

        LOGD("%s", resultStr);

        jclass je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, resultStr);

    }
}

JNIEXPORT jlong JNICALL nativeCreateObject(JNIEnv *jenv, jclass, jstring jFileName, jint faceSize) {
    jlong result = 0;

    callAndLogExceptions([&]() {
        const char *jnamestr = jenv->GetStringUTFChars(jFileName, NULL);
        string stdFileName(jnamestr);

        cv::Ptr<CascadeDetectorAdapter> mainDetector = makePtr<CascadeDetectorAdapter>(
                makePtr<CascadeClassifier>(stdFileName));
        cv::Ptr<CascadeDetectorAdapter> trackingDetector = makePtr<CascadeDetectorAdapter>(
                makePtr<CascadeClassifier>(stdFileName));

        result = reinterpret_cast<jlong>(new DetectorAggregator(mainDetector, trackingDetector));

        if (faceSize > 0) {
            mainDetector->setMinObjectSize(Size(faceSize, faceSize));
            //trackingDetector->setMinObjectSize(Size(faceSize, faceSize));
        }

    }, __func__, jenv);

    return result;
}

JNIEXPORT void JNICALL nativeDestroyObject(JNIEnv *jenv, jclass, jlong thiz) {
    callAndLogExceptions([&]() {
        if (thiz != 0) {
            DetectorAggregator *aggregator = reinterpret_cast<DetectorAggregator *>(thiz);
            aggregator->tracker->stop();
            delete aggregator;
        }
    }, __func__, jenv);
}

JNIEXPORT void JNICALL nativeStart(JNIEnv *jenv, jclass, jlong thiz) {
    callAndLogExceptions([&]() {
        DetectorAggregator *aggregator = reinterpret_cast<DetectorAggregator *>(thiz);
        aggregator->tracker->run();
    }, __func__, jenv);

}

JNIEXPORT void JNICALL nativeStop(JNIEnv *jenv, jclass, jlong thiz) {
    callAndLogExceptions([&]() {
        DetectorAggregator *aggregator = reinterpret_cast<DetectorAggregator *>(thiz);
        aggregator->tracker->stop();
    }, __func__, jenv);
}

JNIEXPORT void JNICALL nativeSetFaceSize(JNIEnv *jenv, jclass, jlong thiz, jint faceSize) {
    callAndLogExceptions([&]() {
        if (faceSize > 0) {
            DetectorAggregator *aggregator = reinterpret_cast<DetectorAggregator *>(thiz);
            aggregator->mainDetector->setMinObjectSize(Size(faceSize, faceSize));
        }
    }, __func__, jenv);
}

JNIEXPORT void JNICALL nativeDetect(JNIEnv *jenv, jclass, jlong thiz, jlong imageGray,
                                    jlong faces) {
    callAndLogExceptions([&]() {
        vector<Rect> RectFaces;
        DetectorAggregator *aggregator = reinterpret_cast<DetectorAggregator *>(thiz);
        aggregator->tracker->process(*(reinterpret_cast<Mat *>(imageGray)));
        aggregator->tracker->getObjects(RectFaces);
        *(reinterpret_cast<Mat *>(faces)) = Mat(RectFaces, true);

    }, __func__, jenv);
}


