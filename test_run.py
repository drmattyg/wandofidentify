import base64
import os
import boto3
import json
from PIL import Image, ImageDraw

def create_test_image():
    """Create a more realistic boat shape"""
    # Create a new image with a sky background
    img = Image.new('RGB', (800, 600), (135, 206, 235))  # Sky blue
    draw = ImageDraw.Draw(img)
    
    # Draw water
    draw.rectangle([(0, 300), (800, 600)], fill=(0, 105, 148))  # Deep blue
    
    # Draw boat hull (more curved)
    hull_points = [
        (200, 350),  # Start
        (250, 300),  # Top front
        (550, 300),  # Top back
        (600, 350),  # Back
        (550, 400),  # Bottom back
        (250, 400),  # Bottom front
    ]
    draw.polygon(hull_points, fill=(139, 69, 19))  # Brown
    
    # Draw main sail (triangular)
    draw.polygon([
        (350, 100),  # Top
        (350, 300),  # Bottom
        (450, 200),  # Back
    ], fill=(255, 255, 255), outline=(0, 0, 0))
    
    # Draw smaller front sail
    draw.polygon([
        (300, 150),  # Top
        (300, 300),  # Bottom
        (350, 225),  # Back
    ], fill=(255, 255, 255), outline=(0, 0, 0))
    
    # Draw mast
    draw.line([(350, 100), (350, 400)], fill=(101, 67, 33), width=5)  # Dark brown
    
    # Add some waves
    for i in range(0, 800, 50):
        draw.arc([i, 280, i+50, 320], 0, 180, fill=(0, 125, 168))
    
    img_path = 'test_boat.jpg'
    img.save(img_path, quality=95)
    return img_path

def test_with_sample_image():
    # Create a test image
    img_path = create_test_image()
    
    try:
        # Read the image file
        with open(img_path, 'rb') as image_file:
            image_data = image_file.read()
        
        # Create the event payload
        event = {
            "image": base64.b64encode(image_data).decode('utf-8')
        }
        
        # Initialize Lambda client
        lambda_client = boto3.client('lambda', region_name='us-east-2')
        
        # Call the Lambda function
        response = lambda_client.invoke(
            FunctionName='recognizeImage',
            InvocationType='RequestResponse',
            Payload=json.dumps(event)
        )
        
        # Parse the response
        response_payload = json.loads(response['Payload'].read().decode('utf-8'))
        print("Lambda Response:", response_payload)
        
    except Exception as e:
        print(f"Error: {str(e)}")
    finally:
        # Clean up
        os.remove(img_path)

if __name__ == "__main__":
    test_with_sample_image() 