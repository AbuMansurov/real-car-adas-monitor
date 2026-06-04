/**
 * @file obd_thread.cpp
 * @brief Реализация функции OBD-потока.
 */

#include "obd_thread.h"
#include <thread>
#include <chrono>
#include <iostream>

void obdThreadFunc(SharedState& state, OBDParser& parser, ONNXClassifier& classifier) {
    size_t index = 0;
    const size_t totalRecords = parser.size();
    
    std::cout << "[OBD Thread] Started. Processing " << totalRecords << " records at 10 Hz.\n";
    
    while (state.running) {
        // Берём следующую запись (циклически)
        OBDRecord rec = parser.getRecord(static_cast<int>(index));
        
        // Формируем вектор признаков
        std::vector<float> features = {
            rec.speed_kmh,
            rec.engine_rpm,
            rec.throttle_pos,
            rec.coolant_temp,
            rec.fuel_level,
            rec.intake_air_temp
        };
        
        // Классифицируем стиль вождения
        ClassificationResult cls = classifier.predict(features);
        
        // Захватываем мьютекс и обновляем общие данные
        {
            std::lock_guard<std::mutex> lock(state.mtx);
            state.currentRecord = rec;
            state.currentClass = cls;
            state.obdProcessed++;
            
            if (cls.label == 2) {  // AGGRESSIVE
                state.aggressiveAlerts++;
            }
        }
        
        // Переход к следующей записи
        index++;
        if (index >= totalRecords) {
            index = 0;
        }
        
        // Ждём 100 мс (10 Hz)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "[OBD Thread] Stopped. Processed " << state.obdProcessed << " records.\n";
}