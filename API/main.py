# This should use almost no memory

from fastapi import FastAPI
from fastapi import HTTPException
import dotenv
import os
from datetime import datetime, timedelta

dotenv.load_dotenv()
LOVE_KEY  = os.getenv("LOVE_KEY")
START_TIME = datetime.now()

app = FastAPI()

# Just easy search
users = {
    0: 1,
    1: 0
}

# No love sent yet
love_sent = {
    0: START_TIME,
    1: START_TIME
}

# Just using root since it's not a complex API
@app.get("/{key}", status_code=200)
def read_root(key: str, user_identifier: int):
    if key != LOVE_KEY:
        raise HTTPException(status_code=403, detail="Invalid key, this isn't for you...")
    
    # Checks if the other use sent love within two minutes.
    if datetime.now() - love_sent[users[user_identifier]] < timedelta(seconds=10):
        return {"love_recieved": True}
    
    else:
        return {"love_recieved": False}

# Sends love to the other user
@app.post("/send_love", status_code=201)
def send_love(key: str, user_identifier: int):
    if key != LOVE_KEY:
        raise HTTPException(status_code=403, detail="Invalid key, this isn't for you...")
    
    love_sent[user_identifier] = datetime.now()
    return {"love_sent": True}


"""
Get reqs just check for love sent within the past 2 minutes
    There are only two users so we'll just send a single identifier to determine who sent love

Post reqs send love to the server
"""