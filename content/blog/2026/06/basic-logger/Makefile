.PHONY: extract

export MAKEFILE_LIST

extract:
	@echo "Checking ESP-IDF terminal..."
	@if [ -z "$$IDF_PATH" ] && [ -z "$$ESP_IDF" ]; then \
		echo "Error: This must be run in an ESP-IDF terminal. Please source the ESP-IDF environment first."; \
		exit 1; \
	fi

	@echo "Checking for uv..."
	@which uv > /dev/null 2>&1 || (echo "Error: uv is not installed. Install it with: pip install uv" && exit 1)

	@echo "Determining PORT..."; \
	if [ -n "$(PORT)" ]; then \
		echo "Using PORT from command line: $(PORT)"; \
		FINAL_PORT="$(PORT)"; \
	elif [ -f ".vscode/settings.json" ] && grep -q '"idf.port"' .vscode/settings.json; then \
		FINAL_PORT=$$(grep '"idf.port"' .vscode/settings.json | sed 's/.*: *"\([^"]*\)".*/\1/'); \
		echo "Using PORT from settings.json: $$FINAL_PORT"; \
	else \
		read -p "Enter PORT (e.g., /dev/ttyUSB0): " FINAL_PORT; \
	fi; \
	if [ -z "$$FINAL_PORT" ]; then \
		echo "Error: PORT must be provided"; \
		exit 1; \
	fi; \
	timestamp=$$(date +%Y%m%d_%H%M%S); \
	OUTPUT_FILE="littlefs_extracted_$$timestamp.bin"; \
	echo "Extracting partition to $$OUTPUT_FILE..."; \
	python $$IDF_PATH/components/partition_table/parttool.py --port "$$FINAL_PORT" read_partition --partition-name=storage --output "$$OUTPUT_FILE" && \
	echo "Listing LittleFS contents..."; \
	uvx --from littlefs-python littlefs-python extract "$$OUTPUT_FILE" --block-size=4096 output/