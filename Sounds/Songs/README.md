# Transform MP3 to WAV

## Install sox (Linux)
```bash
# For Ubuntu
sudo apt-get update
sudo apt-get install sox libsox-fmt-all
```

## Use sox
```bash
sox <input.mp3> -c 1 -r 48000 -b 16 <output.wav>
```
where:
- `-c 1` mono audio
- `-r 48000` sample rate
- `b 16` 16 bits per sample
