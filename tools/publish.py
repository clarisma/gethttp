import base64
import hashlib
import os
import requests
import sys
import zipfile


def create_zip(zip_path, files):
    """
    Creates a ZIP file.

    :param zip_path: The path of the ZIP file.
    :param files: A sequence of tuples: (file_name_in_archive, file_path)
    """
    with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zip:
        for member_name, member_path in files:
            zip.write(member_path, member_name)

def calculate_md5(file_path):
    """
    Calculates the MD5 checksum of a file.
    """
    hash_md5 = hashlib.md5()
    with open(file_path, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_md5.update(chunk)
    return hash_md5.hexdigest()


def create_manifest(manifest_path, url, md5_checksum):
    """
    Generate manifest (XML file) with install metadata.
    """
    content = f'<install url="{url}" md5="{md5_checksum}"/>'
    with open(manifest_path, "w") as manifest:
        manifest.write(content)

def upload_to_repository(repo, file_path, dest_path, token):
    """
    Upload a file to a GitHub repository.

    :param repo: The target repository (e.g., 'username/repo').
    :param file_path: The path of the file to upload.
    :param dest_path: The destination path in the repository (e.g., 'folder/file.txt').
    :param token: The GitHub Personal Access Token (PAT).
    """
    # Read file content
    with open(file_path, "rb") as file:
        content = base64.b64encode(file.read()).decode("utf-8")

    # GitHub API endpoint
    url = f"https://api.github.com/repos/{repo}/contents/{dest_path}"

    # API request headers
    headers = {
        "Authorization": f"Bearer {token}",
        "Content-Type": "application/json",
    }

    # API request payload
    payload = {
        "message": f"Added ${dest_path}",
        "content": content,
        "branch": "main",
    }

    # Make the API request
    response = requests.put(url, json=payload, headers=headers)

    # Check response status
    if response.status_code == 201:
        print(f"File '{file_path}' uploaded successfully to '{dest_path}'.")
    elif response.status_code == 200:
        print(f"File '{file_path}' updated successfully in '{dest_path}'.")
    else:
        print(f"Failed to upload file: {response.status_code}")
        print(response.json())
    response.raise_for_status()  # Raise an exception for HTTP errors

def add_executable(files, executable):
    executable_win = executable + ".exe"
    if os.path.isfile(os.path.join("build", executable_win)):
        executable = executable_win
    files.append((executable, os.path.join("build", executable)))


if __name__ == "__main__":
    product = sys.argv[1]       # e.g. "gethttp"
    version = sys.argv[2]       # major.minor.patch
    platform = sys.argv[3]      # e.g. "win-x64"
    token = sys.argv[4]         # the PAT required to access the downloads repo
    executable = product
    release_name = f"{product}-{version}-{platform}"
    latest_release_name = f"{product}-latest-{platform}"
    download_repo = "clarisma/geodesk-download"

    os.makedirs("publish")
    files = []
    add_executable(files, executable)
    zip_path = os.path.join("publish", release_name + ".zip")
    create_zip(zip_path, files)
    md5 = calculate_md5(zip_path)

    url = f"https://download.geodesk.com/{product}/{release_name}.zip"
    manifest_path = os.path.join("publish", release_name + ".xml")
    create_manifest(manifest_path, url, md5)

    upload_to_repository(download_repo, zip_path, f"product/{release_name}.zip", token)
    upload_to_repository(download_repo, manifest_path, f"product/{release_name}.xml", token)
    upload_to_repository(download_repo, manifest_path, f"product/{latest_release_name}.xml", token)
