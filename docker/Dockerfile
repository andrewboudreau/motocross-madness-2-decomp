FROM debian:bookworm-slim

RUN dpkg --add-architecture i386 \
 && apt-get update \
 && apt-get install -y --no-install-recommends \
      python3 make gcc libarchive-dev cabextract binutils clang wine wine32:i386 \
 && rm -rf /var/lib/apt/lists/*

ENV WINEARCH=win32 \
    WINEPREFIX=/wine \
    WINEDEBUG=-all \
    VC6_ROOT=/toolchains/vc6sp3

WORKDIR /repo
COPY . /repo

# Mount a privately-owned VC6 SP3 tree at /toolchains/vc6sp3 and the game
# installer somewhere under /input. The compiler itself is not part of image.
CMD ["python3", "tools/bootstrap.py", "/input/MCM2PCG.exe"]
