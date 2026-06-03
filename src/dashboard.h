#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include "obd_parser.h"
#include "onnx_classifier.h"

class Dashboard {
public:
    Dashboard(int width = 640, int height = 480);
    
    // Главный метод отрисовки всей панели
    void draw(cv::Mat& frame, const OBDRecord& rec, const ClassificationResult& cls);

private:
    int width_;
    int height_;

    // Круговой прибор с ЗЕЛЁНОЙ и КРАСНОЙ статичными зонами + стрелка
    void drawGauge(cv::Mat& frame, cv::Point center, int radius,
                   float value, float minV, float maxV,
                   const std::string& label,
                   float greenZoneEnd, float redZoneStart,
                   int num_ticks);
                   
    // Горизонтальная полоса
    void drawLinearGauge(cv::Mat& frame, cv::Rect rect,
                         float value, float minV, float maxV,
                         const std::string& label, const std::string& unit, 
                         const cv::Scalar& color, float warningThreshold, bool warnLow);
                         
    // Метод для отрисовки предупреждений с рамкой
    void drawWarning(cv::Mat& frame, const std::string& text, cv::Point topLeft);
};