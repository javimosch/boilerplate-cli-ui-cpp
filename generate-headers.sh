#!/bin/bash
# Generate C++ header files from UI source files

set -e

UI_DIR="ui"
INCLUDE_DIR="include/ui"

# Function to convert file to header with custom delimiter
convert_to_header() {
    local input_file="$1"
    local output_file="$2"
    local var_name="$3"
    
    echo "// Auto-generated from $input_file - DO NOT EDIT" > "$output_file"
    echo "#pragma once" >> "$output_file"
    echo "" >> "$output_file"
    echo "const char* $var_name = R\"UIEOF(" >> "$output_file"
    cat "$input_file" >> "$output_file"
    echo ")UIEOF\";" >> "$output_file"
    
    echo "  Generated: $output_file"
}

echo "Generating C++ headers from UI files..."

# Convert index.html
convert_to_header "$UI_DIR/index.html" "$INCLUDE_DIR/index_html.h" "INDEX_HTML"

# Convert app.js
convert_to_header "$UI_DIR/app.js" "$INCLUDE_DIR/app_js.h" "APP_JS"

# Convert styles.css
convert_to_header "$UI_DIR/css/styles.css" "$INCLUDE_DIR/styles_css.h" "STYLES_CSS"

# Convert components
for file in $UI_DIR/js/components/*.js; do
    filename=$(basename "$file" .js)
    var_name=$(echo "$filename" | sed 's/\b\(.\)/\u\1/g')  # PascalCase
    convert_to_header "$file" "$INCLUDE_DIR/components/${filename}_js.h" "${var_name}_JS"
done

# Convert views
for file in $UI_DIR/js/views/*.js; do
    filename=$(basename "$file" .js)
    var_name=$(echo "$filename" | sed 's/\b\(.\)/\u\1/g')  # PascalCase
    convert_to_header "$file" "$INCLUDE_DIR/views/${filename}_js.h" "${var_name}_JS"
done

echo "Done! Headers generated in $INCLUDE_DIR/"
