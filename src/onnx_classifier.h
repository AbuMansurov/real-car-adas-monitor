#pragma once
#include <string>
#include <vector>
#include <onnxruntime_cxx_api.h>

/**
 * @brief Результат классификации стиля вождения.
 */
struct ClassificationResult {
    int label;                  ///< Предсказанный класс (0/1/2)
    float confidence;           ///< Уверенность модели (0.0 - 1.0)
    std::vector<float> scores;  ///< Вероятности всех 3 классов
};

/**
 * @brief Классификатор стиля вождения на основе ONNX модели.
 * 
 * Загружает нейросеть из файла .onnx и параметры нормализации из JSON.
 * Применяет Z-score нормализацию и Softmax для получения вероятностей.
 */
class ONNXClassifier {
public:
    /**
     * @brief Конструктор: загружает модель и параметры нормализации.
     * @param modelPath Путь к файлу .onnx
     * @param paramsPath Путь к файлу normalization_params.json
     * @throws std::runtime_error Если файлы не найдены или модель невалидна
     */
    ONNXClassifier(const std::string& modelPath, const std::string& paramsPath);
    
    /**
     * @brief Классифицирует стиль вождения по 6 признакам телеметрии.
     * @param features Вектор из 6 float: [speed, rpm, throttle, coolant, fuel, intake]
     * @return Структура ClassificationResult с предсказанием и уверенностью
     * @throws std::runtime_error Если размер features != 6
     */
    ClassificationResult predict(const std::vector<float>& features);

private:
    Ort::Env env{ORT_LOGGING_LEVEL_WARNING, "ADAS_Classifier"}; ///< Среда ONNX Runtime
    Ort::Session session{nullptr};                               ///< Сессия модели
    
    std::vector<float> mean_;                                    ///< Параметры нормализации
    std::vector<float> std_;
    std::vector<std::string> classes_;

    std::vector<float> normalize(const std::vector<float>& x) const;
    std::vector<float> softmax(const std::vector<float>& logits) const;
    std::vector<float> parseJsonArray(const std::string& json, const std::string& key) const;
    std::vector<std::string> parseJsonStringArray(const std::string& json, const std::string& key) const;
};