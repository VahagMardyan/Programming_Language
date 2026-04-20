## Guide: Setting Up Native VHG Language Support in VS Code

This setup allows VS Code to recognize `.vhg` files as a formal language, enabling **perfect "Ctrl + /" comment toggling**, auto-closing brackets, and custom syntax highlighting without any buggy snippets.

### Step 1: Create the Extension Folder
1. Open your File Explorer.
2. Navigate to the VS Code extensions folder. You can do this by pasting this path into your address bar:
   `%USERPROFILE%\.vscode\extensions` (on Windows).
3. Create a new folder named `vhg-lang`.

* Or you can just simply copy and paste this folder into `%USERPROFILE%\.vscode\extensions`. Rename this folder to: 
    `vahagn-mardyan.vhg-language-0.0.1`
---

### Step 2: Create the Configuration Files
Inside the `vhg-lang` folder, create the following two files:

#### 1. `package.json`
This file registers the language ID and connects the `.vhg` extension.
```json
{
    "name": "vhg-language",
    "displayName": "VHG Language",
    "version": "0.0.1",
    "engines": { "vscode": "^1.60.0" },
    "categories": ["Programming Languages"],
    "publisher": "vahagn-mardyan",
    "contributes": {
        "languages": [{
            "id": "vhg",
            "aliases": ["VHG"],
            "extensions": [".vhg"],
            "configuration": "./language-configuration.json"
        }],
        "grammars": [{
            "language": "vhg",
            "scopeName": "source.vhg",
            "path": "./syntaxes/vhg.tmLanguage.json"
        }]
    }
}
```

#### 2. `language-configuration.json`
This file defines the behavior of the editor (comments and brackets).
```json
{
    "comments": {
        "lineComment": {
            "comment" : "#",
            "noIndent" : false
        },
        "blockComment": ["#*", "*#"]
    },
    "brackets": [
        ["{", "}"],
        ["(", ")"],
        ["[", "]"]
    ],
    "autoClosingPairs": [
        { "open": "{", "close": "}" },
        { "open": "(", "close": ")" },
        { "open": "[", "close": "]" },
        { "open": "\"", "close": "\"" },
        { "open": "'", "close": "'" }
    ]
}
```
#### 3. `syntaxes/vhg.tmLangguage.json`
```json
{
    "name": "VHG",
    "scopeName": "source.vhg",
    "patterns": [
        { "include": "#comments" },
        { "include": "#strings" },
        { "include": "#keywords" },
        { "include": "#builtIn" },
        { "include": "#boolean"},
        { "include": "#logical_operators" },
        { "include": "#numbers" },
        { "include": "#variables" }
    ],
    "repository": {
        "comments": {
            "patterns": [
                {
                    "name": "comment.block.vhg",
                    "begin": "#\\*",
                    "end": "\\*#"
                },
                {
                    "name": "comment.line.vhg",
                    "match": "#.*$"
                }
            ]
        },
        "strings": {
            "patterns": [
                {
                    "name": "string.quoted.double.vhg",
                    "begin": "\"",
                    "end": "\"",
                    "patterns": [{
                        "name": "constant.character.escape.vhg",
                        "match": "\\\\."
                    }]
                },
                {
                    "name": "string.quoted.single.vhg",
                    "begin": "'",
                    "end": "'",
                    "patterns": [{
                        "name": "constant.character.escape.vhg",
                        "match": "\\\\."
                    }]
                }
            ]
        },
        "keywords": {
            "match": "\\b(if|else|while|for|print|function|return|void)\\b",
            "name": "keyword.control.vhg"
        },
        "boolean": {
            "match": "\\b(true|false)\\b",
            "name": "keyword.control_boolean.vhg"
        },
        "logical_operators": {
            "match": "\\b(and|or|not|local|global)\\b",
            "name": "keyword.operator.logical.vhg"
        },
        "numbers": {
            "match": "\\b\\d+(\\.\\d+)?\\b",
            "name": "constant.numeric.vhg"
        },
        "variables": {
            "match": "\\b[a-zA-Z_][a-zA-Z0-9_]*\\b",
            "name": "variable.other.vhg"
        },
        "builtIn": {
            "patterns": [
                {
                    "match": "\\b(length|void)\\b",
                    "name": "support.function.vhg"
                }
            ]
        }
    },
    "operators" : {
        "match" : "(\\+|-|\\*|/|%|&|\\||\\^|<<|>>)?=",
        "name" : "keyword.operator.assignment.vhg"
    }
}
```

---

### Step 3: Update VS Code Global Settings
Now, tell VS Code to use this new "vhg" language for your files. 
1. Open VS Code.
2. Press `Ctrl + Shift + P`, type **"Open User Settings (JSON)"**, and press Enter.
3. Find or add the `files.associations` section and set it like this:

```json
"files.associations": {
    "*.vhg": "vhg"
}
```

4. Ensure your `highlight.regexes` are still there to provide the colors (as we configured previously), but make sure they target the `.vhg` files.

---

### Step 4: Custom Syntax Highlighting (Colors)
To apply colors to your language, add the following block to your `settings.json` under the `highlight.regexes` section. This ensures keywords, numbers, and comments are colored specifically for the `vhg` language:
```json
    "editor.tokenColorCustomizations": {
    "textMateRules": [
        { "scope": "keyword.control.vhg", "settings": { "foreground": "#C586C0" } },
        { "scope": "keyword.operator.logical.vhg", "settings": { "foreground": "#0b4da3" } },
        { "scope": "keyword.control_boolean.vhg", "settings": {"foreground": "#1e73ea",} },
        { "scope": "string.quoted.double.vhg", "settings": { "foreground": "#CE7744" } },
        { "scope": "string.quoted.single.vhg", "settings": { "foreground": "#CE7744" } },
        { "scope": "constant.numeric.vhg", "settings": { "foreground": "#B5CEA8" } },
        { "scope": "variable.other.vhg", "settings": { "foreground": "#9CDCFE" } },
        { "scope": "comment.line.vhg", "settings": { "foreground": "#6A9955", "fontStyle": "italic" } },
        { "scope": "comment.block.vhg", "settings": { "foreground": "#6A9955", "fontStyle": "italic" } },
        { "scope": "support.function.vhg", "settings": { "foreground": "#007bff" } }
    ]
    },
```

### Step 5: Finalize
1. **Remove old shortcuts:** Delete any `ctrl + /` bindings in `keybindings.json`.
2. **Restart VS Code:** This is required for the new extension to load.

### Key Features of this Setup:
* Native Toggle: `Ctrl + /` now works instantly using the `#` symbol.
* Auto-Close: Typing `#*`, `(`, `[` or `{` will automatically generate the closing pair.
* Semantic Colors: Your code is now visually organized and easy to read.