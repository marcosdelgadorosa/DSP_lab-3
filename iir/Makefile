all:
	@echo "Targets are:"
	@echo "   iir       Compile iir"
	@echo "   runiir    Run iir as a jack app on pulsewire"
	@echo "   linkports Connect jack client 'simple' to HifiBerry ports"

iir: iir.c 
	gcc iir.c -o iir -ljack

runiir:
	pw-jack ./iir

linkports:
	pw-link simple:output_left alsa_output.platform-soc_sound.stereo-fallback:playback_FL
	pw-link simple:output_right alsa_output.platform-soc_sound.stereo-fallback:playback_FR
	pw-link alsa_input.platform-soc_sound.stereo-fallback:capture_FL simple:input_left
	pw-link alsa_input.platform-soc_sound.stereo-fallback:capture_FR simple:input_right

clean:
	rm -f *~ iir

include ../scripts/pipewire.mk
