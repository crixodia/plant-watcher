CREATE DATABASE IF NOT EXISTS plant_watcher;
USE plant_watcher;

CREATE TABLE readings (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    timestamp_utc DATETIME NOT NULL,   -- UTC timestamp of the reading
    temperature DECIMAL(5,2),          -- supports -99.99 to 999.99
    humidity DECIMAL(5,2),             -- supports 0.00 to 100.00
    soil INT UNSIGNED,                 -- soil moisture level
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP -- record creation time
);

-- Index for faster queries on timestamp
CREATE INDEX idx_timestamp ON readings (timestamp_utc);
