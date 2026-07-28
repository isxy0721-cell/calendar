#!/usr/bin/env python3
"""Offline Vosk transcription helper for CalendarDemo on Linux.

Usage: linux_voice.py <16 kHz mono WAV file>
Set MYSCHEDULE_VOSK_MODEL to the unpacked Vosk model directory.
"""
import json
import os
import sys
import wave


def main() -> int:
    if len(sys.argv) != 2:
        print("ERROR: expected WAV input", file=sys.stderr)
        return 2
    model_path = os.environ.get("MYSCHEDULE_VOSK_MODEL")
    if not model_path or not os.path.isdir(model_path):
        print("ERROR: set MYSCHEDULE_VOSK_MODEL to an unpacked Vosk model directory", file=sys.stderr)
        return 3
    try:
        from vosk import KaldiRecognizer, Model, SetLogLevel
    except ImportError:
        print("ERROR: install the Vosk Python package: python3 -m pip install vosk", file=sys.stderr)
        return 4

    try:
        wav = wave.open(sys.argv[1], "rb")
        if wav.getnchannels() != 1 or wav.getsampwidth() != 2 or wav.getframerate() != 16000:
            print("ERROR: audio must be 16 kHz, 16-bit, mono PCM WAV", file=sys.stderr)
            return 5
        SetLogLevel(-1)
        recognizer = KaldiRecognizer(Model(model_path), wav.getframerate())
        while True:
            data = wav.readframes(4000)
            if not data:
                break
            recognizer.AcceptWaveform(data)
        result = json.loads(recognizer.FinalResult())
        text = result.get("text", "").strip()
        if not text:
            print("ERROR: no speech recognized", file=sys.stderr)
            return 6
        print("RESULT:" + text)
        return 0
    except Exception as error:
        print("ERROR: " + str(error), file=sys.stderr)
        return 7


if __name__ == "__main__":
    raise SystemExit(main())
