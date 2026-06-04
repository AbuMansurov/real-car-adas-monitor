/**
 * @file main.cpp
 * @brief Главная точка входа системы мониторинга автомобиля.
 * 
 * Запускает два потока:
 * - OBD-поток: читает CSV и классифицирует стиль вождения
 * - Главный поток: обрабатывает камеру, рисует интерфейс, пишет видео
 */

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <opencv2/opencv.hpp>

#include "obd_parser.h"
#include "onnx_classifier.h"
#include "dashboard.h"
#include "dms_monitor.h"
#include "dms_hud.h"
#include "shared_state.h"
#include "obd_thread.h"  // <-- Добавлено объявление obdThreadFunc

/// @brief Пути к файлам
const std::string CSV_PATH = "../data/obd_data.csv";
const std::string MODEL_PATH = "../models/driver_classifier.onnx";
const std::string PARAMS_PATH = "../models/normalization_params.json";
const std::string FACE_PROTO = "../models/deploy.prototxt";
const std::string FACE_MODEL = "../models/res10_300x300_ssd_iter_140000.caffemodel";
const std::string EYE_CASCADE = "../models/haarcascade_eye.xml";
const std::string OUTPUT_VIDEO = "../output/result_situation2.mp4";
const std::string OUTPUT_LOG = "../output/dms_alerts.log";

