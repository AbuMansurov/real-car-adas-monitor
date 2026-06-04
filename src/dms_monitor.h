#pragma once
#include <opencv2/opencv.hpp>
#include <deque>
#include <string>

/**
 * @brief Состояние водителя в текущий момент времени.
 */
struct DriverState {
    bool face_detected = false;   ///< Лицо обнаружено на кадре
    bool eyes_open = true;        ///< Глаза открыты
    bool looking_forward = true;  ///< Смотрит прямо
    float eye_openness = 1.0f;    ///< Степень открытости глаз (0.0-1.0)
    float head_turn_deg = 0.0f;   ///< Угол поворота головы в градусах
    bool alert_drowsy = false;    ///< Алерт усталости
    bool alert_distracted = false;///< Алерт отвлечения
    cv::Rect face_rect;           ///< Координаты лица на кадре
};

/**
 * @brief Система мониторинга состояния водителя (Driver Monitoring System).
 * 
 * Использует DNN для детекции лиц и Haar Cascade для детекции глаз.
 * Анализирует последние 15 кадров для определения устойчивого закрытия глаз.
 */
class DMSMonitor {
public:
    /**
     * @brief Конструктор: загружает модели детекции.
     * @param faceProto Путь к deploy.prototxt
     * @param faceModel Путь к res10_300x300_ssd_iter_140000.caffemodel
     * @param eyeCascade Путь к haarcascade_eye.xml
     */
    DMSMonitor(const std::string& faceProto, const std::string& faceModel,
               const std::string& eyeCascade);
    
    /**
     * @brief Анализирует кадр и возвращает состояние водителя.
     * @param frame Входной кадр с камеры (BGR)
     * @return Структура DriverState
     */
    DriverState analyze(const cv::Mat& frame);
    
    /** @brief Проверка успешной загрузки моделей */
    bool isLoaded() const { return loaded_; }

private:
    cv::dnn::Net faceNet_;
    cv::CascadeClassifier eyeCascade_;
    std::deque<bool> eyeHistory_;
    bool loaded_ = false;

    cv::Rect detectFace(const cv::Mat& frame);
    float estimateEyeOpenness(const cv::Mat& faceROI);
    float estimateHeadTurn(const cv::Rect& faceRect, int frameWidth);
};