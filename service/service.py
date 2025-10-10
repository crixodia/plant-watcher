import mysql.connector
import dotenv
import os
import requests
import configparser
import logging
import argparse
import sys

dotenv.load_dotenv()

config = configparser.ConfigParser()
config.read('config.ini')

formatter = logging.Formatter(
    '%(asctime)s - %(name)s - %(levelname)s - %(message)s')

logger = logging.getLogger("PlantWatcherService")
logger.setLevel(config.get("logging", "level"))

# Create and configure file handler
file_handler = logging.FileHandler(config.get("logging", "file"))
file_handler.setLevel(config.get("logging", "level"))
file_handler.setFormatter(formatter)

# Create and configure console handler
console_handler = logging.StreamHandler(sys.stdout)
console_handler.setLevel(config.get("logging", "level"))
console_handler.setFormatter(formatter)

# Add handlers to logger
logger.addHandler(file_handler)
logger.addHandler(console_handler)


def get_db_connection():
    """Establish database connection with proper error handling."""
    try:
        connection = mysql.connector.connect(
            host=config.get("database", "host"),
            user=os.getenv("DB_USER"),
            password=os.getenv("DB_PASSWORD"),
            database=config.get("database", "name"),
            port=config.getint("database", "port", fallback=3306)
        )
        return connection
    except mysql.connector.Error as err:
        logger.error(f"Database connection failed: {err}")
        raise


def fetch_data_from_api(api_url):
    try:
        response = requests.get(api_url, auth=(
            os.getenv("ESP_USER"), os.getenv("ESP_PASSWORD")))
        response.raise_for_status()
        return response.json()
    except requests.RequestException as e:
        logger.error(f"Error fetching data from API: {e}")
        return None


def insert_data_to_db(data):
    """Insert sensor data into database."""
    mydb = None
    cursor = None
    try:
        mydb = get_db_connection()
        cursor = mydb.cursor()
        sql = "INSERT INTO readings (timestamp_utc, temperature, humidity, soil) VALUES (%s, %s, %s, %s)"
        val = (
            data['timestamp'],
            data['temperature'],
            data['humidity'],
            data['soil']
        )
        cursor.execute(sql, val)
        mydb.commit()
        logger.info(data)
    except mysql.connector.Error as err:
        logger.error(f"Error inserting data to database: {err}")
    finally:
        if cursor:
            cursor.close()
        if mydb and mydb.is_connected():
            mydb.close()


def main():
    api_url = config.get("esp", "endpoint")
    data = fetch_data_from_api(api_url)
    if data:
        for key, val in data.items():
            if val is None:
                logger.warning(f"Missing value for {key}, skipping insertion.")
                return
        insert_data_to_db(data)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Fetch data from API and store in database")
    parser.add_argument('--once', action='store_true',
                        help="Run the fetch and store process once")
    args = parser.parse_args()

    if args.once:
        main()
    else:
        import time
        interval = config.getint("service", "interval_seconds", fallback=300)
        while True:
            main()
            time.sleep(interval)
