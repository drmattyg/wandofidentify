import os
import configparser
import pathlib

def setup_aws_credentials():
    # Create ~/.aws directory if it doesn't exist
    aws_dir = os.path.expanduser("~/.aws")
    pathlib.Path(aws_dir).mkdir(parents=True, exist_ok=True)
    
    # Create or update credentials file
    config = configparser.ConfigParser()
    
    # Get credentials from user
    print("Please enter your AWS credentials:")
    aws_access_key = input("AWS Access Key ID: ")
    aws_secret_key = input("AWS Secret Access Key: ")
    
    # Update credentials file
    config["default"] = {
        "aws_access_key_id": aws_access_key,
        "aws_secret_access_key": aws_secret_key,
        "region": "us-east-1"
    }
    
    # Write to credentials file
    with open(os.path.join(aws_dir, "credentials"), "w") as f:
        config.write(f)
    
    print("\nAWS credentials configured successfully!")

if __name__ == "__main__":
    setup_aws_credentials() 