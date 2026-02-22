for dir in ctype stdlib string; do
    echo "--- Diffing $dir ---"
    HIS_DIR="plauger/c_standard_lib-master/$(echo $dir | tr '[:lower:]' '[:upper:]')"
    
    for file in src/$dir/*; do
        # Map local filename to his uppercase filename
        FILENAME=$(basename "$file" | tr '[:lower:]' '[:upper:]')
        HIS_FILE="$HIS_DIR/$FILENAME"
        
        # Check if it exists in both
        if [ -f "$file" ] && [ -f "$HIS_FILE" ]; then
            diff -buq "$file" "$HIS_FILE"
        fi
    done
done
