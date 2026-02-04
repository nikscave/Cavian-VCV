#!/bin/bash
# Script to upload new build artifacts to GitHub Release
# Usage: ./upload-release.sh v2.0.1

TAG=$1
if [ -z "$TAG" ]; then
    echo "Usage: $0 <tag_name>"
    echo "Example: $0 v2.0.1"
    exit 1
fi

# Get token from environment variable or prompt
if [ -z "$GITHUB_TOKEN" ]; then
    echo "Enter your GitHub Personal Access Token: "
    read -s TOKEN
else
    TOKEN=$GITHUB_TOKEN
fi

REPO="nikscave/Cavian-VCV"

# Create release
echo "Creating release $TAG..."
RELEASE_RESPONSE=$(curl -s -X POST \
    -H "Authorization: token $TOKEN" \
    -H "Accept: application/vnd.github.v3+json" \
    -d "{\"tag_name\":\"$TAG\",\"name\":\"$TAG\",\"body\":\"Release $TAG\",\"draft\":false,\"prerelease\":false}" \
    "https://api.github.com/repos/$REPO/releases")

RELEASE_ID=$(echo $RELEASE_RESPONSE | python3 -c "import sys, json; print(json.load(sys.stdin).get('id', ''))")
UPLOAD_URL=$(echo $RELEASE_RESPONSE | python3 -c "import sys, json; print(json.load(sys.stdin).get('upload_url', '').replace('{?name,label}', ''))")

echo "Release created with ID: $RELEASE_ID"
echo "Upload URL: $UPLOAD_URL"

# Upload artifacts
for platform in lin-x64 mac-arm64 mac-x64 win-x64; do
    echo "Uploading $platform plugin..."
    read -p "Drag and drop $platform.vcvplugin here (or press Enter to skip): " file
    if [ -f "$file" ]; then
        curl -s -X POST \
            -H "Authorization: token $TOKEN" \
            -H "Content-Type: application/octet-stream" \
            --data-binary @"$file" \
            "$UPLOAD_URL?name=$(basename $file)"
        echo "  Uploaded: $(basename $file)"
    else
        echo "  Skipped: $platform"
    fi
done

echo ""
echo "Release created: https://github.com/$REPO/releases/tag/$TAG"
