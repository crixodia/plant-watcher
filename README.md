# Plant Watcher

A monitoring system for plant environmental conditions using ESP8266 sensors and automated data collection.

## Overview

This project consists of multiple components that work together to monitor plant conditions:

- **ESP8266 Sensors**: Arduino-based sensors that collect temperature, humidity, and soil moisture data
- **Data Collection Service**: Python service that fetches sensor data and stores it in a MySQL database
- **Database**: MySQL database for storing historical sensor readings

## Project Structure

```
plant-watcher/
├── db/                     # Database schema and setup files
│   ├── db.sql             # Database structure
│   └── mock.sql           # Sample data for testing
├── sender/                # ESP8266 sensor code
│   └── sender.ino         # Arduino code for data transmission
├── server/                # ESP8266 web server code
│   └── server.ino         # Arduino code for web server
└── service/               # Python data collection service
    ├── config.ini         # Service configuration
    ├── service.py         # Main service script
    └── .env               # Environment variables (not committed)
```

## Service Setup

### Prerequisites

- Python 3.7+
- MySQL database
- ESP8266 devices configured and running

### Environment Variables

⚠️ **Important**: The `.env` file is not committed to the repository for security reasons. You must create your own `.env` file in the `service/` directory with the following variables:

```bash
# Database credentials
DB_USER=your_database_user
DB_PASSWORD=your_database_password

# ESP8266 device credentials
ESP_USER=your_esp_username
ESP_PASSWORD=your_esp_password
```

### Installation

1. **Clone the repository**:
   ```bash
   git clone https://github.com/crixodia/plant-watcher.git
   cd plant-watcher/service
   ```

2. **Install Python dependencies**:
   ```bash
   pip install mysql-connector-python python-dotenv requests
   ```

3. **Set up the database**:
   ```bash
   mysql -u root -p < ../db/db.sql
   ```

4. **Create your environment file**:
   ```bash
   cp .env.example .env  # If you have an example file
   # OR create .env manually with the required variables
   ```

5. **Configure the service**:
   Edit `config.ini` to match your setup:
   ```ini
   [database]
   host = your_database_host
   port = 3306
   name = plant_watcher

   [esp]
   endpoint = http://your_esp_device/read
   timeout = 5

   [logging]
   level = DEBUG
   file = service.log

   [service]
   interval_seconds = 600
   ```

6. **Test the setup**:
   ```bash
   # Test database connection and fetch data once
   python service.py --once
   ```

### Running the Service

**Run once** (for testing):
```bash
python service.py --once
```

**Run continuously** (production mode):
```bash
python service.py
```

The service will:
- Fetch sensor data from your ESP8266 device endpoint
- Store readings (timestamp, temperature, humidity, soil moisture) in the MySQL database
- Log all activities with timestamps to both console and log file
- Run at configured intervals (default: 10 minutes)

### Data Format

The service expects JSON data from the ESP8266 in this format:
```json
{
    "timestamp": "2025-10-05T12:30:00Z",
    "temperature": 23.45,
    "humidity": 67.8,
    "soil": 512
}
```

### Database Schema

The `readings` table stores:
- `id`: Auto-increment primary key
- `timestamp_utc`: UTC timestamp of the sensor reading
- `temperature`: Temperature in Celsius (decimal 5,2)
- `humidity`: Humidity percentage (decimal 5,2)  
- `soil`: Soil moisture level (unsigned integer)
- `created_at`: Record creation timestamp

## Troubleshooting

### Database Connection Issues

If you encounter "Access denied" or connection errors:

1. **Verify credentials**: Check your `.env` file has correct database credentials
   ```bash
   DB_USER=your_actual_username
   DB_PASSWORD=your_actual_password
   ```

2. **Check database server**: Ensure MySQL is running and accessible
   ```bash
   # Test connection manually
   mysql -h your_database_host -u your_username -p
   ```

3. **Verify database exists**: Make sure the `plant_watcher` database was created
   ```sql
   SHOW DATABASES;
   USE plant_watcher;
   SHOW TABLES;
   ```

4. **Check user permissions**: Ensure your database user has proper permissions
   ```sql
   GRANT ALL PRIVILEGES ON plant_watcher.* TO 'your_username'@'%';
   FLUSH PRIVILEGES;
   ```

5. **Network connectivity**: If using a remote database, check firewall and network settings

### ESP8266 Connection Issues

- Verify ESP8266 device is powered on and connected to network
- Check the endpoint URL in `config.ini`
- Ensure ESP8266 credentials in `.env` file match device settings
- Test API endpoint manually: `curl -u username:password http://esp-ip/read`

## Security Notes

- **Environment variables**: Never commit `.env` files containing credentials
- **Network security**: Ensure ESP8266 devices are on a secure network
- **Database security**: Use strong passwords and limit database user permissions

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.