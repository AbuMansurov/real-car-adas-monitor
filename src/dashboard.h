#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include "obd_parser.h"
#include "onnx_classifier.h"

/**
 * @brief Приборная панель автомобиля (Dashboard).
 * 
 * Отрисовывает на cv::Mat размером 640x480:
 * - Спидометр (0-140 км/ч) с зелёной/красной зоной
 * - Тахометр (0-6000 RPM) с красной зоной после 4500
 * - Линейные индикаторы: температура, топливо, дроссель
 * - Стиль вождения и предупреждения
 */
class Dashboard {
public:
    /**
     * @brief Конструктор приборной панели.
     * @param width Ширина кадра (по умолчанию 640)
     * @param height Высота кадра (по умолчанию 480)
     */
    Dashboard(int width = 640, int height = 480);
    
    /**
     * @brief Рисует всю приборную панель на переданном кадре.
     * @param frame[out] Кадр для отрисовки (CV_8UC3, 640x480)
     * @param rec Текущая запись телеметрии OBD
     * @param cls Результат классификации стиля вождения
     */
    void draw(cv::Mat& frame, const OBDRecord& rec, const ClassificationResult& cls);

private:
    int width_;
    int height_;

    void drawGauge(cv::Mat& frame, cv::Point center, int radius,
                   float value, float minV, float maxV,
                   const std::string& label,
                   float greenZoneEnd, float redZoneStart,
                   int num_ticks);
                   
    void drawLinearGauge(cv::Mat& frame, cv::Rect rect,
                         float value, float minV, float maxV,
                         const std::string& label, const std::string& unit, 
                         const cv::Scalar& color, float warningThreshold, bool warnLow);
                         
    void drawWarning(cv::Mat& frame, const std::string& text, cv::Point topLeft);
};