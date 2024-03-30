#!/bin/bash

# Check for the presence of an argument (path to the Markdown file)
if [ "$#" -ne 1 ]; then
  echo "Usage: $0 <path_to_markdown_file>"
  exit 1
fi

# The path to the file, obtained from the argument
FILE_PATH="$1"

# Check if the file exists
if [ ! -f "$FILE_PATH" ]; then
  echo "File not found: $FILE_PATH"
  exit 1
fi

# Temporary file for the processed content
TEMP_FILE=$(mktemp)

# Step 1: Remove lines that are list items (starting with '-', accounting for spaces/tabs before '-')
# Step 2: Remove all empty lines
# Step 3: Add an empty line after each remaining line
awk '{
  # Step 1: Skip list item lines
  if (/^[[:space:]]*-/) next;
  # Step 2 is handled implicitly by not printing empty lines here
  if (!/^[[:space:]]*$/) {
    # Step 3: Print the line and then a newline
    print $0 "\n";
  }
}' "$FILE_PATH" > "$TEMP_FILE"

# Replace the original file with the temporary file
mv "$TEMP_FILE" "$FILE_PATH"
echo "File $FILE_PATH was cleaned"