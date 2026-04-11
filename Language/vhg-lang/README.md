## Guide: Setting Up Native VHG Language Support in VS Code

This setup allows VS Code to recognize `.vhg` files as a formal language, enabling **perfect "Ctrl + /" comment toggling**, auto-closing brackets, and custom syntax highlighting without any buggy snippets.

### Step 1: Create the Extension Folder
1. Open your File Explorer.
2. Navigate to the VS Code extensions folder. You can do this by pasting this path into your address bar:
   `%USERPROFILE%\.vscode\extensions` (on Windows).
3. Create a new folder named `vhg-lang`.

* Or you can just simply copy and paste this folder into `%USERPROFILE%\.vscode\extensions` (without this README of course).
---

### Step 2: Create the Configuration Files
Inside the `vhg-lang` folder, create the following two files:

#### 1. `package.json`
This file registers the language ID and connects the `.vhg` extension.
```json
{
    "name": "vhg-lang",
    "displayName": "VHG Language Support",
    "description": "Native support for .vhg files",
    "version": "0.1.0",
    "publisher": "Vahagn Mardyan",
    "engines": {
        "vscode": "^1.0.0"
    },
    "contributes": {
        "languages": [
            {
                "id": "vhg",
                "aliases": ["VHG", "vhg"],
                "extensions": [".vhg"],
                "configuration": "./language-configuration.json"
            }
        ]
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
        "blockComment": [ "#*", "*#" ]
    },
    "brackets": [
        ["{", "}"],
        ["[", "]"],
        ["(", ")"]
    ],
    "autoClosingPairs": [
        { "open": "{", "close": "}" },
        { "open": "[", "close": "]" },
        { "open": "(", "close": ")" },
        { "open": "\"", "close": "\"" },
        { "open": "#*", "close": "*#" }
    ],
    "surroundingPairs": [
        ["{", "}"],
        ["[", "]"],
        ["(", ")"],
        ["\"", "\""],
        ["#*", "*#"]
    ]
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
    "highlight.regexes": {
        // 1. Strings 
        "\"[^\"]*\"" : {
            "filterFileRegex": ".*\\.vhg$",
            "decorations": [{
                "color": "rgb(206, 119, 76)"
            }]
        },
        // 2. Multi-line and Single-line comments
        "(#[*](?:.|\\n|\\r)*?[*][#])": {
            "filterFileRegex": ".*\\.vhg$",
            "decorations": [
                {
                    "color": "#6A9955", "fontStyle": "italic"
                }
            ]
        },
        "(#[^*\\n].*)": {
            "filterFileRegex": ".*\\.vhg$",
            "decorations": [{ "color": "#6A9955", "fontStyle": "italic" }]
        },
        // 3. Keywords (if, else, for, while, print)
        "(?<![\"\\w])\\b(if|else|while|print|for)\\b(?![\"\\w])": {
            "filterFileRegex": ".*\\.vhg$",
            "decorations": [{ "color": "#C586C0" }]
        },
        // 4. logical operators (and, or, not)
        "(?<![\"\\w])\\b(and|or|not)\\b(?![\"\\w])": {
        "filterFileRegex": ".*\\.vhg$",
        "decorations": [{"color": "#0b4da3"}]
    },
    // 5. Variables
    "\\b(?!(?:if|else|while|print|for|and|or|not)\\b)[a-zA-Z_][a-zA-Z0-9_]*\\b": {
        "filterFileRegex": ".*\\.vhg$",
        "decorations": [{ "color": "#9CDCFE" }] 
    },
    // 6. Symbols and numbers
    "[\\(\\)\\{\\};]": {
        "filterFileRegex": ".*\\.vhg$",
        "decorations": [{ "color": "#D4D4D4" }] 
    },
    "\\b(\\d+(\\.\\d+)?)\\b": {
        "filterFileRegex": ".*\\.vhg$",
        "decorations": [{ "color": "#B5CEA8" }]
    }
    },
```

### Step 5: Finalize
1. **Remove old shortcuts:** Delete any `ctrl + /` bindings in `keybindings.json`.
2. **Restart VS Code:** This is required for the new extension to load.

### Key Features of this Setup:
* Native Toggle: `Ctrl + /` now works instantly using the `#` symbol.
* Auto-Close: Typing `#*`, `(`, `[` or `{` will automatically generate the closing pair.
* Semantic Colors: Your code is now visually organized and easy to read.