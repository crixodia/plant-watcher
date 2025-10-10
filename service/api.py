from flask import Flask, request, jsonify
from flask_cors import CORS
import secrets
import os
import requests
from datetime import datetime
import base64

# Import the main function from service.py
from service import main as service_main, config, logger
import dotenv

# Load environment variables
dotenv.load_dotenv()

# Use the same logger from service.py for consistent logging
api_logger = logger


app = Flask(__name__)

# Enable CORS for all routes with permissive headers for Grafana compatibility
CORS(app, origins="*", methods=["GET", "POST", "OPTIONS"], 
     allow_headers=["*"])

# Load API key from environment
API_KEY = os.getenv('API_KEY')
if not API_KEY:
    # Generate a new API key and suggest adding it to .env
    API_KEY = secrets.token_urlsafe(32)
    api_logger.warning("No API_KEY found in .env file, generated a temporary key.")
    api_logger.warning(f"Temporary API Key: {API_KEY}")
else:
    api_logger.info("API key loaded from .env file")

def verify_api_key():
    """Verify API key from query parameters."""
    api_key = request.args.get('api_key')
    if not api_key or api_key != API_KEY:
        return False
    return True


def make_esp_request(endpoint, method='GET'):
    """Make authenticated request to ESP8266 device."""
    try:
        base_url = config.get("esp", "endpoint").replace('/read', '')
        url = f"{base_url}/{endpoint}"
        
        auth = (os.getenv("ESP_USER"), os.getenv("ESP_PASSWORD"))
        timeout = config.getint("esp", "timeout", fallback=5)
        
        if method == 'GET':
            response = requests.get(url, auth=auth, timeout=timeout)
        elif method == 'POST':
            response = requests.post(url, auth=auth, timeout=timeout)
        else:
            raise ValueError(f"Unsupported HTTP method: {method}")
            
        response.raise_for_status()
        return response
        
    except requests.RequestException as e:
        api_logger.error(f"ESP request failed for {endpoint}: {e}")
        raise


@app.before_request
def authenticate():
    """Authenticate all requests except root and OPTIONS (CORS preflight)."""
    # Skip authentication for root endpoint and OPTIONS requests
    if request.endpoint == 'index' or request.method == 'OPTIONS':
        return
        
    if not verify_api_key():
        api_logger.warning(f"Unauthorized access attempt from {request.remote_addr}")
        return jsonify({
            'error': 'Unauthorized',
            'message': 'Valid API key required as query parameter: ?api_key=YOUR_KEY'
        }), 401


@app.route('/')
def index():
    """API information endpoint."""
    return jsonify({
        'name': 'Plant Watcher API',
        'version': '1.0.0',
        'endpoints': {
            '/read': 'Fetch sensor data and store in database',
            '/water': 'Trigger plant watering system',
            '/photo': 'Take a photo of the plant',
            '/restart': 'Restart ESP8266 hardware'
        },
        'authentication': 'Add ?api_key=YOUR_KEY to all requests',
        'cors': 'CORS enabled for cross-origin requests',
        'note': 'API key is loaded from .env file (API_KEY variable)'
    })


@app.route('/read', methods=['GET', 'OPTIONS'])
def read_sensor_data():
    """Run main() from service.py to read and store sensor data."""
    # Handle CORS preflight request
    if request.method == 'OPTIONS':
        return '', 200
        
    try:
        api_logger.info("Reading sensor data via API")
        service_main()
        
        return jsonify({
            'success': True,
            'message': 'Sensor data read and stored successfully',
            'timestamp': datetime.now().isoformat()
        })
        
    except Exception as e:
        api_logger.error(f"Error reading sensor data: {e}")
        return jsonify({
            'success': False,
            'error': str(e),
            'timestamp': datetime.now().isoformat()
        }), 500