int main() {
    try {
        std::cout << "=== REAL CAR ADAS MONITOR v1.0 ===" << std::endl;
        std::cout << "Final Integration Test" << std::endl << std::endl;
        
        // ==========================================
        // ИНИЦИАЛИЗАЦИЯ МОДУЛЕЙ
        // ==========================================
        
        std::cout << "[INIT] Loading OBD data..." << std::endl;
        OBDParser parser;
        int recordCount = parser.load(CSV_PATH);
        if (recordCount < 0) {
            std::cerr << "FATAL: Cannot load CSV file: " << CSV_PATH << "\n";
            return 1;
        }
        std::cout << "[INIT] Loaded " << recordCount << " OBD records.\n" << std::endl;
        
        std::cout << "[INIT] Loading ONNX classifier..." << std::endl;
        // ИСПРАВЛЕНИЕ: создаём сразу с аргументами, не через пустой конструктор
        ONNXClassifier classifier(MODEL_PATH, PARAMS_PATH);
        std::cout << "[INIT] ONNX model loaded successfully.\n" << std::endl;
        
        std::cout << "[INIT] Loading DMS models..." << std::endl;
        DMSMonitor dms(FACE_PROTO, FACE_MODEL, EYE_CASCADE);
        if (!dms.isLoaded()) {
            std::cerr << "FATAL: DMS models failed to load!\n";
            return 1;
        }
        std::cout << "[INIT] DMS models loaded successfully.\n" << std::endl;
        
        DMSHUD hud;
        Dashboard dashboard(640, 480);
        
        // Лог-файл
        std::ofstream logFile(OUTPUT_LOG, std::ios::app);
        if (logFile.is_open()) {
            auto now = std::chrono::system_clock::now();
            std::time_t time = std::chrono::system_clock::to_time_t(now);
            logFile << "\n=== Session started at " << std::ctime(&time);
        }
        
        // ==========================================
        // ЗАПУСК OBD-ПОТОКА
        // ==========================================
        SharedState state;
        
        std::cout << "[MAIN] Starting OBD thread..." << std::endl;
        std::thread obdThread(obdThreadFunc, 
                              std::ref(state), 
                              std::ref(parser), 
                              std::ref(classifier));
        
        // ==========================================
        // ВЕБ-КАМЕРА
        // ==========================================
        std::cout << "[MAIN] Opening webcam..." << std::endl;
        cv::VideoCapture cap(0);
        if (!cap.isOpened()) {
            std::cerr << "FATAL: Cannot open webcam!\n";
            state.running = false;
            if (obdThread.joinable()) obdThread.join();
            return 1;
        }
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
        std::cout << "[MAIN] Webcam opened.\n\n";
        
        // ==========================================
        // ВИДЕОЗАПИСЬ
        // ==========================================
        cv::VideoWriter videoWriter;
        cv::Size frameSize(1280, 480);
        int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
        
        if (videoWriter.open(OUTPUT_VIDEO, fourcc, 15.0, frameSize)) {
            std::cout << "[MAIN] Recording video to: " << OUTPUT_VIDEO << "\n";
        }
        
        // ==========================================
        // ГЛАВНЫЙ ЦИКЛ
        // ==========================================
        std::cout << "\nControls: Q/ESC=Exit, SPACE=Pause, S=Screenshot\n\n";
        
        auto startTime = std::chrono::steady_clock::now();
        bool paused = false;
        cv::Mat frame, leftHalf, rightHalf, combinedFrame;
        
        while (state.running) {
            if (!paused) {
                cap >> frame;
                if (frame.empty()) continue;
                if (frame.cols != 640 || frame.rows != 480) {
                    cv::resize(frame, frame, cv::Size(640, 480));
                }
            }
            
            // Анализ водителя
            DriverState driverState = dms.analyze(frame);
            
            // Логирование алертов
            {
                std::lock_guard<std::mutex> lock(state.mtx);
                if (driverState.alert_drowsy) {
                    state.alertCount++;
                    state.drowsyAlerts++;
                    if (logFile.is_open()) {
                        logFile << "[DROWSY] " << state.drowsyAlerts << "\n";
                    }
                    std::cout << "ALERT: DROWSY\n";
                }
                if (driverState.alert_distracted) {
                    state.alertCount++;
                    state.distractedAlerts++;
                    if (logFile.is_open()) {
                        logFile << "[DISTRACTED] " << state.distractedAlerts << "\n";
                    }
                    std::cout << "ALERT: DISTRACTED\n";
                }
            }
            
            // Рисование кадра 1280x480
            leftHalf = cv::Mat(480, 640, CV_8UC3, cv::Scalar(30, 30, 30));
            rightHalf = frame.clone();
            
            {
                std::lock_guard<std::mutex> lock(state.mtx);
                dashboard.draw(leftHalf, state.currentRecord, state.currentClass);
            }
            
            hud.draw(rightHalf, driverState);
            
            // Объединение
            combinedFrame = cv::Mat(480, 1280, CV_8UC3);
            leftHalf.copyTo(combinedFrame(cv::Rect(0, 0, 640, 480)));
            rightHalf.copyTo(combinedFrame(cv::Rect(640, 0, 640, 480)));
            
            cv::putText(combinedFrame, "REAL CAR ADAS MONITOR", 
                        cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, 
                        cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
            
            cv::imshow("Real Car ADAS Monitor", combinedFrame);
            
            if (videoWriter.isOpened() && !paused) {
                videoWriter.write(combinedFrame);
            }
            
            // Клавиши
            int key = cv::waitKey(30) & 0xFF;
            if (key == 'q' || key == 'Q' || key == 27) {
                std::cout << "\n[MAIN] Exit requested.\n";
                state.running = false;
                break;
            } else if (key == ' ') {
                paused = !paused;
                std::cout << "[MAIN] " << (paused ? "PAUSED" : "RESUMED") << "\n";
            } else if (key == 's' || key == 'S') {
                std::string name = "../output/screenshot_" + 
                    std::to_string(std::time(nullptr)) + ".png";
                cv::imwrite(name, combinedFrame);
                std::cout << "[MAIN] Screenshot saved: " << name << "\n";
            }
        }
        
        // ==========================================
        // ЗАВЕРШЕНИЕ
        // ==========================================
        std::cout << "\n[MAIN] Shutting down...\n";
        
        if (obdThread.joinable()) {
            obdThread.join();
        }
        
        cap.release();
        videoWriter.release();
        cv::destroyAllWindows();
        if (logFile.is_open()) logFile.close();
        
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            endTime - startTime).count();
        
        std::cout << "\n=== SESSION STATISTICS ===" << std::endl;
        std::cout << "Duration:        " << duration << " seconds" << std::endl;
        std::cout << "OBD processed:   " << state.obdProcessed << " records" << std::endl;
        std::cout << "Total alerts:    " << state.alertCount << std::endl;
        std::cout << "  Drowsy:        " << state.drowsyAlerts << std::endl;
        std::cout << "  Distracted:    " << state.distractedAlerts << std::endl;
        std::cout << "  Aggressive:    " << state.aggressiveAlerts << std::endl;
        std::cout << "\nOutput files:" << std::endl;
        std::cout << "  Video: " << OUTPUT_VIDEO << std::endl;
        std::cout << "  Log:   " << OUTPUT_LOG << std::endl;
        std::cout << "\nBuild successful!" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\nFatal Error: " << e.what() << "\n";
        return 1;
    }
}