# CS214 Project II: Word Frequency

## Authors
| **Name** | **NetID** |
|----------|-------------|
| ![Vishal Nagamalla](https://github.com/Vishal-Nagamalla) | vn218 |
| ![Yuhan Li](https://github.com/HiT-T) | yl2355 |

---

## Description

This program, `outlier.c`, processes a set of ASCII-formatted text files and computes word frequencies in each file. It identifies the word in each file that shows the greatest relative increase in frequency compared to its average usage across all files.

The program accepts one or more arguments, each of which may be a file or a directory:
- Files are scanned directly regardless of their names.
- Directories are traversed recursively, and only files ending in `.txt` within the dirs (and their sub-dirs (and their sub-dirs (...))) are scanned.

Output format (one line per file): 

```
filename : word
```

---

## Design Choices

- **Input Handling**: Used `open()` and `read()` as required by the spec to manually read file contents in fixed-size buffers.
- **Word Definition**: A token is considered a valid word if it:
  - Does **not** start with: `(`, `[`, `{`, `"`, or `'`
  - Does **not** end with: `)`, `]`, `}`, `"`, `'`, `,`, `.`, `!`, or `?`
  - Contains **at least one letter**
- **Word Parsing**: Implemented a tokenizer using a simple state machine that accumulates characters and finalizes a word upon encountering whitespace.
- **Frequency Counting**: Each file has a WordEntry `linked list` to store word counts, and a global list accumulates the overall word statistics.
- **Case Insensitivity**: All words are lowercased using `tolower()` before insertion and comparison.

---

## Testing Strategy

- **Functional Testing**:
  - Tested with small hand-crafted files (`A.txt`, `B.txt`, `C.txt`) containing overlapping and unique words.
  - Included edge cases like:
    - Words surrounded by punctuation
    - Words with only symbols or numbers
    - Words with mixed case (e.g., `Spam` vs `spam`)
- **Directory Traversal**:
  - Created nested directory structures to ensure `.txt` filtering and recursion worked correctly.
- **Stress Testing**:
  - Used large `.txt` files (100KB+) to confirm correct buffer handling and performance under load.
- **Valgrind**:
  - Verified with `valgrind` to ensure there are **no memory leaks**, invalid reads/writes, or use-after-free errors.
  
---

## Compilation & Execution

To compile the project:

```
make
```

To run the program:

```
./outlier path/to/file_or_directory ...
```

## Notes

- This program assumes **all input files are ASCII-formatted**, and it does **not** handle UTF-8 or non-English characters.
- Only files explicitly ending with `.txt` are included during directory traversal.
- To demonstrate a mastery of the `POSIX API`, this code avoids use of `fopen()`/`fread()` and instead relies on `open()` and `read()`.
