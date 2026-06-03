#include "dashboard.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

Dashboard::Dashboard(int width, int height) : width_(width), height_(height) {}

// Рисуем круговой прибор (спидометр или тахометр)
void Dashboard::drawGauge(cv::Mat& frame, cv::Point center, int radius,
                          float value, float minV, float maxV,
                          const std::string& label,
                          float greenZoneEnd, float redZoneStart,
                          int num_ticks) {
    // Цвета зон (как на настоящей приборной панели)
    const cv::Scalar GREEN_ZONE_COLOR(0, 200, 0);   // Ярко-зелёный
    const cv::Scalar RED_ZONE_COLOR(0, 0, 220);     // Ярко-красный (BGR)
    const cv::Scalar GRAY_ZONE_COLOR(80, 80, 80);   // Серый для среднего диапазона
    const cv::Scalar WHITE_NEEDLE(255, 255, 255);   // Белая стрелка

    // 1. Фон прибора (тёмный круг)
    cv::circle(frame, center, radius, cv::Scalar(30, 30, 30), -1, cv::LINE_AA);
    // Внешнее кольцо
    cv::circle(frame, center, radius, cv::Scalar(100, 100, 100), 2, cv::LINE_AA);
    // Внутреннее декоративное кольцо
    cv::circle(frame, center, radius - 30, cv::Scalar(50, 50, 50), 1, cv::LINE_AA);

    // 2. СТАТИЧНАЯ ЗЕЛЁНАЯ ЗОНА (от начала шкалы до greenZoneEnd)
    double greenEndAngle = -135.0 + ((greenZoneEnd - minV) / (maxV - minV)) * 270.0;
    cv::ellipse(frame, center, cv::Size(radius - 15, radius - 15),
                0, -135.0, greenEndAngle, GREEN_ZONE_COLOR, 10, cv::LINE_AA);

    // 3. СТАТИЧНАЯ СЕРАЯ ЗОНА (между зелёной и красной, если есть промежуток)
    if (redZoneStart > greenZoneEnd) {
        double grayStartAngle = greenEndAngle;
        double grayEndAngle = -135.0 + ((redZoneStart - minV) / (maxV - minV)) * 270.0;
        cv::ellipse(frame, center, cv::Size(radius - 15, radius - 15),
                    0, grayStartAngle, grayEndAngle, GRAY_ZONE_COLOR, 10, cv::LINE_AA);
    }

    // 4. СТАТИЧНАЯ КРАСНАЯ ЗОНА (от redZoneStart до конца шкалы)
    double redStartAngle = -135.0 + ((redZoneStart - minV) / (maxV - minV)) * 270.0;
    cv::ellipse(frame, center, cv::Size(radius - 15, radius - 15),
                0, redStartAngle, 135.0, RED_ZONE_COLOR, 10, cv::LINE_AA);

    // 5. Деления (штрихи) на внешнем круге
    for (int i = 0; i <= num_ticks; ++i) {
        double angle_deg = -135.0 + (i * 270.0 / num_ticks);
        double angle_rad = angle_deg * CV_PI / 180.0;
        
        // Длинный штрих для основных делений
        cv::Point p1(center.x + (radius - 8) * std::cos(angle_rad),
                     center.y + (radius - 8) * std::sin(angle_rad));
        cv::Point p2(center.x + (radius - 22) * std::cos(angle_rad),
                     center.y + (radius - 22) * std::sin(angle_rad));
        cv::line(frame, p1, p2, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
        
        // Цифра значения (на внешнем круге, за штрихами)
        float tick_val = minV + i * (maxV - minV) / num_ticks;
        std::string val_str = std::to_string(static_cast<int>(tick_val));
        cv::Point text_pos(center.x + (radius - 42) * std::cos(angle_rad) - 12,
                           center.y + (radius - 42) * std::sin(angle_rad) + 5);
        cv::putText(frame, val_str, text_pos, cv::FONT_HERSHEY_SIMPLEX, 0.5, 
                    cv::Scalar(200, 200, 200), 1, cv::LINE_AA);
    }

    // 6. СТРЕЛКА (белая, как на настоящей приборной панели)
    float ratio = (value - minV) / (maxV - minV);
    ratio = std::max(0.0f, std::min(1.0f, ratio));
    double needleAngleDeg = -135.0 + ratio * 270.0;
    double needleAngleRad = needleAngleDeg * CV_PI / 180.0;
    
    int needleLen = radius - 30;
    cv::Point needleTip(center.x + needleLen * std::cos(needleAngleRad),
                        center.y + needleLen * std::sin(needleAngleRad));
    
    // Тень стрелки (для объёма)
    cv::line(frame, center, needleTip, cv::Scalar(0, 0, 0), 5, cv::LINE_AA);
    // Сама стрелка (белая)
    cv::line(frame, center, needleTip, WHITE_NEEDLE, 3, cv::LINE_AA);
    
    // Центральный круг (ось стрелки)
    cv::circle(frame, center, 8, cv::Scalar(200, 200, 200), -1, cv::LINE_AA);
    cv::circle(frame, center, 4, cv::Scalar(50, 50, 50), -1, cv::LINE_AA);

    // 7. Крупное числовое значение по центру (внизу прибора)
    std::string valText = std::to_string(static_cast<int>(value));
    cv::Point textCenter(center.x - 20, center.y + radius - 30);
    cv::putText(frame, valText, textCenter,
                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    
    // Подпись прибора (km/h или RPM)
    cv::putText(frame, label, cv::Point(center.x - 20, center.y + radius - 10),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(180, 180, 180), 1, cv::LINE_AA);
}

// Горизонтальная полоса (температура / топливо / дроссель)
void Dashboard::drawLinearGauge(cv::Mat& frame, cv::Rect rect,
                                float value, float minV, float maxV,
                                const std::string& label, const std::string& unit,
                                const cv::Scalar& color, float warningThreshold, bool warnLow) {
    // Фон полосы
    cv::rectangle(frame, rect, cv::Scalar(50, 50, 50), -1, cv::LINE_AA);
    cv::rectangle(frame, rect, cv::Scalar(120, 120, 120), 1, cv::LINE_AA);
    
    // Заполненная часть
    float ratio = (value - minV) / (maxV - minV);
    ratio = std::max(0.0f, std::min(1.0f, ratio));
    
    // Если значение в опасной зоне — меняем цвет на красный
    cv::Scalar fillColor = color;
    bool inWarning = warnLow ? (value < warningThreshold) : (value > warningThreshold);
    if (inWarning) {
        fillColor = cv::Scalar(0, 0, 220); // Красный
    }
    
    cv::Rect fillRect(rect.x, rect.y, static_cast<int>(rect.width * ratio), rect.height);
    cv::rectangle(frame, fillRect, fillColor, -1, cv::LINE_AA);
    
    // Текст с единицей измерения (например, "TEMP: 92 C")
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%s: %.0f %s", label.c_str(), value, unit.c_str());
    cv::putText(frame, buffer, cv::Point(rect.x, rect.y - 5),
                cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
}

// Предупреждение с рамкой и отступами
void Dashboard::drawWarning(cv::Mat& frame, const std::string& text, cv::Point topLeft) {
    int baseline = 0;
    cv::Size textSize = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.7, 2, &baseline);
    
    // Увеличенные отступы (padding) чтобы текст не наезжал на рамку
    int padding = 12;
    cv::Rect textRect(topLeft.x - padding, topLeft.y - textSize.height - padding, 
                      textSize.width + 2 * padding, textSize.height + baseline + 2 * padding);
    
    // Полупрозрачный красный фон
    cv::rectangle(frame, textRect, cv::Scalar(20, 0, 0), -1, cv::LINE_AA);
    // Яркая красная рамка
    cv::rectangle(frame, textRect, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
    // Текст
    cv::putText(frame, text, cv::Point(topLeft.x, topLeft.y),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
}

// Главный метод сборки всей панели
void Dashboard::draw(cv::Mat& frame, const OBDRecord& rec, const ClassificationResult& cls) {
    // 1. Очистка фона (тёмный)
    frame = cv::Scalar(20, 20, 20);
    
    // 2. СПИДОМЕТР (0-140 км/ч): зелёная до 90, красная после 90, 7 интервалов (шаг 20)
    drawGauge(frame, cv::Point(160, 170), 110, rec.speed_kmh, 0, 140, "km/h", 
              90.0f, 90.0f, 7);
    
    // 3. ТАХОМЕТР (0-6000 RPM): зелёная до 4500, красная после 4500, 6 интервалов (шаг 1000)
    drawGauge(frame, cv::Point(480, 170), 110, rec.engine_rpm, 0, 6000, "RPM", 
              4500.0f, 4500.0f, 6);
    
    // 4. Линейные индикаторы (сдвинуты ниже, под приборами)
    // TEMP: предупреждение если > 100°C
    drawLinearGauge(frame, cv::Rect(40, 320, 240, 22), rec.coolant_temp, 0, 120, 
                    "TEMP", "C", cv::Scalar(0, 180, 220), 100.0f, false);
    // FUEL: предупреждение если < 15%
    drawLinearGauge(frame, cv::Rect(40, 380, 240, 22), rec.fuel_level, 0, 100, 
                    "FUEL", "%", cv::Scalar(0, 220, 0), 15.0f, true);
    // THR: без предупреждения
    drawLinearGauge(frame, cv::Rect(40, 440, 240, 22), rec.throttle_pos, 0, 100, 
                    "THR", "%", cv::Scalar(220, 180, 0), -1.0f, false);
    
    // 5. Стиль вождения (справа, сверху)
    cv::Scalar styleColor;
    std::string styleText;
    if (cls.label == 0) { styleColor = cv::Scalar(220, 180, 0); styleText = "SLOW"; }
    else if (cls.label == 1) { styleColor = cv::Scalar(0, 220, 0); styleText = "NORMAL"; }
    else { styleColor = cv::Scalar(0, 0, 220); styleText = "AGGRESSIVE"; }
    
    cv::putText(frame, "STYLE: " + styleText, cv::Point(300, 330),
                cv::FONT_HERSHEY_SIMPLEX, 1.0, styleColor, 2, cv::LINE_AA);
    
    // 6. Предупреждения (разнесены по вертикали, чтобы не наезжали друг на друга)
    if (rec.coolant_temp > 100.0f) {
        drawWarning(frame, "WARNING: HOT ENGINE!", cv::Point(310, 390));
    }
    if (rec.fuel_level < 15.0f) {
        drawWarning(frame, "WARNING: LOW FUEL!", cv::Point(310, 450));
    }
}