#pragma once
#include <opencv2/opencv.hpp>
#include "dms_monitor.h"

/**
 * @brief HUD (Head-Up Display) для отображения состояния водителя.
 * 
 * Рисует поверх кадра с камеры:
 * - Угловые скобки вокруг лица
 * - Текстовые индикаторы состояния
 * - Плашки алертов (DROWSINESS, DISTRACTION)
 */
class DMSHUD {
public:
    /**
     * @brief Рисует HUD поверх кадра.
     * @param frame[out] Кадр для отрисовки
     * @param state Текущее состояние водителя
     */
    void draw(cv::Mat& frame, const DriverState& state);
};