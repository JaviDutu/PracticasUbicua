CREATE DATABASE IF NOT EXISTS UBICOMP;
USE UBICOMP;

CREATE TABLE TRAFFIC_LIGHT_DATA (
    id INT AUTO_INCREMENT PRIMARY KEY,
    sensor_id VARCHAR(255) NOT NULL,
    street_id VARCHAR(255),
    sensor_timestamp VARCHAR(255), -- El timestamp original del sensor
    received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, -- El timestamp del servidor
    
    -- Location
    latitude DECIMAL(10, 8),
    longitude DECIMAL(11, 8),
    street_name VARCHAR(255),
    district VARCHAR(255),
    neighborhood VARCHAR(255),
    postal_code VARCHAR(10),

    -- Data
    traffic_light_type VARCHAR(100),
    circulation_direction VARCHAR(100),
    current_state VARCHAR(50),
    pedestrian_request BOOLEAN,
    light_level VARCHAR(50),
    current_state_seconds INT,
    current_state_seconds_left INT,
    uptime_seconds BIGINT
);