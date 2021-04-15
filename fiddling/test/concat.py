import os
import subprocess

files = [f for f in os.listdir() if f.endswith(".mp3")]
inputs = " ".join([f"-i {file}" for file in files])
filter_str = "".join([f"[{i + 1}:a:0]" for i, _ in enumerate(files)])
filter_str += f"concat=n={len(files)}:a=1:v=0[outa]"

with open("metadata.txt", "w") as f:
    f.writelines([
        ";FFMETADATA1\n",
        "[CHAPTER]\n",
        "TIMEBASE=1/1000\n",
        "START=0\n",
        "END=50000\n",
        "title=chapter one\n",
        "[CHAPTER]\n",
        "TIMEBASE=1/1000\n",
        "START=50000\n",
        "END=59000\n",
        "title=chapter two\n",
        "[CHAPTER]\n",
        "TIMEBASE=1/1000\n",
        "START=59000\n",
        "END=3862000\n",
        "title=chapter three\n"
    ])

command = f"ffmpeg -i metadata.txt {inputs} " \
          "-map_metadata 0 " \
          f"-filter_complex \"{filter_str}\" " \
          "-map \"[outa]\" " \
          "-ac 2 " \
          "-y " \
          "-codec:a aac " \
          "-b:a 64k " \
          "output.m4b"

print(f"Command: {command}")

out = subprocess.run(command, shell=True)
out = subprocess.run("AtomicParsley output.m4b --artwork cover.png --overWrite", shell=True)

os.remove("metadata.txt")