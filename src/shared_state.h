#pragma once
#include <mutex>
#include <atomic>
#include "obd_parser.h"
#include "onnx_classifier.h"
#include "dms_monitor.h"

/**
 * @brief Общая структура данных, разделяемая между потоками.
 * 
 * Хранит текущее состояние системы мониторинга.
 * Все поля (кроме running) защищены мьютексом mtx.
 */
struct SharedState {
    OBDRecord currentRecord{};           ///< Текущая запись телеметрии
    ClassificationResult currentClass{}; ///< Текущий результат классификации
    std::mutex mtx;                      ///< Мьютекс для защиты данных
    std::atomic<bool> running{true};     ///< Флаг продолжения работы
    
    int obdProcessed = 0;                ///< Счётчик обработанных OBD записей
    int alertCount = 0;                  ///< Общее количество алертов
    int drowsyAlerts = 0;                ///< Алерты усталости
    int distractedAlerts = 0;            ///< Алерты отвлечения
    int aggressiveAlerts = 0;            ///< Алерты агрессивного вождения
};