#pragma once
#include "shared_state.h"
#include "obd_parser.h"
#include "onnx_classifier.h"

/**
 * @brief Функция OBD-потока.
 * 
 * Циклически читает CSV, классифицирует стиль вождения (10 Hz)
 * и обновляет SharedState под мьютексом.
 * 
 * @param state Ссылка на общее состояние
 * @param parser Ссылка на парсер OBD
 * @param classifier Ссылка на ONNX классификатор
 */
void obdThreadFunc(SharedState& state, OBDParser& parser, ONNXClassifier& classifier);