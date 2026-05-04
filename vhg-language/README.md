## Guide: Setting Up Native VHG Language Support in VS Code

This setup allows VS Code to recognize `.vhg` files as a formal language, enabling **perfect "Ctrl + /" comment toggling**, auto-closing brackets, and custom syntax highlighting without any buggy snippets.

### Step 1: Create the Extension Folder
1. Open your File Explorer.
2. Navigate to the VS Code extensions folder. You can do this by pasting this path into your address bar:
   `%USERPROFILE%\.vscode\extensions` (on Windows).
   `~/.vscode/extensions` (on Linux).
   `~/.vscode/extensions` (on macOS).

3. Create a new folder named `vhg-lang`.

* Or you can just simply copy and paste this folder into `EXTENSION_PATH` (based on OS you're using). Rename this folder to: 
    `vahagn-mardyan.vhg-language-0.0.1`
---

### Step 2: Create the Configuration Files
Inside the `vhg-lang` folder, create the following two files:

#### 1. `package.json`
This file registers the language ID and connects the `.vhg` extension.
```json
{
    "name": "vhg-language",
    "displayName": "VHG Language Support",
    "version": "0.0.1",
    "publisher": "vahagn-mardyan",
    "engines": {
        "vscode": "^1.80.0"
    },
    "categories": ["Programming Languages"],
    "contributes": {
        "languages": [{
            "id": "vhg",
            "extensions": [".vhg"],
            "configuration": "./language-configuration.json"
        }],
        "grammars": [{
            "language": "vhg",
            "scopeName": "source.vhg",
            "path": "./syntaxes/vhg.tmLanguage.json"
        }],
        "snippets": [{
            "language": "vhg",
            "path": "./snippets/vhg.json"
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
        { "include": "#math_functions" },
        { "include": "#function_definition" },
        { "include": "#function_call" },
        { "include": "#boolean" },
        { "include": "#logical_operators" },
        { "include": "#numbers" },
        { "include": "#operators" },
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
            "match": "\\b(if|else|while|for|print|function|return|import|break|continue|switch|case|default)\\b",
            "name": "keyword.control.vhg"
        },
        "boolean": {
            "match": "\\b(true|false|none)\\b",
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
        "builtIn": {
            "match": "\\b(input|length|void|m_e|m_pi|ord|chr|bin|oct|dec|hex|type)\\b",
            "name": "support.function.vhg"
        },
        "math_functions": {
            "match": "\\b(sin|cos|tan|asin|acos|atan|atan2|sqrt|cbrt|exp|log|log2|log10|log_ab|ceil|floor|round|abs|fmod)\\b",
            "name": "support.math.functions.vhg"
        },
        "function_definition": {
            "match": "\\bfunction\\s+([a-zA-Z_][a-zA-Z0-9_]*)\\s*\\(",
            "captures": {
                "1": { "name": "entity.name.function.vhg" }
            }
        },
        "function_call": {
            "match": "\\b([a-zA-Z_][a-zA-Z0-9_]*)\\s*\\(",
            "captures": {
                "1": { "name": "support.function.user.vhg" }
            }
        },
        "operators": {
            "match": "(\\+|-|\\*|/|%|&|\\||\\^|<<|>>|\\+\\=|\\-\\=|\\*\\=|\\/\\=|\\%\\=|\\^\\=)?=",
            "name": "keyword.operator.assignment.vhg"
        },
        "variables": {
            "match": "\\b[a-zA-Z_][a-zA-Z0-9_]*\\b",
            "name": "variable.other.vhg"
        }
    }
}
```

#### 4. `snippets/vhg.json`
```
{
    "sin function": {
        "prefix": "sin",
        "body": "sin(${1:angle})",
        "description": "Sine function (sinus)"
    },
    "cos function": {
        "prefix": "cos",
        "body": "cos(${1:angle})",
        "description": "Cosine function"
    },
    "tan function": {
        "prefix": "tan",
        "body": "tan(${1:angle})",
        "description": "Tangent function"
    },
    "asin function": {
        "prefix": "asin",
        "body": "asin(${1:value})",
        "description": "Arc sine (inverse sine)"
    },
    "acos function": {
        "prefix": "acos",
        "body": "acos(${1:value})",
        "description": "Arc cosine"
    },
    "atan function": {
        "prefix": "atan",
        "body": "atan(${1:value})",
        "description": "Arc tangent"
    },
    "atan2 function": {
        "prefix": "atan2",
        "body": "atan2(${1:y}, ${2:x})",
        "description": "2-argument arctangent (y, x)"
    },
    "sqrt function": {
        "prefix": "sqrt",
        "body": "sqrt(${1:number})",
        "description": "Square root"
    },
    "cbrt function": {
        "prefix": "cbrt",
        "body": "cbrt(${1:number})",
        "description": "Cube root"
    },
    "pow function": {
        "prefix": "pow",
        "body": "pow(${1:base}, ${2:exponent})",
        "description": "Power function (base^exponent)"
    },
    "exp function": {
        "prefix": "exp",
        "body": "exp(${1:number})",
        "description": "Exponential function (e^x)"
    },
    "log function": {
        "prefix": "log",
        "body": "log(${1:number})",
        "description": "Natural logarithm (base e)"
    },
    "log2 function": {
        "prefix": "log2",
        "body": "log2(${1:number})",
        "description": "Base-2 logarithm"
    },
    "log10 function": {
        "prefix": "log10",
        "body": "log10(${1:number})",
        "description": "Base-10 logarithm"
    },
    "ln function": {
        "prefix": "ln",
        "body": "ln(${1:number})",
        "description": "Natural logarithm (same as log)"
    },
    "ceil function": {
        "prefix": "ceil",
        "body": "ceil(${1:number})",
        "description": "Round up to nearest integer"
    },
    "floor function": {
        "prefix": "floor",
        "body": "floor(${1:number})",
        "description": "Round down to nearest integer"
    },
    "round function": {
        "prefix": "round",
        "body": "round(${1:number})",
        "description": "Round to nearest integer"
    },
    "abs function": {
        "prefix": "abs",
        "body": "abs(${1:number})",
        "description": "Absolute value"
    },
    "fmod function": {
        "prefix": "fmod",
        "body": "fmod(${1:x}, ${2:y})",
        "description": "Floating-point remainder (x mod y)"
    },
    "length function": {
        "prefix": "length",
        "body": "length(${1:string})",
        "description": "String length"
    },
    "m_pi constant": {
        "prefix": "m_pi",
        "body": "m_pi",
        "description": "Mathematical constant π (3.14159...)"
    },
    "m_e constant": {
        "prefix": "m_e",
        "body": "m_e",
        "description": "Mathematical constant e (2.71828...)"
    },
    "if statement": {
        "prefix": "if",
        "body": [
            "if (${1:condition}) {",
            "\t${2:// code}",
            "}"
        ],
        "description": "If statement"
    },
    "if-else statement": {
        "prefix": "ife",
        "body": [
            "if (${1:condition}) {",
            "\t${2:// code}",
            "} else {",
            "\t${3:// code}",
            "}"
        ],
        "description": "If-else statement"
    },
    "while loop": {
        "prefix": "while",
        "body": [
            "while (${1:condition}) {",
            "\t${2:// code}",
            "}"
        ],
        "description": "While loop"
    },
    "for loop": {
        "prefix": "for",
        "body": [
            "for (${1:i = 0}; ${2:i < 10}; ${3:i = i + 1}) {",
            "\t${4:// code}",
            "}"
        ],
        "description": "For loop"
    },
    "function definition": {
        "prefix": "func",
        "body": [
            "function ${1:name}(${2:params}) {",
            "\t${3:// body}",
            "}"
        ],
        "description": "Function definition"
    },
    "void function": {
        "prefix": "void",
        "body": [
            "void function ${1:name}(${2:params}) {",
            "\t${3:// body}",
            "}"
        ],
        "description": "Void function definition"
    },
    "input statement": {
        "prefix": "input",
        "body" : "input(${1:prompt})",
        "description": "User-input"
    },
    "print statement": {
        "prefix": "print",
        "body": "print(${1:expression});",
        "description": "Print statement"
    },
    "return statement": {
        "prefix": "return",
        "body": "return ${1:value};",
        "description": "Return statement"
    },
    "or keyword": {
        "prefix" : "or",
        "body" : "or",
        "description" : "Logical or"
    },
    "and keyword": {
        "prefix" : "and",
        "body" : "and",
        "description" : "Logical and"
    },
    "not keyword": {
        "prefix" : "not",
        "body" : "not",
        "description" : "Logical not"
    },
    "true keyword": {
        "prefix" : "true",
        "body" : "true",
        "description" : "Boolean true"
    },
    "false keyword": {
        "prefix" : "false",
        "body" : "false",
        "description" : "Boolean false"
    },
    "none keyword": {
        "prefix": "none",
        "body": "none",
        "description": "none type"
    },
    "import keyword": {
        "prefix": "import",
        "body": "import ''",
        "description": "Import variables or functions from other files."
    },
    "break keyword": {
        "prefix":"break",
        "body": "break;",
        "description": "Exit loop earlier/switch-case"
    },
    "continue keyword": {
        "prefix": "continue",
        "body": "continue;",
        "description": "Skip next iteration"
    },
    "switch-case": {
        "prefix":"switch",
        "body": [
            "switch(${1:value}) {",
            "\tcase c1, c2: ",
            "break;",
            "default:",
            "}"
        ],
        "description": "switch-case"
    },
    "ord": {
        "prefix": "ord",
        "body": [
            "ord(string_argument)"
        ],
        "description": "Returns the ASCII code point for one-character string."
    },
    "chr": {
        "prefix": "chr",
        "body": [
            "chr(number_argument)"
        ],
        "description": "Returns the ASCII string of one character with ordinal i; 0 <= i <= 255."
    },
    "bin": {
        "prefix": "bin",
        "body": [
            "bin(integer)"
        ],
        "description": "Returns the binary representation of an integer."
    },
    "oct": {
        "prefix": "oct",
        "body": [
            "oct(integer)"
        ],
        "description": "Returns the octal representation of an integer."
    },
    "hex": {
        "prefix": "hex",
        "body": [
            "hex(integer)"
        ],
        "description": "Returns the hexadecimal representation of an integer."
    },
    "dec": {
        "prefix": "dec",
        "body": [
            "dec(string)"
        ],
        "description": "Returns the decimal representation of given argment (if possible)"
    },
    "type": {
        "prefix": "type",
        "body": [
            "type(argument)"
        ],
        "description": "Returns the type of given argument"
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
        { "scope": "keyword.control_boolean.vhg", "settings": {"foreground": "#1e73ea"} },
        { "scope": "string.quoted.double.vhg", "settings": { "foreground": "#CE7744" } },
        { "scope": "string.quoted.single.vhg", "settings": { "foreground": "#CE7744" } },
        { "scope": "constant.numeric.vhg", "settings": { "foreground": "#B5CEA8" } },
        { "scope": "variable.other.vhg", "settings": { "foreground": "#9CDCFE" } },
        { "scope": "comment.line.vhg", "settings": { "foreground": "#6A9955", "fontStyle": "italic" } },
        { "scope": "comment.block.vhg", "settings": { "foreground": "#6A9955", "fontStyle": "italic" } },
        { "scope": "support.function.vhg", "settings": { "foreground": "#007bff" } },
        { "scope": "support.math.functions.vhg", "settings": { "foreground": "#DCCE80" } },
        { "scope": "entity.name.function.vhg", "settings": { "foreground": "#DCDCAA" } },
        { "scope": "support.function.user.vhg", "settings": { "foreground": "#DCDCAA" } },
        { "scope": "keyword.operator.assignment.vhg", "settings": { "foreground": "#C586C0" } }
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