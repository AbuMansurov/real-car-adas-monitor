#include <gtest/gtest.h>
#include "obd_parser.h"
#include "dms_monitor.h" 
#include <fstream>
#include <iostream>

// ==========================================
// ТЕСТЫ OBDParser (Задание 2)
// ==========================================

TEST(OBDParserTest, LabelConversion) {
    OBDParser parser;
    {
        std::ofstream f("temp_test_labels.csv");
        f << "speed_kmh,engine_rpm,throttle_pos,coolant_temp,fuel_level,intake_air_temp,label\n";
        f << "30,1500,10,85,50,20,SLOW\n";
        f << "70,2800,40,90,40,25,NORMAL\n";
        f << "120,5000,90,95,30,30,AGGRESSIVE\n";
    }
    
    ASSERT_EQ(parser.load("temp_test_labels.csv"), 3);
    EXPECT_EQ(parser.getRecord(0).label, 0);
    EXPECT_EQ(parser.getRecord(1).label, 1);
    EXPECT_EQ(parser.getRecord(2).label, 2);
    
    std::remove("temp_test_labels.csv");
}

TEST(OBDParserTest, FileNotFound) {
    OBDParser parser;
    EXPECT_EQ(parser.load("nonexistent_file_12345.csv"), -1);
}

TEST(OBDParserTest, OutOfRangeException) {
    OBDParser parser;
    EXPECT_THROW(parser.getRecord(0), std::out_of_range);
    
    {
        std::ofstream f("temp_single.csv");
        f << "speed_kmh,engine_rpm,throttle_pos,coolant_temp,fuel_level,intake_air_temp,label\n";
        f << "50,2000,20,90,50,20,NORMAL\n";
    }
    parser.load("temp_single.csv");
    EXPECT_THROW(parser.getRecord(1), std::out_of_range);
    EXPECT_THROW(parser.getRecord(-1), std::out_of_range);
    
    std::remove("temp_single.csv");
}

TEST(OBDParserTest, ParseValidCSV) {
    {
        std::ofstream f("temp_valid.csv");
        f << "speed_kmh,engine_rpm,throttle_pos,coolant_temp,fuel_level,intake_air_temp,label\n";
        f << "87.5,3200.0,50.0,92.0,68.0,25.0,NORMAL\n";
    }
    OBDParser parser;
    ASSERT_EQ(parser.load("temp_valid.csv"), 1);
    
    auto rec = parser.getRecord(0);
    EXPECT_FLOAT_EQ(rec.speed_kmh, 87.5f);
    EXPECT_FLOAT_EQ(rec.engine_rpm, 3200.0f);
    EXPECT_EQ(rec.label, 1);
    
    std::remove("temp_valid.csv");
}

TEST(OBDParserTest, SkipBadLine) {
    {
        std::ofstream f("temp_bad_line.csv");
        f << "speed_kmh,engine_rpm,throttle_pos,coolant_temp,fuel_level,intake_air_temp,label\n";
        f << "87,3200,50,92,68,25,NORMAL\n";
        f << "bad,data,here\n";
        f << "90,3500,55,93,60,22,UNKNOWN_LABEL\n";
        f << "95,3600,60,94,65,23,SLOW\n";
    }
    OBDParser parser;
    ASSERT_EQ(parser.load("temp_bad_line.csv"), 2);
    EXPECT_EQ(parser.getRecord(0).speed_kmh, 87.0f);
    EXPECT_EQ(parser.getRecord(1).speed_kmh, 95.0f);
    
    std::remove("temp_bad_line.csv");
}

// ==========================================
// ТЕСТЫ DMSMonitor (Задание 7.6)
// ==========================================

TEST(DMSMonitorTest, NotLoadedByDefault) {
    DMSMonitor dms("nonexistent.prototxt", "nonexistent.caffemodel", "nonexistent.xml");
    EXPECT_FALSE(dms.isLoaded());
}

TEST(DMSMonitorTest, AnalyzeEmptyFrame) {
    DMSMonitor dms("nonexistent.prototxt", "nonexistent.caffemodel", "nonexistent.xml");
    cv::Mat emptyFrame;
    DriverState state = dms.analyze(emptyFrame);
    EXPECT_FALSE(state.face_detected);
    EXPECT_TRUE(state.eyes_open);
    EXPECT_TRUE(state.looking_forward);
    EXPECT_FALSE(state.alert_drowsy);
    EXPECT_FALSE(state.alert_distracted);
}

TEST(DMSMonitorTest, AnalyzeBlackFrame) {
    DMSMonitor dms("nonexistent.prototxt", "nonexistent.caffemodel", "nonexistent.xml");
    cv::Mat blackFrame(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));
    DriverState state = dms.analyze(blackFrame);
    EXPECT_FALSE(state.face_detected);
    EXPECT_FLOAT_EQ(state.eye_openness, 1.0f);
    EXPECT_FLOAT_EQ(state.head_turn_deg, 0.0f);
}

TEST(DMSMonitorTest, DriverStateDefaults) {
    DriverState state;
    EXPECT_FALSE(state.face_detected);
    EXPECT_TRUE(state.eyes_open);
    EXPECT_TRUE(state.looking_forward);
    EXPECT_FLOAT_EQ(state.eye_openness, 1.0f);
    EXPECT_FLOAT_EQ(state.head_turn_deg, 0.0f);
    EXPECT_FALSE(state.alert_drowsy);
    EXPECT_FALSE(state.alert_distracted);
    EXPECT_EQ(state.face_rect.area(), 0);
}