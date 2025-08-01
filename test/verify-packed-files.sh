#!/usr/bin/env bash

set -euo pipefail

PACKAGE_JSON="package.json"
TARBALL=$(npm pack)
EXTRACT_DIR=$(mktemp -d)
ERRORS=0

echo "📦 Created package: $TARBALL"
tar -xzf "$TARBALL" -C "$EXTRACT_DIR"

# Extract the list of expected files/folders from package.json
EXPECTED_PATHS=$(jq -r '.files[]' "$PACKAGE_JSON")

echo "🔍 Verifying files listed in package.json..."

for path in $EXPECTED_PATHS; do
  # Strip globs like ** or * for checking root path
  CLEAN_PATH=$(echo "$path" | sed -E 's/[*/]+.*//')
  if [[ -z "$CLEAN_PATH" ]]; then
    continue
  fi

  # Check if the cleaned path exists in the package/ directory of the tarball
  if [ -e "$EXTRACT_DIR/package/$CLEAN_PATH" ]; then
    echo "✅ Found: $CLEAN_PATH"
  else
    echo "❌ Missing: $CLEAN_PATH"
    ERRORS=$((ERRORS + 1))
  fi
done

rm -rf "$EXTRACT_DIR"
rm "$TARBALL"

if [ "$ERRORS" -ne 0 ]; then
  echo "❗ $ERRORS missing file(s) from package.json files list."
  exit 1
else
  echo "🎉 All files from package.json 'files' list are present in the package."
fi

