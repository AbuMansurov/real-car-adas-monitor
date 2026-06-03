#include <iostream>
#include <map>
#include <iomanip>
#include <opencv2/opencv.hpp>

#include "obd_parser.h"
#include "onnx_classifier.h"
#include "dashboard.h"

int main() {
    try {
        std::cout << "=== Real Car ADAS Monitor ===" << std::endl;
        std::cout << "Version: 0.4.0 (Dashboard Integration)" << std::endl;
        std::cout << "-----------------------------" << std::endl;

        // ==========================================
        // ЧАСТЬ 1: Загрузка данных и ONNX классификация (Задание 4)
        // ==========================================
        
        // 1. Загружаем CSV с телеметрией
        OBDParser parser;
        int count = parser.load("../data/obd_data.csv");
        if (count < 0) {
            std::cerr << "Critical error: cannot load CSV file!\n";
            std::cerr << "Expected path: ../data/obd_data.csv\n";
            return 1;
        }
        std::cout << "Successfully loaded records: " << count << "\n\n";

        // 2. Инициализируем ONNX классификатор ОДНОЙ строкой (исправленная ошибка C2512)
        ONNXClassifier classifier("../models/driver_classifier.onnx", "../models/normalization_params.json");
        std::cout << "ONNX Model loaded successfully!\n\n";

        // 3. Тестируем классификацию на первых 20 записях
        int limit = std::min(20, count);
        int correctPredictions = 0;

        std::cout << std::left << std::setw(6) << "True" 
                  << std::setw(12) << "Predicted" 
                  << std::setw(12) << "Confidence" << std::endl;
        std::cout << "--------------------------------------" << std::endl;

        for (int i = 0; i < limit; i++) {
            auto rec = parser.getRecord(i);
            
            // Формируем вектор признаков в том же порядке, что и при обучении
            std::vector<float> features = {
                rec.speed_kmh,
                rec.engine_rpm,
                rec.throttle_pos,
                rec.coolant_temp,
                rec.fuel_level,
                rec.intake_air_temp
            };

            // Получаем предсказание
            ClassificationResult pred = classifier.predict(features);

            // Считаем точность
            if (pred.label == rec.label) {
                correctPredictions++;
            }

            // Выводим строку таблицы
            std::cout << std::left << std::setw(6) << rec.label
                      << std::setw(12) << pred.label
                      << std::fixed << std::setprecision(2) << (pred.confidence * 100.0f) << "%" << std::endl;
        }

        double accuracy = (static_cast<double>(correctPredictions) / limit) * 100.0;
        std::cout << "--------------------------------------" << std::endl;
        std::cout << "Accuracy on first 20 records: " << std::fixed << std::setprecision(1) << accuracy << "%" << std::endl;


        // ==========================================
        // ЧАСТЬ 2: Тестирование Dashboard отдельно (Задание 5.4)
        // ==========================================
        std::cout << "\n--- Testing Dashboard Visualization ---" << std::endl;
        std::cout << "Opening Dashboard window. Press ANY KEY to close..." << std::endl;
        
        // Создаём пустой кадр 640x480 (левая половина будущего экрана)
        cv::Mat testFrame(480, 640, CV_8UC3);
        
        // Создаём тестовые данные для демонстрации ВСЕХ функций (включая предупреждения)
        OBDRecord testRec;
        testRec.speed_kmh = 115.0f;       // > 90 (красная зона спидометра)
        testRec.engine_rpm = 5200.0f;     // > 4500 (красная зона тахометра)
        testRec.throttle_pos = 85.0f;     // Высокое положение дросселя
        testRec.coolant_temp = 105.0f;    // > 100 (вызовет WARNING: HOT ENGINE)
        testRec.fuel_level = 12.0f;       // < 15 (вызовет WARNING: LOW FUEL)
        testRec.intake_air_temp = 30.0f;
        testRec.label = 2;                // AGGRESSIVE

        ClassificationResult testCls;
        testCls.label = 2;                // AGGRESSIVE
        testCls.confidence = 0.92f;       // 92% уверенности
        testCls.scores = {0.05f, 0.03f, 0.92f};

        // Инициализируем и рисуем панель
        Dashboard dash(640, 480);
        dash.draw(testFrame, testRec, testCls);
        
        // Показываем графическое окно
        cv::imshow("Dashboard Test (Press any key to exit)", testFrame);
        
        // Ждём нажатия любой клавиши (0 = ждать бесконечно)
        cv::waitKey(0); 
        
        // Закрываем все окна OpenCV
        cv::destroyAllWindows();
        
        std::cout << "Dashboard test finished." << std::endl;
        std::cout << "\nBuild successful!" << std::endl;
        
        return 0;

    } catch (const std::exception& e) {
        // Этот блок перехватит любую ошибку (например, если ONNX модель не найдена)
        std::cerr << "\nFatal Error: " << e.what() << "\n";
        return 1;
    }
}