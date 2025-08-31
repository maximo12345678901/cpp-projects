ffmpeg -framerate 120 -i frames/frame_%04d.png -c:v libx264 -pix_fmt yuv420p output.mp4
