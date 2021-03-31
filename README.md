# tba

Basic directory structure:
* `lib/`: library source code
* `tutorial_game/`: game using library

The library and tutorial game are compiled separately.

To build, please make sure you have Clang 11 installed.

I recommend using the clangd extension for debugging and intellisense on VS Code. With the current setup, errors still show up on clangd, but it largely works once you do an initial compilation.

If you are using VS Code, I also recommend adding `""*.cppm": "cpp"` to your `.vscode/settings.json`.