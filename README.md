# Dungeons and Trolls Generator

Generates floors of various layouts and filled with monsters and traps.

## Usage

Example:

```bash
./dntgenerator -s 10 -e 30 -m 40 -j dung.json -h dung.html
```

This will generate two files: `dung.json` and `dung.html`.
Both files contain same corresponding data, but the json is designed for machine input, and the html is for human verification.

The generated dungeon contains floors starting at level 10 and ending at level 30 inclusive.
The 40 signifies the maximum depth that players have ever reached.
