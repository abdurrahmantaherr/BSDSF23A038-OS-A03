# 🐚 FCIT Custom Shell (myshell)

**Course:** Operating Systems  
**Student:** Abdurrahman Tahir (BSDSF23A038)  
**Instructor:** —  
**Version:** v8  
**Repository:** [github.com/abdurrahmantaherr/BSDSF23A038-OS-A03](https://github.com/abdurrahmantaherr/BSDSF23A038-OS-A03)

---

## 📘 Overview

`myshell` is a **custom Linux command-line shell** built entirely in **C** as part of the Operating Systems course project.  
It implements core shell features including **command execution, redirection, pipelines, background jobs, signal handling**, and even **command history with readline support**.

The shell mimics key functionalities of bash while maintaining simplicity and clarity for learning purposes.

---

## 🧩 Features Summary

| Feature | Description | Version |
|----------|--------------|----------|
| **Feature 1** | Basic shell with input parsing & command execution (`fork`, `execvp`) | v1 |
| **Feature 2** | Built-in commands (`cd`, `exit`, `help`, `jobs`) | v2 |
| **Feature 3** | Command history with `history` and `!n` re-execution | v3 |
| **Feature 4** | Integrated GNU **readline** for line editing, arrow keys, and tab completion | v4 |
| **Feature 5** | Input/output redirection (`<`, `>`) and single pipe (`|`) support | v5 |
| **Feature 6** | Command chaining (`;`), background jobs (`&`), and `jobs` builtin | v6 |
| **Feature 7** | Signal handling for `Ctrl+C` and `Ctrl+Z`, safe terminal control | v7 |
| **Feature 8** | Append redirection (`>>`), multiple pipes, and `fg <job>` command | v8 |

---

## ⚙️ Build Instructions

### 🧰 Prerequisites
Ensure your Linux environment has:
```bash
sudo apt update
sudo apt install build-essential libreadline-dev
