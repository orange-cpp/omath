#!/bin/bash

# Compile Tailwind CSS
pixi run tailwindcss -i ./docs/styles/tailwind.css -o ./docs/styles/tailwind.generated.css --minify

pixi run mkdocs build

echo "Tailwind CSS compiled and MkDocs site built!"
