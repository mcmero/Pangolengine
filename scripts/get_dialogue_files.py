"""
This script is used for generating dialogue files for the game engine.
Given a set of credentials (under $HOME/credentials.json) a folder ID as
input argument, it will fetch all spreadsheets under the folder and convert
them to JSON files, ready to be used as dialogue files.

Usage: python get_dialogue_files.py <folder_id>
"""

import json
import os.path
import pickle
import argparse

import gspread
from google_auth_oauthlib.flow import InstalledAppFlow
from google.auth.transport.requests import Request
from googleapiclient.discovery import build

# OAuth2 credentials file containing client secret
credentials_file = os.path.join(os.path.expanduser(
    "~"), "credentials.json")

# Config file containing folder_id where dialogue files
# are stored as spreadsheets on Google docs
config_file = os.path.join(os.path.expanduser("~"), "gdocs_config.json")

# Scope for accessing Google Sheets and Google Drive
SCOPES = ["https://www.googleapis.com/auth/spreadsheets.readonly",
          "https://www.googleapis.com/auth/drive.metadata.readonly",
          "https://www.googleapis.com/auth/drive.readonly"]


def authenticate():
    creds = None
    # The file token.pickle stores the user's access and refresh tokens, and is
    # created automatically when the authorization flow completes once.
    pickle_file = os.path.join(os.path.expanduser(
        "~"), "token.pickle")
    if os.path.exists(pickle_file):
        with open(pickle_file, "rb") as token:
            creds = pickle.load(token)
    # If there are no (valid) credentials available, let the user log in.
    if not creds or not creds.valid:
        if creds and creds.expired and creds.refresh_token:
            creds.refresh(Request())
        else:
            flow = InstalledAppFlow.from_client_secrets_file(
                credentials_file, SCOPES)
            creds = flow.run_local_server(port=0)
        # Save the credentials for the next run
        with open(pickle_file, "wb") as token:
            pickle.dump(creds, token)
    return creds


def get_sheets_in_folder(folder_id):
    creds = authenticate()
    drive_service = build("drive", "v3", credentials=creds)

    # Query to find all Google Sheets in the specified folder
    mime_type = "application/vnd.google-apps.spreadsheet"
    query = f"'{folder_id}' in parents and mimeType='{mime_type}'"
    results = drive_service.files().list(
        q=query, fields="files(id, name)"
    ).execute()
    items = results.get('files', [])

    return items


def process_dialogue(data):
    # Nest responses under each dialogue line
    processed_data = []
    for i, item in enumerate(data):
        # Set default next node to 0 if blank
        if item["next"] == "":
            item["next"] = -1

        # Append responses to previous for items with blank IDs
        if item["id"] == "" and processed_data:
            processed_data[-1]["responses"].append(
                {"response": item["responses"], "next": item["next"]})
            continue

        if "responses" not in item:
            continue

        # Set/append responses if blank or first response
        if item["responses"] == "" or item["responses"] is None:
            item["responses"] = []
        else:
            item["responses"] = [
                {"response": item["responses"], "next": item["next"]}]
        processed_data.append(item)

    return processed_data


def export_to_json(name):
    # Convert spreadsheet to JSON output
    creds = authenticate()
    client = gspread.authorize(creds)

    # Open spreadsheet and load as a dict
    spreadsheet = client.open(name)
    sheet = spreadsheet.sheet1
    data = sheet.get_all_records()

    # Process dialogue and write to JSON file
    dialogue = process_dialogue(data)
    with open(f"assets/scenes/{name}.json", "w") as json_file:
        json.dump(dialogue, json_file, indent=4)

    print(f"Exported dialogue file: {name}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "folder_id",
        type=str,
        help="Google Drive folder ID containing dialogue files"
    )
    args = parser.parse_args()

    items = get_sheets_in_folder(args.folder_id)
    for item in items:
        export_to_json(item["name"])


if __name__ == "__main__":
    main()
