#pragma once
#include <string>
#include <vector>
#include <stdexcept>

/**
 * @brief Структура для хранения одной записи телеметрии из OBD-II.
 * 
 * Содержит 6 числовых признаков и метку стиля вождения.
 * Используется для передачи данных между парсером и классификатором.
 */
struct OBDRecord {
    float speed_kmh;        ///< Скорость в км/ч
    float engine_rpm;       ///< Обороты двигателя
    float throttle_pos;     ///< Положение дроссельной заслонки (0-100%)
    float coolant_temp;     ///< Температура охлаждающей жидкости (°C)
    float fuel_level;       ///< Уровень топлива (0-100%)
    float intake_air_temp;  ///< Температура всасываемого воздуха (°C)
    int label;              ///< Метка стиля: 0=SLOW, 1=NORMAL, 2=AGGRESSIVE
};

/**
 * @brief Парсер CSV файлов с телеметрией автомобиля OBD-II.
 * 
 * Читает CSV файл, пропускает заголовок, парсит каждую строку в OBDRecord.
 * Некорректные строки пропускаются с предупреждением в std::cerr.
 * 
 * @example
 * @code
 * OBDParser parser;
 * int count = parser.load("data.csv");
 * OBDRecord rec = parser.getRecord(0);
 * @endcode
 */
class OBDParser {
public:
    /**
     * @brief Загружает CSV файл с телеметрией.
     * @param filename Путь к CSV файлу
     * @return Количество успешно загруженных записей, или -1 при ошибке открытия файла
     */
    int load(const std::string& filename);
    
    /**
     * @brief Возвращает запись по индексу.
     * @param index Индекс записи (0-based)
     * @return Копия записи OBDRecord
     * @throws std::out_of_range Если индекс вне диапазона [0, size())
     */
    OBDRecord getRecord(int index) const;
    
    /**
     * @brief Возвращает общее количество загруженных записей.
     * @return Размер вектора records_
     */
    size_t size() const { return records_.size(); }

private:
    std::vector<OBDRecord> records_;  ///< Вектор всех загруженных записей
    
    /**
     * @brief Конвертирует строковую метку в числовой код.
     * @param label Строка: "SLOW", "NORMAL" или "AGGRESSIVE"
     * @return 0, 1 или 2; -1 если метка неизвестна
     */
    int labelToInt(const std::string& label) const;
};