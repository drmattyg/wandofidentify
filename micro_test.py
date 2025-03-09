import urequests
import base64
import json
import ubinascii
import uhashlib
import time

# AWS credentials - replace with your values
AWS_ACCESS_KEY = "YOUR_ACCESS_KEY"
AWS_SECRET_KEY = "YOUR_SECRET_KEY"
AWS_REGION = "us-east-2"
LAMBDA_FUNCTION = "recognizeImage"

def sign_request(key, msg):
    """Create HMAC SHA256 signature"""
    return ubinascii.hexlify(uhashlib.sha256(key + msg).digest())

def get_signature(date_stamp, region_name, service_name):
    """Calculate AWS signature"""
    k_date = sign_request(('AWS4' + AWS_SECRET_KEY).encode('utf-8'), date_stamp.encode('utf-8'))
    k_region = sign_request(k_date, region_name.encode('utf-8'))
    k_service = sign_request(k_region, service_name.encode('utf-8'))
    k_signing = sign_request(k_service, b'aws4_request')
    return k_signing

def recognize_image(image_data):
    """
    Send image to AWS Lambda for recognition
    Args:
        image_data: bytes from camera capture
    Returns:
        label: string with recognized object
    """
    try:
        # Convert image to base64
        image_b64 = base64.b64encode(image_data).decode('utf-8')
        
        # Prepare the request
        host = f"lambda.{AWS_REGION}.amazonaws.com"
        endpoint = f"https://{host}/2015-03-31/functions/{LAMBDA_FUNCTION}/invocations"
        
        # Get current timestamp
        now = time.gmtime()
        amz_date = f"{now[0]}{now[1]:02d}{now[2]:02d}T{now[3]:02d}{now[4]:02d}{now[5]:02d}Z"
        date_stamp = f"{now[0]}{now[1]:02d}{now[2]:02d}"
        
        # Create canonical request
        payload = json.dumps({"image": image_b64})
        payload_hash = ubinascii.hexlify(uhashlib.sha256(payload.encode('utf-8')).digest()).decode('utf-8')
        
        # Prepare headers
        headers = {
            'Content-Type': 'application/json',
            'X-Amz-Date': amz_date,
            'X-Amz-Target': f'AWS_Lambda.{LAMBDA_FUNCTION}',
            'Host': host
        }
        
        # Get signature
        signature = get_signature(date_stamp, AWS_REGION, 'lambda')
        
        # Add authorization header
        credential_scope = f"{date_stamp}/{AWS_REGION}/lambda/aws4_request"
        headers['Authorization'] = (
            f"AWS4-HMAC-SHA256 Credential={AWS_ACCESS_KEY}/{credential_scope}, "
            f"SignedHeaders=content-type;host;x-amz-date;x-amz-target, "
            f"Signature={signature.decode('utf-8')}"
        )
        
        # Send request
        response = urequests.post(endpoint, headers=headers, data=payload)
        
        if response.status_code == 200:
            result = json.loads(response.text)
            return result.get('label', 'Unknown')
        else:
            return f"Error: {response.status_code}"
            
    except Exception as e:
        return f"Error: {str(e)}"
    finally:
        if 'response' in locals():
            response.close()

# Example usage:
"""
# First set your AWS credentials
AWS_ACCESS_KEY = "your_access_key"
AWS_SECRET_KEY = "your_secret_key"

# Then use with your camera
photo = camera.capture()
label = recognize_image(photo)
print("Detected:", label)
""" 