@app.route('/water', methods=['POST', 'OPTIONS'])
def water_plant():
    """Trigger plant watering system via ESP8266."""
    # Handle CORS preflight request
    if request.method == 'OPTIONS':
        return '', 200
        
    try:
        api_logger.info("Triggering plant watering system")
        
        # Make request to ESP8266 water endpoint
        response = make_esp_request('water', method='POST')
        
        # Try to parse JSON response, fallback to text
        try:
            result = response.json()
        except:
            result = {'message': response.text}
        
        api_logger.info("Plant watering triggered successfully")
        return jsonify({
            'success': True,
            'message': 'Plant watering triggered successfully',
            'esp_response': result,
            'timestamp': datetime.now().isoformat()
        })
        
    except Exception as e:
        api_logger.error(f"Error triggering watering system: {e}")
        return jsonify({
            'success': False,
            'error': str(e),
            'timestamp': datetime.now().isoformat()
        }), 500


@app.route('/photo', methods=['GET', 'OPTIONS'])
def take_photo():
    """Take a photo of the plant via ESP8266 camera."""
    # Handle CORS preflight request
    if request.method == 'OPTIONS':
        return '', 200
        
    try:
        api_logger.info("Taking plant photo")
        
        # Make request to ESP8266 photo endpoint
        response = make_esp_request('photo', method='GET')
        
        # Check if response is binary (image) or JSON
        content_type = response.headers.get('content-type', '')
        
        if 'image' in content_type:
            # Return image data as base64
            image_data = base64.b64encode(response.content).decode('utf-8')
            api_logger.info("Photo captured successfully")
            return jsonify({
                'success': True,
                'message': 'Photo captured successfully',
                'image_data': image_data,
                'content_type': content_type,
                'timestamp': datetime.now().isoformat()
            })
        else:
            # Return JSON response from ESP
            try:
                result = response.json()
            except:
                result = {'message': response.text}
                
            api_logger.info("Photo request completed")
            return jsonify({
                'success': True,
                'message': 'Photo request completed',
                'esp_response': result,
                'timestamp': datetime.now().isoformat()
            })
        
    except Exception as e:
        api_logger.error(f"Error taking photo: {e}")
        return jsonify({
            'success': False,
            'error': str(e),
            'timestamp': datetime.now().isoformat()
        }), 500


@app.route('/restart', methods=['POST', 'OPTIONS'])
def restart_hardware():
    """Restart ESP8266 hardware."""
    # Handle CORS preflight request
    if request.method == 'OPTIONS':
        return '', 200
        
    try:
        api_logger.info("Restarting ESP8266 hardware")
        
        # Make request to ESP8266 restart endpoint
        response = make_esp_request('restart', method='POST')
        
        # Try to parse JSON response, fallback to text
        try:
            result = response.json()
        except:
            result = {'message': response.text}
        
        api_logger.info("ESP8266 restart command sent successfully")
        return jsonify({
            'success': True,
            'message': 'ESP8266 restart command sent successfully',
            'esp_response': result,
            'timestamp': datetime.now().isoformat()
        })
        
    except Exception as e:
        api_logger.error(f"Error restarting hardware: {e}")
        return jsonify({
            'success': False,
            'error': str(e),
            'timestamp': datetime.now().isoformat()
        }), 500


@app.errorhandler(404)
def not_found(error):
    """Handle 404 errors."""
    return jsonify({
        'error': 'Not Found',
        'message': 'Endpoint not found. Check / for available endpoints.'
    }), 404


@app.errorhandler(500)
def internal_error(error):
    """Handle 500 errors."""
    return jsonify({
        'error': 'Internal Server Error',
        'message': 'An unexpected error occurred.'
    }), 500


if __name__ == '__main__':
    # Get host and port from config
    api_host = config.get("api", "host", fallback="0.0.0.0")
    api_port = config.getint("api", "port", fallback=5000)
    
    print(f"Plant Watcher API Server")
    print(f"API Key: {API_KEY}")
    print(f"Starting server on http://{api_host}:{api_port}")
    print(f"Example usage: http://localhost:{api_port}/read?api_key={API_KEY}")
    
    app.run(host=api_host, port=api_port, debug=False)