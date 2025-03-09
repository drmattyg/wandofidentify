import json
import boto3
import base64

# Configure AWS region
rekognition = boto3.client("rekognition", region_name="us-east-2")

def lambda_handler(event, context):
    try:
        # Decode the Base64 image
        image_data = base64.b64decode(event["image"])
        
        # Call Rekognition
        response = rekognition.detect_labels(
            Image={"Bytes": image_data},
            MaxLabels=1,  # Get only the main object
            MinConfidence=70
        )
        
        if response["Labels"]:
            label = response["Labels"][0]["Name"]
        else:
            label = "Unknown"
        
        return {"label": label}
    
    except Exception as e:
        return {"error": str(e)}