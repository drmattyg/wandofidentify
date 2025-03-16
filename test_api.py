import requests
import base64
from pathlib import Path

with open('api_key.txt', 'r') as file:
    API_KEY = file.read().strip()

def test_image_recognition(image_path, api_url):
    """
    Test the image recognition API with a local image file
    
    Args:
        image_path (str): Path to the image file
        api_url (str): Your API Gateway invoke URL
    """
    try:
        # Read image file
        with open(image_path, 'rb') as image_file:
            image_data = image_file.read()
        
        # Convert to base64
        image_b64 = base64.b64encode(image_data).decode('utf-8')
        
        # Prepare the request
        headers = {
            'Content-Type': 'application/json',
            'x-api-key': API_KEY
        }
        
        payload = {
            'image': image_b64
        }
        
        # Send request
        print(f"Sending request to {api_url}")
        response = requests.post(api_url, json=payload, headers=headers)
        
        # Check response
        if response.status_code == 200:
            print("Success!")
            print("Response:", response.json())
        else:
            print(f"Error: {response.status_code}")
            print("Response:", response.text)
            
    except Exception as e:
        print(f"Error: {str(e)}")

if __name__ == "__main__":
    # Replace these values with your actual API Gateway URL and image path
    API_URL = "https://2i05n9ncye.execute-api.us-east-2.amazonaws.com/Prod/wandOfIdentify"
    IMAGE_PATH = "airplane.png"  # Put a test image in the same directory
    
    test_image_recognition(IMAGE_PATH, API_URL) 