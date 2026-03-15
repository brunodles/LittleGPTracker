# Docker Build Images

This folder contains Docker images to build LittleGPTracker on Ubuntu without installing toolchains directly on the host.

## Prerequisites

- Docker installed and working (`docker --version`)
- Repository available locally (mounted into the container as a volume)
- Run commands from the repository root (`LittleGPTracker/`)

## 1) Build the images

```bash
docker build -f tools/Dockerfile.rg35xxplus -t lgpt-rg35xxplus .
docker build -f tools/Dockerfile.x64 -t lgpt-x64 .
docker build -f tools/Dockerfile.ubuntu-all -t lgpt-ubuntu-all .
```

## 2) Use each image

### RG35XXPLUS image

This image defaults to `make PLATFORM=RG35XXPLUS`.

```bash
docker run --rm -it -v "$PWD:/workspace" lgpt-rg35xxplus
```

Open a shell instead of auto-building:

```bash
docker run --rm -it -v "$PWD:/workspace" --entrypoint bash lgpt-rg35xxplus
```

### X64 image

This image defaults to `make PLATFORM=X64`.

```bash
docker run --rm -it -v "$PWD:/workspace" lgpt-x64
```

Open a shell instead of auto-building:

```bash
docker run --rm -it -v "$PWD:/workspace" --entrypoint bash lgpt-x64
```

### Ubuntu-all image

This image opens a shell by default and includes extra dependencies/toolchains for multiple Ubuntu-run builds.

```bash
docker run --rm -it -v "$PWD:/workspace" lgpt-ubuntu-all
```

From inside the container:

```bash
cd projects
make PLATFORM=X64
make PLATFORM=X86
make PLATFORM=RG35XXPLUS
```

## Use as your local user (optional)

If you want generated files to be owned by your local user instead of root:

```bash
docker run --rm -it -u "$(id -u):$(id -g)" -v "$PWD:/workspace" lgpt-x64
```

You can apply the same `-u` option to the other images.

## 4) Where outputs are written

Build artifacts are written into the `projects` folder.
