#!/bin/bash

# Compile Tailwind CSS
pixi run tailwindcss -i docs/styles/tailwind.css -o docs/styles/tailwind.generated.css --minify

echo "Tailwind CSS compiled successfully!"

pixi run mkdocs build

echo "MkDocs site built successfully!"
