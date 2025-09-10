CONFIG := $(HOME)/.config/pipewire/pipewire.conf
VALID_RATES := 44100 48000 88200 96000 176400 192000

.PHONY: setrate getrate

setrate:
	@if [ -z "$(RATE)" ]; then \
		echo "Usage: make setrate RATE=44100"; \
		echo "Allowed values: $(VALID_RATES)"; \
		exit 0; \
	fi
	@if ! echo "$(VALID_RATES)" | grep -wq "$(RATE)"; then \
		echo "Error: Invalid RATE '$(RATE)'. Allowed values: $(VALID_RATES)"; \
		exit 2; \
	fi
	@if [ ! -f "$(CONFIG)" ]; then \
		echo "Error: $(CONFIG) not found."; \
		exit 2; \
	fi
	@sed -i -E 's|^([[:space:]]*default\.clock\.rate[[:space:]]*=[[:space:]]*)[0-9]+|\1$(RATE)|' "$(CONFIG)"
	@systemctl --user restart pipewire pipewire-pulse

getrate:
	@grep -E '^[[:space:]]*default\.clock\.rate[[:space:]]*=' "$(CONFIG)" | \
	sed -E 's/^[[:space:]]*default\.clock\.rate[[:space:]]*=[[:space:]]*([0-9]+).*/\1/'
