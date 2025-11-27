CREATE DATABASE IF NOT EXISTS UBICOMP;
USE UBICOMP;

CREATE TABLE TRAFFIC_LIGHT_DATA (
    id INT AUTO_INCREMENT PRIMARY KEY,
    sensor_id VARCHAR(255) NOT NULL,
    street_id VARCHAR(255),
    received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    latitude DECIMAL(10, 8),
    longitude DECIMAL(11, 8),
    street_name VARCHAR(255),
    district VARCHAR(255),
    current_state VARCHAR(50),
    pedestrian_request BOOLEAN,
    light_level VARCHAR(50),
    current_state_seconds INT,
    current_state_seconds_left INT,
    uptime_seconds BIGINT
);